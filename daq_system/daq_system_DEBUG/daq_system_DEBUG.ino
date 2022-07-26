
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
#include <C:\Users\Ariel\OneDrive\Documents\dev\DAQ_box\daq_system\daq_system_DEBUG\datastruct.h>


// Function Declarations
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void incrementHall_FR();
void incrementHall_SEC();
void setColour(int8_t edge);
void sdSend();
void buffPushFloat(int id, float tempData);
void buffPushInt(int id, unsigned long tempData);

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

void sdSend(){
  statusLED=!statusLED;
  digitalWrite(STATUS_PIN,statusLED);
  bajaData = SD.open(fileDir, FILE_WRITE); // Create file
  dataStruct sdTemp;
  while(!buff.isEmpty()){
    buff.pop(sdTemp);
    bajaData.write(sdTemp.timeStamp);
    bajaData.write(sdTemp.id);
    bajaData.write(sdTemp.typ);
    if (sdTemp.typ==INT){
       bajaData.write(sdTemp.data_long);
    }
    else if (sdTemp.typ==FLOAT){
       bajaData.write(sdTemp.data_float);
    }
  }
  bajaData.close();
}

void buffPushFloat(int id, float tempData){
  temp.id=id;
  temp.typ=FLOAT;
  temp.data_float=tempData;
  if (!buff.push(temp)){
    sdSend();
    buff.push(temp);
  }
}

void buffPushInt(int id, int tempData){
  temp.id=id;
  temp.typ=INT;
  temp.data_float=tempData;
  if (!buff.push(temp)){
    sdSend();
    buff.push(temp);
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
  //analogReference(INTERNAL4V3);
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SEC_HALL_PIN), incrementHall_SEC, FALLING);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  
  delay(1000);
  digitalWrite(STATUS_PIN, HIGH);
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
  if (!GPS.begin(115200)) {
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
  digitalWrite(STATUS_PIN, LOW);
  delay(1000);

}

void loop() {
  
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
      buffPushFloat(RPM_FR,FR_hall_count/((FR_past_time / 1000000.0)/60));
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
      buffPushFloat(RPM_SEC,SEC_hall_count/((SEC_past_time / 1000000.0)/60));
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
    
    buffPushInt(BATT_PERC,batPercent);
    buffPushFloat(BATT_VOLT,batVoltage);   
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
          bajaData = SD.open(fileDir, FILE_WRITE); // Create file
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
          digitalWrite(STATUS_PIN,HIGH);
          statusLED=True;
        }
        else {
          if (gps_flash == true) {
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
      if (gps_flash == true) {
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



    buffPushFloat(IMU_ABS_X,((float)event.orientation.x));
    buffPushFloat(IMU_ABS_Y,((float)event.orientation.y));
    buffPushFloat(IMU_ABS_Z,((float)event.orientation.z));

    buffPushFloat(IMU_ACCEL_X,accel.x());
    buffPushFloat(IMU_ACCEL_Y,accel.y());
    buffPushFloat(IMU_ACCEL_Z,accel.z());

    buffPushFloat(IMU_GRAVITY_X,gravity.x());
    buffPushFloat(IMU_GRAVITY_Y,gravity.y());
    buffPushFloat(IMU_GRAVITY_Z,gravity.z());

    buffPushFloat(IMU_GYRO_X,gyro.x());
    buffPushFloat(IMU_GYRO_Y,gyro.y());
    buffPushFloat(IMU_GYRO_Z,gyro.z());

    buffPushFloat(IMU_TEMP,bno.getTemp());
       
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
if (gps_timesend && gps_goodmessage && GPS.fix) {

      buffPushFloat(GPS_LATITUDE,GPS.latitude);
      buffPushInt(GPS_LAT,GPS.lat);

      buffPushFloat(GPS_LONGITUTE,GPS.longitude);
      buffPushInt(GPS_LON,GPS.lon);

      buffPushFloat(GPS_ANGLE,GPS.angle);
      buffPushFloat(GPS_SPEED,GPS.speed);

      buffPushInt(GPS_DAYMONTHYEAR,GPS.day<<16 & GPS.month<<8 & GPS.year);
      buffPushInt(GPS_SECONDMINUTEHOUR,GPS.seconds<<16 & GPS.minute<<8 & GPS.hour);
}


  //Suspension Travel write  
  
  buffPushInt(SUS_TRAV_FL,analogRead(SUS_PIN));
  buffPushInt(STRAIN1,analogRead(STRAIN_PIN));


  //-----------------Send Data--------------------------------------

  if (buff.size()>4 && millis()-sdTimer > (SD_INTERVAL-1)){
    if (gps_active){
      sdSend();
      sdTimer=millis();
    }
  }
  
}
