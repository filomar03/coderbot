#include <stdatomic.h>
#include <assert.h>
#include "encoder.h"

void encoder_gpio_register_isr(encoder_t* encoder, gpioAlertFuncEx_t isr) {
    assert(gpioSetMode(encoder->channel_a.pin, PI_INPUT) == 0);
    assert(gpioSetPullUpDown(encoder->channel_a.pin, PI_PUD_UP) == 0);
    assert(gpioGlitchFilter(encoder->channel_a.pin, GLITCH_FILTER_MICROS) == 0);
    // sarebbe meglio usare isr (con nuova build di pigpio dovrebbero essere stati sistemati).
    // dato che il thread degli alert viene chiamato nominalmente con frequenza di 1000,
    // quindi 1ms (se non di piu) di potenziale delay, quindi fino a ~20 tick che slittano alliterazione successiva.
    assert(gpioSetAlertFuncEx(encoder->channel_a.pin, isr, encoder) == 0);

    assert(gpioSetMode(encoder->channel_b.pin, PI_INPUT) == 0);
    assert(gpioSetPullUpDown(encoder->channel_b.pin, PI_PUD_UP) == 0);
    assert(gpioGlitchFilter(encoder->channel_b.pin, GLITCH_FILTER_MICROS) == 0);
    assert(gpioSetAlertFuncEx(encoder->channel_b.pin, isr, encoder) == 0);
}

void encoder_gpio_cancel_isr(encoder_t* encoder) {
    assert(gpioSetAlertFuncEx(encoder->channel_a.pin, NULL, NULL) == 0);
    assert(gpioSetAlertFuncEx(encoder->channel_b.pin, NULL, NULL) == 0);
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
