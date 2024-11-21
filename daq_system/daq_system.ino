#include "TimeLib.h"
#include "daq_system_Teensy4.h"
#include "src/datastruct/dataTypes.h"
#include "src/fileInformation/file.h"
#include "src/datetime/dateTime.h"
#include "src/hud/hud.h"
#include "src/sdCard/sdCard.h"
#include "src/imu/imu.h"
#include "src/suspensionData/sus.h"
#include "src/strainData/strain.h"
#include "src/counters/counters.h"
#include "src/temperature/temperature.h"
#include "src/RPM/rpm.h"
#include "src/debug/debug.h"

void setup() {

  Serial.begin(115200);

  Serial.println("Setup Starting");

  // Set up LED Strip
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  delay(50);

  pinMode(FR_HALL_PIN, INPUT_PULLUP);
  pinMode(FL_HALL_PIN, INPUT_PULLUP);
  pinMode(REAR_SPEED_HALL_PIN, INPUT_PULLUP);
  pinMode(PRIM_HALL_PIN, INPUT_PULLUP);

  // Attach the interrupts to hall sensors to count rising edges
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, RISING);
  attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), incrementHall_FL, RISING);
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
    strip.setPixelColor(0, strip.Color(255, 0, 0));
  } else {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    Serial.println("BNO SUCCESS");
  }
  strip.show();
  delay(500);

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

  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);

  delay(1000);
}

