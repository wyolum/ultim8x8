from __future__ import print_function
from bibliopixel.drivers.network_udp import DriverNetworkUDP
from bibliopixel import LEDMatrix, MatrixRotation, MultiMapBuilder, mapGen
from BiblioPixelAnimations.matrix.bloom import Bloom
from BiblioPixelAnimations.matrix.Text import ScrollText
from BiblioPixelAnimations.matrix.spectrum import Spectrum
from BiblioPixelAnimations.matrix.MatrixRain import MatrixRainBow
from BiblioPixelAnimations.matrix.TicTacToe import TicTacToe
from bibliopixel import log
import time

log.setLogLevel(log.DEBUG)

w = 8
h = 8
rotation = MatrixRotation.ROTATE_0

print("connecting")
base_ip = '10.0.1.3'
drivers = []
for i in range(1, 10):
    drivers.append(DriverNetworkUDP(num=w * h, broadcast=False,
                                    host=base_ip + str(i), port=1822))

print("setup matrix")
build = MultiMapBuilder()
for _ in range(3):
    build.addRow(mapGen(w, h, rotation=rotation),
                 mapGen(w, h, rotation=rotation),
                 mapGen(w, h, rotation=rotation))

led = LEDMatrix(drivers, width=w * 3, height=h * 3, coordMap=build.map,
                rotation=MatrixRotation.ROTATE_0, vert_flip=False,
                serpentine=True, threadedUpdate=False,
                masterBrightness=64, pixelSize=(1, 1))

try:
    print("run anim")
    while True:
        # anim = Bloom(led, dir=True)
        # anim.run(amt=6, fps=15)
        # anim = ScrollText(led, "Hello World", xPos=w,
        #                   color=(255, 0, 0), font_scale=3)
        # anim.run(amt=1, fps=15)
        # anim = Spectrum(led, vis_list=['BasicLineGraph'], steps_per_vis=100,
        #                 bins=16, max_freq=4000, log_scale=True, auto_gain=True, gain=3)
        # anim.run(amt=1, fps=10, seconds=10)
        # anim = MatrixRainBow(led)
        # anim.run(fps=15)
        anim = TicTacToe(led)
        anim.run(fps=10)
except:
    raise
finally:
    log.debug('All Off')
    for _ in range(10):
        led.all_off()
        led.update()
        time.sleep(0.1)
    log.debug('Off?')
