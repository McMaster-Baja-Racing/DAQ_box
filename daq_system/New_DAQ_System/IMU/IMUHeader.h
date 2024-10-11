#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#ifndef IMUHEADER_H
#define IMUHEADER_H

void imuData();

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

/***  Gyro  ***/ 
imu::Vector<3> accel;
imu::Vector<3> gyro;
imu::Vector<3> gravity;
sensors_event_t event;

int imuAccelCal = false;
int imuGyroCal = false;

#endif