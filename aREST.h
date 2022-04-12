/*
  aREST Library for Arduino
  See the README file for more details.

  Written in 2014 by Marco Schwartz.

  This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License:
  http://creativecommons.org/licenses/by-sa/4.0/

  Version 2.8.1
  Changelog:

  Version 2.8.1: Updated examples with publish()
  Version 2.8.0: Compatibility with the new aREST cloud server
  Version 2.7.5: Added rate-limitation for publish()
  Version 2.7.4: Fix for the Arduino Ethernet 2.0 library
  Version 2.7.3: Added support to set your own ID when using API key
  Version 2.7.2: Bug fixes for aREST.io
  Version 2.7.1: Additional fixes & optimisations by @eykamp 
  Version 2.7.0: Several fixes & optimisations by @eykamp 
  Version 2.6.0: Added support for new aREST cloud app
  Version 2.5.0: Added support for the ESP32 WiFi chip (local & cloud)
  Version 2.4.2: Added publish() support for MKR1000
  Version 2.4.1: Additional fixes for Pro plans
  Version 2.4.0: Added support for aREST Pro & several fixes
  Version 2.3.1: Fixed pin mapping for NodeMCU/Wemos boards
  Version 2.3.0: Implement required changes for the cloud server upgrade
  Version 2.2.1: Added compatibility with the WINC1500 chip
  Version 2.2.0: Added compatibility with the Arduino MKR1000 board
  Version 2.1.2: Added data about hardware type in JSON answer
  Version 2.1.1: Fixed analogWrite() for ESP8266 chips
  Version 2.1.0: Added publish() function
  Version 2.0.2: Able to change MQTT remote server
  Version 2.0.2: Added cloud access support for the Ethernet library
  Version 2.0.1: Added beta support for cloud access via cloud.arest.io
  Version 2.0.0: Added beta support for MQTT communications
  Version 1.9.10: Added support for floats & Strings for Uno (without the CC3000 chip)
  Version 1.9.8: Added support for ESP8266 chip
  Version 1.9.7: Added support for Arduino 1.6.2
  Version 1.9.6: Added support for float variables for Arduino Mega
  Version 1.9.5: Added compatibility with Arduino IDE 1.5.8
  Version 1.9.4: Bug fixes & added support for configuring analog pints as digital outputs
  Version 1.9.3: Added description of available variables for the /id and / routes
  Version 1.9.2: Added compatibility with the Arduino WiFi library
  Version 1.9.1: Added compatibility with CORS
  Version 1.9: New speedup of the library (answers 2x faster in HTTP compared to version 1.8)
  Version 1.8: Speedup of the library (answers 2.5x faster with the CC3000 WiFi chip)
  Version 1.7.5: Reduced memory footprint of the library
  Version 1.7.4: Added a function to read all analog & digital inputs at once
  Version 1.7.3: Added LIGHTWEIGHT mode to only send limited data back
  Version 1.7.2: Added possibility to assign a status pin connected to a LED
  Version 1.7.1: Added possibility to change number of exposed variables & functions
  Version 1.7: Added compatibility with the Arduino Due & Teensy 3.x
  Version 1.6: Added compatibility with the Arduino Yun

  Version 1.5: Size reduction, and added compatibility with Adafruit BLE

  Version 1.4: Added authentification with API key

  Version 1.3: Added support for the Ethernet shield

  Version 1.2: Added support of Serial communications

  Version 1.1: Added variables & functions support

  Version 1.0: First working version of the library
*/

#ifndef aRest_h
#define aRest_h

// Include Arduino header
#include "Arduino.h"
//NEW:This line was added in order to make the linking work!
#include <Ethernet.h>

// MQTT packet size
#undef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512

// Using ESP8266 ?
#if defined(ESP8266) || defined(ESP32)
#include "stdlib_noniso.h"
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
#define ID_SIZE 10

// Subscriptions
#define NUMBER_SUBSCRIPTIONS 4

// Debug mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif


// Use AREST_PARAMS_MODE to control how parameters are parsed when using the function() method.
// Use 0 for standard operation, where everything before the first "=" is stripped before passing the parameter string to the function.
// Useful for simple functions, where only the value is important
//    function?params=hello    ==> hello gets passed to the function
//
// Use 1 to pass the entire parameter string to the function, which will be responsible for parsing the parameter string
// Useful for more complex situations, where the key name as well as its value is important, or there are mutliple key-value pairs
//    function?params=hello    ==> params=hello gets passed to the function
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


class aREST {

private:
struct Variable {
  virtual void addToBuffer(aREST *arest) const = 0;
};


template<typename T>
struct TypedVariable: Variable {
  T *var;
  bool quotable;

  TypedVariable(T *v, bool q) : var{v} { quotable = q; }

