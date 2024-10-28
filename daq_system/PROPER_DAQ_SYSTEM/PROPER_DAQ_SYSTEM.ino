#include "daq_system_Teensy4.h"
#include "datastruct.h"
#include "TimeLib.h"
#include "test.h"
#include "datetime.h"
#include "hud.h"
#include "sd.h"
#include "imu.h"
#include "sus.h"
#include "strain.h"
#include "inc.h"
#include "temp.h"

void setup() {
  // Intialize the serial communication
  Serial.begin(115200);
  Serial.println("Setup Starting");

  // Set up LED Strip
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  delay(50);

  // Configure the hall sensors pins 
  pinMode(FR_HALL_PIN, INPUT_PULLUP);
  pinMode(FR_HALL_PIN, INPUT_PULLUP);
  pinMode(SEC_HALL_PIN, INPUT_PULLUP);
  pinMode(PRIM_HALL_PIN, INPUT_PULLUP);

  // Attach the interrupts to hall sensors to count rising edges
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, RISING);
  attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), incrementHall_FL, RISING);
  attachInterrupt(digitalPinToInterrupt(SEC_HALL_PIN), incrementHall_SEC, RISING);
  attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_PRIM, RISING);

  // Set up status pin as output
  pinMode(STATUS_PIN, OUTPUT);

  delay(1000);

  // Setup IMU
  if (!bno.begin()) {
    // If the IMU fails to initialize, set the status LED to red
    Serial.println("BNO FAILURE");
    strip.setPixelColor(0, strip.Color(255, 0, 0));
  } else {
    // If the IMU initializes successfully, set the status LED to green
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    Serial.println("BNO SUCCESS");
  }
  strip.show();
  delay(500);

  // SD Card Setup
  if (USE_SD) {
    // Attempt to initialize the SD card
    if (!SD.begin(chipSelect)) {
      // If the SD card fails to initialize, set the status LED to red
      Serial.println("Card failed, or not present");
      strip.setPixelColor(1, strip.Color(255, 0, 0));
      strip.show();
      USE_SD = false; // Disable SD card usage
    } else {
      // If the SD card initializes successfully, set the status LED to green
      SdFile::dateTimeCallback(dateTime);
      Serial.println("CARD SUCCESS");
      strip.setPixelColor(1, strip.Color(0, 255, 0));
    }
    strip.show();
    delay(500);
  }

  // Set up GPS
  if (!GPS.begin(9600)) {
    // If the GPS fails to initialize, set the status LED to red
    strip.setPixelColor(2, strip.Color(255, 0, 0));
    Serial.println("GPS failed, or not present");
    use_gps = false;
  } else {
    // If the GPS initializes successfully, set the status LED to green
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    Serial.println("GPS SUCCESS");
  }
  strip.show();
  // Congfigure the GPS settings
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Minimum recommended data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);  // 10 Hz update rate

  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);

  delay(1000);
}

