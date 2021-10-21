from enum import Flag
from sys import path
import serial
from time import sleep, time
import datetime
import signal
import sys

exit = False
def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')
    global exit
    exit = True

signal.signal(signal.SIGINT, signal_handler)




#   TODO
# Implement this
#timeInterval = 1 # time interval between each write to csv file (all reading will be averaged throughtout) 0 - to write every incoming packate 

ser = serial.Serial(
    port='/dev/ttyUSB1',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)
def Read(getCsvHeaders = False):
    bufferStr = ""
    buffChar = ""
    while(buffChar != "\n"):
        try:
            buffChar = ser.read(1).decode("ascii")
        except:
            ser.flushInput()
            return None
        bufferStr = bufferStr + buffChar
    if(buffChar == "\n"):
        print("Got: " + bufferStr)
        if(bufferStr.startswith("Vals: ") and bufferStr.count("Vals") == 1):
            bufferStr = bufferStr[6:-3]
            valList = bufferStr.split("; ")
            csvStr = ""
            csvHeaderList = []
            for n in valList:
                if(getCsvHeaders):
                    csvHeader = ''.join([i for i in n if not i.isdigit()])
                    csvHeader = csvHeader[1:]
                    print(csvHeader)
                    csvHeaderList.append(csvHeader)
                if(csvStr != ""):
                    csvStr = csvStr + "," + ''.join([i for i in n if(i.isdigit() or i == '.')])
                else:
                    csvStr = ''.join([i for i in n if(i.isdigit() or i == '.')])
            if(getCsvHeaders):
                return csvHeaderList
            else:
                return csvStr
        buffChar = ""
        bufferStr = ""


fileName = str(datetime.datetime.now())
fileName = fileName.strip()
fileName = fileName + ".csv"
print(fileName)
csvHeader = None

while(csvHeader == None):
    csvHeader = Read(getCsvHeaders=True)


f = open(fileName, "w")
f.write(csvHeader[0] + "," + csvHeader[1] + "," + csvHeader[2] + "," + csvHeader[3] + ",time" + "\n")
f.close()
start_time = time()
ser.flushInput()
while(exit == False):
    f = open(fileName, "a")
    deltaTime = "," + str(round(time() - start_time, 3))
    buff = Read()
    if(buff != None):
        buff += deltaTime
        buff += "\n"
        #print(buff)
        f.write(buff)
    f.close()







    