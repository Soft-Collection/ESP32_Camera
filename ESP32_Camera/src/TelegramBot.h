#ifndef TelegramBot_h
#define TelegramBot_h

#include <Arduino.h>
#include "FS.h"
#include "config.h"
#include "SD_MMC.h"
#include "camera.h"
#include "mjpeg.h"
#include "string.h"
//#include <ArduinoJson.h>
#include <ArduinoJson.h>
#include <Client.h>
#include <WiFiClientSecure.h>
#include <TelegramKeyboard.h>

#define HOST "api.telegram.org"
#define SSL_PORT 443

#ifndef JSON_BUFF_SIZE
#ifdef ESP8266
#define JSON_BUFF_SIZE 1000
#else
#define JSON_BUFF_SIZE 1000
#endif
#endif

struct message
{
   String update_id;
   String text;
   String chat_id;
   String sender_first_name;
   String sender_last_name;
   String date;
};

class TelegramBot
{
public:
   TelegramBot();
   ~TelegramBot();
   bool begin(const char *token);
   void check();
   message getUpdates(int timeout = 0, int offset = 0);
   message getUpdatesIncrementedOffset(int timeout = 0);
   void dropUpdatesIncrementedOffset(int timeout = 0);
   String sendMessage(String chat_id, String text);
   String sendMessage(String chat_id, String text, TelegramKeyboard &keyboard_markup, bool one_time_keyboard = true, bool resize_keyboard = true);
   String sendLocation(String chat_id, double latitude, double longitude, bool disable_notification);
   String sendPhoto(String chat_id);
   String sendPhoto(String chat_id, String filePath);
   String sendVideo(String chat_id, String filePath);
private:
   String prepareRequestHeader(String token, String command, uint32_t content_length, String content_type);
   String prepareMultipartElement(String boundary, String content_disposition, String content_type, String content_data, bool isLast);
   String prepareMultipartPreDataElement(String boundary, String content_disposition, String content_type);
   String prepareMultipartPostDataElement(String boundary, bool isLast);
   String getResponse();
   String getBoundary();
   void command_request_handler(String command);
   void command_execute_handler();
private:
   const char *token;
   int last_update_id;
   Client *client;
   //-------------------
   bool requestForGetImage;
   bool requestForGetVideo;
   bool requestForGetLocation;
   bool requestForGetChatId;
   bool requestForUpdateChatId;
   //-------------------
   String chat_id;
};

extern TelegramBot Bot;

#endif
