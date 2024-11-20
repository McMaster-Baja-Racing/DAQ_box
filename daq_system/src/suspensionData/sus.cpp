#include "sus.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"

// Initializations
int sus1;
int sus2;
int sus3;
int sus4;

// Function Definitions
void susData1() { // Front Left
  sus1 = analogRead(susPin[0]);
  //  Serial.print("susdata1: ");
  //  Serial.print(sus1);
  buffPush(SUS_TRAV_FR, (unsigned long)(sus1));
}

void susData2() { // Front Right
  sus2 = analogRead(susPin[1]);
  //  Serial.print(" susdata2: ");
  //  Serial.println(sus2);
  buffPush(SUS_TRAV_FL, (unsigned long)(sus2)); 
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