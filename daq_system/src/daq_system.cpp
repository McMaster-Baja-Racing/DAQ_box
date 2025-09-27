#include "TimeLib.h"
#include "daq_system_Teensy4.h"
#include "src/datastruct/dataTypes.h"
#include "src/fileInformation/file.h"
#include "src/gps/gps.h"
#include "src/sdCard/sdCard.h"
#include "src/battery/battery.h"
#include "src/RPM/rpm.h"
#include "src/datastruct/dataTypes.h"


void setup() {

  Serial.begin(115200);

  Serial.println("Setup Starting");

  delay(50);

  pinMode(REAR_SPEED_HALL_PIN, INPUT_PULLUP);
  pinMode(PRIM_HALL_PIN, INPUT_PULLUP);
  pinMode(24, INPUT);
  pinMode(25, INPUT);

  pinMode(STATUS_PIN, OUTPUT);

  delay(1000);

  // Setup section
  // If the sensor fails to initialize, the status LED will be red
  // If the sensor initializes successfully, the status LED will be green

  
  // SD Card Setup
  if (USE_SD) {
 
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      USE_SD = false;
    } else {
      SdFile::dateTimeCallback(dateTime);
      Serial.println("CARD SUCCESS");
    }
    delay(500); 
  }

  // Set up GPS
  if (!GPS.begin(9600)) {
    Serial.println("GPS failed, or not present");
    use_gps = false;
  } else {
    Serial.println("GPS SUCCESS");
  }

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Minimum recommended data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);  // 10 Hz update rate


  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);

  delay(1000);
}

void loop(){

  //-------------Debug Function-------------------

  //-------------Temperature Check-------------------
  //readTemp();

  //-------------Handle Input Button----------------
  handleInputButton();

  //-------------RPM Calculations-------------------
  rpmCalc();

  //-------------Battery Check---------------
  batteryCheck();

  //-------------GPS Data---------------------
  GPS.read();                 
  gps();
  
  /** 
  if ((millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))) {
    queueSizeTimer = millis();
    Serial.print("savingBuff Size = ");
    Serial.println((*savingBuff).size());

    Serial.print("sdBuff Size = ");
    Serial.println((*sdBuff).size());
  }
  */
  
  if (millis() - sdTimer > (SD_INTERVAL - 1)) {
    sdTimer = millis();
    sdSend();
  }
}
