#include "FS.h" 
#include "SD_MMC.h" 


#define SHOW_DEBUG true

bool EN_SEROUT=true;
bool EN_STRAIN=true;

#define STRAIN_FREQ 1 //ms
#define SD_INTERVAL 75
#define QUEUE_SIZE_INTERVAL 1000

#define strainPin 33

bool USE_SD = true;



unsigned long sdTimer=millis();
unsigned long strainTimers=millis();
unsigned long queueSizeTimer = millis();
// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "/00000000.bin";
char directory[] = "/00-00-00";
char fileDir [23];
const int chipSelect =  SS;

File bajaData;
