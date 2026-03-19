#include "statusLED.h"

Adafruit_NeoPixel statusLEDStrip(STATUS_LED_COUNT, STATUS_LED_STRIP_PIN,
                                 NEO_GRB + NEO_KHZ800);

void initializeStatusLED() {
    Serial.println("Initializing status LED");
    statusLEDStrip.begin();
    statusLEDStrip.setBrightness(255);
    updateFileStatus(FILE_STATUS_NO_SD);
    updateGPSStatus(GPS_STATUS_DISABLED);
}

void updateFileStatus(FILE_STATUS status) {
    // TODO:LEDs are in order Green, Red, Blue somehow?
    statusLEDStrip.setPixelColor(
        FileLEDIndex, statusLEDStrip.Color(FileStatusColor[status][1],
                                           FileStatusColor[status][0],
                                           FileStatusColor[status][2]));
    statusLEDStrip.show();
}

void updateGPSStatus(GPS_STATUS status) {
    // TODO: LEDs are in order Green, Red, Blue somehow?
    statusLEDStrip.setPixelColor(
        GPSLEDIndex, statusLEDStrip.Color(GPSStatusColor[status][1],
                                          GPSStatusColor[status][0],
                                          GPSStatusColor[status][2]));
    statusLEDStrip.show();
}