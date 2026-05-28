#include "motor.h"
#include "gpio.h"
#include "constants.h"

void motor_gpio_init(motor_t* motor) {
    gpioSetMode(motor->pin_forward, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_forward, MAX_DUTY_CYCLE);
    gpioSetPWMfrequency(motor->pin_forward, PWM_FREQUENCY);

    gpioSetMode(motor->pin_backward, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_backward, MAX_DUTY_CYCLE);
    gpioSetPWMfrequency(motor->pin_backward, PWM_FREQUENCY);
}

void motor_gpio_reset(motor_t* motor) {
    gpioWrite(motor->pin_forward, 0);
    gpioWrite(motor->pin_backward, 0);
}

gpio_error_t motor_gpio_move(motor_t* motor, float power) {
    if (power < 0.0f || power > 1.0f) {
        return ERROR_PWM_POWER_OUT_OF_RANGE;
    }

    int duty_cycle = power * MAX_DUTY_CYCLE;

    if(motor->direction == DIRECTION_FORWARD) {
        gpioWrite(motor->pin_backward, 0);
        gpioPWM(motor->pin_forward, duty_cycle);
    }
    else {
        gpioWrite(motor->pin_forward, 0);
        gpioPWM(motor->pin_backward, duty_cycle);
    }

    return NO_ERROR;
}
