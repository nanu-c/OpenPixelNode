/*
 * OpenPixelNode - Version 0.21
 * by Media Architecture Institute
 *
 * Author: Tobias Ebsen
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 */

#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>
#include <TrueRandom.h>
#include <FastLED.h>

// Define pins for reset and status LED
#define LED_PIN  9
//#define RST_PIN  8 // Uncomment this to use reset

// Configuration
ArtNetConfig config = {
  {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, // MAC
  {192, 168, 1, 1},                     // IP
  {255, 255, 255, 0},                   // Subnet mask
  0x1936,                               // UDP port
  false,                                // DHCP
  0, 0,                                 // Net and subnet
  "OpenPixelNode",                      // Short name
  "OpenPixelNode by Media Architecture Institute", // Long name
  4,                                    // Number of ports
  {ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT}, // Port types
  {0, 1, 2, 3},                         // Input port addresses
  {0, 1, 2, 3},                         // Output port addresses
  0, 21                                 // Version
};

// Art-Net class instance with 1110 bytes buffer, will support 360 pixels per port
ArtNet artnet = ArtNet(config, 1110);

// LED control
static WS2811Controller800Khz<0, GRB> port1;
static WS2811Controller800Khz<2, GRB> port2;
static WS2811Controller800Khz<4, GRB> port3;
static WS2811Controller800Khz<6, GRB> port4;


void setup() {
   
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
  
  // Initialize FastLED
  port1.init();
  port2.init();
  port3.init();
  port4.init();
  
  // Setup ethernet
  if(config.dhcp)
    Ethernet.begin(config.mac);
  else
    Ethernet.begin(config.mac, config.ip);

  // Maximize socket memory size
  ethernetMaximize();

  // Begin parsing packets
  artnet.begin();
}

void loop() {

  // See if there is an ArtNet packet
  if(artnet.parsePacket()) {
        
    // Indicate activity
    digitalWrite(LED_PIN, HIGH);
    
    // Default handling of packets
    artnet.handleAny();
    
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
        byte cmd = artnet.getIpCommand();
        if(cmd & ARTNET_IPCMD_IP || cmd & ARTNET_IPCMD_SUBNET || cmd & ARTNET_IPCMD_DHCP)
          configWrite(config);
      } break;
      
      case ARTNET_OPCODE_ADDRESS: {
        configWrite(config);
      } break;
      
    }
    digitalWrite(LED_PIN, LOW);
  }
}

void ethernetInit() {

  // Reset Wiznet chip
#ifdef RST_PIN
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW); delay(100);
  digitalWrite(RST_PIN, HIGH); delay(500);
#endif

  // Speed up SPI
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}

void ethernetMaximize() {
  // Set memory sizes
#if defined(W5100_ETHERNET_SHIELD)
  uint16_t sizes[4] = {(8<<10),0,0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5200_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(16<<10),0,0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5500_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(16<<10),0,0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#endif
}

