/*
  This a simple example of the aREST Library working with
  the Arduino Yun. See the README file for more details.

  Written in 2014 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <aREST.h>

// Create aREST instance
aREST rest = aREST();

// Yun Server
YunServer server(80);

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

  // Init variables and expose them to REST API (ID should be 6 characters long)
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // API-Extension to be exposed
  rest.api_extension("aquarium", aquariumController);

  // Give name and ID to device
  rest.set_id("008");
  rest.set_name("mighty_cat");

  // Bridge startup
  Bridge.begin();

  // Listen for incoming connection only from localhost
  server.begin();
}

void loop() {

  // Handle REST calls
  YunClient client = server.accept();
  rest.handle(client);

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(7,state);
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
