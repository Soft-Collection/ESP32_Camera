#if !defined(__CAMERA_H__)
#define __CAMERA_H__

#include "Arduino.h"
#include "esp_camera.h"
#include "esp_err.h"
#include "img_converters.h"

class Camera
{
public:
   Camera();
   ~Camera();
   bool begin();
   //-----------------------------------
   camera_fb_t* get_camera_frame();
   camera_fb_t* get_camera_frame_as_jpeg();
   camera_fb_t* get_camera_frame_as_rgb888();
   void free_camera_frame();
   //-----------------------------------
   sensor_t* Sensors = NULL;
private:
   camera_fb_t* mCameraFrameData = NULL;
   camera_fb_t* mJPEGData = NULL;
   camera_fb_t* mRGB888Data = NULL;
};

extern Camera Cam;

#endif // !defined(__CAMERA_H__)
