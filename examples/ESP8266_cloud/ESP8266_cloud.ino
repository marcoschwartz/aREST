/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  This example illustrates the cloud part of aREST using MQTT
  with automatic status LED showing connection state:
  - LED off: No WiFi
  - LED blinking: WiFi connected, connecting to MQTT broker
  - LED pulsing: Connected to MQTT broker

  Topic structure:
  - devices/{device_id}/commands   - Receives commands from server
  - devices/{device_id}/response   - Sends command responses
  - devices/{device_id}/telemetry  - Sends sensor data
  - devices/{device_id}/state      - Sends device state/heartbeat

  Authentication:
  - MQTT username = device_id
  - MQTT password = api_key

  See the README file for more details.

  Written in 2024 - MQTT implementation
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create MQTT cloud client
MQTTClient client;

// Create aREST instance
aREST rest = aREST(client);

// WiFi parameters
const char* ssid = "your_wifi_network_name";
const char* password = "your_wifi_network_password";

// aREST API key (your device secret key)
const char* key = "your_arest_key";

// Device ID (unique identifier for this device)
const char* device_id = "unique_device_id";

// Status LED pin (built-in LED on NodeMCU is usually GPIO2)
const int STATUS_LED = 2;

// Variables to be exposed to the API
int temperature;
int humidity;

// Function prototypes
int ledControl(String command);
int blink(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Configure MQTT client (server defaults to mqtt.arest.io:1883)
  // To use a custom server: client.setServer("your.server.com", 1883);
  client.deviceId = String(device_id);
  client.apiKey = String(key);

  // Set up status LED
  client.setStatusLED(STATUS_LED);

  // Set aREST API key
  rest.setKey(key);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature", &temperature);
  rest.variable("humidity", &humidity);

  // Functions to be exposed
  rest.function("led", ledControl);
  rest.function("blink", blink);

  // Give ID and name to device
  rest.set_id(device_id);
  rest.set_name("ESP8266 Sensor");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Device ready!");
  Serial.print("Device ID: ");
  Serial.println(device_id);
}

void loop() {
  // Connect to the MQTT broker and handle commands
  rest.handle(client);

  // Update sensor values (simulate sensor readings)
  temperature = 20 + random(10);
  humidity = 40 + random(20);

  // Publish telemetry every 5 seconds
  rest.publish(client, "temperature", temperature, 5000);
  rest.publish(client, "humidity", humidity, 5000);

  // Small delay
  delay(100);
}

// Custom function accessible by the API
// Called via: POST /arest/devices/{device_id}/command
// Body: { "command_type": "function", "function": "led", "args": "1" }
int ledControl(String command) {
  // Get state from command
  int state = command.toInt();

  // Control LED on pin D1 (GPIO5)
  pinMode(D1, OUTPUT);
  digitalWrite(D1, state);

  Serial.print("LED set to: ");
  Serial.println(state);

  return state;
}

// Blink function
// Called via: POST /arest/devices/{device_id}/command
// Body: { "command_type": "function", "function": "blink", "params": { "times": 3 } }
int blink(String command) {
  // Default to 3 blinks
  int times = 3;

  if (command.length() > 0) {
    times = command.toInt();
    if (times <= 0) times = 3;
  }

  pinMode(D1, OUTPUT);

  for (int i = 0; i < times; i++) {
    digitalWrite(D1, HIGH);
    delay(200);
    digitalWrite(D1, LOW);
    delay(200);
  }

  Serial.print("Blinked ");
  Serial.print(times);
  Serial.println(" times");

  return times;
}
