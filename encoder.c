#include "encoder.h"
#include "gpio.h"
#include "motor.h"
#include <pigpio.h>
#include <stdint.h>

void encoder_gpio_register_isr(const encoder_t* encoder, gpioAlertFuncEx_t isr) {  // Interrupt Service Routine
    // Canale A
    gpioSetMode(encoder->pin_a, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_a, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->pin_a, isr, (void*) encoder);
    // Canale B
    gpioSetMode(encoder->pin_b, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_b, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->pin_b, isr, (void*) encoder);
}

void encoder_gpio_cancel_isr(const encoder_t* encoder) {  // Interrupt Service Routine
    gpioSetAlertFuncEx(encoder->pin_a, NULL, NULL);
    gpioSetAlertFuncEx(encoder->pin_b, NULL, NULL);
}

signal_bounce_t debounce(int gpio, encoder_t* encoder, uint32_t now) {
    uint32_t *last = (gpio == encoder->pin_a) ? &(encoder->last_a) : &(encoder->last_b);

    uint32_t elapsed = now - *last;

    if (elapsed < BOUNCE_THRESHOLD) {
        return BOUNCE_DETECTED;
    }

    *last = now;

    return NO_BOUNCE;
}

inline void forward(encoder_t* encoder) {
    encoder->direction = DIRECTION_FORWARD;
    encoder->ticks++;
}

inline void backward(encoder_t* encoder) {
    encoder->direction = DIRECTION_BACKWARD;
    encoder->ticks++;
}

void alert_callback(int gpio, int level, uint32_t now, void *userdata) {
    encoder_t *encoder = (encoder_t *) userdata;
    if (debounce(gpio, encoder, now) == BOUNCE_DETECTED) return;
    if (gpio == encoder->pin_a) {
        if (level == HIGH) {
           encoder->level_b == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->level_b == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->level_a = level;
    } else {
        if (level == HIGH) {
           encoder->level_a == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->level_a == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->level_b = level;
    }
}
