#include "config.h"

Config::Config(void)
{
}

Config::~Config(void)
{
}

bool Config::begin()
{
   if (EEPROM.begin(EEPROM_SIZE))
   {
      if (Load_UI8(EEPROM_ADDRESS_INITIALIZED) == 255)
      {
         //------------------------------------------------
         set_to_be_setup_mode(EEPROM_DEFAULT_TO_BE_SETUP_MODE);
         //----------------
         set_ssid(EEPROM_DEFAULT_SSID);
         set_password(EEPROM_DEFAULT_PASSWORD);
         set_hostname(EEPROM_DEFAULT_HOSTNAME);
         set_bot_token(EEPROM_DEFAULT_BOT_TOKEN);
         set_chat_id(EEPROM_DEFAULT_CHAT_ID);
         //------------------------------------------------
         Save_UI8(EEPROM_ADDRESS_INITIALIZED, 25);
         //------------------------------------------------
      }
      return true;
   }
   return false;
}
//-----------------------------------------------------------
void Config::Save_I8(int address, int8_t value)
{
   EEPROM.writeByte(address, value);
   EEPROM.commit();
}
void Config::Save_UI8(int address, uint8_t value)
{
   EEPROM.writeByte(address, value);
   EEPROM.commit();
}
void Config::Save_I16(int address, int16_t value)
{
   EEPROM.writeShort(address, value);
   EEPROM.commit();
}
void Config::Save_UI16(int address, uint16_t value)
{
   EEPROM.writeUShort(address, value);
   EEPROM.commit();
}
void Config::Save_I32(int address, int32_t value)
{
   EEPROM.writeInt(address, value);
   EEPROM.commit();
}
void Config::Save_UI32(int address, uint32_t value)
{
   EEPROM.writeUInt(address, value);
   EEPROM.commit();
}
void Config::Save_String(int address, String str, int maxLength)
{
   if (str.length() < maxLength)
   {
      EEPROM.writeString(address, str);
      EEPROM.commit();
   }
}
//-----------------------------------------------------------
int8_t Config::Load_I8(int address)
{
   return (EEPROM.readByte(address));
}
uint8_t Config::Load_UI8(int address)
{
   return (EEPROM.readByte(address));
}
int16_t Config::Load_I16(int address)
{
   return (EEPROM.readShort(address));
}
uint16_t Config::Load_UI16(int address)
{
   return (EEPROM.readUShort(address));
}
int32_t Config::Load_I32(int address)
{
   return (EEPROM.readInt(address));
}
uint32_t Config::Load_UI32(int address)
{
   return (EEPROM.readUInt(address));
}
String Config::Load_String(int address, int maxLength)
{
   String str = EEPROM.readString(address);
   return (str.length() < maxLength) ? str : String("");
}
//-----------------------------------------------------------
bool Config::get_to_be_setup_mode()
{
   return (Load_UI8(EEPROM_ADDRESS_TO_BE_SETUP_MODE) != 0);
}
String Config::get_ssid()
{
   return (Load_String(EEPROM_ADDRESS_SSID, EEPROM_LENGTH_SSID));
}
String Config::get_password()
{
   return (Load_String(EEPROM_ADDRESS_PASSWORD, EEPROM_LENGTH_PASSWORD));
}
String Config::get_hostname()
{
   return (Load_String(EEPROM_ADDRESS_HOSTNAME, EEPROM_LENGTH_HOSTNAME));
}
String Config::get_bot_token()
{
   return (Load_String(EEPROM_ADDRESS_BOT_TOKEN, EEPROM_LENGTH_BOT_TOKEN));
}
String Config::get_chat_id()
{
   return (Load_String(EEPROM_ADDRESS_CHAT_ID, EEPROM_LENGTH_CHAT_ID));
}
//-----------------------------------------------------------
void Config::set_to_be_setup_mode(bool is_setup_mode)
{
   Save_UI8(EEPROM_ADDRESS_TO_BE_SETUP_MODE, (is_setup_mode) ? 1 : 0);
}
void Config::set_ssid(String ssid)
{
   Save_String(EEPROM_ADDRESS_SSID, ssid, EEPROM_LENGTH_SSID);
}
void Config::set_password(String password)
{
   Save_String(EEPROM_ADDRESS_PASSWORD, password, EEPROM_LENGTH_PASSWORD);
}
void Config::set_hostname(String hostname)
{
   Save_String(EEPROM_ADDRESS_HOSTNAME, hostname, EEPROM_LENGTH_HOSTNAME);
}
void Config::set_bot_token(String bot_token)
{
   Save_String(EEPROM_ADDRESS_BOT_TOKEN, bot_token, EEPROM_LENGTH_BOT_TOKEN);
}
void Config::set_chat_id(String chat_id)
{
   Save_String(EEPROM_ADDRESS_CHAT_ID, chat_id, EEPROM_LENGTH_CHAT_ID);
}
//-----------------------------------------------------------

Config Cfg;
