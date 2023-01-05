#include "camera.h"
//--------------------------------------------------------------------------
// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
//--------------------------------------------------------------------------
#include "camera_pins.h"
//--------------------------------------------------------------------------

Camera::Camera(void)
{
}

Camera::~Camera(void)
{
}

bool Camera::begin()
{
   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer = LEDC_TIMER_0;
   config.pin_d0 = Y2_GPIO_NUM;
   config.pin_d1 = Y3_GPIO_NUM;
   config.pin_d2 = Y4_GPIO_NUM;
   config.pin_d3 = Y5_GPIO_NUM;
   config.pin_d4 = Y6_GPIO_NUM;
   config.pin_d5 = Y7_GPIO_NUM;
   config.pin_d6 = Y8_GPIO_NUM;
   config.pin_d7 = Y9_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   config.pin_pclk = PCLK_GPIO_NUM;
   config.pin_vsync = VSYNC_GPIO_NUM;
   config.pin_href = HREF_GPIO_NUM;
   config.pin_sscb_sda = SIOD_GPIO_NUM;
   config.pin_sscb_scl = SIOC_GPIO_NUM;
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.xclk_freq_hz = 20000000;
   config.pixel_format = PIXFORMAT_JPEG;
   //--------------------------------------------------------------------------
   //init with high specs to pre-allocate larger buffers
   if (psramFound())
   {
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
   }
   else
   {
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
   }
   //--------------------------------------------------------------------------
#if defined(CAMERA_MODEL_ESP_EYE)
   pinMode(13, INPUT_PULLUP);
   pinMode(14, INPUT_PULLUP);
#endif
   //--------------------------------------------------------------------------
   // camera init
   esp_err_t err = esp_camera_init(&config);
   if (err != ESP_OK)
   {
      Serial.printf("Camera init failed with error 0x%x", err);
      return false;
   }
   Sensors = esp_camera_sensor_get();
   //initial sensors are flipped vertically and colors are a bit saturated
   if (Sensors->id.PID == OV3660_PID)
   {
      Sensors->set_vflip(Sensors, 1); //flip it back
      Sensors->set_brightness(Sensors, 1); //up the blightness just a bit
      Sensors->set_saturation(Sensors, -2); //lower the saturation
   }
   //drop down frame size for higher initial frame rate
   Sensors->set_framesize(Sensors, FRAMESIZE_VGA);
#if defined(CAMERA_MODEL_M5STACK_WIDE)
   Sensors->set_vflip(Sensors, 1);
   Sensors->set_hmirror(Sensors, 1);
#endif
   //--------------------------------------------------------------------------
   return true;
}

camera_fb_t* Camera::get_camera_frame()
{
   mCameraFrameData = esp_camera_fb_get();
   return mCameraFrameData;
}

camera_fb_t* Camera::get_camera_frame_as_jpeg()
{
   mCameraFrameData = esp_camera_fb_get();
   if (!mCameraFrameData)
   {
      Serial.println("Unable to get camera frame");
      return NULL;
   }
   if (mCameraFrameData->format == PIXFORMAT_JPEG) //We don't need to perform frame conversion. It is already JPEG.
   {
      return mCameraFrameData;
   }
   //We need to perform frame conversion.
   mJPEGData = new camera_fb_t;
   memcpy(mJPEGData, mCameraFrameData, sizeof(camera_fb_t));
   bool jpeg_converted = frame2jpg(mCameraFrameData, 80, &mJPEGData->buf, &mJPEGData->len);
   if (mCameraFrameData) //Release Camera Frame.
   {
      esp_camera_fb_return(mCameraFrameData);
      mCameraFrameData = NULL;
   }
   if (!jpeg_converted) //JPEG conversion failed.
   {
      Serial.println("JPEG compression failed");
      delete mJPEGData;
      mJPEGData = NULL;
      return mJPEGData;
   }
   //JPEG conversion success.
   return mJPEGData;
}

camera_fb_t* Camera::get_camera_frame_as_rgb888()
{
   mCameraFrameData = esp_camera_fb_get();
   if (!mCameraFrameData)
   {
      Serial.println("Unable to get camera frame");
      return NULL;
   }
   //We need to perform frame conversion.
   mRGB888Data = new camera_fb_t;
   memcpy(mRGB888Data, mCameraFrameData, sizeof(camera_fb_t));
   mRGB888Data->format = PIXFORMAT_RGB888;
   mRGB888Data->len = mCameraFrameData->width * mCameraFrameData->height * 3;
   mRGB888Data->buf = new uint8_t[mRGB888Data->len];
   bool rgb888_converted = fmt2rgb888(mCameraFrameData->buf, mCameraFrameData->len, mCameraFrameData->format, mRGB888Data->buf);
   if (mCameraFrameData) //Release Camera Frame.
   {
      esp_camera_fb_return(mCameraFrameData);
      mCameraFrameData = NULL;
   }
   if (!rgb888_converted) //RGB888 conversion failed.
   {
      Serial.println("RGB888 compression failed");
      delete(mRGB888Data->buf);
      delete mRGB888Data;
      mRGB888Data = NULL;
      return mRGB888Data;
   }
   //RGB888 conversion success.
   return mRGB888Data;
}

void Camera::free_camera_frame()
{
   if (mRGB888Data)
   {
      if (mRGB888Data->buf)
      {
         delete(mRGB888Data->buf);
         mRGB888Data->buf = NULL;
      }
      delete mRGB888Data;
      mRGB888Data = NULL;
   }
   if (mJPEGData)
   {
      if (mJPEGData->buf)
      {
         free(mJPEGData->buf);
         mJPEGData->buf = NULL;
      }
      delete mJPEGData;
      mJPEGData = NULL;
   }
   if (mCameraFrameData)
   {
      esp_camera_fb_return(mCameraFrameData);
      mCameraFrameData = NULL;
   }
}

Camera Cam;
