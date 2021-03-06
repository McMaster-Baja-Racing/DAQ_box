/***************************************
Desc: Tool for finding a speifc colour in RGB format.
***************************************/
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN        6 

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 10

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals.
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(9600);
  while(!Serial){delay(10);} // Wait until Serial is ready

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();            // Turn OFF all strip ASAP
  pixels.setBrightness(20); // Set BRIGHTNESS (max = 255)
}

void loop()
{
  Serial.println("Enter R value (0-255): ");
  while (Serial.available() == 0) {} 
  int R = Serial.parseInt(SKIP_ALL);

  Serial.println("Enter G value (0-255): ");
  while (Serial.available() == 0) {} 
  int G = Serial.parseInt(SKIP_ALL);
  
  Serial.println("Enter B value (0-255): ");
  while (Serial.available() == 0) {} 
  int B = Serial.parseInt(SKIP_ALL);

  Serial.print("\nCurrent colour: ");
  Serial.print("R:"); Serial.println(R);
  Serial.print("G:"); Serial.println(G);
  Serial.print("B:"); Serial.println(B);

  pixels.clear(); // Set all pixel colors to 'off'
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<LED_COUNT; i++) {
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(R, G, B));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
  // 100 msec delay
  delay(50);
}
