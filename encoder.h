#include "gpio.h"
#include <stdint.h>

#define CHANNEL_A 0
#define CHANNEL_B 1

#define BOUNCE_DETECTED 1
#define NO_BOUNCE 0

typedef struct {
    gpio_t pin_a, pin_b;
    gpio_t last_edge;
    uint16_t level_a, level_b;
    uint8_t direction;
    int64_t ticks;
} encoder_t;

void encoder_gpio_init(const encoder_t*);
void encoder_gpio_register_isr(const encoder_t*, int, int, void(*), void(*));
void encoder_gpio_cancel_isr(const encoder_t*, int, int);
