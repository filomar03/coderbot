#include "encoder.h"
#include "gpio.h"
#include "motor.h"
#include <assert.h>
#include <pigpio.h>

// visto che init inizzializza entrmbi i canali, avrebbe senso che anche queste funzioni fanno entrambi!!
void encoder_gpio_register_isr(const encoder_t* encoder, gpioAlertFuncEx_t isr) {  // Interrupt Service Routine
    // Canale A
    gpioSetMode(encoder->pin_a, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_a, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->pin_a, isr, (void*) encoder);
    // Canale B
    gpioSetMode(encoder->pin_b, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_b, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->pin_b, isr, (void*) encoder);
}

// visto che init inizzializza entrmbi i canali, avrebbe senso che anche queste funzioni fanno entrambi!!
void encoder_gpio_cancel_isr(const encoder_t* encoder) {  // Interrupt Service Routine
    gpioSetAlertFuncEx(encoder->pin_a, NULL, NULL);
    gpioSetAlertFuncEx(encoder->pin_b, NULL, NULL);
}

void forward(encoder_t* encoder) {
    encoder->direction = DIRECTION_FORWARD;
    encoder->ticks++;
}

void backwards(encoder_t* encoder) {
    encoder->direction = DIRECTION_BACKWARD;
    encoder->ticks++;
}