#include "sd.h"
#include <cstdint>
#include "../datastruct/dataTypeNames.h"

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

void sdSend() {
  if (gps_active) {
    // Swap the saving and processing buffers
    if (savingBuff == &buff1) {
      savingBuff = &buff2;
      sdBuff = &buff1;
    } else if (savingBuff == &buff2) {
      savingBuff = &buff1;
      sdBuff = &buff2;
    }
    // Toggle the status LED
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
      // Pop the data from the sdBuff and push it to the sdTemp array
      (*sdBuff).pop(sdTemp[counter]);
      // If the counter is 7, write the data to the SD card
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

// Function to push data to the saving buffer with float data
void buffPush(int id, float tempData) {
  if (!USE_SD && EN_SEROUT) {
    Serial.print("FL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData, 4));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_float = tempData;
  // Check if the GPS is active and push the data to the saving buffer
  if (gps_active && !(*savingBuff).push(temp)) {
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }
}

// Function to push data to the saving buffer with unsigned long data
void buffPush(int id, unsigned long tempData) {
  // Check if the SD card is not being used and the serial output is enabled
  if (!USE_SD && EN_SEROUT) {
    Serial.println("UL* ID: " + String(DataTypeNames[id]) + " Data: " + String(tempData));
    return;
  }
  // Set the timestamp and type for the data
  temp.timeStamp_typ = (millis() << 6) | id;
  temp.data_long = tempData;
  // Check if the GPS is active and push the data to the saving buffer
  if (gps_active && !(*savingBuff).push(temp)) {
    Serial.println("Lost Data; savingBuff Size = " + String((*savingBuff).size()) + "; sdBuff Size = " + String((*sdBuff).size()));
  }
}