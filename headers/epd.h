#ifndef EPD_H
#define EPD_H

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

typedef enum {
    EPD_INIT,
    EPD_INIT_SPI,
    EPD_INIT_CONFIG,
    EPD_INIT_POWER_ON,
    EPD_READY,
    EPD_SEND_FRAMES,
    EPD_WAIT_FRAME
} EPD_STATE;

void epd_state_machine();

#endif