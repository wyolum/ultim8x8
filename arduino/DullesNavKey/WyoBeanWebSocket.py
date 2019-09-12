from __future__ import print_function
import json
import tkinter
import websocket
import glob
import re
import time

root = tkinter.Tk()

ESP32_IP = [192,168,7,93]
esp32_vars = [tkinter.IntVar() for i in range(4)]
[esp32_vars[i].set(ESP32_IP[i]) for i in range(4)]

ws = websocket.WebSocket(timeout=1)
ws.settimeout(1)

def hello():
    print ("hello")

def tap():
    port = '81'
    ESP32_IP = esp32_ip.get()
    ip_str = 'ws://%s:%s/' % (ESP32_IP, port)
    print (ip_str, end=' ')
    out = False
    for i in range(3):
        try:
            ws.connect(ip_str)
            out = True
            break
        except:
            pass
    print (out)
    return out
    
def send_msg(msg):
    port = '81'
    ESP32_IP = esp32_ip.get()
    ip_str = 'ws://%s:%s/' % (ESP32_IP, port)
    print(ip_str)
    try:
        ws.connect(ip_str)
    except:
        print("FAILED")
        return 
    ws.send(msg)
    greeting1 = ws.recv()
    try:
        response = ws.recv()
        print (response)
    except:
        response = None
    result = ws.close()
    print(greeting1)
    return response

def brighter():
    send_msg('clockiot/brighter')
def dimmer():
    send_msg('clockiot/dimmer')
def send_mqtt_ip():
    bytes = []
    for i in range(4):
        bytes.append(mqtt_ip[i].get())
    msg = 'clockiot/mqtt_ip//' + '.'.join(bytes)
    send_msg(msg)

def flip_display():
    send_msg('clockiot/flip_display')
    
def next_display():
    send_msg('clockiot/next_display')

def set_time():
    send_msg('clockiot/set_time//0')

def use_ntp():
    send_msg('clockiot/use_ntp')

def set_display_idx():
    send_msg('clockiot/display_idx//0')
def get_displays():
    send_msg('clockiot/get_displays')

faceplate_idx = 0
def set_faceplate_idx():
    global faceplate_idx
    faceplate_idx += 1
    send_msg('clockiot/faceplate_idx//%d' % faceplate_idx)
def get_faceplates():
    send_msg('clockiot/get_faceplates')
    

menubar = tkinter.Menu(root)

# create a pulldown menu, and add it to the menu bar
filemenu = tkinter.Menu(menubar, tearoff=0)
filemenu.add_command(label="Open", command=hello)
filemenu.add_command(label="Save", command=hello)
filemenu.add_separator()
filemenu.add_command(label="Exit", command=root.quit)
menubar.add_cascade(label="File", menu=filemenu)

# create more pulldown menus
editmenu = tkinter.Menu(menubar, tearoff=0)
editmenu.add_command(label="Cut", command=hello)
editmenu.add_command(label="Copy", command=hello)
editmenu.add_command(label="Paste", command=hello)
menubar.add_cascade(label="Edit", menu=editmenu)

toolsmenu = tkinter.Menu(menubar, tearoff=0)
    
menubar.add_cascade(label="Tools", menu=toolsmenu)

# display the menu
root.config(menu=menubar)
esp32_ip = tkinter.StringVar()

have = []
print ('Searching for ClockIOTs')
for localip in localips:
    if localip and localip not in have:
        ip = localip["localip"]
        t = localip["dev_type"]
        esp32_ip.set(ip)
        have.append(localip)
        if tap():
            b = tkinter.Radiobutton(root, text="%s-%s" % (ip, t),
                                    variable=esp32_ip, value=ip)
            b.pack(anchor=tkinter.W)
    

frame = tkinter.Frame(root)
tkinter.Button(frame, text="Brighter", command=brighter).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Dimmer", command=dimmer).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Flip", command=flip_display).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Next", command=next_display).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Get Displays", command=get_displays).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Set Display", command=set_display_idx).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Get Faceplates", command=get_faceplates).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Set Faceplate", command=set_faceplate_idx).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Set Time", command=set_time).pack(side=tkinter.LEFT)
tkinter.Button(frame, text="Use NTP", command=use_ntp).pack(side=tkinter.LEFT)
frame.pack()

mqtt_ip = []
mqtt_frame = tkinter.Frame(root)
tkinter.Button(mqtt_frame, text="Update MQTT IP:", command=send_mqtt_ip).pack(side=tkinter.LEFT)
bytes = ['192', '168', '1', '159']
for i in range(4):
    e = tkinter.Entry(mqtt_frame, width=3)
    e.insert(0, bytes[i])
    mqtt_ip.append(e)
    e.pack(side=tkinter.LEFT)
    if i < 3:
        tkinter.Label(mqtt_frame, text=".").pack(side=tkinter.LEFT)
mqtt_frame.pack()
root.mainloop()

