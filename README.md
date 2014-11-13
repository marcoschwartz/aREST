# aREST

Version 1.9.4

## Overview

A simple library that implements a REST API for Arduino. It is designed to be universal and currently supports REST calls via HTTP (using the CC3000 WiFi chip, the Ethernet shield or the Arduino Yun), via the Serial port (using the USB serial connection, Bluetooth, and XBee) and also via Bluetooth Low Energy.

The library now requires the latest version of the Arduino IDE (1.5.7) to work correctly, as this version of the IDE comes with a lot of optimizations in the compiler.

If you want to know more about aREST, go over to [http://arest.io/](http://arest.io/).

## Contents

- aREST.h: the library file.
- examples: several examples using the aREST library
- test: Python-based unit tests of the library

## Supported hardware

### Arduino boards

The library is at the moment compatible with the following Arduino boards: Uno, Mega, Due, Yun and Teensy 3.x.

### HTTP

For HTTP communications, the library is compatible with most CC3000 breakout boards, and was tested with the Adafruit CC3000 breakout board and the CC3000 WiFi shield. It was also tested with the Tiny Circuit WiFi shield (but in that case, you will have to change the pins configuration inside the example WiFi sketch. See the Tiny Circuit WiFi shield documentation for more details). The library is also compatible with the official Arduino Ethernet shield, with the official Arduino WiFi shield, and with the Arduino Yun via the embedded WiFi connection.

### Serial

For Serial communications, the library has been tested with the direct USB serial connection on an Arduino Uno board, with the Adafruit BlueFruit EZ-Link Bluetooth module, and with XBee Series 1 devices.

### Bluetooth LE

For Bluetooth Low Energy communications, the library has been tested with the Adafruit BLE nRF8001 breakout board.

## Requirements

- [Arduino IDE 1.5.7 Beta](http://arduino.cc/en/Main/OldSoftwareReleases)

### For WiFi using the CC3000 chip

- [Adafruit CC3000 Library](https://github.com/adafruit/Adafruit_CC3000_Library)
- [Adafruit MDNS Library](https://github.com/adafruit/CC3000_MDNS)
- MDNS support in your operating system:
  - For OS X it is supported through Bonjour, you don't have anything to install.
  - For Linux, you need to install [Avahi](http://avahi.org/).
  - For Windows, you need to install [Bonjour](http://www.apple.com/support/bonjour/).

### For Bluetooth Low Energy

- [Adafruit nRF8001 Library](https://github.com/adafruit/Adafruit_nRF8001)

## Setup

To install the library, simply clone this repository in the /libraries folder of your Arduino folder.

## Quick test (WiFi)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the WiFi_CC3000 example sketch and modify the WiFi SSID, password & security
3. Upload the sketch
4. Go to a web browser and type `arduino.local/mode/8/o` to set the pin as an output
5. Now type `arduino.local/digital/8/1` and the LED should turn on

## Quick test (Ethernet)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Make sure your computer is connected via Ethernet to the board and has the IP address 192.168.2.x
3. Upload the sketch
4. Go to a web browser and type `192.168.2.2/mode/8/o` to set the pin as an output
5. Now type `192.168.2.2/digital/8/1` and the LED should turn on

## Quick test (Serial)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the Serial example sketch
3. Upload the sketch
4. Go to a the Serial monitor and set the options to "Carriage return" and "115200 bauds"
5. Type `/mode/8/o` to set the pin as an output
6. Now type `/digital/8/1` and the LED should turn on

## Quick test (BLE)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the BLE example sketch
3. Upload the sketch
4. Use the [BlueFruit LE Connect app](https://itunes.apple.com/fr/app/adafruit-bluefruit-le-connect/id830125974?mt=8) to connect to the BLE chip 
5. Type `/mode/8/o /` to set the pin as an output
6. Now type `/digital/8/1 /` and the LED should turn on

## API documentation

The API currently supports five type of commands: digital, analog, and mode, variables, and user-defined functions.

### Digital

Digital is to write or read on digital pins on the Arduino. For example:
  * `/digital/8/0` sets pin number 8 to a low state
  * `/digital/8/1` sets pin number 8 to a high state
  * `/digital/8` reads value from pin number 8 in JSON format (note that for compatibility reasons, `/digital/8/r` produces the same result)

### Analog

Analog is to write or read on analog pins on the Arduino. Note that you can only write on PWM pins for the Arduino Uno, and only read analog values from analog pins 0 to 5. For example:
  * `/analog/6/123` sets pin number 6 to 123 using PWM
  * `/analog/0` returns analog value from pin number A0 in JSON format (note that for compatibility reasons, `/analog/0/r` produces the same result)

### Mode

Mode is to change the mode on a pin. For example:
  * `/mode/8/o` sets pin number 8 as an output
  * `/mode/8/i` sets pin number 8 as an input

### Variables

You can also directly call variables that are defined in your sketch. At the moment only integer variables are supported by the library. To access a variable in your sketch, you have to declare it first, and then call it from with a REST call. For example, if your aREST instance is called "rest" and the variable "temperature":
  * `rest.variable("temperature",&temperature);` declares the temperature in the Arduino sketch
  * `/temperature` returns the value of the variable in JSON format

### Functions

You can also define your own functions in your sketch that can be called using the REST API. To access a function defined in your sketch, you have to declare it first, and then call it from with a REST call. Note that all functions needs to take a String as the unique argument (for parameters to be passed to the function) and return an integer. For example, if your aREST instance is called "rest" and the function "ledControl":
  * `rest.function("led",ledControl);` declares the function in the Arduino sketch
  * `/led?params=0` executes the function

### Get data about the board

You can also access a description of all the variables that were declared on the board with a single command. This is useful to automatically build graphical interfaces based on the variables exposed to the API. This can be done via the following calls:
  * `/` or `/id`
  * The names & types of the variables will then be stored in the variables field of the returned JSON object

### Status LED (BETA)

To know the activity of the library while the sketch is running, there is the possibility to connect a LED to a pin to show this activity in real-time. Simply connect a 220 Ohm resistor in series with a 5mm LED to the pin of your choice, and enter this line in the setup() function of your Arduino sketch:

```c
rest.set_status_led(led_pin);
```

### Lightweight mode (BETA)

There is the possibility to use a lightweight mode for aREST. This means that for commands to control the Arduino board (like digitalWrite commands), no data is returned at all. For commands that ask for data to be sent back (like asking for a variable), in this mode the library will only return the value of the data that was requested. 

This mode was made for cases where the memory footprint of the aREST library has to be as small as possible, or with devices that can't send/receive a lot of data at the same time, like Bluetooth LE. To enable this lightweight mode, simply start your sketch with:

```c
#define LIGHTWEIGHT 1
```

## Troubleshooting

In case you cannot access your Arduino board via the CC3000 mDNS service (by typing arduino.local in your browser), you need to get the IP address of the board. Upload the sketch to the Arduino board, and then open the Serial monitor. The IP address of the board should be printed out. Simply copy it on a web browser, and you can make REST call like:

```
192.168.1.104/digital/8/1
```