#ifndef APP_BOARD_H
#define APP_BOARD_H

// Pin definitions
#define SSD1680_DC   25
#define SSD1680_RST  21
#define SSD1680_SDA  23
#define SSD1680_SCK  18
#define SSD1680_CS   5
#define SSD1680_BUSY 22

// If it's a command byte DC will be 0 or else 1
#define U_CMD ((void*)0)
#define U_DATA ((void*)1)

/*!
    @brief  Struct that holds a commands and related data bytes to be sent through SPI
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} epd_init_cmd_t;


#endif