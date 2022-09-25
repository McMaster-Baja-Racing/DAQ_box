
from os import times
import struct

file = open(r"C:\Users\Ariel\OneDrive\Documents\dev\DAQ_box\PythonScripts\test3.BIN", "rb")
DataType=["F_IMU_ABS_X",
  "F_IMU_ABS_Y",
  "F_IMU_ABS_Z",
  "F_IMU_ACCEL_X",
  "F_IMU_ACCEL_Y",
  "F_IMU_ACCEL_Z",
  "F_IMU_GRAVITY_X",
  "F_IMU_GRAVITY_Z",
  "F_IMU_GRAVITY_Y",
  "F_IMU_GYRO_X",
  "F_IMU_GYRO_Y",
  "F_IMU_GYRO_Z",
  "F_IMU_TEMP",
  "F_GPS_LATITUDE",
  "INT_GPS_LAT",
  "F_GPS_LONGITUTE",
  "INT_GPS_LON",
  "F_GPS_ANGLE",
  "F_GPS_SPEED",
  "INT_GPS_TIME",
  "INT_GPS_DAYMONTHYEAR",
  "INT_GPS_SECONDMINUTEHOUR",
  "F_PRIM_TEMP",
  "F_SEC_TEMP",
  "INT_SUS_TRAV_FR",
  "INT_SUS_TRAV_FL",
  "INT_SUS_TRAV_RR",
  "INT_SUS_TRAV_RL",
  "INT_STRAIN1",
  "INT_STRAIN2",
  "INT_STRAIN3",
  "INT_STRAIN4",
  "INT_STRAIN5",
  "INT_STRAIN6",
  "F_RPM_FR",
  "F_RPM_FL",
  "F_RPM_SEC",
  "F_RPM_PRIM",
  "INT_BATT_PERC",
  "INT_BATT_VOLT",
  "DATATYPE_COUNT"
]

import csv

def write_csv(data, filename):
    with open(str("PythonScripts/data/"+filename+'.csv'), 'a',newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(data)

for type in DataType:
    with open(str("PythonScripts/data/"+type+'.csv'), 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Timestamp (ms)",type])

while (True):
    data = file.read(8)
    if (data==b''):
        break
    timestamp=int.from_bytes(data[0:4], byteorder='little')
    id=timestamp&0x3F
    timestamp=timestamp>>6
    print(timestamp)
    print(DataType[id])
    if (DataType[id].index("_")==0):
        print("ERROR")
        break
    elif (DataType[id].index("_")==1):
        typ="f"
        value=struct.unpack('f', data[4:8])[0]
    elif (DataType[id].index("_")==3):
        typ="i"
        value=int.from_bytes(data[4:8], byteorder='little')

    write_csv([timestamp,value], DataType[id])
    print(timestamp,id,typ,value)