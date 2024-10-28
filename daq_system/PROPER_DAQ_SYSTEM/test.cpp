#include "test.h"
#include <cstdio>


char filename [] = "/00000000.bin";
char directory[] = "/00-00-00";


void getFilename(uint8_t hour, uint8_t minute, uint8_t second) {
  // Only hour, minute and second are saved in UTC time
  // Please move files before start of new day
  sprintf(filename, "/%02d%02d%02d.bin", (hour, minute, second));
}

void getDirectory(uint8_t day, uint8_t month, uint8_t year) {
  // Please move files before start of new day
  sprintf(directory, "/%02d-%02d-%02d", year, month, day);
}