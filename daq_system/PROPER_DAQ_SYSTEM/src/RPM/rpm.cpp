#include "rpm.h"
#include "../datastruct/dataTypeNames.h"
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"
float PRIM_hall_count = 0;
float SEC_hall_count = 0;

bool EN_RPM = true; 
bool SEC_stopped = false;
bool PRIM_stopped = false;

unsigned long SEC_start = micros();
unsigned long SEC_end_time = micros();
unsigned long SEC_past_time = micros();

unsigned long PRIM_start = micros();
unsigned long PRIM_end_time = micros();
unsigned long PRIM_past_time = micros();

int SEC_rpm = 0;
int PRIM_rpm = 0;

void rpmCalc() {
    if (EN_RPM) {
    if (SEC_hall_count > HALL_THRESH) {
      SEC_end_time = micros();
      SEC_past_time = (SEC_end_time - SEC_start);
      if (SEC_stopped) {
        SEC_stopped = false;
      }
      SEC_rpm = (SEC_hall_count / ((SEC_past_time / 1000000.0) / 60)) / SEC_counts_per_rotation;
      buffPush(RPM_SEC, (float)SEC_rpm);
      SEC_hall_count = 0;
      SEC_start = micros();
    }
    if (!SEC_stopped && (micros() - SEC_start >= 1000000)) {
      buffPush(RPM_SEC, float(0));
      SEC_stopped = true;
    }

    if (PRIM_hall_count > HALL_THRESH) {
      PRIM_end_time = micros();
      PRIM_past_time = (PRIM_end_time - PRIM_start);
      if (PRIM_stopped) {
        PRIM_stopped = false;
      }
      PRIM_rpm = (PRIM_hall_count / ((PRIM_past_time / 1000000.0) / 60)) / Prim_counts_per_rotation;
      buffPush(RPM_PRIM, (float)PRIM_rpm);
      PRIM_hall_count = 0;
      PRIM_start = micros();
    }
    if (!PRIM_stopped && (micros() - PRIM_start >= 1000000)) {
      buffPush(RPM_PRIM, float(0));
      PRIM_stopped = true;
    }
  }
}

void incrementHall_SEC() {
  //Serial.println("Secondary RPM Pin Hit");
  SEC_hall_count += 1;
}

void incrementHall_PRIM() {
  //Serial.println("PRIM RPM Pin Hit");
  PRIM_hall_count += 1;
}