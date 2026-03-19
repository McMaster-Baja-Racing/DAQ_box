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
#include "src/statusLED/statusLED.h"
#include "src/config.h"

void setup() {
    Serial.begin(115200);

    Serial.println("Setup Starting");

    // Set up LED Strip
    initializeStatusLED();
    delay(50);

    pinMode(REAR_SPEED_HALL_PIN, INPUT);
    pinMode(PRIM_HALL_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(REAR_SPEED_HALL_PIN),
                    incrementHall_REAR_SPEED, RISING);
    attachInterrupt(digitalPinToInterrupt(PRIM_HALL_PIN), incrementHall_PRIM,
                    RISING);

    pinMode(STATUS_PIN, OUTPUT);

    delay(1000);

    // new gps setup using ublox:
    GPSSerial.begin(38400);  // defaults to 38400 on uart
    if (!myGNSS.begin(GPSSerial)) {
        updateGPSStatus(GPS_STATUS_DISABLED);
        Serial.println("GPS failed or not present");
        EN_GPS = false;

    } else {
        updateGPSStatus(GPS_STATUS_NO_FIX);
        Serial.println("GPS serial started");
        myGNSS.setUART1Output(
            COM_TYPE_UBX);  // Set the UART port to output UBX only, no nmea
                            // noise (was a problem maybe?)
        myGNSS.setNavigationFrequency(10);  // the gps runs 10hz
        myGNSS.setAutoPVT(true);  // automatic PVT updates (lat, lon, speed, datetime)
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
                updateFileStatus(FILE_STATUS_WAITING_FOR_GPS);
            }
        }

        if (!EN_FAST_SD) {
            if (!SD.begin(chipSelect)) {
                Serial.println("Card failed, or not present");
                updateFileStatus(FILE_STATUS_NO_SD);
                USE_SD = false;
            } else {
                SdFile::dateTimeCallback(dateTime);
                Serial.println("CARD SUCCESS");
                updateFileStatus(FILE_STATUS_WAITING_FOR_GPS);
            }
        }
        delay(500);
    }

    Serial.println("Setup Finished");
    digitalWrite(STATUS_PIN, LOW);

    delay(1000);
}

void loop() {
    if (ENABLE_PROFILING) {
        Serial.printf("%d: New loop\n", millis());
    }
    //-------------Debug Function-------------------
    controlDebug(Debug::NONE);

    if (ENABLE_PROFILING) {
        Serial.printf("%d: handleInputButton\n", millis());
    }
    //-------------Handle Input Button----------------
    handleInputButton();

    //-------------RPM Calculations-------------------
    if (ENABLE_PROFILING) {
        Serial.printf("%d: rpmCalc\n", millis());
    }
    rpmCalc();

    //-------------Battery Check---------------
    if (ENABLE_PROFILING) {
        Serial.printf("%d: batteryCheck\n", millis());
    }
    batteryCheck();

    //-------------LED Strip---------------------
    if (ENABLE_PROFILING) {
        Serial.printf("%d: setHUD\n", millis());
    }
    setHUD();

    //-------------GPS Data---------------------
    if (ENABLE_PROFILING) {
        Serial.printf("%d: gps\n", millis());
    }
    gps();

    if (ENABLE_PROFILING) {
        Serial.printf("%d: misc sensors\n", millis());
    }

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

    if (ENABLE_PROFILING) {
        Serial.printf("%d: file creation\n", millis());
    }

    // If we are using SD card but haven't created the data file yet
    if (USE_SD && !(bajaDataFast || bajaData)) {
        // If the GPS timeout has been reached
        if (!gps_active && millis() > GPS_TIMEOUT_MILLIS) {
            Serial.println("GPS Timed out, creating sequential file");
            createSequentialFile();
            updateFileStatus(FILE_STATUS_WRITING_SEQUENTIAL);
        } else if (gps_active) {
            Serial.println("GPS Acquired, creating datetime file");
            createDateTimeFile(GPS_day, GPS_month, GPS_year, GPS_hour,
                               GPS_minute, GPS_seconds);
            updateFileStatus(FILE_STATUS_WRITING_DATETIME);
        }
    }

    if (ENABLE_PROFILING) {
        Serial.printf("%d: debug prints\n", millis());
    }

    if (SHOW_DEBUG && (millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))) {
        queueSizeTimer = millis();
        Serial.print("savingBuff Size = ");
        Serial.println((*savingBuff).size());

        Serial.print("sdBuff Size = ");
        Serial.println((*sdBuff).size());
    }

    if (ENABLE_PROFILING) {
        Serial.printf("%d: sdSend\n", millis());
    }
    if (millis() - sdTimer > (SD_INTERVAL - 1)) {
        sdTimer = millis();
        sdSend();
    }
}
