//Libraries
#include <SD.h>

/***  Settings  ***/

#define SHOW_DEBUG false
bool EN_TEMP = false;  // Uses prim_temp connector on PCB
bool EN_BRAKE = false;
bool EN_IMU = false;
bool EN_STRAIN1 = false; // On for Sheave position
bool EN_STRAIN2 = false;
bool EN_SUS1 = false;
bool EN_SUS2 = false;
bool EN_SUS3 = false;
bool EN_SUS4 = false;

/***  General  ***/
// Interval between each data collection point, this is where you set data logging rate
// These values are in ms not hz
//TO DO: Add comments explaining why interval values are set to what they are

#define IMU_INTERVAL 10
#define SD_INTERVAL 25
#define QUEUE_SIZE_INTERVAL 1000  // For debugging purposes, shows you current queue length on serial moniter
#define BRAKE_INTERVAL 10
#define STRAIN_INTERVAL 10
#define SUS_INTERVAL 5
#define TEMP_INTERVAL 500

// Time until the program stops waiting for GPS fix and writes to default filename
#define GPS_TIMEOUT_MILLIS (15 * 1000)

/***  Pins  ***/
#define FR_HALL_PIN 2
#define FL_HALL_PIN 3
#define REAR_SPEED_HALL_PIN 27
#define PRIM_HALL_PIN 26

/***********************************/
/***  Start of Global variables  ***/
/***********************************/

//Timers
unsigned long tempTimer = millis();
unsigned long queueSizeTimer = millis();
unsigned long brakeTimer = millis();
unsigned long imuTimer = millis();
unsigned long sdTimer = millis();
unsigned long strainTimer1 = millis();
unsigned long susTimer1 = millis();
unsigned long strainTimer2 = millis();
unsigned long susTimer2 = millis();
unsigned long susTimer3 = millis();
unsigned long susTimer4 = millis();

const int chipSelect = BUILTIN_SDCARD;

//Timezone
int8_t TIMEZONE_OFFSET = -4; //UTC -4 for EDT