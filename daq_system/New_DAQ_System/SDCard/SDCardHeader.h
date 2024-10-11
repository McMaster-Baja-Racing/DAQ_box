#include <SPI.h>
#include <SD.h>

#ifndef SDCARDHEADER_H
#define SDCARDHEADER_H

void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void sdSend();
void buffPush(int id, float tempData); // Overloaded function to push data to the saving buffer with float data
void buffPush(int id, unsigned long tempData); // Overloaded function to push data to the saving buffer with unsigned long data

bool send_data = false;
SdFs sd;
FsFile file;

// File name MUST be 8 or less characters
// https://www.arduino.cc/en/Reference/SDCardNotes
char filename[] = "/00000000.bin";
char directory[] = "/00-00-00";
char fileDir [23];
const int chipSelect = BUILTIN_SDCARD;

File bajaData;

#endif