#ifndef OIMAGE_H
#define OIMAGE_H

/// Overlays:   images through QImage
///   OImageOriginal. View: image with fixed size
///   OImageStretched. View: image with size depended of draw
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"


class QImage;
class OVLQImage
{
public:
  enum  IMAGECONVERT { IC_AUTO,       /// creates new image and converts to ARGB 32bit (with image->convertToFormat(QImage::Format_ARGB32))
                       IC_ASIS,       /// no any convertations, waiting for ARGB 32bit
                       IC_ASISOWNER,  ///  no any convertations, waiting for ARGB 32bit, delete when destroy
                       IC_BLOCKALPHA, /// creates new image and converts to RGB 32bit (with image->convertToFormat(QImage::Format_RGB32))
                     };
  enum  IMAGEOPTIONS { IO_ORIGINAL, IO_STRETCHED, IO_ORIGINAL_AUTOROTATE, IO_STRETCHED_AUTOROTATE };
public:
  OVLQImage(QImage* image, IMAGECONVERT icvt, bool autorotated, bool detach=false);
  ~OVLQImage();
  QImage*         getImage(){ return m_pImage; }
  const QImage*   getImage() const { return m_pImage; }
protected:
  bool            assignImage(QImage* image, IMAGECONVERT icvt, bool autorotated, bool detach);
protected:
  dmtype_image_t  m_dmti;         /// dont forget to register it after coords&dimms!
  QImage*         m_pImage;
  bool            m_autorotated;
  bool            m_imageowner;
};



class DrawOverlay_ColorForegoingImage: public DrawOverlay_ColorForegoing, public OVLQImage
{
protected:
  bool            m_banalpha;
public:
  DrawOverlay_ColorForegoingImage(QImage* image, IMAGECONVERT icvt, bool autorotated, bool detach=false): 
    OVLQImage(image, icvt, autorotated, detach), m_banalpha(false){}
public:
  void            banAlphaChannel(bool ban, bool update=true);
  bool            isAlphaBanned() const { return m_banalpha; }
public:
  void  reUpdate();
  bool  setImage(QImage* image, IMAGECONVERT icvt, bool autorotated, bool detach, bool update=true);
};


class OImageOriginal: public DrawOverlay_ColorForegoingImage, public OVLCoordsDynamic, public OVLDimms2Dynamic
{
public:
  OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w=1.0f, float mult_h=1.0f);
  OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic* pcoords, float offset_x, float offset_y, float mult_w=1.0f, float mult_h=1.0f);
protected:
  virtual int     fshTrace(int overlay, bool rotated, char* to) const;
  float           m_sizemultiplier[2];
};

class OImageStretched: public DrawOverlay_ColorForegoingImage, public OVLCoordsStatic, public OVLDimms2Dynamic
{
public:
  OImageStretched(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn=CR_RELATIVE, float x=0.0f, float y=0.0f, float mult_w=1.0f, float mult_h=1.0f);
  OImageStretched(QImage* image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic* pcoords, float offset_x, float offset_y, float mult_w=1.0f, float mult_h=1.0f);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

void  bs_detachFutureImagesInConstructors(bool);

#endif // OIMAGE_H
