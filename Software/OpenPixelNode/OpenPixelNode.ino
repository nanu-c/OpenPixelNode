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

#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>   // https://github.com/media-architecture/WIZ_Ethernet_Library
#include <ArtNet.h>     // https://github.com/media-architecture/Arduino_ArtNet/
#include <TrueRandom.h> // https://code.google.com/p/tinkerit/wiki/TrueRandom
#include <FastLED.h>    // https://github.com/FastLED/FastLED

#define LED_PIN  9

// Firmware version
#define OPN_VERSION_HIGH 0
#define OPN_VERSION_LOW  22

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

ArtNet artnet = ArtNet(config, 530);

// LED control
static WS2811Controller800Khz<0, GRB> port1;
static WS2811Controller800Khz<2, GRB> port2;
static WS2811Controller800Khz<4, GRB> port3;
static WS2811Controller800Khz<6, GRB> port4;

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  // Initialization and optimization
  ethernetInit();
  
  // Read config from EEPROM
  configGet(config);
  config.verHi = OPN_VERSION_HIGH;
  config.verLo = OPN_VERSION_LOW;

  // Initialize FastLED
  port1.init();
  port2.init();
  port3.init();
  port4.init();

  // Start ArtNet
  artnet.begin();
  
  ethernetMaximize();
  
  wdt_enable(WDTO_2S);

  digitalWrite(LED_PIN, LOW);
}

void loop() {
  
  if(artnet.parsePacket() > 0) {
    
    digitalWrite(LED_PIN, HIGH);

    switch(artnet.getOpCode()) {
      
      case ARTNET_OPCODE_DMX: {
          switch(artnet.getDmxPort()) {
            case 0: port1.show((CRGB*)artnet.getDmxData(), artnet.getDmxLength() / 3);break;
            case 1: port2.show((CRGB*)artnet.getDmxData(), artnet.getDmxLength() / 3);break;
            case 2: port3.show((CRGB*)artnet.getDmxData(), artnet.getDmxLength() / 3);break;
            case 3: port4.show((CRGB*)artnet.getDmxData(), artnet.getDmxLength() / 3);break;
          }
      } break;
      
      case ARTNET_OPCODE_IPPROG:
        artnet.handleIpProg();
        configWrite(config);
        break;
        
      case ARTNET_OPCODE_ADDRESS:
        artnet.handleAddress();
        configWrite(config);
        break;
      
      default:
        artnet.handleAny();
    }
    digitalWrite(LED_PIN, LOW);
  }
  // Check DHCP lease
  if(config.dhcp)
    artnet.maintain();
  
  wdt_reset();
}
