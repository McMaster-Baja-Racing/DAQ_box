#include "sdCard.h"
#include <cstdint>
#include "../datastruct/dataTypeNames.h"

// Initializations
bool gps_active = false;
bool statusLED = 0;
bool EN_SEROUT= false;
bool USE_SD = true;

RingBuff<dataStruct, dataBufferSize> buff1;
RingBuff<dataStruct, dataBufferSize> buff2;
RingBuff<dataStruct, dataBufferSize> *savingBuff = &buff1;
RingBuff<dataStruct, dataBufferSize> *sdBuff = &buff1;
dataStruct temp;

File bajaData;
FsFile bajaDataFast;

unsigned long oldMillis = 0;
// Profiling counters
static unsigned long writeCountFast = 0, totalWriteFast = 0, maxWriteFast = 0;
static unsigned long writeCountSlow = 0, totalWriteSlow = 0, maxWriteSlow = 0;
static unsigned long syncCountFast = 0, totalSyncFast = 0, maxSyncFast = 0;
static unsigned long flushCountFast = 0, totalFlushFast = 0, maxFlushFast = 0;
static unsigned long flushCountSlow = 0, totalFlushSlow = 0, maxFlushSlow = 0;
static unsigned long statsMillis = 0;

// Function Definitions
void sdSend() {
  if (gps_active) {
    // swap the buffers to load the data to the SD card
    if (savingBuff == &buff1) {
      savingBuff = &buff2;
      sdBuff = &buff1;
    } else if (savingBuff == &buff2) {
      savingBuff = &buff1;
      sdBuff = &buff2;
    }

    statusLED = !statusLED;
    digitalWrite(STATUS_PIN, statusLED);
    
    // Temporary array to hold data for writing to SD
    dataStruct sdTemp[8];

    int counter = 0;
    unsigned long str = millis();

    // Process the data from the sdBuff while its not empty
    while (!(*sdBuff).isEmpty()) {
      if (millis() > (str + 100)) {
        Serial.println("SD LONG BOI");
        break;
      }

      (*sdBuff).pop(sdTemp[counter]);

      if (counter >= 7) {
        // Profiled write: measure micros for fast vs slow backend
        if (EN_FAST_SD) {
          unsigned long t0 = micros();
          bajaDataFast.write((uint8_t*)&sdTemp, sizeof(sdTemp)); 
          unsigned long dt = micros() - t0;
          writeCountFast++; totalWriteFast += dt; if (dt > maxWriteFast) maxWriteFast = dt;
        } else {
          unsigned long t0 = micros();
          bajaData.write((uint8_t*)&sdTemp, sizeof(sdTemp));
          unsigned long dt = micros() - t0;
          writeCountSlow++; totalWriteSlow += dt; if (dt > maxWriteSlow) maxWriteSlow = dt;
        }
        counter = -1;
      }
      counter++;
    }

    // Write the remaining data to the SD card (only write valid elements)
    if (counter > 0 && millis() <= (str + 100)) {
      if (EN_FAST_SD) {
        unsigned long t0 = micros();
        bajaDataFast.write((uint8_t*)sdTemp, counter * sizeof(dataStruct)); 
        unsigned long dt = micros() - t0;
        writeCountFast++; totalWriteFast += dt; if (dt > maxWriteFast) maxWriteFast = dt;
      } else {
        unsigned long t0 = micros();
        bajaData.write((uint8_t*)sdTemp, counter * sizeof(dataStruct));
        unsigned long dt = micros() - t0;
        writeCountSlow++; totalWriteSlow += dt; if (dt > maxWriteSlow) maxWriteSlow = dt;
      }
      
    }
    //sync every minute, otherwise flush, only if fast is enabled, otherwise use fallback flush
    if (EN_FAST_SD) {
      if (millis() % 1000 < 50) {
        unsigned long t0 = micros();
        auto ok = bajaDataFast.sync();
        unsigned long dt = micros() - t0;
        syncCountFast++; totalSyncFast += dt; if (dt > maxSyncFast) maxSyncFast = dt;
      } else {
        unsigned long t1 = micros();
        bajaDataFast.flush();
        unsigned long dtf = micros() - t1;
        flushCountFast++; totalFlushFast += dtf; if (dtf > maxFlushFast) maxFlushFast = dtf;
      }
    } else {
      unsigned long t1 = micros();
      bajaData.flush();
      unsigned long dtf = micros() - t1;
      flushCountSlow++; totalFlushSlow += dtf; if (dtf > maxFlushSlow) maxFlushSlow = dtf;
    }

    // Periodically print profiling stats (every 5s)
    if (millis() - statsMillis > 5000) {
      statsMillis = millis();
      if (writeCountFast || syncCountFast || flushCountFast) {
        /*Serial.print("FAST writes avg(us):");
        Serial.print(writeCountFast ? (totalWriteFast / writeCountFast) : 0);
        Serial.print(" max:"); Serial.print(maxWriteFast);
        Serial.print(" sync avg(us):"); Serial.print(syncCountFast ? (totalSyncFast / syncCountFast) : 0);
        Serial.print(" max:"); Serial.print(maxSyncFast);
        Serial.print(" flush avg(us):"); Serial.print(flushCountFast ? (totalFlushFast / flushCountFast) : 0);
        Serial.print(" max:"); Serial.println(maxFlushFast);*/
      }
      if (writeCountSlow || flushCountSlow) {
        /*Serial.print("SLOW writes avg(us):");
        Serial.print(writeCountSlow ? (totalWriteSlow / writeCountSlow) : 0);
        Serial.print(" max:"); Serial.print(maxWriteSlow);
        Serial.print(" flush avg(us):"); Serial.print(flushCountSlow ? (totalFlushSlow / flushCountSlow) : 0);
        Serial.print(" max:"); Serial.println(maxFlushSlow);*/
      }
      // keep counters rolling (do not reset to let averages over time accumulate)
    }
  }
}

void buffPush(int id, float tempData) {
  if (!USE_SD && EN_SEROUT) {
    Serial.print("FL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData, 4));
    return;
  }
  // Set the timestamp and type for the data
  unsigned long time = millis();
  temp.timeStamp_typ = (time << 6) | id;
  temp.data_float = tempData;
 
  if (gps_active && !(*savingBuff).push(temp)) {
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }

  //debugging code to prove that the problem existed in the sdsend
  /*if (temp.timeStamp_typ > time) {
    oldMillis = time;
    //print the time roughly every 10 seconds using modulo or some other trick
    if (time % 10000 < 50) {
      Serial.println("Current" + String(time) +  "Prev: " + String(oldMillis) + "(time << 6) | id;" + String(temp.timeStamp_typ) + " ID: " + String(id) + " Data: " + String(tempData, 4));
    }
  } else {  
    Serial.println("Time: " + String(temp.timeStamp_typ) + " ID: " + String(id) + " Data: " + String(tempData, 4));
  }*/

}

void buffPush(int id, unsigned long tempData) {
  if (!USE_SD && EN_SEROUT) {
    Serial.println("UL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_long = tempData;

  if (gps_active && !(*savingBuff).push(temp)) {
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }

  //Serial.println("Time: " + String(temp.timeStamp_typ) + " ID: " + String(id) + " Data: " + String(tempData));
}