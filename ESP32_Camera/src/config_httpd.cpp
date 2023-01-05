#include "esp_http_server.h"
#include "config.h"
#include "config_index.h"
#include "Arduino.h"

httpd_handle_t config_httpd = NULL;
extern Config Cfg;
extern bool resetDevice;

static esp_err_t cmd_handler(httpd_req_t *req)
{
   char *buf;
   size_t buf_len;
   char variable[64] = { 0, };
   char value[64] = { 0, };
   buf_len = httpd_req_get_url_query_len(req) + 1;
   if (buf_len > 1)
   {
      buf = (char*) malloc(buf_len);
      if (!buf)
      {
         httpd_resp_send_500(req);
         return ESP_FAIL;
      }
      if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
      {
         if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK && httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK)
         {
         }
         else
         {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
         }
      }
      else
      {
         free(buf);
         httpd_resp_send_404(req);
         return ESP_FAIL;
      }
      free(buf);
   }
   else
   {
      httpd_resp_send_404(req);
      return ESP_FAIL;
   }
   if (!strcmp(variable, "wifi-ssid")) Cfg.set_ssid(value);
   else if (!strcmp(variable, "wifi-password")) Cfg.set_password(value);
   else if (!strcmp(variable, "wifi-hostname")) Cfg.set_hostname(value);
   else if (!strcmp(variable, "telegram-bot-token")) Cfg.set_bot_token(value);
   else if (!strcmp(variable, "restart"))
   {
      if (!strcmp(value, "1"))
      {
         resetDevice = true;
      }
   }
   //if (res) return httpd_resp_send_500(req);
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
   return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req)
{
   static char json_response[1024];
   char *p = json_response;
   *p++ = '{';
   p += sprintf(p, "\"wifi-ssid\":\"%s\",", Cfg.get_ssid().c_str());
   p += sprintf(p, "\"wifi-password\":\"%s\",", Cfg.get_password().c_str());
   p += sprintf(p, "\"wifi-hostname\":\"%s\",", Cfg.get_hostname().c_str());
   p += sprintf(p, "\"telegram-bot-token\":\"%s\"", Cfg.get_bot_token().c_str());
   *p++ = '}';
   *p++ = 0;
   httpd_resp_set_type(req, "application/json");
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
   return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t index_handler(httpd_req_t *req)
{
   httpd_resp_set_type(req, "text/html");
   httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
   return httpd_resp_send(req, (const char*) index_config_html_gz, index_config_html_gz_len);
}

void startConfigServer()
{
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
   httpd_uri_t status_uri = { .uri = "/status", .method = HTTP_GET, .handler = status_handler, .user_ctx = NULL };
   httpd_uri_t cmd_uri = { .uri = "/control", .method = HTTP_GET, .handler = cmd_handler, .user_ctx = NULL };
   Serial.printf("Starting web server on port: '%d'\n", config.server_port);
   if (httpd_start(&config_httpd, &config) == ESP_OK)
   {
      httpd_register_uri_handler(config_httpd, &index_uri);
      httpd_register_uri_handler(config_httpd, &cmd_uri);
      httpd_register_uri_handler(config_httpd, &status_uri);
   }
}
