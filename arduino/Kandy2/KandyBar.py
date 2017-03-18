from Tkinter import *
from kandy_commands import *
import serial

r = Tk()
port = sys.argv[1]
ser = serial.Serial(port, baudrate=115200, timeout=1)

class UpDown:
    def __init__(self, parent, label, up_val, down_val, side=LEFT):
        self.parent = parent
        self.up_val = up_val
        self.down_val = down_val
        self.frame = Frame(parent)
        Label(self.frame, text=label).pack()
        Button(self.frame, text="^", command=self.on_up).pack(expand=True)
        Button(self.frame, text="v", command=self.on_down).pack(expand=True)
        self.frame.pack(side=side)        

    def on_up(self):
        write(self.up_val)
        
    def on_down(self):
        write(self.down_val)
        
def write(v):
    ser.write(chr(v % 256))
    
f = Frame(r)    
def send(*args):
    s = e.get()
    try:
        v = int(s)
        write(v)
    except Exception, err:
        print err
        raise
        pass
    
def next_mode(*args):
    write(18)
    
def prev_mode(*args):
    write(19)

def start(*args):
    write(1)
    
def noop(*args):
    pass

e = Entry(f, width = 20)
e.pack(side=LEFT)
Button(f, text="Enter", command=send).pack(side=LEFT)
f.pack()

hhmm = Frame(r)
Label(hhmm, text="Time:").pack(side=LEFT)
UpDown(hhmm, 'HH', 3, 4)
UpDown(hhmm, 'MM', 5, 6)
hhmm.pack()

standby_hhmmss = Frame(r)
Label(standby_hhmmss, text="Countdown:").pack(side=LEFT)
UpDown(standby_hhmmss, 'HH', 10, 11)
UpDown(standby_hhmmss, 'MM', 12, 13)
UpDown(standby_hhmmss, 'SS', 14, 15)
standby_hhmmss.pack()

race_hhmmss = Frame(r)
Label(race_hhmmss, text="Race:").pack(side=LEFT)
UpDown(race_hhmmss, 'HH', 10, 11)
UpDown(race_hhmmss, 'MM', 12, 13)
UpDown(race_hhmmss, 'SS', 14, 15)
race_hhmmss.pack()

f = Frame(r)
UpDown(f, "Mode", 18, 19)
UpDown(f, 'Bright', 16, 17, side=TOP)
f.pack()

Button(r, text="Start", command=start).pack()

r.bind('<Return>', send)
r.mainloop()
