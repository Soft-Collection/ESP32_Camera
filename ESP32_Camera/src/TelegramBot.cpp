#include "TelegramBot.h"

TelegramBot::TelegramBot(void)
{
   this->last_update_id = 0;
   this->client = NULL;
   this->token = NULL;
   //-------------------
   this->requestForGetImage = false;
   this->requestForGetVideo = false;
   this->requestForGetLocation = false;
   this->requestForGetChatId = false;
   this->requestForUpdateChatId = false;
   //-------------------
   this->chat_id = "0";
}

TelegramBot::~TelegramBot(void)
{
   if (this->client != NULL)
   {
      delete this->client;
      this->client = NULL;
   }
}

bool TelegramBot::begin(const char *token)
{
   this->token = token;
   this->chat_id = Cfg.get_chat_id();
   //----------------------------------------------------
   if (this->client == NULL)
   {
      this->client = new WiFiClientSecure();
   }
   //----------------------------------------------------
   return true;
}

void TelegramBot::check()
{
   static bool firstTime = false;
   if (!firstTime)
   {
      this->dropUpdatesIncrementedOffset(0); //Drop all messages.
      firstTime = true;
      //----------------------------------------------------
      ////command_request_handler("/get_image");
      command_request_handler("/get_video");
      //----------------------------------------------------
   }
   message m = this->getUpdatesIncrementedOffset(0); // Read new message
   command_request_handler(m.text);
   command_execute_handler();
}

/************************************************************************************
 * GetUpdates - function to receive messages from telegram as a Json and parse them *
 ************************************************************************************/
message TelegramBot::getUpdates(int timeout, int offset)
{
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   StaticJsonBuffer< JSON_BUFF_SIZE> jsonBuffer;
   JsonObject &buff = jsonBuffer.createObject();
   buff["timeout"] = String(timeout);
   if (offset > 0) buff["offset"] = String(offset);
   buff["limit"] = String(1); //Read only 1 message.
   String data;
   buff.printTo(data);
   //----------------------------------------------------------------------
   String header = prepareRequestHeader(token, "getUpdates", data.length(), "application/json");
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(data);
   Serial.println(data);
   //----------------------------------------------------------------------
   String response = getResponse();
   Serial.println(response);
   /* EXAMPLE:
    {
    "ok":true,
    "result":
    [
    {
    "update_id":239499093,
    "message":
    {
    "message_id":246,
    "from":
    {
    "id":162885179,
    "is_bot":false,
    "first_name":"Michael",
    "last_name":"Margold",
    "language_code":"en"
    },
    "chat":
    {
    "id":162885179,
    "first_name":"Michael",
    "last_name":"Margold",
    "type":"private"
    },
    "date":1581796192,
    "text":"hjkhj"
    }
    }
    ]
    }
    */
   message retVal = { "", "", "", "", "", "" };
   if (response != "")
   {
      StaticJsonBuffer<JSON_BUFF_SIZE> jsonBuffer;
      JsonObject &root = jsonBuffer.parseObject(response);
      if (root.success())
      {
         String update_id = root["result"][0]["update_id"];
         String sender_first_name = root["result"][0]["message"]["from"]["first_name"];
         String sender_last_name = root["result"][0]["message"]["from"]["last_name"];
         String text = root["result"][0]["message"]["text"];
         String chat_id = root["result"][0]["message"]["chat"]["id"];
         String date = root["result"][0]["message"]["date"];
         retVal.update_id = update_id;
         retVal.sender_first_name = sender_first_name;
         retVal.sender_last_name = sender_last_name;
         retVal.text = text;
         retVal.chat_id = chat_id;
         retVal.date = date;
         //-------------------------------
         if (chat_id != "")
         {
            if (this->chat_id == "0")
            {
               Cfg.set_chat_id(chat_id);
               this->chat_id = Cfg.get_chat_id();
            }
            else
            {
               this->chat_id = chat_id;
            }
         }
      }
      else //Response is too long, handle it as String and not as JSON object.
      {
         Serial.println("Message too long, skipped.");
         int idx1 = response.indexOf("update_id", 0);
         if (idx1 != (-1))
         {
            int idx2 = response.indexOf(':', idx1 + 1);
            if (idx2 != (-1))
            {
               int idx3 = response.indexOf(',', idx2 + 1);
               if (idx3 != (-1))
               {
                  retVal.update_id = response.substring(idx2 + 1, idx3);
               }
            }
         }
      }
   }
   else //Response is empty.
   {
      Serial.println("Response is empty.");
   }
   return retVal;
}

