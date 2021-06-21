/*
  This a simple example of the aREST Library for the ESP32 WiFi chip.
  This example illustrate the cloud part of aREST that makes the board accessible from anywhere
  See the README file for more details.
  Written in 2017 by Marco Schwartz under a GPL license.
  
*/
/*|-----------------------------------------------------------|*/
/*|aREST CLOUD ENTERPRISE connection                          |*/
/*|EMAIL: martinius96@gmail.com                               |*/
/*|WEBSITE: https://arduino.php5.sk                           |*/
/*|-----------------------------------------------------------|*/
// Import required libraries
#include <WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks

// Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Create aREST instance
aREST rest = aREST(client);

// Unique ID to identify the device for cloud.arest.io
char* device_id = "unique_device_id";

// WiFi & identity parameters
char EAP_ANONYMOUS_IDENTITY[] = "anonymous@example.com";
char EAP_IDENTITY[] = "nickname@example.com";
char EAP_PASSWORD[] = "EDUROAM_PASSWORD";
const char* ssid = "eduroam"; // Eduroam SSID

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);

// Functions
void callback(char* topic, byte* payload, unsigned int length);

void setup()
{
  
  // Start Serial
  Serial.begin(115200);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
  esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function
  
  // Set callback
  client.setCallback(callback);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id(device_id);
  rest.set_name("esp32");

  // Connect to WiFi
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

void loop() {
  
  // Connect to the cloud
  rest.handle(client);
  
}

// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  rest.handle_callback(client, topic, payload, length);

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}
