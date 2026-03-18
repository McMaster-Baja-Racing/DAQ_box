#include "gps.h"
#include <cstdio>
#include <SD.h>
//#include <Adafruit_GPS.h>
#include "../sdCard/sdCard.h"
#include "../fileInformation/file.h"
#include "../hud/hud.h"
#include "../datastruct/dataTypes.h"

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;
//serial pins on 7/8




// Initializations

//Adafruit_GPS GPS = Adafruit_GPS(&GPSSerial);


uint8_t GPS_month = 1;
uint8_t GPS_day = 1;
uint8_t GPS_hour = 1; 
uint8_t GPS_minute = 30; 
uint8_t GPS_seconds = 25;

uint16_t GPS_year = 2026;

uint32_t gpsTimer = millis();


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

  if(myGNSS.getFixType() >= 2) {
    year = myGNSS.getYear();
    month = myGNSS.getMonth();
    day = myGNSS.getDay();
    hour = myGNSS.getHour();
    minute = myGNSS.getMinute();
    second = myGNSS.getSecond();
    timezoneAdjust(year, month, day, hour);
    
  } else { //default if no fix.
    year = 2030; month = 3; day = 1;
    hour = 13; minute = 0; second = 0;
  }

  *date = FAT_DATE(year, month, day);
  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
}

// TODO: Cleanup this function
void handleGPS() {
  if(millis() - gpsTimer > (GPS_INTERVAL -1)){
    gpsTimer = millis();
    gps_timesend = true;

    bool hasFix = myGNSS.getFixType() >= 2; // 2D or better fix

    if(hasFix){
      if(USE_SD && gps_active == false){ 
        GPS_year = myGNSS.getYear();
        GPS_month = myGNSS.getMonth();
        GPS_day = myGNSS.getDay();
        GPS_hour = myGNSS.getHour();
        GPS_minute = myGNSS.getMinute();
        GPS_seconds = myGNSS.getSecond();

        timezoneAdjust(GPS_year, GPS_month, GPS_day, GPS_hour);
        getFilename(GPS_hour, GPS_minute, GPS_seconds);
        getDirectory(GPS_day, GPS_month, GPS_year);
        SD.mkdir(directory);
        Serial.println(filename);
        Serial.println(directory);
        strcpy(fileDir, directory);
        strcat(fileDir, filename);
        Serial.println(fileDir);

        if(EN_FAST_SD){
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
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.setPixelColor(1, strip.Color(0, 0, 0));
        strip.setPixelColor(2, strip.Color(0, 0, 0));
        strip.setPixelColor(3, strip.Color(255, 0, 0));
        strip.show();
        Serial.println("fix found, recording starting");

        digitalWrite(STATUS_PIN, HIGH);
        delay(1000);
        statusLED = true;
      }
      gps_active = true;

    } else if (EN_GPS) {
      for (int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, gps_flash ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0));
      }
      gps_flash = !gps_flash;
      strip.show();
    }
  }
  
}

void gpsData(){
    if (EN_GPS && gps_timesend && myGNSS.getFixType() >= 2) {
      long lat = myGNSS.getLatitude(); 
      long lon = myGNSS.getLongitude();



      buffPush(GPS_LATITUDE, (float)(lat/10000000.0));
      buffPush(GPS_LAT, (unsigned long)(lat >= 0 ? 'N' : 'S'));

      buffPush(GPS_LONGITUTE, (float)(lon/10000000.0));
      buffPush(GPS_LON, (unsigned long)(lon >= 0 ? 'E' : 'W'));

      buffPush(GPS_ANGLE, (float)myGNSS.getHeading()/10000000.0);

      // GPS speed is in knots
      gps_speed = myGNSS.getGroundSpeed() * 0.0036; //mm/s to km/h
      buffPush(GPS_SPEED, gps_speed);

      uint8_t mo = myGNSS.getMonth();
      uint8_t da = myGNSS.getDay();
      uint16_t ye = myGNSS.getYear() % 100; //last 2 digits of year
      uint8_t ho = myGNSS.getHour();
      uint8_t mi = myGNSS.getMinute();
      uint8_t se = myGNSS.getSecond();

      buffPush(GPS_DAYMONTHYEAR, (unsigned long)((da << 16) + (mo << 8) + (ye)));
      buffPush(GPS_SECONDMINUTEHOUR, (unsigned long)((se << 16) + (mi << 8) + (ho)));

    if (mi != GPS_minute) {
      GPS_year = myGNSS.getYear();
      GPS_month = mo;
      GPS_day = da;
      GPS_hour = ho;
      GPS_minute = mi;
      GPS_seconds = se;
    }
    gps_timesend = false;
  }
}

void gps(){
  myGNSS.checkUblox(); // Continuously check for new GPS data.
  handleGPS();
  gpsData();
}

