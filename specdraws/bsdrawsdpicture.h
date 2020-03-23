#ifndef DRAWCORESDP_H
#define DRAWCORESDP_H

#include "../core/bsqdraw.h"

class QImage;

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
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_DD; }
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB);
};

#endif // DRAWCORESDP_H
