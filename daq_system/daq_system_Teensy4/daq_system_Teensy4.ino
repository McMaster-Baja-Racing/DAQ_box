

#include "daq_system_Teensy4.h"
#include "datastruct.h"


// Function Declarations
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void incrementHall_FR();
void incrementHall_FL();
void incrementHall_SEC();
void incrementHall_PRIM();
void setColour(int8_t edge);
void sdSend();
void buffPush(int id, float tempData);
void buffPush(int id, unsigned long tempData);
void strainData();
void imuData();
void susData();

void getFilename(uint8_t hour, uint8_t minute, uint8_t second)
{
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "/%02d%02d%02d.bin", (hour-5)%24+1, minute, second);
}

void getDirectory(uint8_t day, uint8_t month, uint8_t year)
{
  // Please move files before start of new day
  sprintf(directory, "/%02d-%02d-%02d", day, month, year);
}
void incrementHall_FR() {
  Serial.println("Front Right RPM Pin Hit");
  FR_hall_count += 1;
}
void incrementHall_FL() {
  Serial.println("Front Left RPM Pin Hit");
  FL_hall_count += 1;
}
void incrementHall_SEC() {
  //Serial.println("Secondary RPM Pin Hit");
  SEC_hall_count += 1;
}

void incrementHall_PRIM() {
  Serial.println("PRIM RPM Pin Hit");
  PRIM_hall_count += 1;
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
  if(gps_active){
    //Serial.println("PRE WRITE");
    //Serial.print("premicros: ");
    int preMicros=micros();
    //Serial.println(preMicros);
    //Serial.print("Size of buffer that saves to SD: ");
    float currentSize=100*(*savingBuff).size()/(float)dataBufferSize;
    if (maxSize<currentSize){
      maxSize=currentSize;
    }
    //Serial.println(currentSize);
    //Serial.print("Size of buffer that will get written to: ");
    //Serial.println((*sdBuff).size());
    if (savingBuff==&buff1){
      savingBuff=&buff2;
      sdBuff=&buff1;
    }
    else if (savingBuff==&buff2){
      savingBuff=&buff1;
      sdBuff=&buff2;
    }
    //Serial.print("Writing to SD name: ");
    //Serial.println(fileDir);
    statusLED=!statusLED;
    digitalWrite(STATUS_PIN,statusLED);
    dataStruct sdTemp[8];
    int counter=0;
    while(!(*sdBuff).isEmpty()){
      (*sdBuff).pop(sdTemp[counter]);
      if (counter>=7){
        bajaData.write((uint8_t *)&sdTemp,sizeof(sdTemp));
        counter=-1;
      }
      counter++;
    }
    if (counter!=0){
      bajaData.write((uint8_t *)&sdTemp,sizeof(sdTemp));
    }
    bajaData.flush();
    //Serial.println("POST WRITE");
//    Serial.print("post micros: ");
//    Serial.println(micros()-preMicros);
//    Serial.print("Size of buffer that saves to SD: ");
//    Serial.println(100*(*sdBuff).size()/(float)dataBufferSize);
//    Serial.print("Size of buffer that will get written to: ");
//    Serial.println((*savingBuff).size());
  }
}

void buffPush(int id, float tempData){
  if (!USE_SD){
    Serial.print("FL* ID: ");
    Serial.print(DataTypeNames[id]);
    Serial.print(" Data: ");
    Serial.println(tempData,4);
    return;
  }
  temp.timeStamp_typ=(millis()<<6)|id;
  temp.data_float=tempData;
  if (!(*savingBuff).push(temp)){
    Serial.println("Lost Data");
  }
}

void buffPush(int id, unsigned long tempData){
  if (!USE_SD){
    Serial.print("UL* ID: ");
    Serial.print(DataTypeNames[id]);
    Serial.print(" Data: ");
    Serial.println(tempData);
    return;
  }
  temp.timeStamp_typ=(millis()<<6)|id;
  temp.data_long=tempData;
  if (!(*savingBuff).push(temp)){
    Serial.println("Lost Data");
  }
}

void strainData(){
  //Serial.print("a,");
  for (int i =0;i<2;i++){
    buffPush(STRAIN1+i,(unsigned long)(analogRead(strainPin[i])));
  }
}

void imuData(){
    bno.getEvent(&event);
    accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    gravity= bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);



    buffPush(IMU_ABS_X,((float)event.orientation.x));
    buffPush(IMU_ABS_Y,((float)event.orientation.y));
    buffPush(IMU_ABS_Z,((float)event.orientation.z));

    buffPush(IMU_ACCEL_X,float(accel.x()));
    buffPush(IMU_ACCEL_Y,float(accel.y()));
    buffPush(IMU_ACCEL_Z,float(accel.z()));

    buffPush(IMU_GRAVITY_X,float(gravity.x()));
    buffPush(IMU_GRAVITY_Y,float(gravity.y()));
    buffPush(IMU_GRAVITY_Z,float(gravity.z()));

    buffPush(IMU_GYRO_X,float(gyro.x()));
    buffPush(IMU_GYRO_Y,float(gyro.y()));
    buffPush(IMU_GYRO_Z,float(gyro.z()));

    buffPush(IMU_TEMP,(unsigned long)bno.getTemp());
}

