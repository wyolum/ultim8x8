import re
from numpy import *
from pylab import *
import sys

x = re.compile(r'BITMAP\n([0-9A-F\n]*?)\nENDCHAR', re.MULTILINE)

usage = Exception("python bdf_to_c.py bdf_file.bdf")
if len(sys.argv) < 2:
    raise usage

fn = sys.argv[1]
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

print 'byte font_x16[16*128] = {'

for m in x.finditer(t):
    bytes = m.group(1).strip()
    bytes = [int(s.strip(), 16) for s in bytes.splitlines()]
    print ', '.join(format(bytes)) + ','
    i += 1
    if i == 128:
        break
print '};'
