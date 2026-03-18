#include <cstdint>
#include <Bounce.h>

// Largest filename that can be saved sequentially (if no GPS present for datetime filename)
const unsigned long MAX_FILENAME = 99999999UL;

// Definitions
#define BUTT_PIN 32

// Declarations
extern Bounce inputButton;
extern int lastPressed;

extern bool EN_FAST_SD;

// Function Declarations
void getFilename(uint8_t hour, uint8_t minute, uint8_t second);
void getDirectory(uint8_t day, uint8_t month, uint8_t year);
void handleInputButton();
void createFile();
void createSequentialFile();
void createDateTimeFile(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t second );
