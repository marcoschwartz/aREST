# aREST

**Version 3.1.2**

[![Build Status](https://travis-ci.org/marcoschwartz/aREST.svg?branch=master)](https://travis-ci.org/marcoschwartz/aREST)

## Overview

A simple library that implements a REST API for Arduino & the ESP8266/ESP32 WiFi chips.

**Version 3.1.2**: This version migrates to MQTT protocol for cloud connectivity, providing lightweight, reliable IoT communication with automatic reconnection and status LED feedback.

It is designed to be universal and currently supports REST calls via HTTP (using the CC3000 WiFi chip, the Arduino WiFi library or the Ethernet shield), via the Serial port (using the USB serial connection, Bluetooth, and XBee) and also via Bluetooth Low Energy.

It also works with the ESP8266 and ESP32 WiFi chips, operating as independent IoT devices that can be controlled from anywhere in the world via the aREST cloud.

## Contents

- aREST.h: the library file
- examples: several examples using the aREST library
- test: unit tests of the library

## Supported Hardware

### ESP8266/ESP32 (Recommended for Cloud)

The library is compatible with most ESP8266 modules & development boards (NodeMCU, Wemos D1, etc.), as well as most boards based on the ESP32 WiFi chip.

### Arduino/Genuino Boards

Compatible with: Uno, Mega, Due, Yun, Teensy 3.x, and MKR1000.

## Requirements

### Arduino IDE

- [Arduino IDE 1.8.5+](http://arduino.cc/en/main/software)

### For ESP8266

1. Open Arduino IDE Preferences
2. Add to "Additional Board Manager URLs": `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Open Tools > Board > Boards Manager, search "esp8266" and install

### For ESP32

1. Add to "Additional Board Manager URLs": `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. Open Tools > Board > Boards Manager, search "esp32" and install

### For Cloud Access (ESP8266/ESP32)

Install these libraries via Arduino Library Manager (Sketch > Include Library > Manage Libraries):

- **PubSubClient** by Nick O'Leary - MQTT client
- **ArduinoJson** by Benoit Blanchon - JSON parsing

## Cloud Setup - Getting Started

### Step 1: Create an Account

1. Go to [dashboard.arest.io](https://dashboard.arest.io)
2. Create an account or sign in

### Step 2: Create a Device

1. Click "Add Device" in the dashboard
2. Give your device a name (e.g., "Living Room Sensor")
3. The dashboard will generate:
   - **Device ID**: A unique identifier (e.g., `esp32_living_room_a1b2c3`)
   - **API Key**: A secret key for authentication (e.g., `k8f92j4kd8f7g6h5j4k3l2m1`)

**Important**: Save your API Key! It's only shown once.

### Step 3: Upload the Sketch

```cpp
#include <WiFi.h>        // For ESP32
// #include <ESP8266WiFi.h>  // For ESP8266
#include <aREST.h>

// Create MQTT client (connects to mqtt.arest.io by default)
MQTTClient client;
aREST rest = aREST(client);

// WiFi credentials
const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";

// Device credentials from dashboard
const char* device_id = "esp32_living_room_a1b2c3";  // Your Device ID
const char* api_key = "k8f92j4kd8f7g6h5j4k3l2m1";    // Your API Key

// Variables to expose
int temperature = 25;
int humidity = 60;

// Custom function
int ledControl(String command) {
  int state = command.toInt();
  digitalWrite(LED_BUILTIN, state);
  return state;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Set device credentials
  client.deviceId = String(device_id);
  client.apiKey = String(api_key);

  // Optional: Status LED (pulses when connected)
  client.setStatusLED(2);

  // Configure aREST
  rest.set_id(device_id);
  rest.set_name("Living Room");
  rest.setKey(api_key);

  // Expose variables
  rest.variable("temperature", &temperature);
  rest.variable("humidity", &humidity);

  // Expose functions
  rest.function("led", ledControl);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  // Handle cloud connection and commands
  rest.handle(client);

  // Update sensor values
  temperature = 20 + random(10);
  humidity = 50 + random(20);

  // Publish telemetry every 5 seconds
  rest.publish(client, "temperature", temperature, 5000);
  rest.publish(client, "humidity", humidity, 5000);

  delay(100);
}
```

### Step 4: Control Your Device

Once connected, you can control your device via:

**REST API:**
```bash
# Set pin HIGH
curl https://cloud.arest.io/{device_id}/digital/5/1

# Read pin
curl https://cloud.arest.io/{device_id}/digital/5

# Call function
curl https://cloud.arest.io/{device_id}/led?params=1

# Get variable
curl https://cloud.arest.io/{device_id}/temperature

# Get device info
curl https://cloud.arest.io/{device_id}
```

**Dashboard:**
- View real-time telemetry
- Send commands
- Create dashboards with widgets

## MQTT Topic Structure

The library uses the following MQTT topics:

| Topic | Direction | Purpose |
|-------|-----------|---------|
| `devices/{device_id}/commands` | Server → Device | Receive commands |
| `devices/{device_id}/response` | Device → Server | Send command responses |
| `devices/{device_id}/telemetry` | Device → Server | Send sensor data |
| `devices/{device_id}/state` | Device → Server | Send heartbeat/status |

## Status LED

Connect an LED to show connection status:

```cpp
client.setStatusLED(2);  // GPIO2 (built-in LED on most boards)
```

| LED State | Meaning |
|-----------|---------|
| Off | No WiFi connection |
| Fast blink | WiFi connected, connecting to MQTT |
| Slow pulse | Connected to cloud |

## Custom MQTT Server

To use your own MQTT broker instead of mqtt.arest.io:

```cpp
client.setServer("your-broker.com", 1883);
```

## API Documentation

### Digital

```
/digital/8/0    - Set pin 8 LOW
/digital/8/1    - Set pin 8 HIGH
/digital/8      - Read pin 8
```

### Analog

```
/analog/6/123   - PWM write 123 to pin 6
/analog/0       - Read analog pin A0
```

### Mode

```
/mode/8/o       - Set pin 8 as OUTPUT
/mode/8/i       - Set pin 8 as INPUT
```

### Variables

Expose variables in your sketch:

```cpp
int temperature = 25;
rest.variable("temperature", &temperature);
```

Access via: `/temperature`

### Functions

Define custom functions:

```cpp
int ledControl(String command) {
  int state = command.toInt();
  digitalWrite(LED_BUILTIN, state);
  return state;
}
rest.function("led", ledControl);
```

Call via: `/led?params=1`

### Telemetry

Send data to the cloud:

```cpp
rest.publish(client, "temperature", 25);           // Send immediately
rest.publish(client, "temperature", 25, 5000);     // Send max every 5 seconds
```

## Command Format (MQTT)

Commands received on the `commands` topic use JSON format:

```json
{
  "command_id": "cmd_123456",
  "type": "function",
  "function": "led",
  "params": { "value": 1 }
}
```

Supported command types:
- `digital_write` - Write to digital pin
- `digital_read` - Read digital pin
- `analog_write` - PWM write
- `analog_read` - Read analog pin
- `function` - Call custom function
- `get_variable` - Get variable value
- `get_info` - Get device info

## Troubleshooting

### Device not connecting

1. Check WiFi credentials
2. Verify Device ID and API Key are correct
3. Ensure your network allows outbound MQTT (port 1883)
4. Check Serial monitor for debug messages

### Commands not working

1. Ensure device shows as "online" in dashboard
2. Check that the function/variable is registered in setup()
3. Look at Serial monitor for incoming commands

### Enable Debug Mode

Add before including aREST.h:

```cpp
#define DEBUG_MODE 1
#include <aREST.h>
```

## Migration from v3.0.x (WebSocket)

If upgrading from version 3.0.x:

1. Replace `CloudClient` with `MQTTClient`
2. Remove ArduinoWebsockets library
3. Install PubSubClient library
4. Update includes (no changes needed, same `<aREST.h>`)

```cpp
// Old (v3.0.x)
CloudClient client;

// New (v3.1.2)
MQTTClient client;
```

## Local Testing (Without Cloud)

You can also use aREST locally via HTTP:

```cpp
#include <WiFi.h>
#include <aREST.h>

aREST rest;
WiFiServer server(80);

void setup() {
  // ... WiFi setup ...
  server.begin();
  rest.variable("temperature", &temperature);
}

void loop() {
  WiFiClient client = server.available();
  rest.handle(client);
}
```

Access via: `http://192.168.1.x/temperature`

## License

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
