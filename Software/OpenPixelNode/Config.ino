#include <EEPROM.h>

boolean configCheck(int address, byte *data, int len) {
  for(int i=0; i<len; i++) {
    if(EEPROM.read(address+i) != data[i])
      return false;
  }
  return true;
}

void configRead(int address, byte *data, int len) {
  for(int i=0; i<len; i++)
    data[i] = EEPROM.read(address+i);
}

void configRead(int address, char *data, int len) {
  configRead(address, (byte*)data, len);
}

uint32_t configReadInt32(int address) {
  uint32_t data;
  configRead(address, (byte*)&data, 4);
  return data;
}

uint32_t configReadByte(int address) {
  byte data;
  configRead(address, (byte*)&data, 1);
  return data;
}

void configWrite(int address, byte *data, int len) {
  for(int i=0; i<len; i++)
    EEPROM.write(address+i, data[i]);
}

void configWrite(int address, char *data, int len) {
  configWrite(address, (byte*)data, len);
}

void configWriteInt32(int address, uint32_t data) {
  configWrite(address, (byte*)&data, 4);
}

void configWriteByte(int address, byte data) {
  configWrite(address, (byte*)&data, 1);
}
