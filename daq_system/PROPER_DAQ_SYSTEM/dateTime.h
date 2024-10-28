#include <cstdint>
#include <Adafruit_GPS.h>
#include <Adafruit_NeoPixel.h>
#include "TimeLib.h"


#define GPSSerial Serial2 

void dateTime(uint16_t* date, uint16_t* time);

extern Adafruit_GPS GPS;

extern uint16_t GPS_year;
extern uint8_t GPS_month;
extern uint8_t GPS_day;
extern uint8_t GPS_hour;
extern uint8_t GPS_minute;
extern uint8_t GPS_seconds;