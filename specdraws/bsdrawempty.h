#ifndef DRAWCOREEMPTY_H
#define DRAWCOREEMPTY_H

/// This file contains special derived class for trivial cases
/// DrawEmpty does nothing, just holds actual for 2D draws space (throught sizeAndScaleHint)
///
/// Example:
/// #include "specdraws/bsdrawempty.h"
/// 
/// DrawEmpty* draw = new DrawEmpty(SAMPLES, MAXLINES, 0x00FFFFFF);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"

class DrawEmpty: public DrawQWidget
{
  bool      m_allowNoscaledResize;
public:
  DrawEmpty(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backcolor, bool allowNoscaledResize=false);
protected:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};

#endif // DRAWCOREEMPTY_H