message TelegramBot::getUpdatesIncrementedOffset(int timeout)
{
   message retVal;
   if (last_update_id == 0)
   {
      retVal = getUpdates(timeout);
      if (retVal.update_id != "")
      {
         last_update_id = retVal.update_id.toInt();
      }
   }
   else
   {
      retVal = getUpdates(timeout, ++last_update_id);
      if (retVal.update_id == "") --last_update_id;
   }
   return retVal;
}

void TelegramBot::dropUpdatesIncrementedOffset(int timeout)
{
   message tempMessage;
   while (true)
   {
      if (last_update_id == 0)
      {
         tempMessage = getUpdates(timeout);
         if (tempMessage.update_id != "")
         {
            last_update_id = tempMessage.update_id.toInt();
         }
         else //tempMessage.update_id == ""
         {
            break;
         }
      }
      else
      {
         tempMessage = getUpdates(timeout, ++last_update_id);
         if (tempMessage.update_id == "")
         {
            --last_update_id;
            break;
         }
      }
   }
}

// send a simple text message to a telegram chat
String TelegramBot::sendMessage(String chat_id, String text)
{
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   StaticJsonBuffer< JSON_BUFF_SIZE> jsonBuffer;
   JsonObject &buff = jsonBuffer.createObject();
   buff["chat_id"] = chat_id;
   buff["text"] = text;
   String data;
   buff.printTo(data);
   //----------------------------------------------------------------------
   String header = prepareRequestHeader(token, "sendMessage", data.length(), "application/json");
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(data);
   Serial.println(data);
   //----------------------------------------------------------------------
   return getResponse();
}

// send a message to a telegram chat with a reply markup
String TelegramBot::sendMessage(String chat_id, String text, TelegramKeyboard &keyboard_markup, bool one_time_keyboard, bool resize_keyboard)
{
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   StaticJsonBuffer< JSON_BUFF_SIZE> jsonBuffer;
   JsonObject &buff = jsonBuffer.createObject();
   buff["chat_id"] = chat_id;
   buff["text"] = text;
   JsonObject &reply_markup = buff.createNestedObject("reply_markup");
   JsonArray &keyboard = reply_markup.createNestedArray("keyboard");
   for (int a = 1; a <= keyboard_markup.length(); a++)
   {
      JsonArray &row = keyboard.createNestedArray();
      for (int b = 1; b <= keyboard_markup.rowSize(a); b++)
      {
         row.add(keyboard_markup.getButton(a, b));
      }
   }
   reply_markup.set<bool>("one_time_keyboard", one_time_keyboard);
   reply_markup.set<bool>("resize_keyboard", resize_keyboard);
   reply_markup.set<bool>("selective", false);
   String data;
   buff.printTo(data);
   //----------------------------------------------------------------------
   String header = prepareRequestHeader(token, "sendMessage", data.length(), "application/json");
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(data);
   Serial.println(data);
   //----------------------------------------------------------------------
   return getResponse();
}

