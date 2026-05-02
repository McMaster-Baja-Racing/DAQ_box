#include <cstdint>
#include "../datastruct/dataTypeNames.h"

// Definitions
#define Rear_speed_counts_per_rotation 16 // 16 teeth on s3
#define Prim_counts_per_rotation 4
// Gear ratio from the s3 gear to the secondary (apply to recorded rear RPM)
#define REAR_GEAR_RATIO (40.0f/15.0f)
// Threshold for how many magnets until an rpm is recorded, as this number increase noise reduces but data can be missed
// In 2023 we came to the conclusion that noise is okay and we can filter the data very well in post using the s-golay filter
#define HALL_THRESH 1

// Declarations
extern bool EN_RPM; 
extern bool REAR_SPEED_stopped;
extern bool PRIM_stopped;

// timestamps and ISR-shared counts (used by new ISR-based calc)
extern volatile uint32_t PRIM_hall_count;
extern volatile uint32_t REAR_SPEED_hall_count;
extern volatile unsigned long PRIM_first_pulse;
extern volatile unsigned long PRIM_last_pulse;
extern volatile bool PRIM_window_started;
extern volatile unsigned long REAR_SPEED_first_pulse;
extern volatile unsigned long REAR_SPEED_last_pulse;
extern volatile bool REAR_SPEED_window_started;

// last pulse times for stopped detection
extern volatile unsigned long PRIM_last_pulse_time;
extern volatile unsigned long REAR_SPEED_last_pulse_time;


// output variables (keep an int copy for existing code but publish float RPM)
extern float REAR_SPEED_rpm;
extern int PRIM_rpm;


// Function Declarations
void rpmCalc();
void incrementHall_REAR_SPEED();
void incrementHall_PRIM();