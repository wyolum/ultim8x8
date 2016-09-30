import os
import tkFileDialog
from pylab import *
from numpy import *
from Tkinter import *
import sys

import sys
    
def hex2int(byte):
    return int(byte, 16)

def byte2bits(byte):
    out = zeros(8, bool)
    for i in range(8):
        out[i] = (byte >> i) & 1
    return out

def bits2byte(bits):
    return dot(bits, [2 ** i for i in range(8)])
assert bits2byte([1, 1, 1, 1, 1, 1, 1, 1]) == 255

class bitmap:
    def __init__(self, bitwidth, bitheight, bytes=None):
        self.n_bit = bitwidth * bitheight
        self.n_byte = self.n_bit // 8 + (self.n_bit % 8 > 0)
        self.w = bitwidth
        self.h = bitheight
        self.bits = zeros(self.n_byte * 8, bool)
        if bytes is not None:
            assert self.n_byte == len(bytes), '%s != %s' % (self.n_byte, len(bytes))
            for i in range(self.n_byte):
                self.bits[i * 8:(i + 1) * 8] = byte2bits(bytes[i])
    def getbit(self, i, j):
        return self.bits[j * self.h + i]
    def setbit(self, i, j, val=True):
        self.bits[j * self.h + i] = val
    def togglebit(self, i, j):
        self.setbit(i, j, not self.getbit(i, j))
    def transpose(self):
        ''' return a new bitmap'''
        out = bitmap(self.h, self.w)
        for i in range(self.h):
            for j in range(self.w):
                val = self.getbit(i, j)
                out.setbit(j, i, val)
        return out
    def getBytes(self):
        bytes = []
        for i in range(self.n_byte):
            bytes.append(bits2byte(self.bits[i * 8: (i + 1) * 8]))
        return bytes
    def __str__(self):
        bytestr = ','.join(['0x%02x' % b for b in self.getBytes()])
        out = '0x%02x,0x%02x,0x%02x::%s' % (self.n_byte, self.w, self.h, bytestr)
        return out
    def ascii_art(self):
        sys.stdout.write('+')
        for j in range(self.w):
            sys.stdout.write('-')
        sys.stdout.write('+\n')
        for i in range(self.h):
            sys.stdout.write('|')
            for j in range(self.w):
                if self.getbit(i, j):
                    sys.stdout.write('X')
                else:
                    sys.stdout.write(' ')
            sys.stdout.write('|\n')
        sys.stdout.write('+')
        for j in range(self.w):
            sys.stdout.write('-')
        sys.stdout.write('+\n')

def str2bitmap(s):
    dims, bytes = s.split('::')
    dims = [hex2int(x) for x in dims.split(',')]
    bytes = [hex2int(x) for x in bytes.split(',')]
    return bitmap(dims[1], dims[2], bytes=bytes)

def test():
    digits = [
      0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, # 0
      0x18,0x1c,0x1e,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xff,0xff, # 1
      0x3c,0x7e,0xe7,0xc3,0xc0,0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x07,0x03,0x03,0xff,0xff, # 2
      0x3c,0x7e,0xe7,0xc3,0xc0,0xc0,0xe0,0x78,0x78,0xe0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, # 3
      0x60,0x63,0x63,0x63,0x63,0x63,0x63,0xff,0xff,0x60,0x60,0x60,0x60,0x60,0x60,0x60, # 4
      0xff,0xff,0x03,0x03,0x03,0x03,0x3f,0x7f,0xe0,0xc0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, # 5
      0x3c,0x7e,0xe7,0xc3,0x03,0x03,0x03,0x3f,0x7f,0xe3,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, # 6
      0xff,0xff,0xc0,0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x06,0x06,0x06,0x06,0x06,0x06,0x06, # 7
      0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xe7,0x7e,0x7e,0xe7,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, # 8
      0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xe7,0xfe,0xfc,0xc0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, # 9
    ]

    c = bitmap(7, 5)
    c.setbit(3, 6, True)
    d = c.transpose()
    c.ascii_art()
    d.ascii_art()
    for digit in range(10):
        bitmap(8, 16, bytes=digits[digit * 16:(digit+1)*16]).ascii_art()
    fat_nat = bitmap(8, 16 * 10, bytes=digits)
    print fat_nat
    here
