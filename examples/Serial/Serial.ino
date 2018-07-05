/*
  This a simple example of the aREST Library for Arduino (Uno/Mega/Due/Teensy)
  using the Serial port. See the README file for more details.

  Written in 2014 by Marco Schwartz under a GPL license.
*/

// Libraries
#include <SPI.h>
#include <aREST.h>
#include <avr/wdt.h>

// Create aREST instance
aREST rest = aREST();

// Variables to be exposed to the API
int temperature;
int humidity;

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

  // Give name and ID to device (ID should be 6 characters long)
  rest.set_id("2");
  rest.set_name("serial");

  // Start watchdog
  wdt_enable(WDTO_4S);
}

void loop() {

  // Handle REST calls
  rest.handle(Serial);
  wdt_reset();

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}
