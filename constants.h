#include <math.h>

#define TRANSMISSION_RATIO 120
#define ENCODER_TICKS 32
#define TICKS_PER_REV (ENCODER_TICKS * TRANSMISSION_RATIO)
#define WHEEL_CIRCUMFERENCE 0.214
#define METERS_PER_TICK ((float) (WHEEL_CIRCUMFERENCE) / TICKS_PER_REV)
#define RAD_PER_TICK (2 * M_PI / TICKS_PER_REV)
#define MAX_VELOCITY 0.5 // meters/seconds (approssimazione ricavata contanto i giri di ruota in 5 secondi con pwm al massimo)
#define MAX_TICK_FREQ (MAX_VELOCITY / WHEEL_CIRCUMFERENCE * TICKS_PER_REV)
#define MAX_TICK_DURATION 1 / MAX_TICK_FREQ
#define SECS_TO_MICROS powf(10, 6)
