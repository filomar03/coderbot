#include "gpio.h"
#include <stdint.h>

typedef struct {
    gpio_t pin_a, pin_b;
    gpio_t last_edge;
    uint16_t level_a, level_b;
    uint8_t direction;
    int64_t ticks;
} encoder_t;

void encoder_gpio_init(const encoder_t*);
void encoder_gpio_register_isr(const encoder_t*, int, int);
void encoder_gpio_cancel_isr(const encoder_t*, int, int);
