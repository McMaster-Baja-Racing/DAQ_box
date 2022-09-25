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
#define SHOW_DEBUG false

// HUD
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS 25     // Max brightness = 255


// General
#define BATT_INTERVAL 10000
#define IMU_INTERVAL 10 // ms
#define GPS_INTERVAL 200 // ms (Should be multiple of IMU_INTERVAL)
#define TEMP_INTERVAL 400 // ms (Should be multiple of IMU_INTERVAL)
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)
#define SD_INTERVAL 1000
#define QUEUE_SIZE_INTERVAL 1000

// RPM Sensors
#define HALL_THRESH 3

// PINS
#define VOLT_PIN  14
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define SEC_HALL_PIN 5
#define PRIM_HALL_PIN 4
#define STATUS_PIN 28
#define HUD_PIN    9      // Digital Pin 6 for HUD LED's

#define SD_CS_PIN 4

int strainPin []={20,21,22,23,41,40};
int susPins[] = {24,25,26,27};

//GPS 
#define GPSSerial Serial2

/***  Start of Global variables  ***/
/***********************************/

//Object Definitions
Adafruit_GPS GPS(&GPSSerial);
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

IntervalTimer strainTimer;
IntervalTimer imuTimer;
IntervalTimer susTimer;
IntervalTimer sdTimer;

//Timers
unsigned long battTimer = millis();
unsigned long tempTimer = millis();
unsigned long ledTimer = millis();
unsigned long queueSizeTimer = millis();


uint32_t gpsTimer = millis();

//GPS
bool use_gps = false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;
bool gps_active = false;

//SD CARD
bool send_data = false;
SdFs sd;
FsFile file;

//RPM&SPEED
unsigned long FR_start = micros();
unsigned long FR_end_time = micros();
unsigned long FR_past_time = micros();
bool FR_stopped = false;
float FR_hall_count = 0;
int FR_rpm = 0;

unsigned long FL_start = micros();
unsigned long FL_end_time = micros();
unsigned long FL_past_time = micros();
bool FL_stopped = false;
float FL_hall_count = 0;
int FL_rpm = 0;

unsigned long PRIM_start = micros();
unsigned long PRIM_end_time = micros();
unsigned long PRIM_past_time = micros();
bool PRIM_stopped = false;
float PRIM_hall_count = 0;
int PRIM_rpm = 0;

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


float maxSize=0;


// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "/00000000.CSV";
char directory[] = "/00-00-00";
char fileDir [23];
const int chipSelect = BUILTIN_SDCARD;

File bajaData;
bool USE_SD = true;
