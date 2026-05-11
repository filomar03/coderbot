#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <pigpio.h>
#include "gpio.h"
#include "motor.h"
#include "encoder.h"

encoder_t left_encoder = {
    PIN_ENCODER_LEFT_A,
    PIN_ENCODER_LEFT_B,
    LAST_EDGE_INIT,
    LEVEL_INIT,
    LEVEL_INIT,
    DIRECTION_FORWARD,
    TICKS_INIT
};

encoder_t left_encoder = {
    PIN_ENCODER_LEFT_A,
    PIN_ENCODER_LEFT_B,
    LAST_EDGE_INIT,
    LEVEL_INIT,
    LEVEL_INIT,
    DIRECTION_FORWARD,
    TICKS_INIT
};

encoder_t right_encoder = {
    PIN_ENCODER_RIGHT_A,
    PIN_ENCODER_RIGHT_B,
    LAST_EDGE_INIT,
    LEVEL_INIT,
    LEVEL_INIT,
    DIRECTION_FORWARD,
    TICKS_INIT
};

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
    encoder_gpio_init(&left_encoder);
    encoder_gpio_init(&right_encoder);

    // registrare le edges
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
