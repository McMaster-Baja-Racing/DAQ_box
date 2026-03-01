#include "file.h"
#include <cstdio>
#include "../HUD/hud.h"
#include "../sdCard/sdCard.h"
#include "../gps/gps.h"

// Initializations
char filename [] = "/00000000.bin";
char directory[] = "/00-00-00";
char fileDir [23];

Bounce inputButton = Bounce(BUTT_PIN,5);
int lastPressed = 0;

// Function Definitions
void getFilename(uint8_t hour, uint8_t minute, uint8_t second) {
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "/%02d%02d%02d.bin", hour, minute, second);
}

void getDirectory(uint8_t day, uint8_t month, uint8_t year) {
  // Please move files before start of new day
  sprintf(directory, "/%02d-%02d-%02d", year, month, day);
}

void handleInputButton() {
  inputButton.update();

  if (inputButton.fallingEdge()) {
    Serial.print("Button Pressed: ");

    if (gps_active) {
      HUD_SHOW = (HUD_SHOW + 1) % HUD_MODES; // Cycle through the HUD modes
      Serial.print(" Mode = "); 
      Serial.print(HUD_SHOW);
    } else {

      if (millis() - lastPressed < 250) {
        Serial.println("Start recording");
        //strip.setPixelColor(0, strip.Color(100, 100, 100));
        EN_GPS = false;
        gps_active = true;

        SD.mkdir(directory);
        Serial.println(filename);
        Serial.println(directory);

        strcpy(fileDir, directory);
        strcat(fileDir, filename);
        Serial.println(fileDir);

        if (EN_FAST_SD) {
          bajaDataFast.open(&SD.sdfs, fileDir, O_RDWR | O_CREAT | O_AT_END);
          if (!bajaDataFast) {
            Serial.println("File failed to write");
            USE_SD = false;
          }
        } else {
          bajaData = SD.open(fileDir, FILE_WRITE);
          if (bajaData == 0) {
            Serial.println("File failed to write");
            USE_SD = false;
          }
        }

        // File header example
        // bajaData.println("Time, Absolute X, Absolute Y, Absolute Z, Accel X, Accel Y, Accel Z, Gravity X, Gravity Y, Gravity Z, Gyro X, Gyro Y, Gyro Z, IMU Temp, HasGPS, Latitude (DDMM.MMMMM), Longitude (DDDMM.MMMMM)(will remove leading zeros), Angle (North is 0 and CW)), Speed (knots), Date + Time, Primary Temp i2c, Secondary Temp i2c, Suspension Travel, Strain, FR_RPM, REAR_SPEED, Battery Percentage, Battery Voltage");
        
        gps_flash = true;
        
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 255, 0));
        }

        Serial.print("GPS cancelled");
        digitalWrite(STATUS_PIN, HIGH);
        strip.show();
        delay(1000);
        statusLED = true;
      }
      lastPressed = millis();
    }
    Serial.println();
  }
}