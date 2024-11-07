#include <cstdint>
#include "../datastruct/dataTypeNames.h"

// Definitions
#define SEC_counts_per_rotation 3
#define Prim_counts_per_rotation 4
// Threshold for how many magnets until an rpm is recorded, as this number increase noise reduces but data can be missed
// In 2023 we came to the conclusion that noise is okay and we can filter the data very well in post using the s-golay filter
#define HALL_THRESH 1

// Declarations
extern bool EN_RPM; 
extern bool SEC_stopped;
extern bool PRIM_stopped;

extern unsigned long SEC_start;
extern unsigned long SEC_end_time;
extern unsigned long SEC_past_time;

extern unsigned long PRIM_start;
extern unsigned long PRIM_end_time;
extern unsigned long PRIM_past_time;

extern int SEC_rpm;
extern int PRIM_rpm;

extern float PRIM_hall_count;
extern float SEC_hall_count;

// Function Declarations
void rpmCalc();
void incrementHall_SEC();
void incrementHall_PRIM();