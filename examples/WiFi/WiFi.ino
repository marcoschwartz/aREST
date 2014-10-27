/* 
  This a simple example of the aREST Library for Arduino (Uno/Mega/Due/Teensy)
  using the WiFi library (for example to be used with the Arduino WiFi shield). 
  See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 
*/

#include <SPI.h>
#include <WiFi.h>
#include <aREST.h>

char ssid[] = "yourWiFiNetwork"; //  your network SSID (name)
char pass[] = "yourWiFiPassword";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Create aREST instance
aREST rest = aREST();

// Initialize the WiFi server library
WiFiServer server(80);

// Variables to be exposed to the API
int temperature;
int humidity;

void setup() {

  // Start Serial
  Serial.begin(115200);
  
  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
  
  rest.function("led",ledControl);
  
  // Give name and ID to device
  rest.set_id("008");
  rest.set_name("dapper_drake");

  // Function to be exposed
  rest.function("led",ledControl);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // Attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // Wait 10 seconds for connection
    delay(10000);
  }

  // Start the server
  server.begin();

  // Print out the status
  printWifiStatus();
}


void loop() {
  
  // listen for incoming clients
  WiFiClient client = server.available();
  rest.handle(client);
  
}

// Custom function accessible by the API
int ledControl(String command) {
  
  // Get state from command
  int state = command.toInt();
  
  digitalWrite(6,state);
  return 1;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
