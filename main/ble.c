#include "app_pch.h"

extern EventGroupHandle_t app_events;
extern QueueHandle_t app_event_queue;
extern TaskHandle_t app_task;
extern uint8_t *frame_buffer; // B/W
extern uint8_t *frame_buffer_red; // Red
extern size_t buffer_size;

static BLE_config config = {
    .value = 0,
    .current_state = BLE_INITIALIZE,
    .addr_val = {0},
    .event_rx = {.fsm = BLE, .event = EVENT_BLE_SCAN},
    .all_devices = {},
    .current_devices = 0,
    .setup_completed = false,
    .scan_completed = false
};

inline static void format_addr(char *addr_str, uint8_t addr[]) {
    sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1],
            addr[2], addr[3], addr[4], addr[5]);
}

static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
            struct ble_hs_adv_fields fields;
            // Parse advertisment to C struct - fileds
            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);

            // Max devices 5
            if(config.current_devices >= 5) break;

            // Add address to device
            for(int i = 0; i < 6; i++){
                config.all_devices[config.current_devices].addr[i] = event->disc.addr.val[i];
            }

            // Add the strength
            config.all_devices[config.current_devices].rssi = event->disc.rssi;
            config.current_devices++;

            return 0;
        }

        case BLE_GAP_EVENT_DISC_COMPLETE:
            // Turn the flag on
            config.scan_completed = true;
            return 0;

        default:
            return 0;
    }

    return 0;
}


static void start_advertising(void) {
    struct ble_gap_disc_params scan_params;
    memset(&scan_params, 0, sizeof(scan_params));

    // Ignore the same device packets
    scan_params.filter_duplicates = 1;   
    scan_params.passive = 0;

    // Scan interval and window
    scan_params.itvl = BLE_GAP_SCAN_ITVL_MS(100);
    scan_params.window = BLE_GAP_SCAN_WIN_MS(90);

    // NimBLE discovery procedure with ble_gap_event callback function
    // For 10 seconds
    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, 10000, &scan_params, ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to start BLE scan; rc=%d", rc);
    }
}

static void adv_init(){
    int rc = 0;
    char addr_str[18] = {0};

    // Make sure the proper BT id address is set
    rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(TAG, "device does not have any available bt address!");
        return;
    }

    // BT address to use while advertising
    rc = ble_hs_id_infer_auto(0, &config.own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to infer address type, error code: %d", rc);
        return;
    }

    // Copy device address to addr value
    rc = ble_hs_id_copy_addr(config.own_addr_type, config.addr_val, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to copy device address, error code: %d", rc);
        return;
    }
    format_addr(addr_str, config.addr_val);
    ESP_LOGI(TAG, "device address: %s", addr_str);
}

static int gap_init(){
    int rc = 0;

    // Initiate the GAP service
    ble_svc_gap_init();
    // Set the device name
    rc = ble_svc_gap_device_name_set(DEVICE_NAME);

    if(rc != 0){
        ESP_LOGE("BLE", "Failed to set device name to %s, error code: %d", 
            DEVICE_NAME, rc);
            return rc;
    }

    // Set the device appearance
    rc = ble_svc_gap_device_appearance_set(BLE_GAP_APPEARANCE_GENERIC_TAG);
    if(rc != 0){
        ESP_LOGE("BLE", "Failed to set device appearance, error code: %d", rc);
        return rc;
    }

    return rc;
}

