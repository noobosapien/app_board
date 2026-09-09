#include "app_pch.h"

#define SCAN_LIST_SIZE 5
#define TAG "WIFI"

extern QueueHandle_t fb_queue;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red
extern size_t buffer_size;

static WIFI_STATE current_state = WIFI_INITIALIZE;
static esp_netif_t* sta_netif = NULL;
static wifi_ap_record_t ap_info[SCAN_LIST_SIZE];

static uint16_t ap_count = 0;
static uint16_t number = 0;
static uint8_t ph = 1;
static bool draw_finished = true;
static char ssid[13] = "";
static char rssi[5] = "";

static void wifi_init(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    memset(ap_info, 0, sizeof(ap_info));
    ap_count = 0;
    number = SCAN_LIST_SIZE;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    
}

static void wifi_scan(){
    esp_wifi_scan_start(NULL, true);

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);

    for(int i = 0; i < number; i++){
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
    }
}

extern EventGroupHandle_t app_events;
extern QueueHandle_t app_event_queue;
extern TaskHandle_t epd_task;
extern TaskHandle_t app_task;
extern TaskHandle_t wifi_task;
extern TaskHandle_t ble_task;
extern SemaphoreHandle_t epd_mutex;

static BaseType_t n_success;
static FSM_Event current_notification;
static uint32_t value = 0;

static app_event event_rx = {.fsm = WIFI, .event = EVENT_WIFI_SCAN};

static void render_wifi_scan(){
    write_line all_lines[] = {
        {0, 10, "Wifi Scanner", 12, 0x00, frame_buffer},
        {0, 60, "Scanning For Wifi...", 20, 0xff, frame_buffer_red}
    };

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

static void render_wifi_available(){
    const uint16_t last_idx = 4;
    write_line all_lines[10] = {
        {0, 10, "Wifi Scanner", 12, 0x00, frame_buffer},
        {0, 30, "-------------------------", 25, 0x00, frame_buffer},
        {0, 40, "Name             Strength", 25, 0xff, frame_buffer_red},
        {0, 50, "-------------------------", 25, 0x00, frame_buffer}
    };

    for(uint16_t i = last_idx; i < last_idx + number; i++){
        all_lines[i].x = 0;
        all_lines[i].y = 20 + i * 10;
        all_lines[i].len = 25;
        all_lines[i].color = 0xff;
        all_lines[i].frame_buffer = frame_buffer_red;

        snprintf(all_lines[i].text, sizeof(all_lines[i].text), "%-13.13s       %d", (const char*)ap_info[i-last_idx].ssid, 
        ap_info[i-last_idx].rssi);
    }

    all_lines[last_idx + number] = (write_line){176, 120, "<< >>", 5, 0x00, frame_buffer};

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

static void render_wifi_not_available(){
    write_line all_lines[] = {
        {0, 10, "Wifi Scanner", 12, 0x00, frame_buffer},
        {0, 60, "No Wifi Devices Found", 21, 0xff, frame_buffer_red},
        {176, 120, "<< >>", 5, 0x00, frame_buffer}
    };

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

void wifi_state_machine(){
    while(true){
        switch(current_state){
            case WIFI_INITIALIZE:
                ESP_ERROR_CHECK(esp_netif_init());
                ESP_ERROR_CHECK(esp_event_loop_create_default());

                sta_netif = esp_netif_create_default_wifi_sta();
                assert(sta_netif);

                wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                ESP_ERROR_CHECK(esp_wifi_init(&cfg));
                
                memset(ap_info, 0, sizeof(ap_info));
                ap_count = 0;
                number = SCAN_LIST_SIZE;


                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

                ESP_ERROR_CHECK(esp_wifi_start());

                current_state = WIFI_READY;
            
                break;
            
            case WIFI_READY:
                xEventGroupSetBits(app_events, 1 << WIFI_INITIALIZED);

                n_success = xTaskNotifyWait(0xffffffff, 0xffffffff, &value, portMAX_DELAY);

                if(n_success == pdTRUE){

                    current_notification = (FSM_Event) value;
                    switch(current_notification){
                        case EVENT_WIFI_SCAN:
                            current_state = WIFI_SCAN;
                            break;
                        case EVENT_WIFI_AVAILABLE:
                            current_state = WIFI_AVAILABLE;
                            break;
                        case EVENT_WIFI_NOT_AVAILABLE:
                            current_state = WIFI_NOT_AVAILABLE;
                            break;
                        default:
                            break;
                    }

                }else{
                    // TODO: Error
                }

                break;
            
            case WIFI_SCAN:
                render_wifi_scan();

                wifi_scan();

                if(number == 0){
                    event_rx.fsm = WIFI; 
                    event_rx.event = EVENT_WIFI_NOT_AVAILABLE;
                }else{
                    event_rx.fsm = WIFI; 
                    event_rx.event = EVENT_WIFI_AVAILABLE;
                }

                xQueueSend(app_event_queue, &event_rx, (TickType_t)portMAX_DELAY);
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = WIFI_READY;
                break;
            
            case WIFI_AVAILABLE:
                render_wifi_available();

                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = WIFI_READY;
                break;
            
            case WIFI_NOT_AVAILABLE:
                render_wifi_not_available();

                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                current_state = WIFI_READY;
                break;
        }
    }
}
