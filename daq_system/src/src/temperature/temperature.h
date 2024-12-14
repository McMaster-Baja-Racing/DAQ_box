#include <cstdint>
#include <Adafruit_MCP9600.h>

// Definitions
#define TEMPERATURE_PIN 16

// Declarations
extern float temperature;
extern bool mcp_initialized;
extern Adafruit_MCP9600 mcp;

// Function Declarations
void tempData();
void tempSetup();
void readTemp();
