#ifndef DRAWCOREGRAPH_H
#define DRAWCOREGRAPH_H

/// DrawGraph is a graph for standart 1D functions 
/// Input: 1D array
/// Scaling: vertical scaling is 1 by default, so You can easily resize Draw
/// 
/// Example:
/// #include "bsdrawgraph.h"
/// #include "palettes/bspalettes_std.h"
/// 
/// DrawGraph* draw = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp());
/// draw->setDataPalette(&paletteBkGrWh);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov


#include "core/bsqdraw.h"
#include "core/bsgraphopts.h"

/// 2D graph
class DrawGraph: public DrawQWidget
{
protected:
  DrawGraph(unsigned int samples, unsigned int graphs, unsigned int memForDeploy, const graphopts_t& graphopts, const coloropts_t& coloropts, SPLITPORTIONS splitGraphs=SL_NONE);
  void  reConstructor(unsigned int samples);
public:
  DrawGraph(unsigned int samples, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts(), SPLITPORTIONS splitGraphs=SL_NONE);
  DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts(), SPLITPORTIONS splitGraphs=SL_NONE);
  const graphopts_t&        graphopts() const;
  const coloropts_t&        coloropts() const;
  
  void    setOpts(const graphopts_t& go);
  void    setOpts(const coloropts_t& co);
  void    setOpts(const graphopts_t& go, const coloropts_t& co);
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
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts(), SPLITPORTIONS splitGraphs=SL_NONE);
  DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts(), SPLITPORTIONS splitGraphs=SL_NONE);
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
};

/// 2D graph with saved data for ScrollBar connecting
class DrawGraphMoveEx: public DrawGraph
{
private:
  unsigned int          m_stepSamples;
  int                   m_filloffset;
  MemExpand1D           m_memory;
  
  bool                  m_resizeExpanding;
public:
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory=0, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true, SPLITPORTIONS splitGraphs=SL_NONE);
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true, SPLITPORTIONS splitGraphs=SL_NONE);
  
//  void  rescaleA(int v);
  
  unsigned int  stepSamples() const { return m_stepSamples; }
  unsigned int  memfloatsFilled() const { return m_memory.filled(); }  // in floats
  unsigned int  memfloatsNonfilled() const { return m_memory.nonfilled(); }  // in floats
  unsigned int  memfloatsTotal() const { return m_memory.total(); }  // in floats
  
  void  viewDiap(unsigned int* from, unsigned int* to);  
  void  memoryPreClear() { m_memory.onClearData();  m_filloffset = 0; }
  void  memoryRangeSize(int mem);
  
  float*        getMemfloats() { return m_memory.rawData(); }
  void          setMemfloatsUpdate();
public:
  virtual void  setData(const float* data);
  virtual void  setData(const float* data, DataDecimator* dcim);
  virtual void  clearData();
public:
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
protected:
  virtual int             sizeAndScaleChanged(bool changedDimmA, bool changedDimmB);
protected:
          void            fillMatrix();
          void            clampFilloffset();
public:
  virtual int             scrollValue() const;
public slots:
  virtual void            scrollDataTo(int);
  virtual void            scrollRelativeTo(int);
};

#endif // DRAWCOREGRAPH_H
