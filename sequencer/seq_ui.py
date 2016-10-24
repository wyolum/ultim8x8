import os
import tkFileDialog
from pylab import *
from numpy import *
from Tkinter import *
import sys
from argparse import ArgumentParser
from bibliopixel import colors
from bibliopixel.drivers.network import DriverNetwork


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
    '''
    0-1 2D bitmap.  Each pixel is either on or off.  You can use 3 bitmaps to define an 8-color RGB bitmap.
    '''

    def __init__(self, bitwidth, bitheight, bytes=None):
        '''
        Create a new bitmap.
        bitwidth  -- how many pixels wide the bitmap is
        bitheight -- how many pixels tall the bitmap is
        '''
        self.n_bit = bitwidth * bitheight
        self.n_byte = self.n_bit // 8 + (self.n_bit % 8 > 0)
        self.w = bitwidth
        self.h = bitheight
        self.bits = zeros(self.n_byte * 8, bool)
        if bytes is not None:
            assert self.n_byte == len(bytes), '%s != %s' % (
                self.n_byte, len(bytes))
            for i in range(self.n_byte):
                self.bits[i * 8:(i + 1) * 8] = byte2bits(bytes[i])

    def getbit(self, i, j):
        return self.bits[j * self.h + i]

    def setbit(self, i, j, val=True):
        self.bits[j * self.h + i] = val

    def togglebit(self, i, j):
        self.setbit(i, j, not self.getbit(i, j))

    def transpose(self):
        '''
        Return a new bitmap transposed along the main diagonal.
        '''
        out = bitmap(self.h, self.w)
        for i in range(self.h):
            for j in range(self.w):
                val = self.getbit(i, j)
                out.setbit(j, i, val)
        return out

    def getBytes(self):
        '''
        Return an tightly packed array of bytes by rows (8 pixels per byte).
        If the rows are not an multiple of 8, the end of one row
        will share a byte with the beginning of the next row.
        '''
        bytes = zeros(self.n_byte, uint8)
        for i in range(self.n_byte):
            bytes[i] = bits2byte(self.bits[i * 8: (i + 1) * 8])
        return bytes

    def __str__(self):
        '''
        Return a hex string with the format:
        n_byte,n_col,n_row:: ...bytes...
        '''
        bytestr = ','.join(['0x%02x' % b for b in self.getBytes()])
        out = '0x%02x,0x%02x,0x%02x::%s' % (
            self.n_byte, self.w, self.h, bytestr)
        return out

    def ascii_art(self):
        '''
        Print out representation of bitmap to stdout
        '''
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
    '''
    convert a hex string to a bitmap (see bitmap.__str__)
    '''
    dims, bytes = s.split('::')
    dims = [hex2int(x) for x in dims.split(',')]
    bytes = [hex2int(x) for x in bytes.split(',')]
    return bitmap(dims[1], dims[2], bytes=bytes)


def curry(func, arg):
    def out():
        return func(arg)
    return out


class Pallette:
    '''
    Color pallette
    '''

    def __init__(self, parent, pixel_w=20):
        self.parent = parent
        self.pixel_w = pixel_w
        self.can = Canvas(self.parent,
                          height=(1 + 2) * self.pixel_w,
                          width=(8 + 2) * self.pixel_w)

        self.colors = ['#000000', '#FF0000', '#00FF00',
                       '#0000FF', '#00FFFF', '#FF00FF', '#FFFF00', '#FFFFFF']
        self.can.bind('<Button-1>', self.change_color)
        self.show_current_color = self.can.create_rectangle(self.pixel_w * (4 + 0), self.pixel_w * (0 + 0),
                                                            self.pixel_w *
                                                            (4 + 1), self.pixel_w *
                                                            (0 + 1),
                                                            outline='white', fill='#FFFFFF')
        for i, color in enumerate(self.colors):
            self.can.create_rectangle(self.pixel_w * (i + 1), self.pixel_w * (1 + 0),
                                      self.pixel_w *
                                      (i + 2), self.pixel_w * (1 + 1),
                                      outline='white', fill=color)
        self.can.pack()

    def change_color(self, event):
        '''
        Change the current pallete color.
        '''
        col, row = event.x // self.pixel_w - 1, event.y // self.pixel_w - 1
        if row == 0:
            self.can.itemconfig(self.show_current_color, fill=self.colors[col])

    def get_current_color(self):
        return self.can.itemcget(self.show_current_color, 'fill')


