#ifndef DRAWCOREGRAPH_H
#define DRAWCOREGRAPH_H

#include "core/bsqdraw.h"

struct  graphopts_t
{
  enum  GRAPHTYPE { GT_DOTS, GT_LINTERP, GT_LINTERPSMOOTH, GT_LINDOWN, GT_LINDOWN_CROSSMAX, GT_LINDOWN_CROSSMIN };
  GRAPHTYPE     graphtype;
  float         specopc;
  unsigned int  backcolor;
  unsigned int  dotsize;
  float         dotweight;
  enum  DESCALING { DE_NONE, DE_CENTER, DE_TRIANGLE, DE_TRIANGLE2, DE_HYPERB };
  DESCALING     descaling;
  graphopts_t(GRAPHTYPE gtype=GT_LINTERP, float opacity=0.0f, unsigned int specbckgcolor=0xFFFFFFFF, unsigned int dsize=0, float dweight=0.0f, DESCALING ds=DE_NONE):
    graphtype(gtype), specopc(opacity), backcolor(specbckgcolor), dotsize(dsize), dotweight(dweight), descaling(ds){}
  graphopts_t(GRAPHTYPE gtype, DESCALING ds, unsigned int specbckgcolor=0xFFFFFFFF):
    graphtype(gtype), specopc(0.0f), backcolor(specbckgcolor), dotsize(0), dotweight(0.0f), descaling(ds){}
};


/// 2D graph
class DrawGraph: public DrawQWidget
{
protected:
  void  reConstructor(unsigned int samples);
public:
  enum DOWNCOLORIZE { DC_OFF, DC_DOWNNEXT, DC_DOWNBASE };
  
  DrawGraph(unsigned int samples, unsigned int graphs=1, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
  DrawGraph(unsigned int samples, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
protected:
  virtual unsigned int    portionSize()const{  return m_matrixWidth; }
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_1D; }
  void                    resizeGL(int w, int h);
};

/// 2D graph with data append
class DrawGraphMove: public DrawGraph
{
private:
  unsigned int          m_stepSamples;
public:
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs=1, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
};

/// 2D graph with saved data for ScrollBar connecting
class DrawGraphMoveEx: public DrawGraph
{
private:
  unsigned int          m_stepSamples;
  int                   m_stopped;
  MemExpand1D           m_memory;
public:
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory=0, unsigned int graphs=1, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize=DC_OFF, float colorize_base=1.0f, float colorize_step=-1.0f);
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
  virtual void  clearData();
protected:
          void            fillMatrix();
  virtual void            slideLmHeight(int);
};

#endif // DRAWCOREGRAPH_H
