from serial import *
import time
s = Serial('/dev/ttyUSB0', baudrate=115200, timeout=.1)

i = 0
while 1:
    s.write('clockiot/set_colorwheel//%d' % i)
    i += 4
    i %= 256
    time.sleep(.4)
    print i
