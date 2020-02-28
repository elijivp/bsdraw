#ifndef DRAWCOREGRAPH_H
#define DRAWCOREGRAPH_H

#include "core/bsqdraw.h"


enum  BSGRAPHTYPE {
                    GT_DOTS,                /// most simple, most lightweight
                    GT_LINTERP,             /// standard linear interpolation (default)
                    GT_LINTERPSMOOTH,       /// linear interpolation through glsl smoothstep method
                    GT_HISTOGRAM,           /// histogram
                    GT_HISTOGRAM_CROSSMAX,  /// histogram with max of many graphs on cross
                    GT_HISTOGRAM_CROSSMIN   /// histogram with all graphs on cross
                };

enum  BSDESCALING {
                    DE_NONE,                /// no descaling (default)
                    DE_LINTERP,             /// descaling through linear interpolation
                    DE_CENTER,              /// only central point left
                    DE_TRIANGLE,            /// ... additional smooth descaling algos
                    DE_TRIANGLE2, 
                    DE_HYPERB,
                    DE_SINTERP,             /// halfsmooth descaling through 3 points
                    DE_QINTERP              /// smooth descaling through 4 points
                };

enum  BSPOSTRECT {
                    PR_STANDARD,
                    PR_VALUEAROUND,
                    PR_VALUEONLY,
                    PR_SUMMARY
                };

struct  graphopts_t
{
  BSGRAPHTYPE     graphtype;
  BSDESCALING     descaling;
  float           opacity;
  unsigned int    backcolor;                /// color to use instead of palette color. 0xFFFFFFFF - not use
  int             dotsize;                  /// >0 : abs size; <0 : scaling + size
  float           dotsmooth;                /// 0..2 optimal, smooth quality
  float           smooth;
  BSPOSTRECT      postrect;
  
  graphopts_t(BSGRAPHTYPE gtype=GT_DOTS, BSDESCALING ds=DE_NONE, float opacity=0.0f, unsigned int specbckgcolor=0xFFFFFFFF, 
              int dsize=0, float dsmooth=0.0f, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD):
    graphtype(gtype), descaling(ds), opacity(opacity), backcolor(specbckgcolor), 
    dotsize(dsize), dotsmooth(dsmooth), smooth(smoothcoef), postrect(pr){}
  
  
  // 1. typical linterp graph
  static graphopts_t goInterp(BSDESCALING ds, unsigned int specbckgcolor=0xFFFFFFFF, int dsize=0, float dsmooth=0.0f)
  { return graphopts_t(GT_LINTERP, ds, 0.0f, specbckgcolor, dsize, dsmooth, 0.0f, PR_STANDARD);  }
  static graphopts_t goInterp(float smoothcoef, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF, int dsize=0, float dsmooth=0.0f)
  { return graphopts_t(GT_LINTERP, ds, 0.0f, specbckgcolor, dsize, dsmooth, smoothcoef, PR_STANDARD);  }
  
  static graphopts_t goInterp2(BSDESCALING ds, unsigned int specbckgcolor=0xFFFFFFFF, int dsize=0, float dsmooth=0.0f)
  { return graphopts_t(GT_LINTERPSMOOTH, ds, 0.0f, specbckgcolor, dsize, dsmooth, 0.0f, PR_STANDARD);  }
  static graphopts_t goInterp2(float smoothcoef, BSDESCALING ds, unsigned int specbckgcolor=0xFFFFFFFF, int dsize=0, float dsmooth=0.0f)
  { return graphopts_t(GT_LINTERPSMOOTH, ds, 0.0f, specbckgcolor, dsize, dsmooth, smoothcoef, PR_STANDARD);  }
  
  static graphopts_t goDots(int dsize=0, float dsmooth=0.0f, unsigned int specbckgcolor=0xFFFFFFFF, BSDESCALING ds=DE_NONE)
  { return graphopts_t(GT_DOTS, ds, 0.0f, specbckgcolor, dsize, dsmooth, 0.0f, PR_STANDARD);  }
  static graphopts_t goDots(BSDESCALING ds, int dsize=0, float dsmooth=0.0f)
  { return graphopts_t(GT_DOTS, ds, 0.0f, 0xFFFFFFFF, dsize, dsmooth, 0.0f, PR_STANDARD);  }
  
