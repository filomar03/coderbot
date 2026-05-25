#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>
#include "constants.h"

#define CONTROL_LOOP_INTERVAL (10 / SECS_TO_MICROS)
#define MAX_CLAMPING_EVENTS 30

typedef struct {
    float k_p;
    float k_i;
    float k_d;
} pid_controller_t;

typedef struct {
    float error;
    float error_prev;
    float error_sum;
} params_t;

float update(pid_controller_t controller, params_t* params, float dt);
bool clamp(float *val);

#endif
