/*
  aREST Library for Arduino
  See the README file for more details.

  Written in 2014 by Marco Schwartz.
  Modified for MQTT cloud connectivity.

  This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License:
  http://creativecommons.org/licenses/by-sa/4.0/

  Version 3.1.2
  Changelog:

  Version 3.1.2: Migrated to MQTT protocol for cloud connectivity
                 Uses PubSubClient for MQTT
                 Topic structure: devices/{device_id}/commands, /response, /telemetry, /state
                 Authentication: username=device_id, password=api_key
  Version 3.0.0: Migrated from MQTT to WebSocket protocol for cloud connectivity
  Version 2.9.7: Added URL for mqtt server
  Version 2.9.6: Add ID generator for ESP32
*/

#ifndef aRest_h
#define aRest_h

// Include Arduino header
#include "Arduino.h"

// MQTT packet size
#define MQTT_MAX_PACKET_SIZE 1024

// Using ESP8266 ?
#if defined(ESP8266) || defined(ESP32)
#include "stdlib_noniso.h"
#endif

// MQTT support for cloud connectivity
#if defined(ESP8266) || defined(ESP32)
  #include <PubSubClient.h>
  #include <ArduinoJson.h>

  // WiFi includes for status LED
  #if defined(ESP8266)
    #include <ESP8266WiFi.h>
  #elif defined(ESP32)
    #include <WiFi.h>
  #endif
#endif

// Which board?
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(CORE_WILDFIRE) || defined(ESP8266) || defined(ESP32)
#define NUMBER_ANALOG_PINS 16
#define NUMBER_DIGITAL_PINS 54
#define OUTPUT_BUFFER_SIZE 2000
#elif defined(__AVR_ATmega328P__) && !defined(ADAFRUIT_CC3000_H)
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#define OUTPUT_BUFFER_SIZE 350
#elif defined(ADAFRUIT_CC3000_H)
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#define OUTPUT_BUFFER_SIZE 275
#else
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#define OUTPUT_BUFFER_SIZE 350
#endif

// Hardware data
#if defined(ESP8266)
#define HARDWARE "esp8266"
#elif defined(ESP32)
#define HARDWARE "esp32"
#else
#define HARDWARE "arduino"
#endif

// Size of name & ID
#define NAME_SIZE 20
#define ID_SIZE 64  // Increased to support longer device IDs

// Subscriptions
#define NUMBER_SUBSCRIPTIONS 4

// Debug mode
// Set to 1 to enable detailed MQTT debugging output
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif


// Use AREST_PARAMS_MODE to control how parameters are parsed when using the function() method.
#ifndef AREST_PARAMS_MODE
#define AREST_PARAMS_MODE 0
#endif

// Use light answer mode
#ifndef LIGHTWEIGHT
#define LIGHTWEIGHT 0
#endif

#ifdef AREST_NUMBER_VARIABLES
#define NUMBER_VARIABLES AREST_NUMBER_VARIABLES
#endif

#ifdef AREST_NUMBER_FUNCTIONS
#define NUMBER_FUNCTIONS AREST_NUMBER_FUNCTIONS
#endif

// Default number of max. exposed variables
#ifndef NUMBER_VARIABLES
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(CORE_WILDFIRE) || defined(ESP8266)|| defined(ESP32) || !defined(ADAFRUIT_CC3000_H)
  #define NUMBER_VARIABLES 10
  #else
  #define NUMBER_VARIABLES 5
  #endif
#endif

// Default number of max. exposed functions
#ifndef NUMBER_FUNCTIONS
  #if defined(__AVR_ATmega1280__) || defined(ESP32) || defined(__AVR_ATmega2560__) || defined(CORE_WILDFIRE) || defined(ESP8266)
  #define NUMBER_FUNCTIONS 10
  #else
  #define NUMBER_FUNCTIONS 5
  #endif
#endif


#ifdef AREST_BUFFER_SIZE
  #define OUTPUT_BUFFER_SIZE AREST_BUFFER_SIZE
#endif

#if defined(ESP8266) || defined(ESP32)

// Forward declaration
class aREST;

// Default MQTT server settings
#define AREST_MQTT_SERVER "mqtt.arest.io"
#define AREST_MQTT_PORT 1883

// MQTT client wrapper for cloud connectivity
class MQTTClient {
public:
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  String mqttHost = AREST_MQTT_SERVER;  // Default server
  uint16_t mqttPort = AREST_MQTT_PORT;  // Default port
  String deviceId;
  String apiKey;
  String lastCommandId;
  void (*messageCallback)(char*, byte*, unsigned int);
  unsigned long lastPingTime;
  unsigned long pingInterval = 30000;
  bool isConnected = false;
  aREST* arest = nullptr;

  // Status LED variables
  int statusLedPin = -1;  // -1 means disabled
  unsigned long lastLedUpdate = 0;
  int ledBrightness = 0;
  int ledDirection = 5;
  const long ledBlinkInterval = 500;
  const long ledPulseInterval = 20;

  // Topics
  String commandsTopic;
  String responseTopic;
  String telemetryTopic;
  String stateTopic;

  MQTTClient() : mqttClient(wifiClient) {
    // Set default server on construction
    mqttClient.setServer(AREST_MQTT_SERVER, AREST_MQTT_PORT);
  }

  void setServer(const char* host, uint16_t port);

  void setStatusLED(int pin);

  void setCallback(void (*callback)(char*, uint8_t*, unsigned int));

  bool connect(const String& clientId);

  bool connected();

  void loop();

  bool publish(const String& topic, const char* payload);

  bool publishTelemetry(const char* payload);

  bool publishState(const char* payload);

  bool publishResponse(const char* payload);

