#ifndef DRAWCORERECORDER_H
#define DRAWCORERECORDER_H

/// DrawRecorder is a draw for 2D painting by 1D lines
/// Input: 1D array who moves in top of draw
/// 
/// Example:
/// #include "bsdrawrecorder.h"
/// #include "palettes/bspalettes_std.h"
/// 
/// DrawRecorder* draw = new DrawRecorder(SAMPLES, MAXLINES, 1000, 1);
/// draw->setDataPalette(&paletteBkGrWh);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "core/bsqdraw.h"

class DrawRecorder: public DrawQWidget
{
protected:
  int                     m_stopped;
  MemExpand2D             m_memory;
  unsigned int            m_resizelim;
public:
  DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory=1000, unsigned int portions=1, ORIENTATION orient=OR_LRTB, SPLITPORTIONS splitPortions=SL_NONE, unsigned int resizeLimit=2160);
public:
  virtual void            setData(const float*);
  virtual void            setData(const float*, DataDecimator* decim);
  virtual void            clearData();
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_15D; }
  virtual void            resizeGL(int w, int h);
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
protected:
          void            fillMatrix();
public:
  virtual int             scrollValue() const;
protected slots:
  virtual void            scrollDataTo(int);
};

#endif // DRAWCORERECORDER_H
