#ifndef _GPIO_H
#define _GPIO_H

typedef enum {
    NO_ERROR,
    ERROR_PWM_POWER_OUT_OF_RANGE = 10,
} gpio_error_t;

typedef enum {
    LOW = 0,
    HIGH = 1
} pin_state_t;

typedef enum gpiov5 {
    PIN_LEFT_FORWARD = 17,
    PIN_LEFT_BACKWARD = 18,
    PIN_RIGHT_FORWARD = 23,
    PIN_RIGHT_BACKWARD = 22,
    PIN_ENCODER_LEFT_A = 14,
    PIN_ENCODER_LEFT_B = 15,
    PIN_ENCODER_RIGHT_A = 24,
    PIN_ENCODER_RIGHT_B = 25,
} gpio_t;

#endif
