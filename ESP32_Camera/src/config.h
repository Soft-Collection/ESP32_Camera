//https://randomnerdtutorials.com/esp32-flash-memory/
#if !defined(__CONFIG_H__)
#define __CONFIG_H__

#include "Arduino.h"
#include "EEPROM.h"

class Config
{
public:
   Config();
   ~Config();
   bool begin();
   //-----------------------------------
   bool get_to_be_setup_mode();
   String get_ssid();
   String get_password();
   String get_hostname();
   String get_bot_token();
   String get_chat_id();
   //-----------------------------------
   void set_to_be_setup_mode(bool is_setup_mode);
   void set_ssid(String ssid);
   void set_password(String password);
   void set_hostname(String hostname);
   void set_bot_token(String bot_token);
   void set_chat_id(String chat_id);
private:
   void Save_I8(int address, int8_t value);
   void Save_UI8(int address, uint8_t value);
   void Save_I16(int address, int16_t value);
   void Save_UI16(int address, uint16_t value);
   void Save_I32(int address, int32_t value);
   void Save_UI32(int address, uint32_t value);
   void Save_String(int address, String str, int maxLength);
   //-----------------------------------
   int8_t Load_I8(int address);
   uint8_t Load_UI8(int address);
   int16_t Load_I16(int address);
   uint16_t Load_UI16(int address);
   int32_t Load_I32(int address);
   uint32_t Load_UI32(int address);
   String Load_String(int address, int maxLength);
   //-----------------------------------
   const int EEPROM_SIZE = 512;
   //----------------
   const int EEPROM_ADDRESS_INITIALIZED = 0;
   const int EEPROM_LENGTH_INITIALIZED = 1;
   const uint8_t EEPROM_DEFAULT_INITIALIZED = 0;
   //----------------
   const int EEPROM_ADDRESS_TO_BE_SETUP_MODE = EEPROM_ADDRESS_INITIALIZED + EEPROM_LENGTH_INITIALIZED;
   const int EEPROM_LENGTH_TO_BE_SETUP_MODE = 1;
   const uint8_t EEPROM_DEFAULT_TO_BE_SETUP_MODE = 0;
   //----------------
   const int EEPROM_ADDRESS_SSID = EEPROM_ADDRESS_TO_BE_SETUP_MODE + EEPROM_LENGTH_TO_BE_SETUP_MODE;
   const int EEPROM_LENGTH_SSID = 64;
   const String EEPROM_DEFAULT_SSID = "SSID";
   //----------------
   const int EEPROM_ADDRESS_PASSWORD = EEPROM_ADDRESS_SSID + EEPROM_LENGTH_SSID;
   const int EEPROM_LENGTH_PASSWORD = 64;
   const String EEPROM_DEFAULT_PASSWORD = "12345678";
   //----------------
   const int EEPROM_ADDRESS_HOSTNAME = EEPROM_ADDRESS_PASSWORD + EEPROM_LENGTH_PASSWORD;
   const int EEPROM_LENGTH_HOSTNAME = 64;
   const String EEPROM_DEFAULT_HOSTNAME = "camdevice";
   //----------------
   const int EEPROM_ADDRESS_BOT_TOKEN = EEPROM_ADDRESS_HOSTNAME + EEPROM_LENGTH_HOSTNAME;
   const int EEPROM_LENGTH_BOT_TOKEN = 64;
   const String EEPROM_DEFAULT_BOT_TOKEN = "MY_BOT_TOKEN";
   //----------------
   const int EEPROM_ADDRESS_CHAT_ID = EEPROM_ADDRESS_BOT_TOKEN + EEPROM_LENGTH_BOT_TOKEN;
   const int EEPROM_LENGTH_CHAT_ID = 64;
   const String EEPROM_DEFAULT_CHAT_ID = "0";
};

extern Config Cfg;

#endif // !defined(__CONFIG_H__)