  bool subscribe(const String& topic);

private:
  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  void sendCommandResponse(const String& commandId, const String& status, const String& message);
  void sendCommandResponse(const String& commandId, const String& status, const String& message, int value);
  void sendPing();
  void updateStatusLED();
};

#endif // ESP8266 || ESP32

class aREST {

friend class MQTTClient;

private:
struct Variable {
  char type;  // 'i' for int, 'f' for float, 's' for string
  void* ptr;  // Pointer to the actual variable
  virtual void addToBuffer(aREST *arest) const = 0;
};


template<typename T>
struct TypedVariable: Variable {
  T *var;
  bool quotable;

  TypedVariable(T *v, bool q, char t) : var{v} {
    quotable = q;
    type = t;
    ptr = v;
  }

  void addToBuffer(aREST *arest) const override {
    arest->addToBuffer(*var, quotable);
  }
};

public:

public:

aREST() {
  initialize();
}

aREST(char* rest_remote_server, int rest_port) {

  initialize();

  remote_server = rest_remote_server;
  port = rest_port;

}


// Generic template - defaults to integer type
template<typename T>
void variable(const char *name, T *var, bool quotable) {
  variables[variables_index] = new TypedVariable<T>(var, quotable, 'i');
  variable_names[variables_index] = name;
  variables_index++;
}

// Specialization for int
void variable(const char *name, int *var, bool quotable) {
  variables[variables_index] = new TypedVariable<int>(var, quotable, 'i');
  variable_names[variables_index] = name;
  variables_index++;
}

// Specialization for float
void variable(const char *name, float *var, bool quotable) {
  variables[variables_index] = new TypedVariable<float>(var, quotable, 'f');
  variable_names[variables_index] = name;
  variables_index++;
}

// Specialization for double
void variable(const char *name, double *var, bool quotable) {
  variables[variables_index] = new TypedVariable<double>(var, quotable, 'f');
  variable_names[variables_index] = name;
  variables_index++;
}

// Specialization for String
void variable(const char *name, String *var, bool quotable) {
  variables[variables_index] = new TypedVariable<String>(var, quotable, 's');
  variable_names[variables_index] = name;
  variables_index++;
}

// Specialization for char arrays
void variable(const char *name, char *var, bool quotable) {
  variables[variables_index] = new TypedVariable<char>(var, quotable, 's');
  variable_names[variables_index] = name;
  variables_index++;
}

template<typename T>
void variable(const char *name, T *var) {
  variable(name, var, true);
}


private:

void initialize() {
  reset();
  status_led_pin = 255;
}

// Used when resetting object back to oringial state
void reset() {
  command = 'u';
  state = 'u';
  pin_selected = false;
}

public:


// Constructor that accepts MQTTClient for cloud connectivity
aREST(MQTTClient& client) {
  initialize();
  mqttClient = &client;

  // Set up automatic callback handling
  client.arest = this;
}

// Get topic
String get_topic() {
  return out_topic;
}

// Subscribe to events
void subscribe(const String& device, const String& eventName) {

  // Build topic
  String topic = device + "_" + eventName + "_in";

  // Subscribe
  char charBuf[50];
  topic.toCharArray(charBuf, 50);

  subscriptions_names[subscriptions_index] = charBuf;
  subscriptions_index++;

}


template <typename T>
void publish(MQTTClient& client, const String& eventName, T data) {

  uint32_t currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    // Get event data
    if (DEBUG_MODE) {
      Serial.print("Publishing event " + eventName + " with data: ");
      Serial.println(data);
    }

    // Build telemetry message
    StaticJsonDocument<512> doc;
    doc[eventName] = data;

    String message;
    serializeJson(doc, message);

    if (DEBUG_MODE) {
      Serial.print("Sending telemetry via MQTT: ");
      Serial.println(message);
    }

    // Publish via MQTT
    client.publishTelemetry(message.c_str());

  }

}


template <typename T>
void publish(MQTTClient& client, const String& eventName, T data, uint32_t customInterval) {

  uint32_t currentMillis = millis();

  if (currentMillis - previousMillis >= customInterval) {

    previousMillis = currentMillis;

    // Get event data
    if (DEBUG_MODE) {
      Serial.print("Publishing event " + eventName + " with data: ");
      Serial.println(data);
    }

    // Build telemetry message
    StaticJsonDocument<512> doc;
    doc[eventName] = data;

    String message;
    serializeJson(doc, message);

    if (DEBUG_MODE) {
      Serial.print("Sending telemetry via MQTT: ");
      Serial.println(message);
    }

    // Publish via MQTT
    client.publishTelemetry(message.c_str());

  }

}

void setKey(const char* api_key) {
  Serial.print(F("[aREST] Setting API key: "));
  Serial.println(api_key);

  // Set
  proKey = String(api_key);

  if (id.length() == 0) {
    Serial.println(F("[aREST] No device ID set, generating random ID"));
    // Generate random ID
    id = gen_random(6);
    Serial.print(F("[aREST] Generated ID: "));
    Serial.println(id);
  }

  // Build topics IDs (for MQTT)
  in_topic = "devices/" + id + "/commands";
  out_topic = "devices/" + id + "/response";
  publish_topic = "devices/" + id + "/telemetry";

  // Build client ID
  client_id = id + "_" + String(millis() % 10000);

  Serial.print(F("[aREST] Client ID: "));
  Serial.println(client_id);
}

// Set status LED
void set_status_led(uint8_t pin){

  // Set variables
  status_led_pin = pin;

  // Set pin as output
  pinMode(status_led_pin, OUTPUT);
}

#if !defined(ESP32)
// Glow status LED
void glow_led() {

  if(status_led_pin != 255){
    unsigned long i = millis();
    int j = i % 4096;
    if (j > 2048) { j = 4096 - j;}
      analogWrite(status_led_pin,j/8);
    }
}
#endif

