#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include "hud.h"

Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);

int HUD_SHOW = BRAKE;

void setColour(int8_t edge) {
  // Set all setColour to off/0
  strip.clear();
  
  // Set pixel colour up to strip[edge]
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
  strip.setPixelColor(9, strip.Color(butColour[HUD_SHOW][0], butColour[HUD_SHOW][1], butColour[HUD_SHOW][2]));
}