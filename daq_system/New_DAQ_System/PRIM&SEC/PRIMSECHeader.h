#include "TimeLib.h"

#ifndef PRIMSECHEADER_H
#define PRIMSECHEADER_H

#define HALL_THRESH 1

unsigned long FR_start = micros();
unsigned long FR_end_time = micros();
unsigned long FR_past_time = micros();
bool FR_stopped = false;
float FR_hall_count = 0;
int FR_rpm = 0;

unsigned long FL_start = micros();
unsigned long FL_end_time = micros();
unsigned long FL_past_time = micros();
bool FL_stopped = false;
float FL_hall_count = 0;
int FL_rpm = 0;

unsigned long PRIM_start = micros();
unsigned long PRIM_end_time = micros();
unsigned long PRIM_past_time = micros();
bool PRIM_stopped = false;
float PRIM_hall_count = 0;
int PRIM_rpm = 0;
int Prim_counts_per_rotation = 4;

unsigned long SEC_start = micros();
unsigned long SEC_end_time = micros();
unsigned long SEC_past_time = micros();
bool SEC_stopped = false;
float SEC_hall_count = 0;
int SEC_rpm = 0;
int SEC_counts_per_rotation = 3;

#endif