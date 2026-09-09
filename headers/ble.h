#ifndef BLE_H
#define BLE_H

#define DEVICE_NAME "BLE_ESP32"
#define TAG "BLE"
#define BLE_GAP_APPEARANCE_GENERIC_TAG 0x0200

typedef enum {
    BLE_INITIALIZE,
    BLE_READY,
    BLE_SCAN,
    BLE_AVAILABLE,
    BLE_NOT_AVAILABLE,
    BLE_EXIT
} BLE_STATE;

typedef struct {
    uint8_t addr[6];
    int8_t rssi;
} ble_device;

typedef struct {
    BaseType_t n_success;
    FSM_Event current_notification;
    uint32_t value;
    BLE_STATE current_state;
    uint8_t own_addr_type;
    uint8_t addr_val[6];
    app_event event_rx;
    ble_device all_devices[5];
    uint8_t current_devices;
    bool setup_completed;
    bool scan_completed;
} BLE_config;

void ble_store_config_init(void);
void ble_state_machine();
#endif