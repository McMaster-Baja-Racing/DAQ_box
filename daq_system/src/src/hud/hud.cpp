#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include "hud.h"
#include "../sdCard/sdCard.h"
#include "../RPM/rpm.h"
#include "../suspensionData/sus.h"
#include "../gps/gps.h"

// Initializations
Adafruit_NeoPixel strip(LED_COUNT, HUD_PIN, NEO_GRB + NEO_KHZ800);

int HUD_SHOW = BRAKE;
int brake_pres = 0.0;

bool EN_HUD = true;

unsigned long ledTimer = millis();


// Function Definitions
void setColour(int8_t edge) {
  strip.clear();
  
  for (uint8_t i = 0; i <= edge; i++) {
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
  strip.setPixelColor(9, strip.Color(butColour[HUD_SHOW][0], butColour[HUD_SHOW][1], butColour[HUD_SHOW][2]));
}

void setHUD() {
  if (EN_HUD && millis() - ledTimer > LED_INTERVAL) {
    ledTimer = millis();
    if (gps_active) {
      int numLED = 0;
      switch (HUD_SHOW) {
        case PRIM:
          numLED = map(PRIM_rpm, 1700, 3800, -1, 8);
          break;
        case REAR_SPEED_HUD:
          numLED = map(REAR_SPEED_int, 0, 5000, -1, 8);
          break;
        case BRAKE:
          numLED = map(brake_pres, 0, 1200, -1, 8);
          break;
        case GPS_S:
          numLED = map(gps_speed, 5, 45, -1, 8);
          break;
          //TODO: fix this once battery check is merged in
        // case BATT_PERCENT:
        //   numLED = map(batPercent, 0, 100, -1, 8);
        //   break;
        case STRAIN:
          // Placeholder until strain data is available
          numLED = map(0, 0, 1000, -1, 8);
          break;
        case SUS1:
          numLED = map(sus1, 140, 310, -1, 8);
          break;
        case SUS2:
          numLED = map(sus2, 50, 312, -1, 8);
          break;
      }
      if (numLED > 8) {
            numLED = 8;
        }
      setColour(numLED);
    }
    strip.show();  // Send the updated pixel colors to the hardware.
  }
}