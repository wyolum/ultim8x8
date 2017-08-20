import textwrap
from bibliopixel import gen_matrix as mapGen
from bibliopixel.layout.multimap import MultiMapBuilder
from bibliopixel.layout import Rotation as MatrixRotation

R090 = MatrixRotation.ROTATE_90
R270 = MatrixRotation.ROTATE_270

class ULTiMap(MultiMapBuilder):
    '''
    Return the map for array of ULTiM8x8 boards.
    total LED count is 64 * n_8x8_row * n_8x8_col
    '''
    
    def __init__(self, n_8x8_row, n_8x8_col):
        MultiMapBuilder.__init__(self)
        self.n_8x8_row = n_8x8_row
        self.n_8x8_col = n_8x8_col
        for row_i in range(n_8x8_row):
            self.addRow(mapGen(8, 8 * n_8x8_col, serpentine=True,
                               rotation=[R090, R270][row_i % 2],
                               y_flip=False))
        self.w = len(self.map[0])
        self.h = len(self.map)

    def toH(self, f):
        ultim = 'ULTIM%dx%d' % (self.h, self.w)
        print('#ifdef %s' % ultim, file=f)
        print('#define MatrixWidth %d' % self.w, file=f)
        print('#define MatrixHeight %d' % self.h, file=f)
        
        lines = ['uint16_t MatrixMap[MatrixHeight][MatrixWidth] = {']
        for row in self.map:
            lines.append('{' + ','.join(map(str, row)) + '},')
        lines.append('};')
        lines = '\n'.join(lines)
        print(lines, file=f)

        print('#endif', file=f)
        print('', file=f)
        
## build 24x24 array map
### test old way
ultim24x24 = MultiMapBuilder()
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  y_flip=False))
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))

def test():
    import numpy
    my_24x24 = numpy.array(ULTiMap(3, 3).map)
    u24x24 = numpy.array(ultim24x24.map)
    assert numpy.linalg.norm(my_24x24 - u24x24) < 1e-8
test()

def create_map_h():
    f = open('MatrixMaps.h', 'w')
    # maps = [ultim8x8, ultim8x24, ultim24x24, ultim48x24, ultim8x72, ultim16x56]
    sizes = ((1, 1), (1, 3), (3, 3), (6, 3), (1, 9), (2, 7))
    for n_8x8_row, n_8x8_col in sizes:
        mmb = ULTiMap(n_8x8_row, n_8x8_col)
        mmb.toH(f)
    print('wrote', f.name)
create_map_h()

