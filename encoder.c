#include "encoder.h"
#include "gpio.h"
#include "motor.h"
#include <pigpio.h>
#include <stdint.h>

void encoder_gpio_register_isr(const encoder_t* encoder, gpioAlertFuncEx_t isr) {
    gpioSetMode(encoder->channelA.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channelA.pin, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->channelA.pin, isr, (void*) encoder);

    gpioSetMode(encoder->channelB.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channelB.pin, PI_PUD_UP);
    gpioSetAlertFuncEx(encoder->channelB.pin, isr, (void*) encoder);
}

void encoder_gpio_cancel_isr(const encoder_t* encoder) {
    gpioSetAlertFuncEx(encoder->channelA.pin, NULL, NULL);
    gpioSetAlertFuncEx(encoder->channelB.pin, NULL, NULL);
}

signal_bounce_t debounce(int gpio, encoder_t* encoder, uint32_t now) {
    uint32_t *last = (gpio == encoder->channelA.pin) ?
        &(encoder->channelA.last_alert_tick) : &(encoder->channelB.last_alert_tick);

    uint32_t elapsed = now - *last;

    if (elapsed < BOUNCE_THRESHOLD) {
        return BOUNCE_DETECTED;
    }

    *last = now;

    return NO_BOUNCE;
}

void forward(encoder_t* encoder) {
    encoder->direction = DIRECTION_FORWARD;
    encoder->ticks++;
}

void backward(encoder_t* encoder) {
    encoder->direction = DIRECTION_BACKWARD;
    encoder->ticks--;
}

// non dovrebbero esserci problemi si sincronizzazione (almeno non sull'incremento).
// siccome i callback vengono chiamati tutti sullo stesso thread in maniera sincrona
void alert_callback(int gpio, int level, uint32_t tick, void *userdata) {
    encoder_t *encoder = (encoder_t *) userdata;
    // qua il prof avrebbe aggiunto un controllo che se non
    // esgue il debounce al primo movimento, sinceramente da quello
    // che ho capito gia il bounce non dovrebbe essere un problema con
    // encoder a effetto hall, ma anzi penso dovremmo utilizzare pigpio glitchFilter
    if (debounce(gpio, encoder, tick) == BOUNCE_DETECTED) return;
    if (gpio == encoder->channelA.pin) {
        if (level == HIGH) {
            // il prof qua usa gpioRead invece che usare i campi nello struct,
            // ma nella documentazione della libreria dice espressamente di non
            // fare cosi (usare gpioRead), perche il callback potrebbe essere
            // chiamato in ritardo (di parecchi ms) rispetto all'effettivo
            // cambio di fronte e quindi bisognerebbe verificare
            // che siano gia avvenuti altri cambi di fronte nel mentre
            encoder->channelB.level == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->channelB.level == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->channelA.level = level;
    } else {
        if (level == HIGH) {
           encoder->channelA.level == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->channelA.level == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->channelB.level = level;
    }
}
