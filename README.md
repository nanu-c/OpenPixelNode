OpenPixelNode
=============

This project includes both hardware and software for an open-source LED controller for up to 4 ArtNet universes.

![OpenPixelNode](http://farm6.staticflickr.com/5523/12052788786_0aee2eae79_c.jpg)

More images here: http://flic.kr/s/aHsjQY2NrD

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

[Arduino_ws2812](https://github.com/media-architecture/Arduino_ws2812)

[WIZ_Ethernet_Library](https://github.com/media-architecture/WIZ_Ethernet_Library)

[TrueRandom](https://code.google.com/p/tinkerit/wiki/TrueRandom)


Version history
---------------

### Prototype 2 (January 2014)
### Prototype 1 (September 2013)
