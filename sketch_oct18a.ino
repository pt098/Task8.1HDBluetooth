#include <ArduinoBLE.h>
#include <Wire.h>
#include <BH1750.h>

// BLE service and characteristic UUIDs
const char* serviceUUID = "7546d197-156d-48a1-848b-7a27a4a5dc3a";
const char* charUUID = "ad1ad0d9-a4a4-4879-9d02-ec57410dab48";

// Initialize BLE service and characteristic
BLEService ledService(serviceUUID);
BLEIntCharacteristic lightCharacteristic(charUUID, BLERead | BLEWrite); // Use IntCharacteristic for light level

// Initialize BH1750 light sensor
BH1750 lightSensor(0x23);
unsigned long lastMeasurementTime = 0; // To manage measurement timing
const unsigned long measurementInterval = 1000; // Measurement interval in milliseconds

void setup() {
    // Start serial communication
    Serial.begin(9600);
    while (!Serial);

    // Start BLE
    Serial.println("Starting BLE...");
    if (!BLE.begin()) {
        Serial.println("Failed to start BLE module!");
        while (1);
    }
    Serial.println("BLE module started successfully.");

    // Set up BLE
    BLE.setLocalName("LED");
    BLE.setAdvertisedService(ledService);
    ledService.addCharacteristic(lightCharacteristic);
    BLE.addService(ledService);
    BLE.advertise();
    Serial.println("Advertising BLE Light Sensor Peripheral...");

    // Initialize I2C for the light sensor
    Wire.begin();
    if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("BH1750 Sensor initialized.");
    } else {
        Serial.println("Error initializing BH1750 sensor.");
        while (1);
    }
}

void loop() {
    // Wait for a central device to connect
    BLEDevice central = BLE.central();

    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        // While the central is connected
        while (central.connected()) {
            // Check if it's time to take a reading
            if (millis() - lastMeasurementTime >= measurementInterval) {
                lastMeasurementTime = millis(); // Update the last measurement time

                // Read light level
                float lux = lightSensor.readLightLevel();

                // Check if reading was successful
                if (lux >= 0) {
                    Serial.print("Light Intensity: ");
                    Serial.print(lux);
                    Serial.println(" lx");

                    // Convert the light intensity (lux) to an integer for BLE
                    int luxInt = (int)lux;

                    // Send the light intensity over BLE
                    lightCharacteristic.writeValue(luxInt);
                } else {
                    Serial.println("Failed to read light level.");
                }
            }

            // Yield to allow BLE processing
            BLE.poll();
        }

        // When the central disconnects
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }

    // Allow BLE processing when not connected
    BLE.poll();
}
