import time
from ultim8x8 import ULTiM8x8, ULTiM8x24, ULTiM8x72, ULTiM24x24, ULTiM16x56 
from ultim8x8 import ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
from bibliopixel import log
from BiblioPixelAnimations.matrix.bloom import Bloom
log.setLogLevel(log.DEBUG)


## change this line to indentify your serial USB port
dev = '/dev/ttyACM0'

## change as appropriate (trail and error?)
rotation=ROTATE_180
vert_flip=False

## change this line as appropropriate for your array
led = ULTiM8x8(dev, rotation=rotation, vert_flip=vert_flip)
led.setMasterBrightness(32) ## use low brightness for running off of USB

try:
    ## Initialize and run animation
    anim = Bloom(led, dir=True) ## expanding rainbow rings
    anim.run(amt=1, fps=15)
except KeyboardInterrupt:
    ## Quietly exit with keyboard interrupt (CTRL-C)
    pass
finally:
    led.all_off()
    led.update()
