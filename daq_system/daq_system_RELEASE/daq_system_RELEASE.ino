/**************************************************************************
  Author: DAQ Team
  Created on: 16/10/2020
**************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// HUD
#define LED_PIN    6      // Digital Pin 6 for LED's
#define LED_COUNT  10     // 2 x 5 LED strip
#define BRIGHTNESS 25     // Max brightness = 255
#define LED_INTERVAL 100  // Period in msec for LED update (larger than 100 produces noticable lag)

// BOX
#define BATT_INTERVAL 1000
#define IMU_INTERVAL 100 
#define GPS_INTERVAL 1000
#define GPSECHO  false
#define HALL_THRESH 10
#define POT A2
#define VOLT_PIN  A6
#define RELAY_PIN 4
#define HALL_PIN 5

/***  Start of Global variables  ***/
/***********************************/
Adafruit_GPS GPS(&mySerial);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

unsigned long battTimer = millis();
unsigned long ledTimer = millis();
unsigned long imuTimer = millis();
unsigned long start = micros();
unsigned long end_time = micros();
unsigned long past_time = micros();
uint32_t gpsTimer = millis();

bool gpsPrint = true;
float hall_count = 0;
bool on_state = false;
uint16_t rpm = 2600;

/***  End of Global variables  ***/
/*********************************/

// Declare function for setting pixel colours
void setColour(int8_t edge); // turns off LEDs from (start to LED_COUNT)

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);

   if(!bno.begin()) {while(1);}

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  delay(50);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  mySerial.println(PMTK_Q_RELEASE);
}

void loop() {
//-------------Hall Effect Sensor--------------------
 
 // counting number of times the hall sensor is tripped
 // but without double counting during the same trip
 if (digitalRead(HALL_PIN)==1){
   if (on_state==false){
     on_state = true;
     hall_count += 1;
   }
 } else{
   on_state = false;
 }
 
 if (hall_count > HALL_THRESH){
   end_time = micros();
   past_time = (end_time-start);
   rpm = (hall_count*1000000/past_time)*60;
   hall_count = 0;
   start = micros();
 }

  //-------------Battery Check---------------
  if (millis() - battTimer > BATT_INTERVAL){
    battTimer = millis();
    if (analogRead(VOLT_PIN) >= 6.0){
      digitalWrite(RELAY_PIN, HIGH);
    }
    else{
      digitalWrite(RELAY_PIN, LOW);
    }
  }

   //-------------LED Strip---------------------
  if (millis() - ledTimer > LED_INTERVAL){
    ledTimer = millis();
    
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

 //-----------------IMU--------------------------------------
 if (millis() - imuTimer > IMU_INTERVAL){
   imuTimer = millis();

 }

//-----------------GPS--------------------------------------
 if (millis() - gpsTimer > GPS_INTERVAL) {
   gpsTimer = millis(); // reset the timer

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
