#ifndef DRAWCORERECORDER_H
#define DRAWCORERECORDER_H

#include "core/bsqdraw.h"

class DrawRecorder: public DrawQWidget
{
protected:
  int                     m_stopped;
  MemExpand2D             m_memory;
  unsigned int            m_resizelim;
public:
  DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory=1000, unsigned int portions=1, ORIENTATION orient=OR_LRTB, unsigned int resizeLimit=1440);
public:
  virtual void            setData(const float*);
  virtual void            setData(const float*, DataDecimator* decim);
  virtual void            clearData();
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_15D; }
  virtual void            resizeGL(int w, int h);
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB);
protected:
          void            fillMatrix();
  virtual void            slideLmHeight(int);
};

#endif // DRAWCORERECORDER_H
