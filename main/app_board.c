#include "app_pch.h"

extern QueueHandle_t fb_queue;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red
extern size_t buffer_size;

EventGroupHandle_t app_events;
QueueHandle_t app_event_queue;
SemaphoreHandle_t epd_mutex;
TaskHandle_t app_task = NULL;
TaskHandle_t splash_task = NULL;
TaskHandle_t epd_task = NULL;
TaskHandle_t wifi_task = NULL;
TaskHandle_t ble_task = NULL;

static app_event event_rx;


// Main function
/**************************************************************************/
/*!
   @brief   Run the tasks
*/
/**************************************************************************/

void app_main(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    app_events = xEventGroupCreate();
    xEventGroupClearBits(app_events, 0x00ffffff);
    app_event_queue = xQueueCreate(4, sizeof(app_event));

    epd_mutex = xSemaphoreCreateMutex();

    // TODO: Error handling

    if (xTaskCreate(run_app_state_machine, "APP", 1024*8, NULL, 1, &app_task) != pdPASS) {
        ESP_LOGE("APP_BOARD", "Failed to create APP task");
    }
    if (xTaskCreate(splash_state_machine, "SPLASH", 1024*8, NULL, 1, &splash_task) != pdPASS) {
        ESP_LOGE("APP_BOARD", "Failed to create SPLASH task");
    }
    if (xTaskCreate(epd_state_machine, "EPD", 1024*8, NULL, 1, &epd_task) != pdPASS) {
        ESP_LOGE("APP_BOARD", "Failed to create EPD task");
    }
    if (xTaskCreate(wifi_state_machine, "WIFI", 1024*8, NULL, 1, &wifi_task) != pdPASS) {
        ESP_LOGE("APP_BOARD", "Failed to create WIFI task");
    }
    if (xTaskCreate(ble_state_machine, "BLE", 1024*8, NULL, 1, &ble_task) != pdPASS) {
        ESP_LOGE("APP_BOARD", "Failed to create BLE task");
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    // event_rx.fsm = SPLASH;
    // event_rx.event = EVENT_SPLASH_DRAW;
    // xQueueSend(app_event_queue, &event_rx, (TickType_t)portMAX_DELAY);    

    // vTaskDelay(50000 / portTICK_PERIOD_MS);
    event_rx.fsm = WIFI;
    event_rx.event = EVENT_WIFI_SCAN;
    xQueueSend(app_event_queue, &event_rx, (TickType_t)portMAX_DELAY);   
    
}
