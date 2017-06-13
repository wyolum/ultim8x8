from bibliopixel.drivers.serial_driver import DriverSerial, LEDTYPE
from bibliopixel import LEDMatrix, log
from BiblioPixelAnimations.matrix.bloom import Bloom
log.setLogLevel(log.DEBUG)

driver = DriverSerial(LEDTYPE.GENERIC, 64, hardwareID="16C0:0483", dev="/dev/ttyACM0")
led = LEDMatrix(driver)

try:
    anim = Bloom(led, dir=True)
    anim.run(amt=1, fps=15)
except:
    led.all_off()
    led.update()
