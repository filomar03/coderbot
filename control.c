#include "control.h"

float update(pid_controller_t controller, params_t* params, float dt) {
    float proportional = params->error * controller.k_p;

    params->error_sum += params->error * dt;
    float integral = params->error_sum * controller.k_i;

    float error_delta = params->error - params->error_prev;
    float derivative = error_delta / dt * controller.k_d;
    params->error_prev = params->error;

    return proportional + integral + derivative;
}

bool clamp(float *val) {
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
