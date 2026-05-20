#ifndef CONTROL_H
#define CONTROL_H

#include "constants.h"

#define K_P 0.5
#define K_I 0.0
#define K_D 0.0

#define CONTROL_LOOP_INTERVAL 0.01
#define MAX_CLAMPING_EVENTS 100

typedef struct {
    int error;
    int error_prev;
    int error_sum;
} params_t;

float update(params_t* params, float dt);
float clamp(float *val);

#endif