// send location.
String TelegramBot::sendLocation(String chat_id, double latitude, double longitude, bool disable_notification)
{
   if (latitude > 90) latitude = 90;
   if (latitude < (-90)) latitude = (-90);
   if (longitude > 180) longitude = 180;
   if (longitude < (-180)) longitude = (-180);
   //----------------------------------------------------------------------
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   StaticJsonBuffer< JSON_BUFF_SIZE> jsonBuffer;
   JsonObject &buff = jsonBuffer.createObject();
   buff["chat_id"] = chat_id;
   buff["latitude"] = String(latitude);
   buff["longitude"] = String(longitude);
   buff["disable_notification"] = String((disable_notification) ? "True" : "False");
   String data;
   buff.printTo(data);
   //----------------------------------------------------------------------
   String header = prepareRequestHeader(token, "sendLocation", data.length(), "application/json");
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(data);
   Serial.println(data);
   //----------------------------------------------------------------------
   return getResponse();
}
//Don't use this function to send image from camera!
//Use next sendPhoto function!
//This function gets an image directly from the camera related memory,
//and retains this memory until sending process ends.
//Better way is to use SD Card to save image and release the camera related memory.
//After that, just to send an image from SD Card to Telegram.
String TelegramBot::sendPhoto(String chat_id)
{
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   String boundary = getBoundary();
   //----------------------------------------------------------------------
   camera_fb_t* fb = Cam.get_camera_frame_as_jpeg();
   if (!fb)
   {
      Serial.println(F("Camera capture failed"));
      return String("");
   }
   //----------------------------------------------------------------------
   String chatIdElement = prepareMultipartElement(boundary, "form-data; name=\"chat_id\"", "", chat_id, false);
   String prePhotoElement = prepareMultipartPreDataElement(boundary, "form-data; name=\"photo\"; filename=\"/my_photo.jpg\"", "image/jpeg");
   String postPhotoElement = prepareMultipartPostDataElement(boundary, false);
   String notificationElement = prepareMultipartElement(boundary, "form-data; name=\"disable_notification\"", "", "False", true);
   String header = prepareRequestHeader(token, "sendPhoto", chatIdElement.length() + prePhotoElement.length() + fb->len + postPhotoElement.length() + notificationElement.length(), String("multipart/form-data; boundary=") + boundary);
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(chatIdElement);
   Serial.println(chatIdElement);
   client->print(prePhotoElement);
   Serial.println(prePhotoElement);
   //------------------------------------------
   client->write(fb->buf, fb->len);
   Cam.free_camera_frame();
   //------------------------------------------
   client->print(postPhotoElement);
   Serial.println(postPhotoElement);
   client->print(notificationElement);
   Serial.println(notificationElement);
   //----------------------------------------------------------------------
   return getResponse();
}

//Use this function to send image from camera!
//This function uses SD Card to save an image.
//After that, it sends an image from SD Card to Telegram.
String TelegramBot::sendPhoto(String chat_id, String path)
{
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   String boundary = getBoundary();
   //----------------------------------------------------------------------
   File jpgFile;
   byte buf[2048];
   jpgFile = SD_MMC.open(path, FILE_READ);
   if (!jpgFile)
   {
      Serial.println(F("JPG file open failed"));
      return String("");
   }
   //----------------------------------------------------------------------
   String chatIdElement = prepareMultipartElement(boundary, "form-data; name=\"chat_id\"", "", chat_id, false);
   String prePhotoElement = prepareMultipartPreDataElement(boundary, "form-data; name=\"photo\"; filename=\"" + path + "\"", "image/jpeg");
   String postPhotoElement = prepareMultipartPostDataElement(boundary, false);
   String notificationElement = prepareMultipartElement(boundary, "form-data; name=\"disable_notification\"", "", "False", true);
   String header = prepareRequestHeader(token, "sendPhoto", chatIdElement.length() + prePhotoElement.length() + jpgFile.size() + postPhotoElement.length() + notificationElement.length(), String("multipart/form-data; boundary=") + boundary);
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(chatIdElement);
   Serial.println(chatIdElement);
   client->print(prePhotoElement);
   Serial.println(prePhotoElement);
   //------------------------------------------
   size_t n;
   while ((n = jpgFile.read(buf, sizeof(buf))) > 0)
   {
      client->write(buf, n);
   }
   jpgFile.close();
   //------------------------------------------
   client->print(postPhotoElement);
   Serial.println(postPhotoElement);
   client->print(notificationElement);
   Serial.println(notificationElement);
   //----------------------------------------------------------------------
   return getResponse();
}

