#include "STRAINHeader.h"
#include "../GlobalVars.h"

void strainData(int offset) {
  strain[offset] = analogRead(strainPin[offset]);
  buffPush(STRAIN1+offset, (unsigned long)(strain[offset]));
}