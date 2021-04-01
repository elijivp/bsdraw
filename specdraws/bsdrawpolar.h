#ifndef DRAWPOLAR_H
#define DRAWPOLAR_H

/// This file contains special derived class for show images
/// 
/// 
/// 
/// 
/// Example:
/// #include "specdraws/bsdrawsdpicture.h"
/// 
/// DrawPolar* draw = new DrawPolar(SAMPLES, MAXLINES, PORTIONS);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"

class DrawPolar: public DrawQWidget
{
  Q_OBJECT
protected:
  class Degrator*             m_dgt;
  float                       m_deltaScale;
protected:
  void  reConstructor(unsigned int samplesHorz, unsigned int samplesVert);
public:
  DrawPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, unsigned int backgroundColor=0x00000000, SPLITPORTIONS splitGraphs=SL_NONE);
  DrawPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, float turn, float deltaScale01, unsigned int backgroundColor, SPLITPORTIONS splitGraphs);
  ~DrawPolar();
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
  virtual unsigned int    colorBack() const;
public slots:
  void    turn(float rotate01);
  void    turnDegree(float rotateDeg);
  void    turnRadian(float rotateRad);
protected:
  void    initializeGL();
  void    paintGL();
};

#endif // DRAWPOLAR_H
