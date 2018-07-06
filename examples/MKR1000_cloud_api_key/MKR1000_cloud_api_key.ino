/*
  This a simple example of the aREST Library for the Arduino/Genuino MKR1000 board.
  See the README file for more details.

  Written in 2016 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <aREST.h>

// Status
int status = WL_IDLE_STATUS;

// Clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Create aREST instance
aREST rest = aREST(client);

// aREST API key (that you can get at dashboard.arest.io)
char * key = "your_arest_key";

// WiFi parameters
char ssid[] = "your_wifi_network_name";
char password[] = "your_wifi_network_password";

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);

// Callback function for the cloud connection
void callback(char* topic, byte* payload, unsigned int length);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Set aREST API key
  rest.setKey(key);

  // Set callback
  client.setCallback(callback);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // Give ID to device (optional, if not set, a device ID will be auto-assigned to the device)
  // rest.set_id("unique_device_id");

  // Give name to device
  rest.set_name("mkr1000");

  // Connect to WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);

    // Wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("WiFi connected");

}

void loop() {

  // Connect to the cloud
  rest.handle(client);

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}

// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  // Handle
  rest.handle_callback(client, topic, payload, length);

}
