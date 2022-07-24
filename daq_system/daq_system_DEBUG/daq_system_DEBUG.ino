
/************************************************************************
  Author: DAQ Team
  Created on: 16/10/2020
**************************************************************************/



/*

   This version of the code includes using the 2 i2c temperature sesors

*/

//TODO
//CHANGE DIRECTORY BASED ON CURRENT DAY
//SECOND IMU
//Implement status LEDS
//Recording LED
//If the battery is low LAST LED goes Red
//more RPM sensors on inturrepts
#include <C:\Users\Ariel\OneDrive\Documents\dev\DAQ_box\daq_system\daq_system_DEBUG\daq_system_DEBUG.h>


// Function Declarations
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void incrementHall_FR();
void incrementHall_SEC();
void setColour(int8_t edge);

void getFilename(uint8_t hour, uint8_t minute, uint8_t second)
{
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "/%02d%02d%02d.CSV", (hour-5)%24+1, minute, second);
}

void getDirectory(uint8_t day, uint8_t month, uint8_t year)
{
  // Please move files before start of new day
  sprintf(directory, "/%02d-%02d-%02d", day, month, year);
}
void incrementHall_FR() {
  Serial.println("Front");
  FR_hall_count += 1;
}
void incrementHall_SEC() {
  Serial.println("Rear");
  SEC_hall_count += 1;
}

