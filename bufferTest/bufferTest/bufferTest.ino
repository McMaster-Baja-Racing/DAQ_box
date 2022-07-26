// Should likely be 32 bit aligned (if using FAT32 filesystem)
typedef struct dataStruct_t
{
  unsigned long timeStamp; // 32 bits
  unsigned int id;     // 16 bits
  unsigned int spare_16;   // 16 bit
    
  union 
  {
    unsigned long data_long;
    float data_float;
    unsigned long printData;
  };
} dataStruct;

enum DataType
{
  GPS,
  IMU_X_ACCEL,
  IMU_Y_ACCEL,
  IMU_Z_ACCEL,
  DATATYPE_COUNT
};

dataStruct data[12];
int dataCount = 0;

#include <stdio.h>

unsigned long millis()
{
    return 12345;
}

int main()
{
  data[dataCount].timeStamp = millis();
  data[dataCount].id = GPS;
  data[dataCount].data_long = 1233466;
  dataCount++;

  data[dataCount].timeStamp = millis();
  data[dataCount].id = IMU_X_ACCEL;
  data[dataCount].data_float = 0.1245;
  dataCount++;
  
  for (int i = 0; i < dataCount; i++)
  {
      printf("Time: %d, Type: %d, Data: %d\n", data[i].timeStamp, data[i].id, data[i].printData);
  }

  return 0;
}
