/*
  This a simple example of the aREST Library for the ESP32 WiFi chip.
  See the README file for more details.
  Written in 2017 by Marco Schwartz under a GPL license.
  
*/
/*|----------------------------------------------------------|*/
/*|aREST ENTERPRISE connection                               |*/
/*|EMAIL: martinius96@gmail.com                              |*/
/*|WEBSITE: https://arduino.php5.sk                          |*/
/*|----------------------------------------------------------|*/

// Import required libraries
#include <WiFi.h>
#include <aREST.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks
// Create aREST instance
aREST rest = aREST();

// WiFi & identity parameters
char EAP_ANONYMOUS_IDENTITY[] = "anonymous@example.com";
char EAP_IDENTITY[] = "nickname@example.com";
char EAP_PASSWORD[] = "EDUROAM_PASSWORD";
const char* ssid = "eduroam"; // Eduroam SSID

// Create an instance of the server
WiFiServer server(80);

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);

void setup()
{
  
  // Start Serial
  Serial.begin(115200);
   esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
  esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function
  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp32");

  // Connect to WiFi
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

}

void loop() {
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}
