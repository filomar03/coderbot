#!/bin/bash

cc -o cb -DDEBUG -std=c23 -pthread -Wall -pedantic main.c motor.c encoder.c control.c -lpthread -lpigpio
