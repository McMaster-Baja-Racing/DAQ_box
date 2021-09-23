/**************************************************************************
  Author: DAQ Team
  Created on: 16/10/2020
**************************************************************************/
#include <Wire.h>
// For IMU
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
// For GPS
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
// For display
#include <Adafruit_NeoPixel.h>
// For SD Card
#include <SPI.h>
#include <SD.h>

// HUD
#define LED_PIN    6      // Digital Pin 6 for LED's
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS 25     // Max brightness = 255
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)

// BOX
#define BATT_INTERVAL 1000
#define IMU_INTERVAL 200 // ms
#define GPS_INTERVAL 1000 // ms (Should be multiple of IMU_INTERVAL)
#define HALL_THRESH 10
#define POT A2
#define VOLT_PIN  A6
#define RELAY_PIN 4
#define HALL_PIN 5

/***  Start of Global variables  ***/
/***********************************/
//SoftwareSerial GPSSerial(8, 7); // GPS uses software serial and can't send to board
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

unsigned long battTimer = millis();
unsigned long ledTimer = millis();
unsigned long imuTimer = millis();
unsigned long start = micros();
unsigned long end_time = micros();
unsigned long past_time = micros();
uint32_t gpsTimer = millis();

bool gps_timesend = false;
bool gps_goodmessage = false;
bool send_data = false;
bool gps_active = false;
float hall_count = 0;
bool on_state = false;
uint16_t rpm = 2600;

const int chipSelect = 4;

#define USE_SD

/***  End of Global variables  ***/
/*********************************/

// Declare function for setting pixel colours
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)

void setup() {
  //Set up serial
  Serial.begin(115200);
  while(!Serial){delay(10);} // Wait until Serial is ready

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);

  // Set up imu
  if(!bno.begin()) {
    // There was a problem detecting the BNO055 ... check your connections
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

#ifdef USE_SD
  // SD Card Setup
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  if (SD.exists("data.txt")) {
    SD.remove("data.txt"); // Delete old file
  }
  File bajaData = SD.open("data.txt", FILE_WRITE); // Create file
  bajaData.close();
#endif

  // Set up led strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  delay(50);

  
  // Set up GPS
  GPS.begin(9600);
  // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  delay(1000);
  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
}

