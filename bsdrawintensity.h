#ifndef DRAWCOREINTENSITY_H
#define DRAWCOREINTENSITY_H

#include "core/bsqdraw.h"

class DrawIntensity: public DrawQWidget
{
public:
  DrawIntensity(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions=1, ORIENTATION orient=OR_LRBT);
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_2D; }
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};


class DrawIntensePoints: public DrawIntensity
{
  int     m_dcip;
  float*  m_clearBuf;
public:
  enum { DCIP_NONE=0, DCIP_CLEAR2BUF };
public:
  DrawIntensePoints(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions=1, ORIENTATION orient=OR_LRBT, int dcip = DCIP_NONE);
public:
  /// standard method 'setData' still working
  /// Additional methods for data:
  void setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, float value=1.0f);
  void setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, const float* datavalues);
  
  void setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, float value=1.0f);
  void setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, const float* datavalues);
  
  void setData(const unsigned int* dataCountByPortions, const unsigned int* absXs, const unsigned int* absYs, float value=1.0f);
  void setData(const unsigned int* dataCountByPortions, const unsigned int* absXs, const unsigned int* absYs, const float* datavalues);
};

#endif // DRAWCOREINTENSITY_H
