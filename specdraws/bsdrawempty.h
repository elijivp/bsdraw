#ifndef DRAWCOREEMPTY_H
#define DRAWCOREEMPTY_H

#include "../core/bsqdraw.h"

class DrawEmpty: public DrawQWidget
{
public:
  DrawEmpty(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backcolor);
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_2D; }
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB);
};

#endif // DRAWCOREEMPTY_H
