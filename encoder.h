#ifndef ENCODER_H
#define ENCODER_H

#include "gpio.h"
#include "motor.h"
#include <pigpio.h>
#include <stdint.h>

typedef enum {
    CHANNEL_A,
    CHANNEL_B
} channel_t;

#define BOUNCE_THRESHOLD 30

typedef enum {
    BOUNCE_DETECTED,
    NO_BOUNCE
} signal_bounce_t;

typedef struct {
    gpio_t pin;
    uint32_t last_alert_tick;
    pin_state_t level;
} encoder_channel_t;

typedef struct {
    encoder_channel_t channelA;
    encoder_channel_t channelB;
    direction_t direction;
    int64_t ticks; // TODO: rendere  variabile atomica
} encoder_t;

void encoder_gpio_register_isr(const encoder_t*, gpioAlertFuncEx_t);
void encoder_gpio_cancel_isr(const encoder_t*);
void alert_callback(int gpio, int level, uint32_t now, void *userdata);

#endif
