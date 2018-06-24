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

// Device ID for the device on the cloud (should be 6 characters long)
char * device_id = "20g83d";

// WiFi parameters
char ssid[] = "your_wifi_network_name";
char password[] = "your_wifi_network_password";

// Variables to be exposed to the API
int temperature;
int humidity;
String local_ip = "";

// Declare functions to be exposed to the API
int ledControl(String command);
void aquariumController(aREST *arest, const String& name, const String& command);

// Callback function for the cloud connection
void callback(char* topic, byte* payload, unsigned int length);

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Set callback
  client.setCallback(callback);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
  rest.variable("local_ip", &local_ip);

  // Function to be exposed
  rest.function("led",ledControl);

  // API-Extension to be exposed
  rest.api_extension("aquarium", aquariumController);

  // Give name and ID to device (ID should be 6 characters long)
  rest.set_id(device_id);
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

  // Start the server
  server.begin();
  Serial.println("Local server started on IP:");

  // Print the IP address
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  local_ip = ipToString(WiFi.localIP());

}

void loop() {

  // Connect to the cloud
  rest.handle(client);

  // Handle REST calls
  WiFiClient localClient = server.available();
  if (!localClient) {
    return;
  }
  while(!localClient.available()){
    delay(1);
  }
  rest.handle(localClient);

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


// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  // Handle
  rest.handle_callback(client, topic, payload, length);

}

// Convert IP address to String
String ipToString(IPAddress address)
{
  return String(address[0]) + "." +
    String(address[1]) + "." +
    String(address[2]) + "." +
    String(address[3]);
}
