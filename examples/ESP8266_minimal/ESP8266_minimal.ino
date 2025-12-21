/*
  aREST Minimal Example for ESP8266

  Just connects to WiFi and the aREST cloud. Nothing else.

  Get your Device ID and API Key from dashboard.arest.io
*/

#include <ESP8266WiFi.h>
#include <aREST.h>

MQTTClient client;
aREST rest = aREST(client);

// WiFi
const char* ssid = "your_wifi";
const char* password = "your_password";

// Device credentials from dashboard.arest.io
const char* device_id = "your_device_id";
const char* device_api_key = "your_device_api_key";

void setup() {
  Serial.begin(115200);

  // Set credentials
  client.deviceId = String(device_id);
  client.apiKey = String(device_api_key);
  rest.set_id(device_id);
  rest.set_name("ESP8266");
  rest.setKey(device_api_key);

  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected");
}

void loop() {
  rest.handle(client);
}
