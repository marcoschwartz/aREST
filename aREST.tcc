//TB:Redeclaration here?

template <typename T>
void aREST::addToBuffer(T toAdd, bool quotable=false) {
  addStringToBuffer(String(toAdd).c_str(), false);   // Except for our overrides, this will be adding numbers, which don't get quoted
}

// Register a function instead of a plain old variable!
template <typename T>
void aREST::addToBuffer(T(*toAdd)(), bool quotable=true) { 
  addToBuffer(toAdd(), quotable);
} 

template <typename T>
void aREST::sendBuffer(T& client, uint8_t chunkSize, uint8_t wait_time) {

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

template<typename T>
void aREST::variable(const char *name, T *var, bool quotable) { 
  variables[variables_index] = new TypedVariable<T>(var, quotable);
  variable_names[variables_index] = name;
  variables_index++;
}

template<typename T>
void aREST::variable(const char *name, T *var) { 
  variable(name, var, true);
}

#if defined(PubSubClient_h)

// Publish to cloud
template <typename T>
void aREST::publish(PubSubClient& client, const String& eventName, T data) {

  uint32_t currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    // Get event data
    if (DEBUG_MODE) {
      Serial.print("Publishing event " + eventName + " with data: ");
      Serial.println(data);
    }

    // Build message
    String message = "{\"client_id\": \"" + id + "\", \"event_name\": \"" + eventName + "\", \"data\": \"" + String(data) + "\"}";

    if (DEBUG_MODE) {
      Serial.print("Sending message via MQTT: ");
      Serial.println(message);
    }

    // Convert
    char charBuf[100];
    message.toCharArray(charBuf, 100);

    // Publish
    client.publish(publish_topic, charBuf);

  }

}

template <typename T>
void aREST::publish(PubSubClient& client, const String& eventName, T data, uint32_t customInterval) {

  uint32_t currentMillis = millis();

  if (currentMillis - previousMillis >= customInterval) {

    previousMillis = currentMillis;

    // Get event data
    if (DEBUG_MODE) {
      Serial.print("Publishing event " + eventName + " with data: ");
      Serial.println(data);
    }

    // Build message
    String message = "{\"client_id\": \"" + id + "\", \"event_name\": \"" + eventName + "\", \"data\": \"" + String(data) + "\"}";

    if (DEBUG_MODE) {
      Serial.print("Sending message via MQTT: ");
      Serial.println(message);
    }

    // Convert
    char charBuf[100];
    message.toCharArray(charBuf, 100);

    // Publish
    client.publish(publish_topic, charBuf);

  }

}

#endif

// Handle request with the Adafruit CC3000 WiFi library
#ifdef ADAFRUIT_CC3000_H

template <typename T>
void aREST::publish(Adafruit_CC3000_ClientRef& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request with the Arduino Yun
#elif defined(_YUN_CLIENT_H_)

template <typename T>
void aREST::publish(YunClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request with the Adafruit BLE board
#elif defined(_ADAFRUIT_BLE_UART_H_)

// template <typename T>
// void aREST::publish(Adafruit_BLE_UART& serial, const String& eventName, T value) {

//   // Publish request
//   publish_proto(client, eventName, value);

// }

// Handle request for the Arduino Ethernet shield
#elif defined(ethernet_h_)
template <typename T>
void aREST::publish(EthernetClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the ESP8266 chip
#elif defined(ESP8266) || defined (ESP32)

template <typename T>
void aREST::publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the Arduino MKR1000 board
#elif defined(WIFI_H)

template <typename T>
void aREST::publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

// Handle request for the Arduino WiFi shield
#elif defined(WiFi_h)

template <typename T>
void aREST::publish(WiFiClient& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

#elif defined(CORE_TEENSY)

template <typename T>
void aREST::publish(usb_serial_class& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}

#elif defined(__AVR_ATmega32U4__)

template <typename T>
void aREST::publish(Serial_& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}
#else

template <typename T>
void aREST::publish(HardwareSerial& client, const String& eventName, T value) {

  // Publish request
  publish_proto(client, eventName, value);

}
#endif

template <typename T, typename V>
void aREST::publish_proto(T& client, const String& eventName, V value) {

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
void aREST::handle_proto(T& serial, bool headers, uint8_t read_delay, bool decode)
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




/*

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

*/