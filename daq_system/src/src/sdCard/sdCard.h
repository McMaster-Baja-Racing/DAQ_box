#include <cstdint>
#include <SPI.h>
#include <SD.h>
#include "RingBuff.h"

// Definitions
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

// Declarations
extern RingBuff<dataStruct, dataBufferSize> buff1;
extern RingBuff<dataStruct, dataBufferSize> buff2;
extern RingBuff<dataStruct, dataBufferSize> *savingBuff;
extern RingBuff<dataStruct, dataBufferSize> *sdBuff;
extern dataStruct temp;

extern File bajaData;

extern bool statusLED;
extern bool EN_SEROUT;
extern bool USE_SD;

extern bool EN_FAST_SD;
extern FsFile bajaDataFast;

// Function Declarations
void sdSend();

void buffPush(int id, float tempData);

void buffPush(int id, unsigned long tempData);