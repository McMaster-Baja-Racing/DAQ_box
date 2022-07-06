/************************************************************************
  Author: DAQ Team
  Created on: 16/10/2020
**************************************************************************/



/*
 * 
 * This version of the code includes using the 2 i2c temperature sesors
 * 
 */

//TODO
//CHANGE DIRECTORY BASED ON CURRENT DAY
//SECOND IMU
//Implement status LEDS
  //Recording LED
  //If the battery is low LAST LED goes Red 
//more RPM sensors on inturrepts



#include <Wire.h>
// For IMU
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
// For GPS
#include <Adafruit_GPS.h>
// For display
#include <Adafruit_NeoPixel.h>
// For SD Card
#include <SPI.h>
#include <SD.h>
//FOr Temp Sensors
#include "Adafruit_MCP9808.h"

// HUD
#define LED_PIN    6      // Digital Pin 6 for LED's
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS 25     // Max brightness = 255
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)

// BOX

#define BATT_INTERVAL 400
#define IMU_INTERVAL 10 // ms
#define GPS_INTERVAL 100 // ms (Should be multiple of IMU_INTERVAL)
#define TEMP_INTERVAL 400 // ms (Should be multiple of IMU_INTERVAL)
#define HALL_THRESH 4
#define POT A2
#define VOLT_PIN  A3
#define RELAY_PIN 4
#define HALL_PIN 21

#define PRIMARYTEMP_PIN A0
#define SECONDARYTEMP_PIN A1



/***  Start of Global variables  ***/
/***********************************/
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
Adafruit_MCP9808 primaryTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 secondaryTempSensor = Adafruit_MCP9808();


unsigned long battTimer = millis();
unsigned long tempTimer = millis();
unsigned long ledTimer = millis();
unsigned long imuTimer = millis();
unsigned long start = micros();
unsigned long end_time = micros();
unsigned long past_time = micros();
uint32_t gpsTimer = millis();

bool use_gps=false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;
bool send_data = false;
bool gps_active = false;
float hall_count = 0;
bool on_state = false;
bool stopped = false;

int rpm=0;
float spd = 2600;
int spdTarget=40;

bool usePrimI2C = true;
bool useSecI2C = true;


float batVoltage = 0;
int batPercent=0;
float primaryTemp = 0;
float primaryTemp_basic=0;
float secondaryTemp = 0;
float secondaryTemp_basic=0;
int8_t boardTemp = 0;
int tempResolution=0;
  // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

const int chipSelect = 4;

// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "00000000.CSV";

bool USE_SD=true;

#define USE_GPS_SPEED
bool showRPM = false; //variable to choose whether the HUD shows RPM or SPEED, false for SPEED true for RPM.

/***  End of Global variables  ***/
/*********************************/

// Declare function for setting pixel colours
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)

void getFilename(uint8_t hour, uint8_t minute, uint8_t second)
{
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "%02d%02d%02d.CSV", hour, minute, second);
}

