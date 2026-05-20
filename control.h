#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>
#include "constants.h"

#define K_P 5
#define K_I 0.0
#define K_D 0.0

#define CONTROL_LOOP_INTERVAL 0.01
#define MAX_CLAMPING_EVENTS 10

typedef struct {
    float error;
    float error_prev;
    float error_sum;
} params_t;

float update(params_t* params, float dt);
bool clamp(float *val);

#endif
