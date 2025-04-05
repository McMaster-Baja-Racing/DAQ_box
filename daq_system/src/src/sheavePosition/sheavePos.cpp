#include "sheavePos.h"
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"

// Initializations
Adafruit_VL6180X vl = Adafruit_VL6180X();

uint8_t range;
uint8_t status;

void sheavePos() {
  range = vl.readRange();
  //status = vl.readRangeStatus();
  //Serial.print("Range: "); 
  //Serial.print(range); 
  //Serial.print(" mm  ");
  //Serial.println((unsigned long) (range));
 // float range = 5;

  buffPush(SUS_TRAV_FL, (unsigned long)(range));
}