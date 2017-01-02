from __future__ import print_function
from bibliopixel.drivers.network_udp import DriverNetworkUDP
from bibliopixel import LEDMatrix, MatrixRotation, MultiMapBuilder, mapGen
from BiblioPixelAnimations.matrix.bloom import Bloom
from BiblioPixelAnimations.matrix.Text import ScrollText
from BiblioPixelAnimations.matrix.spectrum import Spectrum
from bibliopixel import log
import time

log.setLogLevel(log.DEBUG)

w = 8
h = 8
rotation = MatrixRotation.ROTATE_0

print("connecting")
ip_list = [
    '10.0.1.31',
    '10.0.1.32',
    '10.0.1.33',
    '10.0.1.34'
]
drivers = []
for ip in ip_list:
    drivers.append(DriverNetworkUDP(num=w * h, broadcast=False, host=ip, port=1822))

print("setup matrix")
build = MultiMapBuilder()
build.addRow(mapGen(w, h, rotation=rotation), mapGen(w, h, rotation=rotation))
build.addRow(mapGen(w, h, rotation=rotation), mapGen(w, h, rotation=rotation))
led = LEDMatrix(drivers, width=w * 2, height=h * 2, coordMap=build.map,
                rotation=MatrixRotation.ROTATE_0, vert_flip=False,
                serpentine=True, threadedUpdate=False,
                masterBrightness=128, pixelSize=(1, 1))

try:
    print("run anim")
    while True:
        anim = Bloom(led, dir=True)
        anim.run(amt=6, fps=15, seconds=5)
        anim = ScrollText(led, "Hello World", xPos=w * 2,
                          color=(255, 0, 0), font_scale=2)
        anim.run(amt=1, fps=15, untilComplete=True)
        # anim = Spectrum(led, vis_list=['BasicLineGraph'], steps_per_vis=100,
        #                 bins=16, max_freq=4000, log_scale=True, auto_gain=True, gain=3)
        # anim.run(amt=1, fps=10, seconds=10)
except:
    raise
finally:
    log.debug('All Off')
    for _ in range(10):
        led.all_off()
        led.update()
        time.sleep(0.1)
    log.debug('Off?')
