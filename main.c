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

typedef struct {
    uint32_t micros;
    float dt;
    int ticks;
    float vel;
    float control_action;
} stats_t;

void print_stats(stats_t *s, int n) {
    for (size_t i = 0; i < n; ++i) {
        if (s == NULL) {
            printf("[%03zu] <NULL>\n", i);
            continue;
        }

        printf("[%03zu] micros=%5u  dt=%8.6f  ticks=%6d  vel=%5.4f  clamped=%d  control_action=%5.4f\n",
            i,
            (unsigned)s->micros,
            (double)s->dt,
            s->ticks,
            (double)s->vel,
            (double)s->control_action);
    }
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

    stats_t stats[512];
    int i = 0;

    float target_vel = 1.0f;

    params_t params = {
        .error = 0.0,
        .error_prev = 0.0,
        .error_sum = 0.0
    };

    uint32_t time_prev = gpioTick();

    right_motor.direction = DIRECTION_FORWARD;
    motor_gpio_move(&right_motor, 128 * RIGHT_MOTOR_K);

    while(!stop) {
        usleep(CONTROL_LOOP_INTERVAL * SECS_TO_MICROS);

        uint32_t time = gpioTick();
        float dt = (time - time_prev) / SECS_TO_MICROS;
        time_prev = time;
        
        int ticks = atomic_load_explicit(&left_encoder.ticks, memory_order_acquire);
        float velocity = ticks * M_PER_TICK / dt;

        params.error = target_vel - velocity;

        float control_action = update(&params, dt);
        if (clamp(&control_action)) {
            printf("Clamping event limit exceeded.\n");
            stop = true;
        }

        left_motor.direction = control_action >= 0 ? DIRECTION_FORWARD : DIRECTION_BACKWARD;
        motor_gpio_move(&left_motor, abs(control_action) * MAX_DUTY_CYCLE * LEFT_MOTOR_K);
        
        if (i < 512) {
            stats[i].micros = time;
            stats[i].dt = dt;
            stats[i].ticks = ticks;
            stats[i].vel = velocity;
            stats[i].control_action = control_action;
            i++;
        }
    }

    print_stats((stats_t *) &stats[0], 512);
    exit(EXIT_SUCCESS);
}
