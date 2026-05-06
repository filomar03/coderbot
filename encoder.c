#include "encoder.h"
#include "gpio.h"
#include "motor.h"
#include <assert.h>
#include <pigpio.h>

void encoder_gpio_init(const encoder_t *encoder){
    // Canale A
    gpioSetMode(encoder->pin_a, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_a, PI_PUD_UP);
    // Canale B
    gpioSetMode(encoder->pin_b, PI_INPUT);
    gpioSetPullUpDown(encoder->pin_b, PI_PUD_UP);
}

void encoder_gpio_register_isr(const encoder_t* encoder, int selected_channel, int timeout, void(*isr_raising), void(*isr_falling)) {  // Interrupt Service Routine
    gpio_t channel;
    if (selected_channel == CHANNEL_A) {
        channel = encoder->pin_a;
    }
    else {
        channel = encoder->pin_b;
    }
    gpioSetISRFuncEx(channel, RISING_EDGE, timeout, isr_raising, (void*)encoder);
    gpioSetISRFuncEx(channel, FALLING_EDGE, timeout, isr_falling, (void*)encoder);
}

void encoder_gpio_cancel_isr(const encoder_t* encoder, int selected_channel, int timeout) {  // Interrupt Service Routine
    gpio_t channel;
    if (selected_channel == CHANNEL_A) {
        channel = encoder->pin_a;
    }
    else {
        channel = encoder->pin_b;
    }
    gpioSetISRFuncEx(channel, RISING_EDGE, timeout, NULL, NULL);
    gpioSetISRFuncEx(channel, FALLING_EDGE, timeout, NULL, NULL);
}

int debounce(int gpio, encoder_t* enc) { // controllare questa funzione
    if (gpio == enc->last_edge) return BOUNCE_DETECTED;
    return NO_BOUNCE;
}

void forward(encoder_t* enc) {
    enc->direction = DIRECTION_FORWARD;
    enc->ticks++;
}

void backwards(encoder_t* enc) {
    enc->direction = DIRECTION_BACKWARD;
    enc->ticks++;
}

void isr_rising(int gpio, int level, uint32_t event_ts_us, void* enc_gen) {
    assert(level == RISING_EDGE);
    encoder_t* enc = (encoder_t*) enc_gen;
    if (debounce(gpio, enc) == BOUNCE_DETECTED) return;
    if (gpio == enc->pin_a) {
        if (enc->level_b) {
            forward(enc); // le direzioni poterbbero essere invertite!!
        } else {
            backwards(enc);
        }
        enc->level_a = HIGH;
    } else {
        if (enc->level_a) {
            backwards(enc);
        } else {
            forward(enc);
        }
        enc->level_b = HIGH;
    }
    enc->last_edge = gpio; // sono abbastanza sicuro ci vada anche se non mi pare di averlo letto
}

void isr_falling(int gpio, int level, uint32_t event_ts_us, void* enc_gen) {
    assert(level == FALLING_EDGE);
    encoder_t* enc = (encoder_t*) enc_gen;
    if (debounce(gpio, enc)) return;
    if (gpio == enc->pin_a) {
        if (enc->level_b) {
            forward(enc); // le direzioni poterbbero essere invertite!!
        } else {
            backwards(enc);
        }
        enc->level_a = LOW;
    } else {
        if (enc->level_a) {
            backwards(enc);
        } else {
            forward(enc);
        }
        enc->level_b = LOW;
    }
    enc->last_edge = gpio; // sono abbastanza sicuro ci vada anche se non mi pare di averlo letto
}
