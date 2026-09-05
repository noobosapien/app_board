#include "app_pch.h"

extern QueueHandle_t fb_queue;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red

extern EventGroupHandle_t app_events;
extern QueueHandle_t app_event_queue;
extern TaskHandle_t splash_task;
extern TaskHandle_t epd_task;
extern TaskHandle_t app_task;
extern TaskHandle_t wifi_task;
extern TaskHandle_t ble_task;

static APP_STATE current_state = APP_INITIALIZE;
static uint8_t ph = 1;
static BaseType_t n_success;
static uint32_t value;
static APP_NOTIFICATION current_notification = NOP;

static app_event event_rx;


void run_app_state_machine(){
    while(1){
        switch(current_state){
            case APP_INITIALIZE:
                // xEventGroupWaitBits( app_events, 1 << SPLASH_INITIALIZED | 1 << EPD_INITIALIZED | 1 << WIFI_INITIALIZED | 1 << BLE_INITIALIZED,
                //       pdFALSE, pdTRUE, portMAX_DELAY );
                current_state = APP_READY;

                break;

            case APP_READY:
                // TODO: Turn on interrupt handlers

                if(xQueueReceive(app_event_queue, &(event_rx), (TickType_t)portMAX_DELAY) == pdPASS){
                    // TODO: Turn off interrupt handlers
                    switch(event_rx.fsm){
                        case SPLASH:
                            // Call Splash task notification
                            xTaskNotify(splash_task, (uint32_t)event_rx.event, eSetValueWithOverwrite);
                            break;
                        case EPD:
                            // Call EPD task notification
                            xTaskNotify(epd_task, (uint32_t)event_rx.event, eSetValueWithOverwrite);
                            break;

                        case WIFI:
                            // Call Wifi task notification
                            xTaskNotify(wifi_task, (uint32_t)event_rx.event, eSetValueWithOverwrite);
                            break;
                        
                        case BLE:
                            // Call BLE task notification
                            xTaskNotify(ble_task, (uint32_t)event_rx.event, eSetValueWithOverwrite);
                            break;
                    }
                }
                current_state = APP_BUSY;
                break;

            case APP_BUSY:
                n_success = xTaskNotifyWait(0xffffffff, 0xffffffff, &value, portMAX_DELAY);

                if(n_success == pdTRUE){

                    current_notification = (APP_NOTIFICATION) value;
                    switch(current_notification){
                        case NOP:
                            break;
                        case FSM_DONE:
                            current_state = APP_READY;
                            break;
                        default:
                            current_notification = NOP;
                            break;
                    }

                }else{
                    // TODO: Error
                }

                break;
            
            default:
                current_state = APP_INITIALIZE;
                break;
        }
    }
}