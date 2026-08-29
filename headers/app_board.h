#ifndef APP_BOARD_H
#define APP_BOARD_H

/*!
    @brief  Struct that holds a commands and related data bytes to be sent through SPI
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} epd_init_cmd_t;


#endif