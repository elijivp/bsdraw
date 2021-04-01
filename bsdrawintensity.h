#ifndef DRAWCOREINTENSITY_H
#define DRAWCOREINTENSITY_H

/// DrawIntensity is a draw for 2D painting
/// Input: 2D array [samplesVert]x[samplesHorz]
/// 
/// Example:
/// #include "bsdrawintensity.h"
/// #include "palettes/bspalettes_std.h"
/// 
/// DrawIntensity* draw = new DrawIntensity(SAMPLES, MAXLINES, 1);
/// draw->setDataPalette(&paletteBkGrWh);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "core/bsqdraw.h"

class DrawIntensity: public DrawQWidget
{
public:
  DrawIntensity(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions=1, ORIENTATION orient=OR_LRBT, SPLITPORTIONS splitPortions=SL_NONE);
protected:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};

class DrawIntensePoints: public DrawIntensity
{
  int     m_dcip;
  float*  m_clearBuf;
public:
  enum { DCIP_NONE=0, DCIP_CLEAR2BUF };
public:
  DrawIntensePoints(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions=1, ORIENTATION orient=OR_LRBT, SPLITPORTIONS splitPortions=SL_NONE, int dcip = DCIP_NONE);
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