class Pixels:

    def __init__(self, can, n_col, n_row, pixel_w=20, pixel_update_callback=None):
        self.can = can
        self.n_col = n_col
        self.n_row = n_row
        self.pixel_w = pixel_w
        self.pixel_update_callback = pixel_update_callback
        self.__pixels = []
        for j in range(self.n_col):
            self.__pixels.append([])
            for i in range(self.n_row):
                self.__pixels[-1].append(self.can.create_rectangle(j * self.pixel_w,
                                                                   i * self.pixel_w,
                                                                   (j + 1) *
                                                                   self.pixel_w,
                                                                   (i + 1) *
                                                                   self.pixel_w,
                                                                   fill='#000000',
                                                                   outline='#FFFFFF',
                                                                   tags=("layer_0", "row_%d" % i, "col_%d" % j)))

    def do_update_callback(self):
        if self.pixel_update_callback:
            self.pixel_update_callback(self.get_pixel_color_matrix())

    def set_pixel_color(self, row, col, color, layer=None):
        if (0 <= row and row < self.n_row and
                0 <= col and col < self.n_col):
            if layer is None or layer == 0:
                self.can.itemconfig(self.__pixels[col][row], fill=color)
            else:
                self.can.create_rectangle(col * self.pixel_w,
                                          row * self.pixel_w,
                                          (col + 1) * self.pixel_w,
                                          (row + 1) * self.pixel_w,
                                          fill=color,
                                          outline='white',
                                          tags=("layer_%d" % layer, "row_%d" % row, "col_%d" % col))

        # self.do_update_callback()

    def get_pixel_color(self, row, col):
        if (0 <= row and row < self.n_row and
                0 <= col and col < self.n_col):
            out = self.can.itemcget(self.__pixels[col][row], 'fill')
        else:
            out = None
        return out

    def get_pixel_color_matrix(self):
        out = []
        for y in xrange(self.n_row):
            row = []
            for x in xrange(self.n_col):
                color = self.can.itemcget(self.__pixels[x][y], 'fill')
                row.extend(colors.hex2rgb(color))
            out.extend(row)
        return out

    def get_pixel_coords(self, event):
        return event.x // self.pixel_w, event.y // self.pixel_w


class Tool:

    def __init__(self):
        '''
        You must assign pixels and pallette before use
        '''
        self.pixels = None
        self.pallette = None
        self.start_pixel = None
        self.current_pixel = None
        self.end_pixel = None

    def draw_pixel(self, row, col, layer=0):
        '''
        change pixel at row, col to current pallette color
        '''
        color = self.pallette.get_current_color()
        if color != self.pixels.get_pixel_color(row, col):
            self.pixels.set_pixel_color(row, col, color, layer)

    def get_pixel_coords(self, event):
        return self.pixels.get_pixel_coords(event)

    def on_b1_down(self, event):
        self.start_pixel = self.pixels.get_pixel_coords(event)
        self.current_pixel = self.start_pixel
        self.end_pixel = self.start_pixel
        self.draw_pixel(self.start_pixel[1], self.start_pixel[0], layer=1)

    def on_b1_motion(self, event):
        new_pixel = self.get_pixel_coords(event)
        out = False
        if self.current_pixel is None:
            self.current_pixel = (-1, -1)
        if (new_pixel[0] != self.current_pixel[0] or
                new_pixel[1] != self.current_pixel[1]):
            self.current_pixel = new_pixel
            out = True
        return out

    def on_b1_release(self, event):
        self.end_pixel = self.get_pixel_coords(event)
        self.on_drag_release(event)
        self.finalize()

    def finalize(self):
        # copy layer_1 down to layer_0
        def get_row_from_tags(tags):
            out = None
            for tag in tags:
                if tag.startswith('row_'):
                    out = int(tag[4:])
                    break
            return out

        def get_col_from_tags(tags):
            out = None
            for tag in tags:
                if tag.startswith('col_'):
                    out = int(tag[4:])
                    break
            return out
        for pid in self.pixels.can.find_withtag('layer_1'):
            tags = self.pixels.can.gettags(pid)
            row = get_row_from_tags(tags)
            col = get_col_from_tags(tags)
            color = self.pixels.can.itemcget(pid, 'fill')
            self.pixels.set_pixel_color(row, col, color, layer=0)
            self.pixels.can.delete(pid)

    def on_click(self, event):
        '''
        Press and release on on same pixel
        '''
        pass

    def on_drag_release(self, event):
        '''
        Drag event complete.
        '''
        pass


