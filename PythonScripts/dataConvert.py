
file = open(r"C:\Users\Ariel\OneDrive\Documents\dev\DAQ_box\PythonScripts\data.bin", "rb")
DataType=["INT","FLOAT",
  "IMU_ABS_X",
  "IMU_ABS_Y",
  "IMU_ABS_Z",
  "IMU_ACCEL_X",
  "IMU_ACCEL_Y",
  "IMU_ACCEL_Z",
  "IMU_GRAVITY_X",
  "IMU_GRAVITY_Y",
  "IMU_GRAVITY_Z",
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
  "RPM_FR",
  "RPM_FL",
  "RPM_SEC",
  "RPM_PRIM",
  "BATT_PERC",
  "BATT_VOLT",
  "DATATYPE_COUNT"
]

import csv

def write_csv(data, filename):
    with open(str("PythonScripts/data/"+filename+'.csv'), 'a') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(data)

for type in DataType:
    with open(str("PythonScripts/data/"+type+'.csv'), 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Timestamp (ms)",type])

while (True):
    data = file.read(12)
    if (data==b''):
        break
    timestamp=int.from_bytes(data[0:4], byteorder='little')
    print(timestamp)
    id=int.from_bytes(data[4:6], byteorder='little')
    typ=int.from_bytes(data[6:8], byteorder='little')
    if (typ==0):
        value=int.from_bytes(data[8:12], byteorder='little')
    elif (typ==1):
        value=float.from_bytes(data[8:12], byteorder='little')
    write_csv([timestamp,value], DataType[id])
    print(timestamp,id,typ,value)