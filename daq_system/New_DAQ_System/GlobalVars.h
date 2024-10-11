//Libraries
#include <Wire.h>

#include <SPI.h>
#include <SD.h>

//For Temp Sensors
//#include "Adafruit_MCP9808.h"

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
bool EN_IMU = false;
bool EN_STRAIN1 = false;
bool EN_STRAIN2 = false;
bool EN_SUS1 = true;
bool EN_SUS2 = true;
bool EN_SUS3 = true;
bool EN_SUS4 = true;
bool EN_SEROUT= false;
bool USE_SD = true;

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

/***  Pins  ***/
#define VOLT_PIN  14
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define SEC_HALL_PIN 4
#define PRIM_HALL_PIN 5
#define STATUS_PIN 28
#define HUD_PIN 9  // Digital Pin 6 for HUD LED's
#define BUTT_PIN 29
#define TEMPERATURE_PIN 16
int strainPin [] = {20,21,22,23,41,40};
int susPin [] = {24,25,26,27};

/***********************************/
/***  Start of Global variables  ***/
/***********************************/

/***  Timers  ***/ 
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

/***  Temperature Data  ***/ 
int temperature;

/***  Status LED that is connected to the outside of the box  ***/ 
bool statusLED = 0;

/***  Battery  ***/ 
float batVoltage = 0;
int batPercent = 0;

/***  Button Bouncer  ***/ 
Bounce inputButton = Bounce(BUTT_PIN,5);
int lastPressed = 0;