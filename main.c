#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <pigpio.h>
#include "gpio.h"
#include "motor.h"

motor_t left_motor = {
    PIN_LEFT_FORWARD,
    PIN_LEFT_BACKWARD,
    DIRECTION_FORWARD
};
motor_t right_motor = {
    PIN_RIGHT_FORWARD,
    PIN_RIGHT_BACKWARD,
    DIRECTION_FORWARD
};

void init() {
    gpioInitialise();
    motor_gpio_init(&left_motor);
    motor_gpio_init(&right_motor);
}

void terminate() {
    motor_gpio_reset(&left_motor);
    motor_gpio_reset(&right_motor);
    gpioTerminate();
}

int main(void) {
    init();
    atexit(terminate);

    uint32_t ms = 5000;
    printf("forward for %dms\n", ms);
    motor_gpio_move(&left_motor, 128);
    motor_gpio_move(&right_motor, 128);
    sleep(ms);
    exit(EXIT_SUCCESS);
}
