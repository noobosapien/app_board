#include "app_pch.h"


extern EventGroupHandle_t app_events;
extern QueueHandle_t app_event_queue;
extern TaskHandle_t epd_task;
extern TaskHandle_t app_task;
extern TaskHandle_t wifi_task;
extern TaskHandle_t ble_task;
extern SemaphoreHandle_t epd_mutex;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red
extern size_t buffer_size;

static BaseType_t n_success;
static FSM_Event current_notification;
static uint32_t value = 0;

static BLE_STATE current_state = BLE_INITIALIZE;

static app_event event_rx = {.fsm = BLE, .event = EVENT_BLE_SCAN};

void ble_state_machine(){
    while(true){
        switch(current_state){
            case BLE_INITIALIZE:
                current_state = BLE_READY;
                break;
            
            case BLE_READY:
                xEventGroupSetBits(app_events, 1 << BLE_INITIALIZED);

                n_success = xTaskNotifyWait(0xffffffff, 0xffffffff, &value, portMAX_DELAY);

                if(n_success == pdTRUE){

                    current_notification = (FSM_Event) value;
                    switch(current_notification){
                        case EVENT_BLE_SCAN:
                            current_state = BLE_SCAN;
                            break;
                        case EVENT_BLE_AVAILABLE:
                            current_state = BLE_AVAILABLE;
                            break;
                        case EVENT_BLE_NOT_AVAILABLE:
                            current_state = BLE_NOT_AVAILABLE;
                            break;
                        default:
                            break;
                    }

                }else{
                    // TODO: Error
                }

                break;
            
            case BLE_SCAN:
                xSemaphoreTake(epd_mutex, portMAX_DELAY ); // Indefinite time not recommended

                memset(frame_buffer, 0xff, buffer_size);
                memset(frame_buffer_red, 0x00, buffer_size);
                draw_char_line(0, 10, "BLE Scanner", 12, 0x00, frame_buffer);
                draw_char_line(0, 80, "Scanning For BLE...", 20, 0xff, frame_buffer_red);

                xTaskNotify(epd_task, (uint32_t)EVENT_EPD_DRAW, eSetValueWithOverwrite);
                vTaskDelay(10 / portTICK_PERIOD_MS);
                xEventGroupWaitBits( app_events, 1 << EPD_INITIALIZED,
                      pdFALSE, pdTRUE, portMAX_DELAY );
                xSemaphoreGive(epd_mutex);

                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = BLE_READY;
                break;
            
            case BLE_AVAILABLE:
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = WIFI_READY;
                break;
            
            case BLE_NOT_AVAILABLE:
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = WIFI_READY;
                break;
        }
    }
}