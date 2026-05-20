#ifndef ENCODER_H
#define ENCODER_H

#include "constants.h"
#include "gpio.h"
#include "motor.h"
#include <pigpio.h>
#include <stdint.h>
#include <stdatomic.h>

typedef enum {
    CHANNEL_A,
    CHANNEL_B
} channel_t;

#define BOUNCE_THRESHOLD 30
#define GLITCH_FILTER_PERCENTAGE 0.1
#define GLITCH_FILTER_MICROS (1 / (MAX_VELOCITY / WHEEL_CIRCUMFERENCE * TICKS_PER_REV) * SECS_TO_MICROS * GLITCH_FILTER_PERCENTAGE)

typedef enum {
    BOUNCE_DETECTED,
    NO_BOUNCE
} signal_bounce_t;

typedef struct {
    gpio_t pin;
    uint32_t last_alert_tick; // questo non serve piu se utilizziamo glitch filter
    pin_state_t level;
} encoder_channel_t;

typedef struct {
    encoder_channel_t channelA;
    encoder_channel_t channelB;
    direction_t direction; // in teoria anche questo dovrebbe essere reso atomico
    // (prababilmente si puo direttamente togliere e ottenere confrontanto tick
    // attuali con tick passsati nel controllore)
    atomic_int_fast64_t ticks;
} encoder_t;

void encoder_gpio_register_isr(encoder_t*, gpioAlertFuncEx_t);
void encoder_gpio_cancel_isr(encoder_t*);
void alert_callback(int gpio, int level, uint32_t now, void *userdata);

#endif