# test()
justi_narrow_bytes = [0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, # 0
                      0x20, 0x30, 0x38, 0x34, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, # 1
                      0x38, 0x6c, 0xc6, 0xc6, 0xc0, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x06, 0x06, 0x06, 0xfe, # 2
                      0x38, 0x6c, 0xc6, 0xc6, 0xc0, 0xc0, 0xc0, 0x70, 0xc0, 0xc0, 0xc0, 0xc0, 0xc6, 0xc6, 0x6c, 0x38, # 3
                      0xc0, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, # 4
                      0xfe, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7e, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc6, 0xc6, 0x7c, # 5
                      0x78, 0xc4, 0xc4, 0x06, 0x06, 0x06, 0x06, 0x3e, 0x66, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x44, 0x38, # 6
                      0xfe, 0xfe, 0xc0, 0xc0, 0xe0, 0x70, 0x38, 0x1c, 0x0e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, # 7
                      0x38, 0x44, 0xc6, 0xc6, 0xc6, 0xc6, 0x44, 0x38, 0x44, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x44, 0x38, # 8
                      0x38, 0x44, 0xc6, 0xc6, 0xc6, 0xc6, 0xc4, 0xf8, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x46, 0x24, 0x18] # 9
justi_narrow = bitmap(8, 160, justi_narrow_bytes)

font = justi_narrow

def curry(func, arg):
    def out():
        return func(arg)
    return out
    
