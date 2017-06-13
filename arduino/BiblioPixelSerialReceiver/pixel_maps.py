from bibliopixel.led import mapGen, MultiMapBuilder, MatrixRotation
import textwrap


## build 8x8 map
ultim8x8 = MultiMapBuilder()
ultim8x8.addRow(mapGen(8, 8, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  vert_flip=False))
## build 8x24 map
ultim8x24 = MultiMapBuilder()
ultim8x24.addRow(mapGen(8, 24, serpentine=True,
                      rotation=MatrixRotation.ROTATE_90,
                      vert_flip=False))


## build 8x72 map
ultim8x72 = MultiMapBuilder()
ultim8x72.addRow(mapGen(8, 72, serpentine=True,
                      rotation=MatrixRotation.ROTATE_90,
                      vert_flip=False))


## build 24x24 array map
ultim24x24 = MultiMapBuilder()
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  vert_flip=False))
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  vert_flip=False))
ultim24x24.addRow(mapGen(8, 24, serpentine=True,
                  rotation=MatrixRotation.ROTATE_90,
                  vert_flip=False))

## build 16x56 array map
ultim16x56 = MultiMapBuilder()
ultim16x56.addRow(mapGen(8, 56, serpentine=True,
                       rotation=MatrixRotation.ROTATE_90,
                       vert_flip=False))
ultim16x56.addRow(mapGen(8, 56, serpentine=True,
                  rotation=MatrixRotation.ROTATE_270,
                  vert_flip=False))

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
    for mmb in [ultim8x8, ultim8x24, ultim24x24, ultim8x72, ultim16x56]:
        m = mmb.map
        w = len(m[0])
        h = len(m)
        ultim = 'ULTIM%dx%d' % (h, w)
        print >> f, '#ifdef %s' % ultim
        print >> f, '#define MatrixWidth %d' % w
        print >> f, '#define MatrixHeight %d' % h
        print >> f, map2h(mmb.map, 'MatrixMap')
        print >> f, '#endif'
        print >> f, ''

    
    
