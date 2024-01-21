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
//#include "Adafruit_MCP9808.h"
// For Push`s
#include <Bounce.h>

// Settings
#define SHOW_DEBUG false
bool EN_BATT = true;
bool EN_HUD = true;
bool EN_GPS = true;
bool EN_RPM = true;
bool EN_TEMP = true; //uses prim_temp connector on PCB
bool EN_BRAKE = true;
bool EN_IMU = false;
bool EN_STRAIN1 = false;
bool EN_STRAIN2 = false;
bool EN_SUS1 = true;
bool EN_SUS2 = true;
bool EN_SUS3 = true;
bool EN_SUS4 = true;
bool EN_SEROUT= false;
bool USE_SD = true;







// HUD
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS  50    // Max brightness = 255

enum HUD
{
  PRIM,
  SEC,
  BRAKE,
  GPS_S,
  BATT_PERCENT,
  STRAIN,
  SUS1,
  SUS2,  
  HUD_MODES
};

int butColour [] [3]={
{255,0,0},//red = prim
{0,255,0},//green = sec
{0,0,255},//blue = brake
{255,255,255},//white = spd
{138,43,226},//,purple = battery percent
{255,182,193},//,light pink = strain1
{170,51,106},//,dark pink = sus1
{150,74,0}};//brown = sus2

int HUD_SHOW =BRAKE;


int brake_pres=0.0;

// General

//Interval between each data collection point, this is where you set data logging rate
//These values are in ms not hz

#define BATT_INTERVAL 10000
#define IMU_INTERVAL 10 // ms
#define GPS_INTERVAL 100 // ms 
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)
#define SD_INTERVAL 25
#define QUEUE_SIZE_INTERVAL 1000//for debugging purposes, shows you current queue length on serial moniter
#define BRAKE_INTERVAL 10//ms
#define STRAIN_INTERVAL 1
#define SUS_INTERVAL 5
#define TEMP_INTERVAL 500 //500

// RPM Sensors

//Threshold for how many magnets until an rpm is recorded, as this number increase noise reduces but data can be missed
//In 2023 we came to the conclusion that noise is okay and we can filter the data very well in post using the s-golay filter
#define HALL_THRESH 1

// PINS
#define VOLT_PIN  14
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define SEC_HALL_PIN 4
#define PRIM_HALL_PIN 5
#define STATUS_PIN 28
#define HUD_PIN    9      // Digital Pin 6 for HUD LED's
#define BUTT_PIN 29
#define TEMPERATURE_PIN 16
int strainPin []={20,21,22,23,41,40};
int susPin[] = {24,25,26,27};




//GPS 
#define GPSSerial Serial2 

/***  Start of Global variables  ***/
/***********************************/

//Object Definitions
Adafruit_GPS GPS(&GPSSerial);
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

//Timers
unsigned long battTimer = millis();
unsigned long tempTimer = millis();
unsigned long ledTimer = millis();
unsigned long queueSizeTimer = millis();
unsigned long rpmTimer=millis();
unsigned long brakeTimer=millis();
unsigned long imuTimer=millis();
unsigned long sdTimer=millis();
unsigned long strainTimer1=millis();
unsigned long susTimer1=millis();
unsigned long strainTimer2=millis();
unsigned long susTimer2=millis();
unsigned long susTimer3=millis();
unsigned long susTimer4=millis();
uint32_t gpsTimer = millis();

//GPS
bool use_gps = false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;
bool gps_active = false;

uint16_t GPS_year=2023;
uint8_t GPS_month=1;
uint8_t GPS_day=1;
uint8_t GPS_hour=1; 
uint8_t GPS_minute=30; 
uint8_t GPS_seconds=25;

float gps_speed=0;

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
int Prim_counts_per_rotation=4;

unsigned long SEC_start = micros();
unsigned long SEC_end_time = micros();
unsigned long SEC_past_time = micros();
bool SEC_stopped = false;
float SEC_hall_count = 0;
int SEC_rpm = 0;
int SEC_counts_per_rotation=3;

//strain data
int strain [6];

//suspension travel data
int sus1;
int sus2;
int sus3;
int sus4;

//temperature data
int temperature;

//Status LED that is connected to the outside of the box
bool statusLED=0;

/* DEPRICATED FOR WHEN WE USED AN I2C TEMP SENSOR. WE NOW USE THE LM35 WITH A VOLTAGE DIVIDER
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
*/

//Battery
float batVoltage = 0;
int batPercent = 0;

//Gyro
imu::Vector<3> accel;
imu::Vector<3> gyro;
imu::Vector<3> gravity;
sensors_event_t event;

int imuAccelCal=false;
int imuGyroCal=false;



//button bouncer
Bounce inputButton = Bounce(BUTT_PIN,5);
int lastPressed=0;


// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "/00000000.bin";
char directory[] = "/00-00-00";
char fileDir [23];
const int chipSelect = BUILTIN_SDCARD;

File bajaData;
