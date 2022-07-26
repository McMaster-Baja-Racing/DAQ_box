//Libraries
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

// Settings
#define USE_GPS_SPEED //if spd is equal to gps speed or RPM speed
#define showRPM false //variable to choose whether the HUD shows RPM or SPEED, false for SPEED true for RPM.

// HUD
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS 25     // Max brightness = 255


// General
#define BATT_INTERVAL 400
#define IMU_INTERVAL 10 // ms
#define GPS_INTERVAL 200 // ms (Should be multiple of IMU_INTERVAL)
#define TEMP_INTERVAL 400 // ms (Should be multiple of IMU_INTERVAL)
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)
#define SD_INTERVAL 1000

// RPM Sensors
#define HALL_THRESH 4

// PINS
#define VOLT_PIN  A3
#define FR_HALL_PIN 5
#define SEC_HALL_PIN 7
#define STATUS_PIN 13
#define HUD_PIN    6      // Digital Pin 6 for HUD LED's
#define SUS_PIN A1
#define STRAIN_PIN A2
#define SD_CS_PIN 4

//GPS 
#define GPSSerial Serial1

//Temperature Sensors
#define PrimaryTempAdress 0x18
#define SecondaryTempAdress 0x19
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

/***  Start of Global variables  ***/
/***********************************/

//Object Definitions
Adafruit_GPS GPS(&GPSSerial);
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
Adafruit_MCP9808 primaryTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 secondaryTempSensor = Adafruit_MCP9808();

//Timers
unsigned long battTimer = millis();
unsigned long tempTimer = millis();
unsigned long ledTimer = millis();
unsigned long imuTimer = millis();
unsigned long sdTimer = millis();


uint32_t gpsTimer = millis();

//GPS
bool use_gps = false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;
bool gps_active = false;

//SD CARD
bool send_data = false;
File bajaData;

//RPM&SPEED
unsigned long FR_start = micros();
unsigned long FR_end_time = micros();
unsigned long FR_past_time = micros();
bool FR_stopped = false;
float FR_hall_count = 0;
int FR_rpm = 0;

unsigned long SEC_start = micros();
unsigned long SEC_end_time = micros();
unsigned long SEC_past_time = micros();
bool SEC_stopped = false;
float SEC_hall_count = 0;
int SEC_rpm = 0;

int Sus_Travel=0;
int STRAIN=0;

bool statusLED=0;

//Temperature Settings
bool usePrimI2C = true;
bool useSecI2C = true;
float primaryTemp = 0;
float secondaryTemp = 0;
int8_t boardTemp = 0;
int tempResolution = 0;
// sets the resolution mode of reading, the modes are defined in the table bellow:
// Mode Resolution SampleTime
//  0    0.5°C       30 ms
//  1    0.25°C      65 ms
//  2    0.125°C     130 ms
//  3    0.0625°C    250 ms

//Battery
float batVoltage = 0;
int batPercent = 0;

//Gyro
imu::Vector<3> accel;
imu::Vector<3> gyro;
imu::Vector<3> gravity;
sensors_event_t event;




// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "/00000000.CSV";
char directory[] = "/00-00-00";
char fileDir [23];

bool USE_SD = true;