String TelegramBot::sendVideo(String chat_id, String path)
{
   if (chat_id == "0" || chat_id == "")
   {
      Serial.println("Chat_id not defined");
      return String("");
   }
   //----------------------------------------------------------------------
   if (!client->connected()) client->connect(HOST, SSL_PORT);
   //----------------------------------------------------------------------
   String boundary = getBoundary();
   //----------------------------------------------------------------------
   File aviFile;
   byte buf[2048];
   aviFile = SD_MMC.open(path, FILE_READ);
   if (!aviFile)
   {
      Serial.println(F("AVI file open failed"));
      return String("");
   }
   //----------------------------------------------------------------------
   String chatIdElement = prepareMultipartElement(boundary, "form-data; name=\"chat_id\"", "", chat_id, false);
   String prePhotoElement = prepareMultipartPreDataElement(boundary, "form-data; name=\"video\"; filename=\"" + path + "\"", "video/mp4");
   String postPhotoElement = prepareMultipartPostDataElement(boundary, false);
   String streamingElement = prepareMultipartElement(boundary, "form-data; name=\"supports_streaming\"", "", "True", true);
   String notificationElement = prepareMultipartElement(boundary, "form-data; name=\"disable_notification\"", "", "False", true);
   String header = prepareRequestHeader(token, "sendVideo", chatIdElement.length() + prePhotoElement.length() + aviFile.size() + postPhotoElement.length() + streamingElement.length() + notificationElement.length(), String("multipart/form-data; boundary=") + boundary);
   //----------------------------------------------------------------------
   client->print(header);
   Serial.println(header);
   client->print(chatIdElement);
   Serial.println(chatIdElement);
   client->print(prePhotoElement);
   Serial.println(prePhotoElement);
   //------------------------------------------
   size_t n;
   while ((n = aviFile.read(buf, sizeof(buf))) > 0)
   {
      client->write(buf, n);
   }
   aviFile.close();
   //------------------------------------------
   client->print(postPhotoElement);
   Serial.println(postPhotoElement);
   client->print(notificationElement);
   Serial.println(notificationElement);
   //----------------------------------------------------------------------
   return getResponse();
}

String TelegramBot::prepareRequestHeader(String token, String command, uint32_t content_length, String content_type)
{
   String headerTemplate = "POST /bot<token>/<command> HTTP/1.1\r\n"
         "Host: api.telegram.org\r\n"
         "Accept-Encoding: identity\r\n"
         "Content-Length: <content_length>\r\n"
         "Content-Type: <content_type>\r\n"
         "connection: keep-alive\r\n"
         "user-agent: Python Telegram Bot (https://github.com/python-telegram-bot/python-telegram-bot)\r\n"
         "\r\n";
   headerTemplate.replace("<token>", token);
   headerTemplate.replace("<command>", command);
   headerTemplate.replace("<content_length>", String(content_length));
   headerTemplate.replace("<content_type>", content_type);
   return headerTemplate;
}

String TelegramBot::prepareMultipartElement(String boundary, String content_disposition, String content_type, String content_data, bool isLast)
{
   String elementTemplate = "--<boundary>\r\n"
         "Content-Disposition: <content_disposition>\r\n"
         "<content_type>"
         "\r\n"
         "<content_data>\r\n";
   if (isLast) elementTemplate += "--<boundary>--\r\n";
   elementTemplate.replace("<boundary>", boundary);
   elementTemplate.replace("<content_disposition>", content_disposition);
   elementTemplate.replace("<content_type>", (content_type != "") ? "Content-Type: " + content_type + "\r\n" : "");
   elementTemplate.replace("<content_data>", content_data);
   return elementTemplate;
}

