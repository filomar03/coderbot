#ifndef _CONST_H
#define _CONST_H

#define TRANSMISSION_RATIO 120
#define ENCODER_TICKS 32
#define TICKS_PER_REV (ENCODER_TICKS * TRANSMISSION_RATIO)
#define WHEEL_CIRCUMFERENCE 0.214f
#define METERS_PER_TICK (WHEEL_CIRCUMFERENCE / TICKS_PER_REV)
#define PI 3.14159265358979323846f
#define RAD_PER_TICK (2 * PI / TICKS_PER_REV)
#define MAX_VELOCITY 0.5f // m/s. approssimazione ricavata contanto i giri di ruota in 5 secondi con pwm al massimo
#define MAX_TICK_FREQ (MAX_VELOCITY / WHEEL_CIRCUMFERENCE * TICKS_PER_REV) // 1/s
#define MAX_TICK_DURATION (1.0f / MAX_TICK_FREQ) // s

#endif
