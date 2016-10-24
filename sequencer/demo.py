from argparse import ArgumentParser
from bibliopixel import LEDMatrix
from bibliopixel.drivers.visualizer import DriverVisualizer
from BiblioPixelAnimations.receivers.GenericNetworkReceiver import GenericNetworkReceiver

from seq_ui import PixelPainter


def main():
    parser = ArgumentParser()
    parser.add_argument("--width", help="Display width", default=8, type=int)
    parser.add_argument("--height", help="Display height", default=16, type=int)
    args = parser.parse_args()
    driver = DriverVisualizer(num=0, width=args.width, height=args.height,
                              pixelSize=15, port=1618, stayTop=True)

    led = LEDMatrix(driver, serpentine=True, threadedUpdate=False)

    anim = GenericNetworkReceiver(led, port=3142, interface='0.0.0.0')
    anim.run(threaded=True)

    PixelPainter(n_row=args.height, n_col=args.width,
                 display_ip="127.0.0.1", display_port=3142)


if __name__ == "__main__":
    main()
