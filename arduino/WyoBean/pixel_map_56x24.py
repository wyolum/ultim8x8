from pylab import *
from numpy import *
w = 56
h = 24
digits = '%s'
template = """
#define MatrixWidth %s
#define MatrixHeight %s
uint16_t MatrixMap[MatrixHeight][MatrixWidth] = {
%s
};
""" % (w, h, digits)
even8 = arange(8)
odd8 = even8[::-1] + 8
cols8x2 = vstack([even8, odd8]).T
u88 = hstack([cols8x2 + 16 * j for j in range(4)])

u88row = hstack([u88 + i * 64 for i in range(7)])
rect = vstack([u88row,
               u88row[::-1,::-1] + 64 * 7,
               u88row + 64 * 14])[:,::-1]

def fmt(val):
    return '%4d' % val
print ('''\
#define MatrixWidth 56
#define MatrixHeight 24
uint16_t MatrixMap[MatrixHeight][MatrixWidth] = {''')
for row in rect:
    print ('{' + ','.join(map(fmt, row)) + '},')
print('};')
