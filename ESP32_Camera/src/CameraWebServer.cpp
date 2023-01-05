#include <WiFi.h>
#include "config.h"
#include "camera.h"
#include "mjpeg.h"
#include "driver/gpio.h"
#include <TelegramBot.h>
//--------------------------------------------------------------------------
#include "soc/soc.h" //disable brownour problems
#include "soc/rtc_cntl_reg.h"  //disable brownour problems
//--------------------------------------------------------------------------
//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//
//--------------------------------------------------------------------------
#define LED_GPIO 4
//--------------------------------------------------------------------------
bool isSetupMode = false;
bool resetDevice = false;
//------------------------------
String wifi_ssid = "";
String wifi_password = "";
String wifi_hostname = "";
//------------------------------
String bot_token = "";
//------------------------------
String chat_id = "";
//------------------------------
void startConfigServer();
////void startCameraServer();
void setupMode();
void normalMode();

void setup()
{
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
   //--------------------------------------------------------------------------
   Serial.begin(115200);
   Serial.setDebugOutput(true);
   Serial.println();
   //--------------------------------------------------------------------------
   if (!Cfg.begin())
   {
      Serial.println("Configuration Failed");
      return;
   }
   isSetupMode = Cfg.get_to_be_setup_mode();
   wifi_ssid = Cfg.get_ssid();
   wifi_password = Cfg.get_password();
   wifi_hostname = Cfg.get_hostname();
   bot_token = Cfg.get_bot_token();
   chat_id = Cfg.get_chat_id();
   Serial.println("SSID:" + wifi_ssid);
   Serial.println("Password:" + wifi_password);
   Serial.println("Hostname:" + wifi_hostname);
   Serial.println("Bot Token:" + bot_token);
   Serial.println("Chat ID:" + chat_id);
   //--------------------------------------------------------------------------
   /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
    muxed to GPIO on reset already, but some default to other
    functions and need to be switched to GPIO. Consult the
    Technical Reference for a list of pads and their default
    functions.)
    */
   gpio_pad_select_gpio(LED_GPIO);
   /* Set the GPIO as a push/pull output */
   gpio_set_direction((gpio_num_t) LED_GPIO, (gpio_mode_t) GPIO_MODE_OUTPUT);
   Cfg.set_to_be_setup_mode(true);
   gpio_set_level((gpio_num_t) LED_GPIO, HIGH);
   vTaskDelay(20);
   gpio_set_level((gpio_num_t) LED_GPIO, LOW);
   vTaskDelay(3000);
   gpio_set_level((gpio_num_t) LED_GPIO, HIGH);
   vTaskDelay(20);
   gpio_set_level((gpio_num_t) LED_GPIO, LOW);
   Cfg.set_to_be_setup_mode(false);
   //--------------------------------------------------------------------------
   if (isSetupMode)
   {
      setupMode();
   }
   else
   {
      normalMode();
   }
}

void loop()
{
   Bot.check();
   if (resetDevice)
   {
      Serial.println("Resetting...\n");
      ESP.restart();
   }
   delay(1000);
}

void setupMode()
{
   Serial.println("Setup Mode");
   //--------------------------------------------------------------------------
   WiFi.setHostname("esp32_cam");
   WiFi.mode(WIFI_AP);
   bool result = WiFi.softAP("esp32_cam_ap" /*MUST BE DIFFERENT FROM HOSTNAME!!!*/, "12345678", 1, 0);
   if (!result)
   {
       Serial.println("AP Config failed.");
       return;
   }
   else
   {
       Serial.println("AP Config Success.");
       Serial.print("AP MAC: ");
       Serial.println(WiFi.softAPmacAddress());
       Serial.print("AP IP: ");
       Serial.println(WiFi.softAPIP());
   }
   startConfigServer();
   //--------------------------------------------------------------------------
}

void normalMode()
{
   WiFi.setHostname(wifi_hostname.c_str());
   WiFi.mode(WIFI_STA);
   WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
   while (WiFi.status() != WL_CONNECTED)
   {
      delay(500);
      Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
   //--------------------------------------------------------------------------
   if (!Bot.begin(bot_token.c_str()))
   {
      Serial.println("Telegram Bot Failed");
      return;
   }
   //--------------------------------------------------------------------------
   if (!SD_MMC.begin())
   {
      Serial.println("Card Mount Failed");
      return;
   }
   uint8_t cardType = SD_MMC.cardType();
   if (cardType == CARD_NONE)
   {
      Serial.println("No SD_MMC card attached");
      return;
   }
   Serial.print("SD_MMC Card Type: ");
   if (cardType == CARD_MMC)
   {
      Serial.println("MMC");
   }
   else if (cardType == CARD_SD)
   {
      Serial.println("SDSC");
   }
   else if (cardType == CARD_SDHC)
   {
      Serial.println("SDHC");
   }
   else
   {
      Serial.println("UNKNOWN");
   }
   uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
   Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
   //--------------------------------------------------------------------------
   if (!Cam.begin())
   {
      Serial.println("Camera Failed");
      return;
   }
   //--------------------------------------------------------------------------
   if (!Mjpg.begin())
   {
      Serial.println("MJPEG Failed");
      return;
   }
   //--------------------------------------------------------------------------
   ////startCameraServer();
   //--------------------------------------------------------------------------
}
