/* 
  aREST Library for Arduino
  See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 

  Version 1.7.5

  Changelog:
  Version 1.7.5: Reduce memory footprint of the library
  Version 1.7.4: Added a function to read all analog inputs at once
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
#elif defined(__AVR_ATmega328P__)
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
#else
#define NUMBER_ANALOG_PINS 6
#define NUMBER_DIGITAL_PINS 14
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
  state_selected = false;
  command_selected = false;
  api_key_received = false;
  api_key_match = false;

  status_led_set = false;

  variables_index = 0;
  functions_index = 0;
  //name = "default_name";
  //id = "001";
  api_key = "";
}

// Set status LED
void set_status_led(uint8_t pin){
  
  // Set variables
  status_led_set = true;
  status_led_pin = pin;
  
  // Set pin as output
  pinMode(status_led_pin,OUTPUT);
}

// Flash status LED
void flash_led(){

  if(status_led_set){
    digitalWrite(status_led_pin,HIGH);
    delay(10);
    digitalWrite(status_led_pin,LOW);  
  }
}

// Send HTTP headers for Ethernet & WiFi
template <typename T>
void send_http_headers(T& client){
  client.print(F("HTTP/1.1 200 OK"));
  client.print(F("\r\n"));
  client.print(F("Content-Type: application/json"));
  client.print(F("\r\n"));
  client.print(F("Connection: close"));
  client.print(F("\r\n"));
  client.print(F(""));
  client.print(F("\r\n"));  
}

// Reset variables after a request
void reset_status() {
  answer = "";
  command = 'u';
  command_selected = false;
  pin_selected = false;
  state_selected = false;
  command_sent = false;
  state = 'u';
  arguments = "";
  api_key_received = false;
  api_key_match = false;

}

// Handle request with the Adafruit CC3000 WiFi library
#ifdef ADAFRUIT_CC3000_H
void handle(Adafruit_CC3000_ClientRef& client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,true);
    
    // Give the web browser time to receive the data
    delay(5);
    client.close();  
   
    // Reset variables for the next command
    reset_status();
  } 
}
#endif

// Handle request with the SparkFun CC3000 WiFi library
#ifdef SFE_CC3000_H
void handle(SFE_CC3000_Client& client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,true);
    
    // Give the web browser time to receive the data
    delay(5);
    client.close();  
   
    // Reset variables for the next command
    reset_status();
  } 
}
#endif

// Handle request with the Arduino Yun
#ifdef _YUN_CLIENT_H_
void handle(YunClient& client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,true);
    
    // Give the web browser time to receive the data
    delay(5);
    client.stop();  
   
    // Reset variables for the next command
    reset_status();
  } 
}
#endif

// Handle request with the Adafruit BLE board
#ifdef _ADAFRUIT_BLE_UART_H_
void handle(Adafruit_BLE_UART& serial) {
  
  if (serial.available()) {

    // Handle request
    handle_proto(serial,false);
    
    // Wait for data to be sent
    delay(5);

    // Reset variables for the next command
    reset_status();
  } 
}
#endif

// Handle request for the Arduino Ethernet shield
#ifdef ethernet_h
void handle(EthernetClient& client){

  if (client.available()) {

    // Handle request
    handle_proto(client,true);

    // Give the web browser time to receive the data and stop connection
    delay(5);
    client.stop();  
   
    // Reset variables for the next command
    reset_status();   
  }
}
#endif

#if defined(CORE_TEENSY)
// Handle request on the Serial port
void handle(usb_serial_class& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false);

    // Wait for data to be sent
    delay(10);

    // Reset variables for the next command
    reset_status();     
  }
}
#endif

// Handle request on the Serial port
void handle(HardwareSerial& serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false);

    // Wait for data to be sent
    delay(10);

    // Reset variables for the next command
    reset_status();     
  }
}

template <typename T>
void handle_proto(T& serial, bool headers) 
{

  // Flash LED when request is received
  flash_led();

  // API key needed ?
  if (api_key == "") {api_key_received = true;}

  // Check if there is data available to read
  while (serial.available()) {
       
    // Get the server answer
    char c = serial.read();
    delay(1);
    answer = answer + c;
    //Serial.print(c);

    // Check for API key or end of request
    if (c == '\n') {

      // API key ?
      if(!api_key_received){

        // Check for API key
        if(answer.startsWith("X-ApiKey")) {
          //String received_key = answer.substring(10);
          char * received_key;
          strcpy(received_key, answer.substring(10).c_str());
          //received_key.trim();

          if (received_key == api_key) {
          api_key_match = true;
            //Serial.println("API key match");
          }

          //Serial.println("API key received");
          api_key_received = true;
          free(received_key);
        }

      }

      // Reset answer
      answer = "";

    }  

    // Check if we are receveing useful data and process it
    if ((c == '/' || c == '\r') && !state_selected) {

      // Trim answer
      //answer.trim();

      // Debug output
      // Serial.println(answer); 
      
      // If the command is mode, and the pin is already selected    
      if (command == 'm' && pin_selected && !state_selected) {
        
        // Get state
        state = answer[0];
       
        // Indicate that the state has been selected     
        state_selected = true;
            
     }
     
     // If a digital command has been received, process the data accordingly     
     if (command == 'd' && pin_selected && !state_selected) {
                
       //Serial.println("Digital command, finding nature");

       // If it's a read command, read from the pin and send data back
       if (answer[0] == 'r') {state = 'r';}
       
       // If not, get value we want to apply to the pin        
       else {value = answer.toInt(); state = 'w';}
       
       // Declare that the state has been selected         
       state_selected = true;
     }
     
     // If analog command has been selected, process the data accordingly     
     if (command == 'a' && pin_selected && !state_selected) {
                
       // If it's a read, read from the correct pin
       if (answer[0] == 'r') {state = 'r';}
       
       // Else, write analog value        
       else {value = answer.toInt(); state = 'w';}
       
       // Declare that the state has been selected        
       state_selected = true;
     }
     
     // If the command is already selected, get the pin     
     if (command_selected == true && pin_selected == false) {
       
       // Get pin
       pin = answer.toInt();
       // Serial.println("Pin " + String(pin) + " selected");
       pin_selected = true;

       // Nothing more ?
       if (answer.length() == String(pin).length() || answer[String(pin).length()] != '/') {

        // Nothing more & digital ?
        if (command == 'd') {

          // Read all digital ?
          if (answer[0] == 'a') {
            state = 'a';
            state_selected = true; 
          }

          else {
            // Save state & end there
            state = 'r';
            state_selected = true;
          }
        }

       // Nothing more & analog ?
       if (command == 'a') {

         // Read all analog ?
         if (answer[0] == 'a') {
           state = 'a';
           state_selected = true; 
         }
        
         // Save state & end there
         else {
          state = 'r';
          state_selected = true;
         }
       }
     }  

   }
     
     // Digital command received ?    
     if (answer.startsWith("digital")) {
       // Serial.println("Digital command received");
       command = 'd';
       command_selected = true;
     }
          
     // Mode command received ?
     if (answer.startsWith("mode")) {
       //Serial.println("Mode command received");
       command = 'm';
       command_selected = true;
     }
          
     // Analog command received ?
     if (answer.startsWith("analog")) {
       //Serial.println("Analog command received");
       command = 'a';
       command_selected = true;
     }

     // Variable or function request received ?
     if (command_selected == false) {
       
       // Check if variable name is in array
       for (uint8_t i = 0; i < variables_index; i++){
         if(answer.startsWith(int_variables_names[i])) {
           //Serial.println(F("Variable found")); 
           
           // End here
           command_selected = true;
           pin_selected = true;
           state_selected = true;

           // Set state
           command = 'v';
           value = i;
           
         }
       }

       // Check if function name is in array
       for (uint8_t i = 0; i < functions_index; i++){
         if(answer.startsWith(functions_names[i])) {
           //Serial.println(F("Function found"));
           
           // End here
           command_selected = true;
           pin_selected = true;
           state_selected = true;

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
       if (answer.startsWith("id")){
           //Serial.println(F("id command found"));

           // Set state
           command = 'i';

           // End here
           command_selected = true;
           pin_selected = true;
           state_selected = true;
       }
     }
     
     //Serial.println("Answer reset");  
     //Serial.print("Command selected:");   
     //Serial.println(command_selected);
     //Serial.print("Pin selected:");   
     //Serial.println(pin_selected);
     //Serial.print("State selected:");   
     //Serial.println(state_selected);   
     answer = "";
     
     }
     
     // Send commands
     if (command_selected && pin_selected && state_selected && !command_sent && api_key_received) {
       //Serial.println("Sending command: " + command + String(pin) + state);
        
       // Is the API key needed ?
       if (api_key == "" || api_key_match){

       // Start of message
       if (headers) {send_http_headers(serial);}

       // Mode selected
       if (command == 'm'){

         // Send feedback to client 
         if (!LIGHTWEIGHT){
           serial.print(F("{\"message\": \"Pin D"));
           serial.print(pin); 
         } 
         
         // Input
         if (state == 'i'){
          
          // Set pin to Input     
          pinMode(pin,INPUT);
              
          // Send feedback to client
          if (!LIGHTWEIGHT){serial.print(F(" set to input\", "));}
         }

         // Output
         if (state == 'o'){

           // Set to Output  
           pinMode(pin,OUTPUT);
              
           // Send feedback to client
           if (!LIGHTWEIGHT){serial.print(F(" set to output\", "));}
         }

       }

       // Digital selected
       if (command == 'd') {
         if (state == 'r'){

           // Read from pin
           value = digitalRead(pin);

           // Send answer
           if (LIGHTWEIGHT){serial.print(value);}
           else {
            serial.print(F("{\"return_value\": "));
            serial.print(value);
            serial.print(F(", "));
          }
         }
         if (state == 'a') {
           if (!LIGHTWEIGHT) {serial.print(F("{"));}
           
           for (uint8_t i = 0; i < NUMBER_DIGITAL_PINS; i++) {       
             
             // Read analog value
             value = digitalRead(i);
          
             // Send feedback to client
             if (LIGHTWEIGHT){
               serial.print(value);
               serial.print(F(","));
             }
             else {
               serial.print(F("\"D"));
               serial.print(i);
               serial.print(F("\": "));
               serial.print(value);
               serial.print(F(", "));
             } 
         }
        }
         if (state == 'w') {

           // Apply on the pin      
           digitalWrite(pin,value);

           // Send feedback to client
           if (!LIGHTWEIGHT){
            serial.print(F("{\"message\": \"Pin D"));
            serial.print(pin);
            serial.print(F(" set to "));
            serial.print(value);
            serial.print(F("\", "));
           }
         }
       }

       // Analog selected
       if (command == 'a') {
         if (state == 'r'){
           
           // Read analog value
           value = analogRead(pin);
          
           // Send feedback to client
           if (LIGHTWEIGHT){serial.print(value);}
           else {
            serial.print(F("{\"return_value\": "));
            serial.print(value);
            serial.print(F(", "));
           }
         }
         if (state == 'a') {
           if (!LIGHTWEIGHT) {serial.print(F("{"));}
           
           for (uint8_t i = 0; i < NUMBER_ANALOG_PINS; i++) {       
             
             // Read analog value
             value = analogRead(i);
          
             // Send feedback to client
             if (LIGHTWEIGHT){
               serial.print(value);
               serial.print(F(","));
             }
             else {
               serial.print(F("\"A"));
               serial.print(i);
               serial.print(F("\": "));
               serial.print(value);
               serial.print(F(", "));
             } 
         }
       }
       if (state == 'w') {

         // Write output value
         analogWrite(pin,value);
 
         // Send feedback to client
         serial.print(F("{\"message\": \"Pin D"));
         serial.print(pin);
         serial.print(F(" set to "));
         serial.print(value);
         serial.print(F("\", "));

       }
      }

      // Variable selected
      if (command == 'v') {          

           // Send feedback to client
           if (LIGHTWEIGHT){serial.print(*int_variables[value]);}
           else {
            serial.print(F("{\""));
            serial.print(int_variables_names[value]);
            serial.print(F("\": "));
            serial.print(*int_variables[value]);
            serial.print(F(", ")); 
           }
      }

      // Function selected
      if (command == 'f') {

        // Execute function
        uint8_t result = functions[value](arguments);

        // Send feedback to client
        if (!LIGHTWEIGHT) {
         serial.print(F("{\"return_value\": "));
         serial.print(result);
         serial.print(F(", \"message\": \""));
         serial.print(functions_names[value]);
         serial.print(F(" executed\", "));
        }
      }

      if (command == 'i') {
        if (LIGHTWEIGHT) {serial.print(id);}
        else {serial.print(F("{"));}
      }

       // End of message
       if (LIGHTWEIGHT){
         serial.print(F("\r\n"));
       }

       else {
         serial.print(F("\"id\": \""));
         serial.print(id);
         serial.print(F("\", \"name\": \""));
         serial.print(name);
         serial.print(F("\", ")); 
         serial.print(F("\"connected\": true}"));
         serial.print(F("\r\n"));
       }

       // End here
       command_sent = true;
      }
      else {

        // Send message
        if (headers) {send_http_headers(serial);}
        serial.print(F("{\"message\": \"API key invalid.\"}"));
        serial.print(F("\r\n"));

        // End here
        command_sent = true;
      }

     } 
       
   }
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

  id = device_id;
}

// Set device name
void set_name(char *device_name){
  
  name = device_name;
}

// Set API key
void set_api_key(char * the_api_key){
  
  api_key = the_api_key;
}
  
private:
  String answer;
  char command;
  uint8_t pin;
  char state;
  uint16_t value;
  boolean pin_selected;
  boolean state_selected;
  boolean command_selected;
  boolean command_sent;
  boolean api_key_received;
  boolean api_key_match;
  char *name;
  char *id;
  char * api_key;
  String arguments;

  // Status LED
  uint8_t status_led_pin;
  boolean status_led_set;

  // Variables arrays
  uint8_t variables_index;
  int * int_variables[NUMBER_VARIABLES];
  char * int_variables_names[NUMBER_VARIABLES];

  // Functions array
  uint8_t functions_index;
  int (*functions[NUMBER_FUNCTIONS])(String);
  char * functions_names[NUMBER_FUNCTIONS];
};