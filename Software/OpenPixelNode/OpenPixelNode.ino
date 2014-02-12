#include <Ethernet/utility/w5100.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>
#include <TrueRandom.h>
#include <FastLED.h>

#define RST_PIN  8
#define LED_PIN  9

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
char nodeName[] = "OpenPixelNode";

// LED control
static WS2811Controller800Khz<0, GRB> port1;
static WS2811Controller800Khz<2, GRB> port2;
static WS2811Controller800Khz<4, GRB> port3;
static WS2811Controller800Khz<6, GRB> port4;

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
 
  // Reset Wiznet w5200
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW); delay(100);
  digitalWrite(RST_PIN, HIGH); delay(500);
  
  digitalWrite(LED_PIN, LOW);

  // Speed up SPI
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  // Read configuration
  configInit();
  
  if(useDhcp)
    Ethernet.begin(mac);
  else
    Ethernet.begin(mac, ip);

  uint16_t sizes[8] = {(16<<10),0,0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);

  artnet.setNumPorts(4);
  artnet.begin(mac);
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
        switch(port) {
          case 0: port1.show((const CRGB*)artnet.getDmxData(), artnet.getDmxLength());break;
          case 1: port2.show((const CRGB*)artnet.getDmxData(), artnet.getDmxLength());break;
          case 2: port3.show((const CRGB*)artnet.getDmxData(), artnet.getDmxLength());break;
          case 3: port4.show((const CRGB*)artnet.getDmxData(), artnet.getDmxLength());break;
        }
      } break;
      
      case ARTNET_OPCODE_IPPROG: {
        byte cmd = artnet.getIpCommand();
        
        if(cmd & ARTNET_IPCMD_IP)
          configWriteInt32(CONFIG_IP, Ethernet.localIP());
        if(cmd & ARTNET_IPCMD_SUBNET)
          configWriteInt32(CONFIG_SUBNETMASK, Ethernet.subnetMask());
        if(cmd & ARTNET_IPCMD_DHCP)
          configWriteInt32(CONFIG_DHCP, Ethernet.maintain() != DHCP_CHECK_NONE);

      } break;
      
      case ARTNET_OPCODE_ADDRESS: {
        configWrite(CONFIG_SHORTNAME, artnet.getShortName(), 18);
        configWrite(CONFIG_LONGNAME, artnet.getLongName(), 64);

        for(int i=0; i<4; i++)
          configWriteByte(CONFIG_PORT1+i, artnet.getPortAddress(i));

      } break;
      
    }
    digitalWrite(LED_PIN, LOW);
  }
}

void configInit() {

  if(configCheck(CONFIG_MAGIC, EEPROM_MAGIC, 4)) {
    
    configRead(CONFIG_MAC, mac, 6);
    ip = configReadInt32(CONFIG_IP);
    subnetmask = configReadInt32(CONFIG_SUBNETMASK);
    useDhcp = configReadByte(CONFIG_DHCP);
    
    configRead(CONFIG_SHORTNAME, artnet.getShortName(), 18);
    configRead(CONFIG_LONGNAME, artnet.getLongName(), 64);
    artnet.setPortAddress(0, configReadByte(CONFIG_PORT1));
    artnet.setPortAddress(1, configReadByte(CONFIG_PORT2));
    artnet.setPortAddress(2, configReadByte(CONFIG_PORT3));
    artnet.setPortAddress(3, configReadByte(CONFIG_PORT4));
  }
  // Initialize default configuration
  else {
    
    configWrite(CONFIG_MAGIC, EEPROM_MAGIC, 4);
    TrueRandom.mac(mac);
    configWrite(CONFIG_MAC, mac, 6);
    configWriteInt32(CONFIG_IP, ip);
    configWriteInt32(CONFIG_SUBNETMASK, subnetmask);
    configWrite(CONFIG_DHCP, &useDhcp, 1);
    
    configWrite(CONFIG_SHORTNAME, nodeName, strlen(nodeName)+1);
    artnet.setShortName(nodeName);
    configWrite(CONFIG_LONGNAME, nodeName, strlen(nodeName)+1);
    artnet.setLongName(nodeName);
    for(int i=0; i<4; i++) {
      configWriteByte(CONFIG_PORT1+i, i);
      artnet.setPortAddress(i, i);
    }
  }
}
