#include "HUDHeader.h"
#include "../GlobalVars.h"

void setColour(int8_t edge) {
  // Set all setColour to off/0
  strip.clear();

  const uint8_t R[10] = { 255, 255, 255, 255, 255, 255, 100, 0, 0, 75 };
  const uint8_t G[10] = { 0, 80, 150, 200, 200, 235, 255, 255, 0, 0 };
  const uint8_t B[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 255, 255 };

  // Set pixel colour up to strip[edge]
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
  strip.setPixelColor(9, strip.Color(butColour[HUD_SHOW][0], butColour[HUD_SHOW][1], butColour[HUD_SHOW][2]));
}

