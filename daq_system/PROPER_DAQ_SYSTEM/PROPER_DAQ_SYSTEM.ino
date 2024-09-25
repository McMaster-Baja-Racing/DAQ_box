#include "daq_system_Teensy4.h"
#include "datastruct.h"
#include "TimeLib.h"

// Function Declarations
void setColour(int8_t edge);  // turns off LEDs from (start to edge)
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void setColour(int8_t edge);
void sdSend();
void buffPush(int id, float tempData); // Overloaded function to push data to the saving buffer with float data
void buffPush(int id, unsigned long tempData); // Overloaded function to push data to the saving buffer with unsigned long data
void strainData();
void imuData();
void susData();
void dateTime();

void dateTime(uint16_t* date, uint16_t* time) {

  uint16_t year;
  uint8_t month, day, hour, minute, second;
  // User gets date and time from GPS or real-time clock here
  if (GPS.fix) {
    year = GPS_year;
    month = GPS_month;
    day = GPS_day;
    hour = GPS_hour;
    minute = GPS_minute;
    second = GPS_seconds;
  } else {
    year = 2030;
    month = 3;
    day = 1;
    hour = 13;
    minute = 2;
    second = 50;
  }
  // Return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year, month, day);
  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
}

void getFilename(uint8_t hour, uint8_t minute, uint8_t second) {
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "/%02d%02d%02d.bin", (hour, minute, second));
}

void getDirectory(uint8_t day, uint8_t month, uint8_t year) {
  // Please move files before start of new day
  sprintf(directory, "/%02d-%02d-%02d", year, month, day);
}

void setColour(int8_t edge) {
  // Set all setColour to off/0
  strip.clear();

  const uint8_t R[10] = { 255, 255, 255, 255, 255, 255, 100, 0, 0, 75 };
  const uint8_t G[10] = { 0, 80, 150, 200, 200, 235, 255, 255, 0, 0 };
  const uint8_t B[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 255, 255 };

  // Set pixel colour up to strip[edge]
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
  strip.setPixelColor(9, strip.Color(butColour[HUD_SHOW][0], butColour[HUD_SHOW][1], butColour[HUD_SHOW][2]));
}

void sdSend() {
  // Check if GPS is active
  if (gps_active) {
    // Swap the saving and processing buffers
    if (savingBuff == &buff1) {
      savingBuff = &buff2; // Set saving buffer to buff2
      sdBuff = &buff1; // Set processing buffer to buff1
    } else if (savingBuff == &buff2) {
      savingBuff = &buff1; // Set saving buffer to buff1
      sdBuff = &buff2; //  Set processing buffer to buff2
    }
    // Toggle the status LED
    statusLED = !statusLED;
    digitalWrite(STATUS_PIN, statusLED);
    // Temporary array to hold data for writing to SD
    dataStruct sdTemp[8];
    int counter = 0;
    unsigned long str = millis();
    // Process the data from the sdBuff while its not empty
    while (!(*sdBuff).isEmpty()) {
      // Check if the time has exceeded 100ms
      if (millis() > (str + 100)) {
        Serial.println("SD LONG BOI");
        break;
      }
      // Pop the data from the sdBuff and push it to the sdTemp array
      (*sdBuff).pop(sdTemp[counter]);
      // Check if the counter has reached 7 and write the data to the SD card
      if (counter >= 7) {
        bajaData.write((uint8_t*)&sdTemp, sizeof(sdTemp));
        counter = -1;
      }
      // Increment the counter
      counter++;
    }
    // Write the remaining data to the SD card
    if (counter != 0 && millis() <= (str + 100)) {
      bajaData.write((uint8_t*)&sdTemp, sizeof(sdTemp));
    }
    // Flush the data to the SD card
    bajaData.flush(); 
  }
}

