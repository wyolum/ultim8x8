from pixel_maps import  ultim8x8, ultim8x24, ultim8x72, ultim24x24, ultim16x56
from bibliopixel.drivers.serial_driver import DriverSerial, LEDTYPE
from bibliopixel import LEDMatrix, log, MatrixRotation
from BiblioPixelAnimations.matrix.ScreenGrab import ScreenGrab

ROTATE_0   = MatrixRotation.ROTATE_0
ROTATE_90  = MatrixRotation.ROTATE_90
ROTATE_180 = MatrixRotation.ROTATE_180
ROTATE_270 = MatrixRotation.ROTATE_270

def getPixelArray(pixel_map, dev, rotation, vert_flip):
        '''
        pixel_map -- MultiMap Builder
        dev -- com port ident string
        '''
        width = len(pixel_map.map[0])
        height = len(pixel_map.map)
        n_pixel = width * height
        driver = DriverSerial(LEDTYPE.GENERIC,
                              n_pixel,
                              hardwareID="16C0:0483",
                              dev=dev)
        led = LEDMatrix(driver,
                        width=width,
                        height=height,
                        coordMap=pixel_map.map,
                        rotation=rotation,
                        vert_flip=vert_flip)
        return led

def ULTiM8x8(dev, rotation=ROTATE_0, vert_flip=False):
    pixel_map = ultim8x8         
    return getPixelArray(pixel_map, dev, rotation=rotation, vert_flip=vert_flip)

def ULTiM8x24(dev, rotation=ROTATE_0, vert_flip=False):
    pixel_map = ultim8x24         
    return getPixelArray(pixel_map, dev, rotation=rotation, vert_flip=vert_flip)

def ULTiM8x72(dev, rotation=ROTATE_0, vert_flip=False):
    pixel_map = ultim8x72        
    return getPixelArray(pixel_map, dev, rotation=rotation, vert_flip=vert_flip)

def ULTiM24x24(dev, rotation=ROTATE_0, vert_flip=False):
    pixel_map = ultim24x24        
    return getPixelArray(pixel_map, dev, rotation=rotation, vert_flip=vert_flip)

def ULTiM16x56(dev, rotation=ROTATE_0, vert_flip=False):
    pixel_map = ultim16x56
    return getPixelArray(pixel_map, dev, rotation=rotation, vert_flip=vert_flip)

        
