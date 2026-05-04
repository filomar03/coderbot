#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

// #include "gpio.h"
// #include "motor.h"

void init() {
    // init gpio lib
    // init motor struct
}

void terminate() {
    // reset motor gpio
    // terminate gpio lib
}

int main(void) {
    init();
    atexit(terminate);

    u_int32_t ms = 5000;
    printf("forward for %dms\n", ms);
    // move motors
    sleep(ms);
    exit(EXIT_SUCCESS);
}
