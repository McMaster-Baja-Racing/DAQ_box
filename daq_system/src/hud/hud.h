#include <cstdint>
#include <Adafruit_NeoPixel.h>

// Definitions
#define LED_COUNT 10  // 2 x 5 LED strip
#define BRIGHTNESS 50  // Max brightness = 255
#define HUD_PIN 9  // Digital Pin 6 for HUD LED's
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)

enum HUD {
  PRIM,
  REAR_SPEED_HUD,
  BRAKE,
  GPS_S,
  BATT_PERCENT,
  STRAIN,
  SUS1,
  SUS2,  
  HUD_MODES
};

// Constants
const int butColour[][3] = {
  {255, 0, 0},      // Red = prim
  {0, 255, 0},      // Green = Rear Wheel Speed
  {0, 0, 255},      // Blue = brake
  {255, 255, 255},  // White = spd
  {138, 43, 226},   // Purple = battery percent
  {255, 182, 193},  // Light pink = strain1
  {170, 51, 106},   // Dark pink = sus1
  {150, 74, 0}      // Brown = sus2
};

const uint8_t R[10] = { 255, 255, 255, 255, 255, 255, 100, 0, 0, 75 };
const uint8_t G[10] = { 0, 80, 150, 200, 200, 235, 255, 255, 0, 0 };
const uint8_t B[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 255, 255 };

// Declarations
extern Adafruit_NeoPixel strip;
extern int HUD_SHOW;
extern int brake_pres;

extern bool EN_HUD;

extern unsigned long ledTimer;

// Function Declarations
void setColour(int8_t edge);

void setHUD();