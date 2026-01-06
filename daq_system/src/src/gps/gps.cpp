#include "gps.h"
#include <cstdio>
#include <SD.h>
#include <Adafruit_GPS.h>
#include "../sdCard/sdCard.h"
#include "../fileInformation/file.h"
#include "../hud/hud.h"
#include "../datastruct/dataTypes.h"


// Initializations
Adafruit_GPS GPS = Adafruit_GPS(&GPSSerial);

uint8_t GPS_month = 1;
uint8_t GPS_day = 1;
uint8_t GPS_hour = 1; 
uint8_t GPS_minute = 30; 
uint8_t GPS_seconds = 25;

uint16_t GPS_year = 2025;

uint32_t gpsTimer = millis();

bool gps_goodmessage = false;
bool gps_timesend = false;
bool gps_flash = true;
bool EN_GPS = true;

float gps_speed = 0;

void timezoneAdjust(uint16_t &year, uint8_t &month, uint8_t &day, uint8_t &hour) {
    // Make a signed copy for calculation
    int16_t signedHour = (int16_t)hour;  
    signedHour += TIMEZONE_OFFSET; // negative offsets are safe now

    // Days in each month
    uint8_t daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) daysInMonth[1] = 29;

    // Backward rollover only (hour < 0 → previous day)
    while (signedHour < 0) {
        signedHour += 24;
        day--;
    }

    // Handle day < 1 → previous month/year
    while (day < 1) {
        month--;
        if (month < 1) {
            month = 12;
            year--;
        }
        day += daysInMonth[month-1];
    }

    // Write the adjusted hour back to the original unsigned variable
    hour = (uint8_t)signedHour;
}

// Function Definitions
void dateTime(uint16_t* date, uint16_t* time) {

  uint16_t year;
  uint8_t month, day, hour, minute, second;

  if (GPS.fix) {
    // Return date using FAT_DATE macro to format fields
  //GPS year is returned as the last 2 digits only, I wonder how long this has been a bug lol
    year = GPS_year + 2000;
    month = GPS_month;
    day = GPS_day;
    hour = GPS_hour;
    minute = GPS_minute;
    second = GPS_seconds;

    timezoneAdjust(year, month, day, hour);
  } else {
    year = 2030;
    month = 3;
    day = 1;
    hour = 13;
    minute = 2;
    second = 50;
  }
  
  *date = FAT_DATE(year, month, day);
  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
  Serial.println("DateTime set to: " + String(year) + "-" + String(month) + "-" + String(day) + " " + String(hour) + ":" + String(minute) + ":" + String(second));
}

void gpsMessage(){
  if (GPS.newNMEAreceived()) {  
    if (!GPS.parse(GPS.lastNMEA())) {  // this also sets the newNMEAreceived() flag to false
      gps_goodmessage = false;
    } else {
      gps_goodmessage = true;
    }
  }
}

// TODO: Cleanup this function
void handleGPS() {
  if (millis() - gpsTimer > (GPS_INTERVAL - 1)) {
    gpsTimer = millis();
    gps_timesend = true;
    if (GPS.fix) {
      if (USE_SD) {
        if (gps_active == false) {
          GPS_year = GPS.year;
          GPS_year = GPS.year;
          GPS_month = GPS.month;
          GPS_day = GPS.day;
          GPS_hour = GPS.hour;
          GPS_minute = GPS.minute;
          GPS_seconds = GPS.seconds;
          getFilename(GPS.hour, GPS.minute, GPS.seconds);
          getDirectory(GPS.day, GPS.month, GPS.year);
          SD.mkdir(directory);
          Serial.println(filename);
          Serial.println(directory);
          strcpy(fileDir, directory);
          strcat(fileDir, filename);
          Serial.println(fileDir);

          if(EN_FAST_SD) {
            bajaDataFast.open(fileDir, O_RDWR | O_CREAT | O_AT_END);  // Create file
            if (!bajaDataFast) {
              Serial.println("File failed to write");
              // don't do anything more:
              USE_SD = false;
            }
            bajaDataFast.sync();
          } else {
            bajaData = SD.open(fileDir, FILE_WRITE);  // Create file
            if (bajaData == 0) {
              Serial.println("File failed to write");
              // don't do anything more:
              USE_SD = false;
            }
          }
          gps_flash = true;
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
          }
          Serial.println("Fix Found Recording Starting");
          digitalWrite(STATUS_PIN, HIGH);
          strip.show();
          delay(1000);
          statusLED = true;
        }
      }
      gps_active = true;

    } else if (EN_GPS) {
      if (gps_flash == true) {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(255, 0, 0));
        }
        gps_flash = false;
      } else {
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        gps_flash = true;
      }

      strip.show();
    }
  }
}

void gpsData(){
    if (EN_GPS && gps_timesend && gps_goodmessage && GPS.fix) {

    buffPush(GPS_LATITUDE, GPS.latitude);
    buffPush(GPS_LAT, (unsigned long)GPS.lat);

    buffPush(GPS_LONGITUTE, GPS.longitude);
    buffPush(GPS_LON, (unsigned long)GPS.lon);

    buffPush(GPS_ANGLE, GPS.angle);

    // GPS speed is in knots
    gps_speed = GPS.speed * 1.852;
    buffPush(GPS_SPEED, gps_speed);
    buffPush(GPS_DAYMONTHYEAR, (unsigned long)((GPS.day << 16) + (GPS.month << 8) + (GPS.year)));
    buffPush(GPS_SECONDMINUTEHOUR, (unsigned long)((GPS.seconds << 16) + (GPS.minute << 8) + (GPS.hour)));
    if (GPS.minute != GPS_minute) {
      GPS_year = GPS.year;
      GPS_year = GPS.year;
      GPS_month = GPS.month;
      GPS_day = GPS.day;
      GPS_hour = GPS.hour;
      GPS_minute = GPS.minute;
      GPS_seconds = GPS.seconds;
    }
    gps_timesend = false;
  }
}

void gps(){
  gpsMessage();
  handleGPS();
  gpsData();
}