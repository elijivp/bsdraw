#ifndef OIMAGE_H
#define OIMAGE_H

#include "../core/bsoverlay.h"

class QImage;
class IOverlaySimpleImage: public IOverlaySimple
{
public:
  enum  IMAGECONVERT { IC_AUTO,       /// creates new image and converts to ARGB 32bit (with image->convertToFormat(QImage::Format_ARGB32))
                       IC_ASIS,       /// no any convertations, waiting for ARGB 32bit
                       IC_ASISOWNER,  ///  no any convertations, waiting for ARGB 32bit, delete when destroy
                       IC_BLOCKALPHA, /// creates new image and converts to RGB 32bit (with image->convertToFormat(QImage::Format_RGB32))
                     };
  enum  IMAGEOPTIONS { IO_ORIGINAL, IO_STRETCHED, IO_ORIGINAL_AUTOROTATE, IO_STRETCHED_AUTOROTATE };
public:
  IOverlaySimpleImage(QImage* image, IMAGECONVERT icvt, bool autorotated);
  ~IOverlaySimpleImage();
protected:
  DTYPE           m_dtype;    /// dont forget to register it after coords&dimms!
  dmtype_image_t  m_dmti;
  QImage*         m_pImage;
  bool            m_autorotated;
  bool            m_imageowner;
};
/////////////////

class OImageOriginal: public IOverlaySimpleImage, public OVLCoordsDynamic, public OVLDimms2Dynamic
{
public:
  OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w=1.0f, float mult_h=1.0f);
  OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic* pcoords, float offset_x, float offset_y, float mult_w=1.0f, float mult_h=1.0f);
protected:
  virtual int     fshTrace(int overlay, char* to) const;
  float           m_sizemultiplier[2];
};

class OImageStretched: public IOverlaySimpleImage, public OVLCoordsStatic, public OVLDimms2Static
{
public:
  OImageStretched(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn=CR_RELATIVE, float x=0.0f, float y=0.0f, float mult_w=1.0f, float mult_h=1.0f);
  OImageStretched(QImage* image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic* pcoords, float offset_x, float offset_y, float mult_w=1.0f, float mult_h=1.0f);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};


#endif // OIMAGE_H
