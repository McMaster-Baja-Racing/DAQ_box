#include <cstdint>

// Definitions
#define BATT_INTERVAL 10000
#define VOLT_PIN  14

// Declarations
extern bool EN_BATT;

extern float batVoltage;

extern int batPercent;

extern unsigned long battTimer;

// Function Declarations
void batteryCheck();