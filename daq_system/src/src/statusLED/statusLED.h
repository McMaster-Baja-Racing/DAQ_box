#include <cstdint>
#include <Adafruit_NeoPixel.h>

// Definitions
#define STATUS_LED_COUNT 4
#define STATUS_LED_STRIP_PIN 2  // Digital Pin 6 for STATUS LED's

const uint8_t GPSLEDIndex = 3;
const uint8_t FileLEDIndex = 2;

enum GPS_STATUS {
  GPS_STATUS_DISABLED,
  GPS_STATUS_NO_FIX,
  GPS_STATUS_HAS_FIX
};

enum FILE_STATUS {
  FILE_STATUS_NO_SD,
  FILE_STATUS_WAITING_FOR_GPS,
  FILE_STATUS_WRITING_SEQUENTIAL,
  FILE_STATUS_WRITING_DATETIME
};

// Constants
const uint8_t GPSStatusColor[][3] = {
  {255, 0, 0},      // Red 
  {255, 255, 0},    // Yellow 
  {0, 255, 0},      // Green
};

const uint8_t FileStatusColor[][3] = {
  {255, 0, 0},      // Red 
  {255, 255, 0},    // Yellow 
  {0, 0, 255},      // Blue
  {0, 255, 0}       // Green
};

void updateFileStatus(FILE_STATUS status);
void updateGPSStatus(GPS_STATUS status);
void initializeStatusLED();