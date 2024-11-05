#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include "hud.h"

// Initializations
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);

int HUD_SHOW = BRAKE;

// Function Definitions
void setColour(int8_t edge) {
  strip.clear();
  
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
  strip.setPixelColor(9, strip.Color(butColour[HUD_SHOW][0], butColour[HUD_SHOW][1], butColour[HUD_SHOW][2]));
}