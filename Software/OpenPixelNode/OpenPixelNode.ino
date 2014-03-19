/*
 * OpenPixelNode - Version 0.21
 * by Media Architecture Institute
 *
 * https://github.com/media-architecture/OpenPixelNode
 *
 * Author: Tobias Ebsen
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 */

#include <SPI.h>
#include <Ethernet.h>   // https://github.com/media-architecture/WIZ_Ethernet_Library
#include <ArtNet.h>     // https://github.com/media-architecture/Arduino_ArtNet/
#include <TrueRandom.h> // https://code.google.com/p/tinkerit/wiki/TrueRandom
#include <FastLED.h>    // https://github.com/FastLED/FastLED

// Firmware version
#define OPN_VERSION_HIGH 0
#define OPN_VERSION_LOW  21

// Define pins
#define PIN_PORT1_DATA  0
#define PIN_PORT1_CLK   1
#define PIN_PORT2_DATA  2
#define PIN_PORT2_CLK   3
#define PIN_PORT3_DATA  4
#define PIN_PORT3_CLK   5
#define PIN_PORT4_DATA  6
#define PIN_PORT4_CLK   7
#define PIN_RESET       8 // Uncomment this to use reset
#define PIN_STATUS_LED  9

// Default Configuration
ArtNetConfig config = {
  {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, // MAC
  {192, 168, 1, 1},                     // IP
  {255, 255, 255, 0},                   // Subnet mask
  0x1936,                               // UDP port
  true,                                 // DHCP
  0, 0,                                 // Net and subnet
  "OpenPixelNode",                      // Short name
  "OpenPixelNode by Media Architecture Institute", // Long name
  4,                                    // Number of ports
  {ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT}, // Port types
  {0, 1, 2, 3},                         // Input port addresses
  {0, 1, 2, 3}                          // Output port addresses
};

// Art-Net class instance with 1110 bytes buffer, will support 360 pixels per port
ArtNet artnet = ArtNet(config, 530);

// LED control
static WS2811Controller800Khz<PIN_PORT1_DATA, GRB> port1;
static WS2811Controller800Khz<PIN_PORT2_DATA, GRB> port2;
static WS2811Controller800Khz<PIN_PORT3_DATA, GRB> port3;
static WS2811Controller800Khz<PIN_PORT4_DATA, GRB> port4;


void setup(){

  pinMode(PIN_STATUS_LED, OUTPUT);

  // Initialize the ethernet interface
  ethernetInit();

  // Check if existing configuration is stored
  if(configCheckMagic()) {
    // Read configuration
    configRead(config);
  }
  // Initialize default configuration
  else {
    TrueRandom.mac(config.mac);
    configWriteMagic();
    configWrite(config);
  }
  
  // Set firmware version
  config.verHi = OPN_VERSION_HIGH;
  config.verLo = OPN_VERSION_LOW;
    
  // Setup ethernet
  if(config.dhcp) {
    digitalWrite(PIN_STATUS_LED, HIGH);
    Ethernet.begin(config.mac);
    digitalWrite(PIN_STATUS_LED, LOW);
  }
  else
    Ethernet.begin(config.mac, config.ip);

  // Maximize socket memory size
  ethernetMaximize();

  // Initialize FastLED
  port1.init();
  port2.init();
  port3.init();
  port4.init();

  // Begin parsing packets
  artnet.begin();
}

void loop() {

  // See if there is an ArtNet packet
  if(artnet.parsePacket()) {
        
    // Indicate activity
    digitalWrite(PIN_STATUS_LED, HIGH);
        
    // Do some handling on specific packets
    switch(artnet.getOpCode()) {
      
      case ARTNET_OPCODE_DMX: {
        byte port = artnet.getDmxPort();
        short nLeds = artnet.getDmxLength() / sizeof(CRGB);
        CRGB *data = (CRGB*)artnet.getDmxData();
        switch(port) {
          case 0: port1.show(data, nLeds);break;
          case 1: port2.show(data, nLeds);break;
          case 2: port3.show(data, nLeds);break;
          case 3: port4.show(data, nLeds);break;
        }
      } break;
      
      case ARTNET_OPCODE_IPPROG: {
        artnet.handleIpProg();
        configWrite(config);
      } break;
      
      case ARTNET_OPCODE_ADDRESS: {
        artnet.handleAddress();
        configWrite(config);
      } break;

      default:
        artnet.handleAny();
    }

    digitalWrite(PIN_STATUS_LED, LOW);
  }
}

void ethernetInit() {

  // Reset Wiznet chip
#ifdef PIN_RESET
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW); delay(100);
  digitalWrite(PIN_RESET, HIGH); delay(500);
#endif

}

void ethernetMaximize() {
  // Set memory sizes
#if defined(W5100_ETHERNET_SHIELD)
  uint16_t sizes[4] = {(4<<10),(4<<10),0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5200_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(8<<10),(8<<10),0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5500_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(8<<10),(8<<10),0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#endif

  // Speed up SPI
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}

