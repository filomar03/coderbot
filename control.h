#ifndef _CONTROL_H
#define _CONTROL_H

#include <stdbool.h>

#define CONTROL_LOOP_INTERVAL_MS 10
#define MAX_CLAMPING_EVENTS 30

typedef struct {
    float k_p;
    float k_i;
    float k_d;
} pid_controller_t;

// questi potrebbero essere semplici float,
// ma non ho bene idea con quale scala di valori lavoriamo
typedef struct {
    double error;
    double error_prev;
    double error_sum;
} params_t;

float update(pid_controller_t controller, params_t *params, double dt);
bool clamp(float *val);

#endif
