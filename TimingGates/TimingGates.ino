#include "SevSeg.h"

//Initialize seven segment display as an object
SevSeg sevseg; 

// FSR pin definitions and analog variables
int fsr1 = 0; 
int fsr2 = 1; 
int fsrReading1;
int fsrReading2;
int fsrSensitivity = 2;

// Timer variable definitions
float startTime;
float totalTime;
int timerFlag;

void setup(void) {
  Serial.begin(9600);

  // Setup seven segment display
  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = false;
  byte hardwareConfig = COMMON_CATHODE;
  bool updateWithDelays = false; 
  bool leadingZeros = false; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(10);
}
 
void loop(void) {
  sevseg.refreshDisplay(); // Refresh display
  fsrReading1 = analogRead(fsr1); // Read FSR 1
  if (fsrReading1 >= fsrSensitivity) { // Start timer when FSR 1 is pressed
    sevseg.blank(); // Clear display
    timerFlag = 1; // Set flag
    startTime = millis(); // Store current time
    while(timerFlag){
      fsrReading2 = analogRead(fsr2); // Read FSR 2
      if (fsrReading2 >= fsrSensitivity) { // Stop timer when FSR 2 is pressed
        totalTime = millis() - startTime; // Calculate total time
        totalTime /= 1000; // Convert milliseconds to seconds
        timerFlag = 0; // Reset flag for next lap
        sevseg.setNumberF(totalTime, 2);
        sevseg.refreshDisplay(); // Update display
        delay(1000); // Delay to prevent double timing error
      }
    }
  }
}
