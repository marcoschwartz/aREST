# aREST

## Overview

A simple library that implements a REST API for Arduino. It is designed to be universal and currently supports REST calls via HTTP (using the CC3000 WiFi chip) and via the Serial port (using the USB serial connection, Bluetooth, and XBee).

## Supported hardware

The library is at the moment compatible with the following Arduino boards: Uno, Mega, Due, Teensy 3.0.

For the HTTP part, the library is compatible with most CC3000 breakout boards, and was tested with the Adafruit CC3000 breakout board and CC3000 WiFi shield.

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

## Quick test (HTTP)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the WiFi_CC3000 example sketch and modify the WiFi SSID, password & security
3. Upload the sketch
4. Go to a web browser and type arduino.local/mode/8/o to set the pin as an output
5. Now type arduino.local/digital/8/1 and the LED should turn on

## Quick test (Serial)

1. Connect a LED & resistor to pin number 8 of your Arduino board
2. Open the Serial example sketch
3. Upload the sketch
4. Go to a the Serial monitor and set the options to "Carriage return" and "115200 bauds"
5. Type /mode/8/o to set the pin as an output
6. Now type arduino.local/digital/8/1 and the LED should turn on

## API documentation

The API currently supports five type of commands: digital, analog, and mode, variables, and user-defined functions.

Digital is to write or read on digital pins on the Arduino. For example:
  * /digital/8/0 sets pin number 8 to a low state
  * /digital/8/1 sets pin number 8 to a high state
  * /digital/8/r reads value from pin number 8 in JSON format

Analog is to write or read on analog pins on the Arduino. Note that you can only write on PWM pins for the Arduino Uno, and only read analog values from analog pins 0 to 5. For example:
  * /analog/6/123 sets pin number 6 to 123 using PWM
  * /analog/0/r returns analog value from pin number A0 in JSON format

Mode is to change the mode on a pin. For example:
  * /mode/8/o sets pin number 8 as an output
  * /mode/8/i sets pin number 8 as an input

You can also directly call variables that are defined in your sketch. At the moment only integer variables are supported by the library. To access a variable in your sketch, you have to declare it first. For example, if your aREST instance is called "rest" and the variable "temperature":
  * rest.variable("temperature",&temperature);
You can then call the variable using a REST call:
  * /temperature returns the value of the variable in JSON format

## Troubleshooting

In case you cannot access your Arduino board via the mDNS service (by typing arduino.local in your browser), you need to get the IP address of the board. Upload the sketch to the Arduino board, and then open the Serial monitor. The IP address of the board should be printed out. Simply copy it on a web browser, and you can make REST call like:

192.168.1.104/digital/8/1
