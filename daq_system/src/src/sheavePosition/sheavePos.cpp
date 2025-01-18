#include "sheavePos.h"
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"

// Initializations
Adafruit_VL6180X vl;

uint8_t range;
uint8_t status;

void sheavePos() {
  range = vl.readRange();
  status = vl.readRangeStatus();
  //Serial.print("Range: "); 
  //Serial.print(range); 
  //Serial.print(" mm  ");

  buffPush(STRAIN1, (unsigned long)(range));
}