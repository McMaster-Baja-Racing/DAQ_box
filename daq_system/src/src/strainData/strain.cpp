#include "strain.h"
#include "../sdCard/sdCard.h"
#include "../datastruct/dataTypes.h"

// Initializations
int strain [6];

// Function Definitions
void strainData(int offset) {
  strain[offset] = analogRead(strainPin[offset]);
  //printf("Strain %d: %d\n", offset + 1, strain[offset]);
  buffPush(STRAIN1+offset, (unsigned long)(strain[offset]));
}