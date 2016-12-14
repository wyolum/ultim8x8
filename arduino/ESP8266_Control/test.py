from __future__ import print_function
from bibliopixel.drivers.network_udp import DriverNetworkUDP
from bibliopixel import LEDMatrix, MatrixRotation
from BiblioPixelAnimations.matrix.bloom import Bloom
from bibliopixel import log
import time

log.setLogLevel(log.DEBUG)

print("connecting")
driver = DriverNetworkUDP(num=8 * 8, broadcast=False, host="10.0.1.138", port=1822)

print("setup matrix")
led = LEDMatrix(driver, width=8, height=8, coordMap=None,
                rotation=MatrixRotation.ROTATE_90, vert_flip=False,
                serpentine=True, threadedUpdate=False,
                masterBrightness=255, pixelSize=(1, 1))

print("setup anim")
anim = Bloom(led, dir=True)

try:
    print("run anim")
    anim.run(amt=6, fps=15)
except:
    raise
finally:
    log.debug('All Off')
    for _ in range(10):
        led.all_off()
        led.update()
        time.sleep(0.1)
    log.debug('Off?')