class PixelPainter():
    pixel_w = 20
    file_options = {'defaultextension':'.xpm2',
                    'filetypes':[('xpm2','.xpm2')],
                    'initialdir':'./',
                    'title':'XPM2'}
    
    def __init__(self, n_row=16, n_col=8):
        self.filename = None
        self.modified = False
        self.r_bitmap = None
        self.g_bitmap = None
        self.b_bitmap = None
        
        self.r = Tk()
        self.menubar = Menu(self.r)
        self.file_menu = Menu(self.menubar, tearoff=0)
        self.file_menu.add_command(label='New', command=self.new_file, underline=0)
        self.file_menu.add_command(label='Open', command=self.open_file, underline=0)
        self.file_menu.add_command(label='Save', command=self.save, underline=0)
        self.file_menu.add_command(label='Save As', command=self.save_as, underline=5)
        self.menubar.add_cascade(label="File", menu=self.file_menu, underline=0)
        self.r.config(menu=self.menubar)

        
        self.r.title('')
        self.n_row_var = IntVar(self.r)
        self.n_row_var.set(n_row)
        self.n_col_var = IntVar(self.r)
        self.n_col_var.set(n_col)
        
        self.menubar = Menu(self.r)
        f = Frame(self.r)
        Label(f, text='W:').pack(side=LEFT)
        Entry(f, w=3, textvariable=self.n_col_var).pack(side=LEFT)
        Label(f, text='H:').pack(side=LEFT)
        Entry(f, w=3, textvariable=self.n_row_var).pack(side=LEFT)
        Button(f, text="resize", command=self.resize).pack()
        f.pack()

        self.can = Canvas(self.r,
                          width=(self.n_col_var.get() + 0) * self.pixel_w,
                          height=(self.n_row_var.get() + 0) * self.pixel_w,
        )
        self.can.bind('<Button-1>', self.on_canvas_click)
        self.can.bind('<B1-Motion>', self.on_canvas_drag)
        self.can.config(cursor='dotbox')
        self.resize()
        self.can.pack(side=TOP)
        self.pallette = Canvas(self.r,
                               height = (1 + 2) * self.pixel_w,
                               width =  (8 + 2) * self.pixel_w,
                               )
        self.pallette.bind('<Button-1>', self.change_color)
        self.colors = ['#000000', '#FF0000', '#00FF00', '#0000FF', '#00FFFF', '#FF00FF', '#FFFF00', '#FFFFFF']

        self.show_current_color = self.pallette.create_rectangle(self.pixel_w * (4 + 0), self.pixel_w * (0 + 0),
                                                            self.pixel_w * (4 + 1), self.pixel_w * (0 + 1),
                                                            outline='white', fill='#FFFFFF')
        for i, color in enumerate(self.colors):
            self.pallette.create_rectangle(self.pixel_w * (i + 1), self.pixel_w * (1 + 0),
                                           self.pixel_w * (i + 2), self.pixel_w * (1 + 1),
                                           outline='white', fill=color)
        self.pallette.pack()

        f = Frame(self.r)
        Button(f, text="Print", command=self.printbitmap).pack(side=LEFT)
        Button(f, text="Clear", command=self.clear).pack(side=LEFT)
        f.pack(side=TOP)
        self.r.mainloop()

    def open_file(self):
        self.modified = False
        self.file_options['parent'] = self.r
        file = tkFileDialog.askopenfile(mode='r', **self.file_options)
        if file:
            self.filename = os.path.abspath(file.name)
            head = file.readline().strip()
            assert '! XPM2' == head
            counts = file.readline().strip().split()
            assert len(counts) == 4, 'corrupted xpm2 file, got %s counts instead of 4' % len(counts)
            n_col, n_row, n_color, n_char = map(int, counts)
            self.n_col_var.set(n_col)
            self.n_row_var.set(n_row)
            self.resize()
            assert n_color == 8, 'only 8 colors supported, got %d' % n_color
            file_colors = {}
            for i in range(n_color):
                line = file.readline().strip().split()
                assert len(line) == 3, 'expected 3 elements in color def line got %s' % line
                c, x, color = line
                color = color.replace('01', 'FF')
                assert color in self.colors, 'Color %s not supported' % color
                file_colors[c] = color
            for row in range(n_row):
                line = file.readline().strip()
                for col in range(n_col):
                    color = file_colors[line[col]]
                    self.r_bitmap.setbit(col, row, color[1].upper() == 'F')
                    self.g_bitmap.setbit(col, row, color[3].upper() == 'F')
                    self.b_bitmap.setbit(col, row, color[5].upper() == 'F')
                    self.paintbit(row, col, color)
            self.r.title(' ' + os.path.split(self.filename)[1])
            self.modified = False
    def new_file(self):
        self.filename = None
        self.modified = False
        self.resize()
        self.r.title('')
    def save(self):
        if self.filename is not None:
            file = open(self.filename, 'w')
            self.save_xpm2(file)
        else:
            self.save_as()
    def save_as(self):
        file = tkFileDialog.asksaveasfile(mode='w', **self.file_options)
        if file:
            self.save_xpm2(file)
        self.modified = False
        
    def save_xpm2(self, file):
        print >> file, self.toxpm2()
        self.filename = os.path.abspath(file.name)
        self.r.title(' ' + os.path.split(self.filename)[1])
        
    def change_color(self, event):
        col, row = event.x / self.pixel_w - 1, event.y // self.pixel_w - 1
        if row == 0:
            self.pallette.itemconfig(self.show_current_color, fill=self.colors[col])

    def on_canvas_drag(self, event):
        self.on_canvas_click(event)

    def printbitmap(self):
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()
        
        print str(self.r_bitmap)
        print str(self.g_bitmap)
        print str(self.b_bitmap)
        print

    def toxpm2(self):
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()
        
        out = ['! XPM2',
               '%d %d %d %d' % (n_col, n_row, 8, 1)]
        for i, c in enumerate('abcdefgh'):
            out.append(' '.join([c, 'c', self.colors[i].replace('FF', '01')]))
        for i in range(n_row):
            row = []
            for j in range(n_col):
                color = '#' + (('00', 'FF')[self.r_bitmap.getbit(j, i)] +
                               ('00', 'FF')[self.g_bitmap.getbit(j, i)] +
                               ('00', 'FF')[self.b_bitmap.getbit(j, i)])
                row.append('abcdefgh'[self.colors.index(color)])
            out.append(''.join(row))
        return '\n'.join(out)
    
    def printxpm2(self):
        print self.toxpm2()
        
    def resize(self):
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()
        
        self.r_bitmap = bitmap(n_row, n_col)
        self.g_bitmap = bitmap(n_row, n_col)
        self.b_bitmap = bitmap(n_row, n_col)

        self.can.config(width=n_col * self.pixel_w,
                        height=n_row * self.pixel_w)
        self.can.delete(all)
        self.pixels = []
        for j in range(self.n_col_var.get()):
            self.pixels.append([])
            for i in range(self.n_row_var.get()):
                self.pixels[-1].append(self.can.create_rectangle(j * self.pixel_w,
                                                                 i * self.pixel_w,
                                                                 (j + 1) * self.pixel_w,
                                                                 (i + 1) * self.pixel_w,
                                                                 fill='black',
                                                                 outline='white'))
    def on_canvas_click(self, event):
        col, row = event.x / self.pixel_w, event.y / self.pixel_w
        current_color = self.pallette.itemcget(self.show_current_color, 'fill')
        self.paintbit(row, col, current_color)
        
    def paintbit(self, row, col, color):
        if 0 <= row and row < self.n_row_var.get() and 0 <= col and col < self.n_col_var.get():
            if not self.modified:
                self.modified = True
                if self.filename is not None:
                    title = os.path.split(self.filename)[1]
                else:
                    title = ''
                self.r.title('*' + title)
            r = color[1].upper() == 'F'
            g = color[3].upper() == 'F'
            b = color[5].upper() == 'F'
            self.r_bitmap.setbit(col, row, r)
            self.g_bitmap.setbit(col, row, g)
            self.b_bitmap.setbit(col, row, b)
            self.can.itemconfig(self.pixels[col][row], fill=color)
    def clear(self):
        self.resize()
        
p = PixelPainter()
