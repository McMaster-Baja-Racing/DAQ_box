#include <cstdint>
#include "debug.h"
#include "../datastruct/dataTypes.h"
#include "../strainData/strain.h"
#include "../suspensionData/sus.h"
#include "../RPM/rpm.h"

// TODO: Move Brake Pressure stuff to own folder
void debug_brake_pressure(){
    // Serial.print("Brake pressue (Psi): ");
    // Serial.println(brake_pres);
    // delay(50);
}

void debug_rpm(){
    Serial.print("prim: ");
    Serial.println(PRIM_rpm);
    delay(10);
    Serial.print(", Sec: ");
    Serial.println(REAR_SPEED_int);
}

void debug_strain(){
    int strain [6];

    for (int i = 0; i < 6; i++) {
    Serial.print(i);
    Serial.print(" is: ");
    Serial.print(strain[i]);
    Serial.print("  |  ");
    }

    Serial.println("");
    delay(100);
}


void debug_suspension(){
    Serial.print("Sus1: ");
    Serial.print(sus1);
    Serial.print(", Sus2: ");
    Serial.print(sus2);
    Serial.print(", Sus3: ");
    Serial.print(sus3);
    Serial.print(", Sus4: ");
    Serial.println(sus4);
    delay(100); 
}  

void controlDebug(Debug mode){
    switch(mode){
    case Debug::BRAKE_PRESSURE_DEBUG:
      debug_brake_pressure();
      break;
    case Debug::RPM_DEBUG:
      debug_rpm();
      break;
    case Debug::STRAIN_DEBUG:
      debug_strain();
      break;
    case Debug::SUSPENSION_DEBUG:
      debug_suspension();
      break;
    case Debug::NONE:
      break;
  }
}