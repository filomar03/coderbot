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
        LOW
    },
    {
        PIN_ENCODER_LEFT_B,
        LOW
    },
    DIRECTION_FORWARD,
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
    DIRECTION_FORWARD,
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

pid_controller_t controller = {
    .k_p = 10.0f,
    .k_i = 0,
    .k_d = 0,
};

#define RIGHT_LEFT_MOTOR_RATIO 1.07425

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

        printf("us=%u\tdt=%8.6f\tticks=%d\tv=%.3f\te=%f\tca=%f\tpwm=%d\n",
            (unsigned)s[i].micros,
            (double)s[i].dt * SECS_TO_MICROS,
            s[i].ticks,
            (double)s[i].vel,
            (double)s[i].error,
            (double)s[i].control_action,
            s[i].pwm
        );
    }
}
#endif

volatile sig_atomic_t stop = false;

void signal_handler(int signum) {
    stop = true;
}

int main(void) {
    init();
    atexit(terminate);

    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

#ifdef DEBUG
    size_t stats_num = 1000000;
    stat_t *stats = malloc(sizeof(stat_t) * stats_num);
    if (stats == NULL) {
        fprintf(stderr, "failed to allocate memory for stats.\n");
    }
#endif

    float target_vel = 0.08f;
    params_t params = {
        .error = 0.0,
        .error_prev = 0.0,
        .error_sum = 0.0
    };

    uint32_t time_prev = gpioTick();
    int ticks_prev = 0;

    right_motor.direction = DIRECTION_FORWARD;
    motor_gpio_move(&right_motor, 128);

    int i = 0;
    while(!stop) {
        // TODO: usare timer ad alta definizione
        if (usleep(1000) != 0) {
            stop = true;
            continue;
        }

        // measure time
        uint32_t time = gpioTick();
        float d_t = (time - time_prev) / SECS_TO_MICROS;
        time_prev = time;

        // measure ticks
        int ticks = atomic_load_explicit(&left_encoder.ticks, memory_order_acquire);
        int d_ticks = ticks - ticks_prev;
        ticks_prev = ticks;

        // measure velocity
        float velocity = d_ticks * METERS_PER_TICK / d_t;
        params.error = target_vel - velocity;

        // PID
        float control_action = update(controller, &params, d_t);
        if (clamp(&control_action)) {
            // printf("Clamping event limit exceeded.\n");
            // stop = true;
        }

        // power motor
        left_motor.direction = control_action >= 0 ? DIRECTION_FORWARD : DIRECTION_BACKWARD;
        int pwm = fabsf(control_action) * MAX_DUTY_CYCLE;
        if (motor_gpio_move(&left_motor, pwm) != NO_ERROR) {
            stop = true;
            fprintf(stderr, "error moving motor.");
        }

#ifdef DEBUG
        // collect stats
        if (i < stats_num) {
            stats[i].micros = time;
            stats[i].dt = d_t;
            stats[i].ticks = d_ticks;
            stats[i].vel = velocity;
            stats[i].error = params.error;
            stats[i].control_action = control_action;
            stats[i].pwm = pwm;
        }
        i++;
#endif
    }

#ifdef DEBUG
    if (stats != NULL) {
        printf("controller did %d iterations.\n", i);
        print_stats(&stats[0], i < stats_num ? i : stats_num);
        free(stats);
    }
#endif
    exit(EXIT_SUCCESS);
}
