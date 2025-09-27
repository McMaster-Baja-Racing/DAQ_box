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
        bajaData.write((uint8_t*)&sdTemp, sizeof(sdTemp));
        counter = -1;
      }
      counter++;
    }

    // Write the remaining data to the SD card
    if (counter != 0 && millis() <= (str + 100)) {
      bajaData.write((uint8_t*)&sdTemp, sizeof(sdTemp));
    }

    // Flush the data to the SD card
    bajaData.flush(); 
  }
}

void buffPush(int id, float tempData) {
  Serial.printf("in buffpush in sd card, float");
  
  if (!USE_SD && EN_SEROUT) {
    
    float timestamp = (millis() << 6) | id;
    Serial.println("FL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData, 4) + ", Timestamp: " + String(timestamp));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_float = tempData;
 
  if (gps_active && !(*savingBuff).push(temp)) {
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }
}

void buffPush(int id, unsigned long tempData) {
    Serial.printf("in buffpush in sd card, long");

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
}