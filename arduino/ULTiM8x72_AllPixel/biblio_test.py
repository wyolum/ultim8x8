from pixels import *
import os
import time
import glob
import serial
import struct
from pylab import *

port = glob.glob('/dev/ttyACM*')[0]
    
# pixels = Pixels(8,72)
pixels = Pixels(16, 56)
s = serial.Serial(port, baudrate=115200, timeout=1.)
pixels.setBrightness(s, 10)
res = s.read(1)
if len(res) > 1:
    print ord(res[0])
x = zeros([16, 56])
for row in range(16):
    for col in range(56):
        x[row, col] = pixels.snake(row, col)
# pcolormesh(x)
# show()
        
pacman = [XPM2('pacman_%d.xpm2' % i) for i in [1, 2, 3, 4, 5, 4, 3, 2]]
pacman16x16 = [XPM2('pacman_16x16_%d.xpm2' % i) for i in [1, 2, 3, 4, 5, 4, 3, 2]]
ghost = XPM2('ghost_1.xpm2')
shadow = XPM2('shadow.xpm2')
speedy = XPM2('speedy.xpm2')
bashful = XPM2('bashful.xpm2')
pokey = XPM2('pokey.xpm2')

ghosts = [shadow, speedy, bashful, pokey, ghost]
for g in ghosts:
    g.pixels *= 255

while 1:
    s = serial.Serial(port, baudrate=115200, timeout=1.)

    pacman = [XPM2('pacman_%d.xpm2' % i) for i in [1, 2, 3, 4, 5, 4, 3, 2]]
    pacman16x16 = [XPM2('pacman_16x16_%d.xpm2' % i) for i in [1, 2, 3, 4, 5, 4, 3, 2]]
    shadow16x16 = XPM2('shadow16x16.xpm2')
    shadow16x16.flip_rl()
    shadow16x16.pixels *= 255

    for i in range(len(pacman)):
        pacman[i].pixels *= 255
    for i in range(len(pacman16x16)):
        pacman16x16[i].pixels *= 255

    for i in range(56 - 8):
        pixels.clear()
        for j in range(0, 56 + 20, 3):
            if j > i:
                pixels.setPixel(8, j, [255, 255, 255])
        pixels.setPixel(8, 53,  [255, 255, 255])
        pixels.setPixel(8, 54,  [255, 255, 255])
        pixels.setPixel(8, 55,  [255, 255, 255])
        pixels.setPixel(9, 53,  [255, 255, 255])
        pixels.setPixel(9, 54,  [255, 255, 255])
        pixels.setPixel(9, 55,  [255, 255, 255])
        pixels.setPixel(7, 53,  [255, 255, 255])
        pixels.setPixel(7, 54,  [255, 255, 255])
        pixels.setPixel(7, 55,  [255, 255, 255])
        pixels.setImage(4, i, pacman[i % len(pacman16x16)])
        pixels.setImage(0, i - 20, shadow16x16)
        pixels.stream(s)
        time.sleep(.025)
    for p in pacman:
        p.flip_rl()
    for p in pacman16x16:
        p.flip_rl()
    # shadow16x16.replace_color([0, 0, 255], [0, 0, 1])
    # shadow16x16.replace_color([255, 0, 0], [0, 0, 255])
    shadow16x16.flip_rl()

    for i in range(2): 
        pixels.clear()
        pixels.setImage(0, 56 -  16, pacman16x16[i % len(pacman16x16)])
        pixels.setImage(4, 56 - 28, ghost)
        pixels.stream(s)
        time.sleep(.1)
        pixels.clear()
        pixels.setImage(4, 56 - 8, pacman[i % len(pacman)])
        pixels.setImage(0, 56 - 32, shadow16x16)
        pixels.stream(s)
        time.sleep(.1)
    for i in range(56 - 12, -18, -1):
        pixels.clear()
        # pixels.setImage(0, i, pacman[i % len(pacman)])
        # pixels.setImage(8, i, pacman[i % len(pacman)])
        pixels.setImage(0, i, pacman16x16[i % len(pacman16x16)])
        pixels.setImage(4, i - 12, ghost)
        # pixels.setImage(0, i - 10, ghost)
        # pixels.setImage(0, i - 20, ghost)
        # pixels.setImage(0, i - 30, ghost)
        # pixels.setImage(0, i - 40, ghost)
        pixels.stream(s)
        # time.sleep(.05)
    del s
    time.sleep(.1)
