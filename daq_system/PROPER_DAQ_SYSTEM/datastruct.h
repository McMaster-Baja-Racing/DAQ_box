
#include "RingBuff.h"

const int dataBufferSize=1024*10;

typedef struct dataStruct_t
{
  unsigned long timeStamp_typ; // 32 bits
    
  union 
  {
    unsigned long data_long;
    float data_float;
    unsigned long printData;
  };
} dataStruct;

enum DataType
{
  IMU_ABS_X,
  IMU_ABS_Y,
  IMU_ABS_Z,
  IMU_ACCEL_X,
  IMU_ACCEL_Y,
  IMU_ACCEL_Z,
  IMU_GRAVITY_X,
  IMU_GRAVITY_Z,
  IMU_GRAVITY_Y,
  IMU_GYRO_X,
  IMU_GYRO_Y,
  IMU_GYRO_Z,
  IMU_TEMP,
  GPS_LATITUDE,
  GPS_LAT,
  GPS_LONGITUTE,
  GPS_LON,
  GPS_ANGLE,
  GPS_SPEED,
  GPS_TIME,
  GPS_DAYMONTHYEAR,
  GPS_SECONDMINUTEHOUR,
  PRIM_TEMP,
  SEC_TEMP,
  SUS_TRAV_FR,
  SUS_TRAV_FL,
  SUS_TRAV_RR,
  SUS_TRAV_RL,
  STRAIN1,
  STRAIN2,
  STRAIN3,
  STRAIN4,
  STRAIN5,
  STRAIN6,
  RPM_FR,
  RPM_FL,
  RPM_SEC,
  RPM_PRIM,
  BATT_PERC,
  BATT_VOLT,
  BRAKE_PRESS,
  IMU_QUAT_W,
  IMU_QUAT_X,
  IMU_QUAT_Y,
  IMU_QUAT_Z,
  DATATYPE_COUNT
};

String DataTypeNames[]={
  "IMU_ABS_X",
  "IMU_ABS_Y",
  "IMU_ABS_Z",
  "IMU_ACCEL_X",
  "IMU_ACCEL_Y",
  "IMU_ACCEL_Z",
  "IMU_GRAVITY_X",
  "IMU_GRAVITY_Z",
  "IMU_GRAVITY_Y",
  "IMU_GYRO_X",
  "IMU_GYRO_Y",
  "IMU_GYRO_Z",
  "IMU_TEMP",
  "GPS_LATITUDE",
  "GPS_LAT",
  "GPS_LONGITUTE",
  "GPS_LON",
  "GPS_ANGLE",
  "GPS_SPEED",
  "GPS_TIME",
  "GPS_DAYMONTHYEAR",
  "GPS_SECONDMINUTEHOUR",
  "PRIM_TEMP",
  "SEC_TEMP",
  "SUS_TRAV_FR",
  "SUS_TRAV_FL",
  "SUS_TRAV_RR",
  "SUS_TRAV_RL",
  "STRAIN1",
  "STRAIN2",
  "STRAIN3",
  "STRAIN4",
  "STRAIN5",
  "STRAIN6",
  "RPM_FR",
  "RPM_FL",
  "RPM_SEC",
  "RPM_PRIM",
  "BATT_PERC",
  "BATT_VOLT",
  "BRAKE_PRESS",
  "IMU_QUAT_W",
  "IMU_QUAT_X",
  "IMU_QUAT_Y",
  "IMU_QUAT_Z",
  "DATATYPE_COUNT"
};

RingBuff<dataStruct ,dataBufferSize> buff1;
RingBuff<dataStruct ,dataBufferSize> buff2;
RingBuff<dataStruct ,dataBufferSize> *savingBuff = &buff1;
RingBuff<dataStruct ,dataBufferSize> *sdBuff = &buff1;
dataStruct temp;
