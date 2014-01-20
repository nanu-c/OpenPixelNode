OpenPixelNode
=============

LED controller with Ethernet interface. This project includes both hardware and software for an open-source LED controller for up to 4 ArtNet universes.

Hardware
--------
The hardware design is Arduino-compatible in terms of being based on an ATmega328 microcontroller running at 16MHz.
It uses the Wiznet Wiz820io addon board for Ethernet communication. There are voltage regulators for both 5V and 3.3V for the two logic circuits. This allows the board to be supplied with voltages of 5V to 30V.
There are 4 LED outputs with two digital ports each. This allows the board to control either SPI or single-wire LED strips.
At the moment there is only ICSP headers for programming.

Software
--------
Software is based on the ArtNet protocol and whatever LED communication protocol you may want to use. At the moment there is only support for WS2812.


Dependencies
------------

Version history
---------------

### Prototype 1 (September 2013)
