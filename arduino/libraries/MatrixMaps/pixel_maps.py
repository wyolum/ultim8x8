import textwrap
from bibliopixel import gen_matrix as mapGen
from bibliopixel.layout.multimap import MultiMapBuilder
from bibliopixel.layout import Rotation as MatrixRotation


## build 8x8 map
ultim8x8 = MultiMapBuilder()
ultim8x8.addRow(mapGen(8, 8, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))
## build 8x24 map
ultim8x24 = MultiMapBuilder()
ultim8x24.addRow(mapGen(8, 24, serpentine=True,
                      rotation=MatrixRotation.ROTATE_90,
                      y_flip=False))
## build 8x48 map
ultim8x48 = MultiMapBuilder()
ultim8x48.addRow(mapGen(8, 48, serpentine=True,
                      rotation=MatrixRotation.ROTATE_90,
                      y_flip=False))


## build 8x72 map
ultim8x72 = MultiMapBuilder()
ultim8x72.addRow(mapGen(8, 72, serpentine=True,
                      rotation=MatrixRotation.ROTATE_90,
                      y_flip=False))


## build 24x24 array map
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

## build 48x24 array map
ultim48x24 = MultiMapBuilder()
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  y_flip=False))
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  y_flip=False))
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  y_flip=False))
ultim48x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  y_flip=False))

## build 16x56 array map
ultim16x56 = MultiMapBuilder()
ultim16x56.addRow(mapGen(8, 56, serpentine=True,
                       rotation=MatrixRotation.ROTATE_90,
                       y_flip=False))
ultim16x56.addRow(mapGen(8, 56, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  y_flip=False))


def map2h(m, name):
    w = len(m[0])
    h = len(m)
    out = ['uint16_t %s[MatrixHeight][MatrixWidth] = {' % name]
    for row in m:
        line = '{' + ','.join(map(str, row)) + '},'
        #lines = ['    ' + l for l in textwrap.wrap(line, 80)]
        # out.extend(lines)
        out.append(line)
    out.append('};')
    return '\n'.join(out)

def create_map_h():
    f = open('MatrixMaps.h', 'w')
    maps = [ultim8x8, ultim8x24, ultim24x24, ultim48x24, ultim8x72, ultim16x56]
    for mmb in maps:
        m = mmb.map
        w = len(m[0])
        h = len(m)
        ultim = 'ULTIM%dx%d' % (h, w)
        print('#ifdef %s' % ultim, file=f)
        print('#define MatrixWidth %d' % w, file=f)
        print('#define MatrixHeight %d' % h, file=f)
        print(map2h(mmb.map, 'MatrixMap'), file=f)
        print('#endif', file=f)
        print('', file=f)
    print('wrote', f.name)
create_map_h()
    
    
