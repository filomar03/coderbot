#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <pigpio.h>
#include "control.h"
#include "constants.h"
#include "gpio.h"
#include "motor.h"
#include "encoder.h"

#ifdef REVERSE
#define MOTOR_DIRECTION DIRECTION_BACKWARD
#endif
#ifndef REVERSE
#define MOTOR_DIRECTION DIRECTION_FORWARD
#endif

// TODO: crearre struttura coderbot globale

encoder_t left_encoder = {
    {
        PIN_ENCODER_LEFT_A,
        LOW
    },
    {
        PIN_ENCODER_LEFT_B,
        LOW
    },
    0
};

encoder_t right_encoder = {
    {
        PIN_ENCODER_RIGHT_A,
        LOW
    },
    {
        PIN_ENCODER_RIGHT_B,
        LOW
    },
    0
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

pid_controller_t controller_vel = {
    .k_p = 40.0f,
    .k_i = 0,
    .k_d = 0,
};

// solo uno dei 2 deve essere usato,
// sono compensazioni per fali andare uguale
#define R2L_PWM_COMPENSATION 1.07425
#define L2R_PWM_COMPENSATION (1 / R2L_PWM_COMPENSATION)

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

#ifdef DEBUG
#define STATS_NUM 10000

typedef struct {
    int64_t exec_time_nanos;
    int64_t ctrl_loop_nanos;
    int ticks;
    double vel;
    double error;
    float ctrl_act;
    float power;
} stat_t;

void print_stats(stat_t *s, int n) {
    printf("%s || %s || %s || %s || %s || %s || %s\n",
        "exec time (us)",
        "ctrl loop (ms)",
        "ticks",
        "vel (m/s)",
        "error   ",
        "ctrl act",
        "power"
    );

    for (size_t i = 0; i < n; ++i) {
        if (s == NULL) {
            printf("no stats available.\n");
            continue;
        }

        printf("%14.5f || %14.5f || %5d || %9.5f || %8.5f || %8.5f || %5.3f\n",
            s[i].exec_time_nanos / 1'000.0,
            s[i].ctrl_loop_nanos / 1'000'000.0,
            s[i].ticks,
            s[i].vel,
            s[i].error,
            s[i].ctrl_act,
            s[i].power
        );
    }
}
#endif

volatile sig_atomic_t stop = false;

void signal_handler(int signum) {
    stop = true;
}

static inline struct timespec compute_time_diff(struct timespec *t0, struct timespec *t1) {
    struct timespec diff = {
        .tv_sec = t1->tv_sec - t0->tv_sec,
        .tv_nsec = t1->tv_nsec - t0->tv_nsec,
    };
    return diff;
}

static inline int64_t timespec_to_ns(struct timespec *ts) {
    return ts->tv_sec * 1'000'000'000 + ts->tv_nsec;
}

int main(void) {
    init();
    atexit(terminate);

    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

    left_motor.direction = MOTOR_DIRECTION;
    motor_gpio_move(&left_motor, 0.5f);
    right_motor.direction = MOTOR_DIRECTION;
    motor_gpio_move(&right_motor, 0.5f * R2L_PWM_COMPENSATION);

    struct timespec ts = {
        .tv_sec = 5,
        .tv_nsec = 0,
    };

    nanosleep(&ts, NULL);

    motor_gpio_reset(&left_motor);
    motor_gpio_reset(&right_motor);

    exit(EXIT_SUCCESS);
}
