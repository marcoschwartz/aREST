<h1>aREST</h1>

<span>[![Build Status](https://travis-ci.org/marcoschwartz/aREST.svg?branch=master)](https://travis-ci.org/marcoschwartz/aREST)</span>
<span>[![Backers on Open Collective](https://opencollective.com/arest/backers/badge.svg)](#backers) [![Sponsors on Open Collective](https://opencollective.com/arest/sponsors/badge.svg)](#sponsors) [![Donate](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=3Q73345CWMYE8)</span>

## Overview

A simple library that implements a REST API for Arduino & the ESP8266 WiFi chip.

It is designed to be universal and currently supports REST calls via HTTP (using the CC3000 WiFi chip, the Arduino WiFi library or the Ethernet shield), via the Serial port (using the USB serial connection, Bluetooth, and XBee) and also via Bluetooth Low Energy. The library is also compatible with the Arduino MKR1000 board.

It also works with the ESP8266 WiFi chip using the ESP8266 processor, therefore working as an independent unit. It also works on the ESP32 WiFi chip.

Boards running aREST can also be accessed from anywhere in the world via an API available at `cloud.arest.io`. Check the rest of this file and the examples ending with *_cloud* for more details. This currently only works with the Ethernet library for Arduino & the ESP8266 WiFi chip.

If you want to know more about aREST, go over to [http://arest.io/](http://arest.io/).

## Contents

- aREST.h: the library file.
- examples: several examples using the aREST library
- test: unit tests of the library

## Supported hardware

### Arduino/Genuino Boards

The library is at the moment compatible with the following Arduino boards: Uno, Mega, Due, Yun and Teensy 3.x. It is also compatible with the Arduino/Genuino MKR1000 board.

### ESP8266/ESP32

The library is compatible with most of the ESP8266 modules & ESP8266 development boards, as well as most boards based on the ESP32 WiFi chip.

### HTTP

For HTTP communications, the library is compatible with most CC3000 breakout boards, and was tested with the Adafruit CC3000 breakout board and the CC3000 WiFi shield. It was also tested with the Tiny Circuit WiFi shield (but in that case, you will have to change the pins configuration inside the example WiFi sketch. See the Tiny Circuit WiFi shield documentation for more details). The library is also compatible with the official Arduino Ethernet shield, with the official Arduino WiFi shield, and with the Arduino Yun via the embedded WiFi connection.

### Serial

For Serial communications, the library has been tested with the direct USB serial connection on an Arduino Uno board, with the Adafruit BlueFruit EZ-Link Bluetooth module, and with XBee Series 1 devices.

### Bluetooth LE

For Bluetooth Low Energy communications, the library has been tested with the Adafruit BLE nRF8001 breakout board.

## Requirements

To use the library with Arduino boards you will need the latest version of the Arduino IDE:

- [Arduino IDE 1.6.8](http://arduino.cc/en/main/software)

### For WiFi using the ESP8266 chip

To use the library with the ESP8266 WiFi chip you will need to install the required module from the Boards Manager of the Arduino IDE. These are the steps to install the ESP8266 package inside the Arduino IDE:

1. Start the Arduino IDE and open the Preferences window
2. Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into the Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
3. Open the Boards Manager from Tools > Board menu and install the esp8266 package (and after that don't forget to select your ESP8266 board from Tools > Board menu).

### For WiFi using the ESP32 chip

To use the library with the ESP32 WiFi chip you will need to install the board definitions for the Arduino IDE. These are the steps to install support for the ESP32 chip inside the Arduino IDE:

1. Follow the install instructions from [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)
2. Restart the Arduino IDE
3. Select your ESP32 board from Tools > Board menu.

### For WiFi using the CC3000 chip

- [Adafruit CC3000 Library](https://github.com/adafruit/Adafruit_CC3000_Library)
- [Adafruit MDNS Library](https://github.com/adafruit/CC3000_MDNS)
- MDNS support in your operating system:
  - For OS X it is supported through Bonjour, you don't have anything to install.
  - For Linux, you need to install [Avahi](http://avahi.org/).
  - For Windows, you need to install [Bonjour](http://www.apple.com/support/bonjour/).

### For WiFi using the MKR1000 Board

To use aREST with the MKR1000 board, you first need to install the MKR1000 board definition from the Arduino IDE board manager. You also need to install the following library:

- [WiFi 101 Library](https://github.com/arduino-libraries/WiFi101)

### For Bluetooth Low Energy

- [Adafruit nRF8001 Library](https://github.com/adafruit/Adafruit_nRF8001)

### For Cloud Access

- [PubSub Library](https://github.com/knolleary/pubsubclient)

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
4. Go to a the Serial monitor and set the options to "BOTH NL & CR" and "115200 bauds"
5. Type `/mode/8/o` to set the pin as an output
6. Now type `/digital/8/1` and the LED should turn on

## Quick test (BLE)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the BLE example sketch
3. Upload the sketch
4. Use the [BlueFruit LE Connect app](https://itunes.apple.com/fr/app/adafruit-bluefruit-le-connect/id830125974?mt=8) to connect to the BLE chip
5. Type `/mode/8/o /` to set the pin as an output
6. Now type `/digital/8/1 /` and the LED should turn on

## Quick test (ESP8266/ESP32)

1. Connect a LED & resistor to pin number 5 of your ESP8266/ESP32 board
2. Open the ESP8266/ESP32 example sketch and modify the WiFi SSID & password
3. Upload the sketch
4. Open the Serial monitor to get the IP address of the board, for example 192.168.1.103
5. Go to a web browser and type `192.168.1.103/mode/5/o` to set the pin as an output
6. Now type `192.168.1.103/digital/5/1` and the LED should turn on

## Cloud Access (Ethernet)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the Ethernet_cloud example sketch and modify the MAC address, and also give a unique ID to your project, for example 47fd9g
3. Make sure your shield is connected to the web via an Ethernet cable
3. Upload the sketch to the board
5. Go to a web browser and type `cloud.arest.io/47fd9g/mode/8/o` to set the pin as an output
6. Now type `cloud.arest.io/47fd9g/digital/8/1` and the LED should turn on

## Cloud Access (ESP8266)

1. Connect a LED & resistor to pin number 5 of your ESP8266 board
2. Open the ESP8266_cloud example sketch and modify the WiFi SSID & password, and also give a unique ID to your project, for example 47fd9g
3. Upload the sketch to the board
5. Go to a web browser and type `cloud.arest.io/47fd9g/mode/5/o` to set the pin as an output
6. Now type `cloud.arest.io/47fd9g/digital/5/1` and the LED should turn on

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

You can also directly call variables that are defined in your sketch. Integer variables are supported by the library. Float and String variables are also supported, but only by the Arduino Mega board & by the ESP8266.

To access a variable in your sketch, you have to declare it first, and then call it from with a REST call. For example, if your aREST instance is called "rest" and the variable "temperature":
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

## Contributors

This project exists thanks to all the people who contribute. [[Contribute](CONTRIBUTING.md)].
<a href="graphs/contributors"><img src="https://opencollective.com/arest/contributors.svg?width=890" /></a>


## Backers

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/arest#backer)]

<a href="https://opencollective.com/arest#backers" target="_blank"><img src="https://opencollective.com/arest/backers.svg?width=890"></a>


## Sponsors

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/arest#sponsor)]

<a href="https://opencollective.com/arest/sponsor/0/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/0/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/1/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/1/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/2/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/2/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/3/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/3/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/4/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/4/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/5/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/5/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/6/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/6/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/7/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/7/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/8/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/8/avatar.svg"></a>
<a href="https://opencollective.com/arest/sponsor/9/website" target="_blank"><img src="https://opencollective.com/arest/sponsor/9/avatar.svg"></a>