void loop() {
  int jump = 99;
  int rpm = 2600;

  
//-------------Hall Effect Sensor--------------------
//  
//  // counting number of times the hall sensor is tripped
//  // but without double counting during the same trip
//  
//  if (digitalRead(HALL_PIN)==1){
//    if (on_state==false){
//      on_state = true;
//      hall_count += 1;
//    }
//  } else{
//    on_state = false;
//  }
//  
//  if (hall_count > HALL_THRESH){
//        // print information about Time and RPM
//    end_time = micros();
//    past_time = (end_time-start);
////    Serial.print("Time Passed: ");
////    Serial.print(time_passed);
////    Serial.println("s");
//    //rpm = (hall_count*1000000/past_time)*60;
//    //rpm = map(analogRead(POT)*100, 0, 500, 1400, 4000);
//    //Serial.println(" ");
//    //Serial.print(rpm);
//    //Serial.print(" RPM");
//    hall_count = 0;
//    start = micros();
//  }


//-------------Battery Check---------------
  if (millis() - battTimer > BATT_INTERVAL){
    battTimer = millis();
    if (analogRead(VOLT_PIN) >= 6.0){
      digitalWrite(RELAY_PIN, HIGH);
    }
    else{
      digitalWrite(RELAY_PIN, LOW);
    }
  }


//-------------LED Strip---------------------
  if (millis() - ledTimer > LED_INTERVAL){
    ledTimer = millis();
    
    if( rpm > 3800 )                       {setColour(9);}
    else if( (rpm > 3600) && (rpm < 3800) ){setColour(8);}
    else if( (rpm > 3400) && (rpm < 3600) ){setColour(7);}
    else if( (rpm > 3200) && (rpm < 3400) ){setColour(6);}
    else if( (rpm > 3000) && (rpm < 3200) ){setColour(5);}
    else if( (rpm > 2800) && (rpm < 3000) ){setColour(4);}
    else if( (rpm > 2600) && (rpm < 2800) ){setColour(3);}
    else if( (rpm > 2400) && (rpm < 2600) ){setColour(2);}
    else if( (rpm > 2100) && (rpm < 2400) ){setColour(1);}
    else if( (rpm > 1800) && (rpm < 2100) ){setColour(0);}
    else if( rpm < 1800 )                  {setColour(-1);}
    else                                   {setColour(-1);}  // error/default
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

    if (!GPS.parse(GPS.lastNMEA())){  // this also sets the newNMEAreceived() flag to false
      gps_goodmessage = false;
    }else{
      gps_goodmessage = true;
    }
  }

  if (millis() - gpsTimer > (GPS_INTERVAL-1)){
    gpsTimer = millis();
    gps_timesend = true;
    if(GPS.fix){
      gps_active = true;
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
  if (millis() - imuTimer > (IMU_INTERVAL-1)){
    // Format
    // Absolute orientation is euler vector (also can output quaternion for if needed)
    // Acceleration includes gravity (use VECTOR_LINEARACCEL to exclude) and is shows the Three axis of acceleration in (m/s^2)
    // Gyro is an Angular Velocity Vector measured in (rad/s)
    // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z

    imuTimer = millis(); // Get time
    bno.getEvent(&event); 
    accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

    send_data = true;
  }


//-----------------Send Data--------------------------------------

  String dataString = "";
  if (send_data && gps_active){
    // Set up SD Card reader
#ifdef USE_SD
    File bajaData = SD.open("data.txt", FILE_WRITE);
#else
    bool bajaData = true;
#endif
    if (bajaData){
      // IMU Format
      // Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gyro X, Gyro Y, Gyro Z
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
  
      // GPS Format
      // Please read NMEA documentation to understand
      // Note that Fix is true or false and GPS Quality Indicator works like this: 0 - fix not available, 1 - GPS fix, 2 - Differential GPS fix
      // We get signals from GPGGA or GPRMC
      // Many values have to be converted to be usable (most software wants DD, not DMM)
      // GPS has its own timer, but due to the need of synchronous input, we have to accept some time inaccuracy and just use the imu timer
      // HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Altitude (m)(Ellipsoid), Speed (knots), Angle (North is 0 and CW), Satellites
      if(gps_timesend && gps_goodmessage && GPS.fix){
        dataString += (int)GPS.fix; dataString += F(",");
        dataString += String(GPS.latitude,4); dataString += String(GPS.lat); dataString += F(",");
        dataString += String(GPS.longitude,4); dataString += String(GPS.lon); dataString += F(",");
        dataString += String(GPS.angle); dataString += F(",");
        dataString += String(GPS.speed); dataString += F(",");
        dataString += String(GPS.altitude); dataString += F(",");
        dataString += String((int)GPS.satellites);
        gps_timesend = false;
      }else{
        //Not GPS placeholder
        dataString += (int)GPS.fix;
        dataString += F(",");
        dataString += "-1,-1,-1,-1,-1,-1";
      }
#ifdef USE_SD
      bajaData.println(dataString);
      bajaData.close();
#else
      Serial.println(dataString);
#endif
    }
    send_data = false;
  }
}


void setColour(int8_t edge)
{
  // Set all setColour to off/0
  strip.clear();

  const uint8_t R[10] = {255, 255, 255, 255, 255, 255, 100,   0,   0,  75};
  const uint8_t G[10] = {  0,  80, 150, 200, 200, 235, 255, 255,   0,   0};
  const uint8_t B[10] = {  0,   0,   0,   0,   0,   0,   0,   0, 255, 255};
  
  //Set pixel colour up to strip[edge]
  for(uint8_t i=0; i<=edge; i++){
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
}
