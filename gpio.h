#ifndef GPIO_H
#define GPIO_H

#define E_DUTY_CYC_OUT_OF_RANGE 10
#define E_INVALID_MODE 20

#define HIGH 1
#define LOW 0

enum gpiov5 {
    PIN_LEFT_FORWARD = 17,
    PIN_LEFT_BACKWARD = 18,
    PIN_RIGHT_FORWARD = 23,
    PIN_RIGHT_BACKWARD = 22,
    PIN_ENCODER_LEFT_A = 14,
    PIN_ENCODER_LEFT_B = 15,
    PIN_ENCODER_RIGHT_A = 24,
    PIN_ENCODER_RIGHT_B = 25,
};

typedef enum gpiov5 gpio_t;

#endif
