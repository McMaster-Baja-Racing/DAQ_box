#include "sus.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"

// Initializations
float mRight = -24.282;  // Slope for the right sensor
float bRight = 439.54;  // Intercept for the right sensor
float mLeft = -16.843;   // Slope for the left sensor
float bLeft = 406.62;   // Intercept for the left sensor

int sus1;
int sus2;
int sus3;
int sus4;

// Function Definitions
void susData1() { // Front Left
  int sensorValueRight = analogread(susPin[0]);
  float suspensionDistanceRight = ( sensorValueRight - bRight) / mRight;
  //Serial.print("Suspension Distance Right: ");
  //Serial.println(suspensionDistanceRight);
  buffPush(SUS_TRAV_FR, (unsigned long)(suspensionDistanceRight));
}

void susData2() { // Front Right
  int sensorValueLeft = analogread(susPin[1]);
  float suspensionDistanceLeft = ( sensorValueLeft - bLeft) / mLeft;
  //Serial.print("Suspension Distance Left: ");
  //Serial.println(suspensionDistanceLeft);
  buffPush(SUS_TRAV_FL, (unsigned long)(suspensionDistanceLeft)); 
}

void susData3() { // Rear Right
  sus3 = analogRead(susPin[2]);
  //  Serial.print("susdata3: ");
  //  Serial.println(sus3);
  buffPush(SUS_TRAV_RR, (unsigned long)(sus3));
}

void susData4() { // This is the data for the steering column
  sus4 = analogRead(susPin[3]);
  //  Serial.print("susdata4: ");
  //  Serial.println(sus4);
  //  buffPush(STRAIN6, (unsigned long)(sus4));
}