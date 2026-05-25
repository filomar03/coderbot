#include <pigpio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../motor.h"

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
    terminate();
    exit(EXIT_SUCCESS);
}
