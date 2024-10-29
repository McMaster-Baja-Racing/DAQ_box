#include <cstdint>
#include <SPI.h>
#include <SD.h>
#include "../../RingBuff.h"

#define STATUS_PIN 28
#define dataBufferSize 10240

typedef struct dataStruct_t
{
  unsigned long timeStamp_typ; // 32 bits
  union 
  {
    unsigned long data_long;
    float data_float;
    unsigned long printData;
  };
} dataStruct;

extern RingBuff<dataStruct, dataBufferSize> buff1;
extern RingBuff<dataStruct, dataBufferSize> buff2;
extern RingBuff<dataStruct, dataBufferSize> *savingBuff;
extern RingBuff<dataStruct, dataBufferSize> *sdBuff;
extern dataStruct temp;

extern File bajaData;

extern bool gps_active;
extern bool statusLED;

extern bool EN_SEROUT;
extern bool USE_SD;

void sdSend();
// Function to push data to the saving buffer with float data
void buffPush(int id, float tempData);

// Function to push data to the saving buffer with unsigned long data
void buffPush(int id, unsigned long tempData);