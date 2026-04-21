#include <cstdint>
#include "../datastruct/dataTypeNames.h"

// Definitions
#define Rear_speed_counts_per_rotation 4 //using 4 as temp fix, maybe incccreases lowspeed sampling. //This is for s3, which has 16 teeth, the ration from this to the secondary RPM is ___
#define Prim_counts_per_rotation 4
// Threshold for how many magnets until an rpm is recorded, as this number increase noise reduces but data can be missed
// In 2023 we came to the conclusion that noise is okay and we can filter the data very well in post using the s-golay filter
#define HALL_THRESH 1

// Declarations
extern bool EN_RPM; 
extern bool REAR_SPEED_stopped;
extern bool PRIM_stopped;

extern unsigned long REAR_SPEED_start;
extern unsigned long REAR_SPEED_end_time;
extern unsigned long REAR_SPEED_past_time;

extern unsigned long PRIM_start;
extern unsigned long PRIM_end_time;
extern unsigned long PRIM_past_time;

extern int REAR_SPEED_int;
extern int PRIM_rpm;

extern float PRIM_hall_count;
extern float REAR_SPEED_hall_count;
extern float avgPrimPulse;
extern float avgRearSpeedPulse;

// Function Declarations
void rpmCalc();
void incrementHall_REAR_SPEED();
void incrementHall_PRIM();