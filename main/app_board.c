#include <stdio.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"

#include "text.h"
#include "app_board.h"

spi_device_handle_t spi_device;
const int display_height = 250;
const int display_width = 128;

uint8_t *frame_buffer = NULL; // B/W
uint8_t *frame_buffer_red = NULL; // Red
static size_t buffer_size = 0;

// Both the sequences should be in the Data RAM
DRAM_ATTR static const epd_init_cmd_t init_cmds[] = {
    {0x12, {},0}, // Software reset
    {0x01, {0xf9, 0x00, 0x00}, 3}, // Driver output control / Gate setting
    {0x11, {0x03}, 1}, // Data entry mode / Data entry sequence / X, Y address auto increment
    {0x3C, {0x05}, 1}, // Border waveform control / Follow LUT 1
    {0x18, {0x80}, 1}, // Temperature sensor control / Internal
    {0x21, {0x00, 0x80}, 2} // Display update control / Normal / Source from S0 - S175
};

DRAM_ATTR static const epd_init_cmd_t power_on_cmds1[] = {
    {0x22, {0xf7}, 1}, // Display update control 2 / Enable clock signal
    {0x20, {}, 0}, // Master activation / Activate display update sequence
};

// Change the DC line before a transaction
/**************************************************************************/
/*!
   @brief   Callback function to decide whether the DC line should be high or low
    @param    t   SPI transaction to send
*/
/**************************************************************************/

static void lcd_spi_pre_transfer_callback(spi_transaction_t* t){
    // Check the user flag: if it is a command line will be low else high
    gpio_set_level(SSD1680_DC, (int)t->user);
}

// Check for busy flag
/**************************************************************************/
/*!
   @brief   Check the busy flag as long as it is 1 the EPD is busy
*/
/**************************************************************************/

void lcd_chkstatus(void)
{
	while(gpio_get_level(SSD1680_BUSY)==1){
        vTaskDelay(50 / portTICK_PERIOD_MS);
    } 
}

// Send a command through SPI
/**************************************************************************/
/*!
   @brief   Send a command and required data from a SPI transaction
    @param    cmd   Command byte
    @param    data  Data associated with the command
    @param    len   Length of the data
*/
/**************************************************************************/

static void epd_cmd(const uint8_t cmd, const uint8_t* data, int len){
    // Create 2 SPI transactions, 1 for the command byte and the other for data
    spi_transaction_t tx[2] = {
        // .user is handled by the callback
        {.length = 8, .tx_buffer = &cmd, .user = U_CMD},
        {.length = len * 8, .tx_buffer = data, .user = U_DATA}
    };

    ESP_ERROR_CHECK(spi_device_transmit(spi_device, &tx[0]));
    if(len > 0){
        ESP_ERROR_CHECK(spi_device_transmit(spi_device, &tx[1]));
    }
}


// Initialize GPIOs
/**************************************************************************/
/*!
   @brief   Initialize the pins for RST, DC and BUSY
*/
/**************************************************************************/

void init_gpios(void)
{
    gpio_config_t dc_rst_conf = {
        .pin_bit_mask = (1ULL << SSD1680_DC) | (1ULL << SSD1680_RST), // 2 GPIOS sharing the same config
        .mode = GPIO_MODE_OUTPUT, // Only output
        .pull_up_en = GPIO_PULLUP_ENABLE,     // When inactive these 2 should be high
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // This will be always active
        .intr_type = GPIO_INTR_DISABLE, // No interrupts
    };
    gpio_config(&dc_rst_conf);

    gpio_set_level(SSD1680_DC, 1); // Set these 2 explicitly to be sure
    gpio_set_level(SSD1680_RST, 1);

    // This a different config for busy pin as it should be input.
    gpio_config_t busy_conf = {
        .pin_bit_mask = (1ULL << SSD1680_BUSY),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // Default: high
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&busy_conf);
}


// Set the display area of SSD1680
/**************************************************************************/
/*!
   @brief   Setting the configuration for framebuffer in the SSD1680
    @param    x   Starting X position
    @param    y   Starting Y position
    @param    width   Width of the EPD
    @param    height Height of the EPD
*/
/**************************************************************************/

