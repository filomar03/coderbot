#include <math.h>
#include <pthread.h>
#include <stddef.h>
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
#define RIGHT_MOTOR_K 1.11

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

typedef struct {
    uint32_t micros;
    float error;
    float dt;
    int ticks;
    float vel;
    float control_action;
    int pwm;
} stat_t;

void print_stats(stat_t *s, int n) {
    for (size_t i = 0; i < n; ++i) {
        if (s == NULL) {
            printf("no stats available.\n");
            continue;
        }

        printf("us=%u dt=%8.6f e=%f ticks=%d v=%f ca=%f pwm=%d\n",
            (unsigned)s[i].micros,
            (double)s[i].dt,
            (double)s[i].error,
            s[i].ticks,
            (double)s[i].vel,
            (double)s[i].control_action,
            s[i].pwm
        );
    }
}

volatile sig_atomic_t stop = false;

void signal_handler(int signum) {
    stop = true;
}

int main(void) {
    init();
    atexit(terminate);

    int s = 15;
    int times = 10;
    int pwm = 128;

    float min_k = 0.6f;
    float max_k = 1.4f;

    float target_diff = 0.1f / 100;

    bool running = true;

    while(running) {
        float k = (min_k + max_k) / 2;

        int ticks_left = 0;
        int ticks_right = 0;

        for (int i = 0; i < times; i++) {
            left_motor.direction = DIRECTION_FORWARD;
            motor_gpio_move(&left_motor, pwm);
            right_motor.direction = DIRECTION_FORWARD;
            motor_gpio_move(&right_motor, roundf(pwm * k));

            sleep(s);

            ticks_left += atomic_load_explicit(&left_encoder.ticks, memory_order_acquire);
            ticks_right += atomic_load_explicit(&right_encoder.ticks, memory_order_acquire);

            motor_gpio_reset(&left_motor);
            motor_gpio_reset(&right_motor);

            left_encoder.ticks = 0;
            right_encoder.ticks = 0;

            sleep(1);
        }

        int max = ticks_left > ticks_right ? ticks_left : ticks_right;
        int min = ticks_left < ticks_right ? ticks_left : ticks_right;
        float diff = (float) (max - min) / 2 / ((float) (max + min) / 2);

        printf("range: [%.3f, %.3f]\n", min_k, max_k);
        printf("k: %f\n", k);
        printf("ticks: %d(l) - %d(r)\n", ticks_left, ticks_right);
        printf("diff: %.2f%%\n", diff * 100);
        printf("--------------------\n");

        if (diff - target_diff <= 0 || (max_k - min_k) * 1000 < 2) {
            running = false;
            continue;
        }

        if (ticks_left > ticks_right) {
            min_k = k;
        } else {
            max_k = k;
        }
    }

    exit(EXIT_SUCCESS);
}
