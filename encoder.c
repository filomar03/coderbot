#include "encoder.h"
#include <stdatomic.h>

void encoder_gpio_register_isr(encoder_t* encoder, gpioAlertFuncEx_t isr) {
    gpioSetMode(encoder->channel_a.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channel_a.pin, PI_PUD_UP);
    gpioGlitchFilter(encoder->channel_a.pin, GLITCH_FILTER_MICROS);
    // sarebbe meglio usare isr (con nuova build di pigpio dovrebbero essere stati sistemati).
    // dato che il thread degli alert viene chiamato nominalmente con frequenza di 1000,
    // quindi 1ms (se non di piu) di potenziale delay, quindi fino a ~20 tick che slittano alliterazione successiva.
    gpioSetAlertFuncEx(encoder->channel_a.pin, isr, encoder);

    gpioSetMode(encoder->channel_b.pin, PI_INPUT);
    gpioSetPullUpDown(encoder->channel_b.pin, PI_PUD_UP);
    gpioGlitchFilter(encoder->channel_b.pin, GLITCH_FILTER_MICROS);
    gpioSetAlertFuncEx(encoder->channel_b.pin, isr, encoder);
}

void encoder_gpio_cancel_isr(encoder_t* encoder) {
    gpioSetAlertFuncEx(encoder->channel_a.pin, NULL, NULL);
    gpioSetAlertFuncEx(encoder->channel_b.pin, NULL, NULL);
}

// uso relaxed perche gli ordering non influiscono sul delay
// di visibilita negli altri thread, ma definiscono solo dipendenza tra dati
// e qui non abbiamo dipendenza tra dati siccome avviene tutto su un thread solo
void forward(encoder_t* encoder) {
    atomic_fetch_add_explicit(&encoder->ticks, 1, memory_order_relaxed);
}

void backward(encoder_t* encoder) {
    atomic_fetch_add_explicit(&encoder->ticks, -1, memory_order_relaxed);
}

// I callback vengono chiamati sequenzialmente in un thread apposito
void alert_callback(int gpio, int level, uint32_t tick, void *userdata) {
    encoder_t *encoder = userdata;
    if (gpio == encoder->channel_a.pin) {
        if (level == HIGH) {
            encoder->channel_b.level == HIGH ? backward(encoder) : forward(encoder);
        } else {
            encoder->channel_b.level == LOW ? backward(encoder) : forward(encoder);
        }
        encoder->channel_a.level = level;
    } else {
        if (level == HIGH) {
           encoder->channel_a.level == HIGH ? forward(encoder) : backward(encoder);
        } else {
            encoder->channel_a.level == LOW ? forward(encoder) : backward(encoder);
        }
        encoder->channel_b.level = level;
    }
}
