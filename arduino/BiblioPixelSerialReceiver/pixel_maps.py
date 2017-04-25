from bibliopixel.led import mapGen, MultiMapBuilder, MatrixRotation


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




