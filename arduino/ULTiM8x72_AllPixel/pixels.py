from numpy import *
import struct
import os

class Pixels:
    def __init__(self, n_row, n_col):
        self.max_pos = 0
        self.n_row = n_row
        self.n_col = n_col
        self.data_len = (n_row * n_col * 3) + 3
        self.data = zeros((n_row * n_col, 3), uint8)
        
        self.pixel_data_cmd = chr(2) + struct.pack('>H', self.data_len)
        self.brightness_cmd = chr(3) + struct.pack('>H', 4) + chr(1)

    def setBrightness(self, ser, v):
        brightness_cmd = chr(3) + struct.pack('>H', 4) + chr(v)
        ser.write(brightness_cmd)

    def clear(self):
        self.data[:,:] = 0
        
    def setPixel(self, row, col, color):
        if 0 <= row and row < self.n_row and 0 <= col and col < self.n_col:
            # pos = row * 56 + col
            pos = self.snake(row, col)
            if pos > self.max_pos:
                # print 'max_pos', pos, row, col, self.n_row, self.n_col
                self.max_pos = pos
            if pos > 0 and pos < len(self.data):
                self.data[pos] = color

    def setImage(self, row, col, xpm2):
        for r in arange(xpm2.n_row):
            for c in arange(xpm2.n_col):
                self.setPixel(r + row, c + col, xpm2.getPixel(r, c))
                         
    def snake(self, row, col):
        if row >= 8 and row < 16:
            out = self.snake(15 - row, 55 - col) + self.n_col * 8
            return out
        if row >= 16:
            return self.snake(row - 16, col) + self.n_col * 16
        ## single 8x8 row
        if col % 2 == 0:
            return col * 8 + 7 - row
        else:
            return col * 8 + row
                         
    def stream(self, ser):
        msg = self.pixel_data_cmd + self.data.tostring()
        ser.write(msg)

    def asciiart(self):
        print sum(self.data)
        for r in range(self.n_row):
            for c in range(self.n_col):
                if sum(self.data[self.snake(r, c)]):
                    print sum(self.data[self.snake(r, c)]),
                else:
                    print ' ',
            print

class XPM2:
    def __init__(self, filename):
        file = open(filename)
        self.filename = os.path.abspath(file.name)

        head = file.readline().strip()
        assert '! XPM2' == head
        counts = file.readline().strip().split()
        assert len(counts) == 4, 'corrupted xpm2 file, got %s counts instead of 4' % len(counts)
        n_col, n_row, n_color, n_char = map(int, counts)

        file_colors = {}
        for i in range(n_color):
            line = file.readline().strip().split()
            assert len(line) == 3, 'expected 3 elements in color def line got %s' % line
            c, x, color = line
            file_colors[c] = color
        self.pixels = zeros((n_row, n_col, 3), uint8)
        for row in range(n_row):
            line = file.readline().strip()
            for col in range(n_col):
                color = file_colors[line[col]]
                self.setPixel(row, col, color) 
        self.n_row = n_row
        self.n_col = n_col
    def setPixel(self, row, col, color): ## was paintbit
        '''
        Change color of pixel at row/col to color.
        '''
        r = int(color[1:3], 16)
        g = int(color[3:5], 16)
        b = int(color[5:7], 16)
        self.pixels[row, col] = [r, g, b]
    def getPixel(self, row, col):
        return self.pixels[row, col]

    def asciiart(self):
        for r in range(self.n_row):
            for c in range(self.n_col):
                if sum(self.pixels[r, c]) > 0:
                    print 'X',
                else:
                    print ' ',
            print ''
    def flip_rl(self):
        self.pixels = self.pixels[:,::-1]

    def replace_color(self, frm, _to):
        for r in range(len(self.pixels)):
            for c in range(len(self.pixels[r])):
                if linalg.norm(self.pixels[r, c] - frm) == 0:
                    self.pixels[r, c] = _to
                
