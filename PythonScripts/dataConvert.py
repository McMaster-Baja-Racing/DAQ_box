
from os import times
import os
import time

start_time=time.time()

import struct
filename=r"D:\00-00-00\00000000.bin"
file = open(filename, "rb")
size=os.path.getsize(filename)
current=0
DataType=[
    "F_IMU_ABS_X",
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
  "F_BRAKE_PRESS"
  "DATATYPE_COUNT"
]



import csv
files=[]
queue=[""]
def write_csv(data, id):
    files[id].write(data)


for i in range(0, len(DataType)):
    queue.append([""])
# Print iterations progress
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█', printEnd = "\r"):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
        printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print(f'\r{prefix} |{bar}| {percent}% {suffix}', end = printEnd)
    # Print New Line on Complete
    if iteration == total: 
        print()

for i in range(len(DataType)) :
    f=open(str(r"C:\Users\Ariel\OneDrive\Documents\dev\DAQ_box\PythonScripts\data/"+DataType[i]+'.csv'), 'w', newline='')
    files.append(f)
    writer = csv.writer(f)
    writer.writerow(["Timestamp (ms)",DataType[i]])
counter=0
counter2=0
data_whole=file.read()
addition=""
current=0
while (True):
    data = data_whole[current:current+8]
    current+=8
    if (data==b''):
        break
    timestamp=int.from_bytes(data[0:4], byteorder='little')
    id=timestamp&0x3F
    timestamp=timestamp>>6
    try:
        if (DataType[id].index("_")==1):
            typ="f"
            value=struct.unpack('f', data[4:8])[0]
        elif (DataType[id].index("_")==3):
            typ="i"
            value=int.from_bytes(data[4:8], byteorder='little')
        if ("GPS" not in DataType[id]):
            sav=time.time()
            tot=0
            st=str(str(timestamp)+","+str(value)+"\n")

            queue[id][0]+=st
            counter2+=1
            if counter2>10000:
                for i in queue:
                    if (i):
                        write_csv(i[0], queue.index(i))
                    #print(i)
                queue=[""]
                for i in range(0, len(DataType)):
                    queue.append([""])
                tot=time.time()-sav
                counter2=0
            if tot!=0:
                print(tot)
        else:
            addition=" errir"
    except Exception as e:
        print(e)
        print("Error")
        print("ID: "+str(id))
        print("Timestamp: "+str(timestamp))
    printProgressBar(current, size, prefix = (addition+' Time taken so far (s): ' +"{:.2f}".format(time.time()-start_time)+', Progress:'))
    addition=""
for file in files:
    file.close()
print("Time taken (s): "+str(time.time()-start_time))