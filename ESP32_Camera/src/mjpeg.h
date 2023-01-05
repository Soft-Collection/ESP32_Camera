#if !defined(__MJPEG_H__)
#define __MJPEG_H__

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include "camera.h"
#include "string.h"

#define  AVIF_HASINDEX       0x00000010
#define  AVIF_MUSTUSEINDEX   0x00000020
#define  AVIF_ISINTERLEAVED  0x00000100
#define  AVIF_TRUSTCKTYPE    0x00000800
#define  AVIF_WASCAPTUREFILE 0x00010000
#define  AVIF_COPYRIGHTED    0x00020000

typedef uint32_t FOURCC;

typedef struct
{
   FOURCC fccRIFF;
   uint32_t cbRIFF;
   FOURCC fccAVI;
   //-----------------------------
   char listHDRL[4]; //LIST
   uint32_t cbListHDRL;
   FOURCC fccHDRL;
   //-----------------------------
   struct
   {
      FOURCC fcc;
      uint32_t cb;
      uint32_t dwMicroSecPerFrame;
      uint32_t dwMaxBytesPerSec;
      uint32_t dwPaddingGranularity;
      uint32_t dwFlags;
      uint32_t dwTotalFrames;
      uint32_t dwInitialFrames;
      uint32_t dwStreams;
      uint32_t dwSuggestedBufferSize;
      uint32_t dwWidth;
      uint32_t dwHeight;
      uint32_t dwReserved[4];
   } aviMainHeader;
   //-----------------------------
   char listSTRL[4]; //LIST
   uint32_t cbListSTRL;
   FOURCC fccSTRL;
   //-----------------------------
   struct
   {
      FOURCC fcc;
      uint32_t cb;
      FOURCC fccType;
      FOURCC fccHandler;
      uint32_t dwFlags;
      uint16_t wPriority;
      uint16_t wLanguage;
      uint32_t dwInitialFrames;
      uint32_t dwScale;
      uint32_t dwRate;
      uint32_t dwStart;
      uint32_t dwLength;
      uint32_t dwSuggestedBufferSize;
      uint32_t dwQuality;
      uint32_t dwSampleSize;
      struct
      {
         int16_t left;
         int16_t top;
         int16_t right;
         int16_t bottom;
      } rcFrame;
   } aviStreamHeader;
   //-----------------------------
   struct
   {
      FOURCC fcc;
      uint32_t cb;
      uint32_t biSize;
      int32_t biWidth;
      int32_t biHeight;
      int16_t biPlanes;
      int16_t biBitCount;
      uint32_t biCompression;
      uint32_t biSizeImage;
      int32_t biXPelsPerMeter;
      int32_t biYPelsPerMeter;
      uint32_t biClrUsed;
      uint32_t biClrImportant;
   } bitmapInfoHeader;
   //-----------------------------
   char listMOVI[4]; //LIST
   uint32_t cbListMOVI;
   FOURCC fccMOVI;
   //-----------------------------
} RIFF_START;

typedef struct
{
   FOURCC fccIDX1;
   uint32_t cbListIDX1;
} AVIOLDINDEXHEADER;

typedef struct
{
   uint32_t dwChunkId;
   uint32_t dwFlags;
   uint32_t dwOffset;
   uint32_t dwSize;
} AVIOLDINDEX;

class MJPEG
{
public:
   MJPEG();
   ~MJPEG();
   bool begin();
   //-----------------------------------
   void record(String path, int totalFrames);
   void snap(String path);
private:
   void init_riff_start(uint32_t width, uint32_t height, uint32_t totalFrames);
   void write_frame(camera_fb_t *fb, File* aviFile);
   void write_avi_index(uint32_t offset, uint32_t size, File* aviFile);
private:
   RIFF_START mRIFF_Start;
};

extern MJPEG Mjpg;

#endif // !defined(__MJPEG_H__)
