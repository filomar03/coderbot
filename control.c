#include "control.h"

float update(pid_controller_t controller, params_t* params, double dt) {
    double proportional = params->error * controller.k_p;

    params->error_sum += params->error * dt;
    double integral = params->error_sum * controller.k_i;

    double error_delta = params->error - params->error_prev;
    double derivative = error_delta / dt * controller.k_d;
    params->error_prev = params->error;

    return proportional + integral + derivative;
}

// oltre a fare il clamp cerca di capire quando il robot e' incastrato
bool clamp(float *val) {
    static unsigned int events = 0;

    if (*val > 1.0f) {
        *val = 1.0f;
        events++;
    } else if (*val < -1.0f) {
        *val = -1.0f;
        events++;
    } else {
        events--;
    }

    return events > MAX_CLAMPING_EVENTS;
}
