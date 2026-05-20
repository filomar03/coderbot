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

        printf("us=%u\tdt=%8.6f\te=%f\tticks=%d\tv=%f\tca=%f\tpwm=%d\n",
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
    srand(time(NULL));

    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

    size_t stats_num = 10000;
    stat_t *stats = (stat_t *) malloc(sizeof(stat_t) * stats_num);
    if (stats == NULL) {
        fprintf(stderr, "failed to allocate memory.\n");
        stop = true;
    }

    float target_vel = 2.0;
    params_t params = {
        .error = 0.0,
        .error_prev = 0.0,
        .error_sum = 0.0
    };

    uint32_t time_prev = gpioTick();

    right_motor.direction = DIRECTION_FORWARD;
    motor_gpio_move(&right_motor, 128 * RIGHT_MOTOR_K);

    int i = 0;
    while(!stop) {
        if (usleep(CONTROL_LOOP_INTERVAL * SECS_TO_MICROS) != 0) {
            fprintf(stderr, "usleep interrotto.\n");
            stop = true;
            continue;
        }

        uint32_t time = gpioTick();
        float dt = (time - time_prev) / SECS_TO_MICROS;
        time_prev = time;

        // measure and compute data
        int ticks = atomic_load_explicit(&left_encoder.ticks, memory_order_acquire);
        float velocity = ticks * M_PER_TICK / dt;
        params.error = target_vel - velocity;

        // PID
        float control_action = update(&params, dt);
        if (clamp(&control_action)) {
            // printf("Clamping event limit exceeded.\n");
            // stop = true;
        }

        // power motor
        left_motor.direction = control_action >= 0 ? DIRECTION_FORWARD : DIRECTION_BACKWARD;
        int pwm = fabsf(control_action) * MAX_DUTY_CYCLE * LEFT_MOTOR_K;
        motor_gpio_move(&left_motor, pwm);

        // collect stats
        if (i < stats_num) {
            stats[i].micros = time;
            stats[i].dt = dt;
            stats[i].ticks = ticks;
            stats[i].vel = velocity;
            stats[i].control_action = control_action;
            stats[i].pwm = pwm;
        }
        i++;
    }

    print_stats(&stats[0], i < stats_num ? i : stats_num);
    printf("controller did %d iterations.", i);
    free((void *) stats);
    exit(EXIT_SUCCESS);
}
