#include "mjpeg.h"

MJPEG::MJPEG(void)
{
}

MJPEG::~MJPEG(void)
{
}

bool MJPEG::begin()
{
   return true;
}
//-----------------------------------------------------------

void MJPEG::init_riff_start(uint32_t width, uint32_t height, uint32_t totalFrames)
{
   memset(&mRIFF_Start, 0, sizeof(mRIFF_Start));
   //-----------------------------------------
   strncpy((char*)&mRIFF_Start.fccRIFF, "RIFF", 4);
   mRIFF_Start.cbRIFF = 0; //Must be file size - 8
   strncpy((char*)&mRIFF_Start.fccAVI, "AVI ", 4);
   //-----------------------------------------
   strncpy(mRIFF_Start.listHDRL, "LIST", 4);
   mRIFF_Start.cbListHDRL = ((uint8_t*)&mRIFF_Start.listMOVI - (uint8_t*)&mRIFF_Start.listHDRL) - 8;
   strncpy((char*)&mRIFF_Start.fccHDRL, "hdlr", 4);
   //-----------------------------------------
   //https://docs.microsoft.com/en-us/previous-versions/windows/desktop/api/Aviriff/ns-aviriff-avimainheader
   strncpy((char*)&mRIFF_Start.aviMainHeader.fcc, "avih", 4);
   mRIFF_Start.aviMainHeader.cb = sizeof(mRIFF_Start.aviMainHeader) - 8;
   mRIFF_Start.aviMainHeader.dwMicroSecPerFrame = 40000;
   mRIFF_Start.aviMainHeader.dwMaxBytesPerSec = 25000;
   mRIFF_Start.aviMainHeader.dwPaddingGranularity = 0;
   mRIFF_Start.aviMainHeader.dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE;
   mRIFF_Start.aviMainHeader.dwTotalFrames = totalFrames;
   mRIFF_Start.aviMainHeader.dwInitialFrames = 0;
   mRIFF_Start.aviMainHeader.dwStreams = 1;
   mRIFF_Start.aviMainHeader.dwSuggestedBufferSize = 0x100000;
   mRIFF_Start.aviMainHeader.dwWidth = width;
   mRIFF_Start.aviMainHeader.dwHeight = height;
   //-----------------------------------------
   strncpy(mRIFF_Start.listSTRL, "LIST", 4);
   mRIFF_Start.cbListSTRL = ((uint8_t*)&mRIFF_Start.listMOVI - (uint8_t*)&mRIFF_Start.listSTRL) - 8;
   strncpy((char*)&mRIFF_Start.fccSTRL, "strl", 4);
   //-----------------------------------------
   //https://docs.microsoft.com/en-us/previous-versions/ms779638(v%3Dvs.85)
   strncpy((char*)&mRIFF_Start.aviStreamHeader.fcc, "strh", 4);
   mRIFF_Start.aviStreamHeader.cb = sizeof(mRIFF_Start.aviStreamHeader) - 8;
   strncpy((char*)&mRIFF_Start.aviStreamHeader.fccType, "vids", 4);
   strncpy((char*)&mRIFF_Start.aviStreamHeader.fccHandler, "MJPG", 4);
   mRIFF_Start.aviStreamHeader.dwFlags = 0;
   mRIFF_Start.aviStreamHeader.wPriority = 0;
   mRIFF_Start.aviStreamHeader.wLanguage = 0;
   mRIFF_Start.aviStreamHeader.dwInitialFrames = 0;
   mRIFF_Start.aviStreamHeader.dwScale = 1;
   mRIFF_Start.aviStreamHeader.dwRate = 25;
   mRIFF_Start.aviStreamHeader.dwStart = 0;
   mRIFF_Start.aviStreamHeader.dwLength = totalFrames;
   mRIFF_Start.aviStreamHeader.dwSuggestedBufferSize = 8644;
   mRIFF_Start.aviStreamHeader.dwQuality = -1;
   mRIFF_Start.aviStreamHeader.dwSampleSize = 0;
   mRIFF_Start.aviStreamHeader.rcFrame.left = 0;
   mRIFF_Start.aviStreamHeader.rcFrame.top = 0;
   mRIFF_Start.aviStreamHeader.rcFrame.right = width;
   mRIFF_Start.aviStreamHeader.rcFrame.bottom = height;
   //-----------------------------------------
   //https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
   strncpy((char*)&mRIFF_Start.bitmapInfoHeader.fcc, "strf", 4);
   mRIFF_Start.bitmapInfoHeader.cb = sizeof(mRIFF_Start.bitmapInfoHeader) - 8;
   mRIFF_Start.bitmapInfoHeader.biSize = sizeof(mRIFF_Start.bitmapInfoHeader) - 8;
   mRIFF_Start.bitmapInfoHeader.biWidth = width;
   mRIFF_Start.bitmapInfoHeader.biHeight = height;
   mRIFF_Start.bitmapInfoHeader.biPlanes = 1;
   mRIFF_Start.bitmapInfoHeader.biBitCount = 24;
   strncpy((char*)&mRIFF_Start.bitmapInfoHeader.biCompression, "MJPG", 4);
   mRIFF_Start.bitmapInfoHeader.biSizeImage = width * height * 3;
   mRIFF_Start.bitmapInfoHeader.biXPelsPerMeter = 0;
   mRIFF_Start.bitmapInfoHeader.biYPelsPerMeter = 0;
   mRIFF_Start.bitmapInfoHeader.biClrUsed = 0;
   mRIFF_Start.bitmapInfoHeader.biClrImportant = 0;
   //-----------------------------------------
   strncpy(mRIFF_Start.listMOVI, "LIST", 4);
   mRIFF_Start.cbListMOVI = 0; //idx1 offset - mRIFF.cbListMOVI offset - 8
   strncpy((char*)&mRIFF_Start.fccMOVI, "movi", 4);
}

