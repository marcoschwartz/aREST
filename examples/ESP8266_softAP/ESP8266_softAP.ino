/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "your_wifi_network_name";
const char* password = "your_wifi_network_password";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);
void aquariumController(aREST *arest, const String& name, const String& command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // API-Extension to be exposed
  rest.api_extension("aquarium", aquariumController);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp8266");

  // Setup WiFi network
  WiFi.softAP(ssid, password);
  Serial.println("");
  Serial.println("WiFi created");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
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

void aquariumController(aREST *arest, const String& name, const String& command) {
  Serial.print(F("============= controller: "));
  Serial.println(command);
  // check format of command
  if (command == F("/aquarium")
      || command == F("/aquarium/")) {
    Serial.println(F("list of sensors"));

    // Send feedback to client
    if (LIGHTWEIGHT) {
      bool isFirstSensor = true;
      auto count = 5;
      for (uint32_t i = 0; i < count; ++i) {
        if (isFirstSensor) {
          isFirstSensor = false;
        } else {
          arest->addToBufferF(F(","));
        }
        auto id = i + 100;
        arest->addToBuffer(id);
      }
    } else {
      arest->addToBufferF(F("\"sensor-ids\": ["));
      bool isFirstSensor = true;
      auto count = 5;
      for (uint32_t i = 0; i < count; ++i) {
        if (isFirstSensor) {
          isFirstSensor = false;
        } else {
          arest->addToBufferF(F(", "));
        }
        arest->addToBufferF(F("\""));
        auto id = i + 100;
        arest->addToBuffer(id);
        arest->addToBufferF(F("\""));
      }
      arest->addToBufferF(F("]"));
    }
  } else if (command.startsWith(F("/aquarium/water_limit/lower/set/"))) {
    String args = command.substring(32); // 32 = length of "/aquarium/water_limit/lower/set/"

    Serial.print(F("set lower water limit to "));
    Serial.println(args);

    // Send feedback to client
    if (!LIGHTWEIGHT) {
      arest->addToBufferF(F("\"message\": \"lower water limit set to "));
      arest->addToBuffer(args);
      arest->addToBufferF(F("cm\""));
    }
  } else {
    arest->addToBufferF(F("\"message\": \"Unknown command '"));
    arest->addToBuffer(command);
    arest->addToBufferF(F("'.\""));
  }
}
