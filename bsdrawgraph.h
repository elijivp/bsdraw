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
  int                   m_stopped;
  MemExpand1D           m_memory;
  
  bool                  m_resizeExpanding;
public:
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory=0, unsigned int graphs=1, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true, SPLITPORTIONS splitGraphs=SL_NONE);
  DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts=coloropts_t::copts(), bool resizeExpanding=true, SPLITPORTIONS splitGraphs=SL_NONE);
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
public:
  virtual int             scrollValue() const;
protected slots:
  virtual void            scrollDataTo(int);
};

#endif // DRAWCOREGRAPH_H
