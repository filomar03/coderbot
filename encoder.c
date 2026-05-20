#include "encoder.h"

void encoder_gpio_register_isr(const encoder_t* encoder, gpioAlertFuncEx_t isr) {
    gpioSetMode(encoder->channelA.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channelA.pin, PI_PUD_UP);
    gpioGlitchFilter(encoder->channelA.pin, GLITCH_FILTER_MICROS);
    gpioSetAlertFuncEx(encoder->channelA.pin, isr, (void*) encoder);

    gpioSetMode(encoder->channelB.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channelB.pin, PI_PUD_UP);
    gpioGlitchFilter(encoder->channelB.pin, GLITCH_FILTER_MICROS);
    gpioSetAlertFuncEx(encoder->channelB.pin, isr, (void*) encoder);
}

void encoder_gpio_cancel_isr(const encoder_t* encoder) {
    gpioSetAlertFuncEx(encoder->channelA.pin, NULL, NULL);
    gpioSetAlertFuncEx(encoder->channelB.pin, NULL, NULL);
}

signal_bounce_t debounce(int gpio, encoder_t* encoder, uint32_t now) {
    uint32_t *last = (gpio == encoder->channelA.pin) ?
        &(encoder->channelA.last_alert_tick) : &(encoder->channelB.last_alert_tick);

    uint32_t elapsed = now - *last;

    if (elapsed < BOUNCE_THRESHOLD) {
        return BOUNCE_DETECTED;
    }

    *last = now;

    return NO_BOUNCE;
}

// TODO: spostare in file fsm.c
void forward(encoder_t* encoder) { // rendere inline
    encoder->direction = DIRECTION_FORWARD;
    atomic_fetch_add_explicit(&encoder->ticks, 1, memory_order_release);
}

void backward(encoder_t* encoder) { // rendere inline
    encoder->direction = DIRECTION_BACKWARD;
    atomic_fetch_add_explicit(&encoder->ticks, -1, memory_order_release);
}

// I callback vengono chiamati sequenzialmente in un thread separato
void alert_callback(int gpio, int level, uint32_t tick, void *userdata) {
    encoder_t *encoder = (encoder_t *) userdata;
    // sostituito con glitch filter
    // if (debounce(gpio, encoder, tick) == BOUNCE_DETECTED) return;
    if (gpio == encoder->channelA.pin) {
        if (level == HIGH) {
            encoder->channelB.level == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->channelB.level == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->channelA.level = level;
    } else {
        if (level == HIGH) {
           encoder->channelA.level == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->channelA.level == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->channelB.level = level;
    }
}
