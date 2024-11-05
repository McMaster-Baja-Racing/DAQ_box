#include "temperature.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"
#include <Adafruit_MCP9600.h>

// Initializations
float temperature;
bool mcp_initialized = false;
Adafruit_MCP9600 mcp;

// Function Definitions
void tempData() {
  //temperature = (analogRead(TEMPERATURE_PIN))/*/2*/;
  //Serial.print("primtemp: ");
  //Serial.println(temperature);
  if(mcp_initialized) {
    temperature = mcp.readThermocouple();
    buffPush(PRIM_TEMP, (unsigned long)(temperature));
  }
}

void tempSetup() {
  if (!mcp.begin(0x67)) {
    Serial.println("MCP9600 failed, or not present");
    mcp_initialized = false;
  } else {
    mcp.setThermocoupleType(MCP9600_TYPE_K);
    Serial.println("MCP9600 K-TYPE SUCCESS");
    mcp_initialized = true;
  }
}