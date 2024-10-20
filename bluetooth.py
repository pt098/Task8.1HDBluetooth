import asyncio
import gpiod
from bleak import BleakScanner, BleakClient

# Define  generated UUIDs
LED_UUID = "7546d197-156d-48a1-848b-7a27a4a5dc3a"  # Service UUID
CHAR_UUID = "ad1ad0d9-a4a4-4879-9d02-ec57410dab48"  # Characteristic UUID

# Set up GPIO for LED control
chip = gpiod.Chip('gpiochip4')
line = chip.get_line(17)  # GPIO LED pin number
line.request(consumer="led_controller", type=gpiod.LINE_REQ_DIR_OUT)

async def connect_to_device(ble_device):
    """Attempts to connect to the BLE device."""
    try:
        async with BleakClient(ble_device, timeout=10.0) as client:  # Increase timeout
            print("Connected to device")
            while True:
                try:
                    # Read light intensity from the characteristic
                    light_intensity = await client.read_gatt_char(CHAR_UUID)
                    intensity_value = int.from_bytes(light_intensity, byteorder='little')

                    # Control LED based on light intensity
                    if intensity_value > 100:  # Adjust threshold as needed
                        line.set_value(1)  # Turn on LED
                    else:
                        line.set_value(0)  # Turn off LED
                    
                    print(f"Light Intensity: {intensity_value}, LED state: {line.get_value()}")
                    await asyncio.sleep(1)  # Adjust this value to control how often you want to read the intensity

                except Exception as read_error:
                    print(f"Error reading characteristic: {read_error}")
                    break  # Exit the loop if reading fails

    except Exception as e:
        print(f"Error connecting to device: {e}")

async def main():
    while True:
        print("Searching for Arduino Nano 33 IoT 'LED' device, please wait...")
        devices = await BleakScanner.discover(timeout=5)

        for ble_device in devices:
            if ble_device.name and 'LED' in ble_device.name:
                print("Device found:", ble_device.name)
                await connect_to_device(ble_device)
                print("Attempting to reconnect in 10 seconds...")  # Wait 10 sec before trying to reconnect
                await asyncio.sleep(10)  # Wait before attempting to reconnect

        # If no device is found wait before scanning again
        print("No device found, scanning again in 5 seconds...")
        await asyncio.sleep(5)

if __name__ == "__main__":
    asyncio.run(main())