void set_partial_ram_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
    epd_init_cmd_t temp = {};
    temp.cmd = 0x44; // Set RAM X Start/End position
    temp.data[0] = x / 8; // X start
    temp.data[1] = (x + width - 1)/ 8; // X end
    temp.databytes = 2;
    epd_cmd(temp.cmd, temp.data, temp.databytes);

    temp.cmd = 0x45; // Set RAM Y Start/End position
    temp.data[0] = y % 256; // Y start
    temp.data[1] = y / 256; // Y start last bit
    temp.data[2] = (y + height - 1) % 256; // Y end
    temp.data[3] = (y + height - 1) / 256; // Y end last bit
    temp.databytes = 4;
    epd_cmd(temp.cmd, temp.data, temp.databytes);

    temp.cmd = 0x4e; // Set RAM X address counter
    temp.data[0] = x / 8; // Initial X address
    temp.databytes = 1;
    epd_cmd(temp.cmd, temp.data, temp.databytes);

    temp.cmd = 0x4f; // Set RAM Y address counter
    temp.data[0] = y % 256; // Initial address
    temp.data[1] = y / 256; // Initial address last bit
    temp.databytes = 2;
    epd_cmd(temp.cmd, temp.data, temp.databytes);
}

// Main function
/**************************************************************************/
/*!
   @brief   Initialize all systems and run the tasks
*/
/**************************************************************************/

void app_main(void)
{
    // Configure the SPI bus first
    spi_bus_config_t buscfg = {
        .mosi_io_num = SSD1680_SDA,
        .sclk_io_num = SSD1680_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = display_height * display_width * 2 + 8
    };

    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &buscfg, 1)); // The dev board is originally designed to use only this host

    // Add the device to the bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000, // 4MHz is enough for this, depending on the tracks might have to change later
        .mode = 0,
        .spics_io_num = SSD1680_CS,
        .queue_size = 8,
        .pre_cb = lcd_spi_pre_transfer_callback, // This callback decides whether it is data or a command
        .flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX
    };

    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &devcfg, &spi_device));
    init_gpios(); // Initialize the other GPIOs

    buffer_size = (display_height * display_width) / 8; // Bit packed: Each pixel is 1 bit
    
    // Need 2 frame buffers, 1 for B/W and the other for Red
    frame_buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    frame_buffer_red = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);

    if (frame_buffer == NULL || frame_buffer_red == NULL) {
        ESP_LOGE("EPD", "Failed to allocate memory for frame buffer!");
        return;
    }

    // Hardware reset
    gpio_set_level(SSD1680_RST, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(SSD1680_RST, 1); 
    vTaskDelay(60 / portTICK_PERIOD_MS);

    //Software reset
    epd_cmd(init_cmds[0].cmd, init_cmds[0].data, init_cmds[0].databytes);
    vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms from the logic analyzer
    for(int i = 1; i < 6; i++){
        // Send all the config bytes
        epd_cmd(init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].databytes);
    }

    // Set the SSD1680 RAM for the framebuffer
    set_partial_ram_area(0, 0, display_width, display_height);

    memset(frame_buffer, 0xff, buffer_size);
    memset(frame_buffer_red, 0x00, buffer_size);
    
    // Placeholder
    draw_char_line(0, 10, "Wifi / BLE Scanner", 18, 0x00, frame_buffer);
    draw_char_line(0, 60, "| SSID_1 ###", 12, 0xff, frame_buffer_red);
    draw_char_line(0, 120, "Total: 1", 8, 0x00, frame_buffer);
    epd_cmd(0x26, frame_buffer_red, buffer_size); // Send RED
    epd_cmd(0x24, frame_buffer, buffer_size); // Send B/W

    // Power on after a reset
    for(int i = 0; i < 2; i++){
        epd_cmd(power_on_cmds1[i].cmd, power_on_cmds1[i].data, power_on_cmds1[i].databytes);
    }

}