void loop(){

  // Debug function, input mode you want to debug
  controlDebug(Debug::NONE);

  if(mcp_initialized) {
    float hotTemp = mcp.readThermocouple();
    Serial.print("Thermocouple Temperature: ");
    Serial.print(hotTemp);
    Serial.println(" C");
  }

  inputButton.update();

  if (inputButton.fallingEdge()) {
    Serial.print("Button Pressed: ");

    if (gps_active) {
      HUD_SHOW = (HUD_SHOW + 1) % HUD_MODES; // Cycle through the HUD modes
      Serial.print(" Mode = "); 
      Serial.print(HUD_SHOW);
    } else {

      if (millis() - lastPressed < 250) {
        Serial.println("Start recording"); 
        EN_GPS = false;
        gps_active = true;

        SD.mkdir(directory);
        Serial.println(filename);
        Serial.println(directory);

        strcpy(fileDir, directory);
        strcat(fileDir, filename);
        Serial.println(fileDir);

        bajaData = SD.open(fileDir, FILE_WRITE);
        if (bajaData == 0) {
          Serial.println("File failed to write");
          USE_SD = false;
        }

        // File header example
        // bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage");
        
        gps_flash = true;
        
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 255, 0));
        }

        Serial.print("GPS cancelled");
        digitalWrite(STATUS_PIN, HIGH);
        strip.show();
        delay(1000);
        statusLED = true;
      }
      lastPressed = millis();
    }
    Serial.println();
  }

  //-------------RPM Check-------------------

  rpmCalc();

  //-------------Battery Check---------------
  if (EN_BATT && millis() - battTimer > BATT_INTERVAL) {
    battTimer = millis();
    batVoltage = analogRead(VOLT_PIN);
    batPercent = map(batVoltage, 820, 930, 0, 100);
    if (batPercent <= 0) {
      batPercent = 0;
    } else if (batPercent >= 100) {
      batPercent = 100;
    }
    batVoltage = ((batVoltage / 1024) * 9.1905);

    buffPush(BATT_PERC, (unsigned long)(batPercent));
    buffPush(BATT_VOLT, batVoltage);
  }

  setHUD();

  // Read data from the GPS in the 'main loop'
  GPS.read();                 
  if (GPS.newNMEAreceived()) {  
    if (!GPS.parse(GPS.lastNMEA())) {  // this also sets the newNMEAreceived() flag to false
      gps_goodmessage = false;
    } else {
      gps_goodmessage = true;
    }
  }

  if (millis() - gpsTimer > (GPS_INTERVAL - 1)) {
    gpsTimer = millis();
    gps_timesend = true;
    if (GPS.fix) {
      if (USE_SD) {
        if (gps_active == false) {
          GPS_year = GPS.year;
          GPS_year = GPS.year;
          GPS_month = GPS.month;
          GPS_day = GPS.day;
          GPS_hour = GPS.hour;
          GPS_minute = GPS.minute;
          GPS_seconds = GPS.seconds;
          getFilename(GPS.hour, GPS.minute, GPS.seconds);
          getDirectory(GPS.day, GPS.month, GPS.year);
          SD.mkdir(directory);
          Serial.println(filename);
          Serial.println(directory);
          strcpy(fileDir, directory);
          strcat(fileDir, filename);
          Serial.println(fileDir);
          bajaData = SD.open(fileDir, FILE_WRITE);  // Create file
          if (bajaData == 0) {
            Serial.println("File failed to write");
            // don't do anything more:
            USE_SD = false;
          }
          gps_flash = true;
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
          }
          Serial.println("Fix Found Recording Starting");
          digitalWrite(STATUS_PIN, HIGH);
          strip.show();
          delay(1000);
          statusLED = true;
        }
      }
      gps_active = true;

    } else if (EN_GPS) {
      if (gps_flash == true) {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(255, 0, 0));
        }
        gps_flash = false;
      } else {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        gps_flash = true;
      }

      strip.show();
    }
  }

  if (EN_GPS && gps_timesend && gps_goodmessage && GPS.fix) {

    buffPush(GPS_LATITUDE, GPS.latitude);
    buffPush(GPS_LAT, (unsigned long)GPS.lat);

    buffPush(GPS_LONGITUTE, GPS.longitude);
    buffPush(GPS_LON, (unsigned long)GPS.lon);

    buffPush(GPS_ANGLE, GPS.angle);

    // GPS speed is in knots
    gps_speed = GPS.speed * 1.852;
    buffPush(GPS_SPEED, gps_speed);
    buffPush(GPS_DAYMONTHYEAR, (unsigned long)((GPS.day << 16) + (GPS.month << 8) + (GPS.year)));
    buffPush(GPS_SECONDMINUTEHOUR, (unsigned long)((GPS.seconds << 16) + (GPS.minute << 8) + (GPS.hour)));
    if (GPS.minute != GPS_minute) {
      GPS_year = GPS.year;
      GPS_year = GPS.year;
      GPS_month = GPS.month;
      GPS_day = GPS.day;
      GPS_hour = GPS.hour;
      GPS_minute = GPS.minute;
      GPS_seconds = GPS.seconds;
    }
    gps_timesend = false;
  }
  
  if (EN_BRAKE && millis() - brakeTimer > (BRAKE_INTERVAL - 1)) {
    brakeTimer = millis();
    brake_pres = ((((analogRead(17) / 1024.0) * 5.15625) - 0.4) * 1250);
    buffPush(BRAKE_PRESS, (float)(brake_pres));
  }
  if (EN_IMU && millis() - imuTimer > (IMU_INTERVAL - 1)) {
    imuTimer = millis();
    imuData();
  }
  if (EN_STRAIN1 && millis() - strainTimer1 > (STRAIN_INTERVAL - 1)) {
    strainTimer1 = millis();
    strainData(2);
  }
  if (EN_STRAIN2 && millis() - strainTimer2 > (STRAIN_INTERVAL - 1)) {
    strainTimer2 = millis();
    strainData(3);
  }
  if (EN_SUS1 && millis() - susTimer1 > (SUS_INTERVAL - 1)) {
    susTimer1 = millis();
    susData1();
  }
  if (EN_SUS2 && millis() - susTimer2 > (SUS_INTERVAL - 1)) {
    susTimer2 = millis();
    susData2();
  }
  if (EN_SUS3 && millis() - susTimer3 > (SUS_INTERVAL - 1)) {
    susTimer3 = millis();
    susData3();
  }
  if (EN_SUS4 && millis() - susTimer4 > (SUS_INTERVAL - 1)) {
    susTimer4 = millis();
    susData4();
  }
  if (EN_TEMP && millis() - tempTimer > (TEMP_INTERVAL - 1)) {
    tempTimer = millis();
    tempData();
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
