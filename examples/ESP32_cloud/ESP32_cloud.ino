/*
  This a simple example of the aREST Library for the ESP32 WiFi chip.
  This example illustrates the cloud part of aREST using WebSocket
  with automatic status LED showing connection state:
  - LED off: No WiFi
  - LED blinking: WiFi connected, connecting to cloud
  - LED pulsing: Connected to cloud
  
  See the README file for more details.

  Written in 2024 - WebSocket implementation
*/

// Import required libraries
#include <WiFi.h>
#include <aREST.h>

// Create cloud client
CloudClient client;

// Create aREST instance
aREST rest = aREST(client);

// WiFi parameters
const char* ssid = "your_wifi_network_name";
const char* password = "your_wifi_network_password";

// aREST API key (that you can get at dashboard.arest.io)
char * key = "your_arest_key";

// Device ID
char * device_id = "unique_device_id";

// Status LED pin (built-in LED on most ESP32 boards)
const int STATUS_LED = 2;

// Variables to be exposed to the API
int temperature;
int humidity;

// Function to be exposed
int ledControl(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Configure cloud client
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
  
  // Function to be exposed
  rest.function("led", ledControl);

  // Give ID to device
  rest.set_id(device_id);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Connect to the cloud
  rest.handle(client);

  // Update sensor values
  temperature = 20 + random(10);
  humidity = 40 + random(20);

  // Small delay
  delay(100);
}

// Custom function accessible by the API
int ledControl(String command) {
  // This controls a different LED than the status LED
  // Get state from command
  int state = command.toInt();
  
  digitalWrite(5, state);
  
  return 1;
}