void MJPEG::write_frame(camera_fb_t *fb, File* aviFile)
{
   uint32_t temp_register = 0;
   aviFile->write((uint8_t*)"00dc", 4); //"start of video data chunk" (00 = data stream #0, d = video, c = "compressed")
   temp_register = fb->len; aviFile->write((uint8_t*)&temp_register, 4); //Write Length of JPEG to file.
   strncpy((char*)fb->buf + 6, "AVI1", 4); //Overwrite "JFIF" (still images) with more appropriate "AVI1"
   aviFile->write(fb->buf, fb->len); //Write Data of JPEG to file.
   if (fb->len & 0x00000001)
   {
      temp_register = 0;
      aviFile->write((uint8_t*)&temp_register, 1); //Align to 16 bit: add 0 or 1 "0x00" bytes
   }
}

void MJPEG::write_avi_index(uint32_t offset, uint32_t size, File* aviFile)
{
   AVIOLDINDEX avi_old_idx;
   strncpy((char*)&avi_old_idx.dwChunkId, "00dc", 4);
   avi_old_idx.dwFlags = 0;
   avi_old_idx.dwOffset = offset;
   avi_old_idx.dwSize = size;
   aviFile->write((uint8_t*)&avi_old_idx, sizeof(avi_old_idx));
}