class Pencil(Tool):

    def on_b1_motion(self, event):
        if Tool.on_b1_motion(self, event):
            self.draw_pixel(self.current_pixel[
                            1], self.current_pixel[0], layer=1)


class Line(Tool):

    def on_b1_motion(self, event):
        Tool.on_b1_motion(self, event)
        # delete old line
        for pid in self.pixels.can.find_withtag('layer_1'):
            self.pixels.can.delete(pid)

        # draw new line
        start = array(self.start_pixel, float)
        stop = array(self.current_pixel, float)
        d = stop - start
        l = linalg.norm(d)
        if l > 0:
            d /= l
            for t in arange(0, l, .1):
                p = (start + t * d).astype(int)
                self.draw_pixel(p[1], p[0], layer=1)


class Circle(Tool):

    def __init__(self, fill=False):
        Tool.__init__(self)
        self.fill = fill

    def on_b1_motion(self, event):
        Tool.on_b1_motion(self, event)
        # delete old circle
        for pid in self.pixels.can.find_withtag('layer_1'):
            self.pixels.can.delete(pid)

        # draw new circle
        center = array(self.start_pixel, float) + [.5, .5]
        edge = array(self.current_pixel, float)
        r = linalg.norm(center - edge)
        if r > 0:
            if self.fill:
                x, y = meshgrid(arange(self.pixels.n_row),
                                arange(self.pixels.n_col))
                yx = transpose([y.ravel(), x.ravel()])
                d = linalg.norm(center[newaxis] - yx, axis=1)
                for i in range(len(d)):
                    if d[i] <= r:
                        self.draw_pixel(yx[i, 1], yx[i, 0], layer=1)

            else:
                c = 2 * pi * r
                for theta in arange(0, 2 * pi, .1 / c):
                    d = array([cos(theta), sin(theta)])
                    p = (center + r * d).astype(int)
                    self.draw_pixel(p[1], p[0], layer=1)
        else:
            self.draw_pixel(int(center[1]), int(center[0]), layer=1)


class Box(Tool):

    def on_b1_motion(self, event):
        Tool.on_b1_motion(self, event)

        # delete old box
        for pid in self.pixels.can.find_withtag('layer_1'):
            self.pixels.can.delete(pid)

        # draw new box
        start = array(self.start_pixel)
        stop = array(self.current_pixel)

        x_start = min([start[0], stop[0]])
        x_stop = max([start[0], stop[0]])
        y_start = min([start[1], stop[1]])
        y_stop = max([start[1], stop[1]])
        for row in range(y_start, y_stop + 1):
            for col in range(x_start, x_stop + 1):
                self.draw_pixel(row, col, layer=1)


