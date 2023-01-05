// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "esp_http_server.h"
#include "esp_timer.h"
#include "camera_index.h"
#include "camera.h"
#include "Arduino.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
extern Camera Cam;

static esp_err_t capture_handler(httpd_req_t *req)
{
   camera_fb_t *fb = NULL;
   esp_err_t res = ESP_OK;
   int64_t fr_start = esp_timer_get_time();
   fb = Cam.get_camera_frame_as_jpeg();
   if (!fb)
   {
      Serial.println("Camera capture failed");
      httpd_resp_send_500(req);
      return ESP_FAIL;
   }
   httpd_resp_set_type(req, "image/jpeg");
   httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
   res = httpd_resp_send(req, (const char*) fb->buf, fb->len);
   uint32_t fb_len = fb->len;
   Cam.free_camera_frame();
   int64_t fr_end = esp_timer_get_time();
   Serial.printf("JPG: %uB %ums\n", (uint32_t) (fb_len), (uint32_t) ((fr_end - fr_start) / 1000));
   return res;
}

static esp_err_t stream_handler(httpd_req_t *req)
{
   esp_err_t res = ESP_OK;
   camera_fb_t *fb = NULL;
   char *part_buf[64];
   res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
   if (res != ESP_OK)
   {
      return res;
   }
   while (true)
   {
      fb = Cam.get_camera_frame_as_jpeg();
      if (!fb)
      {
         Serial.println("Camera capture failed");
         httpd_resp_send_500(req);
         res = ESP_FAIL;
      }
      if (res == ESP_OK)
      {
         size_t hlen = snprintf((char*) part_buf, 64, _STREAM_PART, fb->len);
         res = httpd_resp_send_chunk(req, (const char*) part_buf, hlen);
      }
      if (res == ESP_OK)
      {
         res = httpd_resp_send_chunk(req, (const char*) fb->buf, fb->len);
      }
      if (res == ESP_OK)
      {
         res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }
      Cam.free_camera_frame();
      if (res != ESP_OK)
      {
         break;
      }
   }
   return res;
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
   char *buf;
   size_t buf_len;
   char variable[32] = { 0, };
   char value[32] = { 0, };
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
   int val = atoi(value);
   int res = 0;
   if (!strcmp(variable, "framesize"))
   {
      if (Cam.Sensors->pixformat == PIXFORMAT_JPEG) res = Cam.Sensors->set_framesize(Cam.Sensors, (framesize_t) val);
   }
   else if (!strcmp(variable, "quality")) res = Cam.Sensors->set_quality(Cam.Sensors, val);
   else if (!strcmp(variable, "contrast")) res = Cam.Sensors->set_contrast(Cam.Sensors, val);
   else if (!strcmp(variable, "brightness")) res = Cam.Sensors->set_brightness(Cam.Sensors, val);
   else if (!strcmp(variable, "saturation")) res = Cam.Sensors->set_saturation(Cam.Sensors, val);
   else if (!strcmp(variable, "gainceiling")) res = Cam.Sensors->set_gainceiling(Cam.Sensors, (gainceiling_t) val);
   else if (!strcmp(variable, "colorbar")) res = Cam.Sensors->set_colorbar(Cam.Sensors, val);
   else if (!strcmp(variable, "awb")) res = Cam.Sensors->set_whitebal(Cam.Sensors, val);
   else if (!strcmp(variable, "agc")) res = Cam.Sensors->set_gain_ctrl(Cam.Sensors, val);
   else if (!strcmp(variable, "aec")) res = Cam.Sensors->set_exposure_ctrl(Cam.Sensors, val);
   else if (!strcmp(variable, "hmirror")) res = Cam.Sensors->set_hmirror(Cam.Sensors, val);
   else if (!strcmp(variable, "vflip")) res = Cam.Sensors->set_vflip(Cam.Sensors, val);
   else if (!strcmp(variable, "awb_gain")) res = Cam.Sensors->set_awb_gain(Cam.Sensors, val);
   else if (!strcmp(variable, "agc_gain")) res = Cam.Sensors->set_agc_gain(Cam.Sensors, val);
   else if (!strcmp(variable, "aec_value")) res = Cam.Sensors->set_aec_value(Cam.Sensors, val);
   else if (!strcmp(variable, "aec2")) res = Cam.Sensors->set_aec2(Cam.Sensors, val);
   else if (!strcmp(variable, "dcw")) res = Cam.Sensors->set_dcw(Cam.Sensors, val);
   else if (!strcmp(variable, "bpc")) res = Cam.Sensors->set_bpc(Cam.Sensors, val);
   else if (!strcmp(variable, "wpc")) res = Cam.Sensors->set_wpc(Cam.Sensors, val);
   else if (!strcmp(variable, "raw_gma")) res = Cam.Sensors->set_raw_gma(Cam.Sensors, val);
   else if (!strcmp(variable, "lenc")) res = Cam.Sensors->set_lenc(Cam.Sensors, val);
   else if (!strcmp(variable, "special_effect")) res = Cam.Sensors->set_special_effect(Cam.Sensors, val);
   else if (!strcmp(variable, "wb_mode")) res = Cam.Sensors->set_wb_mode(Cam.Sensors, val);
   else if (!strcmp(variable, "ae_level")) res = Cam.Sensors->set_ae_level(Cam.Sensors, val);
   else
   {
      res = -1;
   }
   if (res)
   {
      return httpd_resp_send_500(req);
   }
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
   return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req)
{
   static char json_response[1024];
   char *p = json_response;
   *p++ = '{';
   p += sprintf(p, "\"framesize\":%u,", Cam.Sensors->status.framesize);
   p += sprintf(p, "\"quality\":%u,", Cam.Sensors->status.quality);
   p += sprintf(p, "\"brightness\":%d,", Cam.Sensors->status.brightness);
   p += sprintf(p, "\"contrast\":%d,", Cam.Sensors->status.contrast);
   p += sprintf(p, "\"saturation\":%d,", Cam.Sensors->status.saturation);
   p += sprintf(p, "\"sharpness\":%d,", Cam.Sensors->status.sharpness);
   p += sprintf(p, "\"special_effect\":%u,", Cam.Sensors->status.special_effect);
   p += sprintf(p, "\"wb_mode\":%u,", Cam.Sensors->status.wb_mode);
   p += sprintf(p, "\"awb\":%u,", Cam.Sensors->status.awb);
   p += sprintf(p, "\"awb_gain\":%u,", Cam.Sensors->status.awb_gain);
   p += sprintf(p, "\"aec\":%u,", Cam.Sensors->status.aec);
   p += sprintf(p, "\"aec2\":%u,", Cam.Sensors->status.aec2);
   p += sprintf(p, "\"ae_level\":%d,", Cam.Sensors->status.ae_level);
   p += sprintf(p, "\"aec_value\":%u,", Cam.Sensors->status.aec_value);
   p += sprintf(p, "\"agc\":%u,", Cam.Sensors->status.agc);
   p += sprintf(p, "\"agc_gain\":%u,", Cam.Sensors->status.agc_gain);
   p += sprintf(p, "\"gainceiling\":%u,", Cam.Sensors->status.gainceiling);
   p += sprintf(p, "\"bpc\":%u,", Cam.Sensors->status.bpc);
   p += sprintf(p, "\"wpc\":%u,", Cam.Sensors->status.wpc);
   p += sprintf(p, "\"raw_gma\":%u,", Cam.Sensors->status.raw_gma);
   p += sprintf(p, "\"lenc\":%u,", Cam.Sensors->status.lenc);
   p += sprintf(p, "\"vflip\":%u,", Cam.Sensors->status.vflip);
   p += sprintf(p, "\"hmirror\":%u,", Cam.Sensors->status.hmirror);
   p += sprintf(p, "\"dcw\":%u,", Cam.Sensors->status.dcw);
   p += sprintf(p, "\"colorbar\":%u,", Cam.Sensors->status.colorbar);
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
   if (Cam.Sensors->id.PID == OV3660_PID)
   {
      return httpd_resp_send(req, (const char*) index_ov3660_html_gz, index_ov3660_html_gz_len);
   }
   return httpd_resp_send(req, (const char*) index_ov2640_html_gz, index_ov2640_html_gz_len);
}

void startCameraServer()
{
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
   httpd_uri_t status_uri = { .uri = "/status", .method = HTTP_GET, .handler = status_handler, .user_ctx = NULL };
   httpd_uri_t cmd_uri = { .uri = "/control", .method = HTTP_GET, .handler = cmd_handler, .user_ctx = NULL };
   httpd_uri_t capture_uri = { .uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL };
   httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };
   Serial.printf("Starting web server on port: '%d'\n", config.server_port);
   if (httpd_start(&camera_httpd, &config) == ESP_OK)
   {
      httpd_register_uri_handler(camera_httpd, &index_uri);
      httpd_register_uri_handler(camera_httpd, &cmd_uri);
      httpd_register_uri_handler(camera_httpd, &status_uri);
      httpd_register_uri_handler(camera_httpd, &capture_uri);
   }
   config.server_port += 1;
   config.ctrl_port += 1;
   Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
   if (httpd_start(&stream_httpd, &config) == ESP_OK)
   {
      httpd_register_uri_handler(stream_httpd, &stream_uri);
   }
}
