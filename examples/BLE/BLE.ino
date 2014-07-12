/* 
  This a simple example of the aREST Library for Arduino (Uno/Mega/Due/Teensy)
  using the Adafruit BLE UART library. See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 
*/

// Libraries
#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include <aREST.h>

// BLE module pin
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9

// Create aREST instance
aREST rest = aREST();

// BLE instance
Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

// Variables to be exposed to the API
int temperature;
int humidity;

void setup(void)
{  
  // Start Serial
  Serial.begin(9600);

  // Start BLE
  BTLEserial.begin();
  
  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);
 
  // Give name and ID to device
  rest.set_id("008");
  rest.set_name("ble_drake"); 
}

// Status message
aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop() {  
  
  // Tell the nRF8001 to do whatever it should be working on.
  BTLEserial.pollACI();
  
  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
  }
  
  // Handle REST calls
  if (status == ACI_EVT_CONNECTED) {
    rest.handle(BTLEserial);
  }
}

// Custom function accessible by the API
int ledControl(String command) {
  
  // Get state from command
  int state = command.toInt();
  
  digitalWrite(7,state);
  return 1;
}