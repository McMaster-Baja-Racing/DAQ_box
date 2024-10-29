#include "strain.h"
#include "../sd/sd.h"
#include "../datastruct/datastruct.h"

int strain [6];

void strainData(int offset) {
  strain[offset] = analogRead(strainPin[offset]);
  buffPush(STRAIN1+offset, (unsigned long)(strain[offset]));
}