#!/bin/bash

cc -o cb -pthread -Wall -pedantic main.c motor.c encoder.c control.c -lm -lrt -lpthread -lpigpio
