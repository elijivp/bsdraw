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
  Q_OBJECT
protected:
  int                     m_filldirection;
  int                     m_stopped;
  MemExpand2D             m_memory;
  unsigned int            m_resizelim;
public:
  enum  { FILL_OUTSIDE, FILL_INSIDE, FILL_DEFAULT=FILL_OUTSIDE };
  DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory=1000, unsigned int portions=1, ORIENTATION orient=OR_LRTB, SPLITPORTIONS splitPortions=SL_NONE, unsigned int resizeLimit=2160);
          void            setFillDirection(int fd);
public:
  virtual void            setData(const float*);
  virtual void            setData(const float*, DataDecimator* decim);
  virtual void            clearData();
protected:
  virtual void            resizeGL(int w, int h);
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
protected:
          void            fillMatrix();
public:
  virtual int             scrollValue() const;
          
          unsigned int    filled() const;
public slots:
  virtual void            scrollDataTo(int);
  virtual void            scrollDataToAbs(int);
};

class DrawRecorderPaged: public DrawRecorder
{
  Q_OBJECT
  
  MemExpand2D::mem_t*     m_pages;
  const unsigned int      m_pagescount;
  unsigned int            m_pagehole;
public:
  DrawRecorderPaged(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory=1000, unsigned int portions=1, 
                  unsigned int pages=1, ORIENTATION orient=OR_LRTB, SPLITPORTIONS splitPortions=SL_NONE, unsigned int resizeLimit=2160);
  ~DrawRecorderPaged();

  unsigned int  countPages() const { return m_pagescount; }
public:
  virtual void            implicitSetData(const float*);
  void                    switchPage(int idx, bool currentPageImplicitlyUpdated);
  void                    switchPageNoUpdate(int idx);  // for setData immediately
public slots:
  void              slt_switchPage(int idx);          // implicitlyUpdated = true
  void              slt_switchPageNoUpdate(int idx);  // for setData immediately
};

#endif // DRAWCORERECORDER_H
