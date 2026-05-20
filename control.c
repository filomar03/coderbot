#include "control.h"

float update(params_t* params, float dt) {
    float proportional = params->error * K_P;

    params->error_sum += params->error * dt;
    float integral = params->error_sum * K_I;

    float error_delta = params->error - params->error_prev;
    float derivative = error_delta / dt * K_D;

    params->error_prev = params->error;

    return proportional + integral + derivative;
}

float clamp(float *val) {
    static unsigned int events = 0;

    if (*val > 1.0f) {
        *val = 1.0f;
        events++;
    } else if (*val < -1.0f) {
        *val = -1.0f;
        events++;
    } else {
        events = 0;
    }

    return events > MAX_CLAMPING_EVENTS;
}
