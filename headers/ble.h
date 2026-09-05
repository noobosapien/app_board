#ifndef BLE_H
#define BLE_H

typedef enum {
    BLE_INITIALIZE,
    BLE_READY,
    BLE_SCAN,
    BLE_AVAILABLE,
    BLE_NOT_AVAILABLE
} BLE_STATE;

void ble_state_machine();
#endif