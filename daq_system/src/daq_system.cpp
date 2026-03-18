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
#include "src/battery/battery.h"
#include "src/RPM/rpm.h"
#include "src/debug/debug.h"
#include "src/sheavePosition/sheavePos.h"

void setup() {

  Serial.begin(115200);

  Serial.println("Setup Starting");

  // Set up LED Strip
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
  delay(50);

  pinMode(REAR_SPEED_HALL_PIN, INPUT);
  pinMode(PRIM_HALL_PIN, INPUT);
  pinMode(24, INPUT);
  pinMode(25, INPUT);

  // Attach the interrupts to hall sensors to count rising edges
  // Uncomment when  in use

  // attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, RISING);
  // attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), incrementHall_FL, RISING);

  attachInterrupt(digitalPinToInterrupt(REAR_SPEED_HALL_PIN), incrementHall_REAR_SPEED, RISING);
  attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_PRIM, RISING);

  pinMode(STATUS_PIN, OUTPUT);

  delay(1000);

  // Setup section
  // If the sensor fails to initialize, the status LED will be red
  // If the sensor initializes successfully, the status LED will be green

  // Setup IMU
  if (!bno.begin()) {
    Serial.println("BNO FAILURE");
  } else {
    Serial.println("BNO SUCCESS");
  }
  strip.show();
  delay(500);

  // SD Card Setup
  if (USE_SD) {

    if (EN_FAST_SD) {
      //note, DMA_SDIO was slower, but might be useful in the future
      if (!SD.sdfs.begin(SdioConfig(FIFO_SDIO))) {
        Serial.println("SD using FIFO failed, falling back to slow SD");
        EN_FAST_SD = false;
      } else {
        Serial.println("SD using DMA");
        SdFile::dateTimeCallback(dateTime);
        Serial.println("CARD SUCCESS");
        //strip.setPixelColor(1, strip.Color(0, 255, 0));
        //EN_FAST_SD = false; // Disable fast SD for now due to stability issues
      }
    } else if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
     //strip.setPixelColor(1, strip.Color(100, 0, 0));
      strip.show();
      USE_SD = false;
    } else {
      SdFile::dateTimeCallback(dateTime);
      Serial.println("CARD SUCCESS");
      //strip.setPixelColor(1, strip.Color(0, 100, 0));
    }
    strip.show();
    delay(500);
  }

  // Set up GPS
  strip.setPixelColor(0, strip.Color(50, 0, 0));
  strip.show();
  if (!GPS.begin(38400)) {
    Serial.println("GPS failed, or not present");
  } else {
    //strip.setPixelColor(0, strip.Color(0, 30, 0));
    Serial.println("GPS SUCCESS");
    strip.setPixelColor(3, strip.Color(90, 195, 0));
  }
  strip.show();

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Minimum recommended data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);  // 10 Hz update rate

  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);
  //strip.setPixelColor(0, strip.Color(0, 255, 255));

  delay(1000);
}

void loop(){

  //-------------Debug Function-------------------
  controlDebug(Debug::NONE);

  //-------------Handle Input Button----------------
  handleInputButton();

  //-------------RPM Calculations-------------------
  rpmCalc();

  //-------------Battery Check---------------
  batteryCheck();

  //-------------LED Strip---------------------
  setHUD();

  //-------------GPS Data---------------------
  GPS.read();                 
  gps();
  
  if (EN_BRAKE && millis() - brakeTimer > (BRAKE_INTERVAL - 1)) {
    brakeTimer = millis();
    brake_pres = ((((analogRead(17) / 1024.0) * 5.15625) - 0.4) * 1250);
    // buffPush(BRAKE_PRESS, (float)(brake_pres));
  }

  if (EN_IMU && millis() - imuTimer > (IMU_INTERVAL - 1)) {
    imuTimer = millis();
    // imuData();
  }

  if (EN_STRAIN1 && millis() - strainTimer1 > (STRAIN_INTERVAL - 1)) {
    strainTimer1 = millis();
    //Uncomment when this works
    //sheavePos();
  }

  if (EN_STRAIN2 && millis() - strainTimer2 > (STRAIN_INTERVAL - 1)) {
    strainTimer2 = millis();
    // strainData(3);
  }
  
  if (EN_SUS1 && millis() - susTimer1 > (SUS_INTERVAL - 1)) {
    susTimer1 = millis();
    // susData1();
  }

  if (EN_SUS2 && millis() - susTimer2 > (SUS_INTERVAL - 1)) {
    susTimer2 = millis();
    // susData2();
  }

  if (EN_SUS3 && millis() - susTimer3 > (SUS_INTERVAL - 1)) {
    susTimer3 = millis();
    // susData3();
  }

  if (EN_SUS4 && millis() - susTimer4 > (SUS_INTERVAL - 1)) {
    susTimer4 = millis();
    // susData4();
  }

  if (EN_TEMP && millis() - tempTimer > (TEMP_INTERVAL - 1)) {
    tempTimer = millis();
    // tempData();
  }

  // If we are using SD card but haven't created the data file yet
  if(USE_SD && !(bajaDataFast || bajaData))
  {
    // If the GPS timeout has been reached
    if(!gps_active && millis() > GPS_TIMEOUT_MILLIS)
    {
      Serial.println("GPS Timed out, creating sequential file");
      createSequentialFile();
    }
    else if(gps_active) {
      Serial.println("GPS Acquired, creating datetime file");
      createDateTimeFile(GPS_day, GPS_month, GPS_year, GPS_hour, GPS_minute, GPS_seconds);
    }
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
