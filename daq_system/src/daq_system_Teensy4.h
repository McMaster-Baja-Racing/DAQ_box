//Libraries
#include <SD.h>

/***  Settings  ***/


/***  General  ***/
// Interval between each data collection point, this is where you set data logging rate
// These values are in ms not hz
//TO DO: Add comments explaining why interval values are set to what they are

#define IMU_INTERVAL 10
#define SD_INTERVAL 25
#define QUEUE_SIZE_INTERVAL 1000  // For debugging purposes, shows you current queue length on serial moniter
#define BRAKE_INTERVAL 10
#define STRAIN_INTERVAL 10
#define TEMP_INTERVAL 500

/***  Pins  ***/
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define REAR_SPEED_HALL_PIN 26
#define PRIM_HALL_PIN 27

/***********************************/
/***  Start of Global variables  ***/
/***********************************/

//Timers
unsigned long queueSizeTimer = millis();
unsigned long sdTimer = millis();

/***  GPS  ***/ 
bool use_gps = false;

const int chipSelect = BUILTIN_SDCARD;