#include "counters.h"

// Initializations
float FR_hall_count = 0;
float FL_hall_count = 0;
float PRIM_hall_count = 0;
float SEC_hall_count = 0;

// Function Definitions
void incrementHall_FR() {
  //Serial.println("Front Right RPM Pin Hit");
  FR_hall_count += 1;
}
void incrementHall_FL() {
  //Serial.println("Front Left RPM Pin Hit");
  FL_hall_count += 1;
}
void incrementHall_SEC() {
  //Serial.println("Secondary RPM Pin Hit");
  SEC_hall_count += 1;
}

void incrementHall_PRIM() {
  //Serial.println("PRIM RPM Pin Hit");
  PRIM_hall_count += 1;
}