// Function to push data to the saving buffer with float data
void buffPush(int id, float tempData) {
  // Check if the SD card is not being used and the serial output is enabled
  if (!USE_SD && EN_SEROUT) {
    // Print the ID and data type to the Serial monitor for debugging
    Serial.print("FL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData, 4));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_float = tempData;
  // Check if the GPS is active and push the data to the saving buffer
  if (gps_active && !(*savingBuff).push(temp)) {
    // Print a message to the Serial monitor if the data is lost
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }
}

// Function to push data to the saving buffer with unsigned long data
void buffPush(int id, unsigned long tempData) {
  // Check if the SD card is not being used and the serial output is enabled
  if (!USE_SD && EN_SEROUT) {
    // Print the ID and data type to the Serial monitor for debugging
    Serial.println("UL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_long = tempData;
  // Check if the GPS is active and push the data to the saving buffer
  if (gps_active && !(*savingBuff).push(temp)) {
    // Print a message to the Serial monitor if the data is lost
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }
}

void strainData(int offset) {
  strain[offset] = analogRead(strainPin[offset]);
  buffPush(STRAIN1+offset, (unsigned long)(strain[offset]));
}

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

void tempData() {
  temperature = (analogRead(TEMPERATURE_PIN))/*/2*/;
  //  Serial.print("primtemp: ");
  //  Serial.println(temperature);
  buffPush(PRIM_TEMP, (unsigned long)(temperature));
}

void imuData() {
  // Retrieve teh latest IMU event data
  bno.getEvent(&event);

  // Initialize the variables for the IMU data
  uint8_t system_cal, gyro_cal, accel_cal, mag_cal;
  system_cal = gyro_cal = accel_cal = mag_cal = 0;

  // Get the calibration status of the IMU
  bno.getCalibration(&system_cal, &gyro_cal, &accel_cal, &mag_cal);

  // Get the accelerometer, gyro, and gravity vectors
  accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  gravity = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);

  // Check if the IMU is calibrated
  if (system_cal > 0) {
    // Push the orientation data to the buffer
    buffPush(IMU_ABS_X, ((float)event.orientation.x));
    buffPush(IMU_ABS_Y, ((float)event.orientation.y));
    buffPush(IMU_ABS_Z, ((float)event.orientation.z));
    // Push the temperature data to the buffer
    buffPush(IMU_TEMP, (unsigned long)bno.getTemp());
  }

  // Check if the accelerometer is calibrated
  if (accel_cal > 0) {
    // Push the accelerometer data to the buffer
    buffPush(IMU_ACCEL_X, float(accel.x()));
    buffPush(IMU_ACCEL_Y, float(accel.y()));
    buffPush(IMU_ACCEL_Z, float(accel.z()));
    // Push the gravity data to the buffer
    buffPush(IMU_GRAVITY_X, float(gravity.x()));
    buffPush(IMU_GRAVITY_Y, float(gravity.y()));
    buffPush(IMU_GRAVITY_Z, float(gravity.z()));
  }

  // Check if the gyro is calibrated
  if (gyro_cal > 0) {
    // Push the gyro data to the buffer
    buffPush(IMU_GYRO_X, float(gyro.x()));
    buffPush(IMU_GYRO_Y, float(gyro.y()));
    buffPush(IMU_GYRO_Z, float(gyro.z()));
  }
}

void setup() {
  // Intialize the serial communication
  Serial.begin(115200);
  Serial.println("Setup Starting");

  // Set up LED Strip
  strip.begin();                    // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                     // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS);  // Set BRIGHTNESS (max = 255)
  delay(50);

  // Configure the hall sensors pins 
  pinMode(FR_HALL_PIN, INPUT_PULLUP);
  pinMode(FR_HALL_PIN, INPUT_PULLUP);
  pinMode(SEC_HALL_PIN, INPUT_PULLUP);
  pinMode(PRIM_HALL_PIN, INPUT_PULLUP);

  // Attach the interrupts to hall sensors to count rising edges
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), FR_hall_count += 1, RISING);
  attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), FL_hall_count += 1, RISING);
  attachInterrupt(digitalPinToInterrupt(SEC_HALL_PIN), SEC_hall_count += 1, RISING);
  attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), PRIM_hall_count += 1, RISING);

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
      Serial.println("Card failed, or not present"); // Error message
      strip.setPixelColor(1, strip.Color(255, 0, 0)); // Set LED to Red
      strip.show();
      USE_SD = false; // Disable SD card usage
    } else {
      SdFile::dateTimeCallback(dateTime); // Set the date and time callback
      Serial.println("CARD SUCCESS"); // Success message
      strip.setPixelColor(1, strip.Color(0, 255, 0)); // Set LED to Green
    }
    strip.show();
    delay(500);
  }

  // Set up GPS
  if (!GPS.begin(9600)) {
    strip.setPixelColor(2, strip.Color(255, 0, 0)); // Set LED to Red
    Serial.println("GPS failed, or not present"); // Error message
    use_gps = false; // Disable GPS usage
  } else {
    strip.setPixelColor(2, strip.Color(0, 255, 0)); // Set LED to Green
    Serial.println("GPS SUCCESS"); // Success message
  }
  strip.show();
  // Congfigure the GPS settings
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Minimum recommended data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);  // 10 Hz update rate

  Serial.println("Setup Finished"); // Setup complete message
  digitalWrite(STATUS_PIN, LOW); // Turn off the status LED

  delay(1000);
}

void loop(){
  //CALRATION UNCOMMENT THIS LINE AND CHANGE TO YOUR VARIABLE
//  Serial.print("Brake pressue (Psi): ");
//  Serial.println(brake_pres);
//  delay(50);
  

  //Variable names
  /*
  Serial.print("prim: ");
  Serial.println(PrRIM_rpm);
  delay(10);
  Serial.print(", Sec: ");
  Serial.println(SEC_rpm);
  */

  
//  strain data
//  int strain [6];
//
//  for (int i = 0; i < 6; i++) {
//    Serial.print(i);
//    Serial.print(" is: ");
//    Serial.print(strain[i]);
//    Serial.print("  |  ");
//  }
//
//  Serial.println("");
//  delay(100);

//  //temperature data...../
//  int temperature;
  

  //DO NOT RUN WITH FINAL CODE
//   Serial.print("Sus1: ");
//   Serial.print(sus1);
//   Serial.print(", Sus2: ");
//   Serial.print(sus2);
//   Serial.print(", Sus3: ");
//   Serial.print(sus3);
//   Serial.print(", Sus4: ");
//  Serial.println(sus4);
//   delay(100); 

//  Serial.print("Temp (C): ");
//  Serial.println(temperature);
//  delay(100);

  inputButton.update();
  if (inputButton.fallingEdge()) {
    Serial.print("Button Pressed: ");
    if (gps_active) {
      HUD_SHOW = (HUD_SHOW + 1) % HUD_MODES;
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
        bajaData = SD.open(fileDir, FILE_WRITE);  // Create file
        if (bajaData == 0) {
          Serial.println("File failed to write");
          // don't do anything more:
          USE_SD = false;
        }
        //bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage");
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

  //-------------RPM----------------------
  if (EN_RPM) {
    if (SEC_hall_count > HALL_THRESH) {
      // print information about Time and spd
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
      // print information about Time and spd
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
