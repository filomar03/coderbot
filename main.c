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
    {
        PIN_ENCODER_LEFT_A,
        BOUNCE_THRESHOLD,

    },
    {
        PIN_ENCODER_LEFT_B,
        BOUNCE_THRESHOLD,

    },
    DIRECTION_FORWARD,
    0
};

encoder_t right_encoder = {
    {
        PIN_ENCODER_RIGHT_A,
        BOUNCE_THRESHOLD,

    },
    {
        PIN_ENCODER_RIGHT_B,
        BOUNCE_THRESHOLD,

    },
    DIRECTION_FORWARD,
    0
};

#define LEFT_MOTOR_K 1.0
#define RIGHT_MOTOR_K 1.07

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
    encoder_gpio_register_isr(&left_encoder, &alert_callback);
    encoder_gpio_register_isr(&right_encoder, &alert_callback);
}

void terminate() {
    motor_gpio_reset(&left_motor);
    motor_gpio_reset(&right_motor);
    encoder_gpio_cancel_isr(&left_encoder);
    encoder_gpio_cancel_isr(&right_encoder);
    gpioTerminate();
}

int main(void) {
    init();
    atexit(terminate);

    uint32_t ms = 5000;
    motor_gpio_move(&left_motor, 128 * LEFT_MOTOR_K);
    motor_gpio_move(&right_motor, 128 * RIGHT_MOTOR_K);
    sleep(ms / 1000);
    printf("registered ticks: %ld - %ld\npwm factor: %.3f - %.3f\ntime: %dms\ndistance: (INSERT MANUALLY)mm\n\n", left_encoder.ticks, right_encoder.ticks, LEFT_MOTOR_K, RIGHT_MOTOR_K, ms);
    exit(EXIT_SUCCESS);
}