void setColour(int8_t edge)
{
  // Set all setColour to off/0
  strip.clear();

  const uint8_t R[10] = {255, 255, 255, 255, 255, 255, 100,   0,   0,  75};
  const uint8_t G[10] = {  0,  80, 150, 200, 200, 235, 255, 255,   0,   0};
  const uint8_t B[10] = {  0,   0,   0,   0,   0,   0,   0,   0, 255, 255};

  //Set pixel colour up to strip[edge]
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
}
void setup() {
  //Set up serial
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait until Serial is ready
  }
  Serial.println("Setup Starting");
  // Set up led strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  delay(50);


  //Setup analog reference votlage
  analogReference(INTERNAL4V3);
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SEC_HALL_PIN), incrementHall_SEC, FALLING);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(RECORDLED_PIN, OUTPUT);
  
  delay(1000);
  digitalWrite(RECORDLED_PIN, HIGH);
  delay(2000);
  // Set up imu
  if (!bno.begin()) {
    // There was a problem detecting the BNO055 ... check your connections
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    strip.setPixelColor(0, strip.Color(255, 0, 0));
  }
  else {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);

  if (!primaryTempSensor.begin(PrimaryTempAdress)) {
    Serial.println("Couldn't find primary MCP9808! Check your connections and verify the address is correct.");
    usePrimI2C = false;
    strip.setPixelColor(1, strip.Color(255, 0, 0));
  }
  else {
    strip.setPixelColor(1, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  if (!secondaryTempSensor.begin(SecondaryTempAdress)) {
    Serial.println("Couldn't find secondary MCP9808! Check your connections and verify the address is correct.");
    useSecI2C = false;
    strip.setPixelColor(2, strip.Color(255, 0, 0));
  }
  else {
    strip.setPixelColor(2, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  if (usePrimI2C) {
    primaryTempSensor.setResolution(2);
  }
  if (useSecI2C) {
    secondaryTempSensor.setResolution(2);
  }
  if (USE_SD) {
    // SD Card Setup
    if (!SD.begin(SD_CS_PIN)) {
      Serial.println("Card failed, or not present");
      strip.setPixelColor(3, strip.Color(255, 0, 0));
      strip.show();
      // don't do anything more:
      USE_SD = false;
    }
    else {
      Serial.println("card initialized.");
      strip.setPixelColor(3, strip.Color(0, 255, 0));
    }
    strip.show();
    
    delay(500);
  }


  // Set up GPS
  if (!GPS.begin(9600)) {
    strip.setPixelColor(4, strip.Color(255, 0, 0));
    Serial.println("GPS failed, or not present");
    use_gps = false;
  }
  else {
    strip.setPixelColor(4, strip.Color(0, 255, 0));
  }
  strip.show();
  // turn on turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); // 1 Hz update rate
  Serial.println("Setup Finished");
  digitalWrite(RECORDLED_PIN, LOW);
  delay(1000);

}

void loop() {
  Serial.println(STRAIN);
  //-------------Hall Effect Sensor--------------------

  // counting number of times the hall sensor is tripped
  // but without double counting during the same trip

  if (FR_hall_count > HALL_THRESH) {
  
    // print information about Time and spd
    FR_end_time = micros();
    FR_past_time = (FR_end_time - FR_start);
    if (FR_stopped) {
      FR_rpm = 0;
      FR_stopped = false;
    }
    else {
      FR_rpm = FR_hall_count/((FR_past_time / 1000000.0)/60);
    }
    FR_hall_count = 0;
    FR_start = micros();
  }
  if (!FR_stopped && (micros() - FR_start >= 1000000)) {
    FR_rpm  = 0;
    FR_stopped = true;
  }
  
  if (SEC_hall_count > HALL_THRESH) {

    // print information about Time and spd
    SEC_end_time = micros();
    SEC_past_time = (SEC_end_time - SEC_start);
    if (SEC_stopped) {
      SEC_rpm = 0;
      SEC_stopped = false;
    }
    else {
      SEC_rpm = SEC_hall_count/((SEC_past_time / 1000000.0)/60);
    }
    SEC_hall_count = 0;
    SEC_start = micros();
  }
  if (!SEC_stopped && (micros() - SEC_start >= 1000000)) {
    SEC_rpm  = 0;
    SEC_stopped = true;
  }



  //-------------Battery Check---------------
  if (millis() - battTimer > BATT_INTERVAL) {
    battTimer = millis();
    batVoltage = analogRead(VOLT_PIN);
    batPercent = map(batVoltage, 660, 750, 0, 100);
    batVoltage = (((batVoltage / 1024) * 4.3) * (16)) / 6;
  }


  //-------------LED Strip---------------------
  if (millis() - ledTimer > LED_INTERVAL) {
    ledTimer = millis();
    if (showRPM) {
      int numLED = map(SEC_rpm, 1800, 3800, -1, 9);
      if (numLED > 9) {
        numLED = 9;
      }
      setColour(numLED);
    }
    if (!showRPM) {
      int numLED = 0;
      numLED = map(SEC_rpm, 0, 5000, -1, 9);
      if (numLED > 9) {
        numLED = 9;
      }
      setColour(numLED);

    }
    strip.show();   // Send the updated pixel colors to the hardware.
  }




  //----------------------GPS--------------------------------

  // Using the adafruit ultimate gps chip
  // GPS works on interupt, but we output at reular intervals
  // https://learn.adafruit.com/adafruit-ultimate-gps
  // The regular output will output a gps signal that is up to 1 second behind the actual position
  // There are fixes for this (check GPS_HardwareSerial_Timing), but our current kalman fitler cannot handle aynchronous input
  // so there isn't any point to fixing it and we have to live with the small time inaccuracy
  // NMEA is the standard GPS format

  // read data from the GPS in the 'main loop'
  char c = GPS.read(); // c is raw gps data
  if (GPS.newNMEAreceived()) { // Interrupt signal for GPS signal
    // Do not handle or output data in this section. Only store it.
    // Me end up not listening and catching other sentences if we do output here

    if (!GPS.parse(GPS.lastNMEA())) { // this also sets the newNMEAreceived() flag to false
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
          getFilename(GPS.hour, GPS.minute, GPS.seconds);
          getDirectory(GPS.day, GPS.month, GPS.year);
          SD.mkdir(directory);
          Serial.println(filename);
          Serial.println(directory);
          strcpy(fileDir, directory);
          strcat(fileDir, filename);
          Serial.println(fileDir);
          File bajaData = SD.open(fileDir, FILE_WRITE); // Create file
          if (bajaData == 0) {
            Serial.println("File failed to write");
            // don't do anything more:
            strip.setPixelColor(5, strip.Color(255, 0, 0));
            while (1);
          }
          strip.setPixelColor(5, strip.Color(0, 255, 0));
          delay(500);
          bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage");
          bajaData.close();
          gps_flash = true;
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
          }
          Serial.println("Fix Found Recording Starting");
          digitalWrite(RECORDLED_PIN,HIGH);
        }
        else {
          if (gps_flash = true) {
            strip.setPixelColor(9, strip.Color(0, 255, 0));
            gps_flash = false;
          }
          else {
            strip.setPixelColor(9, strip.Color(0, 0, 0));
            gps_flash = true;
          }
          strip.show();
        }
      }
      gps_active = true;

    }
    else {
      if (gps_flash = true) {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(255, 0, 0));
        }
        gps_flash = false;
      }
      else {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        gps_flash = false;
      }
      strip.show();
    }
  }


  //-----------------IMU--------------------------------------
  // The chip used is the bno055-absolute-orientation-sensor
  // https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor
  // The imu of the chip combines its sensor data by itself to a aboslute orientation measurement using the chip hardware
  // It outputs absolute orientation and auto calibrates
  // It is recommended to help it calibrate by doing these steps
  // 1) Gyroscope: keep it still
  // 2) Magnetometer: Do figure 8 motions in all 3D directions
  // 3) Accelerometer: placed in 6 standing positions for +X, -X, +Y, -Y, +Z and -Z
  // We need raw values for the kalman filter though

  if (millis() - imuTimer > (IMU_INTERVAL - 1)) {
    
    // Format
    // Absolute orientation is euler vector (also can output quaternion for if needed)
    // Acceleration includes gravity (use VECTOR_LINEARACCEL to exclude) and is shows the Three axis of acceleration in (m/s^2)
    // Gyro is an Angular Velocity Vector measured in (rad/s)
    // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z

    imuTimer = millis(); // Get time
    bno.getEvent(&event);
    accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    gravity= bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);
    boardTemp = bno.getTemp();
    send_data = true;
  }

  //-----------------TEMP--------------------------------------


  if (millis() - tempTimer > TEMP_INTERVAL) {
    if (usePrimI2C) {
      primaryTemp = primaryTempSensor.readTempC();
    }
    if (usePrimI2C) {
      secondaryTemp = secondaryTempSensor.readTempC();
    }
  }



  Sus_Travel=(analogRead(SUS_PIN));
  STRAIN=(analogRead(STRAIN_PIN));

  //-----------------Send Data--------------------------------------

  String dataString = "";
  if (send_data && gps_active) {
    // Set up SD Card reader
    File bajaData;
    if (USE_SD) {
      bajaData = SD.open(fileDir, FILE_WRITE);
    }
    // IMU Format
    // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp
    // Time

    dataString += String(imuTimer); // Time since in ms
    dataString += F(",");

    // Orientation
    dataString += String((float)event.orientation.x); dataString += F(",");
    dataString += String((float)event.orientation.y); dataString += F(",");
    dataString += String((float)event.orientation.z); dataString += F(",");

    dataString += String(accel.x()); dataString += F(",");
    dataString += String(accel.y()); dataString += F(",");
    dataString += String(accel.z()); dataString += F(",");

    dataString += String(gravity.x()); dataString += F(",");
    dataString += String(gravity.y()); dataString += F(",");
    dataString += String(gravity.z()); dataString += F(",");

    dataString += String(gyro.x()); dataString += F(",");
    dataString += String(gyro.y()); dataString += F(",");
    dataString += String(gyro.z()); dataString += F(",");
    dataString += String(boardTemp); dataString += F(",");

    // GPS Format
    // Please read NMEA documentation to understand
    // Note that Fix is true or false and GPS Quality Indicator works like this: 0 - fix not available, 1 - GPS fix, 2 - Differential GPS fix
    // We get signals from GPGGA or GPRMC
    // Many values have to be converted to be usable (most software wants DD, not DMM)
    // GPS has its own timer, but due to the need of synchronous input, we have to accept some time inaccuracy and just use the imu timer
    // HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time
    if (gps_timesend && gps_goodmessage && GPS.fix) {

#ifdef USE_GPS_SPEED
      //spd = GPS.speed * 1.852;
#endif

      dataString += (int)GPS.fix; dataString += F(",");
      dataString += String(GPS.latitude, 4); dataString += String(GPS.lat); dataString += F(",");
      dataString += String(GPS.longitude, 4); dataString += String(GPS.lon); dataString += F(",");
      dataString += String(GPS.angle); dataString += F(",");
      dataString += String(GPS.speed); dataString += F(",");
      dataString += String(GPS.day) + "-" + String(GPS.month) + "-" + String(GPS.year) + " " + String(GPS.hour) + "-" + String(GPS.minute) + "-" + String(GPS.seconds) + " " + String(GPS.milliseconds); dataString += F(",");
      gps_timesend = false;
    } else {
      //Not GPS placeholder
      dataString += (int)GPS.fix;
      dataString += F(",");
      dataString += "-1,-1,-1,-1,-1,";
    }




    //Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain,FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage
    dataString += String(primaryTemp); dataString += F(",");
    dataString += String(secondaryTemp); dataString += F(",");
    dataString += String(Sus_Travel); dataString += F(",");
    dataString += String(STRAIN); dataString += F(",");
    dataString += String(FR_rpm); dataString += F(",");
    dataString += String(SEC_rpm); dataString += F(",");
    dataString += String(batPercent); dataString += F(",");
    dataString += String(batVoltage);

    if (USE_SD) {
      bajaData.println(dataString);
      bajaData.flush();
    }
    else {
      Serial.println(dataString);
    }
    send_data = false;
  }
  
}
