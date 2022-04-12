#include "aREST.h"

aREST::aREST() {
  initialize();
}

aREST::aREST(char* rest_remote_server, int rest_port) {

  initialize();

  remote_server = rest_remote_server;
  port = rest_port;

}



void aREST::initialize() {
  reset();
  status_led_pin = 255;
}

void aREST::reset() {
  command = 'u';
  state = 'u';
  pin_selected = false;
}

#if defined(PubSubClient_h)

// With default server
aREST::aREST(PubSubClient& client) {

  initialize();

  private_mqtt_server = false;
  client.setServer(mqtt_server, 1883);

}

// With another server
aREST::aREST(PubSubClient& client, char* new_mqtt_server) {

  initialize();

  private_mqtt_server = true;
  setMQTTServer(new_mqtt_server);
  client.setServer(new_mqtt_server, 1883);

}

// Subscribe to events
void aREST::subscribe(const String& device, const String& eventName) {

  // Build topic
  String topic = device + "_" + eventName + "_in";

  // Subscribe
  char charBuf[50];
  topic.toCharArray(charBuf, 50);

  subscriptions_names[subscriptions_index] = charBuf;
  subscriptions_index++;

}

void aREST::setKey(char* api_key) {

  // Set
  proKey = String(api_key);

  if (id.length() == 0) {

    // Generate MQTT random ID
    id = gen_random(6);

  }

  // Build topics IDs
  in_topic = id + String(api_key) + String("_in");
  out_topic = id + String(api_key) + String("_out");

  // strcpy(in_topic, inTopic.c_str());
  // strcpy(out_topic, outTopic.c_str());

  // Build client ID
  client_id = id + String(api_key);

}

// void aREST::setKey(char* api_key, PubSubClient& client) {

//   // Set
//   proKey = String(api_key);

//   if (id.length() == 0) {

//     // Generate MQTT random ID
//     id = gen_random(6);

//   }

//   // Build topics IDs
//   String inTopic = id + String(api_key) + String("_in");
//   String outTopic = id + String(api_key) + String("_out");

//   strcpy(in_topic, inTopic.c_str());
//   strcpy(out_topic, outTopic.c_str());

//   // Build client ID
//   client_id = id + String(api_key);
//   client_id = id + String(proKey);

// }

#endif


// Set status LED
void aREST::set_status_led(uint8_t pin){

  // Set variables
  status_led_pin = pin;

  // Set pin as output
  pinMode(status_led_pin, OUTPUT);
}

#if !defined(ESP32)
// Glow status LED
void aREST::glow_led() {

  if(status_led_pin != 255){
    unsigned long i = millis();
    int j = i % 4096;
    if (j > 2048) { j = 4096 - j;}
      analogWrite(status_led_pin,j/8);
    }
}
#endif

void aREST::addToBufferF(const __FlashStringHelper *toAdd){

  if (DEBUG_MODE) {
    #if defined(ESP8266)|| defined (ESP32)
    Serial.print("Memory loss:");
    Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    freeMemory = ESP.getFreeHeap();
    #endif
    Serial.print(F("Added to buffer as progmem: "));
    Serial.println(toAdd);
  }

  uint8_t idx = 0;

  PGM_P p = reinterpret_cast<PGM_P>(toAdd);

  for ( unsigned char c = pgm_read_byte(p++);
        c != 0 && index < OUTPUT_BUFFER_SIZE;
        c = pgm_read_byte(p++), index++) {
    buffer[index] = c;
  }
}

// Send HTTP headers for Ethernet & WiFi
void aREST::send_http_headers(){

  addToBufferF(F("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"));

}

