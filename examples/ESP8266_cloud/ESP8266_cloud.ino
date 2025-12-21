/*
  aREST Cloud Example for ESP8266

  This example connects your ESP8266 to the aREST cloud via MQTT.

  Setup:
  1. Go to dashboard.arest.io and create an account
  2. Click "Add Device" to create a new device
  3. Copy your Device ID and Device API Key
  4. Replace the placeholders below with your credentials

  Topic structure:
  - devices/{device_id}/commands   - Receives commands from server
  - devices/{device_id}/response   - Sends command responses
  - devices/{device_id}/telemetry  - Sends sensor data
  - devices/{device_id}/state      - Sends device state/heartbeat

  Authentication:
  - MQTT username = device_id
  - MQTT password = device_api_key

  Status LED:
  - LED off: No WiFi
  - LED blinking: WiFi connected, connecting to MQTT broker
  - LED pulsing: Connected to cloud

  See the README file for more details.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create MQTT cloud client
MQTTClient client;

// Create aREST instance
aREST rest = aREST(client);

// WiFi credentials
const char* ssid = "your_wifi_network";
const char* password = "your_wifi_password";

// Device credentials from dashboard.arest.io
const char* device_id = "your_device_id";           // e.g., "esp8266_kitchen_x1y2z3"
const char* device_api_key = "your_device_api_key"; // e.g., "k8f92j4kd8f7g6h5j4k3l2m1"

// Status LED pin (built-in LED on NodeMCU is GPIO2)
const int STATUS_LED = 2;

// Variables to be exposed to the API
int temperature;
int humidity;

// Function prototypes
int ledControl(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Configure MQTT client (connects to mqtt.arest.io by default)
  client.deviceId = String(device_id);
  client.apiKey = String(device_api_key);

  // Set up status LED
  client.setStatusLED(STATUS_LED);

  // Set device credentials for aREST
  rest.set_id(device_id);
  rest.set_name("ESP8266 Sensor");
  rest.setKey(device_api_key);

  // Expose variables to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature", &temperature);
  rest.variable("humidity", &humidity);

  // Expose functions to REST API
  rest.function("led", ledControl);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Device ID: ");
  Serial.println(device_id);
}

void loop() {
  // Handle cloud connection and commands
  rest.handle(client);

  // Update sensor values (replace with real sensor readings)
  temperature = 20 + random(10);
  humidity = 40 + random(20);

  // Publish telemetry every 5 seconds
  rest.publish(client, "temperature", temperature, 5000);
  rest.publish(client, "humidity", humidity, 5000);

  delay(100);
}

// Custom function accessible via cloud
// Call via: https://cloud.arest.io/{device_id}/led?params=1
int ledControl(String command) {
  int state = command.toInt();

  pinMode(D1, OUTPUT);  // GPIO5 on NodeMCU
  digitalWrite(D1, state);

  Serial.print("LED set to: ");
  Serial.println(state);

  return state;
}
