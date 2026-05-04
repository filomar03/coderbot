#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <pigpio.h>
#include <stdint.h>

#include "gpio.h"

#define PWM_FREQUENCY 100
#define MAX_DUTY_CYCLE 255

#define DIRECTION_FORWARD 0
#define DIRECTION_BACKWARD 1

typedef struct {
    gpio_t pin_forward;
    gpio_t pin_backward;
    uint8_t direction;
} motor_t;

void motor_gpio_init(const motor_t*);
void motor_gpio_reset(const motor_t*);
int motor_gpio_move(const motor_t*, uint32_t);

#endif