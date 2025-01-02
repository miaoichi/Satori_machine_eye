from pynput import mouse
from pynput import keyboard
from bleak import BleakClient
from threading import Lock
import asyncio
import time
import json

DEVICE_ADDRESS = "8c:bf:ea:0a:64:01"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

x_max, y_max = 2559, 1599
servo_range = [(90, 170, 130), (45, 100, 75), (70, 170, 140)]

position = [85, 30, 70]
action = 0
stay = False
new_data = False
last_sent_time = 0
lock = Lock()


def on_move(x, y):
    global position, action, new_data
    with lock:
        position[0] = servo_range[0][0] + x / x_max * (servo_range[0][1] - servo_range[0][0])
        position[1] = servo_range[1][0] + y / y_max * (servo_range[1][1] - servo_range[1][0])
        if not stay:
            action = 0
        new_data = True


def on_scroll(x, y, dx, dy):
    global position, action, new_data
    with lock:
        position[2] -= dy
        position[2] = max(servo_range[2][0], min(position[2], servo_range[2][1]))
        if not stay:
            action = 0
        new_data = True


def on_press(key):
    global position, action, stay, new_data
    try:
        char = key.char
    except AttributeError:
        char = None
    with lock:
        match char:
            case ('0'):
                action = 0
                stay = False
                new_data = True
            case ('1'):
                action = 1
                stay = False
                new_data = True
            case ('2'):
                action = 2
                stay = False
                new_data = True
            case ('3'):
                action = 3
                stay = False
                new_data = True
            case ('4'):
                action = 4
                stay = True
                new_data = True
            case ('5'):
                action = 5
                stay = False
                position[2] = servo_range[2][2]
                new_data = True


async def main():
    global new_data, last_sent_time
    async with BleakClient(DEVICE_ADDRESS) as client:
        print("Connected to ESP32 BLE")

        while True:
            with lock:
                angle_x = round(position[0], 1)
                angle_y = round(position[1], 1)
                angle_z = round(position[2], 1)
                ac = action

            if new_data and time.time() - last_sent_time > 0.05:
                new_data = False
                last_sent_time = time.time()

                send_message = json.dumps({"x": angle_x, "y": angle_y, "z": angle_z, "action": ac})
                await client.write_gatt_char(CHARACTERISTIC_UUID, send_message.encode())
                print(f"Sent angle: {send_message}")

                response = await client.read_gatt_char(CHARACTERISTIC_UUID)
                response_json = json.loads(response.decode('utf-8'))
                print(f"Received response: {response_json}")


if __name__ == "__main__":
    mouse_listener = mouse.Listener(on_move=on_move, on_scroll=on_scroll)
    keyboard_listener = keyboard.Listener(on_press=on_press)

    mouse_listener.start()
    keyboard_listener.start()
    asyncio.run(main())