  static graphopts_t goHistogram(float opacity=0.0f, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { return graphopts_t(GT_HISTOGRAM, ds, opacity, specbckgcolor, 0, 0.0f, smoothcoef, pr);  }
  static graphopts_t goHistogram(BSPOSTRECT pr, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF)
  { return graphopts_t(GT_HISTOGRAM, ds, 0.0f, specbckgcolor, 0, 0.0f, 0.0f, pr);  }
  
  static graphopts_t goHistogramCrossMax(float opacity=0.0f, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF)
  { return graphopts_t(GT_HISTOGRAM_CROSSMAX, ds, opacity, specbckgcolor, 0, 0.0f, 0.0f, PR_STANDARD);  }
  static graphopts_t goHistogramCrossMax(BSPOSTRECT pr, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF)
  { return graphopts_t(GT_HISTOGRAM_CROSSMAX, ds, 0.0f, specbckgcolor, 0, 0.0f, 0.0f, pr);  }
  
  static graphopts_t goHistogramCrossMin(float opacity=0.0f, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF)
  { return graphopts_t(GT_HISTOGRAM_CROSSMIN, ds, opacity, specbckgcolor, 0, 0.0f, 0.0f, PR_STANDARD);  }
  static graphopts_t goHistogramCrossMin(BSPOSTRECT pr, BSDESCALING ds=DE_NONE, unsigned int specbckgcolor=0xFFFFFFFF)
  { return graphopts_t(GT_HISTOGRAM_CROSSMIN, ds, 0.0f, specbckgcolor, 0, 0.0f, 0.0f, pr);  }
};

inline graphopts_t& operator+=(graphopts_t& go, BSDESCALING ds){ go.descaling = ds; return go; }
inline graphopts_t& operator+=(graphopts_t& go, BSPOSTRECT pr){ go.postrect = pr; return go; }
inline graphopts_t& operator+=(graphopts_t& go, float opacity){ go.opacity = opacity; return go; }
inline graphopts_t& operator+=(graphopts_t& go, unsigned int backgroundcolor){ go.backcolor = backgroundcolor; return go; }

inline graphopts_t operator+(const graphopts_t& go, BSDESCALING ds){ graphopts_t result(go); result.descaling = ds; return result; }
inline graphopts_t operator+(const graphopts_t& go, BSPOSTRECT pr){ graphopts_t result(go); result.postrect = pr; return result; }
inline graphopts_t operator+(const graphopts_t& go, float opacity){ graphopts_t result(go); result.opacity = opacity; return result; }
inline graphopts_t operator+(const graphopts_t& go, unsigned int backgroundcolor){ graphopts_t result(go); result.backcolor = backgroundcolor; return result; }



/// 2D graph
class DrawGraph: public DrawQWidget
{
public:
  enum COLORPOLICY          // IPalette split onto graphs
  { 
    CP_SINGLE,              // Once color, which is colorize_start + #portion*interval[colorize_start...colorize_stop]
    CP_OWNRANGE,            // color, gradiented in palette, according with portions
    CP_OWNRANGE_GROSS,      // color, gradiented in palette, according with portions, nonlinear gradient
    CP_OWNRANGE_SYMMETRIC,  // color, gradiented in palette, according with portions, centre symmetrion
    CP_RANGE,               // color, gradiented in palette, ignoring portions
//    CP_RANGE_GROSS,         // color, gradiented in palette, ignoring portions, nonlinear gradient
    CP_SUBPAINTED           // portions ignored. value point direct onto palette
  };
protected:
  graphopts_t             m_graphopts;
  DrawGraph::COLORPOLICY  m_colorpolicy;
protected:
  void  reConstructor(unsigned int samples);
public:
  DrawGraph(unsigned int samples, unsigned int graphs=1, COLORPOLICY downcolorize=CP_SINGLE, float colorize_start=1.0f, float colorize_stop=0.5f);
  DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize=CP_SINGLE, float colorize_start=1.0f, float colorize_stop=0.5f);
  const graphopts_t&      graphOpts() const { return m_graphopts; }
  COLORPOLICY             colorPolicy() const { return m_colorpolicy; }
protected:
  virtual unsigned int    portionSize()const{  return m_matrixDimmA; }
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_1D; }
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB);
  virtual unsigned int    colorBack() const;
};

/// 2D graph with data append
class DrawGraphMove: public DrawGraph
{
private:
  unsigned int          m_stepSamples;
public:
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs=1, COLORPOLICY downcolorize=CP_SINGLE, float colorize_base=1.0f, float colorize_step=-1.0f);
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize=CP_SINGLE, float colorize_base=1.0f, float colorize_step=-1.0f);
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
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory=0, unsigned int graphs=1, COLORPOLICY downcolorize=CP_SINGLE, float colorize_base=1.0f, float colorize_step=-1.0f);
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize=CP_SINGLE, float colorize_base=1.0f, float colorize_step=-1.0f);
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
  virtual void  clearData();
protected:
          void            fillMatrix();
  virtual void            slideLmHeight(int);
};

#endif // DRAWCOREGRAPH_H
