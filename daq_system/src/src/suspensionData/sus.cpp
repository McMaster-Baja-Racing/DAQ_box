#include "sus.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"

// Initializations
float mRight = 0.8812;  // Slope for the right sensor
float bRight = -2.66;  // Intercept for the right sensor
float mLeft = 1.091;   // Slope for the left sensor
float bLeft = -4.13;   // Intercept for the left sensor

int sus1;
int sus2;
int sus3;
int sus4;

// Function Definitions
void susData1() { // back Left
  int sensorValueLeft = analogRead(susPin[0]);
  //Serial.print("Sensor Value Left: ");
  //Serial.println(sensorValueLeft);
  //float suspensionDistanceLeft = ( sensorValueLeft - bLeft) / mLeft;
  buffPush(SUS_TRAV_RL, (unsigned long)(sensorValueLeft));
}

void susData2() { // back right
  int sensorValueRight = analogRead(susPin[1]);
  //Serial.print("Sensor Value right: ");
  //Serial.println(sensorValueRight);
  //float suspensionDistanceRight = ( sensorValueRight - bRight) / mRight;
  buffPush(SUS_TRAV_RR, (unsigned long)(sensorValueRight)); 
}

void susData3() { // Rear Right
  sus3 = analogRead(susPin[2]);
  //  Serial.print("susdata3: ");
  //  Serial.println(sus3);
  //buffPush(SUS_TRAV_RR, (unsigned long)(sus3));
}

void susData4() { // This is the data for the steering column
  sus4 = analogRead(susPin[3]);
  //  Serial.print("susdata4: ");
  //  Serial.println(sus4);
  //  buffPush(STRAIN6, (unsigned long)(sus4));
}