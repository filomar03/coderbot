#include "motor.h"
#include "gpio.h"
#include "constants.h"
#include "pigpio.h"
#include <assert.h>

void motor_gpio_init(motor_t* motor) {
    assert(gpioSetMode(motor->pin_forward, PI_OUTPUT) == 0);
    int pwm_range_fw = gpioSetPWMrange(motor->pin_forward, MAX_DUTY_CYCLE);
    assert(pwm_range_fw != PI_BAD_DUTYRANGE && pwm_range_fw != PI_BAD_USER_GPIO);
    int pwm_freq_fw = gpioSetPWMfrequency(motor->pin_forward, PWM_FREQUENCY);
    assert(pwm_freq_fw != PI_BAD_USER_GPIO);

    assert(gpioSetMode(motor->pin_backward, PI_OUTPUT) == 0);
    int pwm_range_bw = gpioSetPWMrange(motor->pin_backward, MAX_DUTY_CYCLE);
    assert(pwm_range_bw != PI_BAD_DUTYRANGE && pwm_range_bw != PI_BAD_USER_GPIO);
    int pwm_freq_bw = gpioSetPWMfrequency(motor->pin_backward, PWM_FREQUENCY);
    assert(pwm_freq_bw != PI_BAD_USER_GPIO);
}

void motor_gpio_reset(motor_t* motor) {
    assert(gpioWrite(motor->pin_forward, 0) == 0);
    assert(gpioWrite(motor->pin_backward, 0) == 0);
}

gpio_error_t motor_gpio_move(motor_t* motor, float power) {
    if (power < 0.0f || power > 1.0f) {
        return ERROR_PWM_POWER_OUT_OF_RANGE;
    }

    int duty_cycle = power * MAX_DUTY_CYCLE;

    if(motor->direction == DIRECTION_FORWARD) {
        assert(gpioWrite(motor->pin_backward, 0) == 0);
        assert(gpioPWM(motor->pin_forward, duty_cycle) == 0);
    }
    else {
        assert(gpioWrite(motor->pin_forward, 0) == 0);
        assert(gpioPWM(motor->pin_backward, duty_cycle) == 0);
    }

    return NO_ERROR;
}