String TelegramBot::prepareMultipartPreDataElement(String boundary, String content_disposition, String content_type)
{
   String elementTemplate = "--<boundary>\r\n"
         "Content-Disposition: <content_disposition>\r\n"
         "<content_type>"
         "\r\n";
   elementTemplate.replace("<boundary>", boundary);
   elementTemplate.replace("<content_disposition>", content_disposition);
   elementTemplate.replace("<content_type>", (content_type != "") ? "Content-Type: " + content_type + "\r\n" : "");
   return elementTemplate;
}

String TelegramBot::prepareMultipartPostDataElement(String boundary, bool isLast)
{
   String elementTemplate = "\r\n";
   if (isLast) elementTemplate += "--<boundary>--\r\n";
   elementTemplate.replace("<boundary>", boundary);
   return elementTemplate;
}

String TelegramBot::getResponse()
{
   if (client->connected())
   {
      String response = client->readString();
      int idxStart = response.lastIndexOf("\r\n\r\n");
      if (idxStart != (-1))
      {
         idxStart += 4;
         response = response.substring(idxStart);
         return response;
      }
      return "";
   }
   return "";
}

String TelegramBot::getBoundary()
{
   String boundary = "";
   for (int i = 0; i < 32; i++)
   {
      long rnd = random(0, 15);
      char boundaryLetter = (char)((rnd < 10) ? rnd + 48 : (rnd - 10) + 97);
      boundary += String(boundaryLetter);
   }
   return boundary;
}

void TelegramBot::command_request_handler(String command)
{
   bool busyPerformingCommand = requestForGetImage || requestForGetVideo || requestForGetLocation || requestForGetChatId || requestForUpdateChatId;
   if (!busyPerformingCommand)
   {
      if (command == "/get_image")
      {
         requestForGetImage = true;
      }
      else if (command == "/get_video")
      {
         requestForGetVideo = true;
      }
      else if (command == "/get_location")
      {
         requestForGetLocation = true;
      }
      else if (command == "/get_chat_id")
      {
         requestForGetChatId = true;
      }
      else if (command == "/update_chat_id")
      {
         requestForUpdateChatId = true;
      }
   }
}

void TelegramBot::command_execute_handler()
{
   bool busyPerformingCommand = requestForGetImage || requestForGetVideo || requestForGetLocation || requestForGetChatId || requestForUpdateChatId;
   if (busyPerformingCommand)
   {
      if (this->requestForGetImage)
      {
         Mjpg.snap("/photo.jpg");
         sendPhoto(this->chat_id, "/photo.jpg");
         requestForGetImage = false;
      }
      if (this->requestForGetVideo)
      {
         Mjpg.record("/video.avi", 100);
         sendVideo(this->chat_id, "/video.avi");
         requestForGetVideo = false;
      }
      if (this->requestForGetLocation)
      {
         sendLocation(this->chat_id, -32.045, -132.045, true);
         requestForGetLocation = false;
      }
      if (this->requestForGetChatId)
      {
         sendMessage(this->chat_id, "Your Chat ID: " + this->chat_id);
         sendMessage(this->chat_id, "Active Chat ID: " + Cfg.get_chat_id());
         requestForGetChatId = false;
      }
      if (this->requestForUpdateChatId)
      {
         Cfg.set_chat_id(this->chat_id);
         sendMessage(this->chat_id, "Your Chat ID: " + this->chat_id);
         sendMessage(this->chat_id, "Active Chat ID: " + Cfg.get_chat_id());
         requestForUpdateChatId = false;
      }
      dropUpdatesIncrementedOffset(0); //Drop all messages received while executing previous command.
   }
}

TelegramBot Bot;