// Reset variables after a request
void aREST::reset_status() {

  if (DEBUG_MODE) {
    #if defined(ESP8266)|| defined (ESP32)
      Serial.print("Memory loss before reset:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
    #endif
  }

  reset();
  answer = "";
  arguments = "";

  index = 0;
  //memset(&buffer[0], 0, sizeof(buffer));

  if (DEBUG_MODE) {
    #if defined(ESP8266)|| defined (ESP32)
    Serial.print("Memory loss after reset:");
    Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    freeMemory = ESP.getFreeHeap();
    Serial.print("Memory free:");
    Serial.println(freeMemory, DEC);
    #endif
  }

}

// Handle request with the Adafruit CC3000 WiFi library
#ifdef ADAFRUIT_CC3000_H
void aREST::handle(Adafruit_CC3000_ClientRef& client) {

  if (client.available()) {

    // Handle request
    handle_proto(client,true,0,false);

    // Answer
    sendBuffer(client,32,20);
    client.stop();

    // Reset variables for the next command
    reset_status();

  }
}



// Handle request with the Arduino Yun
#elif defined(_YUN_CLIENT_H_)
void aREST::handle(YunClient& client) {

  if (client.available()) {

    // Handle request
    handle_proto(client,false,0,false);

    // Answer
    sendBuffer(client,25,10);
    client.stop();

    // Reset variables for the next command
    reset_status();
  }
}


// Handle request with the Adafruit BLE board
#elif defined(_ADAFRUIT_BLE_UART_H_)
void aREST::handle(Adafruit_BLE_UART& serial) {

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,0,false);

    // Answer
    sendBuffer(serial,100,1);

    // Reset variables for the next command
    reset_status();
  }
}



// Handle request for the Arduino Ethernet shield
#elif defined(ethernet_h_)
void aREST::handle(EthernetClient& client){

  if (client.available()) {

    // Handle request
    handle_proto(client,true,0,false);

    // Answer
    sendBuffer(client,50,0);
    client.stop();

    // Reset variables for the next command
    reset_status();
  }
}



// Handle request for the Cytron Clone ESP8266
#elif defined(_CYTRONWIFISERVER_H_)
void aREST::handle(ESP8266Client& client){

  if (client.available()) {

    // Handle request
    handle_proto(client,true,0,true);

    // Answer
    sendBuffer(client,0,0);
    client.stop();

    // Reset variables for the next command
    reset_status();

  }
}

// Handle request for the ESP8266 chip
#elif defined(ESP8266) || defined (ESP32)
void aREST::handle(WiFiClient& client){

  if (DEBUG_MODE) {
    Serial.print("Memory loss before available:");
    Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    freeMemory = ESP.getFreeHeap();
  }

  if (client.available()) {

    if (DEBUG_MODE) {
      Serial.print("Memory loss before handling:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
    }

    // Handle request
    handle_proto(client,true,0,true);

    if (DEBUG_MODE) {
      Serial.print("Memory loss after handling:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
    }

    // Answer
    sendBuffer(client,0,0);
    client.stop();

    // Reset variables for the next command
    reset_status();

  }
}

// Handle request on the Serial port
void aREST::handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}



// Handle request for the Arduino MKR1000 board
#elif defined(WIFI_H)
void aREST::handle(WiFiClient& client){

  if (client.available()) {

    if (DEBUG_MODE) {Serial.println("Request received");}

    // Handle request
    handle_proto(client,true,0,true);

    // Answer
    sendBuffer(client,0,0);
    client.stop();

    // Reset variables for the next command
    reset_status();
  }
}


// Handle request for the Arduino WiFi shield
#elif defined(WiFi_h)
void aREST::handle(WiFiClient& client){

  if (client.available()) {

    if (DEBUG_MODE) {Serial.println("Request received");}

    // Handle request
    handle_proto(client,true,0,true);

    // Answer
    sendBuffer(client,50,1);
    client.stop();

    // Reset variables for the next command
    reset_status();
  }
}



#elif defined(CORE_TEENSY)
// Handle request on the Serial port
void aREST::handle(usb_serial_class& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}



#elif defined(__AVR_ATmega32U4__)
// Handle request on the Serial port
void aREST::handle(Serial_& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}


#else
// Handle request on the Serial port
void aREST::handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}


#endif

void aREST::handle(char * string) {

  // Process String
  handle_proto(string);

  // Reset variables for the next command
  reset_status();
}