void setup() {
  //Set up serial
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait until Serial is ready
  }
  // Set up led strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  delay(50);


  //Setup analog reference votlage
  analogReference(INTERNAL4V3);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN),incrementHall,FALLING);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT_PULLUP);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);

  // Set up imu
  if (!bno.begin()) {
    // There was a problem detecting the BNO055 ... check your connections
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    strip.setPixelColor(0,strip.Color(255,0,0));
  }
  else{
    strip.setPixelColor(0,strip.Color(0,255,0));
  }
  strip.show();
  delay(500);
  //Set up temp sensors with adress corespondeing to the table below
  //  A2 A1 A0 address
  //  0  0  0   0x18  this is the default address
  //  0  0  1   0x19
  //  0  1  0   0x1A
  //  0  1  1   0x1B
  //  1  0  0   0x1C
  //  1  0  1   0x1D
  //  1  1  0   0x1E
  //  1  1  1   0x1F
  if (!primaryTempSensor.begin(0x18)) {
    Serial.println("Couldn't find primary MCP9808! Check your connections and verify the address is correct.");
    usePrimI2C=false;
    strip.setPixelColor(1,strip.Color(255,0,0));
  }
  else{
    strip.setPixelColor(1,strip.Color(0,255,0));
  }
  strip.show();
  delay(500);
  if (!secondaryTempSensor.begin(0x19)) {
    Serial.println("Couldn't find secondary MCP9808! Check your connections and verify the address is correct.");
    useSecI2C=false;
    strip.setPixelColor(2,strip.Color(255,0,0));
  }
  else{
    strip.setPixelColor(2,strip.Color(0,255,0));
  }
  strip.show();
  delay(500);
  // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms
  if (usePrimI2C){
  primaryTempSensor.setResolution(2);
  }
  if(useSecI2C){
    secondaryTempSensor.setResolution(2);
  }
  if(USE_SD){
  // SD Card Setup
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    strip.setPixelColor(3,strip.Color(255,0,0));
    strip.show();
    // don't do anything more:
    USE_SD=false;
  }
  strip.setPixelColor(3,strip.Color(0,255,0));
  strip.show();
  Serial.println("card initialized.");
  delay(500);
  }

  

  // Set up GPS
  if(!GPS.begin(9600)){
    strip.setPixelColor(4,strip.Color(255,0,0));
    use_gps=false;
  }
  else{
     strip.setPixelColor(4,strip.Color(0,255,0));
  }
  strip.show();
  // turn on turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); // 1 Hz update rate
 
  delay(1000);
  
}

