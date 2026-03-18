#include "TimeLib.h"
#include "daq_system_Teensy4.h"
#include "src/RPM/rpm.h"
#include "src/battery/battery.h"
#include "src/datastruct/dataTypes.h"
#include "src/debug/debug.h"
#include "src/fileInformation/file.h"
#include "src/gps/gps.h"
#include "src/hud/hud.h"
#include "src/sdCard/sdCard.h"

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

    // new gps setup using ublox:
    GPSSerial.begin(38400);  // defaults to 38400 on uart
    if (!myGNSS.begin(GPSSerial)) {
        strip.setPixelColor(2, strip.Color(255, 0, 0));
        Serial.println("GPS failed or not present");
        EN_GPS = false;

    } else {
        strip.setPixelColor(2, strip.Color(0, 255, 0));
        Serial.println("GPS serial started");
        myGNSS.setUART1Output(
            COM_TYPE_UBX);  // Set the UART port to output UBX only, no nmea
                            // noise (was a problem maybe?)
        myGNSS.setNavigationFrequency(10);  // the gps runs 10hz
        myGNSS
            .saveConfiguration();  // Save the current settings to flash and BBR
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
            SdFile::dateTimeCallback(dateTime);
            Serial.println("CARD SUCCESS");
            // strip.setPixelColor(1, strip.Color(0, 100, 0));
        }
        strip.show();
        delay(500);
    }

    // Set up GPS
    strip.setPixelColor(0, strip.Color(50, 0, 0));
    strip.show();

    strip.show();

    Serial.println("Setup Finished");
    digitalWrite(STATUS_PIN, LOW);

    delay(1000);
}

void loop() {
    //-------------Debug Function-------------------
    controlDebug(Debug::NONE);

    //-------------Handle Input Button----------------
    handleInputButton();

    //-------------RPM Calculations-------------------
    rpmCalc();

    //-------------Battery Check---------------
    batteryCheck();

    //-------------LED Strip---------------------
    setHUD();

    //-------------GPS Data---------------------
    gps();

    if (EN_BRAKE && millis() - brakeTimer > (BRAKE_INTERVAL - 1)) {
        brakeTimer = millis();
        brake_pres = ((((analogRead(17) / 1024.0) * 5.15625) - 0.4) * 1250);
        // buffPush(BRAKE_PRESS, (float)(brake_pres));
    }

    if (EN_IMU && millis() - imuTimer > (IMU_INTERVAL - 1)) {
        imuTimer = millis();
        // imuData();
    }

    if (EN_STRAIN1 && millis() - strainTimer1 > (STRAIN_INTERVAL - 1)) {
        strainTimer1 = millis();
        // Uncomment when this works
        // sheavePos();
    }

    if (EN_STRAIN2 && millis() - strainTimer2 > (STRAIN_INTERVAL - 1)) {
        strainTimer2 = millis();
        // strainData(3);
    }

    if (EN_SUS1 && millis() - susTimer1 > (SUS_INTERVAL - 1)) {
        susTimer1 = millis();
        // susData1();
    }

    if (EN_SUS2 && millis() - susTimer2 > (SUS_INTERVAL - 1)) {
        susTimer2 = millis();
        // susData2();
    }

    if (EN_SUS3 && millis() - susTimer3 > (SUS_INTERVAL - 1)) {
        susTimer3 = millis();
        // susData3();
    }

    if (EN_SUS4 && millis() - susTimer4 > (SUS_INTERVAL - 1)) {
        susTimer4 = millis();
        // susData4();
    }

    if (EN_TEMP && millis() - tempTimer > (TEMP_INTERVAL - 1)) {
        tempTimer = millis();
        // tempData();
    }

    // If we are using SD card but haven't created the data file yet
    if (USE_SD && !(bajaDataFast || bajaData)) {
        // If the GPS timeout has been reached
        if (!gps_active && millis() > GPS_TIMEOUT_MILLIS) {
            Serial.println("GPS Timed out, creating sequential file");
            createSequentialFile();
        } else if (gps_active) {
            Serial.println("GPS Acquired, creating datetime file");
            createDateTimeFile(GPS_day, GPS_month, GPS_year, GPS_hour,
                               GPS_minute, GPS_seconds);
        }
    }

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