  void addToBuffer(aREST *arest) const override { 
    arest->addToBuffer(*var, quotable);
  }  
};

public:

public:

aREST();
aREST(char* rest_remote_server, int rest_port);


template<typename T>
void variable(const char *name, T *var, bool quotable);

template<typename T>
void variable(const char *name, T *var);


private:

void initialize();

// Used when resetting object back to oringial state
void reset();

public:


#if defined(PubSubClient_h)

// With default server
aREST(PubSubClient& client);

// With another server
aREST(PubSubClient& client, char* new_mqtt_server);

// Get topic
// String get_topic() {
//   return out_topic;
// }

// Subscribe to events
void subscribe(const String& device, const String& eventName);

// Publish to cloud
template <typename T>
void publish(PubSubClient& client, const String& eventName, T data);

template <typename T>
void publish(PubSubClient& client, const String& eventName, T data, uint32_t customInterval);

void setKey(char* api_key);

// void setKey(char* api_key, PubSubClient& client) {

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
void set_status_led(uint8_t pin);

#if !defined(ESP32)
// Glow status LED
void glow_led();
#endif

void addToBufferF(const __FlashStringHelper *toAdd);

// Send HTTP headers for Ethernet & WiFi
void send_http_headers();

// Reset variables after a request
void reset_status();


// Handle request with the Adafruit CC3000 WiFi library
#ifdef ADAFRUIT_CC3000_H
void handle(Adafruit_CC3000_ClientRef& client);

template <typename T>
void publish(Adafruit_CC3000_ClientRef& client, const String& eventName, T value);

// Handle request with the Arduino Yun
#elif defined(_YUN_CLIENT_H_)
void handle(YunClient& client);

template <typename T>
void publish(YunClient& client, const String& eventName, T value);


// Handle request with the Adafruit BLE board
#elif defined(_ADAFRUIT_BLE_UART_H_)
void handle(Adafruit_BLE_UART& serial);

// template <typename T>
// void publish(Adafruit_BLE_UART& serial, const String& eventName, T value);

// Handle request for the Arduino Ethernet shield
#elif defined(ethernet_h_)
void handle(EthernetClient& client);

template <typename T>
void publish(EthernetClient& client, const String& eventName, T value);

// Handle request for the Cytron Clone ESP8266
#elif defined(_CYTRONWIFISERVER_H_)
void handle(ESP8266Client& client);

// Handle request for the ESP8266 chip
#elif defined(ESP8266) || defined (ESP32)
void handle(WiFiClient& client);

// Handle request on the Serial port
void handle(HardwareSerial& serial);

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value);

// Handle request for the Arduino MKR1000 board
#elif defined(WIFI_H)
void handle(WiFiClient& client);

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value);

// Handle request for the Arduino WiFi shield
#elif defined(WiFi_h)
void handle(WiFiClient& client);

template <typename T>
void publish(WiFiClient& client, const String& eventName, T value);

#elif defined(CORE_TEENSY)
// Handle request on the Serial port
void handle(usb_serial_class& serial);

template <typename T>
void publish(usb_serial_class& client, const String& eventName, T value);

#elif defined(__AVR_ATmega32U4__)
// Handle request on the Serial port
void handle(Serial_& serial);

template <typename T>
void publish(Serial_& client, const String& eventName, T value);

#else
// Handle request on the Serial port
void handle(HardwareSerial& serial);

template <typename T>
void publish(HardwareSerial& client, const String& eventName, T value);
#endif

void handle(char * string);

void handle_proto(char * string);

template <typename T, typename V>
void publish_proto(T& client, const String& eventName, V value);

template <typename T>
void handle_proto(T& serial, bool headers, uint8_t read_delay, bool decode);

#if defined(PubSubClient_h)

// Process callback
void handle_callback(PubSubClient& client, char* topic, byte* payload, unsigned int length);

// Handle request on the Serial port
void loop(PubSubClient& client);

void handle(PubSubClient& client);

void reconnect(PubSubClient& client);
#endif

void process(char c);

// Modifies arguments in place
void urldecode(String &arguments);


bool send_command(bool headers, bool decodeArgs);


virtual void root_answer();


void function(char * function_name, int (*f)(String));

// Set device ID
void set_id(const String& device_id);

#if defined(__arm__)
String getChipId();
#endif

#if defined(PubSubClient_h)
String gen_random(int length);
#endif

// Set device name
void set_name(char *device_name);

// Set device name
void set_name(const String& device_name);

// Remove last char from buffer
void removeLastBufferChar();


void addQuote();


void addStringToBuffer(const char * toAdd, bool quotable);


// Add to output buffer

template <typename T>
void addToBuffer(T toAdd, bool quotable=false);

// Register a function instead of a plain old variable!
template <typename T>
void addToBuffer(T(*toAdd)(), bool quotable=true);


// // Add to output buffer
// void addToBuffer(const __FlashStringHelper *toAdd, bool quotable);

template <typename T>
void sendBuffer(T& client, uint8_t chunkSize, uint8_t wait_time);

char * getBuffer();

void resetBuffer();

uint8_t esp_12_pin_map(uint8_t pin);


void addVariableToBuffer(uint8_t index);


void addHardwareToBuffer();


// For non AVR boards
#if defined (__arm__)
char *dtostrf (double val, signed char width, unsigned char prec, char *sout);
#endif

// Memory debug
#if defined(ESP8266) || defined(ESP32)
void initFreeMemory();
#endif

#if defined(PubSubClient_h)
void setMQTTServer(char* new_mqtt_server);
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

  // MQTT client
  #if defined(PubSubClient_h)

  // Topics
  String in_topic;
  String out_topic;
  char publish_topic[ID_SIZE + 10];
  String client_id;

  // Subscribe topics & handlers
  uint8_t subscriptions_index;
  char * subscriptions_names[NUMBER_SUBSCRIPTIONS];

  // aREST.io server
  char* mqtt_server = "104.248.48.85";
  bool private_mqtt_server;

  #endif


  // Functions array
  uint8_t functions_index;
  int (*functions[NUMBER_FUNCTIONS])(String);
  char * functions_names[NUMBER_FUNCTIONS];

  // Memory debug
  #if defined(ESP8266) || defined(ESP32)
  int freeMemory;
  #endif

};


// // Some specializations of our template
// template <>
// void aREST::addToBuffer(bool toAdd, bool quotable);


// template <>
// void aREST::addToBuffer(const char *toAdd, bool quotable);


// template <>
// void aREST::addToBuffer(const String *toAdd, bool quotable);


// template <>
// void aREST::addToBuffer(const String toAdd, bool quotable);


// template <>
// void aREST::addToBuffer(char toAdd[], bool quotable);

#include <aREST.tcc>

#endif
