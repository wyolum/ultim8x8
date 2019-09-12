import os
import tkFileDialog
from pylab import *
from numpy import *
    
DEG = pi/180
BYT = pi/128
PIXEL_W = 20

N_ROW = 11
N_COL = 11

pixels = zeros((N_ROW, N_COL), bool)
flat_pixels = pixels.ravel()

D = N_ROW - 1.
R = D / 2.
_x = arange(N_ROW) - R
_y = arange(N_COL) - R
x, y = meshgrid(_x, _y)
x = x.ravel()
y = y.ravel()
z = zeros(len(y))

r = sqrt(x ** 2 + y  ** 2)
theta =arctan2(y, x)

def circ():
    width = .5
    light = abs(r - R) < width
    flat_pixels[light] = True

def semi_circ():
    width = .5
    light = logical_and(abs(r - R) < width, x > 0)
    flat_pixels[light] = True

def terminator(Theta_byt):
    flat_pixels[:] = False
    if Theta_byt == 0:
        circ()
    else:
        semi_circ()
    term = (flat_pixels == True)

    for theta_byt in range(1, Theta_byt):
        theta = theta_byt * BYT
        rot = array([[cos(theta), 0, -sin(theta)],
                     [         0, 1,           0],
                     [sin(theta), 0,  cos(theta)]])
        ring = dot(rot, vstack([x[term], y[term], z[term]])).T
        for px in ring[ring[:,2] > 0]:
            if px[2] > 0:
                pixels[int(px[1] + R), int(px[0] + R)] = True;

def reconstitute(hm):
    out = zeros((11, 11), bool)
    flat = [int(v, 16) for v in hm.split(',')]
    n = len(flat)
    n_byte = n
    for i in range(n_byte):
        byte = flat[i]
        for j in range(8):
            k = i * 8 + j
            bit = (byte >> j & 1)
            row, col = divmod(k, 11)
            # print n, n_byte, i, j, k, row, col
            out[row, col] = bit
    return out
def hexmap(flat):
    n = len(flat)
    n_byte, n_bit = divmod(n, 8)

    bytes = zeros(n_byte, uint8)
    for i in range(n_byte):
        b = 0
        for j in range(8):
            k = i * 8 + j
            if k < n:
                b += (flat[k] << j)
        bytes[i] = b
    return ','.join(['0x%02x' % b for b in bytes])

fig, ax = subplots(6, 6, figsize=(10, 8))
ax = ax.ravel()
last_pixels = zeros((N_ROW, N_COL), bool)
i = 0
save = []
def pnt(px):
    for i in range(11):
        row = []
        for j in range(11):
            row.append(' +'[px[i, j]])
        print ''.join(row)
    print

for theta_byt in range(128):
    theta = theta_byt * BYT
    terminator(theta_byt)
    if not array_equal(pixels, last_pixels):
        save.append([theta_byt, hexmap(flat_pixels)])
        save.append([(256 - theta_byt), hexmap(pixels[:,::-1].ravel())])
        ax[i].pcolormesh(pixels.copy(), alpha=1)
        ax[i].axis('equal')
        ax[len(ax) - 1 - i].pcolormesh(pixels[:,::-1].copy(), alpha=1, cmap=cm.binary)
        ax[len(ax) - 1 - i].axis('equal')
        ax[i].set_title(theta_byt)
        ax[i].axis('off')
        i += 1
        last_pixels = pixels.copy()

fig, ax = subplots(5, 9, figsize=(10, 8))
ax = ax.ravel()

sorted = argsort([s[0] for s in save])
save = [save[i] for i in sorted]
print '''\
const uint32_t NEW_MOON_EPOCH = 1546741792; // Jan 6, 2019 2:29:52
const double MOON_PERIOD =  29.530588853 * 86400;
double get_moon_phase(uint32_t gmt){
  uint32_t seconds = gmt - NEW_MOON_EPOCH;
  double phase = 2 * PI * (seconds / MOON_PERIOD - int(seconds / MOON_PERIOD));
  return phase;
}'''

print 'const uint8_t N_MOON_PHASE = %d;' % (len(save) - 1)
print 'const uint8_t MOON_PHASE_STEP = %d;' % (len(flat_pixels) // 8 + 1) ## +1 for theta_byt
print 'const uint8_t moon_phases_11x11[N_MOON_PHASE * MOON_PHASE_STEP] = {'
for i, (theta_byt, hm) in enumerate(save[:-1]):
    print '%3d, %s,' % (theta_byt, hm)
    ax[i].pcolormesh(reconstitute(hm), cmap=cm.binary_r)
    ax[i].axis('off')
    ax[i].axis('equal')
print '};'
for i in range(len(save)-1, len(ax)):
    fig.delaxes(ax[i])

show()
def phase(day):
    phi = 2 * pi * day / 28.
    for row in range(N_ROW):
        for col in range(N_COL):
             theta = arctan2(row - 5, col - 5)
            
