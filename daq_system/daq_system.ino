/**************************************************************************
  Author: DAQ Team
  Created on: 16/10/2020
  Modified by:
  Modified on:
**************************************************************************/
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting Neostrip on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.// (Green Red Blue) format {NOT RGB}
// (0,   255,   0) red
// (158, 255,   0) orange
// (255, 255,   0) yellow
// (255,   0,   0) green
// (  0,   0, 255) blue
// (  0, 255, 255) purple

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);


#define VOLT_PIN  A6
#define RELAY_PIN 4
#define HALL_PIN 5
#define LED_PIN 6
#define LED_COUNT 10
#define LED_INTERVAL 100  // Period in milliseconds for each LED update
#define BATT_INTERVAL 1000
#define IMU_INTERVAL 100 
#define GPS_INTERVAL 1000
#define GPSECHO  false
#define HALL_THRESH 10
#define POT A2


unsigned long battTimer = millis();
unsigned long ledTimer = millis();
unsigned long imuTimer = millis();
unsigned long start = micros();
unsigned long end_time = micros();
unsigned long past_time = micros();
uint32_t gpsTimer = millis();
int ledsOn = 0;
bool gpsPrint = true;
float hall_count = 0;
bool on_state = false;
uint16_t rpm = 2600;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     strip are wired for GRB bitstream (most NeoPixel products)

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);


void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);

   if(!bno.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  //bno.setExtCrystalUse(true);

  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all strip ASAP
  strip.setBrightness(100); // Set BRIGHTNESS (max = 255)
  delay(1000);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  mySerial.println(PMTK_Q_RELEASE);
}

void lights(uint8_t edge);





void loop() {
  //delay(1);
  int jump = 99;
  int rpm = 2600;
//  //-------------Hall Sensor--------------------
//  
//  // counting number of times the hall sensor is tripped
//  // but without double counting during the same trip
//  
//  if (digitalRead(HALL_PIN)==1){
//    if (on_state==false){
//      on_state = true;
//      hall_count += 1;
//    }
//  } else{
//    on_state = false;
//  }
//  
//  if (hall_count > HALL_THRESH){
//        // print information about Time and RPM
//    end_time = micros();
//    past_time = (end_time-start);
////    Serial.print("Time Passed: ");
////    Serial.print(time_passed);
////    Serial.println("s");
//    //rpm = (hall_count*1000000/past_time)*60;
//    //rpm = map(analogRead(POT)*100, 0, 500, 1400, 4000);
//    //Serial.println(" ");
//    //Serial.print(rpm);
//    //Serial.print(" RPM");
//    hall_count = 0;
//    start = micros();
//  }



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
    
    
//    if (ledsOn < LED_COUNT){
//      strip.setPixelColor(ledsOn, strip.Color(150, 150, 0));
//      strip.show();
//      ledsOn = ledsOn + 1; 
//    }
//    else{
//      strip.clear();
//      ledsOn = 0;
//    }
    if( rpm < 1500 ){
      lights(0);
    }
    else if( (rpm > 1500) && (rpm < 2100) ){   
      lights(1);
    }
    else if( (rpm > 2100) && (rpm < 2400) ){    
      lights(2);
    }
    else if( (rpm > 2400) && (rpm < 2600) ){    
      lights(3);
    }
    else if( (rpm > 2600) && (rpm < 2800) ){    
      lights(4);
    }
    else if( (rpm > 2800) && (rpm < 3000) ){    
      lights(5);
    }
    else if( (rpm > 3000) && (rpm < 3200) ){    
      lights(6);
    }
    else if( (rpm > 3200) && (rpm < 3400) ){    
      lights(7);
    }
    else if( (rpm > 3400) && (rpm < 3600) ){    
      lights(8);
    }
    else if( (rpm > 3600) && (rpm < 3800) ){    
      lights(9);
    }
    else if( rpm > 3800 ){                      
      lights(9);
    }
    else{         
      lights(0);  // error default
    }
    
    strip.show();   // Send the updated pixel colors to the hardware.
  }


  
