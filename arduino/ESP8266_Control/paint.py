from __future__ import print_function
from argparse import ArgumentParser
from bibliopixel import LEDMatrix, MatrixRotation
from bibliopixel.drivers.network_udp import DriverNetworkUDP
from BiblioPixelAnimations.receivers.GenericNetworkReceiver import GenericNetworkReceiver
import os, sys, time

try:
    from seq_ui import PixelPainter
except:
    cur_path = os.path.dirname(os.path.realpath(__file__))
    seq_path = os.path.normpath(os.path.join(cur_path, '../../scripts/sequencer/'))
    print(seq_path)
    sys.path.append(seq_path)
    from seq_ui import PixelPainter


def main():
    parser = ArgumentParser()
    parser.add_argument("--width", help="Display width", default=8, type=int)
    parser.add_argument("--height", help="Display height", default=8, type=int)
    parser.add_argument("--bright", help="Brightness Level", default=255, type=int)
    parser.add_argument("--ip", help="Remote IP Address", default="10.0.1.133", type=str, required=True)
    args = parser.parse_args()

    driver = DriverNetworkUDP(num=args.width * args.height,
                              width=args.width, height=args.height,
                              host=args.ip, port=1822)
    try:
        led = LEDMatrix(driver, width=args.width, height=args.height, coordMap=None,
                        rotation=MatrixRotation.ROTATE_90, vert_flip=False,
                        serpentine=True, threadedUpdate=True,
                        masterBrightness=args.bright, pixelSize=(1, 1))

        anim = GenericNetworkReceiver(led, port=3142, interface='0.0.0.0')
        anim.run(threaded=True)

        PixelPainter(n_row=args.width, n_col=args.height,
                     display_ip="127.0.0.1", display_port=3142)
    except Exception as e:
        print("Exception Encountered!", e)
    finally:
        # NetworkUDP might miss off... really hammer it
        for _ in range(10):
            led.all_off()
            led.update()
            time.sleep(0.1)


if __name__ == "__main__":
    main()
