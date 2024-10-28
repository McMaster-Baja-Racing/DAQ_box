#include "temp.h"
#include "datastruct.h"
#include "sd.h"

int temperature;

void tempData() {
  temperature = (analogRead(TEMPERATURE_PIN))/*/2*/;
  //  Serial.print("primtemp: ");
  //  Serial.println(temperature);
  buffPush(PRIM_TEMP, (unsigned long)(temperature));
}