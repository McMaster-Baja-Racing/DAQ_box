#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

void imuData();

extern Adafruit_BNO055 bno;

extern imu::Vector<3> accel;
extern imu::Vector<3> gyro;
extern imu::Vector<3> gravity;
extern sensors_event_t event;