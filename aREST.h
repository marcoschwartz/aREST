/* 
  aREST Library for Arduino (Uno/Mega/Due/Teensy).
  See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 
*/

#include "Arduino.h"

#define NUMBER_VARIABLES 2
#define NUMBER_FUNCTIONS 2

class aREST {

public:
  aREST() {
  command = "";
  pin_selected = false;
  state_selected = false;
  command_selected = false;
  variables_index = 0;
  functions_index = 0;
  //name = "default_name";
  //id = "001";
}

template <typename T>
void send_http_headers(T client){
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();  
}

void handle(Adafruit_CC3000_ClientRef client) {
  
  if (client.available()) {

    // Handle request
    handle_proto(client,true);
    
    // Give the web browser time to receive the data
    delay(5);

    client.close();  
   
    // Reset variables for the next command
    answer = "";
    command = "";
    command_selected = false;
    pin_selected = false;
    state_selected = false;    
  } 
}

void handle(HardwareSerial serial){

  if (serial.available()) {

    // Handle request
    handle_proto(serial,false);

    // Wait for data to be sent
    delay(5);

    // Reset variables for the next command
    answer = "";
    command = "";
    command_selected = false;
    pin_selected = false;
    state_selected = false;    
  }
}

template <typename T>
void handle_proto(T serial, bool headers) 
{ 
    // Check if there is data available to read
    while (serial.available()) {
       
    // Get the server answer
    char c = serial.read();
    delay(1);
    answer = answer + c;
    //Serial.print(c); 
        
    // Check if we are receveing useful data and process it
    if ((c == '/' || c == '\r') && state_selected == false) {

      //Serial.println(answer); 
      
      // If the command is mode, and the pin is already selected    
      if (command == "mode" && pin_selected == true && state_selected == false) {
    
        // Trim answer      
        answer.trim();

        // Input command received ?     
        if (answer.startsWith("i")) {
         
          // Set pin to Input     
          pinMode(pin,INPUT);
              
          // Send feedback to client
          if (headers) {send_http_headers(serial);}
          serial.print(F("Setting pin D"));
          serial.print(pin);
          serial.println(F(" to input"));
       }
       
       // Output command received ?     
       if (answer.startsWith("o")) {
         
         // Set to Output  
         pinMode(pin,OUTPUT);
              
         // Send feedback to client
         if (headers) {send_http_headers(serial);}
         serial.print(F("Setting pin D"));
         serial.print(pin);
         serial.println(F(" to output"));
       }
       
       // Indicate that the state has been selected     
       state_selected = true;
            
     }
     
     // If a digital command has been received, process the data accordingly     
     if (command == "digital" && pin_selected == true && state_selected == false) {
       
       // Trim answer     
       answer.trim();
                
       // If it's a read command, read from the pin and send data back
       if (answer.startsWith("r")) {
         
         // Read from pin
         value = digitalRead(pin);
          
         // Send feedback to client
         if (headers) {send_http_headers(serial);}
         serial.print(F("{\"return_value\": "));
         serial.print(value);
         serial.print(F(", \"id\": \""));
         serial.print(id);
         serial.print(F("\", \"name\": \""));
         serial.print(name);
         serial.println(F("\", \"connected\": true}"));
       }
               
       else {
         
         // Get value we want to apply to the pin        
         value = answer.toInt();
         //Serial.println("State " + answer + " set");
         
         // Apply on the pin      
         digitalWrite(pin,value);
 
         // Send feedback to client
         if (headers) {send_http_headers(serial);}
         serial.print(F("Pin D"));
         serial.print(pin);
         serial.print(F(" set to "));
         serial.println(value);
       }
       
       // Declare that the state has been selected         
       state_selected = true;
               
     }
     
     // If analog command has been selected, process the data accordingly     
     if (command == "analog" && pin_selected == true && state_selected == false) {
                
       // Trim answer
       answer.trim();
                
       // If it's a read, read from the correct pin
       if (answer.startsWith("r")) {
         
         // Read analog value
         value = analogRead(pin);
          
         // Send feedback to client
         if (headers) {send_http_headers(serial);}
         serial.print(F("{\"return_value\": "));
         serial.print(value);
         serial.print(F(", \"id\": \""));
         serial.print(id);
         serial.print(F("\", \"name\": \""));
         serial.print(name);
         serial.println(F("\", \"connected\": true}"));
       }
       
       // Else, write analog value        
       else {
         
         // Get value to apply to the output
         value = answer.toInt();
         //Serial.println("Value " + answer + " set");
               
         // Write output value
         analogWrite(pin,value);
 
         // Send feedback to client
         if (headers) {send_http_headers(serial);}
         serial.print(F("Pin D"));
         serial.print(pin);
         serial.print(F(" set to analog value "));
         serial.println(value);
       }
               
       state_selected = true;
               
     }
     
     // If the command is already selected, get the pin     
     if (command_selected == true && pin_selected == false) {
       
       // Get pin
       pin = answer.toInt();
       // Serial.println("Pin " + String(pin) + " selected");
       pin_selected = true;
     }
     
     // Digital command received ?    
     if (answer.startsWith("digital")) {
       //Serial.println("Digital command received");
       command = "digital";
       command_selected = true;
     }
          
     // Mode command received ?
     if (answer.startsWith("mode")) {
       //Serial.println("Mode command received");
       command = "mode";
       command_selected = true;
     }
          
     // Analog command received ?
     if (answer.startsWith("analog")) {
       //Serial.println("Analog command received");
       command = "analog";
       command_selected = true;
     }

     // Variable or function request received ?
     if (command_selected == false) {
       
       // Check if variable name is in array
       for (int i = 0; i < variables_index; i++){
         if(answer.startsWith(int_variables_names[i])) {
           // Serial.println(F("Variable found")); 
           command_selected = true;
           pin_selected = true;
           state_selected = true;
         
           // Send feedback to client
           if (headers) {send_http_headers(serial);}
           serial.print(F("{\""));
           serial.print(int_variables_names[i]);
           serial.print(F("\": "));
           serial.print(*int_variables[i]);
           serial.print(F(", \"id\": \""));
           serial.print(id);
           serial.print(F("\", \"name\": \""));
           serial.print(name);
           serial.println(F("\", \"connected\": true}"));
           
         }
       }

       // Check if function name is in array
       for (int i = 0; i < functions_index; i++){
         if(answer.startsWith(functions_names[i])) {
           //Serial.println(F("Function found"));
           command_selected = true;
           pin_selected = true;
           state_selected = true;

           // Get command
           int header_length = functions_names[i].length() + 8;
           String command = answer.substring(header_length);

           // Execute function
           functions[i](command);
         
           // Send feedback to client
           if (headers) {send_http_headers(serial);}
           serial.print(F("Function "));
           serial.print(functions_names[i]);
           serial.println(F(" has been executed"));
         }
       }

       // If the command is "id", return device id, name and status
       if (answer.startsWith("id")){
           //Serial.println(F("id command found"));
           if (headers) {send_http_headers(serial);}
           serial.print(F("{\"id\": \""));
           serial.print(id);
           serial.print(F("\", \"name\": \""));
           serial.print(name);
           serial.println(F("\", \"connected\": true}")); 
           command_selected = true;
           pin_selected = true;
           state_selected = true;
       }
     }
     
     //Serial.println("Answer reset");     
     answer = "";
     
     }
       
   }
}

void variable(String variable_name, int *variable){

  int_variables[variables_index] = variable;
  int_variables_names[variables_index] = variable_name;
  variables_index++;

}
  void function(String function_name, int (*f)(String)){

  functions_names[functions_index] = function_name;
  functions[functions_index] = f;
  functions_index++;
}
  void set_id(String device_id){
  id = device_id;
}
  void set_name(String device_name){
  name = device_name;
}
  
private:
  String answer;
  String command;
  uint8_t pin;
  int value;
  boolean pin_selected;
  boolean state_selected;
  boolean command_selected;
  String name;
  String id;

  // Variables arrays
  uint8_t variables_index;
  int * int_variables[NUMBER_VARIABLES];
  String int_variables_names[NUMBER_VARIABLES];

  // Functions array
  uint8_t functions_index;
  int (*functions[NUMBER_FUNCTIONS])(String);
  String functions_names[NUMBER_FUNCTIONS];
};