void susData(){
  
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait until Serial is ready
  }

  Serial.println("Setup Starting");
  Serial.println(sizeof(sdBuff));
  // Set up led strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  delay(50);
  attachInterrupt(digitalPinToInterrupt(FR_HALL_PIN), incrementHall_FR, FALLING);
  attachInterrupt(digitalPinToInterrupt(FL_HALL_PIN), incrementHall_FR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SEC_HALL_PIN), incrementHall_SEC, FALLING);
  attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_SEC, FALLING);
  
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

  if (USE_SD) {
    // SD Card Setup
    if (!SD.begin(chipSelect)) {
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
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  Serial.println("Setup Finished");
  digitalWrite(STATUS_PIN, LOW);

  bajaData = SD.open("test3.bin", FILE_WRITE); // Create file
  gps_active=true;
  //imuTimer.priority(135);
  sdTimer.priority(140);
  //strainTimer.priority(100);
  //strainTimer.begin(strainData, 100);
  //imuTimer.begin(imuData,10000);
  sdTimer.begin(sdSend,50000);

  delay(1000);
}

void loop() {
  //Serial.println("Entering Loop");
  if (FR_hall_count > HALL_THRESH) {
    
    // print information about Time and spd
    FR_end_time = micros();
    FR_past_time = (FR_end_time - FR_start);
    if (FR_stopped) {
      FR_stopped = false;
    }
    buffPush(RPM_FR,float(FR_hall_count/(8*(FR_past_time / 1000000.0)/60)));
    FR_hall_count = 0;
    FR_start = micros();
  }
  if (!FR_stopped && (micros() - FR_start >= 1000000)) {
    buffPush(RPM_FR,float(0));
    FR_stopped = true;
  }

  if (FL_hall_count > HALL_THRESH) {
  
    // print information about Time and spd
    FL_end_time = micros();
    FL_past_time = (FL_end_time - FL_start);
    if (FL_stopped) {
      FL_stopped = false;
    }
    buffPush(RPM_FL,float(FL_hall_count/(8*(FL_past_time / 1000000.0)/60)));
    FL_hall_count = 0;
    FL_start = micros();
  }
  if (!FL_stopped && (micros() - FL_start >= 1000000)) {
    buffPush(RPM_FL,float(0));
    FL_stopped = true;
  }
  
  if (SEC_hall_count > HALL_THRESH) {
    // print information about Time and spd
    SEC_end_time = micros();
    SEC_past_time = (SEC_end_time - SEC_start);
    if (SEC_stopped) {
      SEC_stopped = false;
    }
    buffPush(RPM_SEC,float(SEC_hall_count/(8*(SEC_past_time / 1000000.0)/60)));
    SEC_hall_count = 0;
    SEC_start = micros();
  }
  if (!SEC_stopped && (micros() - SEC_start >= 1000000)) {
    buffPush(RPM_SEC,float(0));
    SEC_stopped = true;
  }
  
  if (PRIM_hall_count > HALL_THRESH) {
    // print information about Time and spd
    PRIM_end_time = micros();
    PRIM_past_time = (PRIM_end_time - PRIM_start);
    if (PRIM_stopped) {
      PRIM_stopped = false;
    }
    buffPush(RPM_PRIM,float(PRIM_hall_count/(8*(PRIM_past_time / 1000000.0)/60)));
    PRIM_hall_count = 0;
    PRIM_start = micros();
  }
  if (!PRIM_stopped && (micros() - PRIM_start >= 1000000)) {
    buffPush(RPM_PRIM,float(0));
    PRIM_stopped = true;
  }
  

  //-------------Battery Check---------------
  if (millis() - battTimer > BATT_INTERVAL) {
    battTimer = millis();
    batVoltage = analogRead(VOLT_PIN);
    batPercent = map(batVoltage, 820, 930, 0, 100);
    if (batPercent<=0){
      batPercent=0;
    }
    else if (batPercent>=100){
      batPercent=100;
    }
    batVoltage = ((batVoltage / 1024) * 9.1905);
    
    buffPush(BATT_PERC,(unsigned long) (batPercent));
    buffPush(BATT_VOLT,batVoltage);   
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

  // read data from the GPS in the 'main loop'
  GPS.read(); // c is raw gps data
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
          //bajaData = SD.open(fileDir, FILE_WRITE); // Create file
          if (bajaData == 0) {
            Serial.println("File failed to write");
            // don't do anything more:
            strip.setPixelColor(5, strip.Color(255, 0, 0));
            while (1);
          }
          strip.setPixelColor(5, strip.Color(0, 255, 0));
          delay(500);
          //bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, SEC_RPM,Battery Percentage, Battery Voltage");
          bajaData.close();
          gps_flash = true;
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
          }
          Serial.println("Fix Found Recording Starting");

          //BEGINING THE STRAIN RECORDING TIMER
          
          
          digitalWrite(STATUS_PIN,HIGH);
          statusLED=true;
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
  if (gps_timesend && gps_goodmessage && GPS.fix) {

      buffPush(GPS_LATITUDE,GPS.latitude);
      buffPush(GPS_LAT,(unsigned long)GPS.lat);

      buffPush(GPS_LONGITUTE,GPS.longitude); 
      buffPush(GPS_LON,(unsigned long)GPS.lon);

      buffPush(GPS_ANGLE,GPS.angle);
      buffPush(GPS_SPEED,GPS.speed);

      buffPush(GPS_DAYMONTHYEAR,(unsigned long)GPS.day<<16 & GPS.month<<8 & GPS.year);
      buffPush(GPS_SECONDMINUTEHOUR,(unsigned long)GPS.seconds<<16 & GPS.minute<<8 & GPS.hour);
   }
   if (SHOW_DEBUG &&(millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))){
      queueSizeTimer=millis();
      Serial.print("savingBuff Size = ");
      Serial.println((*savingBuff).size());

      Serial.print("sdBuff Size = ");
      Serial.println((*sdBuff).size());
   }

}
