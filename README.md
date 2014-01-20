OpenPixelNode
=============

This project includes both hardware and software for an open-source LED controller for up to 4 ArtNet universes.

![OpenPixelNode](http://farm3.staticflickr.com/2849/12051620225_271beee82c_c.jpg)

Hardware
--------
The hardware design is Arduino-compatible in terms of being based on an ATmega328 microcontroller running at 16MHz.
It uses the Wiznet Wiz820io addon board for Ethernet communication. There are voltage regulators for both 5V and 3.3V for the two logic circuits. This allows the board to be supplied with voltages of 5V to 30V.
There are 4 LED outputs with two digital ports each. This allows the board to control either SPI or single-wire LED strips.
At the moment there is only ICSP headers for programming.

Software
--------
The software is an Arduino sketch using a library for the ArtNet protocol and whatever LED communication protocol you may want to use. At the moment there is only support for WS2812.


Dependencies
------------
[Arduino_ArtNet](https://github.com/media-architecture/Arduino_ArtNet)


Version history
---------------

### Prototype 2 (January 2014)
### Prototype 1 (September 2013)
