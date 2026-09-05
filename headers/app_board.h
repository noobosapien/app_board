#ifndef APP_BOARD_H
#define APP_BOARD_H

typedef enum {
    SPLASH,
    EPD,
    WIFI,
    BLE
} FSM;

typedef enum {
    EVENT_SPLASH_DRAW = 1,
    EVENT_EPD_DRAW,
    EVENT_WIFI_SCAN,
    EVENT_WIFI_AVAILABLE,
    EVENT_WIFI_NOT_AVAILABLE,
    EVENT_BLE_SCAN,
    EVENT_BLE_AVAILABLE,
    EVENT_BLE_NOT_AVAILABLE,
} FSM_Event;

typedef struct {
    FSM fsm;
    FSM_Event event;
} app_event;

/*!
    @brief  Struct that holds a commands and related data bytes to be sent through SPI
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} epd_init_cmd_t;

#define SPLASH_INITIALIZED 1
#define EPD_INITIALIZED 2
#define WIFI_INITIALIZED 3
#define BLE_INITIALIZED 4


#endif