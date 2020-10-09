#ifndef DRAWCORESDP_H
#define DRAWCORESDP_H

/// This file contains special derived class for show images
/// DrawSDPicture with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawSDPicture with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
///   You must especially prepare image (paint him by markers) before use
/// 
/// Example:
/// #include "specdraws/bsdrawsdpicture.h"
/// 
/// DrawSDPicture* draw = new DrawSDPicture(SAMPLES, MAXLINES, "path_to_picture.png");
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"

class QImage;

enum { SDPSIZE_NONE =0, SDPSIZE_MARKER=63 };

/// Dont Forget: setData accept pointer to 63 floats
class DrawSDPicture: public DrawQWidget
{
  QImage*     m_pImage;
protected:
  void  reConstructor(unsigned int samplesHorz, unsigned int samplesVert);
public:
  DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, QImage* img, unsigned int backgroundColor=0x00000000);
  DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, const char* imagepath, unsigned int backgroundColor=0x00000000);
  
  DrawSDPicture(QImage* img, float sizeMultiplier=1.0f, unsigned int backgroundColor=0x00000000);
  DrawSDPicture(const char* imagepath, float sizeMultiplier=1.0f, unsigned int backgroundColor=0x00000000);
  QImage*           getImage(){ return m_pImage; }
  const QImage*     getImage() const { return m_pImage; }
  
  bool              isNull() const;
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
  virtual unsigned int    colorBack() const;
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_DD; }
};

#endif // DRAWCORESDP_H
