#include "temperature.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"

// Initializations
int temperature;

// Function Definitions
void tempData() {
  temperature = (analogRead(TEMPERATURE_PIN))/*/2*/;
  //  Serial.print("primtemp: ");
  //  Serial.println(temperature);
  buffPush(PRIM_TEMP, (unsigned long)(temperature));
}