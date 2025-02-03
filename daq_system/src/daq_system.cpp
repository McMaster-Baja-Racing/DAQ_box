#include "TimeLib.h"
#include "daq_system_Teensy4.h"
#include "src/datastruct/dataTypes.h"
#include "src/fileInformation/file.h"
#include "src/gps/gps.h"
#include "src/hud/hud.h"
#include "src/sdCard/sdCard.h"
#include "src/imu/imu.h"
#include "src/suspensionData/sus.h"
#include "src/strainData/strain.h"
#include "src/counters/counters.h"
#include "src/temperature/temperature.h"
#include "src/battery/battery.h"
#include "src/RPM/rpm.h"
#include "src/debug/debug.h"
#include "src/sheavePosition/sheavePos.h"

void setup() {

  Serial.begin(115200);

  Serial.println("Setup Starting");

  // Set up LED Strip
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  delay(50);

  // pinMode(FR_HALL_PIN, INPUT_PULLUP);
  // pinMode(FL_HALL_PIN, INPUT_PULLUP);
  // pinMode(REAR_SPEED_HALL_PIN, INPUT_PULLUP);
  // pinMode(PRIM_HALL_PIN, INPUT_PULLUP);
  // pinMode(24, INPUT);
  // pinMode(25, INPUT);

  // // Attach the interrupts to hall sensors to count rising edges
  // attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, RISING);
  // attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), incrementHall_FL, RISING);
  // attachInterrupt(digitalPinToInterrupt(REAR_SPEED_HALL_PIN), incrementHall_REAR_SPEED, RISING);
  // attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_PRIM, RISING);

  pinMode(STATUS_PIN, OUTPUT);

  delay(1000);

  // Setup section
  // If the sensor fails to initialize, the status LED will be red
  // If the sensor initializes successfully, the status LED will be green

  // Setup IMU
  // if (!bno.begin()) {
  //   Serial.println("BNO FAILURE");
  //   strip.setPixelColor(0, strip.Color(255, 0, 0));
  // } else {
  //   strip.setPixelColor(0, strip.Color(0, 255, 0));
  //   Serial.println("BNO SUCCESS");
  // }
  // strip.show();
  // delay(500);

  // SD Card Setup
  if (USE_SD) {
 
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      strip.setPixelColor(1, strip.Color(255, 0, 0));
      strip.show();
      USE_SD = false;
    } else {
      SdFile::dateTimeCallback(dateTime);
      Serial.println("CARD SUCCESS");
      strip.setPixelColor(1, strip.Color(0, 255, 0));
    }
    strip.show();
    delay(500);
  }

  // Set up Temperature Sensor
  //tempSetup();

  // Set up GPS
  if (!GPS.begin(9600)) {
    strip.setPixelColor(2, strip.Color(255, 0, 0));
    Serial.println("GPS failed, or not present");
    use_gps = false;
  } else {
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    Serial.println("GPS SUCCESS");
  }
  strip.show();

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Minimum recommended data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);  // 10 Hz update rate

  // Serial.println("Adafruit VL6180x test!");
  // if (vl.begin()) {
  //   Serial.println("Found sensor");
  // } else {
  //   Serial.println("Sensor not found");
  // }

  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);

  delay(1000);
}

void loop(){

  //-------------Debug Function-------------------
  //controlDebug(Debug::NONE);

  //-------------Temperature Check-------------------
  //readTemp();

  //-------------Handle Input Button----------------
  //handleInputButton();

  //-------------RPM Calculations-------------------
  //rpmCalc();

  //-------------Battery Check---------------
  //batteryCheck();

  //-------------LED Strip---------------------
  //setHUD();

  //-------------GPS Data---------------------
  GPS.read();                 
  gps();
  
  if (EN_BRAKE && millis() - brakeTimer > (BRAKE_INTERVAL - 1)) {
    brakeTimer = millis();
    brake_pres = ((((analogRead(17) / 1024.0) * 5.15625) - 0.4) * 1250);
    //buffPush(BRAKE_PRESS, (float)(brake_pres));
  }

  if (EN_IMU && millis() - imuTimer > (IMU_INTERVAL - 1)) {
    imuTimer = millis();
    //imuData();
  }

  if (EN_STRAIN1 && millis() - strainTimer1 > (STRAIN_INTERVAL - 1)) {
    strainTimer1 = millis();
    //Uncomment when this works
    //sheavePos();
  }

  if (EN_STRAIN2 && millis() - strainTimer2 > (STRAIN_INTERVAL - 1)) {
    strainTimer2 = millis();
    //strainData(3);
  }
  
  if (EN_SUS1 && millis() - susTimer1 > (SUS_INTERVAL - 1)) {
    susTimer1 = millis();
    //susData1();
  }

  if (EN_SUS2 && millis() - susTimer2 > (SUS_INTERVAL - 1)) {
    susTimer2 = millis();
    //susData2();
  }

  if (EN_SUS3 && millis() - susTimer3 > (SUS_INTERVAL - 1)) {
    susTimer3 = millis();
    //susData3();
  }

  if (EN_SUS4 && millis() - susTimer4 > (SUS_INTERVAL - 1)) {
    susTimer4 = millis();
    //susData4();
  }

  if (EN_TEMP && millis() - tempTimer > (TEMP_INTERVAL - 1)) {
    tempTimer = millis();
    //tempData();
  }

  if (SHOW_DEBUG && (millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))) {
    queueSizeTimer = millis();
    Serial.print("savingBuff Size = ");
    Serial.println((*savingBuff).size());

    Serial.print("sdBuff Size = ");
    Serial.println((*sdBuff).size());
  }
  
  if (millis() - sdTimer > (SD_INTERVAL - 1)) {
    sdTimer = millis();
    sdSend();
  }
}
