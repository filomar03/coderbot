#include "motor.h"
#include "gpio.h"

void motor_gpio_init(const motor_t* motor) {
    // Init forward pin
    gpioSetMode(motor->pin_forward, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_forward, MAX_DUTY_CYCLE);
    gpioSetPWMfrequency(motor->pin_forward, PWM_FREQUENCY);

    // Init backward pin
    gpioSetMode(motor->pin_backward, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_backward, MAX_DUTY_CYCLE);
    gpioSetPWMfrequency(motor->pin_backward, PWM_FREQUENCY);
}

void motor_gpio_reset(const motor_t* motor) {
    gpioWrite(motor->pin_forward, 0);
    gpioWrite(motor->pin_backward, 0);
}

gpio_error_t motor_gpio_move(const motor_t* motor, uint32_t duty_cycle) {
    if(duty_cycle > MAX_DUTY_CYCLE) {
        return ERROR_DUTY_CYC_OUT_OF_RANGE;
    }

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
