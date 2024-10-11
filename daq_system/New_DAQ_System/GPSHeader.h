#include <Adafruit_GPS.h>
#ifndef GPSHEADER_H
#define GPSHEADER_H
#include <stdint.h>  // For uint16_t

// Function Declarations
void dateTime(uint16_t* date, uint16_t* time);

#define GPSSerial Serial2 

// Object Declarations
extern Adafruit_GPS GPS; // Declare the GPS object as extern

extern bool use_gps;           
extern bool gps_flash;         
extern bool gps_timesend;      
extern bool gps_goodmessage;    
extern bool gps_active;        

extern uint16_t GPS_year;      
extern uint8_t GPS_month;      
extern uint8_t GPS_day;        
extern uint8_t GPS_hour;       
extern uint8_t GPS_minute;     
extern uint8_t GPS_seconds;     

extern float gps_speed; 

#endif