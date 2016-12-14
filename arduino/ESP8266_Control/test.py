from __future__ import print_function
from bibliopixel.drivers.network_udp import DriverNetworkUDP
from bibliopixel import LEDMatrix, MatrixRotation
from BiblioPixelAnimations.matrix.bloom import Bloom
from BiblioPixelAnimations.matrix.Text import ScrollText
from bibliopixel import log
from bibliopixel.drivers.driver_base import ChannelOrder
import time
from bibliopixel import colors, font

log.setLogLevel(log.DEBUG)

print("connecting")
driver = DriverNetworkUDP(num=8 * 8, broadcast=False, host="10.0.1.133", port=1822)
# driver = DriverNetworkUDP(num=8 * 8, broadcast=True, host="10.0.1.255", port=1822, broadcast_interface='10.0.1.112')

driver.c_order = ChannelOrder.RGB

print("setup matrix")
led = LEDMatrix(driver, width=8, height=8, coordMap=None,
                rotation=MatrixRotation.ROTATE_90, vert_flip=False,
                serpentine=True, threadedUpdate=False,
                masterBrightness=255, pixelSize=(1, 1))

print("setup anim")
# anim = MatrixCalibrationTest(led)
# anim._internalDelay = 100
# anim = Bloom(led, dir=True)
anim = ScrollText(led, "Hello World", xPos=0, yPos=0,
                  color=colors.Green, bgcolor=colors.Off,
                  font_name=font.default_font, font_scale=1)

try:
    print("run anim")
    # anim.run(amt=1, fps=10, sleep=None, max_steps=0,
    #          untilComplete=False, max_cycles=0, threaded=False,
    #          joinThread=False, callback=None, seconds=None)

    anim.run(amt=1, fps=15)
except:
    raise
finally:
    log.debug('All Off')
    for _ in range(10):
        led.all_off()
        led.update()
        time.sleep(0.1)
    log.debug('Off?')
