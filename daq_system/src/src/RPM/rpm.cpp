#include "rpm.h"
#include "../datastruct/dataTypeNames.h"
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"

// Select calculation mode: set NEW_REAR_CALC to 1 for ISR-timestamp method,
// or set OLD_REAR_CALC to 1 to use legacy EMA window approach.
#ifndef NEW_REAR_CALC
#define NEW_REAR_CALC 1
#endif
#ifndef OLD_REAR_CALC
#define OLD_REAR_CALC 0
#endif


volatile uint32_t PRIM_hall_count       = 0;
volatile uint32_t REAR_SPEED_hall_count = 0;

volatile unsigned long PRIM_first_pulse          = 0;
volatile unsigned long PRIM_last_pulse           = 0;
volatile bool          PRIM_window_started       = false;

volatile unsigned long REAR_SPEED_first_pulse    = 0;
volatile unsigned long REAR_SPEED_last_pulse     = 0;
volatile bool          REAR_SPEED_window_started = false;

volatile unsigned long PRIM_last_pulse_time       = 0;
volatile unsigned long REAR_SPEED_last_pulse_time = 0;

bool EN_RPM = true;
bool PRIM_stopped = false;
bool REAR_SPEED_stopped = false;

float avgRearSpeedPulse = 0.0f;
unsigned long REAR_SPEED_start = micros();

float REAR_SPEED_rpm = 0.0f;
int REAR_SPEED_int = 0;
int PRIM_rpm = 0;

#define STOPPED_TIMEOUT_US 3000000UL //3 seconds 

void rpmCalc() {
    if (!EN_RPM) return;

    unsigned long now = micros();

    // -------------------- PRIMARY (ISR timestamp method) --------------------
    noInterrupts();
    uint32_t      primCount   = PRIM_hall_count;
    unsigned long primFirst   = PRIM_first_pulse;
    unsigned long primLast    = PRIM_last_pulse;
    bool          primStarted = PRIM_window_started;
    PRIM_hall_count     = 0;
    PRIM_window_started = false;
    interrupts();

    if (primCount > 1 && primStarted) {
        float elapsed_sec = (primLast - primFirst) / 1000000.0f;
        if (elapsed_sec > 0.0f) {
            float rpm = ((primCount - 1) / elapsed_sec) / Prim_counts_per_rotation * 60.0f;
            PRIM_rpm = (int)(rpm + 0.5f);
            buffPush(RPM_PRIM, rpm);
            PRIM_stopped = false;
        }
    } else if (!PRIM_stopped && (now - PRIM_last_pulse_time >= STOPPED_TIMEOUT_US)) {
        buffPush(RPM_PRIM, 0.0f);
        PRIM_stopped = true;
    }

    // -------------------- REAR SPEED --------------------------------------
#if NEW_REAR_CALC
    noInterrupts();
    uint32_t      rearCount   = REAR_SPEED_hall_count;
    unsigned long rearFirst   = REAR_SPEED_first_pulse;
    unsigned long rearLast    = REAR_SPEED_last_pulse;
    bool          rearStarted = REAR_SPEED_window_started;
    REAR_SPEED_hall_count      = 0;
    REAR_SPEED_window_started  = false;
    interrupts();

    if (rearCount > 1 && rearStarted) {
        float elapsed_sec = (rearLast - rearFirst) / 1000000.0f;
        if (elapsed_sec > 1e-6f) {
            float rpm = ((rearCount - 1) / elapsed_sec) / Rear_speed_counts_per_rotation * 60.0f;
            // apply gear ratio to convert sensor gear RPM to measured secondary RPM
            rpm *= REAR_GEAR_RATIO;
            REAR_SPEED_rpm = rpm;
            REAR_SPEED_int = (int)(rpm + 0.5f);
            buffPush(REAR_SPEED, rpm);
            REAR_SPEED_stopped = false;
        }
    } else if (!REAR_SPEED_stopped && (now - REAR_SPEED_last_pulse_time >= STOPPED_TIMEOUT_US)) {
        REAR_SPEED_rpm = 0.0f;
        REAR_SPEED_int = 0;
        buffPush(REAR_SPEED, 0.0f);
        REAR_SPEED_stopped = true;
    }

#elif OLD_REAR_CALC
    noInterrupts();
    uint32_t rearCount = REAR_SPEED_hall_count;
    interrupts();

    if (rearCount > HALL_THRESH) {
        unsigned long rearEnd     = micros();
        unsigned long rearElapsed = rearEnd - REAR_SPEED_start;

        if (REAR_SPEED_stopped) REAR_SPEED_stopped = false;

        avgRearSpeedPulse = avgRearSpeedPulse * 0.9f + (float)rearElapsed * 0.1f;
        float periodSec = avgRearSpeedPulse / 1000000.0f;
        float rpm = 0.0f;
        if (periodSec > 1e-6f) {
            rpm = (rearCount * 60.0f / periodSec) / Rear_speed_counts_per_rotation;
            rpm *= REAR_GEAR_RATIO;
        }
        REAR_SPEED_rpm = rpm;
        REAR_SPEED_int = (int)(rpm + 0.5f);
        buffPush(REAR_SPEED, rpm);

        noInterrupts();
        REAR_SPEED_hall_count = 0;
        interrupts();
        REAR_SPEED_start = micros();
    } else if (!REAR_SPEED_stopped && (now - REAR_SPEED_last_pulse_time >= STOPPED_TIMEOUT_US)) {
        REAR_SPEED_rpm = 0.0f;
        REAR_SPEED_int = 0;
        buffPush(REAR_SPEED, 0.0f);
        REAR_SPEED_stopped = true;
    }
#endif
}

void incrementHall_PRIM() {
    unsigned long now = micros();
    if (!PRIM_window_started) {
        PRIM_first_pulse    = now;
        PRIM_window_started = true;
    }
    PRIM_last_pulse      = now;
    PRIM_last_pulse_time = now;
    PRIM_hall_count++;
}

void incrementHall_REAR_SPEED() {
    unsigned long now = micros();
    if (!REAR_SPEED_window_started) {
        REAR_SPEED_first_pulse    = now;
        REAR_SPEED_window_started = true;
    }
    REAR_SPEED_last_pulse      = now;
    REAR_SPEED_last_pulse_time = now;
    REAR_SPEED_hall_count++;
}