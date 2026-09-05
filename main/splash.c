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

static SPLASH_STATE current_state = SPLASH_INITIALIZE;

static void render_splash(){
    write_line all_lines[] = {
        {0, 10, "Wifi / BLE Scanner", 18, 0x00, frame_buffer},
        {0, 60, "| <<<Ready>>> |", 15, 0xff, frame_buffer_red},
        {0, 80, "Press Next To Continue...", 25, 0xff, frame_buffer_red},
        {0, 110, "By: Migara", 10, 0x00, frame_buffer},
        {0, 120, "migar256@gmail.com", 18, 0x00, frame_buffer},
        {200, 120, ">>", 2, 0x00, frame_buffer}
    };

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

void splash_state_machine(){
    while(true){
        switch(current_state){
            case SPLASH_INITIALIZE:
                current_state = SPLASH_READY;
                break;
            case SPLASH_READY:
                xEventGroupSetBits(app_events, 1 << SPLASH_INITIALIZED);

                n_success = xTaskNotifyWait(0xffffffff, 0xffffffff, &value, portMAX_DELAY);

                if(n_success == pdTRUE){

                    current_notification = (FSM_Event) value;
                    switch(current_notification){
                        case EVENT_SPLASH_DRAW:
                            current_state = SPLASH_DRAW;
                            break;
                        default:
                            current_state = SPLASH_INITIALIZE;
                            break;
                    }
                }
        
                break;
            case SPLASH_DRAW:
                // xSemaphoreTake(epd_mutex, portMAX_DELAY ); // Indefinite time not recommended

                // memset(frame_buffer, 0xff, buffer_size);
                // memset(frame_buffer_red, 0x00, buffer_size);
                // draw_char_line(0, 10, "Wifi / BLE Scanner", 18, 0x00, frame_buffer);
                // draw_char_line(0, 60, "| <<<Ready>>> |", 15, 0xff, frame_buffer_red);
                // draw_char_line(0, 80, "Press Next To Continue...", 25, 0xff, frame_buffer_red);
                // draw_char_line(0, 110, "By: Migara", 10, 0x00, frame_buffer);
                // draw_char_line(0, 120, "migar256@gmail.com", 18, 0x00, frame_buffer);
                // draw_char_line(200, 120, ">>", 2, 0x00, frame_buffer);

                // xTaskNotify(epd_task, (uint32_t)EVENT_EPD_DRAW, eSetValueWithOverwrite);
                // vTaskDelay(10 / portTICK_PERIOD_MS);
                // xEventGroupWaitBits( app_events, 1 << EPD_INITIALIZED,
                //       pdFALSE, pdTRUE, portMAX_DELAY );
                // xSemaphoreGive(epd_mutex);

                render_splash();
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = SPLASH_READY;
                break;
            default:
                current_state = SPLASH_INITIALIZE;
                break;
        }
    }
}