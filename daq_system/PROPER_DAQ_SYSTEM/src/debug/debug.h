#include <cstdint>

// Global variables

enum Debug {
    BRAKE_PRESSURE_DEBUG,
    RPM_DEBUG,
    STRAIN_DEBUG,
    TEMPERATURE_DEBUG,
    SUSPENSION_DEBUG,
    NONE
};

// Function definitions

void debug_brake_pressure();

void debug_rpm();

void debug_strain();

void debug_temperature();

void debug_suspension();

void controlDebug(Debug mode);