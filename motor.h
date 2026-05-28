#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <pigpio.h>
#include <stdint.h>
#include "gpio.h"

#define PWM_FREQUENCY 100
#define MAX_DUTY_CYCLE 1024

typedef enum {
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD
} direction_t;

typedef struct {
    gpio_t pin_forward;
    gpio_t pin_backward;
    uint8_t direction;
} motor_t;

void motor_gpio_init(motor_t*);
void motor_gpio_reset(motor_t*);
gpio_error_t motor_gpio_move(motor_t*, float);

#endif
