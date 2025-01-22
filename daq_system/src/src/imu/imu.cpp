#include "imu.h"
#include <cstdint>
#include <Adafruit_BNO055.h>
#include "../datastruct/dataTypes.h"
#include "../sdCard/sdCard.h"

// Initializations

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

imu::Vector<3> accel;
imu::Vector<3> gyro;
imu::Vector<3> gravity;
sensors_event_t event;

// Function Definitions
void imuData() {
  bno.getEvent(&event);

  uint8_t system_cal, gyro_cal, accel_cal, mag_cal;
  system_cal = gyro_cal = accel_cal = mag_cal = 0;

  bno.getCalibration(&system_cal, &gyro_cal, &accel_cal, &mag_cal);

  accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  gravity = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);
  imu::Quaternion quat = bno.getQuat();

  if (system_cal > 0) {

    buffPush(IMU_QUAT_W, (float)quat.w());
    buffPush(IMU_QUAT_X, (float)quat.x());
    buffPush(IMU_QUAT_Y, (float)quat.y());
    buffPush(IMU_QUAT_Z, (float)quat.z());
  
    buffPush(IMU_TEMP, (unsigned long)bno.getTemp());
  }

  if (accel_cal > 0) {
  
    buffPush(IMU_ACCEL_X, float(accel.x()));
    buffPush(IMU_ACCEL_Y, float(accel.y()));
    buffPush(IMU_ACCEL_Z, float(accel.z()));
  
    buffPush(IMU_GRAVITY_X, float(gravity.x()));
    buffPush(IMU_GRAVITY_Y, float(gravity.y()));
    buffPush(IMU_GRAVITY_Z, float(gravity.z()));
  }

  if (gyro_cal > 0) {
 
    buffPush(IMU_GYRO_X, float(gyro.x()));
    buffPush(IMU_GYRO_Y, float(gyro.y()));
    buffPush(IMU_GYRO_Z, float(gyro.z()));
  }
}