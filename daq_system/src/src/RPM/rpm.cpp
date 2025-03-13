#include "rpm.h"
#include "../datastruct/dataTypeNames.h"
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"
float PRIM_hall_count = 0;
float avgPrimPulse = 0;
float REAR_SPEED_hall_count = 0;
float avgRearSpeedPulse = 0;

bool EN_RPM = true; 
bool REAR_SPEED_stopped = false;
bool PRIM_stopped = false;

unsigned long REAR_SPEED_start = micros();
unsigned long REAR_SPEED_end_time = micros();
unsigned long REAR_SPEED_past_time = micros();

unsigned long PRIM_start = micros();
unsigned long PRIM_end_time = micros();
unsigned long PRIM_past_time = micros();

int REAR_SPEED_int = 0;
int PRIM_rpm = 0;

void rpmCalc() {
  if (EN_RPM) {
    if (REAR_SPEED_hall_count > HALL_THRESH) {
      REAR_SPEED_end_time = micros();
      REAR_SPEED_past_time = (REAR_SPEED_end_time - REAR_SPEED_start);
      
      if (REAR_SPEED_stopped) {
        REAR_SPEED_stopped = false;
      }

      avgRearSpeedPulse = avgRearSpeedPulse * 0.9 + (float)REAR_SPEED_past_time * 0.1;

      REAR_SPEED_int = (REAR_SPEED_hall_count / ((avgRearSpeedPulse / 1000000.0) / 60)) / Rear_speed_counts_per_rotation;
      
      buffPush(REAR_SPEED, (float)REAR_SPEED_int);
      
      REAR_SPEED_hall_count = 0;
      REAR_SPEED_start = micros();
    }

    if (!REAR_SPEED_stopped && (micros() - REAR_SPEED_start >= 1000000)) {
      buffPush(REAR_SPEED, float(0));
      REAR_SPEED_stopped = true;
    }

    if (PRIM_hall_count > HALL_THRESH) {
      PRIM_end_time = micros();
      PRIM_past_time = (PRIM_end_time - PRIM_start);

      if (PRIM_stopped) {
        PRIM_stopped = false;
      }

      avgPrimPulse = avgPrimPulse * 0.9 + (float)PRIM_past_time * 0.1;

      PRIM_rpm = (PRIM_hall_count / ((avgPrimPulse / 1000000.0) / 60)) / Prim_counts_per_rotation;
      
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

void incrementHall_REAR_SPEED() {
  //Serial.println("Rear Wheel Speed Pin Hit");
  REAR_SPEED_hall_count += 1;
}

void incrementHall_PRIM() {
  //Serial.println("PRIM RPM Pin Hit");
  PRIM_hall_count += 1;
}