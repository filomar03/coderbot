#include "gpio.h"
#include <stdint.h>

typedef enum {
    CHANNEL_A,
    CHANNEL_B
} channel_t;

typedef enum {
    BOUNCE_DETECTED,
    NO_BOUNCE
} signal_bounce_t;

typedef struct {
    gpio_t pin_a;
    gpio_t pin_b;

    uint16_t last_a;
    uint16_t last_b;

    uint16_t level_a;
    uint16_t level_b;

    uint8_t direction;
    int64_t ticks;
} encoder_t;

void encoder_gpio_register_isr(const encoder_t*, gpioAlertFuncEx_t);
void encoder_gpio_cancel_isr(const encoder_t*);