#!/bin/bash

gcc -o cb -DDEBUG -std=c23 -pthread -Wall -Wno-comment main.c motor.c encoder.c control.c -lpthread -lpigpio
