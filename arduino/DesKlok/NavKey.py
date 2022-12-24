from tkinter import *
from numpy import *
import serial

s = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=.1)

def send(msg):
    s.write(msg)
    print msg

DARK = "#101010"
W = 300
H = 300
r = Tk()
c = Canvas(r, width=W, height=H)
c.pack()
c.create_oval([0, 0, W, H], fill="#000000", outline="#808080")
rot = array([[0, -1], [1, 0]])
center = array([W/2, H/2])
triangle = array([[0, 0], [-8, -20], [8, -20]])
down = triangle + [0, H/2] + center
left = dot(rot, (down - center).T).T + center
up = dot(rot, (left - center).T).T + center
right = dot(rot, (up - center).T).T + center
wheel = c.create_oval([50, 50, W - 50, H - 50], fill=DARK, outline="#FFFFFF")
center_b = c.create_oval([center[0] + 20, center[1] + 20, center[0] - 20, center[1] - 20], fill=DARK, outline="#FFFFFF")
down_b = c.create_polygon(list(down.ravel()), fill=DARK, outline="#FFFFFF", width=1)
left_b = c.create_polygon(list(left.ravel()), fill=DARK, outline="#FFFFFF", width=1)
up_b = c.create_polygon(list(up.ravel()), fill=DARK, outline="#FFFFFF", width=1)
right_b = c.create_polygon(list(right.ravel()), fill=DARK, outline="#FFFFFF", width=1)

buttons = [center_b, down_b, left_b, up_b, right_b]

msgs = {center_b:"toggle_sleep",
             down_b:"dimmer",
             up_b:"brighter",
             left_b:"prev_display",
             right_b:"next_display"}
def callback(msg, arg=''):
    send("clockiot/%s//%s" % (msg, arg))

start = [-1, -1]
colorwheel = [0]

def onclick(event):
    item = c.find_closest(event.x, event.y)[0]
    item_type = c.type(item)
    if item in buttons:
        c.itemconfig(item, fill='white')
        callback(msgs[item])
    if item == wheel:
        start[0] = event.x
        start[1] = event.y

def onmotion(event):
    item = c.find_closest(event.x, event.y)[0]
    if start[0] > 0: ### drag motion on wheel
        theta0 = arctan2(start[1] - center[1],
                         start[0] - center[0])
        theta1 = arctan2(event.y - center[1],
                         event.x - center[0])
        dtheta = (theta1 - theta0) * 180 / pi
        dtheta = (dtheta + 180) % 360 - 180
        was_color = int(colorwheel[0])
        colorwheel[0] += dtheta/3.
        colorwheel[0] %= 255
        is_color = int(colorwheel[0])
        if was_color != is_color:
            callback("set_colorwheel", arg=int(colorwheel[0]))
        start[0] = event.x
        start[1] = event.y
def onrelease(event):
    item = c.find_closest(event.x, event.y)[0]
    item_type = c.type(item)
    if item in buttons:
        c.itemconfig(item, fill=DARK)
    if item == wheel:
        start[0] = -1
        start[1] = -1

def right(event):
    colorwheel[0] += 1
    colorwheel[0] %= 255
    callback("set_colorwheel", arg=int(colorwheel[0]))
def left(event):
    colorwheel[0] -= 1
    colorwheel[0] %= 255
    callback("set_colorwheel", arg=int(colorwheel[0]))
c.bind('<Button-1>', onclick)
c.bind('<B1-Motion>', onmotion)
c.bind('<ButtonRelease-1>', onrelease)
r.bind("<Right>", right)
r.bind("<Left>", left)
r.mainloop()
