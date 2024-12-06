#include <cstdint>
#include <Bounce.h>

// Definitions
#define BUTT_PIN 29

// Declarations
extern char filename[];
extern char directory[];
extern char fileDir[];

extern Bounce button;
extern int lastPressed;

// Function Declarations
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void inputButton();

