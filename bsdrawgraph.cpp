#include "bsdrawgraph.h" 

#include <QResizeEvent>

#include "core/sheigen/bsshgenmain.h"

#ifndef SHNL
#define SHNL "\n"
#endif

class DrawGraph_Sheigen: public ISheiGenerator
{
  graphopts_t             graphopts;
  float                   colorizer[2];
  DrawGraph::COLORPOLICY  dc;
public:
  DrawGraph_Sheigen(const graphopts_t& kgo, float czstart, float czstop, DrawGraph::COLORPOLICY downcolorise): graphopts(kgo), dc(downcolorise)
  {
//    if (czstart > 1)  czstart = 1;
//    else if (czstart < 0) czstart = 0;
    colorizer[0] = czstart; colorizer[1] = czstop;
    for (int i=0; i<2; i++)
    {
      if (colorizer[i] > 1.0f) colorizer[i] = 1.0f;
      else if (colorizer[i] < 0) colorizer[i] = 0;
    }
  }
  ~DrawGraph_Sheigen();
public:
  const graphopts_t&      graphOpts() const { return graphopts; }
  DrawGraph::COLORPOLICY  colorPolicy() const { return dc; }
public:
  virtual const char*   shaderName() const {  return "GRAPH"; }
  virtual int           portionMeshType() const{  return PMT_FORCE1D; }
  virtual unsigned int  shvertex_pendingSize() const { return VshMainGenerator1D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const { return VshMainGenerator1D()(to); }
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return 4000 + FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, ORIENTATION orient, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, orient, ovlscount, ovlsinfo);
    float opacity = graphopts.opacity > 1.0f? 1.0f : graphopts.opacity < 0.0f? 0.0f : graphopts.opacity;
    fmg.cfloatvar("specopc", 1.0f - opacity);
    fmg.goto_func_begin(graphopts.backcolor != 0xFFFFFFFF? FshMainGenerator::INIT_BYVALUE:
                                                           FshMainGenerator::INIT_BYPALETTE, 
                        graphopts.backcolor, fsp);
    {
      bool isDots = graphopts.graphtype == GT_DOTS;
      bool isHistogram = graphopts.graphtype == GT_HISTOGRAM || graphopts.graphtype == GT_HISTOGRAM_CROSSMAX || graphopts.graphtype == GT_HISTOGRAM_CROSSMIN;
      bool isLinterp = graphopts.graphtype == GT_LINTERP || graphopts.graphtype == GT_LINTERPSMOOTH;
      
      if (isHistogram)
        fmg.push("vec4 neib = vec4(0.0, 0.0, 0.0, 0.0);" SHNL);  /// min, max, max_allow
      
      if (isHistogram || isLinterp)
        fmg.cfloatvar("specsmooth", graphopts.smooth < -0.5f? -0.5f : graphopts.smooth);
      
      bool deinterp = graphopts.descaling == DE_LINTERP || graphopts.descaling == DE_SINTERP || graphopts.descaling == DE_QINTERP;
      
      unsigned int needDots = 1;
      if ((!deinterp && isLinterp) || graphopts.descaling == DE_LINTERP || graphopts.descaling == DE_SINTERP)
        needDots = 3;
      else if (graphopts.descaling == DE_QINTERP)
        needDots = 4;
      
      if (needDots == 1)
      {
        fmg.push( 
                  "vec3  fx = vec3(relcoords.x, relcoords.x, relcoords.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "vec3  ffy_spec = vec3(getValue1D(i, fx[0]));" SHNL
                    "ffy_spec.gb = vec2(ffy_spec[0], ffy_spec[0]);" SHNL
                    "vec3  ffy = floor(ffy_spec*(ibounds_noscaled.y-1));" SHNL
              
                    "ffy_spec = ffy_spec * (ibounds.y - 1);" SHNL
                    "ivec3  ify_spec = ivec3(floor(ffy_spec));" SHNL
              );
      }
      else if (needDots == 3)
      {
        fmg.push( 
                  "vec3 fx = vec3(float(max(relcoords.x*ibounds_noscaled.x, 1) - 1)/ibounds_noscaled.x, relcoords.x, float(min(relcoords.x*ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x);" SHNL
//                  "vec3  fx = vec3(float(max(icoords_noscaled.x, 1) - 1)/ibounds_noscaled.x, relcoords.x, float(min(icoords_noscaled.x, ibounds_noscaled.x-1) + 1)/ibounds_noscaled.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "vec3  ffy_spec = vec3(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]));" SHNL
                    "vec3  ffy = floor(ffy_spec*(ibounds_noscaled.y-1));" SHNL
              
                    "ffy_spec = ffy_spec * (ibounds.y - 1);" SHNL
                    "ivec3  ify_spec = ivec3(floor(ffy_spec));" SHNL
              );
      }
      else if (needDots == 4)
      {
        fmg.push( 
                  "vec4 fx = vec4(float(max(relcoords.x*ibounds_noscaled.x, 1) - 1)/ibounds_noscaled.x, relcoords.x, " SHNL
                                  "float(min(relcoords.x*ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x, " SHNL
                                  "float(min(relcoords.x*ibounds_noscaled.x, ibounds_noscaled.x-2)  + 2)/ibounds_noscaled.x);" SHNL
//                                  "float(max(relcoords.x*ibounds_noscaled.x, 2) - 2)/ibounds_noscaled.x);" SHNL
                  "for (int i=0; i<countPortions; i++)" SHNL
                  "{" SHNL
                    "vec4  ffy_spec = vec4(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]), getValue1D(i, fx[3]));" SHNL
                    "vec4  ffy = floor(ffy_spec*(ibounds_noscaled.y-1));" SHNL
              
                    "ffy_spec = ffy_spec * (ibounds.y - 1);" SHNL
                    "ivec4  ify_spec = ivec4(floor(ffy_spec));" SHNL
              );
      }
      
      
      
      
      if (deinterp)
      {
        if (graphopts.descaling == DE_LINTERP)
          fmg.push( 
                    "vec3 fsteps = vec3(float(imoded.x), (ify_spec[2] - ify_spec[1])/float(iscaling.x), (ify_spec[1] - ify_spec[0])/float(iscaling.x));" SHNL
                    "ffy_spec = mix(" SHNL
                      "vec3(ify_spec[1] + fsteps[1]*(fsteps.x-1.0), ify_spec[1] + fsteps[1]*fsteps.x, ify_spec[1] + fsteps[1]*(fsteps.x+1.0))," SHNL
                      "vec3(ify_spec[1] - fsteps[2], ify_spec[1], ify_spec[1] + fsteps[1])," SHNL
//                "vec3(0.0, ify_spec[1], ify_spec[1] + fsteps[1])," SHNL
                      "step(fsteps.x, 0.0) );" SHNL
                    );
        else if (graphopts.descaling == DE_SINTERP)
          fmg.push( 
//                    "float corrector = clamp(float(ify_spec[2] - ify_spec[1] + (ify_spec[1] - ify_spec[0]))/((ify_spec[2] - ify_spec[1]) - (ify_spec[1] - ify_spec[0])), -4.0, 4.0);"
//                    "float corrector = clamp(0.5 + 0.5*(ify_spec[1] - ify_spec[0])/(ify_spec[2] - ify_spec[1]) + step(abs(ify_spec[1] - ify_spec[0]), 0.0)*2*sign(ify_spec[2] - ify_spec[1]), -4.0, 4.0);"
                
                "float corrector = clamp(0.5 + 0.5*(ify_spec[1] - ify_spec[0])/(ify_spec[2] - ify_spec[1]) + 0.0*(ify_spec[2] - ify_spec[1]), -4.0, 4.0);"
//                    "float surrector = 1.0 - abs(ify_spec[2] - ify_spec[1])/float(ibounds.y);"
//                    "float surrector = 1.0 - step(ify_spec[2] - ify_spec[1], 5.0)/(ify_spec[0] - ify_spec[1]);"
                      "float surrector = step(iscaling.y*4.0, float(abs(ify_spec[2] - ify_spec[1])))*step(iscaling.y*4.0, float(abs(ify_spec[0] - ify_spec[1])))*(step(float(sign((ify_spec[2] - ify_spec[1])*(ify_spec[0] - ify_spec[1]))), 0.0));"
//                    "surrector = mix(1.0, 0.0, surrector);"
                "surrector = 1.0;"
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
        else if (graphopts.descaling == DE_QINTERP)
          fmg.push(              
                "vec3 dd = vec3(ify_spec[1] - ify_spec[0], ify_spec[2] - ify_spec[1], ify_spec[3] - ify_spec[2]);" SHNL
                "vec3 ss = vec3(sign(dd[0]), sign(dd[1]), sign(dd[2]));" SHNL
                
                "vec2 srs = vec2((1.0 + ss[0]*ss[1])/2.0, (1.0 + ss[1]*ss[2])/2.0);" SHNL
                "ss[1] = ss[1] + (1.0 - abs(ss[1]))*ss[0];" SHNL
                "ss = vec3(ss[0] + (1.0 - abs(ss[0]))*ss[1], ss[1], ss[2] + (1.0 - abs(ss[2]))*ss[1]);" SHNL
                "vec2 corrector = vec2(  clamp(0.5 + ss[0]*ss[1]*mix(0.5, 0.15, srs[0])*sqrt(dd[0]*dd[0] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0),"
                                        "clamp(0.5 - ss[2]*ss[1]*mix(0.5, 0.15, srs[1])*sqrt(dd[2]*dd[2] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0)"
                                      ");" SHNL
//                "vec2 corrector = vec2( 0.5, 0.5 );" SHNL
                "vec3 fsteps = vec3(float(imoded.x), ify_spec[2] - ify_spec[1], (ify_spec[1] - ify_spec[0])/float(iscaling.x));" SHNL
                "vec3 stepF = vec3((fsteps.x - 1.0)/float(iscaling.x), (fsteps.x   )/float(iscaling.x), (fsteps.x + 1.0)/float(iscaling.x));" SHNL
                
                "dd = vec3(mix(corrector[0], corrector[1], stepF[0]), mix(corrector[0], corrector[1], stepF[1]), mix(corrector[0], corrector[1], stepF[2]));" SHNL

                "vec3 fcorr = vec3("
                    "mix(mix(0.0, dd[0], stepF[0]), mix(dd[0], 1.0, stepF[0]), stepF[0]), " SHNL
                    "mix(mix(0.0, dd[1], stepF[1]), mix(dd[0], 1.0, stepF[1]), stepF[1]), " SHNL
                    "mix(mix(0.0, dd[2], stepF[2]), mix(dd[2], 1.0, stepF[2]), stepF[2]) " SHNL
                                  ");" SHNL
                
                    "ffy_spec = mix(" SHNL
                      "vec4(ify_spec[1] + fsteps[1]*fcorr[0], ify_spec[1] + fsteps[1]*fcorr[1], ify_spec[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
//                      "vec3(ify[1] - fsteps[2], ify[1], ify[1] + fsteps[1]*fcorr[1]),"
                      "vec4(ify_spec[1] - fsteps[2], ify_spec[1], ify_spec[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
//                "vec3(0.0, 0.0, 0.0),"
                      "step(fsteps.x, 0.0) );" SHNL
                    );
        
          

        
        if (needDots == 3)
          fmg.push( "ify_spec = ivec3(floor(ffy_spec));" SHNL
                    "ffy = floor(ffy_spec/(iscaling.y));" SHNL
                    );
        else
          fmg.push( "ify_spec = ivec4(floor(ffy_spec));" SHNL
                    "ffy = floor(ffy_spec/(iscaling.y));" SHNL
                    );
      }
      
      
      
      
      if (isDots)
      {
        fmg.push( 
                    "float MIXWELL = step(distance(fcoords_noscaled, vec2(fcoords_noscaled.x, ffy[1])), 0.0);" SHNL
                    "ppb_sfp[0] = ppb_sfp[0]*(1.0 - MIXWELL) + MIXWELL;" SHNL
                    "float VALCLR = ffy[1];" SHNL
                  );
      }
      else if (isLinterp)
      {
        if (graphopts.graphtype == GT_LINTERP)
          fmg.push( 
                      "float fsig_prev = sign(ffy[0] - ffy[1]);" SHNL
                      "float fsig_next = sign(ffy[2] - ffy[1]);" SHNL
                      "float ffdist = (fcoords_noscaled.y - ffy[1])/(1.0 + specsmooth);"
                      "float fmix_prev = 1.0 - clamp(ffdist/(ffy[0]-ffy[1] + fsig_prev*1), 0.0, 1.0);" SHNL   /// fsig_prev*1 == addit. specsmooth level
                      "float fmix_next = 1.0 - clamp(ffdist/(ffy[2]-ffy[1] + fsig_next*1), 0.0, 1.0);" SHNL   /// fsig_next*2 == addit. specsmooth level
                
//                      "float fmix_eq = 1.0 - abs(icoords_noscaled.y - ffy[1])/(3.0*(1.0 + specsmooth));" SHNL
                      "float fmix_eq = 1.0 - abs(fcoords_noscaled.y - ffy[1])/(1.4*(1.0 + specsmooth));" SHNL
//                      "float fmix_eq = 1.0 - abs(icoords_noscaled.y - ffy[1])/(2.0*(1.0 + specsmooth));" SHNL
                
//                      "float fmix_eq = 0.0;"
//                      "fmix_prev = 0.0;"
//                      "fmix_next = 0.0;"
                    );
        else if (graphopts.graphtype == GT_LINTERPSMOOTH)
          fmg.push( 
                      "float fsig_prev = sign(ffy[0] - ffy[1]);" SHNL
                      "float fmix_prev = smoothstep(ffy[0] + fsig_prev, ffy[1], fcoords_noscaled.y);" SHNL
                      "float fsig_next= sign(ffy[2] - ffy[1]);" SHNL
                      "float fmix_next = smoothstep(ffy[2] + fsig_next, ffy[1], fcoords_noscaled.y);" SHNL
                      
                      "float fmix_eq = 1.0 - abs(fcoords_noscaled.y - ffy[1])/(3.0*(1.0 + specsmooth));"
                    );
        
        
        fmg.push( 
                  "vec3 fhit = vec3(0.0, sign(fcoords_noscaled.y - ffy[1]), 0.0);" SHNL
                  "float VALCLR = mix(max(mix(ffy[0], ffy[1], fmix_prev), mix(ffy[2], ffy[1], fmix_next)), ffy[1], 1.0 - abs(fhit.y));" SHNL
                  "fmix_prev = fmix_prev*fsig_prev*fhit.y;" SHNL
                  "fmix_next = fmix_next*fsig_next*fhit.y;" SHNL
                  "fhit.y = 1.0 - abs(fhit.y);" SHNL
                  "float  MIXWELL = max(fhit.y, specopc*max(max(fmix_prev, fmix_next), fmix_eq ));" SHNL
                  "fhit.x = 1.0 - step(MIXWELL, 0.0);" SHNL
                 );
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fmg.push("vec2 fhit_rect = vec2(min(min(ffy[0], ffy[2]), ffy[1]-1)*iscaling.y, max(max(ffy[0], ffy[2]), ffy[1]+1)*iscaling.y + iscaling.y - 1);" SHNL);
      }
      else if (isHistogram)
      {
        fmg.push(                 
                  "vec3 fhit = vec3(sign(fcoords_noscaled.y - ffy[1]), 0.0, 0.0);" SHNL
                  "fhit.xy = vec2(step(fhit.x, 0.0), 1.0 - abs(fhit.x));" SHNL
                  "fhit.z = (1.0 - fhit.x)*specopc*(1.0 - clamp((fcoords_noscaled.y-ffy[1])/(1.0 + specsmooth*10.0), 0.0, 1.0));" SHNL
//                "fhit.z = 0.0;" SHNL
                  "float MIXWELL = fhit.y + (fhit.x - fhit.y)*specopc + fhit.z;" SHNL
                  "fhit.z = 1.0 - step(fhit.z, 0.0);" SHNL
          
                  "float fneiprec = ify_spec[1];" SHNL
                  );
        
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fmg.push("vec2 fhit_rect = vec2(0.0, int(ffy[1])*iscaling.y + iscaling.y - 1);" SHNL);
        
        if (graphopts.graphtype == GT_HISTOGRAM_CROSSMAX)
          fmg.push( "neib[1] = max(neib[1], fneiprec);" SHNL
                    "fneiprec = step(neib[1], fneiprec);" SHNL  /// reassign!!
                    "MIXWELL = MIXWELL*fneiprec;" SHNL
                    "fhit = fhit*fneiprec;" SHNL
                    );
        else if (graphopts.graphtype == GT_HISTOGRAM_CROSSMIN)
          fmg.push( 
                    "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec)*(1.0 - fhit.x));" SHNL
                    "neib[1] = mix(fneiprec, neib[1], neib[2]*(1.0 - step(fneiprec, neib[1])*fhit.x));" SHNL
                    "neib[2] = mix(fhit.x, 1.0, neib[2]);" SHNL   /// + 0.0*fhit.z

                    "fneiprec = (fhit.z + neib[2])*step(neib[0], fneiprec)*step(floor(fneiprec), neib[1]);" SHNL  /// reassign!!
                    "MIXWELL = MIXWELL*fneiprec;" SHNL
                    "fhit = fhit*fneiprec;" SHNL
                
/*
                "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec)*(1.0 - (fhit.x + 1.0*fhit.z)));" SHNL
                "neib[1] = mix(fneiprec, neib[1], neib[2]*(1.0 - step(fneiprec, neib[1])*(fhit.x + 1.0*fhit.z)));" SHNL
                "neib[2] = mix(fhit.x, 1.0, neib[2]);" SHNL
                "neib[3] = mix(fhit.z*MIXWELL, neib[3], sign(neib[3]));" SHNL

                    "fneiprec = step(neib[0], fneiprec)*step(fneiprec, neib[1]);" SHNL  /// reassign!!
//                    "MIXWELL = MIXWELL*fneiprec;" SHNL
                "MIXWELL = mix(MIXWELL*neib[2]*fneiprec, neib[3], sign(neib[3]));" SHNL
                    "fhit = fhit*neib[2]*fneiprec;" SHNL
                          */
                    );
        
        fmg.push( 
                  "float VALCLR = ffy[1];" SHNL
                  );
      }
      if (isLinterp || isHistogram)
      {
        if (graphopts.postrect == PR_STANDARD)
          fmg.push("ppb_sfp[0] = ppb_sfp[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL);
        if (graphopts.postrect == PR_VALUEONLY)
          fmg.push("ppb_sfp[0] = ppb_sfp[0]*(1.0 - fhit.y)*(1.0 - fhit.x) + fhit.y*fhit.x;" SHNL);
        else if (graphopts.postrect == PR_VALUEAROUND)
          fmg.push("ppb_sfp[0] = ppb_sfp[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
                   "ppb_rect.ga = int(1.0 - fhit.x)*ppb_rect.ga + int(fhit.x)*ivec2(fcoords.y - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
        else if (graphopts.postrect == PR_SUMMARY)
          fmg.push("ppb_sfp[0] = ppb_sfp[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
                   "ppb_rect.ga = int(1.0 - (fhit.x - fhit.y))*ppb_rect.ga + int(fhit.x - fhit.y)*ivec2(fcoords.y - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
      }
      
      
      {
        const char*   descaling[] = { "", "", // DE_NONE and DE_LINTERP
                                      "MIXWELL = MIXWELL * (1.0 - abs(sign( imoded.x - iscaling.x/2)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs(int( imoded.x - iscaling.x/2) / (iscaling.x/2.0)));",
                                      "MIXWELL = MIXWELL * (1.0 - abs(int( imoded.x - iscaling.x/2) / (iscaling.x/4.0)));",
                                      "MIXWELL = MIXWELL * (1.0/abs(int( imoded.x - iscaling.x/2)));",
                                      "", ""  // DE_SINTERP, DE_QINTERP
                                    };
        if ((unsigned int)graphopts.descaling < sizeof(descaling)/sizeof(const char*))
          fmg.push(descaling[int(graphopts.descaling)]);
      }
    
      if (graphopts.dotsize)
      {       
        fmg.push( "{" SHNL
                    "float hscx = floor(iscaling.x/2.0 + 0.49);" SHNL);
        if (graphopts.dotsize > 0)
          fmg.cintvar("dotsize", graphopts.dotsize);
        else
        {
          fmg.cintvar("godot", -graphopts.dotsize - 1);
          fmg.push("int dotsize = max(int(hscx) + godot, 0);" SHNL);
        }
        fmg.push("int dist_limit = int(max((dotsize-1) - hscx + 1, 0.0));" SHNL);
        fmg.cfloatvar("dotsmooth", graphopts.dotsmooth);
        
        if (deinterp)
          fmg.push("float remoded_x = float(relcoords.x*ibounds.x - imoded.x)/fbounds.x*ibounds_noscaled.x + (1.0 - step(float(imoded.x), hscx));" SHNL
                   "vec2  remodedoffs = vec2(hscx, 0.0);" SHNL );
        else
          fmg.push("float remoded_x = float(relcoords.x*ibounds.x - imoded.x)/fbounds.x*ibounds_noscaled.x;" SHNL
                   "vec2  remodedoffs = vec2(0.0, 0.0);" SHNL);
        
        
        fmg.push(   "for (int j=-dist_limit; j<=dist_limit; j++)" SHNL
                    "{" SHNL
                      "float reltoval = float(remoded_x + j)/ibounds_noscaled.x;" SHNL
                      "float NVALCLR = getValue1D(i, reltoval);" SHNL
                      "float fdist_weight = (dotsize - distance(fcoords + remodedoffs, "
                        "vec2( floor(reltoval*fbounds.x), floor(NVALCLR*(ibounds.y-1))) + vec2(hscx, 0.0))"
                                            ") / float(dotsize);" SHNL                    
                      "fdist_weight = min(fdist_weight*(1.0 + dotsmooth), 1.0);" SHNL
                      "VALCLR = mix(NVALCLR*(ibounds_noscaled.y-1), VALCLR, step(fdist_weight, MIXWELL));" SHNL // calc before new MIXWELL
                      "MIXWELL = mix(fdist_weight, MIXWELL, step(fdist_weight, MIXWELL));" SHNL
                    "}" SHNL                    
                  "}" SHNL);
      }
      
      if (isHistogram && dc != DrawGraph::CP_RANGE)
        fmg.push("VALCLR = clamp(fcoords_noscaled.y/VALCLR, 0.0, 1.0);" SHNL);
      else
        fmg.push("VALCLR = VALCLR/fbounds_noscaled.y;" SHNL);
      
            
      
      float base = colorizer[0];
      float interval = colorizer[1] - colorizer[0];
      fmg.cfloatvar("pcBase", base);
      fmg.cfloatvar("pcInterval", interval);
      
      if (allocatedPortions > 1)
      {
        fmg.cintvar("allocatedPortions", allocatedPortions);
        if (dc == DrawGraph::CP_SINGLE)                   fmg.push("float portionColor = pcBase + pcInterval*(float(i)/float(allocatedPortions-1));" SHNL);                   /// -1!
        else if (dc == DrawGraph::CP_OWNRANGE)            fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 1.0 - VALCLR);" SHNL);             /// not -1!
        else if (dc == DrawGraph::CP_OWNRANGE_GROSS)      fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 1.0 - sqrt(VALCLR));" SHNL);       /// not -1!
        else if (dc == DrawGraph::CP_OWNRANGE_SYMMETRIC)  fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 0.5 - abs(VALCLR - 0.5) );" SHNL); /// not -1!
        else if (dc == DrawGraph::CP_RANGE)               fmg.push("float portionColor = (pcBase + pcInterval*(float(i)/float(allocatedPortions)))*VALCLR;" SHNL);            /// not -1!
        else if (dc == DrawGraph::CP_SUBPAINTED)          fmg.push("float portionColor = fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
      }
      else
      {
        if (dc == DrawGraph::CP_SINGLE)                   fmg.push("float portionColor = pcBase;" SHNL);
        else if (dc == DrawGraph::CP_OWNRANGE)            fmg.push("float portionColor = pcBase + pcInterval*VALCLR;" SHNL);
        else if (dc == DrawGraph::CP_OWNRANGE_GROSS)      fmg.push("float portionColor = pcBase + pcInterval*sqrt(VALCLR);" SHNL);
        else if (dc == DrawGraph::CP_OWNRANGE_SYMMETRIC)  fmg.push("float portionColor = pcBase + pcInterval*(1.0 - 0.5 + abs(VALCLR - 0.5));" SHNL);
        else if (dc == DrawGraph::CP_RANGE)               fmg.push("float portionColor = pcBase + pcInterval*VALCLR;" SHNL);
        else if (dc == DrawGraph::CP_SUBPAINTED)          fmg.push("float portionColor = fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
      }

      fmg.push(   "vec3  colorGraph = texture(texPalette, vec2(portionColor, 0.0)).rgb;" SHNL );
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
}

/// m_countPortions === graphs
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, COLORPOLICY downcolorize, float colorize_start, float colorize_stop):
  DrawQWidget(new DrawGraph_Sheigen(graphopts_t::goInterp(0.0f, DE_LINTERP), colorize_start, colorize_stop, downcolorize), graphs, OR_LRBT),
  m_graphopts(graphopts_t::goInterp(0.0f, DE_LINTERP)), m_colorpolicy(downcolorize)
{ reConstructor(samples); }
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize, float colorize_start, float colorize_stop):
  DrawQWidget(new DrawGraph_Sheigen(graphopts, colorize_start, colorize_stop, downcolorize), graphs, OR_LRBT), 
  m_graphopts(graphopts), m_colorpolicy(downcolorize)
{ reConstructor(samples); }


void DrawGraph::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB)
{
  *matrixDimmA = m_matrixDimmA;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = m_scalingB;
  clampScaling(scalingA, scalingB);
  *matrixDimmB = sizeB / *scalingB;
  if (*matrixDimmB == 0)
    *matrixDimmB = 1;
}

unsigned int DrawGraph::colorBack() const
{
  return m_graphopts.backcolor == 0xFFFFFFFF? DrawQWidget::colorBack() : m_graphopts.backcolor;
}

////////////////////////////////////////////////////////////////

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, COLORPOLICY downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples)
{
}

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize, float colorize_base, float colorize_step):
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

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, COLORPOLICY downcolorize, float colorize_base, float colorize_step):
  DrawGraph(samples, graphs, downcolorize, colorize_base, colorize_step), m_stepSamples(stepsamples), m_stopped(0), m_memory(graphs, samples, extmemory)
{
  if (extmemory)
    m_matrixLmSize = samples + extmemory;
}

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, const graphopts_t& graphopts, COLORPOLICY downcolorize, float colorize_base, float colorize_step):
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