//  //----------------------GPS--------------------------------
//  char c = GPS.read();
//  // if you want to debug, this is a good time to do it!
//  if ((c) && (GPSECHO))
//    Serial.write(c);
//
//  // if a sentence is received, we can check the checksum, parse it...
//  if (GPS.newNMEAreceived()) {
//    // a tricky thing here is if we print the NMEA sentence, or data
//    // we end up not listening and catching other sentences!
//    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
//    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
//
//    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
//      return;  // we can fail to parse a sentence in which case we should just wait for another
//  }
//
//  //-----------------IMU--------------------------------------
//  if (millis() - imuTimer > IMU_INTERVAL){
//    imuTimer = millis();
//
//    Serial.print(F("\n"));
//    //Serial.print(F("\nTime: "));
//    Serial.print(imuTimer);
//    Serial.print(F(", "));
//    
//    sensors_event_t event;
//    bno.getEvent(&event); 
//    //Serial.print(F("Orientation: "));
//    Serial.print((float)event.orientation.x);
//    Serial.print(F(", "));
//    Serial.print((float)event.orientation.y);
//    Serial.print(F(", "));
//    Serial.print((float)event.orientation.z);
//    Serial.print(F(", "));
//    
//    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
//    Serial.print(accel.x());
//    Serial.print(", ");
//    Serial.print(accel.y());
//    Serial.print(", ");
//    Serial.print(accel.z());
//    Serial.print(", ");
//  }
//  if (millis() - gpsTimer > GPS_INTERVAL) {
//    gpsTimer = millis(); // reset the timer
//
////    Serial.print("\nTime: ");
////    if (GPS.hour < 10) { Serial.print('0'); }
////    Serial.print(GPS.hour, DEC); Serial.print(':');
////    if (GPS.minute < 10) { Serial.print('0'); }
////    Serial.print(GPS.minute, DEC); Serial.print(':');
////    if (GPS.seconds < 10) { Serial.print('0'); }
////    Serial.print(GPS.seconds, DEC); Serial.print('.');
////    if (GPS.milliseconds < 10) {
////      Serial.print("00");
////    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
////      Serial.print("0");
////    }
////    Serial.println(GPS.milliseconds);
////    Serial.print("Date: ");
////    Serial.print(GPS.day, DEC); Serial.print('/');
////    Serial.print(GPS.month, DEC); Serial.print("/20");
////    Serial.println(GPS.year, DEC);
//    if (GPS.fix) {
//      //Serial.print("  Location: ");
//      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
//      Serial.print(", ");
//      Serial.print(GPS.longitude, 4); Serial.print(GPS.lon);
//      Serial.print(", ");
//      //Serial.print("  Speed (knots): "); 
//      Serial.print(GPS.speed);
//      Serial.print(", ");
//      Serial.print("  Angle: "); Serial.print(GPS.angle);
//      Serial.print(", ");
//      Serial.print("  Altitude: "); Serial.print(GPS.altitude);
//      Serial.print(", ");
//      Serial.print("  Satellites: "); Serial.print((int)GPS.satellites);
//    }
//    else{
//      Serial.print(" Fix: "); Serial.print((int)GPS.fix);
//      Serial.print(" quality: "); Serial.print((int)GPS.fixquality);
//    }
//  }
}

void lights(uint8_t edge){
    uint8_t G[10] = {  0, 158, 222, 222, 255, 255, 255, 255, 128,  51};
    uint8_t R[10] = {255, 255, 255, 255, 255, 255, 196,   0,   0, 153};
    uint8_t B[10] = {  0,   0,   0,   0,   0,   0,   0,   0, 255, 255};
    uint8_t i = 0;
    
    for(i=0; i<edge; i++){
      strip.setPixelColor(i, strip.Color(G[i], R[i], B[i]));
    }

    for(; i<LED_COUNT; i++){
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
}
