#ifndef SPLASH_H
#define SPLASH_H

typedef enum {
    SPLASH_INITIALIZE,
    SPLASH_READY,
    SPLASH_DRAW,
} SPLASH_STATE;

void splash_state_machine();

#endif