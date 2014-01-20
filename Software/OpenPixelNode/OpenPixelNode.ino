#include <Ethernet/utility/w5100.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>
#include <ws2812.h>
#include <TrueRandom.h>

// Config constants
#define EEPROM_MAGIC (byte*)"NODE"

#define CONFIG_MAGIC        0
#define CONFIG_MAC          4
#define CONFIG_IP          10
#define CONFIG_SUBNETMASK  14
#define CONFIG_PORT        18
#define CONFIG_DHCP        20
#define CONFIG_PORT1       22
#define CONFIG_PORT2       23
#define CONFIG_PORT3       24
#define CONFIG_PORT4       25
#define CONFIG_SHORTNAME   26
#define CONFIG_LONGNAME    44

// Art-Net class instance with 1024 bytes buffer
ArtNet artnet = ArtNet(1024);

// Defaults
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(172, 16, 0, 1);
IPAddress subnetmask(255, 255, 255, 0);
boolean useDhcp = false;
char *nodeName = "MAI Ethernet";

// LED control
WS2812 strip1 = WS2812(0);
WS2812 strip2 = WS2812(2);
WS2812 strip3 = WS2812(4);
WS2812 strip4 = WS2812(6);

void setup() {
  
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
 
  // Reset Wiznet w5200
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  delay(100);
  digitalWrite(8, HIGH);
  delay(500);
  digitalWrite(9, LOW);

  // Speed up SPI
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  // Read configuration
  if(configCheck(CONFIG_MAGIC, EEPROM_MAGIC, 4)) {
    
    configRead(CONFIG_MAC, mac, 6);
    ip = configReadInt32(CONFIG_IP);
    subnetmask = configReadInt32(CONFIG_SUBNETMASK);
    Ethernet.begin(mac, ip);

    configRead(CONFIG_SHORTNAME, artnet.getShortName(), 18);
    configRead(CONFIG_LONGNAME, artnet.getLongName(), 64);
    artnet.setPortAddress(0, configReadByte(CONFIG_PORT1));
    artnet.setPortAddress(1, configReadByte(CONFIG_PORT2));
    artnet.setPortAddress(2, configReadByte(CONFIG_PORT3));
    artnet.setPortAddress(3, configReadByte(CONFIG_PORT4));
  }
  // Initialize default configuration
  else {
    
    /*for(int i=0; i<10; i++) {
      digitalWrite(9, HIGH);delay(100);
      digitalWrite(9, LOW);delay(100);
    }*/
    configWrite(CONFIG_MAGIC, EEPROM_MAGIC, 4);
    TrueRandom.mac(mac);
    configWrite(CONFIG_MAC, mac, 6);
    configWriteInt32(CONFIG_IP, ip);
    configWriteInt32(CONFIG_SUBNETMASK, subnetmask);
    Ethernet.begin(mac, ip);

    configWrite(CONFIG_SHORTNAME, nodeName, strlen(nodeName)+1);
    artnet.setShortName(nodeName);
    configWrite(CONFIG_LONGNAME, nodeName, strlen(nodeName)+1);
    artnet.setLongName(nodeName);
    for(int i=0; i<4; i++) {
      configWriteByte(CONFIG_PORT1+i, i);
      artnet.setPortAddress(i, i);
    }
  }
  
  uint16_t sizes[8] = {(16<<10),0,0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);

  artnet.setNet(0);
  artnet.setSubNet(0);
  artnet.setNumPorts(4);
  artnet.setPortType(0, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.setPortType(1, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.setPortType(2, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.setPortType(3, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.begin(mac);
}

void loop() {
    
  int packetSize = artnet.parsePacket();
  if(packetSize) {
    
    digitalWrite(9, HIGH);
    
    switch(artnet.getOpCode()) {
      
      case ARTNET_OPCODE_POLL: {
        artnet.readPoll();
        artnet.sendPollReply();
      } break;
        
      case ARTNET_OPCODE_DMX: {
        artnet_dmx_header* header = artnet.readDmxHeader();
        byte port = artnet.getPortOutFromUni(header->subUni);
        switch(port) {
          case 0: strip1.write(artnet.readDmxData(header->length), header->length);break;
          case 1: strip2.write(artnet.readDmxData(header->length), header->length);break;
          case 2: strip3.write(artnet.readDmxData(header->length), header->length);break;
          case 3: strip4.write(artnet.readDmxData(header->length), header->length);break;
          default: artnet.flush();
        }
      } break;
      
      case ARTNET_OPCODE_IPPROG: {
        artnet_ip_prog* ipprog = artnet.readIpProg();
        
        if(ipprog->command == 0) {
          uint32_t ip = Ethernet.localIP();
          uint32_t sm = Ethernet.subnetMask();
          memcpy(ipprog->ip, &ip, 4);
          memcpy(ipprog->subnet, &sm, 4);
          ipprog->port = 0x1936;
          ipprog->status = useDhcp ? ARTNET_DHCP_ENABLED : 0;
          artnet.sendIpProgReply(ipprog);
        }
        else
        if(ipprog->command & ARTNET_IPCMD_IP) {
          artnet.stop();
          Ethernet.begin(mac, ipprog->ip);
          artnet.begin(mac);
          uint32_t i = Ethernet.localIP();
          uint32_t s = Ethernet.subnetMask();
          memcpy(ipprog->ip, &i, 4);
          memcpy(ipprog->subnet, &s, 4);
          configWriteInt32(CONFIG_IP, i);
          configWriteInt32(CONFIG_SUBNETMASK, s);
          
          artnet.sendIpProgReply(ipprog);
        }
      } break;
      
      case ARTNET_OPCODE_ADDRESS: {
        artnet_address* address = artnet.readAddress();
        artnet.setShortName(address->shortName);
        artnet.setLongName(address->longName);
        configWrite(CONFIG_SHORTNAME, address->shortName, 18);
        configWrite(CONFIG_LONGNAME, address->longName, 64);

        for(int i=0; i<4; i++) {
          artnet.setPortAddress(i, address->swOut[i]);
          configWriteByte(CONFIG_PORT1+i, address->swOut[i]);
        }
      } break;
      
      default:
        artnet.flush();
    }
    
    digitalWrite(9, LOW);
  }
}


