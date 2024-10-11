#include <Adafruit_NeoPixel.h>
#ifndef HUDHEADER_H
#define HUDHEADER_H

// Function Declarations
void setColour(int8_t edge);

// Variables
#define LED_COUNT  10  // 2 x 5 LED strip
#define BRIGHTNESS  50  // Max brightness = 255.

// Object Declarations
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);

// HUD Modes
enum HUD
{
  PRIM,
  SEC,
  BRAKE,
  GPS_S,
  BATT_PERCENT,
  STRAIN,
  SUS1,
  SUS2,  
  HUD_MODES
};

int HUD_SHOW = BRAKE;
int brake_pres = 0.0;


// HUD Colours
int butColour [] [3] = {
  {255,0,0},  // Red = prim
  {0,255,0},  // Green = sec
  {0,0,255},  // Blue = brake
  {255,255,255},  // White = spd
  {138,43,226},  // Purple = battery percent
  {255,182,193},  // Light pink = strain1
  {170,51,106},  // Dark pink = sus1
  {150,74,0}  // Brown = sus2
};  

#endif