void loop() {
  int jump = 99;
                                               

  //-------------Hall Effect Sensor--------------------

  // counting number of times the hall sensor is tripped
  // but without double counting during the same trip

  if (hall_count > HALL_THRESH) {

    // print information about Time and spd
    end_time = micros();
    past_time = (end_time - start);
    //    Serial.print("Time Passed: ");
    //    Serial.print(time_passed);
    //    Serial.println("s");
    if (stopped) {
      spd = 0;
      stopped = false;
    }
    else {
      spd=0.0157774892888*(1/(past_time/1000000.0))*60;
    }
    //spd = map(analogRead(POT)*100, 0, 500, 1400, 4000);
    //Serial.println(" ");
    //Serial.print(spd);
    //Serial.print(" spd");
    hall_count = 0;
    start = micros();
  }
  if (!stopped && (micros() - start >= 1000000)) {
    spd  = 0;
    stopped = true;
  }


  //-------------Battery Check---------------
  if (millis() - battTimer > BATT_INTERVAL) {
    battTimer = millis();
    batVoltage=analogRead(VOLT_PIN);
    batPercent=map(batVoltage,660,750,0,100);
    batVoltage=(((batVoltage/1024)*4.3)*(16))/6;
    
    if (batVoltage >= 6.0) {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else {
      digitalWrite(RELAY_PIN, LOW);
    }
  }


  //-------------LED Strip---------------------
  if (millis() - ledTimer > LED_INTERVAL) {
    ledTimer = millis();
    if (showRPM){
      int numLED=map(rpm,1800,3800,-1,9);
      if (numLED>9){w
        numLED=9;
      }
      setColour(numLED);
    }
    if (!showRPM){
      int numLED=0;
      numLED=map(spd,0,spdTarget,-1,9);
      if (numLED>9){
        numLED=9;
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
      if(USE_SD){
      if (gps_active == false) {
        getFilename(GPS.hour, GPS.minute, GPS.seconds);
        Serial.println(filename);
        if (SD.exists(filename)) {
          SD.remove(filename); // Delete old file
        }
        File bajaData = SD.open(filename, FILE_WRITE); // Create file
        if (bajaData == 0) {
          Serial.println("File failed to write");
          // don't do anything more:
          while (1);
        }
        bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time,Primary Temp i2c, Secondary Temp i2c, Primary Temp Basic, Secondary Temp Basic, Speed (Km/h),Battery Percentage, Battery Voltage");
        bajaData.close();
        gps_flash=true;
        for (int i = 0; i<LED_COUNT;i++){
          strip.setPixelColor(i,strip.Color(0, 255, 0));
        }
      }
      else{
        if (gps_flash=true){
          strip.setPixelColor(9,strip.Color(0, 255, 0));
          gps_flash=false;
        }
        else{
          strip.setPixelColor(9,strip.Color(0, 0, 0));
          gps_flash=true;
        }
        strip.show();
      }
      }
      gps_active = true;
      
    }
    else{
      if (gps_flash=true){
        for (int i = 0; i<LED_COUNT;i++){
          strip.setPixelColor(i,strip.Color(255, 0, 0));
        }
        gps_flash=false;
      }
      else{
        for (int i = 0; i<LED_COUNT;i++){
          strip.setPixelColor(i,strip.Color(0, 0, 0));
        }
        gps_flash=false;
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

  imu::Vector<3> accel;
  imu::Vector<3> gyro;
  sensors_event_t event;
  if (millis() - imuTimer > (IMU_INTERVAL - 1)) {
    // Format
    // Absolute orientation is euler vector (also can output quaternion for if needed)
    // Acceleration includes gravity (use VECTOR_LINEARACCEL to exclude) and is shows the Three axis of acceleration in (m/s^2)
    // Gyro is an Angular Velocity Vector measured in (rad/s)
    // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z

    imuTimer = millis(); // Get time
    bno.getEvent(&event);
    accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    boardTemp = bno.getTemp();
    send_data = true;
  }

  //-----------------TEMP--------------------------------------


  if (millis() - tempTimer > TEMP_INTERVAL) {
    if(usePrimI2C){
    primaryTemp = primaryTempSensor.readTempC();}
    if(usePrimI2C){
    secondaryTemp = secondaryTempSensor.readTempC();
    }
  }
  
  
  //15+\frac{\left(\frac{a}{1024}\cdot 4.3-\frac{870}{1024}\cdot 4.3\right)}{0.007390625}
  primaryTemp_basic=15+(analogRead(PRIMARYTEMP_PIN)/1024*4.3-3741/1024)/0.007390625;
  secondaryTemp_basic=15+(analogRead(SECONDARYTEMP_PIN)/1024*4.3-3741/1024)/0.007390625;

  //-----------------Send Data--------------------------------------

  String dataString = "";
  if (send_data && gps_active) {
    // Set up SD Card reader
    File bajaData;
    if(USE_SD){
    bajaData = SD.open(filename, FILE_WRITE);
    }
      // IMU Format
      // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z, IMU Temp
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
          spd=GPS.speed*1.852;
        #endif
        
        dataString += (int)GPS.fix; dataString += F(",");
        dataString += String(GPS.latitude, 4); dataString += String(GPS.lat); dataString += F(",");
        dataString += String(GPS.longitude, 4); dataString += String(GPS.lon); dataString += F(",");
        dataString += String(GPS.angle); dataString += F(",");
        dataString += String(GPS.speed); dataString += F(",");
        dataString += String(GPS.day) + "-" + String(GPS.month) + "-" + String(GPS.year) + " " + String(GPS.hour) + "-" + String(GPS.minute) + "-" + String(GPS.seconds)+" "+String(GPS.milliseconds); dataString += F(",");
        gps_timesend = false;
      } else {
        //Not GPS placeholder
        dataString += (int)GPS.fix;
        dataString += F(",");
        dataString += "-1,-1,-1,-1,-1,-1,-1,";
      }



      
      //Primary Temp i2c, Secondary Temp i2c, Primary Temp Basic, Secondary Temp Basic, Speed (Km/h),Battery Percentage, Battery Voltage
      dataString += String(primaryTemp); dataString += F(",");
      dataString += String(secondaryTemp); dataString += F(",");
      dataString += String(primaryTemp_basic); dataString += F(",");
      dataString += String(secondaryTemp_basic); dataString += F(",");
      dataString += String(spd);dataString += F(",");
      dataString += String(batPercent); dataString += F(",");
      dataString += String(batVoltage); 

   if(USE_SD){
      bajaData.println(dataString);
      bajaData.close();
   }
   else{
      Serial.println(dataString);
   }
    send_data = false;
  }
}

void incrementHall(){
  hall_count += 1;
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
