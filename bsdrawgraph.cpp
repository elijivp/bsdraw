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
  ~DrawGraph_Sheigen();
  virtual const char*   shaderName() const {  return "GRAPH"; }
  virtual int           portionMeshType() const{  return PMT_FORCE1D; }
  virtual unsigned int  shvertex_pendingSize() const { return VshMainGenerator1D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const { return VshMainGenerator1D()(to); }
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return 2900 + FshMainGenerator::basePendingSize(ovlscount); }
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
      fmg.push("result = colorFon;" SHNL);
      
      bool isDots = graphopts.graphtype == graphopts_t::GT_DOTS;
      bool isLindown = graphopts.graphtype == graphopts_t::GT_LINDOWN || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMAX || graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMIN;
      bool isLinterp = graphopts.graphtype == graphopts_t::GT_LINTERP || graphopts.graphtype == graphopts_t::GT_LINTERPSMOOTH;
      
      if (isLindown)
        fmg.push("vec3 neib = vec3(0.0, ibounds.y, 0.0);" SHNL);
      
      if (isLindown || isLinterp)
        fmg.cfloatvar("specsmooth", graphopts.specsmooth < -0.5f? -0.5f : graphopts.specsmooth);
      
      
      if (isLinterp || graphopts.descaling == graphopts_t::DE_LINTERP || graphopts.dotsize > 0)
      {
        fmg.push( 
                  "vec3 fx = vec3(float(max(relcoords.x*ibounds_noscaled.x, 1) - 1)/ibounds_noscaled.x, relcoords.x, float(min(relcoords.x*ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x);" SHNL
//                  "vec3  fx = vec3(float(max(icoords_noscaled.x, 1) - 1)/ibounds_noscaled.x, relcoords.x, float(min(icoords_noscaled.x, ibounds_noscaled.x-1) + 1)/ibounds_noscaled.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "vec3  ffy_spec = vec3(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]));" SHNL
                    "vec3  ffy = floor(ffy_spec*(ibounds_noscaled.y-1));" SHNL
                    "ivec3 ify = ivec3(ffy);" SHNL
              
                    "ffy_spec = ffy_spec * (ibounds.y - 1);" SHNL
                    "vec3  ify_spec = ivec3(floor(ffy_spec));" SHNL
              );
        if (graphopts.descaling == graphopts_t::DE_LINTERP)
        {
//          fmg.push( 
//                    "vec3 fsteps = vec3(float(imoded.x), (ify_spec[2] - ify_spec[1])/float(iscaling.x), (ify_spec[1] - ify_spec[0])/float(iscaling.x));"
//                    "ffy_spec = mix("
//                      "vec3(ify_spec[1] + fsteps[1]*(fsteps.x-1.0), ify_spec[1] + fsteps[1]*fsteps.x, ify_spec[1] + fsteps[1]*(fsteps.x+1.0)),"
//                      "vec3(ify_spec[1] - fsteps[2], ify_spec[1], ify_spec[1] + fsteps[1]),"
//                      "step(fsteps.x, 0.0) );"
//                    );
          
          
//          fmg.push( 
//                    "float corrector = min(abs(ify_spec[2] - ify_spec[1])/abs(ify_spec[1] - ify_spec[0]), 1.0);"
//                    "corrector = max(corrector, 0.0);"
//                    "vec3 fsteps = vec3(mod(floor(relcoords.x*ibounds.x), float(iscaling.x)), abs(ify_spec[2] - ify_spec[1]), (ify_spec[1] - ify_spec[0])/float(iscaling.x));"
////                    "vec3 fcorr = vec3(ify_spec[1]*(fsteps.x - 1.0)/float(iscaling.x), ify_spec[1]*(fsteps.x)/float(iscaling.x), ify_spec[1]*(fsteps.x + 1.0)/float(iscaling.x));"
//                    "vec3 fcorr = sign(ify_spec[2] - ify_spec[1])*vec3(pow((fsteps.x - 1.0)/float(iscaling.x), corrector), "
//                                      "pow((fsteps.x      )/float(iscaling.x), corrector), "
//                                      "pow((fsteps.x + 1.0)/float(iscaling.x), corrector)"
//                                      ");"
                
//                    "ify_spec = ivec3( mix("
//                      "vec3(ify_spec[1] + fsteps[1]*fcorr[0], ify_spec[1] + fsteps[1]*fcorr[1], ify_spec[1] + fsteps[1]*fcorr[2]),"
//                      "vec3(ify_spec[1] - fsteps[2], ify_spec[1], ify_spec[1] + fsteps[1]*pow(1.0/float(iscaling.x), corrector)),"
//                      "step(fsteps.x, 0.0)"
//                    "));"
//                    );
/*
          fmg.push( 
//                    "float corrector = min(abs(ify_spec[2] - ify_spec[1])/abs(ify_spec[1] - ify_spec[0]), 2.0);"
//                    "corrector = max(corrector, 0.0);"
//                    "float corrector = clamp((ify_spec[2] - ify_spec[1])/(ify_spec[1] - ify_spec[0]), -2.0, 2.0);"
//                    "float corrector = clamp(abs(ify_spec[2] - ify_spec[1] - (ify_spec[1] - ify_spec[0]))/(2.0*(ify_spec[2] - ify_spec[1])), -4, 4);"
                    "float corrector = clamp(float(ify_spec[2] - ify_spec[1] + (ify_spec[1] - ify_spec[0]))/((ify_spec[2] - ify_spec[1]) - (ify_spec[1] - ify_spec[0])), -4.0, 4.0);"
//                    "float corrector = clamp(0.5 + 0.5*(ify_spec[1] - ify_spec[0])/(ify_spec[2] - ify_spec[1]), -4.0, 4.0);"
//                    "float surrector = 1.0 - abs(ify_spec[2] - ify_spec[1])/float(ibounds.y);"
//                    "float surrector = 1.0 - step(ify_spec[2] - ify_spec[1], 5.0)/(ify_spec[0] - ify_spec[1]);"
                      "float surrector = step(iscaling.y*4.0, abs(ify_spec[2] - ify_spec[1]))*step(iscaling.y*4.0, abs(ify_spec[0] - ify_spec[1]))*(step(sign((ify_spec[2] - ify_spec[1])*(ify_spec[0] - ify_spec[1])), 0.0));"
//                    "surrector = mix(1.0, 0.0, surrector);"
                "surrector = 1.0;"
//                    "vec3 fsteps = vec3(mod(floor(relcoords.x*ibounds.x), float(iscaling.x)), ify_spec[2] - ify_spec[1], (ify_spec[1] - ify_spec[0])/float(iscaling.x));"
                    "vec3 fsteps = vec3(float(imoded.x), ify_spec[2] - ify_spec[1], (ify_spec[1] - ify_spec[0])/float(iscaling.x));"
//                    "vec3 fcorr = vec3((fsteps.x - 1.0)/float(iscaling.x)*mix(corrector, 1.0, (fsteps.x - 1.0)/float(iscaling.x)*surrector), "
//                                      "(fsteps.x      )/float(iscaling.x)*mix(corrector, 1.0, (fsteps.x      )/float(iscaling.x)*surrector), "
//                                      "(fsteps.x + 1.0)/float(iscaling.x)*mix(corrector, 1.0, (fsteps.x + 1.0)/float(iscaling.x)*surrector)"
//                                      ");"
                    "vec3 fcorr = vec3((fsteps.x - 1.0)/float(iscaling.x)*mix(1.0, corrector, (1.0 - (fsteps.x - 1.0)/float(iscaling.x))*surrector), "
                                      "(fsteps.x      )/float(iscaling.x)*mix(1.0, corrector, (1.0 - (fsteps.x      )/float(iscaling.x))*surrector), "
                                      "(fsteps.x + 1.0)/float(iscaling.x)*mix(1.0, corrector, (1.0 - (fsteps.x + 1.0)/float(iscaling.x))*surrector)"
                                      ");"
                
                    "ffy_spec = mix("
                      "vec3(ify_spec[1] + fsteps[1]*fcorr[0], ify_spec[1] + fsteps[1]*fcorr[1], ify_spec[1] + fsteps[1]*fcorr[2]),"
//                      "vec3(ify[1] - fsteps[2], ify[1], ify[1] + fsteps[1]*fcorr[1]),"
                      "vec3(ify_spec[1] - fsteps[2], ify_spec[1], ify_spec[1] + fsteps[1]*fcorr[2]),"
//                "vec3(0.0, 0.0, 0.0),"
                      "step(fsteps.x, 0.0) );"
                    );
*/
          fmg.push( "ify_spec = ivec3(ffy_spec);"
                    "ffy = floor(ffy_spec/(iscaling.y));" SHNL
                    "ify = ivec3(ffy);" SHNL
                    );
        } // DE_LINTERP
      }
      else
      {
        fmg.push( 
                  "vec3  fx = vec3(relcoords.x, relcoords.x, relcoords.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "vec3  ffy_spec = vec3(getValue1D(i, fx[0]));" SHNL
                    "ffy_spec.gb = vec2(ffy_spec[0], ffy_spec[0]);" SHNL
                    "vec3  ffy = floor(ffy_spec*(ibounds_noscaled.y-1));" SHNL
                    "ivec3 ify = ivec3(ffy);" SHNL
              
                    "ffy_spec = ffy_spec * (ibounds.y - 1);" SHNL
                    "vec3  ify_spec = ivec3(floor(ffy_spec));" SHNL
              );
      }
      
      if (isDots)
      {
        fmg.push( 
                    "float MIXWELL = step(distance(fcoords_noscaled, vec2(fcoords_noscaled.x, ify[1])), 0.0);" SHNL
                    "ppb_sfp[0] = ppb_sfp[0]*(1.0 - MIXWELL) + MIXWELL;" SHNL
                    "float VALUE = ffy[1];" SHNL
                  );
      }
      else if (isLinterp)
      {
        if (graphopts.graphtype == graphopts_t::GT_LINTERP)
          fmg.push( 
                      "float fsig_prev = sign(ffy[0] - ffy[1]);" SHNL
                      "float fmix_prev = 1.0 - clamp((fcoords_noscaled.y - ffy[1])/(ffy[0]-ffy[1] + fsig_prev*1)/(1.0 + specsmooth), 0.0, 1.0);" SHNL   /// fsig_prev*1 == addit. smooth level
                  
                      "float fsig_next = sign(ffy[2] - ffy[1]);" SHNL
                      "float fmix_next = 1.0 - clamp((fcoords_noscaled.y - ffy[1])/(ffy[2]-ffy[1] + fsig_next*1)/(1.0 + specsmooth), 0.0, 1.0);" SHNL   /// fsig_prev*2 == addit. smooth level
                
//                      "float fmix_eq = 1.0 - abs(icoords_noscaled.y - ffy[1])/(3.0*(1.0 + specsmooth));" SHNL
                      "float fmix_eq = 1.0 - abs(icoords_noscaled.y - ffy[1])/(1.4*(1.0 + specsmooth));" SHNL
//                      "float fmix_eq = 1.0 - abs(icoords_noscaled.y - ffy[1])/(2.0*(1.0 + specsmooth));" SHNL
                
//                      "float fmix_eq = 0.0;"
//                      "fmix_prev = 0.0;"
//                      "fmix_next = 0.0;"
                    );
        else if (graphopts.graphtype == graphopts_t::GT_LINTERPSMOOTH)
          fmg.push( 
                      "float fsig_prev = sign(ffy[0] - ffy[1]);" SHNL
                      "float fmix_prev = smoothstep(ffy[0] + fsig_prev, ffy[1], fcoords_noscaled.y);" SHNL
                      "float fsig_next= sign(ffy[2] - ffy[1]);" SHNL
                      "float fmix_next = smoothstep(ffy[2] + fsig_next, ffy[1], fcoords_noscaled.y);" SHNL
                      
                      "float fmix_eq = 1.0 - abs(fcoords_noscaled.y - ffy[1])/(3.0*(1.0 + specsmooth));"
                    );
        
        
        fmg.push( 
                  "float fhit = sign(fcoords_noscaled.y - ffy[1]);" SHNL
                  "fmix_prev = fmix_prev*fsig_prev*fhit;" SHNL
                  "fmix_next = fmix_next*fsig_next*fhit;" SHNL
              
                  "fhit = 1.0 - abs(fhit);" SHNL
//                  "float  MIXWELL = max(fhit, specopc*max(fmix_prev, fmix_next));" SHNL
                  "float  MIXWELL = max(fhit, specopc*max(max(fmix_prev, fmix_next), fmix_eq ));" SHNL
//                  "int    IVALUE = int(mix(mix(ffy[0], ffy[1], fmix_prev)*(1.0-step(fmix_prev,fmix_next)) + mix(ffy[2], ffy[1], fmix_next)*(1.0 - step(fmix_next,fmix_prev)), ffy[1], fhit));" SHNL
                  "float  VALUE = floor(mix(mix(ffy[0], ffy[1], fmix_prev)*step(fmix_next,fmix_prev) + mix(ffy[2], ffy[1], fmix_next)*(1.0 - step(fmix_next,fmix_prev)), ffy[1], fhit))/iscaling.y;" SHNL
                  "ppb_sfp[0] = ppb_sfp[0]*(1.0 - sign(MIXWELL)) + MIXWELL;" SHNL
                 );
      }
      else if (isLindown)
      {
        fmg.push(                 
                  "float bmix = sign(ify[1] - icoords_noscaled.y);" SHNL
                  "vec3 delta3 = vec3("
                                      "0.0*step(bmix, -1.0)*specopc*(1.0 - clamp((icoords_noscaled.y-ify[1])/(1.0 + specsmooth*10.0), 0.0, 1.0)), "
                                      "step(1.0, bmix)*specopc, "
                                      "1.0 - abs(bmix));" SHNL

                  "float MIXWELL = delta3[0] + delta3[1] + delta3[2];" SHNL
                  "bmix = step(0.0, bmix) + sign(delta3[0]);" SHNL
          
                  "float fneiprec = ify_spec[1];"
                  "neib = vec3(  mix(neib[0], fneiprec, step(neib[0], fneiprec)*(1.0-bmix)), "
                                "mix(neib[1], fneiprec, step(fneiprec, neib[1])*bmix), "
                                "max(neib[2], fneiprec) );" SHNL
                  );
        
        if (graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMAX)
          fmg.push( "MIXWELL = MIXWELL*step(neib[2], fneiprec);" SHNL
                    "bmix = bmix*step(neib[2], fneiprec);"
                    );
        else if (graphopts.graphtype == graphopts_t::GT_LINDOWN_CROSSMIN)
          fmg.push( "MIXWELL = MIXWELL*step(floor(fneiprec), neib[1])*step(neib[0], fneiprec);" SHNL
                    "bmix = bmix*step(floor(fneiprec), neib[1])*step(neib[0], fneiprec);"
                    );
        
        fmg.push( 
                  "float VALUE = ify[1];" SHNL
                  "ppb_sfp[0] = ppb_sfp[0]*(1.0 - bmix) + bmix;" SHNL
                  "ppb_rect.ga = int(1.0 - bmix)*ppb_rect.ga + int(bmix)*ivec2(icoords.y, ify[1]*iscaling.y + iscaling.y - 1);" SHNL
                  );
      }
      
      
      {
        const char*   descaling[] = { "", "", // DE_NONE and DE_LINTERP
                                      "MIXWELL = MIXWELL * (1.0 - abs(sign( imoded.x - iscaling.x/2)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs(int( imoded.x - iscaling.x/2) / (iscaling.x/2.0)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs(int( imoded.x - iscaling.x/2) / (iscaling.x/4.0)));",
                                      "MIXWELL = MIXWELL * (1.0/abs(int( imoded.x - iscaling.x/2)));"
                                    };
        if ((unsigned int)graphopts.descaling < sizeof(descaling)/sizeof(const char*))
          fmg.push(descaling[int(graphopts.descaling)]);
      }
    
      if (graphopts.dotsize)
      {       
        fmg.push( "{"
                    "float hscx = floor(iscaling.x/2.0 + 0.49);");
        if (graphopts.dotsize > 0)
          fmg.cintvar("dotsize", graphopts.dotsize);
        else
        {
          fmg.cintvar("godot", -graphopts.dotsize);
          fmg.push("int dotsize = int(hscx) + godot;");
        }
        fmg.push("int dist_limit = int(max((dotsize-1) - hscx + 1, 0.0));");
        fmg.cfloatvar("dotweight", graphopts.dotweight);
        
        if (graphopts.descaling == graphopts_t::DE_LINTERP)
          fmg.push("float remoded_x = float(relcoords.x*ibounds.x - imoded.x)/fbounds.x*ibounds_noscaled.x + (1.0 - step(float(imoded.x), hscx));"
                   "vec2  remodedoffs = vec2(hscx, 0.0);");
        else
          fmg.push("float remoded_x = float(relcoords.x*ibounds.x - imoded.x)/fbounds.x*ibounds_noscaled.x;"
                   "vec2  remodedoffs = vec2(0.0, 0.0);");
        
        
        fmg.push(   "for (int j=-dist_limit; j<=dist_limit; j++)" SHNL
                    "{" SHNL
                      "float reltoval = float(remoded_x + j)/ibounds_noscaled.x;" SHNL
                      "float fdist_weight = (dotsize - distance(fcoords + remodedoffs, "
                        "vec2( floor(reltoval*fbounds.x), floor(getValue1D(i, reltoval)*(ibounds.y-1))) + vec2(hscx, 0.0))"
                                            ") / float(dotsize);" SHNL                    
//                  "VALUE = floor(mix(nei[1], VALUE, step(fdist_weight, MIXWELL)));" SHNL
//                    "MIXWELL = mix(min(fdist_weight*(1 + dotweight), 1.0), MIXWELL, step(fdist_weight, MIXWELL));" SHNL
                    "fdist_weight = min(fdist_weight*(1.0 + dotweight), 1.0);" SHNL
                    "MIXWELL = mix(fdist_weight, MIXWELL, step(fdist_weight, MIXWELL));" SHNL
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
      
//      if (isLindown)
//        fmg.push(   "MIXWELL = 1.0 - step(MIXWELL,0.0);" SHNL );

      fmg.push(   "result = mix(result, colorGraph, MIXWELL);" SHNL
                  "ovMix = max(ovMix, MIXWELL*relcoords.y);" SHNL
                "}" SHNL // for
            );
    }
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};


DrawGraph_Sheigen::~DrawGraph_Sheigen()
{
}


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
    m_stopped = int(((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixDimmA));
  fillMatrix();
  DrawQWidget::vmanUpData();
}
