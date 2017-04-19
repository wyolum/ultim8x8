from numpy import *
from pixels import *
import gtk.gdk
import serial
import glob
import time

pixels = Pixels(32, 56)
port = glob.glob('/dev/ttyACM*')[0]
    
ser = serial.Serial(port, baudrate=115200, timeout=1.)
pixels.setBrightness(ser, 8)

count = 0
last = time.time()
while 1:
    w = gtk.gdk.get_default_root_window()
    sz = w.get_size()
    scale = min((sz[0] // pixels.n_row, sz[1] // pixels.n_col))
    # print scale
    scale = 13

    # print "The size of the window is %d x %d" % sz
    pb = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB,False,8,sz[0],sz[1])
    pb = pb.get_from_drawable(w,w.get_colormap(),0,0,0,0,sz[0],sz[1])
    dat = pb.get_pixels_array()

    for r in range(32):
        for c in range(56):
            # pixels.setPixel(r, c, dat[200 + r * scale + random.randint(0, scale/2), 100 + c * scale + random.randint(0, scale/2)])
            pixels.setPixel(r, c, dat[200 + r * scale, 100 + c * scale])
    pixels.stream(ser)
    count += 1
    if count % 100 == 99:
        del ser
        ser = serial.Serial(port, baudrate=115200, timeout=1.)
        print 100. / (time.time() - last), 'FPS'
        last = time.time()

if (pb != None):
    pass
    # pb.save("screenshot.png","png")
    # print "Screenshot saved to screenshot.png."
else:
    print "Unable to get the screenshot."

