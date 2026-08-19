#include <cstdint>
//#include <Adafruit_GPS.h>
#include <Adafruit_NeoPixel.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include "TimeLib.h"

// Definitions
#define GPSSerial Serial2 
#define GPS_INTERVAL 100 // 40 ms 25hz, use 100 for 10 hz.

// Declarations
//extern Adafruit_GPS GPS;
extern SFE_UBLOX_GNSS_SERIAL myGNSS;

extern int8_t TIMEZONE_OFFSET;
extern uint8_t GPS_month;
extern uint8_t GPS_day;
extern uint8_t GPS_hour;
extern uint8_t GPS_minute;
extern uint8_t GPS_seconds;

extern uint16_t GPS_year;

extern uint32_t gpsTimer;

//extern bool gps_goodmessage;
extern bool gps_timesend;
extern bool EN_GPS;
extern bool gps_active;

extern float gps_speed;

// Function Declarations
void timezoneAdjust(uint16_t &year, uint8_t &month, uint8_t &day, uint8_t &hour);

void dateTime(uint16_t* date, uint16_t* time);

void gpsPVTCallback(UBX_NAV_PVT_data_t *pvt);

//void gpsMessage();

void handleGPS();

void gpsData();

void gps();