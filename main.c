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
    .k_p = 1.0f,
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
#define STATS_NUM 100000

typedef struct {
    int64_t exec_time_nanos;
    int64_t ctrl_loop_nanos;
    int ticks;
    double vel;
    double error;
    float ctrl_action;
    int pwm;
} stat_t;

void print_stats(stat_t *s, int n) {
    printf("%s || %s || %s || %s || %s || %s || %s\n",
        "exec time (us)",
        "ctrl loop (ms)",
        "ticks",
        "vel (m/s)",
        "error   ",
        "ctrl action",
        "pwm"
    );

    for (size_t i = 0; i < n; ++i) {
        if (s == NULL) {
            printf("no stats available.\n");
            continue;
        }

        printf("%14.5f || %14.5f || %5d || %9.5f || %8.5f || %11.5f || %3d\n",
            s[i].exec_time_nanos / 1'000.0,
            s[i].ctrl_loop_nanos / 1'000'000.0,
            s[i].ticks,
            s[i].vel,
            s[i].error,
            s[i].ctrl_action,
            s[i].pwm
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

    float target_vel = 0.2; // m/s
    params_t params = {};

    int ticks_last = 0;
    struct timespec time_last;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &time_last) != 0) {
        fprintf(stderr, "error while reading clock.\n");
    }

    right_motor.direction = DIRECTION_FORWARD;
    motor_gpio_move(&right_motor, 128);

#ifdef DEBUG
    stat_t *stats = malloc(sizeof(stat_t) * STATS_NUM);
    if (stats == NULL) {
        fprintf(stderr, "failed to allocate memory for stats.\n");
    }
    int i = 0;
#endif
    while(!stop) {
        // measure time
        struct timespec start;
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &start) != 0) {
            fprintf(stderr, "error while reading clock.\n");
        }

        // measure ticks
        // uso relaxed perche gli ordering non influiscono sul delay
        // di visibilita negli altri thread, ma definiscono solo dipendenza tra dati
        int ticks = atomic_load_explicit(&left_encoder.ticks, memory_order_relaxed);
        int delta_ticks = ticks - ticks_last;
        ticks_last = ticks;

        // measure delta time
        struct timespec delta_time = compute_time_diff(&time_last, &start);
        time_last = start;

        // measure velocity
        // potrebbe essere un float, ma non ho idea della scala di valori che potrebbbe asssumere,
        // altrimenti potrei cambiare unita di misura
        double velocity = delta_ticks * METERS_PER_TICK / (timespec_to_ns(&delta_time) / 1'000'000'000.0); // m/s
        params.error = target_vel - velocity;

        // PID
        float control_action = update(controller_vel, &params, timespec_to_ns(&delta_time) /  1'000'000'000.0);
        if (clamp(&control_action)) {
            // printf("Clamping event limit exceeded.\n");
            // stop = true;
        }

        // power motor
        left_motor.direction = control_action >= 0 ? DIRECTION_FORWARD : DIRECTION_BACKWARD;
        int pwm = fabsf(control_action) * MAX_DUTY_CYCLE;
        if (motor_gpio_move(&left_motor, pwm) != NO_ERROR) {
            fprintf(stderr, "error moving motor.\n");
        }

        // compute sleep time
        struct timespec end;
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &end) != 0) {
            fprintf(stderr, "error while reading clock.\n");
        }
        struct timespec exec_time = compute_time_diff(&start, &end);
        struct timespec nsleep_time = {
            .tv_sec = 0,
            .tv_nsec = CONTROL_LOOP_INTERVAL_MS * 1'000'000 - timespec_to_ns(&exec_time),
        };

#ifdef DEBUG
        // collect stats
        if (i < STATS_NUM) {
            stats[i].exec_time_nanos = timespec_to_ns(&exec_time);
            stats[i].ctrl_loop_nanos = timespec_to_ns(&delta_time);
            stats[i].ticks = delta_ticks;
            stats[i].vel = velocity;
            stats[i].error = params.error;
            stats[i].ctrl_action = control_action;
            stats[i].pwm = pwm;
        }
        i++;
#endif

        // sleep
        if (nanosleep(&nsleep_time, NULL) != 0) {
            fprintf(stderr, "nanosleep interrupted.\n");
        }
    }

#ifdef DEBUG
    if (stats != NULL) {
        printf("PID controller did %d iterations.\n", i);
        printf("Controller config:\n\tkP = %f\n\tkI = %f\n\tkD = %f\n",
            controller_vel.k_p,
            controller_vel.k_i,
            controller_vel.k_d
        );
        print_stats(&stats[0], i < STATS_NUM ? i : STATS_NUM);
        free(stats);
    }
#endif
    exit(EXIT_SUCCESS);
}
