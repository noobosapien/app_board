#ifndef WIFI_H
#define WIFI_H

typedef enum {
    WIFI_INITIALIZE,
    WIFI_READY,
    WIFI_SCAN,
    WIFI_AVAILABLE,
    WIFI_NOT_AVAILABLE
} WIFI_STATE;

void wifi_state_machine();

#endif