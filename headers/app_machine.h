#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// typedef enum {
//     APP_INITIALIZE,
//     APP_READY,
//     APP_WIFI,
//     APP_WAIT,
//     APP_BLE_SCANNING,
//     APP_BLE_FOUND,
//     APP_BLE_NOT_FOUND
// } APP_STATE;

typedef enum {
    APP_INITIALIZE,
    APP_READY,
    APP_BUSY,
} APP_STATE;

typedef enum {
    NOP = 0,
    FSM_DONE,
} APP_NOTIFICATION;

void run_app_state_machine();
void app_goto_wifi();

#endif