void aREST::handle_proto(char * string) {
  // Check if there is data available to read
  for (int i = 0; i < strlen(string); i++){

    char c = string[i];
    answer = answer + c;

    // Process data
    process(c);

  }

  // Send command
  send_command(false, false);
}



#if defined(PubSubClient_h)

// Process callback
void aREST::handle_callback(PubSubClient& client, char* topic, byte* payload, unsigned int length) {

  // Process received message
  int i;
  char mqtt_msg[100];
  for(i = 0; i < length; i++) {
    mqtt_msg[i] = payload[i];
  }
  mqtt_msg[i] = '\0';
  String msgString = String(mqtt_msg);

  if (DEBUG_MODE) {
    Serial.print("Received message via MQTT: ");
    Serial.println(msgString);
  }

  // Process aREST commands
    String modified_message = String(msgString) + " /";
    char char_message[100];
    modified_message.toCharArray(char_message, 100);

    // Handle command with aREST
    handle(char_message);

    // Read answer
    char * answer = getBuffer();

    // Send response
    if (DEBUG_MODE) {
      Serial.print("Sending message via MQTT: ");
      Serial.println(answer);
      Serial.print("Size of MQTT message: ");
      Serial.println(strlen(answer));
      Serial.print("Size of client ID: ");
      Serial.println(client_id.length());
    }

    int max_message_size = 128 - 20 - client_id.length();

    if (strlen(answer) < max_message_size) {
      client.publish(out_topic.c_str(), answer);
    }
    else {

      // Max iteration
      uint8_t max_iteration = (int)(strlen(answer)/max_message_size) + 1;

      // Send data
      for (uint8_t i = 0; i < max_iteration; i++) {
        char intermediate_buffer[max_message_size+1];
        memcpy(intermediate_buffer, buffer + i*max_message_size, max_message_size);
        intermediate_buffer[max_message_size] = '\0';

        if (DEBUG_MODE) {
          Serial.print("Intermediate buffer: ");
          Serial.println(intermediate_buffer);
          Serial.print("Intermediate buffer size: ");
          Serial.println(strlen(intermediate_buffer));
        }

        client.publish(out_topic.c_str(), intermediate_buffer);

    }

   }

    // Send message
    // client.publish(out_topic, answer);

    // Reset buffer
    resetBuffer();


}

// Handle request on the Serial port
void aREST::loop(PubSubClient& client){

  // Connect to cloud
  if (!client.connected()) {
    reconnect(client);
  }
  client.loop();

}

void aREST::handle(PubSubClient& client){

  // Connect to cloud
  if (!client.connected()) {
    reconnect(client);
  }
  client.loop();

}

void aREST::reconnect(PubSubClient& client) {

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));

      // Attempt to connect
      if (client.connect(client_id.c_str())) {
        if (private_mqtt_server) {
          Serial.println(F("Connected to MQTT server"));
        }
        else {
          Serial.println(F("Connected to aREST.io"));
        }
        client.subscribe(in_topic.c_str());

        // Subscribe to all
        // if (subscriptions_index > 0) {
        //
        //   for (int i = 0; i < subscriptions_index; i++) {
        //     if (DEBUG_MODE) {
        //       Serial.print(F("Subscribing to additional topic: "));
        //       Serial.println(subscriptions_names[i]);
        //     }
        //
        //     client.subscribe(subscriptions_names[i]);
        //   }
        //
        // }

      } else {
        Serial.print(F("failed, rc="));
        Serial.print(client.state());
        Serial.println(F(" try again in 5 seconds"));

        // Wait 5 seconds before retrying
        delay(5000);
      }

  }
}
#endif

