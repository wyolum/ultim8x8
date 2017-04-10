import re
from numpy import *
from pylab import *
import sys

x = re.compile(r'BITMAP\n([0-9a-fA-F\n]*?)\nENDCHAR', re.MULTILINE)

usage = Exception("python bdf_to_c.py width height bdf_file.bdf")
if len(sys.argv) < 4:
    raise usage

width = int(sys.argv[1])
height = int(sys.argv[2])
fn = sys.argv[3]
t = open(fn).read()

def tobits(b):
    out = []
    for i in range(8):
        out.append(b >> i & 1)
    return out

def format(bytes):
    out = []
    for b in bytes:
        out.append('0x%02x' % b)
    return out
i = 0
out = []

print 'const byte FONT%dx%d_N_ROW = 16;' % (width, height)
print 'const byte FONT%dx%d_N_COL = 8;' % (width, height)
print 'const byte FONT%dx%d_N_CHAR = 128;'% (width, height)

print 'byte font_%dx%d[FONT%dx%s_N_COL*FONT%dx%d_N_CHAR] = {' % (width, height, width, height)

for m in x.finditer(t):
    # print m.group(0)
    bytes = m.group(1).strip()
    bytes = [int(s.strip(), 16) for s in bytes.splitlines()]
    print ', '.join(format(bytes)) + ','
    i += 1
    if i == 128:
        break
print '};'
