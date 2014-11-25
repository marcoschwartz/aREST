/* 
  aREST Library for Arduino
  See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 

  Version 1.9.4

  Changelog:

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

// Include Arduino header
#include "Arduino.h"

// Which board?
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define NUMBER_ANALOG_PINS 16
#define NUMBER_DIGITAL_PINS 54
#define OUTPUT_BUFFER_SIZE 600
#elif defined(__AVR_ATmega328P__)
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#define OUTPUT_BUFFER_SIZE 275
#else
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#define OUTPUT_BUFFER_SIZE 275
#endif

// Size of name & ID
#define NAME_SIZE 20
#define ID_SIZE 10

// Debug mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// Use light answer mode
#ifndef LIGHTWEIGHT
#define LIGHTWEIGHT 0
#endif

// Default number of max. exposed variables
#ifndef NUMBER_VARIABLES
#define NUMBER_VARIABLES 5
#endif

// Default number of max. exposed functions
#ifndef NUMBER_FUNCTIONS
#define NUMBER_FUNCTIONS 5
#endif

class aREST {

public:
  aREST() {
  command = 'u';
  pin_selected = false;
 
  status_led_pin = 255;
  state = 'u';
}

// Set status LED
void set_status_led(uint8_t pin){
  
  // Set variables
  status_led_pin = pin;
  
  // Set pin as output
  pinMode(status_led_pin,OUTPUT);
}

// Glow status LED
void glow_led() {

  if(status_led_pin != 255){
    unsigned long i = millis();
    int j = i % 4096;
    if (j > 2048) { j = 4096 - j;}
      analogWrite(status_led_pin,j/8);
    }
}

// Send HTTP headers for Ethernet & WiFi
void send_http_headers(){

  addToBuffer(F("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"));
}

// Reset variables after a request
void reset_status() {
  answer = "";
  command = 'u';
  pin_selected = false;
  state = 'u';
  arguments = "";

  index = 0;
  //memset(&buffer[0], 0, sizeof(buffer));

}

// Handle request with the Adafruit CC3000 WiFi library
#ifdef ADAFRUIT_CC3000_H
void handle(Adafruit_CC3000_ClientRef& client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,true,0);
        
    // Answer
    sendBuffer(client,32,100);
    client.stop();  

    // Reset variables for the next command
    reset_status();
  } 
}

// Handle request with the Arduino Yun
#elif defined(_YUN_CLIENT_H_) 
void handle(YunClient& client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,false,0);
    
    // Answer
    sendBuffer(client,25,50);
    client.stop();
   
    // Reset variables for the next command
    reset_status();
  } 
}

// Handle request with the Adafruit BLE board
#elif defined(_ADAFRUIT_BLE_UART_H_)
void handle(Adafruit_BLE_UART& serial) {
  
  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,0);
    
    // Answer
    sendBuffer(serial,100,5);

    // Reset variables for the next command
    reset_status();
  } 
}

// Handle request for the Arduino Ethernet shield
#elif defined(ethernet_h)
void handle(EthernetClient& client){

  if (client.available()) {

    // Handle request
    handle_proto(client,true,0);

    // Answer
    sendBuffer(client,50,5);
    client.stop();  
   
    // Reset variables for the next command
    reset_status();   
  }
}

// Handle request for the Arduino Ethernet shield
#elif defined(WiFi_h)
void handle(WiFiClient& client){

  if (client.available()) {

    // Handle request
    handle_proto(client,true,0);

    // Answer
    sendBuffer(client,50,5);
    client.stop();  
   
    // Reset variables for the next command
    reset_status();   
  }
}

#elif defined(CORE_TEENSY)
// Handle request on the Serial port
void handle(usb_serial_class& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1);

    // Answer
    sendBuffer(serial,25,10);

    // Reset variables for the next command
    reset_status();     
  }
}

#else
// Handle request on the Serial port
void handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false,1);

    // Answer
    sendBuffer(serial,25,10);

    // Reset variables for the next command
    reset_status();     
  }
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
  for (int i = 0; i < strlen(string); i++){

    char c = string[i];
    answer = answer + c;

    // Process data
    process(c);
    
  }

  // Send command
  send_command(false);
}

template <typename T>
void handle_proto(T& serial, bool headers, uint8_t read_delay) 
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
   send_command(headers);  
}

void process(char c){

  // Check if we are receveing useful data and process it
  if ((c == '/' || c == '\r') && state == 'u') {

      if (DEBUG_MODE) {Serial.println(answer);}

      // If the command is mode, and the pin is already selected    
      if (command == 'm' && pin_selected && state == 'u') {
        
        // Get state
        state = answer[0];
            
     }
     
     // If a digital command has been received, process the data accordingly     
     if (command == 'd' && pin_selected && state == 'u') {
                
       // If it's a read command, read from the pin and send data back
       if (answer[0] == 'r') {state = 'r';}
       
       // If not, get value we want to apply to the pin        
       else {value = answer.toInt(); state = 'w';}
     }
     
     // If analog command has been selected, process the data accordingly     
     if (command == 'a' && pin_selected && state == 'u') {
                
       // If it's a read, read from the correct pin
       if (answer[0] == 'r') {state = 'r';}
       
       // Else, write analog value        
       else {value = answer.toInt(); state = 'w';}
     }
     
     // If the command is already selected, get the pin     
     if (command != 'u' && pin_selected == false) {
       
       // Get pin
       if (answer[0] == 'A') {
         pin = 14 + answer[1] - '0';  
       }
       else {
         pin = answer.toInt();
       }
       if (DEBUG_MODE) {
        Serial.print("Selected pin: ");
        Serial.println(pin);
       }
       pin_selected = true;

       // Nothing more ?
       if ((answer[1] != '/' && answer[2] != '/') 
        || (answer[1] == ' ' && answer[2] == '/')
        || (answer[2] == ' ' && answer[3] == '/')) {
     
        // Nothing more & digital ?
        if (command == 'd') {

          // Read all digital ?
          if (answer[0] == 'a') {state = 'a';}

          // Save state & end there
          else {state = 'r';}
        }

       // Nothing more & analog ?
       if (command == 'a') {

         // Read all analog ?
         if (answer[0] == 'a') {state = 'a';}
        
         // Save state & end there
         else {state = 'r';}
       }
     }  

   }
     
     // Digital command received ?    
     if (answer.startsWith("digital")) {command = 'd';}
          
     // Mode command received ?
     if (answer.startsWith("mode")) {command = 'm';}
          
     // Analog command received ?
     if (answer.startsWith("analog")) {command = 'a';}

     // Variable or function request received ?
     if (command == 'u') {
       
       // Check if variable name is in array
       for (uint8_t i = 0; i < variables_index; i++){
         if(answer.startsWith(int_variables_names[i])) {
           
           // End here
           pin_selected = true;
           state = 'x';

           // Set state
           command = 'v';
           value = i;
         }
       }

       // Check if function name is in array
       for (uint8_t i = 0; i < functions_index; i++){
         if(answer.startsWith(functions_names[i])) {
           
           // End here
           pin_selected = true;
           state = 'x';

           // Set state
           command = 'f';
           value = i;

           // Get command
           uint8_t header_length = strlen(functions_names[i]) + 8;
           //strcpy(arguments, answer.substring(header_length).c_str());
           arguments = answer.substring(header_length);
         }
       }

       // If the command is "id", return device id, name and status
       if ( (answer[0] == 'i' && answer[1] == 'd') ){

           // Set state
           command = 'i';

           // End here
           pin_selected = true;
           state = 'x';
       }

       if (answer[0] == ' '){

           // Set state
           command = 'r';

           // End here
           pin_selected = true;
           state = 'x';
       }

     }

     answer = "";
     }
}

bool send_command(bool headers) {

   if (DEBUG_MODE) {
     Serial.println(F("Sending command"));
     Serial.print(F("Command: "));
     Serial.println(command);
     Serial.print(F("State: "));
     Serial.println(state);
     Serial.print(F("State of buffer at the start: "));
     Serial.println(buffer);
   }

   // Start of message
   if (headers) {send_http_headers();}

   // Mode selected
   if (command == 'm'){

     // Send feedback to client 
     if (!LIGHTWEIGHT){
       addToBuffer(F("{\"message\": \"Pin D"));
       addToBuffer(pin); 
     } 
     
     // Input
     if (state == 'i'){
      
      // Set pin to Input     
      pinMode(pin,INPUT);
          
      // Send feedback to client
      if (!LIGHTWEIGHT){addToBuffer(F(" set to input\", "));}
     }

     // Output
     if (state == 'o'){

       // Set to Output  
       pinMode(pin,OUTPUT);
          
       // Send feedback to client
       if (!LIGHTWEIGHT){addToBuffer(F(" set to output\", "));}
     }

   }

   // Digital selected
   if (command == 'd') {
     if (state == 'r'){

       // Read from pin
       value = digitalRead(pin);

       // Send answer
       if (LIGHTWEIGHT){addToBuffer(value);}
       else {
        addToBuffer(F("{\"return_value\": "));
        addToBuffer(value);
        addToBuffer(F(", "));
      }
     }
     
     #if !defined(__AVR_ATmega32U4__)
     if (state == 'a') {
       if (!LIGHTWEIGHT) {addToBuffer(F("{"));}
       
       for (uint8_t i = 0; i < NUMBER_DIGITAL_PINS; i++) {       
         
         // Read analog value
         value = digitalRead(i);
      
         // Send feedback to client
         if (LIGHTWEIGHT){
           addToBuffer(value);
           addToBuffer(F(","));
         }
         else {
           addToBuffer(F("\"D"));
           addToBuffer(i);
           addToBuffer(F("\": "));
           addToBuffer(value);
           addToBuffer(F(", "));
         } 
     }
    }
    #endif

     if (state == 'w') {

       // Apply on the pin      
       digitalWrite(pin,value);

       // Send feedback to client
       if (!LIGHTWEIGHT){
        addToBuffer(F("{\"message\": \"Pin D"));
        addToBuffer(pin);
        addToBuffer(F(" set to "));
        addToBuffer(value);
        addToBuffer(F("\", "));
       }
     }
   }

   // Analog selected
   if (command == 'a') {
     if (state == 'r'){
       
       // Read analog value
       value = analogRead(pin);
      
       // Send feedback to client
       if (LIGHTWEIGHT){addToBuffer(value);}
       else {
        addToBuffer(F("{\"return_value\": "));
        addToBuffer(value);
        addToBuffer(F(", "));
       }
     }
     #if !defined(__AVR_ATmega32U4__)
     if (state == 'a') {
       if (!LIGHTWEIGHT) {addToBuffer(F("{"));}
       
       for (uint8_t i = 0; i < NUMBER_ANALOG_PINS; i++) {       
         
         // Read analog value
         value = analogRead(i);
      
         // Send feedback to client
         if (LIGHTWEIGHT){
           addToBuffer(value);
           addToBuffer(F(","));
         }
         else {
           addToBuffer(F("\"A"));
           addToBuffer(i);
           addToBuffer(F("\": "));
           addToBuffer(value);
           addToBuffer(F(", "));
         } 
     }
   }
   #endif
   if (state == 'w') {

     // Write output value
     analogWrite(pin,value);

     // Send feedback to client
     addToBuffer(F("{\"message\": \"Pin D"));
     addToBuffer(pin);
     addToBuffer(F(" set to "));
     addToBuffer(value);
     addToBuffer(F("\", "));

   }
  }

  // Variable selected
  if (command == 'v') {          

       // Send feedback to client
       if (LIGHTWEIGHT){addToBuffer(*int_variables[value]);}
       else {
        addToBuffer(F("{\""));
        addToBuffer(int_variables_names[value]);
        addToBuffer(F("\": "));
        addToBuffer(*int_variables[value]);
        addToBuffer(F(", ")); 
       }
  }

  // Function selected
  if (command == 'f') {

    // Execute function
    uint8_t result = functions[value](arguments);

    // Send feedback to client
    if (!LIGHTWEIGHT) {
     addToBuffer(F("{\"return_value\": "));
     addToBuffer(result);
     addToBuffer(F(", "));
     //addToBuffer(F(", \"message\": \""));
     //addToBuffer(functions_names[value]);
     //addToBuffer(F(" executed\", "));
    }
  }

  if (command == 'r') {
    if (LIGHTWEIGHT) {addToBuffer(id);}
    else {
      addToBuffer(F("{\"variables\": {"));
      if (variables_index > 0){
        
        for (uint8_t i = 0; i < variables_index-1; i++){
          addToBuffer(F("\""));
          addToBuffer(int_variables_names[i]);
          addToBuffer(F("\": \"int32\", "));
        }
        addToBuffer(F("\""));
        addToBuffer(int_variables_names[variables_index-1]);
        addToBuffer(F("\": \"int32\"}, "));
      }
      else {
        addToBuffer(F(" }, "));
      }
      
    }
  }

  if (command == 'i') {
    if (LIGHTWEIGHT) {addToBuffer(id);}
    else {
      addToBuffer(F("{"));
    }
  }

   // End of message
   if (LIGHTWEIGHT){
     addToBuffer(F("\r\n"));
   }

   else {

     addToBuffer(F("\"id\": \""));
     addToBuffer(id);
     addToBuffer(F("\", \"name\": \""));
     addToBuffer(name);
     addToBuffer(F("\", \"connected\": true}\r\n"));
   }

   if (DEBUG_MODE) {
     Serial.print(F("State of buffer at the end: "));
     Serial.println(buffer);
   }
   
   // End here
   return true;
}

void variable(char * variable_name, int *variable){

  int_variables[variables_index] = variable;
  int_variables_names[variables_index] = variable_name;
  variables_index++;

}

void function(char * function_name, int (*f)(String)){

  functions_names[functions_index] = function_name;
  functions[functions_index] = f;
  functions_index++;
}

// Set device ID
void set_id(char *device_id){

  strcpy(id,device_id);
}

// Set device name
void set_name(char *device_name){
  
  strcpy(name, device_name);
}

// Set device name
void set_name(String device_name){

  device_name.toCharArray(name, NAME_SIZE);
}

// Set device ID
void set_id(String device_id){

  device_id.toCharArray(id, NAME_SIZE);
}

// Add to output buffer
void addToBuffer(char * toAdd){

  if (DEBUG_MODE) {
    Serial.print(F("Added to buffer: "));
    Serial.println(toAdd);
  }
  
  for (int i = 0; i < strlen(toAdd); i++){
    buffer[index+i] = toAdd[i];  
  }
  index = index + strlen(toAdd);
}

// Add to output buffer
void addToBuffer(int toAdd){

  char number[10];
  itoa(toAdd,number,10);
  
  addToBuffer(number);
}

// Add to output buffer
void addToBuffer(const __FlashStringHelper *toAdd){

  if (DEBUG_MODE) {
    Serial.print(F("Added to buffer: "));
    Serial.println(toAdd);
  }

  uint8_t idx = 0;

  const char PROGMEM *p = (const char PROGMEM *)toAdd;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    buffer[index + idx] = c;
    idx++;
  }
  index = index + idx;
}

template <typename T>
void sendBuffer(T& client, uint8_t chunkSize, uint8_t wait_time) {

  if (DEBUG_MODE) {
    Serial.print(F("Sending buffer: "));
    Serial.println(buffer);
  }  
  
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
  }
    
    // Wait for the client to get data
    delay(wait_time);
    resetBuffer();
}

char * getBuffer() {
  return buffer;
}

void resetBuffer(){
  memset(&buffer[0], 0, sizeof(buffer));
}
  
private:
  String answer;
  char command;
  uint8_t pin;
  char state;
  uint16_t value;
  boolean pin_selected;

  char name[NAME_SIZE];
  char id[ID_SIZE];
  String arguments;

  // Output uffer
  char buffer[OUTPUT_BUFFER_SIZE];
  uint16_t index;

  // Status LED
  uint8_t status_led_pin;

  // Variables arrays
  uint8_t variables_index;
  int * int_variables[NUMBER_VARIABLES];
  char * int_variables_names[NUMBER_VARIABLES];

  // Functions array
  uint8_t functions_index;
  int (*functions[NUMBER_FUNCTIONS])(String);
  char * functions_names[NUMBER_FUNCTIONS];
};