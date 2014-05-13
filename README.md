# aREST

## Overview

A simple library that implements a REST API for Arduino. It is designed to be universal and currently supports REST calls via HTTP (using the CC3000 WiFi chip or the Ethernet shield) and via the Serial port (using the USB serial connection, Bluetooth, and XBee).

## Contents

- aREST.h: the library file.
- examples: two examples using the library, one for the CC3000 WiFi chip, one for Serial communications.
- gui: a simple graphical user interface to completely control an Arduino Uno board using the library. Works fine with the CC3000 chip, still have some issues with Serial communications. You can find the documentation in this README file.

## Supported hardware

The library is at the moment compatible with the following Arduino boards: Uno, Mega, Due, Teensy 3.0.

For the WiFi part, the library is compatible with most CC3000 breakout boards, and was tested with the Adafruit CC3000 breakout board and the CC3000 WiFi shield. It was also tested with the Tiny Circuit WiFi shield (but in that case, you will have to change the pins configuration inside the example WiFi sketch. See the Tiny Circuit WiFi shield documentation for more details).

For the Ethernet part, the library is compatible with the official Arduino Ethernet shield.

For the Serial part, it has been tested with the direct USB serial connection on an Arduino Uno board, with the Adafruit BlueFruit EZ-Link Bluetooth module, and with XBee Series 1 devices.

## Supported browsers

The project has been tested with Chrome 33, Safari 7 and Firefox 27.

## Requirements

- [Adafruit CC3000 Library](https://github.com/adafruit/Adafruit_CC3000_Library)
- [Adafruit MDNS Library](https://github.com/adafruit/CC3000_MDNS)
- MDNS support in your operating system:
  - For OS X it is supported through Bonjour, you don't have anything to install.
  - For Linux, you need to install [Avahi](http://avahi.org/).
  - For Windows, you need to install [Bonjour](http://www.apple.com/support/bonjour/).

## Setup

To install the library, simply clone this repository in the /libraries folder of your Arduino folder.

## Quick test (WiFi)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the WiFi_CC3000 example sketch and modify the WiFi SSID, password & security
3. Upload the sketch
4. Go to a web browser and type arduino.local/mode/8/o to set the pin as an output
5. Now type arduino.local/digital/8/1 and the LED should turn on

## Quick test (Ethernet)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Make sure your computer is connected via Ethernet to the board and has the IP address 192.168.2.x
3. Upload the sketch
4. Go to a web browser and type 192.168.2.2/mode/8/o to set the pin as an output
5. Now type 192.168.2.2/digital/8/1 and the LED should turn on

## Quick test (Serial)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the Serial example sketch
3. Upload the sketch
4. Go to a the Serial monitor and set the options to "Carriage return" and "115200 bauds"
5. Type /mode/8/o to set the pin as an output
6. Now type arduino.local/digital/8/1 and the LED should turn on

## API documentation

The API currently supports five type of commands: digital, analog, and mode, variables, and user-defined functions.

### Digital

Digital is to write or read on digital pins on the Arduino. For example:
  * /digital/8/0 sets pin number 8 to a low state
  * /digital/8/1 sets pin number 8 to a high state
  * /digital/8 reads value from pin number 8 in JSON format (note that for compatibility reasons, /digital/8/r produces the same result)

### Analog

Analog is to write or read on analog pins on the Arduino. Note that you can only write on PWM pins for the Arduino Uno, and only read analog values from analog pins 0 to 5. For example:
  * /analog/6/123 sets pin number 6 to 123 using PWM
  * /analog/0 returns analog value from pin number A0 in JSON format (note that for compatibility reasons, /analog/0/r produces the same result)

### Mode

Mode is to change the mode on a pin. For example:
  * /mode/8/o sets pin number 8 as an output
  * /mode/8/i sets pin number 8 as an input

### Variables

You can also directly call variables that are defined in your sketch. At the moment only integer variables are supported by the library. To access a variable in your sketch, you have to declare it first, and then call it from with a REST call. For example, if your aREST instance is called "rest" and the variable "temperature":
  * rest.variable("temperature",&temperature); declares the temperature in the Arduino sketch
  * /temperature returns the value of the variable in JSON format

### Functions

You can also define your own functions in your sketch that can be called using the REST API. To access a function defined in your sketch, you have to declare it first, and then call it from with a REST call. Note that all functions needs to take a String as the unique argument (for parameters to be passed to the function) and return an integer. For example, if your aREST instance is called "rest" and the function "ledControl":
  * rest.function("led",ledControl); declares the function in the Arduino sketch
  * /led?params=0 executes the function

### Authentification

You can define an API key to add more security to your projects, so only someone with the key can access to the devices using aREST. The authentification works only with HTTP requests (WiFi and Ethernet). You simply need to define a key in the Arduino sketch with:
 * rest.set_api_key("K9gs1wbodW");
 
Then, you need to send this key in the HTTP header of the request:
 * "X-ApiKey: K9gs1wbodW"

## GUI

The repository includes a web-based Graphical User Interface to control an Arduino Uno board from a web browser. To use it, simply put the gui folder at the root of a web server running on your computer, and open the HTML file. You will then be able to select the communication type (WiFi or Serial) and the target (for example, arduino.local if you are using the example WiFi sketch for your board).

## Troubleshooting

In case you cannot access your Arduino board via the mDNS service (by typing arduino.local in your browser), you need to get the IP address of the board. Upload the sketch to the Arduino board, and then open the Serial monitor. The IP address of the board should be printed out. Simply copy it on a web browser, and you can make REST call like:

192.168.1.104/digital/8/1
