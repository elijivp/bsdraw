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
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return 1600 + FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(const DPostmask& fsp, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, ovlscount, ovlsinfo);
    float specopc = graphopts.specopc > 1.0f? 1.0f : graphopts.specopc < 0.0f? 0.0f : graphopts.specopc;
    fmg.cfloatvar("specopc", specopc);
    fmg.goto_func_begin(fsp);
    {
      if (graphopts.backcolor != 0xFFFFFFFF)
      {
        fmg.ccolor("colorFon", graphopts.backcolor);
      }
      else
        fmg.push("vec3 colorFon = texture(texPalette, vec2(0.0, 0.0)).rgb;" SHNL);
      
      fmg.push("result = vec3(0.0, 0.0, 0.0);" SHNL);
      fmg.push("float resultmix = 0.0;" SHNL);
      fmg.push("int   resultvalue = 0;" SHNL);
      
      const char shgraph_formixwell_dots[] = 
                                        "for (int i=0; i<countPortions; i++)" SHNL
                                        "{" SHNL
                                          "int   IVALUE = int(getValue1D(i, fcoords.x)*(ibounds_noscaled.y - 1));" SHNL
                                          "float MIXWELL = step(distance(vec2(icoords), vec2(icoords.x, IVALUE)), 0.0);" SHNL
          
                                          "ppb_sfp[0] = ppb_sfp[0]*(1.0 - MIXWELL) + MIXWELL;" SHNL;
          
      const char shgraph_formixwell_linterp[] = 
                                          "vec3  fx_around = vec3(float(max(fcoords.x * ibounds_noscaled.x,1) - 1) / ibounds_noscaled.x, fcoords.x, float(min(fcoords.x * ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x);" SHNL
                                          "for (int i=0; i<countPortions; i++)" SHNL
                                          "{" SHNL
                                            "ivec3 ify_around = ivec3(vec3(getValue1D(i, fx_around[0]), getValue1D(i, fx_around[1]), getValue1D(i, fx_around[2])) * (ibounds_noscaled.y - 1));" SHNL
                                            "float fmix_prev = (1 - step(0.5, float(abs(icoords.y - ify_around[1]))));" SHNL
                                            "fmix_prev += (1.0 - specopc)*clamp(float(icoords.y - ify_around[0])/(ify_around[1] - ify_around[0]), 0.0, 1.0) * (1 - step(float(ify_around[1]), float(ify_around[0]))) * (1 - step(float(ify_around[1]), float(icoords.y)));" SHNL
                                            "fmix_prev += (1.0 - specopc)*clamp(float(ify_around[0] - icoords.y)/(ify_around[0] - ify_around[1]), 0.0, 1.0) * (1 - step(float(ify_around[0]), float(ify_around[1]))) * step(float(ify_around[1]), float(icoords.y));" SHNL
                                            "float fmix_next = (1 - step(0.5, float(abs(ify_around[1] - icoords.y))));" SHNL
                                            "fmix_next += (1.0 - specopc)*(1 - clamp(float(icoords.y - ify_around[1])/(ify_around[2] - ify_around[1]), 0.0, 1.0)) * (1 - step(float(ify_around[1]), float(ify_around[2]))) * (1 - step(float(ify_around[1]), float(icoords.y)));" SHNL
                                            "fmix_next += (1.0 - specopc)*(1 - clamp(float(ify_around[1] - icoords.y)/(ify_around[1] - ify_around[2]), 0.0, 1.0)) * (1 - step(float(ify_around[2]), float(ify_around[1]))) * (step(float(ify_around[1]), float(icoords.y)));" SHNL
                                            
                                            "int    IVALUE = ify_around[1];" SHNL
                                            "float  MIXWELL = clamp(fmix_prev + fmix_next,0.0,1.0);" SHNL
          
                                            "ppb_sfp[0] = ppb_sfp[0]*(1.0 - sign(MIXWELL)) + sign(MIXWELL);" SHNL;
      
      const char shgraph_formixwell_lindown[] = 
                                            "for (int i=0; i<countPortions; i++)" SHNL
                                            "{" SHNL
                                              "int IVALUE = int(getValue1D(i, fcoords.x) * (ibounds_noscaled.y - 1));" SHNL
                                              "float bmix = step(float(icoords.y), float(IVALUE));" SHNL
                                              "float MIXWELL = clamp(mix(1.0 - specopc, bmix, step(float(IVALUE), float(icoords.y))), 0.0, 1.0);" SHNL
          
                                              "ppb_sfp[0] = ppb_sfp[0]*(1.0 - bmix) + bmix;" SHNL
                                              "ppb_rect.ga = ivec2(mix(ppb_rect.ga, ivec2(floor(fcoords.y*ibounds.y + 0.49), IVALUE*chnl_vert_scaling + chnl_vert_scaling-1), bmix));" SHNL;
          
      const char*   lfmixes[] = { shgraph_formixwell_dots, shgraph_formixwell_linterp, shgraph_formixwell_lindown, shgraph_formixwell_lindown };
      
      fmg.push(lfmixes[(int)graphopts.graphtype]);
    
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
                      "int CVALUE = int(getValue1D(i, float(fcoords.x*ibounds_noscaled.x + j) / ibounds_noscaled.x) * (ibounds_noscaled.y - 1));" SHNL
                      "float fdist_weight = dist_opc*(dist_limit - distance(vec2(icoords), vec2(icoords.x + j, CVALUE))) / float(dist_limit);" SHNL
                      "IVALUE = int(mix(CVALUE, IVALUE, step(fdist_weight, MIXWELL)));" SHNL
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
      else if (dc == DrawGraph::DC_DOWNBASE)  fmg.push("float portionColor = (pcBase +  pcStep*(1.0 - float(countPortions-i)/float(countPortions)))*float(IVALUE)/ibounds_noscaled.y;" SHNL);
      else if (dc == DrawGraph::DC_DOWNNEXT)  fmg.push("float portionColor = pcBase +  pcStep*(1.0 - float(countPortions-i)/float(countPortions)) *float(IVALUE)/ibounds_noscaled.y;" SHNL);
      
      fmg.push( "vec3  colorGraph = texture(texPalette, vec2(clamp(portionColor, 0.0, 1.0), 0.0)).rgb;" SHNL );
      
      if (graphopts.graphtype == graphopts_t::GT_LINDOWNCROSS)
        fmg.push( "result = mix(result, colorGraph, (1-step(MIXWELL,0.0)) * step(float(resultvalue), float(IVALUE)));" SHNL );
      else
        fmg.push( "result = mix(result, colorGraph, (1-step(MIXWELL,0)));" SHNL );

      fmg.push(   
                  "resultmix = max(resultmix, MIXWELL);" SHNL
                  "resultvalue = max(resultvalue, IVALUE);" SHNL
//                  "ovMix = mix(ovMix, MIXWELL, 1 - step(MIXWELL, 0.0));" SHNL //"ovMix = ovMix + MIXWELL;"
                  "ovMix = max(ovMix, (1.0 - step(MIXWELL, 0.0))*fcoords.y);"
                "}" SHNL); // for)
      
      fmg.push(   "result = mix(colorFon, result, resultmix);" SHNL );
    }
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};


void DrawGraph::reConstructor(unsigned int samples)
{
  m_matrixWidth = samples;
  m_matrixHeight = 1;
  m_portionSize = samples;
  deployMemory();
  m_matrixScHeight = 1;
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
  m_matrixScWidth = (unsigned int)w <= m_matrixWidth? 1 : (w / m_matrixWidth);
  clampScaling();
  m_matrixHeight = h / m_matrixScHeight;
  if (m_matrixHeight == 0) m_matrixHeight = 1;
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
  if (m_matrixLmSize < m_matrixWidth)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixWidth);
  fillMatrix();
  DrawQWidget::vmanUpData();
}