void addToBufferF(const __FlashStringHelper *toAdd){

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
void send_http_headers(){

  addToBufferF(F("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"));

}

// Reset variables after a request
void reset_status() {

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
void handle(Adafruit_CC3000_ClientRef& client) {

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

template <typename T>
void publish(Adafruit_CC3000_ClientRef& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request with the Arduino Yun
#elif defined(_YUN_CLIENT_H_)
void handle(YunClient& client) {

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

template <typename T>
void publish(YunClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request with the Adafruit BLE board
#elif defined(_ADAFRUIT_BLE_UART_H_)
void handle(Adafruit_BLE_UART& serial) {

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
void handle(EthernetClient& client){

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

template <typename T>
void publish(EthernetClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the Cytron Clone ESP8266
#elif defined(_CYTRONWIFISERVER_H_)
void handle(ESP8266Client& client){

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
void handle(WiFiClient& client){

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
void handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the Arduino MKR1000 board
#elif defined(WIFI_H)
void handle(WiFiClient& client){

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

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the Arduino WiFi shield
#elif defined(WiFi_h)
void handle(WiFiClient& client){

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

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

#elif defined(CORE_TEENSY)
// Handle request on the Serial port
void handle(usb_serial_class& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}

template <typename T>
void publish(usb_serial_class& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

#elif defined(__AVR_ATmega32U4__)
// Handle request on the Serial port
void handle(Serial_& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}

template <typename T>
void publish(Serial_& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

#else
// Handle request on the Serial port
void handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1,false);

    // Answer
    sendBuffer(serial,25,1);

    // Reset variables for the next command
    reset_status();
  }
}

template <typename T>
void publish(HardwareSerial& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}
#endif

void handle(char * string) {

  // Process String
  handle_proto(string);

  // Reset variables for the next command
  reset_status();
}

void handle_proto(char * string) {
  // Check if there is data available to read
  if (DEBUG_MODE) {
    Serial.print("[HANDLE_PROTO] Processing string: ");
    Serial.println(string);
  }

  for (int i = 0; i < strlen(string); i++){

    char c = string[i];
    answer = answer + c;

    // Process data
    process(c);

  }

  if (DEBUG_MODE) {
    Serial.print("[HANDLE_PROTO] Final answer: ");
    Serial.println(answer);
    Serial.print("[HANDLE_PROTO] Command: ");
    Serial.println(command);
    Serial.print("[HANDLE_PROTO] State: ");
    Serial.println(state);
  }

  // Send command
  send_command(false, false);
}

template <typename T, typename V>
void publish_proto(T& client, const String& eventName, V value) {

  // Format data
  String data = "name=" + eventName + "&data=" + String(value);

  Serial.println("POST /" + id + "/events HTTP/1.1");
  Serial.println("Host: " + String(remote_server) + ":" + String(port));
  Serial.println(F("Content-Type: application/x-www-form-urlencoded"));
  Serial.print(F("Content-Length: "));
  Serial.println(data.length());
  Serial.println();
  Serial.print(data);

  // Send request
  client.println(F("POST /1/events HTTP/1.1"));
  client.println("Host: " + String(remote_server) + ":" + String(port));
  client.println(F("Content-Type: application/x-www-form-urlencoded"));
  client.print(F("Content-Length: "));
  client.println(data.length());
  client.println();
  client.print(data);

}

template <typename T>
void handle_proto(T& serial, bool headers, uint8_t read_delay, bool decode)
{

  // Check if there is data available to read
  while (serial.available()) {

    // Get the server answer
    char c = serial.read();
    delay(read_delay);
    answer = answer + c;
    //if (DEBUG_MODE) {Serial.print(c);}

    // Process data
    process(c);

   }

   // Send command
   send_command(headers, decode);
}

#if defined(ESP8266) || defined(ESP32)

// Process callback for MQTT
void handle_callback(MQTTClient& client, char* topic, byte* payload, unsigned int length) {

  // Process received message
  int i;
  char mqtt_msg[MQTT_MAX_PACKET_SIZE];
  for(i = 0; i < length && i < MQTT_MAX_PACKET_SIZE - 1; i++) {
    mqtt_msg[i] = payload[i];
  }
  mqtt_msg[i] = '\0';
  String msgString = String(mqtt_msg);

  if (DEBUG_MODE) {
    Serial.println("[CB] === handle_callback called ===");
    Serial.print("[CB] Topic: ");
    Serial.println(topic);
    Serial.print("[CB] Payload: ");
    Serial.println(msgString);
    Serial.print("[CB] Length: ");
    Serial.println(length);
  }

  // Process aREST commands
  String modified_message = String(msgString) + " /";
  char char_message[MQTT_MAX_PACKET_SIZE];
  modified_message.toCharArray(char_message, MQTT_MAX_PACKET_SIZE);

  if (DEBUG_MODE) {
    Serial.print("[CB] Modified message for processing: ");
    Serial.println(char_message);
  }

  // Handle command with aREST
  handle(char_message);

  // Read answer
  char * answer = getBuffer();

  if (DEBUG_MODE) {
    Serial.print("[CB] Buffer content after handle(): ");
    Serial.println(answer);
    Serial.print("[CB] Buffer length: ");
    Serial.println(strlen(answer));
  }

  // Send response via MQTT
  if (DEBUG_MODE) {
    Serial.print("Sending response via MQTT: ");
    Serial.println(answer);
  }

  // Send command response
  if (client.lastCommandId.length() > 0) {
    StaticJsonDocument<1024> doc;
    doc["command_id"] = client.lastCommandId;
    doc["device_id"] = client.deviceId;
    doc["success"] = true;
    doc["timestamp"] = String(millis());

    // Parse response
    StaticJsonDocument<512> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, answer);
    if (!error) {
      doc["result"] = responseDoc;
    } else {
      doc["result"]["raw"] = answer;
    }

    String message;
    serializeJson(doc, message);
    client.publishResponse(message.c_str());
  }

  // Reset buffer
  resetBuffer();
}


// Handle request for MQTT
void handle(MQTTClient& client){
  // Set up callback if not already set
  if (!client.messageCallback) {
    // Store pointer to this instance in a static variable
    static aREST* instance = nullptr;
    instance = this;

    // Set callback using static lambda
    client.setCallback([](char* topic, uint8_t* payload, unsigned int length) {
      if (instance && instance->mqttClient) {
        instance->handle_callback(*(instance->mqttClient), topic, payload, length);
      }
    });
  }

  // Connect to cloud if needed
  if (!client.connected()) {
    reconnect(client);
  }
  client.loop();
}


void reconnect(MQTTClient& client) {
  // Attempt to connect
  if (!client.connected()) {
    Serial.println(F("\n[RECONNECT] Starting reconnection process..."));

    // Set default server if not set
    if (client.mqttHost.length() == 0) {
      Serial.println(F("[RECONNECT] No server set, using defaults"));
      client.mqttHost = String(mqtt_server);
      client.mqttPort = mqtt_port;
      Serial.print(F("[RECONNECT] Default server: "));
      Serial.print(client.mqttHost);
      Serial.print(F(":"));
      Serial.println(client.mqttPort);
    } else {
      Serial.print(F("[RECONNECT] Using configured server: "));
      Serial.print(client.mqttHost);
      Serial.print(F(":"));
      Serial.println(client.mqttPort);
    }

    // Set device info
    if (client.deviceId != id) {
      Serial.print(F("[RECONNECT] Setting device ID from '"));
      Serial.print(client.deviceId);
      Serial.print(F("' to '"));
      Serial.print(id);
      Serial.println(F("'"));
      client.deviceId = id;
    }

    if (client.apiKey != proKey) {
      Serial.println(F("[RECONNECT] Updating API key"));
      client.apiKey = proKey;
    }

    Serial.print(F("[RECONNECT] Using client_id for connection: "));
    Serial.println(client_id);

    // Attempt to connect
    if (client.connect(client_id)) {
      Serial.println(F("[RECONNECT] Connected to MQTT broker"));
    } else {
      Serial.println(F("[RECONNECT] Connection failed, will retry in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
#endif

void process(char c) {

  // Check if we are receveing useful data and process it

  if(state != 'u')
    return;

  if(c != '/' && c != '\r')
    return;


  if (DEBUG_MODE) {
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
    if (DEBUG_MODE) Serial.println("[PROCESS] Digital command detected");
  }

  // Mode command received ?
  if (answer.startsWith("mode")) {
    command = 'm';
    if (DEBUG_MODE) Serial.println("[PROCESS] Mode command detected");
  }

  // Analog command received ?
  if (answer.startsWith("analog")) {
    command = 'a';
    if (DEBUG_MODE) Serial.println("[PROCESS] Analog command detected");

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

        // We're expecting a string of the form <functionName>?xxxxx=<arguments>
        arguments = "";
        uint16_t header_length = strlen(functions_names[i]);
        if (answer.substring(header_length, header_length + 1) == "?") {
          uint16_t footer_start = answer.length();
          if (answer.endsWith(" HTTP/"))
            footer_start -= 6; // length of " HTTP/"

          if(AREST_PARAMS_MODE == 0) {
            uint16_t eq_position = answer.indexOf('=', header_length);
            if (eq_position != -1)
              arguments = answer.substring(eq_position + 1, footer_start);
          }
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
  }

  answer = "";
}


// Modifies arguments in place
void urldecode(String &arguments) {
  char a, b;
  int j = 0;
  for(int i = 0; i < arguments.length(); i++) {
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


bool send_command(bool headers, bool decodeArgs) {

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


virtual void root_answer() {

  #if defined(ADAFRUIT_CC3000_H) || defined(ESP8266) || defined(ethernet_h_) || defined(WiFi_h)
    if (command != 'u') {
      send_http_headers();
    }
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

  addToBufferF(F("\r\n"));
}


void function(const char* function_name, int (*f)(String)){

  functions_names[functions_index] = function_name;
  functions[functions_index] = f;
  functions_index++;
}

// Set device ID
void set_id(const String& device_id) {
  Serial.print(F("[aREST] Setting device ID: "));
  Serial.println(device_id);

  // Store full device ID without truncation
  id = device_id;
  Serial.print(F("[aREST] ID stored as: "));
  Serial.println(id);

  #if defined(ESP8266) || defined(ESP32)

  if (proKey.length() == 0) {
      Serial.println(F("[aREST] No API key set yet, using random prefix"));

      // Generate random ID
      String randomId = gen_random(6);

      // Build topics IDs for MQTT
      in_topic = "devices/" + id + "/commands";
      out_topic = "devices/" + id + "/response";
      publish_topic = "devices/" + id + "/telemetry";

      // Build client ID
      client_id = id + "_" + randomId;

  }
  else {

      // Build topics IDs for MQTT
      in_topic = "devices/" + id + "/commands";
      out_topic = "devices/" + id + "/response";
      publish_topic = "devices/" + id + "/telemetry";

      Serial.print("Commands topic: ");
      Serial.println(in_topic);
      Serial.println("");

      Serial.print("Response topic: ");
      Serial.println(out_topic);

      // Build client ID
      client_id = id + "_" + String(millis() % 10000);

  }

  #endif

}

#if defined(__arm__)
String getChipId() {

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

#if defined(ESP8266) || defined(ESP32)
String gen_random(int length) {

  String randomString;

  #if defined(ESP8266)

    randomString = String(ESP.getChipId());
    randomString = randomString.substring(0, 6);
  #elif defined(ESP32)

    uint32_t chipId = 0;
    for(int i=0; i<17; i=i+8) {
	    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	  }

    randomString = String(chipId);
    randomString = randomString.substring(0, 6);

  #endif

  return randomString;
}
#endif

// Set device name
void set_name(const char* device_name){

  strcpy(name, device_name);
}

// Set device name
void set_name(const String& device_name){

  device_name.toCharArray(name, NAME_SIZE);
}

// Remove last char from buffer
void removeLastBufferChar() {

  index = index - 1;

}


void addQuote() {
  if(index < OUTPUT_BUFFER_SIZE) {
    buffer[index] = '"';
    index++;
  }
}


void addStringToBuffer(const char * toAdd, bool quotable){

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

template <typename T>
void addToBuffer(T toAdd, bool quotable=false) {
  addStringToBuffer(String(toAdd).c_str(), false);
}

// Register a function instead of a plain old variable!
template <typename T>
void addToBuffer(T(*toAdd)(), bool quotable=true) {
  addToBuffer(toAdd(), quotable);
}


template <typename T>
void sendBuffer(T& client, uint8_t chunkSize, uint8_t wait_time) {

  if (DEBUG_MODE) {
    #if defined(ESP8266)|| defined (ESP32)
    Serial.print("Memory loss before sending:");
    Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    freeMemory = ESP.getFreeHeap();
    #endif
    Serial.print(F("Buffer size: "));
    Serial.println(index);
  }

  // Send all of it
  if (chunkSize == 0) {
    client.print(buffer);
  }

  // Send chunk by chunk
  else {

    // Max iteration
    uint8_t max_iteration = (int)(index/chunkSize) + 1;

    // Send data
    for (uint8_t i = 0; i < max_iteration; i++) {
      char intermediate_buffer[chunkSize+1];
      memcpy(intermediate_buffer, buffer + i*chunkSize, chunkSize);
      intermediate_buffer[chunkSize] = '\0';

      // Send intermediate buffer
      #ifdef ADAFRUIT_CC3000_H
      client.fastrprint(intermediate_buffer);
      #else
      client.print(intermediate_buffer);
      #endif

      // Wait for client to get data
      delay(wait_time);

      if (DEBUG_MODE) {
        Serial.print(F("Sent buffer: "));
        Serial.println(intermediate_buffer);
      }
    }
  }

  if (DEBUG_MODE) {
    #if defined(ESP8266) || defined (ESP32)
    Serial.print("Memory loss after sending:");
    Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
    freeMemory = ESP.getFreeHeap();
    #endif
    Serial.print(F("Buffer size: "));
    Serial.println(index);
  }

    // Reset the buffer
    resetBuffer();

    if (DEBUG_MODE) {
      #if defined(ESP8266) || defined (ESP32)
      Serial.print("Memory loss after buffer reset:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
      #endif
      Serial.print(F("Buffer size: "));
      Serial.println(index);
    }
}

char * getBuffer() {
  return buffer;
}

void resetBuffer(){

  memset(&buffer[0], 0, sizeof(buffer));

}

uint8_t esp_12_pin_map(uint8_t pin) {

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


void addVariableToBuffer(uint8_t index) {
  addStringToBuffer(variable_names[index], true);
  addToBufferF(F(": "));
  variables[index]->addToBuffer(this);
}


void addHardwareToBuffer() {
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
char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
#endif

// Memory debug
#if defined(ESP8266) || defined(ESP32)
void initFreeMemory(){
  freeMemory = ESP.getFreeHeap();
}
#endif

#if defined(ESP8266) || defined(ESP32)
void setMQTTServer(const char* new_server){
  mqtt_server = new_server;
}

void setServer(const char* new_server){
  mqtt_server = new_server;
}

void setServer(const char* new_server, uint16_t new_port){
  mqtt_server = new_server;
  mqtt_port = new_port;
}
#endif

private:
  String answer;
  char command;
  uint8_t pin;
  uint8_t message_pin;
  char state;
  uint16_t value;
  boolean pin_selected;

  char* remote_server;
  int port;

  char name[NAME_SIZE];
  String id;
  String proKey;
  String arguments;

  // Output uffer
  char buffer[OUTPUT_BUFFER_SIZE];
  uint16_t index;

  // Status LED
  uint8_t status_led_pin;

  // Interval
  uint32_t previousMillis;
  uint32_t interval = 1000;

  // Int variables arrays
  uint8_t variables_index;
  Variable* variables[NUMBER_VARIABLES];
  const char * variable_names[NUMBER_VARIABLES];

  // Cloud connectivity
  #if defined(ESP8266) || defined(ESP32)

  // Topics for MQTT
  String in_topic;      // devices/{id}/commands
  String out_topic;     // devices/{id}/response
  String publish_topic; // devices/{id}/telemetry
  String client_id;

  // Subscribe topics & handlers
  uint8_t subscriptions_index;
  char * subscriptions_names[NUMBER_SUBSCRIPTIONS];

  // MQTT server - uses defaults from AREST_MQTT_SERVER/AREST_MQTT_PORT
  const char* mqtt_server = AREST_MQTT_SERVER;
  uint16_t mqtt_port = AREST_MQTT_PORT;
  MQTTClient* mqttClient = nullptr;

  #endif


  // Functions array
  uint8_t functions_index;
  int (*functions[NUMBER_FUNCTIONS])(String);
  const char* functions_names[NUMBER_FUNCTIONS];

  // Memory debug
  #if defined(ESP8266) || defined(ESP32)
  int freeMemory;
  #endif

};

#if defined(ESP8266) || defined(ESP32)
// MQTTClient method implementations
void MQTTClient::setServer(const char* host, uint16_t port) {
  mqttHost = String(host);
  mqttPort = port;
  mqttClient.setServer(host, port);
}

void MQTTClient::setStatusLED(int pin) {
  statusLedPin = pin;
  if (pin >= 0) {
    pinMode(statusLedPin, OUTPUT);
    digitalWrite(statusLedPin, LOW);
  }
}

void MQTTClient::setCallback(void (*callback)(char*, uint8_t*, unsigned int)) {
  messageCallback = callback;

  // Set up MQTT callback
  mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->handleMQTTMessage(topic, payload, length);
  });
}

bool MQTTClient::connect(const String& clientId) {
  // Build MQTT topics
  commandsTopic = "devices/" + deviceId + "/commands";
  responseTopic = "devices/" + deviceId + "/response";
  telemetryTopic = "devices/" + deviceId + "/telemetry";
  stateTopic = "devices/" + deviceId + "/state";

  Serial.println("[MQTT] === MQTT Connection Debug ===");
  Serial.print("[MQTT] Host: ");
  Serial.println(mqttHost);
  Serial.print("[MQTT] Port: ");
  Serial.println(mqttPort);
  Serial.print("[MQTT] Device ID: ");
  Serial.println(deviceId);
  Serial.print("[MQTT] API Key: ");
  Serial.println(apiKey.substring(0, 10) + "...");  // Show only first 10 chars for security
  Serial.print("[MQTT] Client ID: ");
  Serial.println(clientId);
  Serial.print("[MQTT] Commands Topic: ");
  Serial.println(commandsTopic);
  Serial.println("[MQTT] Attempting connection...");

  // Set server if not already set
  if (mqttHost.length() > 0) {
    mqttClient.setServer(mqttHost.c_str(), mqttPort);
  }

  // Connect with username (device_id) and password (api_key)
  isConnected = mqttClient.connect(clientId.c_str(), deviceId.c_str(), apiKey.c_str());

  if (isConnected) {
    lastPingTime = millis();
    Serial.println("[MQTT] Connected successfully!");

    // Subscribe to commands topic
    if (mqttClient.subscribe(commandsTopic.c_str())) {
      Serial.print("[MQTT] Subscribed to: ");
      Serial.println(commandsTopic);
    } else {
      Serial.println("[MQTT] Failed to subscribe to commands topic!");
    }
  } else {
    Serial.println("[MQTT] Connection failed!");
    Serial.print("[MQTT] State: ");
    Serial.println(mqttClient.state());
    Serial.println("[MQTT] Possible reasons:");
    Serial.println("[MQTT] - Broker not running or not accessible");
    Serial.println("[MQTT] - Wrong IP address or port");
    Serial.println("[MQTT] - Invalid credentials (device_id/api_key)");
    Serial.println("[MQTT] - Firewall blocking connection");
  }

  return isConnected;
}

bool MQTTClient::connected() {
  return mqttClient.connected();
}

void MQTTClient::loop() {
  if (mqttClient.connected()) {
    mqttClient.loop();
    isConnected = true;

    // Send ping/state periodically
    if (millis() - lastPingTime > pingInterval) {
      sendPing();
      lastPingTime = millis();
    }
  } else {
    isConnected = false;
  }

  // Update status LED
  updateStatusLED();
}

bool MQTTClient::publish(const String& topic, const char* payload) {
  if (!mqttClient.connected()) return false;
  return mqttClient.publish(topic.c_str(), payload);
}

bool MQTTClient::publishTelemetry(const char* payload) {
  return publish(telemetryTopic, payload);
}

bool MQTTClient::publishState(const char* payload) {
  return publish(stateTopic, payload);
}

bool MQTTClient::publishResponse(const char* payload) {
  return publish(responseTopic, payload);
}

bool MQTTClient::subscribe(const String& topic) {
  if (!mqttClient.connected()) return false;
  return mqttClient.subscribe(topic.c_str());
}

void MQTTClient::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  // Debug: Log raw message
  if (DEBUG_MODE) {
    Serial.println("[MQTT] === Received MQTT Message ===");
    Serial.print("[MQTT] Topic: ");
    Serial.println(topic);
  }

  // Convert payload to string
  char mqtt_msg[MQTT_MAX_PACKET_SIZE];
  for(unsigned int i = 0; i < length && i < MQTT_MAX_PACKET_SIZE - 1; i++) {
    mqtt_msg[i] = payload[i];
  }
  mqtt_msg[length < MQTT_MAX_PACKET_SIZE ? length : MQTT_MAX_PACKET_SIZE - 1] = '\0';

  if (DEBUG_MODE) {
    Serial.print("[MQTT] Payload: ");
    Serial.println(mqtt_msg);
  }

  // Parse JSON command
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, mqtt_msg);

  if (error) {
    if (DEBUG_MODE) {
      Serial.println("[MQTT] JSON parse error!");
      Serial.print("[MQTT] Error: ");
      Serial.println(error.c_str());
    }
    return;
  }

  // Extract command info - support multiple formats
  // Format 1: { "command_id": "...", "type": "function", "function": "blink", "params": {...} }
  // Format 2: { "command_id": "...", "command": "digital_write", "parameters": {...} }
  // Format 3: { "id": "...", "command_type": "...", "payload": {...} }

  const char* commandId = doc["command_id"] | doc["id"] | "";
  lastCommandId = String(commandId);

  // Determine command type
  const char* commandType = doc["type"] | doc["command"] | doc["command_type"] | "";

  if (DEBUG_MODE) {
    Serial.print("[MQTT] Command ID: ");
    Serial.println(commandId);
    Serial.print("[MQTT] Command type: ");
    Serial.println(commandType);
  }

  // Get parameters/payload
  JsonObject params;
  if (doc.containsKey("params")) {
    params = doc["params"];
  } else if (doc.containsKey("parameters")) {
    params = doc["parameters"];
  } else if (doc.containsKey("payload")) {
    params = doc["payload"];
  }

  // Process different command types
  if (strcmp(commandType, "digital_write") == 0 || strcmp(commandType, "pin") == 0) {
    int pin = params["pin"] | doc["pin"] | 0;

    // Handle value - can be int, string ("HIGH"/"LOW"), or bool
    int value = 0;
    if (params.containsKey("value")) {
      if (params["value"].is<const char*>()) {
        const char* valueStr = params["value"];
        value = (strcmp(valueStr, "HIGH") == 0 || strcmp(valueStr, "1") == 0) ? 1 : 0;
      } else {
        value = params["value"] | 0;
      }
    } else if (doc.containsKey("value")) {
      if (doc["value"].is<const char*>()) {
        const char* valueStr = doc["value"];
        value = (strcmp(valueStr, "HIGH") == 0 || strcmp(valueStr, "1") == 0) ? 1 : 0;
      } else {
        value = doc["value"] | 0;
      }
    }

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Digital write pin ");
      Serial.print(pin);
      Serial.print(" = ");
      Serial.println(value);
    }

    // Execute
    #if defined(ESP8266)
      analogWrite(pin, 0);
    #endif
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);

    sendCommandResponse(commandId, "success", "Pin D" + String(pin) + " set to " + String(value), value);
  }
  else if (strcmp(commandType, "digital_read") == 0) {
    int pin = params["pin"] | doc["pin"] | 0;

    pinMode(pin, INPUT);
    int value = digitalRead(pin);

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Digital read pin ");
      Serial.print(pin);
      Serial.print(" = ");
      Serial.println(value);
    }

    sendCommandResponse(commandId, "success", "Pin D" + String(pin) + " is " + String(value), value);
  }
  else if (strcmp(commandType, "analog_write") == 0) {
    int pin = params["pin"] | doc["pin"] | 0;
    int value = params["value"] | doc["value"] | 0;

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Analog write pin ");
      Serial.print(pin);
      Serial.print(" = ");
      Serial.println(value);
    }

    #if defined(ESP8266)
      analogWriteRange(255);
    #endif
    analogWrite(pin, value);

    sendCommandResponse(commandId, "success", "Pin A" + String(pin) + " set to " + String(value), value);
  }
  else if (strcmp(commandType, "analog_read") == 0) {
    int pin = params["pin"] | doc["pin"] | 0;

    int value = analogRead(pin);

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Analog read pin ");
      Serial.print(pin);
      Serial.print(" = ");
      Serial.println(value);
    }

    sendCommandResponse(commandId, "success", "Pin A" + String(pin) + " is " + String(value), value);
  }
  else if (strcmp(commandType, "function") == 0 || strcmp(commandType, "call_function") == 0) {
    const char* functionName = params["function"] | doc["function"] | params["name"] | "";
    const char* args = params["args"] | doc["args"] | "";

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Call function: ");
      Serial.print(functionName);
      Serial.print(" with args: ");
      Serial.println(args);
    }

    // Find and call function
    bool found = false;
    for (uint8_t i = 0; i < arest->functions_index; i++) {
      if (strcmp(arest->functions_names[i], functionName) == 0) {
        found = true;
        int result = arest->functions[i](String(args));

        // Build response with function result
        StaticJsonDocument<512> responseDoc;
        responseDoc["command_id"] = commandId;
        responseDoc["device_id"] = deviceId;
        responseDoc["success"] = true;
        responseDoc["result"]["executed"] = functionName;
        responseDoc["result"]["return_value"] = result;
        if (strlen(args) > 0) {
          responseDoc["result"]["args"] = args;
        }
        responseDoc["timestamp"] = String(millis());

        String response;
        serializeJson(responseDoc, response);
        publishResponse(response.c_str());
        break;
      }
    }

    if (!found) {
      sendCommandResponse(commandId, "error", "Function not found: " + String(functionName));
    }
  }
  else if (strcmp(commandType, "get_variable") == 0 || strcmp(commandType, "variable") == 0) {
    const char* varName = params["variable"] | doc["variable"] | params["name"] | "";

    if (DEBUG_MODE) {
      Serial.print("[MQTT] Get variable: ");
      Serial.println(varName);
    }

    // Find variable
    bool found = false;
    for (uint8_t i = 0; i < arest->variables_index; i++) {
      if (strcmp(arest->variable_names[i], varName) == 0) {
        found = true;

        // Get value based on type
        String value;
        if (arest->variables[i]->type == 'i') {
          value = String(*((int*)arest->variables[i]->ptr));
        } else if (arest->variables[i]->type == 'f') {
          value = String(*((float*)arest->variables[i]->ptr), 2);
        } else if (arest->variables[i]->type == 's') {
          value = String((char*)arest->variables[i]->ptr);
        }

        // Build response
        StaticJsonDocument<512> responseDoc;
        responseDoc["command_id"] = commandId;
        responseDoc["device_id"] = deviceId;
        responseDoc["success"] = true;
        responseDoc["result"]["variable"] = varName;
        responseDoc["result"]["value"] = value;
        responseDoc["timestamp"] = String(millis());

        String response;
        serializeJson(responseDoc, response);
        publishResponse(response.c_str());
        break;
      }
    }

    if (!found) {
      sendCommandResponse(commandId, "error", "Variable not found: " + String(varName));
    }
  }
  else if (strcmp(commandType, "get_info") == 0 || strcmp(commandType, "info") == 0) {
    if (DEBUG_MODE) {
      Serial.println("[MQTT] Get device info");
    }

    // Build info response
    StaticJsonDocument<1024> responseDoc;
    responseDoc["command_id"] = commandId;
    responseDoc["device_id"] = deviceId;
    responseDoc["success"] = true;
    responseDoc["result"]["id"] = arest->id;
    responseDoc["result"]["name"] = arest->name;
    responseDoc["result"]["hardware"] = HARDWARE;
    responseDoc["result"]["connected"] = true;

    // Add variables
    JsonObject vars = responseDoc["result"].createNestedObject("variables");
    for (uint8_t i = 0; i < arest->variables_index; i++) {
      const char* name = arest->variable_names[i];
      if (arest->variables[i]->type == 'i') {
        vars[name] = *((int*)arest->variables[i]->ptr);
      } else if (arest->variables[i]->type == 'f') {
        vars[name] = *((float*)arest->variables[i]->ptr);
      } else if (arest->variables[i]->type == 's') {
        vars[name] = (char*)arest->variables[i]->ptr;
      }
    }

    // Add function names
    JsonArray funcs = responseDoc["result"].createNestedArray("functions");
    for (uint8_t i = 0; i < arest->functions_index; i++) {
      funcs.add(arest->functions_names[i]);
    }

    responseDoc["timestamp"] = String(millis());

    String response;
    serializeJson(responseDoc, response);
    publishResponse(response.c_str());
  }
  else {
    // Unknown command type - try to pass to the callback for REST-style processing
    if (messageCallback) {
      messageCallback(topic, payload, length);
    } else if (DEBUG_MODE) {
      Serial.print("[MQTT] Unknown command type: ");
      Serial.println(commandType);
    }
  }
}

void MQTTClient::sendCommandResponse(const String& commandId, const String& status, const String& message) {
  StaticJsonDocument<512> doc;
  doc["command_id"] = commandId;
  doc["device_id"] = deviceId;
  doc["success"] = (status == "success");
  doc["result"]["message"] = message;
  doc["timestamp"] = String(millis());

  String response;
  serializeJson(doc, response);

  if (DEBUG_MODE) {
    Serial.print("[MQTT] Sending response: ");
    Serial.println(response);
  }

  publishResponse(response.c_str());
}

void MQTTClient::sendCommandResponse(const String& commandId, const String& status, const String& message, int value) {
  StaticJsonDocument<512> doc;
  doc["command_id"] = commandId;
  doc["device_id"] = deviceId;
  doc["success"] = (status == "success");
  doc["result"]["message"] = message;
  doc["result"]["return_value"] = value;
  doc["timestamp"] = String(millis());

  String response;
  serializeJson(doc, response);

  if (DEBUG_MODE) {
    Serial.print("[MQTT] Sending response: ");
    Serial.println(response);
  }

  publishResponse(response.c_str());
}

void MQTTClient::sendPing() {
  // Send state update as a ping/heartbeat
  StaticJsonDocument<256> doc;
  doc["connected"] = true;
  doc["uptime"] = millis();
  doc["hardware"] = HARDWARE;

  // Add WiFi signal strength if available
  #if defined(ESP8266) || defined(ESP32)
    doc["rssi"] = WiFi.RSSI();
  #endif

  String message;
  serializeJson(doc, message);
  publishState(message.c_str());
}

void MQTTClient::updateStatusLED() {
  if (statusLedPin < 0) return;  // LED disabled

  unsigned long now = millis();
  bool wifiConnected = false;

  #if defined(ESP8266)
    wifiConnected = (WiFi.status() == WL_CONNECTED);
  #elif defined(ESP32)
    wifiConnected = (WiFi.status() == WL_CONNECTED);
  #endif

  if (!wifiConnected) {
    // No WiFi - LED off
    if (now - lastLedUpdate >= 100) {
      lastLedUpdate = now;
      analogWrite(statusLedPin, 0);
    }
  }
  else if (wifiConnected && !isConnected) {
    // WiFi connected but no MQTT - fast blinking
    if (now - lastLedUpdate >= ledBlinkInterval) {
      lastLedUpdate = now;
      ledBrightness = (ledBrightness == 0) ? 255 : 0;
      analogWrite(statusLedPin, ledBrightness);
    }
  }
  else if (wifiConnected && isConnected) {
    // Both connected - slow pulsing
    if (now - lastLedUpdate >= ledPulseInterval) {
      lastLedUpdate = now;

      ledBrightness += ledDirection;
      if (ledBrightness >= 255) {
        ledBrightness = 255;
        ledDirection = -5;
      } else if (ledBrightness <= 0) {
        ledBrightness = 0;
        ledDirection = 5;
      }

      analogWrite(statusLedPin, ledBrightness);
    }
  }
}
#endif // ESP8266 || ESP32

// Some specializations of our template
template <>
void aREST::addToBuffer(bool toAdd, bool quotable) {
  addStringToBuffer(toAdd ? "true" : "false", false);   // Booleans aren't quoted in JSON
}


template <>
void aREST::addToBuffer(const char *toAdd, bool quotable) {
  addStringToBuffer(toAdd, quotable);                       // Strings must be quoted
}


template <>
void aREST::addToBuffer(const String *toAdd, bool quotable) {
  addStringToBuffer(toAdd->c_str(), quotable);           // Strings must be quoted
}


template <>
void aREST::addToBuffer(const String toAdd, bool quotable) {
  addStringToBuffer(toAdd.c_str(), quotable);           // Strings must be quoted
}


template <>
void aREST::addToBuffer(char toAdd[], bool quotable) {
  addStringToBuffer(toAdd, quotable);           // Strings must be quoted
}

#endif
