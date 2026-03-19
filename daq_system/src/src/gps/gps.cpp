#include "gps.h"

#include <SD.h>

#include <cstdio>
// #include <Adafruit_GPS.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>  //http://librarymanager/All#SparkFun_u-blox_GNSS

#include "../config.h"
#include "../datastruct/dataTypes.h"
#include "../fileInformation/file.h"
#include "../hud/hud.h"
#include "../sdCard/sdCard.h"
#include "../statusLED/statusLED.h"
SFE_UBLOX_GNSS myGNSS;
// serial pins on 7/8

// Initializations

// Adafruit_GPS GPS = Adafruit_GPS(&GPSSerial);

uint8_t GPS_month = 1;
uint8_t GPS_day = 1;
uint8_t GPS_hour = 1;
uint8_t GPS_minute = 30;
uint8_t GPS_seconds = 25;

uint16_t GPS_year = 2026;

uint32_t gpsTimer = millis();

bool gps_timesend = false;
bool EN_GPS = true;
bool gps_active = false;

float gps_speed = 0;

void timezoneAdjust(uint16_t& year, uint8_t& month, uint8_t& day,
                    uint8_t& hour) {
    // Make a signed copy for calculation
    int16_t signedHour = (int16_t)hour;
    signedHour += TIMEZONE_OFFSET;  // negative offsets are safe now

    // Days in each month
    uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        daysInMonth[1] = 29;

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
        day += daysInMonth[month - 1];
    }

    // Write the adjusted hour back to the original unsigned variable
    hour = (uint8_t)signedHour;
}

// Function Definitions
void dateTime(uint16_t* date, uint16_t* time) {
    uint16_t year;
    uint8_t month, day, hour, minute, second;

    if (gps_active) {
        year = myGNSS.packetUBXNAVPVT->data.year;
        month = myGNSS.packetUBXNAVPVT->data.month;
        day = myGNSS.packetUBXNAVPVT->data.day;
        hour = myGNSS.packetUBXNAVPVT->data.hour;
        minute = myGNSS.packetUBXNAVPVT->data.min;
        second = myGNSS.packetUBXNAVPVT->data.sec;
        timezoneAdjust(year, month, day, hour);

    } else {  // default if no fix.
        year = 2030;
        month = 3;
        day = 1;
        hour = 13;
        minute = 0;
        second = 0;
    }

    *date = FAT_DATE(year, month, day);
    // Return time using FAT_TIME macro to format fields
    *time = FAT_TIME(hour, minute, second);
}

// TODO: Cleanup this function
void handleGPS() {
    if (ENABLE_PROFILING) {
        Serial.printf("\t\t%d: check interval\n", millis());
    }
    if (millis() - gpsTimer > (GPS_INTERVAL - 1)) {
        gpsTimer = millis();
        gps_timesend = true;
        if (ENABLE_PROFILING) {
            Serial.printf("\t\t%d: getFixType\n", millis());
        }
        Serial.println(myGNSS.packetUBXNAVPVT->data.fixType);
        bool hasFix = (myGNSS.packetUBXNAVPVT->data.fixType == 2 ||
                       myGNSS.packetUBXNAVPVT->data.fixType == 3);  // 2D or better fix
        if (ENABLE_PROFILING) {
            Serial.printf("\t\t%d: if hasFix\n", millis());
        }
        if (hasFix) {
            if (gps_active == false) {
                if (ENABLE_PROFILING) {
                    Serial.printf("\t\t%d: updateGPSStatus\n", millis());
                }
                updateGPSStatus(GPS_STATUS_HAS_FIX);
                if (ENABLE_PROFILING) {
                    Serial.printf("\t\t%d: getting date\n", millis());
                }
                GPS_year = myGNSS.packetUBXNAVPVT->data.year;
                GPS_month = myGNSS.packetUBXNAVPVT->data.month;
                GPS_day = myGNSS.packetUBXNAVPVT->data.day;
                GPS_hour = myGNSS.packetUBXNAVPVT->data.hour;
                GPS_minute = myGNSS.packetUBXNAVPVT->data.min;
                GPS_seconds = myGNSS.packetUBXNAVPVT->data.sec;
                if (ENABLE_PROFILING) {
                    Serial.printf("\t\t%d: timezoneAdjust\n", millis());
                }
                timezoneAdjust(GPS_year, GPS_month, GPS_day, GPS_hour);
                Serial.println("GPS fix found");
                Serial.println(myGNSS.packetUBXNAVPVT->data.fixType);

                digitalWrite(STATUS_PIN, HIGH);
                delay(1000);
                statusLED = true;
            }
            gps_active = true;
        }
    }
}

void gpsData() {
    if (EN_GPS && gps_timesend && gps_active) {
        float lat = (float)myGNSS.packetUBXNAVPVT->data.lat * 0.0000001;
        float lon = (float)myGNSS.packetUBXNAVPVT->data.lon * 0.0000001;

        // Convert latitude and longitude to DDMM.MMMM format because that's
        // what the old library did And that's what the bin parser library
        // expects
        int lat_deg = (int)lat;
        float lat_min = (lat - lat_deg) * 60.0;
        lat = lat_deg * 100 + lat_min;

        int lon_deg = (int)lon;
        float lon_min = (lon - lon_deg) * 60.0;
        lon = lon_deg * 100 + lon_min;
        if (lon < 0) {
            lon = -lon;
        }

        float heading = (float)myGNSS.packetUBXNAVPVT->data.headVeh * 0.00001;

        buffPush(GPS_LATITUDE, lat);
        buffPush(GPS_LAT, (unsigned long)(lat >= 0 ? 'N' : 'S'));

        buffPush(GPS_LONGITUTE, lon);
        buffPush(GPS_LON, (unsigned long)(lon >= 0 ? 'E' : 'W'));

        buffPush(GPS_ANGLE, (heading));

        // GPS speed is in mm/s, convert to km/h
        gps_speed = myGNSS.packetUBXNAVPVT->data.gSpeed * 0.0036;  // mm/s to km/h
        buffPush(GPS_SPEED, gps_speed);

        uint8_t mo = myGNSS.packetUBXNAVPVT->data.month;
        uint8_t da = myGNSS.packetUBXNAVPVT->data.day;
        uint16_t ye = myGNSS.packetUBXNAVPVT->data.year % 100;  // last 2 digits of year
        uint8_t ho = myGNSS.packetUBXNAVPVT->data.hour;
        uint8_t mi = myGNSS.packetUBXNAVPVT->data.min;
        uint8_t se = myGNSS.packetUBXNAVPVT->data.sec;

        buffPush(GPS_DAYMONTHYEAR,
                 (unsigned long)((da << 16) + (mo << 8) + (ye)));
        buffPush(GPS_SECONDMINUTEHOUR,
                 (unsigned long)((se << 16) + (mi << 8) + (ho)));

        if (mi != GPS_minute) {
            GPS_year = myGNSS.packetUBXNAVPVT->data.year;
            GPS_month = mo;
            GPS_day = da;
            GPS_hour = ho;
            GPS_minute = mi;
            GPS_seconds = se;
        }
        gps_timesend = false;
    }
}

void gps() {
    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: getPVT\n", millis());
    }
    bool newInfo = myGNSS.getPVT();  // Continuously check for new GPS data.
    Serial.println(newInfo ? "New GPS data available" : "No new GPS data");
    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: handleGPS\n", millis());
    }
    handleGPS();
    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: gpsData\n", millis());
    }
    gpsData();
}
