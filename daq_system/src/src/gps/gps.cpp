#include "gps.h"

#include <SD.h>

#include <cstdio>

#include "../config.h"
#include "../datastruct/dataTypes.h"
#include "../fileInformation/file.h"
#include "../hud/hud.h"
#include "../sdCard/sdCard.h"
#include "../statusLED/statusLED.h"
SFE_UBLOX_GNSS_SERIAL myGNSS;
// serial pins on 7/8

// Initializations
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

float gps_speed = 0.0f;

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
    if (ENABLE_PROFILING) {
        Serial.printf("%d: dateTime\n", millis());
    }
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
    uint8_t fixType = myGNSS.getFixType();

    GPS_year = myGNSS.getYear();
    GPS_month = myGNSS.getMonth();
    GPS_day = myGNSS.getDay();
    GPS_hour = myGNSS.getHour();
    GPS_minute = myGNSS.getMinute();
    GPS_seconds = myGNSS.getSecond();

    gps_timesend = true;
    if (ENABLE_PROFILING) {
        Serial.printf("\t\t%d: getFixType\n", millis());
    }
    Serial.println();
    bool hasFix = (fixType == 2 || fixType == 3);  // 2D or better fix
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

            if (ENABLE_PROFILING) {
                Serial.printf("\t\t%d: timezoneAdjust\n", millis());
            }
            timezoneAdjust(GPS_year, GPS_month, GPS_day, GPS_hour);
            Serial.println("GPS fix found");

            digitalWrite(STATUS_PIN, HIGH);
            delay(1000);
            statusLED = true;
        }
        gps_active = true;
    }
}

void gpsData() {
    if (EN_GPS && gps_timesend && gps_active) {
        float lat = (float)myGNSS.getLatitude() * 0.0000001;
        float lon = (float)myGNSS.getLongitude() * 0.0000001;
        float heading = (float)myGNSS.getHeading() * 0.00001;
        // speed in mm/s to km/h
        gps_speed = myGNSS.getGroundSpeed() * 0.0036;

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

        buffPush(GPS_LATITUDE, lat);
        buffPush(GPS_LAT, (unsigned long)(lat >= 0 ? 'N' : 'S'));

        buffPush(GPS_LONGITUTE, lon);
        buffPush(GPS_LON, (unsigned long)(lon >= 0 ? 'E' : 'W'));

        buffPush(GPS_ANGLE, (heading));

        // GPS speed is in mm/s, convert to km/h

        buffPush(GPS_SPEED, gps_speed);

        buffPush(GPS_DAYMONTHYEAR,
                 (unsigned long)((GPS_day << 16) + (GPS_month << 8) + (GPS_year % 100)));
        buffPush(GPS_SECONDMINUTEHOUR,
                 (unsigned long)((GPS_seconds << 16) + (GPS_minute << 8) + (GPS_hour)));
        gps_timesend = false;
    }
}

void gps() {
    if (millis() - gpsTimer < (GPS_INTERVAL - 1)) {
        // Wait for the next interval
        return;
    }

    gpsTimer = millis();

    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: getPVT\n", millis());
    }
    bool newInfo = myGNSS.getPVT();  // Continuously check for new GPS data.
    Serial.println(newInfo ? "New GPS data available" : "No new GPS data");
    if(!newInfo) {
        return;
    }
    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: handleGPS\n", millis());
    }
    handleGPS();
    if (ENABLE_PROFILING) {
        Serial.printf("\t%d: gpsData\n", millis());
    }
    gpsData();
}
