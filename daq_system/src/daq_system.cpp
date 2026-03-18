#include "TimeLib.h"
#include "daq_system_Teensy4.h"
#include "src/RPM/rpm.h"
#include "src/datastruct/dataTypes.h"
#include "src/debug/debug.h"
#include "src/fileInformation/file.h"
#include "src/hud/hud.h"
#include "src/sdCard/sdCard.h"
#include "src/fileInformation/file.h"
#include "src/gps/gps.h"



void setup() {
    Serial.begin(115200);

    Serial.println("Setup Starting");


 

    // Set up LED Strip
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.show();
    delay(50);

    pinMode(REAR_SPEED_HALL_PIN, INPUT);
    pinMode(PRIM_HALL_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(REAR_SPEED_HALL_PIN),
                    incrementHall_REAR_SPEED, RISING);
    attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_PRIM,
                    RISING);

    pinMode(STATUS_PIN, OUTPUT);

    delay(1000);

    // Setup section
    // If the sensor fails to initialize, the status LED will be red
    // If the sensor initializes successfully, the status LED will be green

    strip.show();
    // delay(500);

       //new gps setup using ublox:
    GPSSerial.begin(38400); //defaults to 38400 on uart
    if(!myGNSS.begin(GPSSerial)){
        strip.setPixelColor(2, strip.Color(255, 0, 0));
        Serial.println("GPS failed or not present");
        use_gps = false; //is false by default in daq_system_Teensy4.h but idk.

    } else {
        strip.setPixelColor(2, strip.Color(0, 255, 0));
        Serial.println("GPS detected");
        myGNSS.setUART1Output(COM_TYPE_UBX); // Set the UART port to output UBX only, no nmea noise (was a problem maybe?)
        myGNSS.setNavigationFrequency(10); // the gps runs 10hz
        myGNSS.saveConfiguration(); // Save the current settings to flash and BBR

        use_gps = true;

    }

    // SD Card Setup
    if (USE_SD) {
        if (EN_FAST_SD) {
            // note, DMA_SDIO was slower, but might be useful in the future
            if (!SD.sdfs.begin(SdioConfig(FIFO_SDIO))) {
                Serial.println("SD using FIFO failed, falling back to slow SD");
                EN_FAST_SD = false;
            } else {
                Serial.println("SD using DMA");
                Serial.println("CARD SUCCESS");
                // strip.setPixelColor(1, strip.Color(0, 255, 0));
                // EN_FAST_SD = false; // Disable fast SD for now due to
                // stability issues
            }
        } else if (!SD.begin(chipSelect)) {
            Serial.println("Card failed, or not present");
            // strip.setPixelColor(1, strip.Color(100, 0, 0));
            strip.show();
            USE_SD = false;
        } else {
            Serial.println("CARD SUCCESS");
            // strip.setPixelColor(1, strip.Color(0, 100, 0));
        }
        strip.show();
        delay(500);
    }

    SdFile::dateTimeCallback(dateTime); 

    Serial.println("Setup Finished");
    digitalWrite(STATUS_PIN, LOW);

    // WRITE TO SD CARD
    Serial.println("Creating filename:");
    uint16_t GPS_year = 2000;
    uint16_t GPS_month = 1;
    uint16_t GPS_day = 1;
    uint16_t GPS_hour = 1;
    uint16_t GPS_minute = 1;
    uint16_t GPS_seconds = 1;

    getFilename(GPS_hour, GPS_minute, GPS_seconds);
    getDirectory(GPS_day, GPS_month, GPS_year);

    SD.mkdir(directory);
    Serial.println(filename);
    Serial.println(directory);
    strcpy(fileDir, directory);
    strcat(fileDir, filename);
    
    Serial.println(fileDir);

    if (EN_FAST_SD) {
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

    gps_active = true; //forces a dummy file name always even if the gps off
    delay(1000);
}

void loop() {
    //-------------GPS-------------------
    if (use_gps) {
        gps();
    }


    controlDebug(Debug::RPM_DEBUG);
    //-------------RPM Calculations-------------------
    rpmCalc();

    if (SHOW_DEBUG && (millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))) {
        queueSizeTimer = millis();
        Serial.print("savingBuff Size = ");
        Serial.println((*savingBuff).size());

        Serial.print("sdBuff Size = ");
        Serial.println((*sdBuff).size());
    }

    if (millis() - sdTimer > (SD_INTERVAL - 1)) {
        sdTimer = millis();
        sdSend();
    }
}
