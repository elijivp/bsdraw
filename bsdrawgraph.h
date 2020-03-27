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
  int             dotsize;                  /// >0 : abs size; <0 : scaling + size
  float           dotsmooth;                /// 0..2 optimal, smooth quality
  float           smooth;
  BSPOSTRECT      postrect;

  // 1. typical linterp graph
  static graphopts_t goInterp(BSDESCALING ds, int dsize=0, float dsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP, ds, 0.0f, dsize, dsmooth, 0.0f, PR_STANDARD }; return result;  }
  static graphopts_t goInterp(float smoothcoef, BSDESCALING ds=DE_NONE, int dsize=0, float dsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP, ds, 0.0f, dsize, dsmooth, smoothcoef, PR_STANDARD}; return result; }
  
  static graphopts_t goInterp2(BSDESCALING ds, int dsize=0, float dsmooth=0.0f)
  { graphopts_t result = { GT_LINTERPSMOOTH, ds, 0.0f, dsize, dsmooth, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goInterp2(float smoothcoef, BSDESCALING ds, int dsize=0, float dsmooth=0.0f)
  { graphopts_t result = { GT_LINTERPSMOOTH, ds, 0.0f, dsize, dsmooth, smoothcoef, PR_STANDARD}; return result; }
  
  static graphopts_t goDots(int dsize=0, float dsmooth=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_DOTS, ds, 0.0f, dsize, dsmooth, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goDots(BSDESCALING ds, int dsize=0, float dsmooth=0.0f)
  { graphopts_t result = { GT_DOTS, ds, 0.0f, dsize, dsmooth, 0.0f, PR_STANDARD}; return result; }
  
  static graphopts_t goHistogram(float opacity=0.0f, BSDESCALING ds=DE_NONE, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { graphopts_t result = { GT_HISTOGRAM, ds, opacity, 0, 0.0f, smoothcoef, pr}; return result; }
  static graphopts_t goHistogram(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramCrossMax(float opacity=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMAX, ds, opacity, 0, 0.0f, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goHistogramCrossMax(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMAX, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramCrossMin(float opacity=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMIN, ds, opacity, 0, 0.0f, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goHistogramCrossMin(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMIN, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
};

inline graphopts_t& operator+=(graphopts_t& go, BSDESCALING ds){ go.descaling = ds; return go; }
inline graphopts_t& operator+=(graphopts_t& go, BSPOSTRECT pr){ go.postrect = pr; return go; }
inline graphopts_t& operator+=(graphopts_t& go, float opacity){ go.opacity = opacity; return go; }
//inline graphopts_t& operator+=(graphopts_t& go, unsigned int backgroundcolor){ go.backcolor = backgroundcolor; return go; }

inline graphopts_t operator+(const graphopts_t& go, BSDESCALING ds){ graphopts_t result(go); result.descaling = ds; return result; }
inline graphopts_t operator+(const graphopts_t& go, BSPOSTRECT pr){ graphopts_t result(go); result.postrect = pr; return result; }
inline graphopts_t operator+(const graphopts_t& go, float opacity){ graphopts_t result(go); result.opacity = opacity; return result; }
//inline graphopts_t operator+(const graphopts_t& go, unsigned int backgroundcolor){ graphopts_t result(go); result.backcolor = backgroundcolor; return result; }



enum BSCOLORPOLICY          // IPalette split onto graphs
{ 
  CP_SINGLE,              // Once color, which is cstart + #portion*interval[cstart...cstop]
  CP_OWNRANGE,            // color, gradiented in palette, according with portions
  CP_OWNRANGE_GROSS,      // color, gradiented in palette, according with portions, nonlinear gradient
  CP_OWNRANGE_SYMMETRIC,  // color, gradiented in palette, according with portions, centre symmetrion
  CP_RANGE,               // color, gradiented in palette, ignoring portions
//    CP_RANGE_GROSS,         // color, gradiented in palette, ignoring portions, nonlinear gradient
  CP_SUBPAINTED           // portions ignored. value point direct onto palette
};

struct  coloropts_t
{
  BSCOLORPOLICY   cpolicy;
  float           cstart;
  float           cstop;
  unsigned int    backcolor;                /// color to use instead of palette color. 0xFFFFFFFF - not use
  
  static coloropts_t copts(){  coloropts_t copts = { CP_SINGLE, 1.0f, 0.5f, 0xFFFFFFFF }; return copts; }
  static coloropts_t copts(unsigned int backColor){  coloropts_t copts = { CP_SINGLE, 1.0f, 0.5f, backColor }; return copts; }
  static coloropts_t copts(BSCOLORPOLICY policy, float colorize_start, float colorize_stop, unsigned int backColor=0xFFFFFFFF)
  { coloropts_t copts = { policy, colorize_start, colorize_stop, backColor}; return copts; }
};


/// 2D graph
class DrawGraph: public DrawQWidget
{
protected:
  DrawGraph(unsigned int samples, unsigned int graphs, unsigned int memForDeploy, const graphopts_t& graphopts, const coloropts_t& coloropts);
  void  reConstructor(unsigned int samples);
public:
  DrawGraph(unsigned int samples, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts());
  DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts());
  const graphopts_t&        graphopts() const;
  const coloropts_t&        coloropts() const;
  
  void    setOpts(const graphopts_t& go);
  void    setOpts(const coloropts_t& co);
  void    setOpts(const graphopts_t& go, const coloropts_t& co);
protected:
  virtual DATADIMMUSAGE   getDataDimmUsage() const { return DDU_1D; }
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
  virtual unsigned int    colorBack() const;
};

/// 2D graph with data append
class DrawGraphMove: public DrawGraph
{
private:
  unsigned int          m_stepSamples;
public:
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts());
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts());
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
  
  bool                  m_resizeExpanding;
public:
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory=0, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true);
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true);
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
  virtual void  clearData();
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
protected:
  virtual void            sizeAndScaleChanged();
protected:
          void            fillMatrix();
  virtual void            slideLmHeight(int);
};

#endif // DRAWCOREGRAPH_H
