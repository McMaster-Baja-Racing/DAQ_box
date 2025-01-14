#include "counters.h"

// Initializations
float FR_hall_count = 0;
float FL_hall_count = 0;

// Function Definitions
void incrementHall_FR() {
  //Serial.println("Front Right RPM Pin Hit");
  FR_hall_count += 1;
}
void incrementHall_FL() {
  //Serial.println("Front Left RPM Pin Hit");
  FL_hall_count += 1;
}