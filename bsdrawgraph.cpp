#include "bsdrawgraph.h"

#include "core/sheigen/bsshgenmain.h"

#ifndef SHNL
#define SHNL "\n"
#endif

class DrawGraph_Sheigen: public ISheiGenerator
{
  graphopts_t graphopts;
  float       colorizer[2];
  DrawGraph::DOWNCOLORIZE dc;
public:
  DrawGraph_Sheigen(const graphopts_t& kgo, float czofs, float czstep, DrawGraph::DOWNCOLORIZE downcolorise): graphopts(kgo), dc(downcolorise)
  {
    if (czofs > 1)  czofs = 1;
    else if (czofs < 0) czofs = 0;
    colorizer[0] = czofs; colorizer[1] = czstep;
  }
  virtual const char*   shaderName() const {  return "GRAPH"; }
  virtual int           portionMeshType() const{  return PMT_FORCE1D; }
  virtual unsigned int  shvertex_pendingSize() const { return VshMainGenerator1D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const { return VshMainGenerator1D()(to); }
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return 1800 + FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(const DPostmask& fsp, bool rotated, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, rotated, ovlscount, ovlsinfo);
    float specopc = graphopts.specopc > 1.0f? 1.0f : graphopts.specopc < 0.0f? 0.0f : graphopts.specopc;
    fmg.cfloatvar("specopc", 1.0f - specopc);
    fmg.goto_func_begin(fsp);
    {
      if (graphopts.backcolor != 0xFFFFFFFF)
        fmg.ccolor("colorFon", graphopts.backcolor);
      else
        fmg.push("vec3 colorFon = texture(texPalette, vec2(0.0, 0.0)).rgb;" SHNL);
      
      fmg.push("result = vec3(0.0, 0.0, 0.0);" SHNL);
      fmg.push("float fonmix = 0.0;" SHNL);
      if (graphopts.graphtype == graphopts_t::GT_LINDOWN || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMAX || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMIN)
        fmg.push("vec3 neib = vec3(0.0, ibounds.y, 0.0);" SHNL);
      
      if (graphopts.graphtype == graphopts_t::GT_DOTS)
      {
        fmg.push( "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "float VALUE = floor(getValue1D(i, fcoords.x)*(ibounds_noscaled.y-1));" SHNL
                    "float MIXWELL = step(distance(vec2(icoords), vec2(icoords.x, VALUE)), 0.0);" SHNL
                    "ppb_sfp[0] = ppb_sfp[0]*(1.0 - MIXWELL) + MIXWELL;" SHNL
                  );
      }
      else if (graphopts.graphtype == graphopts_t::GT_LINTERP || graphopts.graphtype == graphopts_t::GT_LINTERPSMOOTH)
      {
        fmg.push( "vec3  fx_around = vec3(float(max(fcoords.x * ibounds_noscaled.x,1) - 1) / ibounds_noscaled.x, fcoords.x, float(min(fcoords.x * ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL         
                    "ivec3 ify_around = ivec3(vec3(getValue1D(i, fx_around[0]), getValue1D(i, fx_around[1]), getValue1D(i, fx_around[2])) * (ibounds_noscaled.y - 1));" SHNL
                    "float fhit = sign(icoords.y - ify_around[1]);" SHNL
                  );
        if (graphopts.graphtype == graphopts_t::GT_LINTERP)
          fmg.push( "float fmix_prev = clamp(float(icoords.y - ify_around[0])/(ify_around[1] - ify_around[0]), 0.0, 1.0);" SHNL
                    "fmix_prev = fmix_prev*(1 - step(float(ify_around[1]), float(ify_around[0]))) * (1.0 - step(0.0, fhit))   +   fmix_prev*(1 - step(float(ify_around[0]), float(ify_around[1]))) * (1.0 - step(fhit, 0.0));" SHNL
                    "float fmix_next = 1.0 - clamp(float(icoords.y - ify_around[1])/(ify_around[2] - ify_around[1]), 0.0, 1.0);" SHNL
                    "fmix_next = fmix_next*(1 - step(float(ify_around[1]), float(ify_around[2]))) * (1.0 - step(0.0, fhit))   +   fmix_next*(1 - step(float(ify_around[2]), float(ify_around[1]))) * (1.0 - step(fhit, 0.0));" SHNL
                    );
        else if (graphopts.graphtype == graphopts_t::GT_LINTERPSMOOTH)
          fmg.push( "float fmix_prev = smoothstep(float(ify_around[0]), float(ify_around[1]), float(icoords.y));" SHNL
                    "fmix_prev = fmix_prev*(1 - step(float(ify_around[1]), float(ify_around[0]))) * (1.0 - step(0.0, fhit))   +   fmix_prev*(1 - step(float(ify_around[0]), float(ify_around[1]))) * (1.0 - step(fhit, 0.0));" SHNL
                    "float fmix_next = smoothstep(float(ify_around[2]), float(ify_around[1]), float(icoords.y));" SHNL
                    "fmix_next = fmix_next*(1 - step(float(ify_around[1]), float(ify_around[2]))) * (1.0 - step(0.0, fhit))   +   fmix_next*(1 - step(float(ify_around[2]), float(ify_around[1]))) * (1.0 - step(fhit, 0.0));" SHNL
                    );
        
        fmg.push( "fhit = 1.0 - abs(fhit);" SHNL
                  "float  MIXWELL = max(fhit, specopc*max(fmix_prev, fmix_next));" SHNL
//                  "int    IVALUE = int(mix(mix(ify_around[0], ify_around[1], fmix_prev)*(1.0-step(fmix_prev,fmix_next)) + mix(ify_around[2], ify_around[1], fmix_next)*(1.0 - step(fmix_next,fmix_prev)), ify_around[1], fhit));" SHNL
                  "float  VALUE = floor(mix(mix(ify_around[0], ify_around[1], fmix_prev)*step(fmix_next,fmix_prev) + mix(ify_around[2], ify_around[1], fmix_next)*(1.0 - step(fmix_next,fmix_prev)), ify_around[1], fhit));" SHNL
                  "ppb_sfp[0] = ppb_sfp[0]*(1.0 - sign(MIXWELL)) + sign(MIXWELL);" SHNL
                 );
      }
      else if (graphopts.graphtype == graphopts_t::GT_LINDOWN || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMAX || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMIN)
      {
        fmg.push(   "for (int i=0; i<countPortions; i++)" SHNL
                    "{" SHNL
                      "float VALUE = getValue1D(i, fcoords.x)*(ibounds_noscaled.y-1);" SHNL
                      "float bmix = step(float(icoords.y), floor(VALUE));" SHNL
                      "neib = vec3( mix(neib[0], VALUE, step(neib[0], VALUE)*(1.0-bmix)), mix(neib[1], VALUE, step(VALUE, neib[1])*bmix), max(neib[2], VALUE) );" SHNL
                      "float MIXWELL = mix(specopc, bmix, step(floor(VALUE), float(icoords.y)));" SHNL
                    );
        
        if (graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMAX)
          fmg.push( "MIXWELL = MIXWELL*step(neib[2], VALUE);" SHNL
                    "bmix = sign(MIXWELL);"
                    "fonmix = fonmix*(1.0-bmix);"
                    );
        
        else if (graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMIN)
          fmg.push( "MIXWELL = MIXWELL*step(floor(VALUE), neib[1])*step(neib[0], VALUE);" SHNL
                    "bmix = sign(MIXWELL);"
                    );
        
        fmg.push( "VALUE = floor(VALUE);" SHNL
                  "ppb_sfp[0] = ppb_sfp[0]*(1.0 - bmix) + bmix;" SHNL
                  "ppb_rect.ga = ivec2(mix(ppb_rect.ga, ivec2(floor(fcoords.y*ibounds.y + 0.49), floor(VALUE*scaling_vert) + scaling_vert-1), bmix));" SHNL
                  );
      }
      
      
      if (graphopts.descaling != graphopts_t::DE_NONE)
      {
        const char*   descaling[] = { "",
                                      "MIXWELL = MIXWELL * (1.0 - abs(sign( int(mod(floor(fcoords.x*ibounds.x + 0.49), scaling_horz)) - scaling_horz/2)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs((int(mod( floor(fcoords.x*ibounds.x + 0.49), scaling_horz)) - scaling_horz/2) / (scaling_horz/2.0)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs((int(mod( floor(fcoords.x*ibounds.x + 0.49), scaling_horz)) - scaling_horz/2) / (scaling_horz/4.0)));",
                                      "MIXWELL = MIXWELL * (1.0/abs((int(mod( floor(fcoords.x*ibounds.x + 0.49), scaling_horz)) - scaling_horz/2)));"
                                    };
        if ((unsigned int)graphopts.descaling < sizeof(descaling)/sizeof(const char*))
          fmg.push(descaling[int(graphopts.descaling)]);
      }
    
      if (graphopts.dotsize > 0)
      {
        fmg.push( "{");
        fmg.cintvar("dist_limit", graphopts.dotsize + 1);
        fmg.cfloatvar("dotweight", graphopts.dotweight > 1.0f? 1.0f : graphopts.dotweight);
        if (graphopts.graphtype == graphopts_t::GT_DOTS)
          fmg.cfloatvar("dist_opc", 1.0f - specopc);
        else
          fmg.cfloatvar("dist_opc", 1.0f);
        fmg.push(   "for (int j=-dist_limit; j<=dist_limit; j++)" SHNL
                    "{" SHNL
                      "float CVALUE = floor(getValue1D(i, float(fcoords.x*ibounds_noscaled.x + j) / ibounds_noscaled.x) * (ibounds_noscaled.y - 1));" SHNL
                      "float fdist_weight = dist_opc*(dist_limit - distance(vec2(icoords), vec2(icoords.x + j, CVALUE))) / float(dist_limit);" SHNL
                      "VALUE = floor(mix(CVALUE, VALUE, step(fdist_weight, MIXWELL)));" SHNL
//                      "MIXWELL = max(MIXWELL, fdist_weight);" SHNL
                      "MIXWELL = mix(mix(fdist_weight, fdist_weight*2.0, step(0.4*dotweight, fdist_weight)), MIXWELL, step(fdist_weight, MIXWELL));" SHNL
//                      "MIXWELL = max(MIXWELL, mix(fdist_weight, 1.0, step(fdist_weight, dotweight)));" SHNL
                    "}" SHNL
                  "}" SHNL);
      }
      
      
      {
        float base = colorizer[0];
        float step = colorizer[1];
        if (step < 0) step = step*base;
        else          step = step*(1-base);
        
        fmg.cfloatvar("pcBase", base);
        fmg.cfloatvar("pcStep", step);
      }
      if (dc == DrawGraph::DC_OFF)            fmg.push("float portionColor = pcBase +  pcStep*(1.0 - float(countPortions-i)/float(countPortions));" SHNL);
      else if (dc == DrawGraph::DC_DOWNBASE)  fmg.push("float portionColor = (pcBase +  pcStep*(1.0 - float(countPortions-i)/float(countPortions)))*VALUE/ibounds_noscaled.y;" SHNL);
      else if (dc == DrawGraph::DC_DOWNNEXT)  fmg.push("float portionColor = pcBase +  pcStep*(1.0 - float(countPortions-i)/float(countPortions))*VALUE/ibounds_noscaled.y;" SHNL);
      
      fmg.push(   "vec3  colorGraph = texture(texPalette, vec2(clamp(portionColor, 0.0, 1.0), 0.0)).rgb;" SHNL );
      fmg.push(   
                  "fonmix = mix(fonmix, MIXWELL, 1.0 - step(MIXWELL,0.0));" SHNL
                  "MIXWELL = 1.0 - step(MIXWELL,0.0);" SHNL
                  "result = mix(result, colorGraph, MIXWELL);" SHNL
                  "ovMix = max(ovMix, MIXWELL*fcoords.y);" SHNL
                "}" SHNL // for
                "result = mix(colorFon, result, fonmix);" SHNL
      );
    }
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};


void DrawGraph::reConstructor(unsigned int samples)
{
  m_matrixDimmA = samples;
  m_matrixDimmB = 1;
  m_portionSize = samples;
  deployMemory();
  if (m_matrixSwitchAB)
    m_scalingWidth = 1;
  else
    m_scalingHeight = 1;
}

/// m_countPortions === graphs
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawQWidget(new DrawGraph_Sheigen(graphopts_t(), colorize_base, colorize_step, downcolorize), graphs, OR_LRBT){ reConstructor(samples); }
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawQWidget(new DrawGraph_Sheigen(graphopts, colorize_base, colorize_step, downcolorize), graphs, OR_LRBT){ reConstructor(samples); }

void DrawGraph::resizeGL(int w, int h)
{
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  
  unsigned int& scalingA = m_matrixSwitchAB? m_scalingHeight : m_scalingWidth;
  unsigned int& scalingB = m_matrixSwitchAB? m_scalingWidth : m_scalingHeight;
  int& sizeA = m_matrixSwitchAB? h : w;
  int& sizeB = m_matrixSwitchAB? w : h;
  
  scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  clampScaling();
  m_matrixDimmB = sizeB / scalingB;
  if (m_matrixDimmB == 0) m_matrixDimmB = 1;
  pendResize(true);
}

////////////////////////////////////////////////////////////////

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples)
{
}

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, graphopts, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples)
{
}

