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

// For SD CardFs
#include <SPI.h>
#include <SD.h>

// For Push`s
#include <Bounce.h>

/***  Settings  ***/

#define SHOW_DEBUG false
bool EN_BATT = true;
bool EN_HUD = true;
bool EN_GPS = true;
bool EN_RPM = true;
bool EN_TEMP = true;  // Uses prim_temp connector on PCB
bool EN_BRAKE = true;
bool EN_IMU = true;
bool EN_STRAIN1 = false;
bool EN_STRAIN2 = false;
bool EN_SUS1 = true;
bool EN_SUS2 = true;
bool EN_SUS3 = true;
bool EN_SUS4 = true;

/***  HUD  ***/
#define LED_COUNT  10  // 2 x 5 LED strip
#define BRIGHTNESS  50  // Max brightness = 255

int brake_pres = 0.0;

/***  General  ***/
// Interval between each data collection point, this is where you set data logging rate
// These values are in ms not hz

#define BATT_INTERVAL 10000
#define IMU_INTERVAL 10
#define GPS_INTERVAL 100 
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)
#define SD_INTERVAL 25
#define QUEUE_SIZE_INTERVAL 1000  // For debugging purposes, shows you current queue length on serial moniter
#define BRAKE_INTERVAL 10
#define STRAIN_INTERVAL 1
#define SUS_INTERVAL 5
#define TEMP_INTERVAL 500

/***  RPM Sensors  ***/

// Threshold for how many magnets until an rpm is recorded, as this number increase noise reduces but data can be missed
// In 2023 we came to the conclusion that noise is okay and we can filter the data very well in post using the s-golay filter
#define HALL_THRESH 1

/***  Pins  ***/
#define VOLT_PIN  14
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define SEC_HALL_PIN 4
#define PRIM_HALL_PIN 5
#define BUTT_PIN 29

/***********************************/
/***  Start of Global variables  ***/
/***********************************/

//Timers
unsigned long battTimer = millis();
unsigned long tempTimer = millis();
unsigned long ledTimer = millis();
unsigned long queueSizeTimer = millis();
unsigned long rpmTimer = millis();
unsigned long brakeTimer = millis();
unsigned long imuTimer = millis();
unsigned long sdTimer = millis();
unsigned long strainTimer1 = millis();
unsigned long susTimer1 = millis();
unsigned long strainTimer2 = millis();
unsigned long susTimer2 = millis();
unsigned long susTimer3 = millis();
unsigned long susTimer4 = millis();
uint32_t gpsTimer = millis();

/***  GPS  ***/ 
bool use_gps = false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;

float gps_speed = 0;

/***  SD Card  ***/ 
bool send_data = false;
SdFs sd;
FsFile file;

/***  RPM & Speed  ***/ 
unsigned long FR_start = micros();
unsigned long FR_end_time = micros();
unsigned long FR_past_time = micros();
bool FR_stopped = false;
int FR_rpm = 0;

unsigned long FL_start = micros();
unsigned long FL_end_time = micros();
unsigned long FL_past_time = micros();
bool FL_stopped = false;
int FL_rpm = 0;

unsigned long PRIM_start = micros();
unsigned long PRIM_end_time = micros();
unsigned long PRIM_past_time = micros();
bool PRIM_stopped = false;
int PRIM_rpm = 0;
int Prim_counts_per_rotation = 4;

unsigned long SEC_start = micros();
unsigned long SEC_end_time = micros();
unsigned long SEC_past_time = micros();
bool SEC_stopped = false;
int SEC_rpm = 0;
int SEC_counts_per_rotation = 3;

/***  Status LED that is connected to the outside of the box  ***/ 

/***  Battery  ***/ 
float batVoltage = 0;
int batPercent = 0;

int imuAccelCal = false;
int imuGyroCal = false;

/***  Button Bouncer  ***/ 
Bounce inputButton = Bounce(BUTT_PIN,5);
int lastPressed = 0;

// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char fileDir [23];
const int chipSelect = BUILTIN_SDCARD;