/**************************************************************************
  Author: Adrian Sochaniwsky
  Created on: 06/10/2020
  Modified by:
  Modified on:
  Desc: This code lights up 10 neopixels based on RPM, a rotary potentiometer 
    provides an rpm output for testing purposes
**************************************************************************/

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting Neostrip on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.

#include <Adafruit_NeoPixel.h>

#define LED_PIN    6
#define LED_COUNT  10
#define BRIGHTNESS 25
#define INTERVAL   100  // Period in milliseconds for each LED update

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
uint16_t rpm = 0;

//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     strip are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)

  Serial.begin(9600);
  while(!Serial){delay(10);} // Wait until Serial is ready
}

// Declare function for setting pixel colours
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)

void loop()
{
  // Code executes IF the interval has elapsed
  currentMillis = millis();
  if ( (currentMillis - previousMillis) >= INTERVAL ){
    previousMillis = currentMillis;

    // Mocking rpm reading with potentiometer
    // Mapping potentiameter output (0 to 1023) --> (1700 to 3900)
    rpm = map(analogRead(A0), 0, 1023, 1700, 3900);
    Serial.print("RPM: "); Serial.println(rpm);

    if( rpm > 3800 )                       {setColour(9);}
    else if( (rpm > 3600) && (rpm < 3800) ){setColour(8);}
    else if( (rpm > 3400) && (rpm < 3600) ){setColour(7);}
    else if( (rpm > 3200) && (rpm < 3400) ){setColour(6);}
    else if( (rpm > 3000) && (rpm < 3200) ){setColour(5);}
    else if( (rpm > 2800) && (rpm < 3000) ){setColour(4);}
    else if( (rpm > 2600) && (rpm < 2800) ){setColour(3);}
    else if( (rpm > 2400) && (rpm < 2600) ){setColour(2);}
    else if( (rpm > 2100) && (rpm < 2400) ){setColour(1);}
    else if( (rpm > 1800) && (rpm < 2100) ){setColour(0);}
    else if( rpm < 1800 )                  {setColour(-1);}
    else                                   {setColour(-1);}  // error/default
    strip.show();   // Send the updated pixel colors to the hardware.
  }
}

void setColour(int8_t edge)
{
  // Set all setColour to off/0
  strip.clear();

  const uint8_t R[10] = {255, 255, 255, 255, 255, 255, 100,   0,   0,  75};
  const uint8_t G[10] = {  0,  80, 150, 200, 200, 235, 255, 255,   0,   0};
  const uint8_t B[10] = {  0,   0,   0,   0,   0,   0,   0,   0, 255, 255};
  
  //Set pixel colour up to strip[edge]
  for(uint8_t i=0; i<=edge; i++){
    strip.setPixelColor(i, strip.Color(R[i], G[i], B[i]));
  }
}