class PixelPainter():
    '''
    RGB bit map editor.  TODO: separate classes for rgb_bitmap and GUI
    '''
    pixel_w = 20  # on screen dimension
    file_options = {'defaultextension': '.xpm2',
                    'filetypes': [('xpm2', '.xpm2')],
                    'initialdir': './',
                    'title': 'XPM2'}

    def __init__(self, n_row=16, n_col=8, display_ip=None, display_port=3142):
        self.display_ip = display_ip
        self.display_port = display_port

        self.filename = None
        self.modified = False
        self.pixels = None
        self.pallette = None

        # set up GUI
        self.r = Tk()
        self.menubar = Menu(self.r)
        self.file_menu = Menu(self.menubar, tearoff=0)
        self.file_menu.add_command(
            label='New', command=self.new_file, underline=0)
        self.file_menu.add_command(
            label='Open', command=self.open_file, underline=0)
        self.file_menu.add_command(
            label='Save', command=self.save, underline=0)
        self.file_menu.add_command(
            label='Save As', command=self.save_as, underline=5)
        self.menubar.add_cascade(
            label="File", menu=self.file_menu, underline=0)
        self.tool_menu = Menu(self.menubar, tearoff=1)
        self.tool_menu.add_command(label="Pencil", command=curry(
            self.change_tool, 0), underline=0)
        self.tool_menu.add_command(label="Line", command=curry(
            self.change_tool, 1), underline=0)
        self.tool_menu.add_command(label="Circle", command=curry(
            self.change_tool, 2), underline=0)
        self.tool_menu.add_command(label="Filled Circle", command=curry(
            self.change_tool, 3), underline=0)
        self.tool_menu.add_command(label="Box", command=curry(
            self.change_tool, 4), underline=0)
        self.menubar.add_cascade(
            label="Tools", menu=self.tool_menu, underline=0)

        self.r.config(menu=self.menubar)
        self.r.title('')
        self.n_row_var = IntVar(self.r)
        self.n_row_var.set(n_row)
        self.n_col_var = IntVar(self.r)
        self.n_col_var.set(n_col)
        self.menubar = Menu(self.r)

        # f = Frame(self.r)
        # Label(f, text='W:').pack(side=LEFT)
        # Entry(f, w=3, textvariable=self.n_col_var).pack(side=LEFT)
        # Label(f, text='H:').pack(side=LEFT)
        # Entry(f, w=3, textvariable=self.n_row_var).pack(side=LEFT)
        # Button(f, text="resize", command=self.resize).pack()
        # f.pack()

        self.can = Canvas(self.r,
                          width=(self.n_col_var.get() + 0) * self.pixel_w,
                          height=(self.n_row_var.get() + 0) * self.pixel_w)
        self.can.bind('<Button-1>', self.on_canvas_click)
        self.can.bind('<B1-Motion>', self.on_canvas_drag)
        self.can.bind('<ButtonRelease-1>', self.on_canvas_release)
        self.pencil = Pencil()
        self.line = Line()
        self.circle = Circle()
        self.filled_circle = Circle(fill=True)
        self.box = Box()
        self.cursors = ['pencil', 'crosshair',
                        'circle', 'dot', 'top_left_corner']
        self.tools = [self.pencil, self.line,
                      self.circle, self.filled_circle, self.box]

        self.driver = None
        if self.display_ip:
            self.driver = DriverNetwork(width=n_col, height=n_row,
                                        host=self.display_ip, port=self.display_port)

        self.resize()
        self.can.pack(side=TOP)
        self.pallette = Pallette(self.r, self.pixel_w)
        for tool in self.tools:
            tool.pallette = self.pallette
        self.change_tool(0)

        f = Frame(self.r)
        Button(f, text="Print", command=self.printbitmap).pack(side=LEFT)
        # Button(f, text="Print", command=self.tobparray).pack(side=LEFT)
        Button(f, text="Clear", command=self.clear).pack(side=LEFT)
        f.pack(side=TOP)

        self.r.mainloop()

    def change_tool(self, index):
        self.current_tool = self.tools[index]
        self.can.config(cursor=self.cursors[index])

    def open_file(self):
        '''
        File dialog to open XPM2 formated file.
        '''
        self.modified = False
        self.file_options['parent'] = self.r
        file = tkFileDialog.askopenfile(mode='r', **self.file_options)
        if file:
            self.filename = os.path.abspath(file.name)
            head = file.readline().strip()
            assert '! XPM2' == head
            counts = file.readline().strip().split()
            assert len(
                counts) == 4, 'corrupted xpm2 file, got %s counts instead of 4' % len(counts)
            n_col, n_row, n_color, n_char = map(int, counts)
            self.n_col_var.set(n_col)
            self.n_row_var.set(n_row)
            self.resize()
            assert n_color == 8, 'only 8 colors supported, got %d' % n_color
            file_colors = {}
            for i in range(n_color):
                line = file.readline().strip().split()
                assert len(
                    line) == 3, 'expected 3 elements in color def line got %s' % line
                c, x, color = line
                color = color.replace('01', 'FF')
                assert color in self.pallette.colors, 'Color %s not supported' % color
                file_colors[c] = color
            for row in range(n_row):
                line = file.readline().strip()
                for col in range(n_col):
                    color = file_colors[line[col]]
                    self.paintbit(row, col, color)
            self.pixels.do_update_callback()
            self.r.title(' ' + os.path.split(self.filename)[1])
            self.modified = False

    def new_file(self):
        '''
        Start over with blank bitmap.
        '''
        self.filename = None
        self.modified = False
        self.resize()
        self.r.title('')

    def save(self):
        '''
        Save current bitmap to current filename (if not None) else Save As
        '''
        if self.filename is not None:
            file = open(self.filename, 'w')
            self.save_xpm2(file)
        else:
            self.save_as()

    def save_as(self):
        '''
        File dialog to save current bitmap.
        '''
        file = tkFileDialog.asksaveasfile(mode='w', **self.file_options)
        if file:
            self.save_xpm2(file)
        self.modified = False

    def save_xpm2(self, file):
        '''
        Write xmp2 formated bitmap to open file.
        '''
        print >> file, self.toxpm2()
        self.filename = os.path.abspath(file.name)
        self.r.title(' ' + os.path.split(self.filename)[1])

    def printbitmap(self):
        '''
        Print bitmap in C-friendly format.
        '''
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()

        r_bitmap = bitmap(n_row, n_col)
        g_bitmap = bitmap(n_row, n_col)
        b_bitmap = bitmap(n_row, n_col)
        for r in range(n_row):
            for c in range(n_col):
                color = self.pixels.get_pixel_color(r, c)
                r_bitmap.setbit(c, r, color[1].upper() == 'F')
                g_bitmap.setbit(c, r, color[3].upper() == 'F')
                b_bitmap.setbit(c, r, color[5].upper() == 'F')
        print str(r_bitmap)
        print str(g_bitmap)
        print str(b_bitmap)

    def toxpm2(self):
        '''
        Return string of bitmap in XPM2 format.
        '''
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()

        out = ['! XPM2',
               '%d %d %d %d' % (n_col, n_row, 8, 1)]
        for i, c in enumerate('abcdefgh'):
            out.append(
                ' '.join([c, 'c', self.pallette.colors[i].replace('FF', '01')]))
        for i in range(n_row):
            row = []
            for j in range(n_col):
                color = self.pixels.get_pixel_color(i, j)
                row.append('abcdefgh'[self.pallette.colors.index(color)])
            out.append(''.join(row))
        return '\n'.join(out)

    def pixel_update_callback(self, data):
        if self.driver:
            self.driver.update(data)

    def printxpm2(self):
        print self.toxpm2()

    def resize(self):
        '''
        Resize and clear the display based on GUI size elements.
        '''
        n_col = self.n_col_var.get()
        n_row = self.n_row_var.get()

        self.can.config(width=n_col * self.pixel_w,
                        height=n_row * self.pixel_w)
        self.can.delete(all)
        self.pixels = Pixels(self.can, n_col, n_row,
                             self.pixel_w, self.pixel_update_callback)
        for tool in self.tools:
            tool.pixels = self.pixels

        self.pixels.do_update_callback()

    def on_canvas_drag(self, event):
        self.current_tool.on_b1_motion(event)

    def on_canvas_release(self, event):
        self.current_tool.on_b1_release(event)
        self.pixels.do_update_callback()

    def on_canvas_click(self, event):
        '''
        Paint the pixel under the cursor the color given by the pallette show_current_color square.
        '''
        self.current_tool.on_b1_down(event)

    def paintbit(self, row, col, color):
        '''
        Change color of pixel at row/col to color.
        '''
        if 0 <= row and row < self.n_row_var.get() and 0 <= col and col < self.n_col_var.get():
            if not self.modified:
                self.modified = True
                if self.filename is not None:
                    title = os.path.split(self.filename)[1]
                else:
                    title = ''
                self.r.title('*' + title)
            color[1].upper() == 'F'  # red
            color[3].upper() == 'F'  # green
            color[5].upper() == 'F'  # blue
            self.pixels.set_pixel_color(row, col, color)

    def clear(self):
        '''
        Clear bitmap
        '''
        self.resize()

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--width", help="Display width", default=8, type=int)
    parser.add_argument("--height", help="Display height",
                        default=16, type=int)
    parser.add_argument(
        "--display_ip", help="Remote Display IP", default=None, type=str)
    parser.add_argument(
        "--display_port", help="Remote Display Port", default=3142, type=int)
    args = parser.parse_args()

    p = PixelPainter(n_row=args.height, n_col=args.width,
                     display_ip=args.display_ip, display_port=args.display_port)