void aREST::process(char c) {

  // Check if we are receveing useful data and process it

  if(state != 'u')
    return;

  if(c != '/' && c != '\r')
    return;


  if (DEBUG_MODE) {
    // #if defined(ESP8266)|| defined (ESP32)
    // Serial.print("Memory loss:");
    // Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    // freeMemory = ESP.getFreeHeap();
    // #endif
    Serial.println(answer);
  }

  // If the command is mode, and the pin is already selected
  if (command == 'm' && pin_selected && state == 'u') {

    // Get state
    state = answer[0];
  }

  // If a digital command has been received, process the data accordingly
  if (command == 'd' && pin_selected && state == 'u') {

    // If it's a read command, read from the pin and send data back
    if (answer[0] == 'r') {
      state = 'r';
    }

    // If not, get value we want to apply to the pin
    else {
      value = answer.toInt();
      state = 'w';
    }
  }

  // If analog command has been selected, process the data accordingly
  if (command == 'a' && pin_selected && state == 'u') {

    // If it's a read, read from the correct pin
    if (answer[0] == 'r') {
      state = 'r';
    }

    // Else, write analog value
    else {
      value = answer.toInt();
      state = 'w';
    }
  }

  // If the command is already selected, get the pin
  if (command != 'u' && pin_selected == false) {

    // Get pin
    if (answer[0] == 'A') {
      pin = 14 + answer[1] - '0';
    } else {
      pin = answer.toInt();
    }

    // Save pin for message
    message_pin = pin;

    // For ESP8266-12 boards (NODEMCU)
    #if defined(ARDUINO_ESP8266_NODEMCU) || defined(ARDUINO_ESP8266_WEMOS_D1MINI)
      pin = esp_12_pin_map(pin);
    #endif

    if (DEBUG_MODE) {
      Serial.print("Selected pin: ");
      Serial.println(pin);
    }

    // Mark pin as selected
    pin_selected = true;

    // Nothing more ?
    if ((answer[1] != '/' && answer[2] != '/') ||
        (answer[1] == ' ' && answer[2] == '/') ||
        (answer[2] == ' ' && answer[3] == '/')) {

      // Nothing more & digital ?
      if (command == 'd') {

        // Read all digital ?
        if (answer[0] == 'a') {
          state = 'a';
        }

        // Save state & end there
        else {
          state = 'r';
        }
      }

      // Nothing more & analog ?
      if (command == 'a') {

        // Read all analog ?
        if (answer[0] == 'a') {
          state = 'a';
        }

        // Save state & end there
        else {
          state = 'r';
        }
      }
    }
  }

  // Digital command received ?
  if (answer.startsWith("digital")) {
    command = 'd';
  }

  // Mode command received ?
  if (answer.startsWith("mode")) {
    command = 'm';
  }

  // Analog command received ?
  if (answer.startsWith("analog")) {
    command = 'a';

    #if defined(ESP8266)
      analogWriteRange(255);
    #endif
  }

  // Variable or function request received ?
  if (command == 'u') {

    // Check if variable name is in int array
    for (uint8_t i = 0; i < variables_index; i++) {
      if (answer.startsWith(variable_names[i])) {

        // End here
        pin_selected = true;
        state = 'x';

        // Set state
        command = 'v';
        value = i;

        break; // We found what we're looking for
      }
    }

    // Check if function name is in array
    for (uint8_t i = 0; i < functions_index; i++) {
      if (answer.startsWith(functions_names[i])) {

        // End here
        pin_selected = true;
        state = 'x';

        // Set state
        command = 'f';
        value = i;

        answer.trim();

        // We're expecting a string of the form <functionName>?xxxxx=<arguments>, where xxxxx can be almost anything as long as it's followed by an '='
        // Get command -- Anything following the first '=' in answer will be put in the arguments string.
        arguments = "";
        uint16_t header_length = strlen(functions_names[i]);
        if (answer.substring(header_length, header_length + 1) == "?") {
          uint16_t footer_start = answer.length();
          if (answer.endsWith(" HTTP/"))
            footer_start -= 6; // length of " HTTP/"

          // Standard operation --> strip off anything preceeding the first "=", pass the rest to the function
          if(AREST_PARAMS_MODE == 0) {
            uint16_t eq_position = answer.indexOf('=', header_length); // Replacing 'magic number' 8 for fixed location of '='
            if (eq_position != -1)
              arguments = answer.substring(eq_position + 1, footer_start);
          } 
          // All params mode --> pass all parameters, if any, to the function.  Function will be resonsible for parsing
          else if(AREST_PARAMS_MODE == 1) {
            arguments = answer.substring(header_length + 1, footer_start);
          }
        }

        break; // We found what we're looking for
      }
    }

    // If the command is "id", return device id, name and status
    if (command == 'u' && (answer[0] == 'i' && answer[1] == 'd')) {

      // Set state
      command = 'i';

      // End here
      pin_selected = true;
      state = 'x';
    }

    if (command == 'u' && answer[0] == ' ') {

      // Set state
      command = 'r';

      // End here
      pin_selected = true;
      state = 'x';
    }

    // Check the type of HTTP request
    // if (answer.startsWith("GET")) {method = "GET";}
    // if (answer.startsWith("POST")) {method = "POST";}
    // if (answer.startsWith("PUT")) {method = "PUT";}
    // if (answer.startsWith("DELETE")) {method = "DELETE";}

    // if (DEBUG_MODE && method != "") {
    //  Serial.print("Selected method: ");
    //  Serial.println(method);
    // }
  }

  answer = "";
}