static void on_stack_reset(int reason){
    ESP_LOGI("BLE", "Nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(){
    adv_init();
    config.setup_completed = true;
}

static void nimble_host_config_init(){
    // ble_hs_cfg is a global variable
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_store_config_init();
}

static void nimble_host_task(void* param){
    ESP_LOGI("BLE", "Nimble host task has been started");
    nimble_port_run(); // Call nimble_port_stop() to exit this
    vTaskDelete(NULL); // Exit the task
}


static void nimble_exit_task(void* param){
    ESP_LOGI("BLE", "Nimble host will be stopped.");
    nimble_port_stop(); 
    vTaskDelete(NULL); // Exit the task
}

static void render_ble_scan(){
    write_line all_lines[] = {
        {0, 10, "BLE Scanner", 12, 0x00, frame_buffer},
        {0, 60, "Scanning For BLE...", 20, 0xff, frame_buffer_red}
    };

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

static void render_ble_available(){
    const uint16_t last_idx = 4;
    write_line all_lines[10] = {
        {0, 10, "BLE Scanner", 12, 0x00, frame_buffer},
        {0, 30, "-------------------------", 25, 0x00, frame_buffer},
        {0, 40, "MAC              Strength", 25, 0xff, frame_buffer_red},
        {0, 50, "-------------------------", 25, 0x00, frame_buffer}
    };

    for(uint16_t i = last_idx; i < last_idx + config.current_devices ; i++){
        all_lines[i].x = 0;
        all_lines[i].y = 20 + i * 10;
        all_lines[i].len = 25;
        all_lines[i].color = 0xff;
        all_lines[i].frame_buffer = frame_buffer_red;

        snprintf(all_lines[i].text, sizeof(all_lines[i].text), "%02x:%02x:%02x:%02x:%02x:%02x   %d",
        config.all_devices[i-last_idx].addr[5], config.all_devices[i-last_idx].addr[4], config.all_devices[i-last_idx].addr[3], config.all_devices[i-last_idx].addr[2], 
        config.all_devices[i-last_idx].addr[1], config.all_devices[i-last_idx].addr[0], config.all_devices[i-last_idx].rssi);
    }

    all_lines[last_idx + config.current_devices] = (write_line){176, 120, "<<", 2, 0x00, frame_buffer};

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

static void render_ble_not_available(){
    write_line all_lines[] = {
        {0, 10, "BLE Scanner", 12, 0x00, frame_buffer},
        {0, 60, "No BLE Devices Found", 21, 0xff, frame_buffer_red},
        {176, 120, "<< >>", 5, 0x00, frame_buffer}
    };

    epd_acquire_and_draw(all_lines, sizeof(all_lines)/sizeof(write_line));
}

void ble_state_machine(){
    while(true){
        switch(config.current_state){
            case BLE_INITIALIZE:
                config.setup_completed = false;
                esp_err_t ret = ESP_OK;
                int rc = 0;

                // Initiate the NimBLE stack
                ret = nimble_port_init();
                if (ret != ESP_OK) {
                    ESP_LOGE("BLE", "failed to initialize nimble stack, error code: %d ",
                            ret);
                    return;
                }

                // Initiate the GAP service
                rc = gap_init();
                if (rc != 0) {
                    ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
                    return;
                }

                nimble_host_config_init();
                xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL, 5, NULL);
                config.current_state = BLE_READY;
                break;
            
            case BLE_READY:
                xEventGroupSetBits(app_events, 1 << BLE_INITIALIZED);

                config.n_success = xTaskNotifyWait(0xffffffff, 0xffffffff, &config.value, portMAX_DELAY);
                config.scan_completed = false;

                if(config.n_success == pdTRUE){
                    config.current_notification = (FSM_Event) config.value;
                    switch(config.current_notification){
                        case EVENT_BLE_SCAN:
                            config.current_state = BLE_SCAN;
                            break;
                        case EVENT_BLE_AVAILABLE:
                            config.current_state = BLE_AVAILABLE;
                            break;
                        case EVENT_BLE_NOT_AVAILABLE:
                            config.current_state = BLE_NOT_AVAILABLE;
                            break;
                        default:
                            break;
                    }

                }else{
                    // TODO: Error
                }

                break;
            
            case BLE_SCAN:
                render_ble_scan();
                while(!config.setup_completed) vTaskDelay(50/ portTICK_PERIOD_MS);
                start_advertising();
                while(!config.scan_completed) vTaskDelay(50/ portTICK_PERIOD_MS);

                if(config.current_devices == 0){
                    config.event_rx.fsm = BLE; 
                    config.event_rx.event = EVENT_BLE_NOT_AVAILABLE;
                }else{
                    config.event_rx.fsm = BLE; 
                    config.event_rx.event = EVENT_BLE_AVAILABLE;
                }

                xQueueSend(app_event_queue, &config.event_rx, (TickType_t)portMAX_DELAY);
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                config.current_state = BLE_READY;
                break;
            
            case BLE_AVAILABLE:
                render_ble_available();
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                config.current_state = WIFI_READY;
                break;
            
            case BLE_NOT_AVAILABLE:
                render_ble_not_available();
                xTaskNotify(app_task, (uint32_t)FSM_DONE, eSetValueWithOverwrite);

                config.current_state = WIFI_READY;
                break;
            
            case BLE_EXIT:
                xTaskCreate(nimble_exit_task, "NimBLE Host Exit", 4 * 1024, NULL, 5, NULL);
                break;
        }
    }
}