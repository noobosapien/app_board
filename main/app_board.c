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
#include "epd.h"

extern QueueHandle_t fb_queue;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red
extern size_t buffer_size;

// Main function
/**************************************************************************/
/*!
   @brief   Run the tasks
*/
/**************************************************************************/

void app_main(void)
{

    // Placeholder
    // draw_char_line(0, 10, "Wifi / BLE Scanner", 18, 0x00, frame_buffer);
    // draw_char_line(0, 60, "| SSID_1 ###", 12, 0xff, frame_buffer_red);
    // draw_char_line(0, 120, "Total: 1", 8, 0x00, frame_buffer);
    
    xTaskCreate(epd_state_machine, "EPD", 1024 * 16, NULL, 1, NULL);
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    uint8_t ph = 1;
    uint8_t color = 0x00;

    for(int i = 0; i < 3; i++){
        memset(frame_buffer, ~color, buffer_size);
        memset(frame_buffer_red, color, buffer_size);

        if(i == 0){
            draw_char_line(0, 10, "Wifi / BLE Scanner", 18, 0x00, frame_buffer);
            draw_char_line(0, 60, "| <<<Ready>>> |", 15, 0xff, frame_buffer_red);
            draw_char_line(0, 80, "Press Next To Continue...", 25, 0xff, frame_buffer_red);
            draw_char_line(0, 120, "By: Migara (migar256@gmail.com)", 30, 0xff, frame_buffer_red);
            draw_char_line(200, 120, ">>", 2, 0x00, frame_buffer);
            xQueueSend(fb_queue, &ph, (TickType_t)portMAX_DELAY);
        } else if(i == 1){
            draw_char_line(0, 10, "Wifi Scanner", 12, 0x00, frame_buffer);
            draw_char_line(0, 30, "-------------------------", 21, 0x00, frame_buffer);
            draw_char_line(0, 40, "Name             Strength", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 50, "-------------------------", 21, 0x00, frame_buffer);
            draw_char_line(0, 60, "SSID          ### (-42db)", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 70, "SSID          ##  (-20db)", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 80, "SSID          #   (-08db)", 21, 0xff, frame_buffer_red);
            draw_char_line(176, 120, "<< >>", 5, 0x00, frame_buffer);
            xQueueSend(fb_queue, &ph, (TickType_t)portMAX_DELAY);
        } else if(i == 2){
            draw_char_line(0, 10, "BLE Scanner", 11, 0x00, frame_buffer);
            draw_char_line(0, 30, "-------------------------", 21, 0x00, frame_buffer);
            draw_char_line(0, 40, "Name             Strength", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 50, "-------------------------", 21, 0x00, frame_buffer);
            draw_char_line(0, 60, "BLE1          ### (-42db)", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 70, "BLE2          ##  (-20db)", 21, 0xff, frame_buffer_red);
            draw_char_line(0, 80, "BLE3          #   (-08db)", 21, 0xff, frame_buffer_red);
            draw_char_line(176, 120, "<<", 3, 0x00, frame_buffer);
            xQueueSend(fb_queue, &ph, (TickType_t)portMAX_DELAY);
        }

        

        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
    
    

}
