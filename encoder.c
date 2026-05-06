#include "encoder.h"
#include <pigpio.h>

void encoder_gpio_init(const encoder_t *encoder){
    // Canale A
    gpioSetMode(encoder->pin_a, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_a, PI_PUD_UP);
    // Canale B
    gpioSetMode(encoder->pin_b, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_b, PI_PUD_UP);
}

void encoder_gpio_register_isr(const encoder_t* encoder, int a_or_b, int timeout, void(*isr_raising), void(*isr_falling)) {  // Interrupt Service Routine
    gpio_t channel;
    if (a_or_b == CHANNEL_A) {
        channel = encoder->pin_a;
    }
    else {
        channel = encoder->pin_b;
    }
    gpioSetISRFuncEx(channel, RISING_EDGE, timeout, isr_raising, (void*)encoder);
    gpioSetISRFuncEx(channel, FALLING_EDGE, timeout, isr_falling, (void*)encoder);
}

void encoder_gpio_cancel_isr(const encoder_t* encoder, int a_or_b, int timeout) {  // Interrupt Service Routine
    gpio_t channel;
    if (a_or_b == CHANNEL_A) {
        channel = encoder->pin_a;
    }
    else {
        channel = encoder->pin_b;
    }
    gpioSetISRFuncEx(channel, RISING_EDGE, timeout, NULL, NULL);
    gpioSetISRFuncEx(channel, FALLING_EDGE, timeout, NULL, NULL);

}