void DrawGraphMove::setData(const float *data)
{
  for (unsigned int p=0; p<m_countPortions; p++)
  {
    unsigned int poffs = p*m_portionSize;
    for (unsigned int i=0; i<m_portionSize - m_stepSamples; i++)
      m_matrixData[poffs + i] = m_matrixData[poffs + i + m_stepSamples];
    for (unsigned int i=0; i<m_stepSamples; i++)
      m_matrixData[poffs + m_portionSize - m_stepSamples + i] = data[p*m_stepSamples + i];
  }
  DrawGraph::vmanUpData();
}

void DrawGraphMove::setData(const float* data, DataDecimator* decim)
{
  for (unsigned int p=0; p<m_countPortions; p++)
  {
    unsigned int poffs = p*m_portionSize;
    for (unsigned int i=0; i<m_portionSize - m_stepSamples; i++)
      m_matrixData[poffs + i] = m_matrixData[poffs + i + m_stepSamples];
    for (unsigned int i=0; i<m_stepSamples; i++)
      m_matrixData[poffs + m_portionSize - m_stepSamples + i] = decim->decimate(data, m_stepSamples, i, p);
  }
  DrawGraph::vmanUpData();
}

////////////////////////////////////////////////////////////////

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples), m_stopped(0), m_memory(graphs, samples, extmemory)
{
  if (extmemory)
    m_matrixLmSize = samples + extmemory;
}

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, graphopts_t graphopts, DOWNCOLORIZE downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, graphopts, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples), m_stopped(0), m_memory(graphs, samples, extmemory)
{
  if (extmemory)
    m_matrixLmSize = samples + extmemory;
}

void DrawGraphMoveEx::setData(const float *data)
{
  m_memory.onSetData(data, m_stepSamples);
  
  if (m_sbStatic && m_stopped != 0) m_stopped+=m_countPortions*m_stepSamples;
  fillMatrix();
  
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::setData(const float* data, DataDecimator* decim)
{
  m_memory.onSetData(data, m_stepSamples, decim);
  
  if (m_sbStatic && m_stopped != 0) m_stopped+=m_countPortions*m_stepSamples;
  fillMatrix();
  
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::clearData()
{
  m_memory.onClearData();
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::fillMatrix()
{
  m_memory.onFillData(m_stopped, m_matrixData, 0);
}

void DrawGraphMoveEx::slideLmHeight(int pp)
{
  if (m_matrixLmSize < m_matrixDimmA)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixDimmA);
  fillMatrix();
  DrawQWidget::vmanUpData();
}

