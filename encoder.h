#ifndef _ENCODER_H
#define _ENCODER_H

#include <pigpio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <math.h>
#include "constants.h"
#include "gpio.h"

// #define BOUNCE_THRESHOLD 30
#define GLITCH_FILTER_PERCENTAGE 0.5
#define GLITCH_FILTER_MICROS floor(MAX_TICK_DURATION * 1000000 * GLITCH_FILTER_PERCENTAGE)

// typedef enum {
//     BOUNCE_DETECTED,
//     NO_BOUNCE
// } signal_bounce_t;

typedef struct {
    gpio_t pin;
    pin_state_t level;
} encoder_channel_t;

typedef struct {
    encoder_channel_t channel_a;
    encoder_channel_t channel_b;
    atomic_int_fast64_t ticks;
} encoder_t;

void encoder_gpio_register_isr(encoder_t*, gpioAlertFuncEx_t);
void encoder_gpio_cancel_isr(encoder_t*);
void alert_callback(int gpio, int level, uint32_t now, void *userdata);

#endif
