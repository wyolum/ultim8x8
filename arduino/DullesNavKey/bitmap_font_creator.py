import os
import tkFileDialog
from pylab import *
from numpy import *
from Tkinter import *

import sys
    
PIXEL_W = 20

N_ROW = 11
N_COL = 7
digits = [
    0x3c, 0x66, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0x66, 0x3c,
    0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x38, 0x18,
    0xff, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0xe7, 0x7e,
    0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0x07, 0x03, 0x03, 0xe7, 0x7e,
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c,
    0x7e, 0xe7, 0x03, 0x03, 0x07, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xff,
    0x7e, 0xe7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e,
    0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x03, 0xff,
    0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e,
    0x7e, 0xe7, 0x03, 0x03, 0x03, 0x7f, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e,
]

r = Tk()
menubar = Menu(r)
digit_menu = Menu(menubar, tearoff=0)
file_menu = Menu(menubar, tearoff=0)

def curry(func, arg):
    def out():
        return func(arg)
    return out

def load_digit(digit):
    clearme()
    bytes = digits[digit * N_ROW: (digit + 1) * N_ROW]

    for row in range(N_ROW):
        byte = bytes[row]
        for col in range(N_COL):
            if byte >> col & 1:
                setbit(row, col, True)
            else:
                setbit(row, col, False)
    e.delete(0, END)
    e.insert(0, str(digit))

file_options = {}
file_options['defaultextension'] = '.kdf'
file_options['filetypes'] = [('kdf','.kdf')]
file_options['initialdir'] = './'
file_options['parent'] = r
file_options['title'] = 'Kandy Digit Format'

digit_filename = None
def read_file(filename):
    global digit_filename
    digit_file = open(filename)
    digit_lines = digit_file.readlines()
    digits = []
    for i, line in enumerate(digit_lines):
        if '//' in line:
            line = line[0:line.find('//')].strip()
        if line.endswith(','):
            line = line[:-1]
        for d in line.split(','):
            d = d.strip()
            d = d[2:] # remove "0x"
            digits.append(int(d, 16))
    load_digit(0)
    digit_filename = digit_file.name
    r.title(os.path.split(digit_filename)[1][:-4])
    
def open_file():
    global digits
    global digit_filename
    
    digit_file = tkFileDialog.askopenfile(mode='r', **file_options)
    if digit_file:
        digit_filename = digit_file.name
        read_file(digit_filename)

if len(sys.argv) > 1:
    filename = sys.argv[1]
    digits = read_file(filename)
    
def save_digits(digit_file):
    for d in range(10):
        for i in range(16):
            print >> digit_file, '0x%02x,' % digits[d * 16 + i],
        print >> digit_file, '//', d
        
def save_file():
    if digit_filename is None:
        digit_file = tkFileDialog.asksaveasfile(mode='w', **file_options)
    else:
        digit_file = open(digit_filename, 'w')
    if digit_file:
        save_digits(digit_file)
    
def saveas_file():
    digit_file = tkFileDialog.asksaveasfile(mode='w', **file_options)
    if digit_file:
        save_digits(digit_file)
    
for digit in range(10):
    digit_menu.add_command(label=str(digit), command=curry(load_digit, digit))
    
file_menu.add_command(label='Open', command=open_file)
file_menu.add_command(label='Save', command=save_file)
file_menu.add_command(label='Save As', command=save_file)

menubar.add_cascade(label="File", menu=file_menu)
menubar.add_cascade(label="Digits", menu=digit_menu)
r.config(menu=menubar)

can = Canvas(r, width=(N_COL + 2) * PIXEL_W, height=(N_ROW + 2) * PIXEL_W)
pixels = []
for j in range(N_ROW):
    j = N_ROW - j - 1
    pixels.append([])
    for i in range(N_COL):
        i = N_COL - i - 1
        pixels[-1].append(can.create_rectangle(PIXEL_W + i * PIXEL_W,
                                               PIXEL_W + j * PIXEL_W,
                                               PIXEL_W + (i + 1) * PIXEL_W,
                                               PIXEL_W + (j + 1) * PIXEL_W,
                                               fill='white'))

bitmap = zeros((N_ROW, N_COL), bool)

def setbit(row, col, bit):
    bitmap[row, col] = bit
    can.itemconfigure(pixels[row][col], fill=['white', 'black'][bitmap[row, col]])
    
def flipbit(event):
    col, row = event.x / PIXEL_W - 1, event.y / PIXEL_W - 1
    row = N_ROW - row - 1
    col = N_COL - col - 1
    if 0 <= row and row < N_ROW and 0 <= col and col < N_COL:
        setbit(row, col, not bitmap[row, col])
    
def saveme():
    XXX = []
    d = int(e.get())
    for row_i, row in enumerate(bitmap):
        out = 0
        for i, bit in enumerate(row):
            out += 2 ** i * bit
        digits[d * 10 + row_i] = out
        XXX.append('0x%02x' % out)
    print ','.join(XXX) + ',', '//', e.get()

def clearme():
    global bitmap
    
    for i, r in enumerate(pixels):
        for j, e in enumerate(r):
            can.itemconfigure(e, fill='white')
            bitmap[i, j] = False
            
can.bind('<Button-1>', flipbit)
can.pack()
e = Entry(r, w=3)
e.pack()
b = Button(r, text="Save Digit", command=saveme)
b.pack()
b = Button(r, text="clearme", command=clearme)
b.pack()
load_digit(0)
r.mainloop()