// Modifies arguments in place
void aREST::urldecode(String &arguments) {
  char a, b;
  int j = 0;
  for(int i = 0; i < arguments.length(); i++) {
    // %20 ==> arguments[i] = '%', a = '2', b = '0'
    if ((arguments[i] == '%') && ((a = arguments[i + 1]) && (b = arguments[i + 2])) && (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a') a -= 'a'-'A';
      if (a >= 'A') a -= ('A' - 10);
      else          a -= '0';

      if (b >= 'a') b -= 'a'-'A';
      if (b >= 'A') b -= ('A' - 10);
      else          b -= '0';

      arguments[j] = char(16 * a + b);
      i += 2;   // Skip ahead
    } else if (arguments[i] == '+') {
      arguments[j] = ' ';
    } else {
     arguments[j] = arguments[i];
    }
    j++;
  }

  arguments.remove(j);    // Truncate string to new possibly reduced length
}


bool aREST::send_command(bool headers, bool decodeArgs) {

  if (DEBUG_MODE) {

    #if defined(ESP8266) || defined(ESP32)
      Serial.print("Memory loss:");
      Serial.println(freeMemory - ESP.getFreeHeap(), DEC);
      freeMemory = ESP.getFreeHeap();
    #endif

    Serial.println(F("Sending command"));
    Serial.print(F("Command: "));
    Serial.println(command);
    Serial.print(F("State: "));
    Serial.println(state);
    Serial.print(F("State of buffer at the start: "));
    Serial.println(buffer);
  }

  // Start of message
  if (headers && command != 'r') {
    send_http_headers();
  }

  // Mode selected
  if (command == 'm') {

    // Send feedback to client
    if (!LIGHTWEIGHT) {
      addToBufferF(F("{\"message\": \"Pin D"));
      addToBuffer(message_pin, false);
    }

    // Input
    if (state == 'i') {

      // Set pin to Input
      pinMode(pin, INPUT);

      // Send feedback to client
      if (!LIGHTWEIGHT) {
        addToBufferF(F(" set to input\", "));
      }
    }

    // Input with pullup
    if (state == 'I') {

      // Set pin to Input with pullup
      pinMode(pin, INPUT_PULLUP);

      // Send feedback to client
      if (!LIGHTWEIGHT) {
        addToBufferF(F(" set to input with pullup\", "));
      }
    }

    // Output
    if (state == 'o') {

      // Set to Output
      pinMode(pin, OUTPUT);

      // Send feedback to client
      if (!LIGHTWEIGHT) {
        addToBufferF(F(" set to output\", "));
      }
    }
  }

  // Digital selected
  if (command == 'd') {
    if (state == 'r') {

      // Read from pin
      value = digitalRead(pin);

      // Send answer
      if (LIGHTWEIGHT) {
        addToBuffer(value, false);
      } else {
        addToBufferF(F("{\"return_value\": "));
        addToBuffer(value, true);
        addToBufferF(F(", "));
      }
    }

    #if !defined(__AVR_ATmega32U4__) || !defined(ADAFRUIT_CC3000_H)
      if (state == 'a') {
        if (!LIGHTWEIGHT) {
          addToBufferF(F("{"));
        }

        for (uint8_t i = 0; i < NUMBER_DIGITAL_PINS; i++) {

          // Read analog value
          value = digitalRead(i);

          // Send feedback to client
          if (LIGHTWEIGHT) {
            addToBuffer(value, false);
            addToBufferF(F(","));
          } else {
            addToBufferF(F("\"D"));
            addToBuffer(i, false);
            addToBufferF(F("\": "));
            addToBuffer(value, true);
            addToBufferF(F(", "));
          }
        }
      }
    #endif

    if (state == 'w') {

    // Disable analogWrite if ESP8266
    #if defined(ESP8266)
      analogWrite(pin, 0);
    #endif

      // Apply on the pin
      digitalWrite(pin, value);

      // Send feedback to client
      if (!LIGHTWEIGHT) {
        addToBufferF(F("{\"message\": \"Pin D"));
        addToBuffer(message_pin, false);
        addToBufferF(F(" set to "));
        addToBuffer(value, false);
        addToBufferF(F("\", "));
      }
    }
  }

  // Analog selected
  if (command == 'a') {
    if (state == 'r') {

      // Read analog value
      value = analogRead(pin);

      // Send feedback to client
      if (LIGHTWEIGHT) {
        addToBuffer(value, false);
      } else {
        addToBufferF(F("{\"return_value\": "));
        addToBuffer(value, true);
        addToBufferF(F(", "));
      }
    }
    
    #if !defined(__AVR_ATmega32U4__)
      if (state == 'a') {
        if (!LIGHTWEIGHT) {
          addToBufferF(F("{"));
        }

        for (uint8_t i = 0; i < NUMBER_ANALOG_PINS; i++) {

          // Read analog value
          value = analogRead(i);

          // Send feedback to client
          if (LIGHTWEIGHT) {
            addToBuffer(value, false);
            addToBufferF(F(","));
          } else {
            addToBufferF(F("\"A"));
            addToBuffer(i, false);
            addToBufferF(F("\": "));
            addToBuffer(value, true);
            addToBufferF(F(", "));
          }
        }
      }
    #endif

    if (state == 'w') {

      // Write output value
      #if !defined(ESP32)
            analogWrite(pin, value);
      #endif

      // Send feedback to client
      addToBufferF(F("{\"message\": \"Pin D"));
      addToBuffer(message_pin, false);
      addToBufferF(F(" set to "));
      addToBuffer(value, false);
      addToBufferF(F("\", "));
    }
  }

  // Variable selected
  if (command == 'v') {
    // Send feedback to client
    if (LIGHTWEIGHT) {
      variables[value]->addToBuffer(this);
    } else {
      addToBufferF(F("{"));
      addVariableToBuffer(value);
      addToBufferF(F(", "));
    }
  }

  // Function selected
  if (command == 'f') {

    // Execute function
    if (decodeArgs)
      urldecode(arguments); // Modifies arguments

    int result = functions[value](arguments);

    // Send feedback to client
    if (!LIGHTWEIGHT) {
      addToBufferF(F("{\"return_value\": "));
      addToBuffer(result, true);
      addToBufferF(F(", "));
      // addToBufferF(F(", \"message\": \""));
      // addStringToBuffer(functions_names[value]);
      // addToBufferF(F(" executed\", "));
    }
  }

  if (command == 'r' || command == 'u') {
    root_answer();
  }

  if (command == 'i') {
    if (LIGHTWEIGHT) {
      addStringToBuffer(id.c_str(), false);
    } else {
      addToBufferF(F("{"));
    }
  }

  // End of message
  if (LIGHTWEIGHT) {
    addToBufferF(F("\r\n"));
  }

  else {
    if (command != 'r' && command != 'u') {
      addHardwareToBuffer();
      addToBufferF(F("\r\n"));
    }
  }

  if (DEBUG_MODE) {
    #if defined(ESP8266) || defined(ESP32)
        Serial.print("Memory loss:");
        Serial.println(freeMemory - ESP.getFreeHeap(), DEC);
        freeMemory = ESP.getFreeHeap();
    #endif
    Serial.print(F("State of buffer at the end: "));
    Serial.println(buffer);
  }

  // End here
  return true;
}


/*virtual*/ void aREST::root_answer() {

  #if defined(ADAFRUIT_CC3000_H) || defined(ESP8266) || defined(ethernet_h_) || defined(WiFi_h)
    #if !defined(PubSubClient_h)
      if (command != 'u') {
        send_http_headers();
      }
    #endif
  #endif

  if (LIGHTWEIGHT) {
    addStringToBuffer(id.c_str(), false);
  }
  else {
    addToBufferF(F("{\"variables\": {"));

    for (uint8_t i = 0; i < variables_index; i++){
      addVariableToBuffer(i);

      if (i < variables_index - 1) {
        addToBufferF(F(", "));
      }
    }

    addToBufferF(F("}, "));
  }

  // End
  addHardwareToBuffer();

  #ifndef PubSubClient_h
    addToBufferF(F("\r\n"));
  #endif
}


void aREST::function(char * function_name, int (*f)(String)){

  functions_names[functions_index] = function_name;
  functions[functions_index] = f;
  functions_index++;
}

// Set device ID
void aREST::set_id(const String& device_id) {

  id = device_id.substring(0, ID_SIZE);

  #if defined(PubSubClient_h)

  if (proKey.length() == 0) {

      // Generate MQTT random ID
      String randomId = gen_random(6);

      // Build topics IDs
      in_topic = randomId + id + String("_in");
      out_topic = randomId + id + String("_out");

      // strcpy(in_topic, inTopic.c_str());
      // strcpy(out_topic, outTopic.c_str());

      // Build client ID
      client_id = randomId + id;

  }
  else {

      // Build topics IDs
      in_topic = id + String(proKey) + String("_in");
      out_topic = id + String(proKey) + String("_out");

      // strcpy(in_topic, keyInTopic.c_str());
      // strcpy(out_topic, keyOutTopic.c_str());

      Serial.print("In topic: ");
      Serial.println(in_topic);
      Serial.println("");

      Serial.print("Out topic: ");
      Serial.println(out_topic);

      // Build client ID
      client_id = id + String(proKey);
      
  }

  #endif

}

#if defined(__arm__)
String aREST::getChipId() {

  volatile uint32_t val1, val2, val3, val4;
  volatile uint32_t *ptr1 = (volatile uint32_t *)0x0080A00C;
  val1 = *ptr1;
  volatile uint32_t *ptr = (volatile uint32_t *)0x0080A040;
  val2 = *ptr;
  ptr++;
  val3 = *ptr;
  ptr++;
  val4 = *ptr;

  char buf[33];
  sprintf(buf, "%8x%8x%8x%8x", val1, val2, val3, val4);

  return String(buf);
}
#endif

#if defined(PubSubClient_h)
String aREST::gen_random(int length) {

  String randomString;

  #if defined(ESP8266)

    randomString = String(ESP.getChipId());
    randomString = randomString.substring(0, 6);

  #elif defined(__arm__)

    randomString = getChipId();
    randomString = randomString.substring(0, 6);

  #else

  String charset = "abcdefghijklmnopqrstuvwxyz0123456789";

  // Generate
  int l = charset.length();
  int key;
  for (int n = 0; n < length; n++) {
    key = random(0, l - 1);
    randomString += charset[key];
  }

  #endif

  return randomString;
}
#endif

// Set device name
void aREST::set_name(char *device_name){

  strcpy(name, device_name);
}

// Set device name
void aREST::set_name(const String& device_name){

  device_name.toCharArray(name, NAME_SIZE);
}

// Remove last char from buffer
void aREST::removeLastBufferChar() {

  index = index - 1;

}


void aREST::addQuote() {
  if(index < OUTPUT_BUFFER_SIZE) {
    buffer[index] = '"';
    index++;
  }  
}


void aREST::addStringToBuffer(const char * toAdd, bool quotable){

  if (DEBUG_MODE) {
    #if defined(ESP8266)|| defined (ESP32)
      Serial.print("Memory loss:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
    #endif
    Serial.print(F("Added to buffer as char: "));
    Serial.println(toAdd);
  }

  if(quotable) {
    addQuote();
  }

  for (int i = 0; i < strlen(toAdd) && index < OUTPUT_BUFFER_SIZE; i++, index++) {
    // Handle quoting quotes and backslashes
    if(quotable && (toAdd[i] == '"' || toAdd[i] == '\\')) {
      if(index == OUTPUT_BUFFER_SIZE - 1)   // No room!
        return;
      buffer[index] = '\\';
      index++;
    }

    buffer[index] = toAdd[i];
  }

  if(quotable) {
    addQuote();
  }
}


// Add to output buffer


// // Add to output buffer
// void aREST::addToBuffer(const __FlashStringHelper *toAdd, bool quotable){

//   if (DEBUG_MODE) {
//     #if defined(ESP8266)|| defined (ESP32)
//     Serial.print("Memory loss:");
//     Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
//     freeMemory = ESP.getFreeHeap();
//     #endif
//     Serial.print(F("Added to buffer as progmem: "));
//     Serial.println(toAdd);
//   }

//   if(quotable) {
//     addQuote();
//   }

//   uint8_t idx = 0;

//   PGM_P p = reinterpret_cast<PGM_P>(toAdd);

//   for ( unsigned char c = pgm_read_byte(p++);
//         c != 0 && index < OUTPUT_BUFFER_SIZE;
//         c = pgm_read_byte(p++), index++) {
//     buffer[index] = c;
//   }

//   if(quotable) {
//     addQuote();
//   }
// }



char * aREST::getBuffer() {
  return buffer;
}

void aREST::resetBuffer(){

  memset(&buffer[0], 0, sizeof(buffer));
  // free(buffer);

}

uint8_t aREST::esp_12_pin_map(uint8_t pin) {

  // Right pin
  uint8_t mapped_pin;

  // Map
  switch (pin) {

    case 0:
      mapped_pin = 16;
      break;
    case 1:
      mapped_pin = 5;
      break;
    case 2:
      mapped_pin = 4;
      break;
    case 3:
      mapped_pin = 0;
      break;
    case 4:
      mapped_pin = 2;
      break;
    case 5:
      mapped_pin = 14;
      break;
    case 6:
      mapped_pin = 12;
      break;
    case 7:
      mapped_pin = 13;
      break;
    case 8:
      mapped_pin = 15;
      break;
    case 9:
      mapped_pin = 3;
      break;
    case 10:
      mapped_pin = 1;
      break;
    default:
      mapped_pin = 0;
  }

  return mapped_pin;

}


void aREST::addVariableToBuffer(uint8_t index) {
  addStringToBuffer(variable_names[index], true);
  addToBufferF(F(": "));
  variables[index]->addToBuffer(this);
}


void aREST::addHardwareToBuffer() {
  addToBufferF(F("\"id\": "));
  addStringToBuffer(id.c_str(), true);
  addToBufferF(F(", \"name\": "));
  addStringToBuffer(name, true);
  addToBufferF(F(", \"hardware\": "));
  addStringToBuffer(HARDWARE, true);
  addToBufferF(F(", \"connected\": true}"));
}


// For non AVR boards
#if defined (__arm__)
char *aREST::dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
#endif

// Memory debug
#if defined(ESP8266) || defined(ESP32)
void aREST::initFreeMemory(){
  freeMemory = ESP.getFreeHeap();
}
#endif

#if defined(PubSubClient_h)
void aREST::setMQTTServer(char* new_mqtt_server){
  mqtt_server = new_mqtt_server;
}
#endif