void loop(){
  // CALIBRATION UNCOMMENT THIS LINE AND CHANGE TO YOUR VARIABLE
  /*
  Serial.print("Brake pressue (Psi): ");
  Serial.println(brake_pres);
  delay(50);
  */

  // Variable names
  /*
  Serial.print("prim: ");
  Serial.println(PrRIM_rpm);
  delay(10);
  Serial.print(", Sec: ");
  Serial.println(SEC_rpm);
  */

  //  Strain data
  /*
  int strain [6];

  for (int i = 0; i < 6; i++) {
    Serial.print(i);
    Serial.print(" is: ");
    Serial.print(strain[i]);
    Serial.print("  |  ");
  }

  Serial.println("");
  delay(100);
  */

  // Temperature data
  /*
  int temperature;
  */
  
  // DO NOT RUN WITH FINAL CODE
  /*
  Serial.print("Sus1: ");
  Serial.print(sus1);
  Serial.print(", Sus2: ");
  Serial.print(sus2);
  Serial.print(", Sus3: ");
  Serial.print(sus3);
  Serial.print(", Sus4: ");
  Serial.println(sus4);
  delay(100); 
  Serial.print("Temp (C): ");
  Serial.println(temperature);
  delay(100);
  */

  inputButton.update();

  //Check if the button is pressed
  if (inputButton.fallingEdge()) {
    Serial.print("Button Pressed: ");

    if (gps_active) {
      HUD_SHOW = (HUD_SHOW + 1) % HUD_MODES; // Cycle through the HUD modes
      Serial.print(" Mode = "); 
      Serial.print(HUD_SHOW);
    } else {
      // If the button was presed within the last 250ms
      if (millis() - lastPressed < 250) {
        Serial.println("Start recording"); 
        EN_GPS = false;
        gps_active = true;

        // Create a directory for SD storage
        SD.mkdir(directory);
        Serial.println(filename);
        Serial.println(directory);

        // Construct the file path for the SD card
        strcpy(fileDir, directory);
        strcat(fileDir, filename);
        Serial.println(fileDir);

        // Open the file for writing
        bajaData = SD.open(fileDir, FILE_WRITE);
        if (bajaData == 0) {
          Serial.println("File failed to write");
          USE_SD = false;
        }

        // File header example
        // bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage");
        
        gps_flash = true;

        // Set the LED strip to green
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

  //-------------RPM----------------------
  if (EN_RPM) {
    if (SEC_hall_count > HALL_THRESH) {
      SEC_end_time = micros();
      SEC_past_time = (SEC_end_time - SEC_start);
      if (SEC_stopped) {
        SEC_stopped = false;
      }
      SEC_rpm = (SEC_hall_count / ((SEC_past_time / 1000000.0) / 60)) / SEC_counts_per_rotation;
      buffPush(RPM_SEC, (float)SEC_rpm);
      SEC_hall_count = 0;
      SEC_start = micros();
    }
    if (!SEC_stopped && (micros() - SEC_start >= 1000000)) {
      buffPush(RPM_SEC, float(0));
      SEC_stopped = true;
    }

    if (PRIM_hall_count > HALL_THRESH) {
      PRIM_end_time = micros();
      PRIM_past_time = (PRIM_end_time - PRIM_start);
      if (PRIM_stopped) {
        PRIM_stopped = false;
      }
      PRIM_rpm = (PRIM_hall_count / ((PRIM_past_time / 1000000.0) / 60)) / Prim_counts_per_rotation;
      buffPush(RPM_PRIM, (float)PRIM_rpm);
      PRIM_hall_count = 0;
      PRIM_start = micros();
    }
    if (!PRIM_stopped && (micros() - PRIM_start >= 1000000)) {
      buffPush(RPM_PRIM, float(0));
      PRIM_stopped = true;
    }
  }


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


  //-------------LED Strip---------------------
  if (EN_HUD && millis() - ledTimer > LED_INTERVAL) {
    ledTimer = millis();
    if (gps_active) {
      int numLED = 0;
      // Switch statement to determine which data to display on the LED strip
      switch (HUD_SHOW) {
        case PRIM:
          numLED = map(PRIM_rpm, 1700, 3800, -1, 8);
          break;
        case SEC:
          numLED = map(SEC_rpm, 0, 5000, -1, 8);
          break;
        case BRAKE:
          numLED = map(brake_pres, 0, 1200, -1, 8);
          break;
        case GPS_S:
          numLED = map(gps_speed, 5, 45, -1, 8);
          break;
        case BATT_PERCENT:
          numLED = map(batPercent, 0, 100, -1, 8);
          break;
        case STRAIN:
          numLED = map(temperature, 0, 150, -1, 8);
          break;
        case SUS1:
          numLED = map(sus1, 140, 310, -1, 8);
          break;
        case SUS2:
          numLED = map(sus2, 50, 312, -1, 8);
          break;
      }
      if (numLED > 8) {
            numLED = 8;
        }
      setColour(numLED);
    }
    strip.show();  // Send the updated pixel colors to the hardware.
  }

  // Read data from the GPS in the 'main loop'
  GPS.read();                   // c is raw gps data
  if (GPS.newNMEAreceived()) {  // Interrupt signal for GPS signal
    // Do not handle or output data in this section. Only store it.
    // Me end up not listening and catching other sentences if we do output here

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
