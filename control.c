#include "control.h"

void update(params_t* params) {

    params->prevTicks = params->ticks;
    params->ticks = ticks;

}

bool clamp(params_t* params) {
    static unsigned int events = 0;
    if (params->dutyCyclePc > 100.0) {
        params->dutyCyclePc = 100.0;
        return true;
    } else if (params->dutyCyclePc < 0.0) {
        params->dutyCyclePc = 0.0;
        return true;
    }
    return(events > MAX);
}