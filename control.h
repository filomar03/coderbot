#ifndef CONTROL_H
#define CONTROL_H

#include "constants.h"

#define K_P 0.5
#define K_I 0.0
#define K_D 0.0

#define CONTROL_LOOP_INTERVAL_MS 10
#define MAX_CLAMPING_EVENTS 100

typedef struct {
    int ticks, prevTicks;
    float dutyCyclePc,  
} params_t;

#endif