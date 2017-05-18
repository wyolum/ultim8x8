import serial
import sys
import time

from kandy_commands import *
port = sys.argv[1]
s = serial.Serial(port, baudrate=115200, timeout=1);

TZ = -4 * 3600
now = time.time() + TZ
hh = int(now % 86400) / 3600
mm = int(now % 86400 - hh * 3600) / 60
ss = int(now % 60)

print hh, mm, ss
s.write(DEC_MODE * 2)
s.write(SET_TO_MIDNIGHT)

s.write(INC_HOUR * hh)

if mm < 30:
    s.write(INC_MIN * mm)
else:
    s.write(INC_HOUR)
    s.write(DEC_MIN * (60 - mm))
    
if ss < 30:
    s.write(INC_MIN * ss)
else:
    s.write(INC_MIN)
    s.write(DEC_SEC * (60 - ss))
    
s.write(INC_SEC * ss)
    
