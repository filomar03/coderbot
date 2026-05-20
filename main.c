#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <pigpio.h>
#include "control.h"
#include "gpio.h"
#include "motor.h"
#include "encoder.h"

encoder_t left_encoder = {
    {
        PIN_ENCODER_LEFT_A,
        -1,
        LOW
    },
    {
        PIN_ENCODER_LEFT_B,
        -1,
        LOW
    },
    DIRECTION_FORWARD,
    0
};

encoder_t right_encoder = {
    {
        PIN_ENCODER_RIGHT_A,
        -1,
        LOW
    },
    {
        PIN_ENCODER_RIGHT_B,
        -1,
        LOW
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
    encoder_gpio_cancel_isr(&right_encoder);
    encoder_gpio_cancel_isr(&left_encoder);
    motor_gpio_reset(&right_motor);
    motor_gpio_reset(&left_motor);
    gpioTerminate();
}

volatile sig_atomic_t stop = 0;

void signal_handler(int signum) {
    stop = 1;
}

int main(void) {
    init();
    atexit(terminate);

    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

    uint32_t time_prev = gpioTick();

    // setto pwm costante motore destra

    while(!stop) {
        uint32_t time = gpioTick();
        float dt = (float) (time - time_prev) / SECS_TO_MICROS;
        time_prev = time;

        int left_ticks = atomic_load_explicit(&left_encoder.ticks, memory_order_acquire);
        // calcolo target e errore
        // update
        // clamp e sicura
        // imposto pwm
        // usleep
    }

    exit(EXIT_SUCCESS);
}
