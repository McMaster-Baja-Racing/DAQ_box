#include "battery.h"
#include <Arduino.h>
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"


// Intializations
bool EN_BATT = true;

float batVoltage = 0;

int batPercent = 0;

unsigned long battTimer = millis();

void batteryCheck(){
    if (EN_BATT && millis() - battTimer > BATT_INTERVAL) {
    battTimer = millis();
    batVoltage = analogRead(VOLT_PIN);
    batPercent = map(batVoltage, 820, 930, 0, 100);
    if (batPercent <= 0) {
      batPercent = 0;
    } else if (batPercent >= 100) {
      batPercent = 100;
    }
    batVoltage = ((batVoltage / 1024) * 9.1905);

    buffPush(BATT_PERC, (unsigned long)(batPercent));
    buffPush(BATT_VOLT, batVoltage);
  }
}