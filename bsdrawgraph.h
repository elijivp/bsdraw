#ifndef DRAWCOREGRAPH_H
#define DRAWCOREGRAPH_H

#include "core/bsqdraw.h"

struct  graphopts_t
{
  enum  GRAPHTYPE { GT_DOTS,          /// most simple, most lightweight
                    GT_LINTERP,       /// standard linear interpolation (default)
                    GT_LINTERPSMOOTH, /// linear interpolation through glsl smoothstep method
                    GT_LINDOWN,       /// histogram
                    GT_LINDOWN_CROSSMAX, /// histogram with max of many graphs on cross
                    GT_LINDOWN_CROSSMIN  /// histogram with all graphs on cross
                  };
  GRAPHTYPE     graphtype;
  float         specopc;
  unsigned int  backcolor;            /// color to use instead of palette color. 0xFFFFFFFF - not use
  int           dotsize;              /// >0 : abs size; <0 : scaling + size
  float         dotweight;            /// 0..1
  
  enum  DESCALING { DE_NONE,          /// no descaling (default)
                    DE_LINTERP,       /// descaling through linear interpolation
                    DE_CENTER,        /// only central point left
                    DE_TRIANGLE,      /// ... additional smooth descaling algos
                    DE_TRIANGLE2, 
                    DE_HYPERB 
                  };
  DESCALING     descaling;
  float         specsmooth;
  graphopts_t(GRAPHTYPE gtype=GT_LINTERP, float opacity=0.0f, unsigned int specbckgcolor=0xFFFFFFFF, 
              unsigned int dsize=0, float dweight=0.0f, DESCALING ds=DE_NONE,
              float smoothcoef=0.0f):
    graphtype(gtype), specopc(opacity), backcolor(specbckgcolor), 
    dotsize(dsize), dotweight(dweight), descaling(ds), specsmooth(smoothcoef){}
  graphopts_t(GRAPHTYPE gtype, DESCALING ds, unsigned int specbckgcolor=0xFFFFFFFF):
    graphtype(gtype), specopc(0.0f), backcolor(specbckgcolor), 
    dotsize(0), dotweight(0.0f), descaling(ds), specsmooth(0.0f){}
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
  virtual unsigned int    portionSize()const{  return m_matrixDimmA; }
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
