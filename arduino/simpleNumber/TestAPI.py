#!/usr/bin/env python

# WS client example

import asyncio
import websockets
import time

async def hello():
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        name = input("What's your name? ")

        await websocket.send(name)
        print(f"> {name}")

        greeting = await websocket.recv()
        print(f"< {greeting}")

async def brighter():
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        await websocket.send("takeanumber/brighter//xxx")
        greeting = await websocket.recv()
        print(f"< {greeting}")
        

async def dimmer():
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        await websocket.send("takeanumber/dimmer")
        greeting = await websocket.recv()
        print(f"< {greeting}")
async def reset_number():
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        await websocket.send("takeanumber/reset_number")
        greeting = await websocket.recv()
        print(f"< {greeting}")

async def increment():
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        await websocket.send("takeanumber/increment")
        greeting = await websocket.recv()
        print(f"< {greeting}")
async def set_number(number):
    async with websockets.connect('ws://192.168.4.1:81') as websocket:
        await websocket.send("takeanumber/set_number//%d" % number)
        greeting = await websocket.recv()
        print(f"< {greeting}")

asyncio.get_event_loop().run_until_complete(reset_number())
#asyncio.get_event_loop().run_until_complete(brighter())
#asyncio.get_event_loop().run_until_complete(dimmer())
#asyncio.get_event_loop().run_until_complete(increment())
#asyncio.get_event_loop().run_until_complete(set_number(99))

for i in range(20):
    asyncio.get_event_loop().run_until_complete(set_number(i))
    time.sleep(.5)

