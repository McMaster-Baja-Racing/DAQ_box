#include "GPSHeader.h"
#include "GlobalVars.h"
#include "TimeLib.h"

// Define the GPS object
Adafruit_GPS GPS(&GPSSerial);

// Define the global variables
bool use_gps = false;
bool gps_flash = true;
bool gps_timesend = false;
bool gps_goodmessage = false;
bool gps_active = false;

uint16_t GPS_year = 2023;
uint8_t GPS_month = 1;
uint8_t GPS_day = 1;
uint8_t GPS_hour = 1; 
uint8_t GPS_minute = 30; 
uint8_t GPS_seconds = 25;

float gps_speed = 0;

void dateTime(uint16_t* date, uint16_t* time) {

  uint16_t year;
  uint8_t month, day, hour, minute, second;
  // User gets date and time from GPS or real-time clock here
  if (GPS.fix) {
    year = GPS_year;
    month = GPS_month;
    day = GPS_day;
    hour = GPS_hour;
    minute = GPS_minute;
    second = GPS_seconds;
  } else {
    year = 2030;
    month = 3;
    day = 1;
    hour = 13;
    minute = 2;
    second = 50;
  }
  // Return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year, month, day);
  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
}