void MJPEG::record(String path, int totalFrames)
{
   File aviFile;
   File idxFile;
   byte buf[256];
   camera_fb_t image_info;
   uint16_t frame_cnt;
   uint32_t startms;
   uint32_t elapsed_ms;
   //------------------------------------------------------------
   camera_fb_t* fb = Cam.get_camera_frame_as_jpeg();
   if (!fb)
   {
      Serial.println(F("Camera capture failed"));
      return;
   }
   image_info = *fb;
   Cam.free_camera_frame();
   //------------------------------------------------------------
   init_riff_start(image_info.width, image_info.height, totalFrames);
   //------------------------------------------------------------
   size_t offset_from = sizeof(mRIFF_Start) + 4;
   //------------------------------------------------------------
   aviFile = SD_MMC.open(path, FILE_WRITE);
   if (!aviFile)
   {
      Serial.println(F("AVI file open failed"));
      return;
   }
   idxFile = SD_MMC.open(path + ".idx", FILE_WRITE);
   if (!idxFile)
   {
      Serial.println(F("Index file open failed"));
      return;
   }
   //------------------------------------------------------------
   //Prepare place for RIFF_Start.
   aviFile.seek(sizeof(mRIFF_Start));
   //------------------------------------------------------------
   startms = millis();
   for (frame_cnt = 0; frame_cnt < totalFrames; frame_cnt++)
   {
      // Write segment. We store 1 frame for each segment (video chunk)
      fb = Cam.get_camera_frame_as_jpeg();
      if (!fb)
      {
         Serial.println(F("Camera capture failed"));
         return;
      }
      write_avi_index((aviFile.position() + 8) - offset_from, fb->len, &idxFile);
      write_frame(fb, &aviFile);
      Cam.free_camera_frame();
   }
   elapsed_ms = millis() - startms;
   //------------------------------------------------------------
   float fRealFPS = (1000.0f * (float) frame_cnt) / ((float) elapsed_ms);
   float fmicroseconds_per_frame = 1000000.0f / fRealFPS;
   uint8_t iAttainedFPS = round(fRealFPS); //Will overwrite AVI header placeholder
   uint32_t us_per_frame = round(fmicroseconds_per_frame); //Will overwrite AVI header placeholder
   unsigned long movi_size = aviFile.size() - (unsigned long)((uint8_t*)&mRIFF_Start.listMOVI - (uint8_t*)&mRIFF_Start);
   unsigned long max_bytes_per_sec = movi_size * iAttainedFPS / frame_cnt; //hdrl.avih.max_bytes_per_sec
   //------------------------------------------------------------
   mRIFF_Start.cbRIFF = aviFile.size() + sizeof(AVIOLDINDEXHEADER) + totalFrames * sizeof(AVIOLDINDEX) - 8; //Must be file size - 8
   mRIFF_Start.cbListMOVI = movi_size;
   mRIFF_Start.aviMainHeader.dwMicroSecPerFrame = us_per_frame;
   mRIFF_Start.aviMainHeader.dwMaxBytesPerSec = max_bytes_per_sec;
   mRIFF_Start.aviStreamHeader.dwRate = iAttainedFPS;
   //------------------------------------------------------------
   AVIOLDINDEXHEADER avi_hdr_idx;
   strncpy((char*)&avi_hdr_idx.fccIDX1, "idx1", 4);
   avi_hdr_idx.cbListIDX1 = totalFrames * sizeof(AVIOLDINDEX);
   //------------------------------------------------------------
   //Write AVI OLD INDEX HEADER
   aviFile.write((const uint8_t*)&avi_hdr_idx, sizeof(avi_hdr_idx));
   //------------------------------------------------------------
   //Write AVI Main Header
   aviFile.seek(0);
   aviFile.write((const uint8_t*)&mRIFF_Start, sizeof(mRIFF_Start));
   //------------------------------------------------------------
   idxFile.close();
   //------------------------------------------------------------
   idxFile = SD_MMC.open(path + ".idx", FILE_READ);
   if (!idxFile)
   {
      Serial.println(F("Index file open failed"));
      return;
   }
   size_t n;
   aviFile.seek(0, (SeekMode)SEEK_END);
   while ((n = idxFile.read(buf, sizeof(buf))) > 0)
   {
      aviFile.write(buf, n);
   }
   idxFile.close();
   if (!SD_MMC.remove(path + ".idx"))
   {
      Serial.println(F("Index file delete failed"));
      return;
   }
   //------------------------------------------------------------
   aviFile.close();
}

void MJPEG::snap(String path)
{
   File jpgFile = SD_MMC.open(path, FILE_WRITE);
   if (!jpgFile)
   {
      Serial.println(F("AVI file open failed"));
      return;
   }
   camera_fb_t* fb = Cam.get_camera_frame_as_jpeg();
   if (!fb)
   {
      Serial.println(F("Camera capture failed"));
      return;
   }
   jpgFile.write(fb->buf, fb->len);
   Cam.free_camera_frame();
   jpgFile.close();
}

MJPEG Mjpg;
