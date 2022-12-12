/// DrawGraph is a graph for standart 1D functions 
/// Input: 1D array
/// Scaling: vertical scaling is 1 by default, so You can easily resize Draw
/// Created By: Elijah Vlasov
#include "bsdrawgraph.h" 

#include "core/sheigen/bsshgenmain.h"

#ifndef SHNL
#define SHNL "\n"
#endif

class DrawGraph_Sheigen: public ISheiGenerator
{
public:
  graphopts_t   graphopts;
  coloropts_t   coloropts;
public:
  DrawGraph_Sheigen(const graphopts_t& kgo, const coloropts_t& kco): graphopts(kgo), coloropts(kco)
  {}
  ~DrawGraph_Sheigen();
//public:
//  const graphopts_t&      gopts() const { return graphopts; }
//  const coloropts_t&      copts() const { return coloropts; }
public:
  virtual const char*   shaderName() const {  return "GRAPH"; }
  virtual int           portionMeshType() const{  return PMT_FORCE1D; }
  virtual unsigned int  shvertex_pendingSize() const { return VshMainGenerator1D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const { return VshMainGenerator1D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 5000 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, 
                                         ORIENTATION orient, SPLITPORTIONS splitGraphs,
                                         const impulsedata_t& imp, const overpattern_t& fsp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitGraphs, imp, ovlscount, ovlsinfo);
    fmg.main_begin( coloropts.backcolor != 0xFFFFFFFF? FshDrawConstructor::INIT_BYVALUE : FshDrawConstructor::INIT_BYPALETTE, 
                    coloropts.backcolor, orient, fsp);
    fmg.push("mixwell = 0.0;");
    
    fmg.cfloatvar("specopc", 1.0f - (graphopts.opacity > 1.0f? 1.0f : graphopts.opacity < 0.0f? 0.0f : graphopts.opacity));
    
    static const char graph_locals[] = 
                                      "vec2  ab_fndimms = vec2(texdatadimm_a, texdatadimm_b);" SHNL
                                      "vec2  ab_fbounds = vec2(ab_ibounds);" SHNL
                                      "float b_coord = abc_coords.y*ab_fbounds.y;" SHNL   /// ! no floor.
                                      "float b_coord_ns = floor(abc_coords.y*ab_fndimms.y);" SHNL
                                    ;
    fmg.push(graph_locals);
    {
      bool isDots = graphopts.graphtype == GT_DOTS;
      bool isHistogram = graphopts.graphtype == GT_HISTOGRAM || 
                         graphopts.graphtype == GT_HISTOGRAM_CROSSMAX || 
                         graphopts.graphtype == GT_HISTOGRAM_CROSSMIN || 
                         graphopts.graphtype == GT_HISTOGRAM_MESH || 
                         graphopts.graphtype == GT_HISTOGRAM_SUM || 
                         graphopts.graphtype == GT_HISTOGRAM_LASTBACK;
      bool isInterp = graphopts.graphtype == GT_LINTERP || graphopts.graphtype == GT_LINTERPSMOOTH;
      
      if (isHistogram)
        fmg.push("vec4 neib = vec4(0.0, 0.0, 0.0, 0.0);" SHNL);  /// min, max, max_allow
      
      if (isHistogram || isInterp)
        fmg.cfloatvar("specsmooth", graphopts.smooth < -0.5f? -0.5f : graphopts.smooth);
      
      bool deinterpSL = graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_SINTERP || graphopts.descaling == DE_QINTERP;
      bool deinterpSC = graphopts.descaling == DE_LINTERP_SCALINGCENTER;
      
      unsigned int needDots = 1;
      if ((deinterpSC == false && deinterpSL == false && isInterp) || 
          graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_LINTERP_SCALINGCENTER ||
          graphopts.descaling == DE_SINTERP)
        needDots = 3;
      else if (graphopts.descaling == DE_QINTERP)
        needDots = 4;
      
      if (needDots == 1)
      {
        fmg.push( "vec3  fx = vec3(abc_coords.x, abc_coords.x, abc_coords.x);" SHNL );
        fmg.push( splitGraphs == SP_NONE? "for (int i=0; i<portions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fmg.push(
                  "{" SHNL
                    "vec3   fy = vec3(getValue1D(i, fx[0]));" SHNL
//                    "vec3   fy_ns = floor(fy*(ab_indimms.y-1));" SHNL
//                    "vec3   fy_view = fy*(ab_ibounds.y-1);" SHNL
                    "vec3   fy_ns = floor(fy*ab_indimms.y);" SHNL
                    "vec3   fy_view = fy*ab_ibounds.y;" SHNL
                    "ivec3  iy_view = ivec3(floor(fy_view));" SHNL
              );
      }
      else if (needDots == 3)
      {
        fmg.push( "vec3 fx = vec3(float(max(abc_coords.x*ab_indimms.x, 1) - 1)/ab_indimms.x, "
                    "abc_coords.x, "
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x);" SHNL
                  );
        fmg.push( splitGraphs == SP_NONE? "for (int i=0; i<portions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fmg.push(
                  "{" SHNL
                    "vec3  fy = vec3(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]));" SHNL
//                    "vec3  fy_ns = floor(fy*(ab_indimms.y-1));" SHNL
//                    "vec3  fy_view = fy*(ab_ibounds.y - 1);" SHNL
                    "vec3  fy_ns = floor(fy*ab_indimms.y);" SHNL
                    "vec3  fy_view = fy*ab_ibounds.y;" SHNL
                    "ivec3 iy_view = ivec3(floor(fy_view));" SHNL
              );
      }
      else if (needDots == 4)
      {
        fmg.push( "vec4 fx = vec4(float(max(abc_coords.x*ab_indimms.x, 1) - 1)/ab_indimms.x, abc_coords.x, " SHNL
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x, " SHNL
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-2)  + 2)/ab_indimms.x);" SHNL
                  );
        fmg.push( splitGraphs == SP_NONE? "for (int i=0; i<portions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fmg.push(
                  "{" SHNL
                    "vec4  fy = vec4(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]), getValue1D(i, fx[3]));" SHNL
//                    "vec4  fy_ns = floor(fy*(ab_indimms.y-1));" SHNL
//                    "vec4  fy_view = fy*(ab_ibounds.y - 1);" SHNL
                    "vec4  fy_ns = floor(fy*ab_indimms.y);" SHNL
                    "vec4  fy_view = fy*ab_ibounds.y;" SHNL
                    "ivec4 iy_view = ivec4(floor(fy_view));" SHNL
              );
      }
      
      
      
      
      if (deinterpSL || deinterpSC)
      {
        if (graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_LINTERP_SCALINGCENTER)
        {
          if (graphopts.descaling == DE_LINTERP_SCALINGLEFT)
            fmg.push( "vec3 fds = vec3(immod.x/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);" SHNL);
          else if (graphopts.descaling == DE_LINTERP_SCALINGCENTER)
            fmg.push( "vec3 fds = vec3((immod.x - ab_iscaler.x/2)/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);" SHNL);
          
          fmg.push( "fy_view = vec3(" SHNL    // fy_view*step(float(ab_iscaler.x), 1.0) + (1 - step(float(ab_iscaler.x), 1.0))*"
                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.x, iy_view[1] + (iy_view[2] - iy_view[1])*fds.x, step(0.0, fds.x))," SHNL
                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.y, iy_view[1] + (iy_view[2] - iy_view[1])*fds.y, step(0.0, fds.y))," SHNL
                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.z, iy_view[1] + (iy_view[2] - iy_view[1])*fds.z, step(0.0, fds.z))" SHNL
                                  ");" SHNL
                    );
        }
        else if (graphopts.descaling == DE_SINTERP)
        {
#if 0
//                    "float corrector = clamp(float(iy_view[2] - iy_view[1] + (iy_view[1] - iy_view[0]))/((iy_view[2] - iy_view[1]) - (iy_view[1] - iy_view[0])), -4.0, 4.0);"
//                    "float corrector = clamp(0.5 + 0.5*(iy_view[1] - iy_view[0])/(iy_view[2] - iy_view[1]) + step(abs(iy_view[1] - iy_view[0]), 0.0)*2*sign(iy_view[2] - iy_view[1]), -4.0, 4.0);"
//                    "float surrector = 1.0 - abs(iy_view[2] - iy_view[1])/float(ab_ibounds.y);"
//                    "float surrector = 1.0 - step(iy_view[2] - iy_view[1], 5.0)/(iy_view[0] - iy_view[1]);"
//                    "surrector = mix(1.0, 0.0, surrector);"
          
//                    "vec3 fcorr = vec3((fsteps.x - 1.0)/float(ab_iscaler.x)*mix(corrector, 1.0, (fsteps.x - 1.0)/float(ab_iscaler.x)*surrector), "
//                                      "(fsteps.x      )/float(ab_iscaler.x)*mix(corrector, 1.0, (fsteps.x      )/float(ab_iscaler.x)*surrector), "
//                                      "(fsteps.x + 1.0)/float(ab_iscaler.x)*mix(corrector, 1.0, (fsteps.x + 1.0)/float(ab_iscaler.x)*surrector)"
//                                      ");"
          
//                      "vec3(ify[1] - fsteps[2], ify[1], ify[1] + fsteps[1]*fcorr[1]),"
//                "vec3(0.0, 0.0, 0.0),"
#endif
          fmg.push( 
                    "float corrector = clamp(0.5 + 0.5*(iy_view[1] - iy_view[0])/(iy_view[2] - iy_view[1]) + 0.0*(iy_view[2] - iy_view[1]), -4.0, 4.0);"
//                    "float surrector = step(ab_iscaler.y*4.0, float(abs(iy_view[2] - iy_view[1])))*step(ab_iscaler.y*4.0, float(abs(iy_view[0] - iy_view[1])))*(step(float(sign((iy_view[2] - iy_view[1])*(iy_view[0] - iy_view[1]))), 0.0));"
                    "float surrector = 1.0;"
                    "vec3 fsteps = vec3(float(immod.x), iy_view[2] - iy_view[1], (iy_view[1] - iy_view[0])/float(ab_iscaler.x));"
                    "vec3 fcorr = vec3((fsteps.x - 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x - 1.0)/float(ab_iscaler.x))*surrector), "
                                      "(fsteps.x      )/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x      )/float(ab_iscaler.x))*surrector), "
                                      "(fsteps.x + 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x + 1.0)/float(ab_iscaler.x))*surrector)"
                                      ");"
                
                    "fy_view = mix("
                      "vec3(iy_view[1] + fsteps[1]*fcorr[0], iy_view[1] + fsteps[1]*fcorr[1], iy_view[1] + fsteps[1]*fcorr[2]),"
                      "vec3(iy_view[1] - fsteps[2], iy_view[1], iy_view[1] + fsteps[1]*fcorr[2]),"
                      "step(fsteps.x, 0.0) );"
                    );
//          fmg.push( 
//                    "float corrector = clamp(0.5 + 0.5*(iy_view[1] - iy_view[0])/(iy_view[2] - iy_view[1]) + 0.0*(iy_view[2] - iy_view[1]), -4.0, 4.0);"
////                    "float surrector = step(ab_iscaler.y*4.0, float(abs(iy_view[2] - iy_view[1])))*step(ab_iscaler.y*4.0, float(abs(iy_view[0] - iy_view[1])))*(step(float(sign((iy_view[2] - iy_view[1])*(iy_view[0] - iy_view[1]))), 0.0));"
//                    "float surrector = 1.0;"
//                    "float fcx = (immod.x - ab_iscaler.x/2);"
////                "float fcx = immod.x;"
//                    "vec3 fcorr = vec3((fcx - 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fcx - 1.0)/float(ab_iscaler.x))*surrector), "
//                                      "(fcx      )/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fcx      )/float(ab_iscaler.x))*surrector), "
//                                      "(fcx + 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fcx + 1.0)/float(ab_iscaler.x))*surrector)"
//                                      ");"
//                    "vec3 fds = vec3((immod.x - ab_iscaler.x/2)/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);"
//                    "fy_view = vec3("     // fy_view*step(float(ab_iscaler.x), 1.0) + (1 - step(float(ab_iscaler.x), 1.0))*"
//                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0]*fcorr[0])*fds.x, iy_view[1] + (iy_view[2] - iy_view[1]*fcorr[0])*fds.x, step(0.0, fds.x)),"
//                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0]*fcorr[1])*fds.y, iy_view[1] + (iy_view[2] - iy_view[1]*fcorr[1])*fds.y, step(0.0, fds.y)),"
//                                      "mix(iy_view[1] + (iy_view[1] - iy_view[0]*fcorr[2])*fds.z, iy_view[1] + (iy_view[2] - iy_view[1]*fcorr[2])*fds.z, step(0.0, fds.z))"
//                                  ");"
//                    );
        }
        else if (graphopts.descaling == DE_QINTERP)
          fmg.push(              
                "vec3 dd = vec3(iy_view[1] - iy_view[0], iy_view[2] - iy_view[1], iy_view[3] - iy_view[2]);" SHNL
                "vec3 ss = vec3(sign(dd[0]), sign(dd[1]), sign(dd[2]));" SHNL
                
                "vec2 srs = vec2((1.0 + ss[0]*ss[1])/2.0, (1.0 + ss[1]*ss[2])/2.0);" SHNL
                "ss[1] = ss[1] + (1.0 - abs(ss[1]))*ss[0];" SHNL
                "ss = vec3(ss[0] + (1.0 - abs(ss[0]))*ss[1], ss[1], ss[2] + (1.0 - abs(ss[2]))*ss[1]);" SHNL
                "vec2 corrector = vec2(  clamp(0.5 + ss[0]*ss[1]*mix(0.5, 0.15, srs[0])*sqrt(dd[0]*dd[0] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0),"
                                        "clamp(0.5 - ss[2]*ss[1]*mix(0.5, 0.15, srs[1])*sqrt(dd[2]*dd[2] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0)"
                                      ");" SHNL
//                "vec2 corrector = vec2( 0.5, 0.5 );" SHNL
                "vec3 fsteps = vec3(float(immod.x), iy_view[2] - iy_view[1], (iy_view[1] - iy_view[0])/float(ab_iscaler.x));" SHNL
                "vec3 stepF = vec3((fsteps.x - 1.0)/float(ab_iscaler.x), (fsteps.x   )/float(ab_iscaler.x), (fsteps.x + 1.0)/float(ab_iscaler.x));" SHNL
                
                "dd = vec3(mix(corrector[0], corrector[1], stepF[0]), mix(corrector[0], corrector[1], stepF[1]), mix(corrector[0], corrector[1], stepF[2]));" SHNL

                "vec3 fcorr = vec3("
                    "mix(mix(0.0, dd[0], stepF[0]), mix(dd[0], 1.0, stepF[0]), stepF[0]), " SHNL
                    "mix(mix(0.0, dd[1], stepF[1]), mix(dd[0], 1.0, stepF[1]), stepF[1]), " SHNL
                    "mix(mix(0.0, dd[2], stepF[2]), mix(dd[2], 1.0, stepF[2]), stepF[2]) " SHNL
                                  ");" SHNL
                
                    "fy_view = mix(" SHNL
                      "vec4(iy_view[1] + fsteps[1]*fcorr[0], iy_view[1] + fsteps[1]*fcorr[1], iy_view[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
//                      "vec3(ify[1] - fsteps[2], ify[1], ify[1] + fsteps[1]*fcorr[1]),"
                      "vec4(iy_view[1] - fsteps[2], iy_view[1], iy_view[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
//                "vec3(0.0, 0.0, 0.0),"
                      "step(fsteps.x, 0.0) );" SHNL
                    );
        
          

        
        if (needDots == 3)
          fmg.push( "iy_view = ivec3(floor(fy_view));" SHNL
                    "fy_ns = floor(fy_view/(ab_iscaler.y));" SHNL
                    );
        else
          fmg.push( "iy_view = ivec4(floor(fy_view));" SHNL
                    "fy_ns = floor(fy_view/(ab_iscaler.y));" SHNL
                    );
      }
      
      
      
      
      if (isDots)
      {
        if (graphopts.dotsize == 0)
          fmg.push( 
//                      "float mixwellp = step(distance(fcoords_noscaled, vec2(fcoords_noscaled.x, fy_ns[1])), 0.0);" SHNL // ??? 0.0?
                      "float mixwellp = step(b_coord_ns, fy_ns[1])*step(fy_ns[1], b_coord_ns);" SHNL // ??? 0.0?
                      "post_mask[0] = post_mask[0]*(1.0 - mixwellp) + mixwellp;" SHNL
                    );
        else
          fmg.push(   "float mixwellp = 0.0;" SHNL );
        
        fmg.push("float VALCLR = fy[1];" SHNL);
      }
      else if (isInterp)
      {
        if (graphopts.smooth <= -0.99f)   /// special 8bit interp
        {
          fmg.push( 
                    "float fmix_self = 1.0 - abs(b_coord_ns - fy_ns[1]);" SHNL
                    "float fmix_prev = (fy_ns[0] - b_coord_ns)/(fy_ns[0]-fy_ns[1]);" SHNL
                    "float fmix_next = (fy_ns[2] - b_coord_ns)/(fy_ns[2]-fy_ns[1]);" SHNL
                    "fmix_prev = step(0.5, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL   /// shot is not in (->0)
                    "fmix_next = step(0.5, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL   /// shot is not in (->0)
                    );
        }
        else
        {
          if (graphopts.graphtype == GT_LINTERP)
          {
#if 0                   /// default simple algo, with bad V-effect
            fmg.push( 
                        "float fmix_self = 1.0 - abs(b_coord_ns - fy_ns[1])/(1.4*(1.0 + specsmooth));" SHNL
                  
                        "float fsig_prev = sign(fy_ns[0] - fy_ns[1]);" SHNL
                        "float fsig_next = sign(fy_ns[2] - fy_ns[1]);" SHNL
                        "float ffdist = (b_coord_ns - fy_ns[1])/(1.0 + specsmooth);"
                        "float fmix_prev = 1.0 - clamp(ffdist/(fy_ns[0]-fy_ns[1] + fsig_prev*1), 0.0, 1.0);" SHNL   /// fsig_prev*1 == addit. specsmooth level
                        "float fmix_next = 1.0 - clamp(ffdist/(fy_ns[2]-fy_ns[1] + fsig_next*1), 0.0, 1.0);" SHNL   /// fsig_next*2 == addit. specsmooth level
                  
                        "fmix_prev = fmix_prev*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*(1.0 - step(1.0, fmix_next));" SHNL
                      );
#elif 0                 /// advanced algo with nonsmooth line transition
            fmg.push( 
  //                      "float fmix_self = abs(b_coord_ns - fy_ns[1]);" SHNL
  //                      "fmix_self = 1.0 / (1.0 + fmix_self*2.0);" SHNL
  //                      "fmix_self = fmix_self + specsmooth*2.0*(0.25-(fmix_self-0.5)*(fmix_self-0.5));" SHNL    /// 2.0 - decay
                        "float fmix_self = abs(b_coord_ns - fy_ns[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL
                  
                        "float fsig_prev = sign(fy_ns[0] - fy_ns[1]);" SHNL
                        "float fsig_next = sign(fy_ns[2] - fy_ns[1]);" SHNL
  //                      "float fsig_self = sign(b_coord_ns - fy_ns[1]);" SHNL
  //                      "float fcoop_prev = fsig_prev*fsig_self;" SHNL
  //                      "float fcoop_next = fsig_next*fsig_self;" SHNL
                  
                        "float fmix_prev = (fy_ns[0] - b_coord_ns)/(fy_ns[0]-fy_ns[1]);" SHNL
                        "float fmix_next = (fy_ns[2] - b_coord_ns)/(fy_ns[2]-fy_ns[1]);" SHNL
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
  //                      "fmix_prev = fmix_prev + specsmooth*2.0*(0.36-(fmix_prev-0.6)*(fmix_prev-0.6));"    // 2.0 - decay
  //                      "fmix_next = fmix_next + specsmooth*2.0*(0.36-(fmix_next-0.6)*(fmix_next-0.6));"    // 2.0 - decay
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - decay
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - decay
                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                    
                        );
#elif 1                 /// advanced algo with smooth line transition
            fmg.push( 
                        "float fmix_self = abs(b_coord_ns - fy_ns[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL  /// 0.25 shading
                  
                        "float fsig_prev = sign(fy_ns[0] - fy_ns[1]);" SHNL
                        "float fsig_next = sign(fy_ns[2] - fy_ns[1]);" SHNL
  
                        "float fmix_prev = fy_ns[0] + fsig_prev;" SHNL      /// adding 1 outside pixel for future shading
                        "float fmix_next = fy_ns[2] + fsig_next;" SHNL      /// adding 1 outside pixel for future shading
                        "fmix_prev = (fmix_prev - b_coord_ns)/(fmix_prev-fy_ns[1]);" SHNL
                        "fmix_next = (fmix_next - b_coord_ns)/(fmix_next-fy_ns[1]);" SHNL
                  
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL   /// shot is not in (->0)
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL   /// shot is not in (->0)
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - decay
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - decay
                        "fmix_prev = mix(fmix_prev, 0.2, (1.0-step(fmix_prev, 0.0))*step(fmix_prev, 0.2));" SHNL    /// 0.2 level for all 0..0.2
                        "fmix_next = mix(fmix_next, 0.2, (1.0-step(fmix_next, 0.0))*step(fmix_next, 0.2));" SHNL    /// 0.2 level for all 0..0.2
                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                        );
#else                 /// advanced algo with smooth line transition but bad V-effect destroys
            fmg.push( 
                        "float fmix_self = abs(b_coord_ns - fy_ns[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL
                  
                        "float fsig_prev = sign(fy_ns[0] - fy_ns[1]);" SHNL
                        "float fsig_next = sign(fy_ns[2] - fy_ns[1]);" SHNL
                        "float fmix_prev = fy_ns[0] + (fy_ns[0] - fy_ns[1])/4 + fsig_prev;" SHNL
                        "float fmix_next = fy_ns[2] + (fy_ns[2] - fy_ns[1])/4 + fsig_next;" SHNL
                        "fmix_prev = (fmix_prev - b_coord_ns)/(fmix_prev-fy_ns[1]);" SHNL
                        "fmix_next = (fmix_next - b_coord_ns)/(fmix_next-fy_ns[1]);" SHNL
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - decay
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - decay
                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                    
                        );
#endif
            }
            else if (graphopts.graphtype == GT_LINTERPSMOOTH)
            {
              fmg.push( 
                          "float fsig_prev = sign(fy_ns[0] - fy_ns[1]);" SHNL
                          "float fsig_next = sign(fy_ns[2] - fy_ns[1]);" SHNL
                          "float fmix_prev = smoothstep(fy_ns[0] + fsig_prev, fy_ns[1], b_coord_ns);" SHNL
                          "float fmix_next = smoothstep(fy_ns[2] + fsig_next, fy_ns[1], b_coord_ns);" SHNL
                          "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                          "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                          "float fmix_self = 1.0 - abs(b_coord_ns - fy_ns[1])/(1.4*(1.0 + specsmooth));"
                        );
            }
        }
        fmg.push( 
                  "vec3 fhit = vec3(0.0, step(1.0, fmix_self), 0.0);" SHNL
                  "float mixwellp =  max(fhit.y, specopc*max(max(fmix_prev, fmix_next), fmix_self ));" SHNL
                  "fhit.x = 1.0 - step(mixwellp, 0.0);" SHNL
                  "float VALCLR = b_coord_ns / ab_fndimms.y;" SHNL
                 );
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fmg.push("ivec2 fhit_rect = ivec2(min(min(fy_ns[0], fy_ns[2]), fy_ns[1]-1)*ab_iscaler.y, max(max(fy_ns[0], fy_ns[2]), fy_ns[1]+1)*ab_iscaler.y + ab_iscaler.y - 1);" SHNL);
      } // interp
      else if (isHistogram)
      {
        //        fmg.push(                 
        //                  "vec3 fhit = vec3(sign(b_coord_ns - fy_ns[1]), 0.0, 0.0);" SHNL
        //                  "fhit.xy = vec2(step(fhit.x, 0.0), 1.0 - abs(fhit.x));" SHNL
        //                  "fhit.z = (1.0 - fhit.x)*specopc*(1.0 - clamp((b_coord_ns-fy_ns[1])/(1.0 + specsmooth*10.0), 0.0, 1.0));" SHNL
        //                  "float mixwellp = fhit.y + (fhit.x - fhit.y)*specopc + fhit.z;" SHNL
        //                  "fhit.z = 1.0 - step(fhit.z, 0.0);" SHNL
                  
        //                  "float fneiprec = iy_view[1];" SHNL
        //                  );
        fmg.push(                               
                  "float fmix_max = max(fy_ns[0], fy_ns[2]); " SHNL
                  "float fy_addit = (fmix_max - fy_ns[1])*ab_iscaler.y;" SHNL
                  "fy_addit = fy_addit*step(0.0, fy_addit);" SHNL
                  "fmix_max = fmix_max + 1.0 - step(fmix_max, fy_ns[1]);" SHNL      /// adding 1 outside pixel for future shading
                  "fmix_max = (fmix_max - b_coord_ns)/(fmix_max-fy_ns[1]);" SHNL
                  "fmix_max = fmix_max*step(0.0, fmix_max)*(1.0 - step(1.0, fmix_max));" SHNL   /// shot is not in (->0)
                  "fmix_max = mix(fmix_max, 0.2, (1.0-step(fmix_max, 0.0))*step(fmix_max, 0.2));" SHNL    /// 0.2 level for all 0..0.2
                  "fmix_max = mix(fmix_max, 1.0, step(1.0, fmix_max));" SHNL
              
                  "float fmix_self = b_coord_ns - fy_ns[1];" SHNL
                  "vec3 fhit = vec3(step(fmix_self, 0.0), 0.0, 0.0);" SHNL
                  "fhit.y = fhit.x*step(0.0, fmix_self);" SHNL
//                  "fhit.z = (1.0 - fhit.x)*step(fmix_self, 1.0)*(0.25+specsmooth*0.375);" SHNL
                  "float mixwellp =  max(fhit.y, specopc*(max(fhit.x, fmix_max)) );" SHNL
          
                  "float fneiprec = iy_view[1] + fy_addit;" SHNL
                  );
        
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fmg.push("ivec2 fhit_rect = ivec2(0, int(fy_ns[1])*(ab_iscaler.y) + ab_iscaler.y - 1);" SHNL);
        
        if (graphopts.graphtype == GT_HISTOGRAM_MESH)
          fmg.push( 
//                    "mixwellp = mix(mixwellp*0.8, mix(mixwellp, 0.5*mixwellp, step(b_coord, neib[0])), 1.0 - step(fneiprec, neib[0]));" SHNL
//                    "mixwellp = mix(mixwellp*0.6, mix(mixwellp, 0.4*mixwellp, step(b_coord, neib[0])), 1.0 - step(fneiprec, neib[0]));" SHNL
                    "mixwellp = mix(mixwellp*0.6, mix(mixwellp, 0.4*mixwellp, step(b_coord, neib[0])), 1.0 - step(fneiprec, neib[0]));" SHNL
                    "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec));" SHNL
//                    "fhit = fhit*step(neib[0], fneiprec);" SHNL
                    );
        else if (graphopts.graphtype == GT_HISTOGRAM_CROSSMAX)
            fmg.push( "neib[1] = max(neib[1], fneiprec);" SHNL
                      "fneiprec = step(neib[1], fneiprec);" SHNL  /// reassign!!
                      "mixwellp = mixwellp*fneiprec;" SHNL
                      "fhit = fhit*fneiprec;" SHNL
                      );
        else if (graphopts.graphtype == GT_HISTOGRAM_CROSSMIN)
            fmg.push( 
                      "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec)*(1.0 - fhit.x));" SHNL
                      "neib[1] = mix(fneiprec, neib[1], neib[2]*(1.0 - step(fneiprec, neib[1])*fhit.x));" SHNL
                      "neib[2] = mix(fhit.x, 1.0, neib[2]);" SHNL   /// + 0.0*fhit.z
  
                      "fneiprec = (fhit.z + neib[2])*step(neib[0], fneiprec)*step(floor(fneiprec), neib[1]);" SHNL  /// reassign!!
                      "mixwellp = mixwellp*fneiprec;" SHNL
                      "fhit = fhit*fneiprec;" SHNL
                      );
          
        if (coloropts.cpolicy != CP_REPAINTED)
//          fmg.push("float VALCLR = clamp(b_coord_ns/fy[1], 0.0, 1.0);" SHNL);
          fmg.push("float VALCLR = clamp(b_coord_ns/fy_ns[1], 0.0, 1.0);" SHNL);
        else
          fmg.push("float VALCLR = fy[1];" SHNL);
      } // histogram
      
      if (isInterp || isHistogram)
      {
        if (graphopts.postrect == PR_STANDARD)
          fmg.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL);
        if (graphopts.postrect == PR_VALUEONLY)
          fmg.push("post_mask[0] = post_mask[0]*(1.0 - fhit.y)*(1.0 - fhit.x) + fhit.y*fhit.x;" SHNL);
        else if (graphopts.postrect == PR_VALUEAROUND)
          fmg.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
                   "imrect.ga = int(1.0 - fhit.x)*imrect.ga + int(fhit.x)*ivec2(b_coord - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
        else if (graphopts.postrect == PR_SUMMARY)
          fmg.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
                   "imrect.ga = int(1.0 - (fhit.x - fhit.y))*imrect.ga + int(fhit.x - fhit.y)*ivec2(b_coord - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
      }
      
      
      {
        const char*   descaling[] = { "", "", "", // DE_NONE and DE_LINTERP
                                      "mixwellp = mixwellp * (1.0 - abs(sign( immod.x - ab_iscaler.x/2)));",
                                      "mixwellp = mixwellp * (1.0 - abs(int( immod.x - ab_iscaler.x/2) / (ab_iscaler.x/2.0)));",
                                      "mixwellp = mixwellp * (1.0 - abs(int( immod.x - ab_iscaler.x/2) / (ab_iscaler.x/4.0)));",
                                      "mixwellp = mixwellp * (1.0/abs(int( immod.x - ab_iscaler.x/2)));",
                                      "", ""  // DE_SINTERP, DE_QINTERP
                                    };
        if ((unsigned int)graphopts.descaling < sizeof(descaling)/sizeof(const char*))
          fmg.push(descaling[int(graphopts.descaling)]);
      }
      
      if (graphopts.dotsize)
      {       
        fmg.push( "{" SHNL
                    "float hscx = floor(ab_iscaler.x/2.0 + 0.49);" SHNL);
        if (graphopts.dotsize > 0)
          fmg.cintvar("dotsize", graphopts.dotsize);
        else
        {
          fmg.cintvar("godot", -graphopts.dotsize - 1);
          fmg.push("int dotsize = max(int(hscx) + godot, 0);" SHNL);
        }
        fmg.push("int dist_limit = int(max((dotsize-1) - hscx + 1, 0.0));" SHNL);
        fmg.cfloatvar("dotsmooth", graphopts.dotsmooth);
        
        if (deinterpSL)
        {
          fmg.push("float demoded_x = float(abc_coords.x*ab_ibounds.x - immod.x)/ab_fbounds.x*ab_indimms.x + (step(ab_iscaler.x/2.0, float(immod.x)));" SHNL
                   "float demoded_offs = ab_iscaler.x/2.0 - 0.5;" SHNL
                   );
        }
        else
          fmg.push("float demoded_x = float(abc_coords.x*ab_ibounds.x - immod.x)/ab_fbounds.x*ab_indimms.x;" SHNL
                   "float demoded_offs = 0.0;" SHNL
                   );
        
        fmg.push(   "for (int j=-dist_limit; j<=dist_limit; j++)" SHNL
                    "{" SHNL
                      "float o_fx = float(demoded_x + j)/ab_indimms.x;" SHNL
                      "float o_VALCLR = getValue1D(i, o_fx);" SHNL
//                      "float fdist_weight = (dotsize - distance(fcoords, vec2( floor(o_fx*ab_fbounds.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) ) / float(dotsize);" SHNL
//                      "float fdist_weight = (dotsize - distance(abc_coords*ab_fbounds, vec2( floor(o_fx*ab_fbounds.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) ) / float(dotsize);" SHNL
//                      "float fdist_weight = 1.0 - distance(abc_coords*ab_fbounds, vec2( floor(o_fx*ab_fbounds.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) / float(dotsize);" SHNL
                      "float fdist_weight = 1.0 - distance(abc_coords*ab_fbounds, vec2( o_fx*ab_fbounds.x - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) / float(dotsize);" SHNL
                      "fdist_weight = min(fdist_weight*(1.0 + dotsmooth), 1.0);" SHNL
                      "VALCLR = mix(o_VALCLR, VALCLR, step(fdist_weight, mixwellp));" SHNL // calc before new mixwellp
                      "mixwellp = mix(fdist_weight, mixwellp, step(fdist_weight, mixwellp));" SHNL
                    "}" SHNL                    
                  "}" SHNL);
      }
      
      if (allocatedPortions > 1 && (splitGraphs & SPFLAG_COLORSPLIT) == 0)
      {
        fmg.cintvar("allocatedPortions", (int)allocatedPortions);
        switch (coloropts.cpolicy)
        {
        case CP_MONO:                 fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i);" SHNL); break;
        case CP_PAINTED:              fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 1.0 - VALCLR);" SHNL); break;
        case CP_PAINTED_GROSS:        fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 1.0 - sqrt(VALCLR));" SHNL); break;
        case CP_PAINTED_SYMMETRIC:    fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 0.5 - abs(VALCLR - 0.5));" SHNL); break;
        case CP_REPAINTED:            fmg.push("float porc = (palrange[1] - (palrange[1] - palrange[0])*(float(allocatedPortions - 1 - i)/float(allocatedPortions)))*VALCLR;" SHNL); break;
          
        case CP_PALETTE:              fmg.push("float porc = palrange[0] + (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
        case CP_PALETTE_SPLIT:        fmg.push("float porc = palrange[0] + (palrange[1] - palrange[0])/float(allocatedPortions)*(i + b_coord_ns/ab_fndimms.y);" SHNL); break;
        }
      }
      else
      {
        switch (coloropts.cpolicy)
        {
        case CP_MONO:                 fmg.push("float porc = palrange[1];" SHNL); break;
        case CP_PAINTED:              fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - VALCLR);" SHNL); break;
        case CP_PAINTED_GROSS:        fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*sqrt(1.0 - VALCLR);" SHNL); break;
        case CP_PAINTED_SYMMETRIC:    fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - 0.5 + abs(VALCLR - 0.5));" SHNL); break;
        case CP_REPAINTED:            fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - VALCLR);" SHNL); break;
          
        case CP_PALETTE: case CP_PALETTE_SPLIT:      fmg.push("float porc = palrange[0] + (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
        }
      }

      fmg.push(  "vec3  colorGraph = texture(texpalette, vec2(porc, 0.0)).rgb;" SHNL );
      
      if (graphopts.graphtype == GT_HISTOGRAM_LASTBACK)
        fmg.push("result = mix(result, mix(colorGraph, backcolor, step(float(portions), float(i+1))), mixwellp);" SHNL );
      else if (graphopts.graphtype == GT_HISTOGRAM_SUM)
        fmg.push("result = result + colorGraph*vec3(mixwellp);" SHNL );
      else
        fmg.push("result = mix(result, colorGraph, mixwellp);" SHNL );
      
      fmg.push(  "mixwell = max(mixwell, mixwellp);" SHNL);
      
      fmg.push(  "dvalue = mix(fy[1], dvalue, step(abs(b_coord - dvalue), abs(b_coord - fy[1])) );" SHNL);
      fmg.push("}" SHNL ); // for
      
//      fmg.push("dvalue = mix(dvalue, b_coord, step(mixwell, 0.0));" SHNL);
    }
    fmg.main_end(fsp);
    return fmg.written();
  }
};


DrawGraph_Sheigen::~DrawGraph_Sheigen()
{
}

DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, unsigned int memForDeploy, const graphopts_t& graphopts, const coloropts_t& coloropts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new DrawGraph_Sheigen(graphopts, coloropts), graphs, OR_LRBT, splitGraphs)
{
  m_dataDimmA = samples;
  m_dataDimmB = 1;
  m_portionSize = samples;
  deployMemory(memForDeploy);
  reXtractDynrange(coloropts);
}


void DrawGraph::reConstructor(unsigned int samples)
{
  m_dataDimmA = samples;
  m_dataDimmB = 1;
  m_portionSize = samples;
  deployMemory();
}

void DrawGraph::reXtractDynrange(const coloropts_t& co)
{
  m_ppalrange[PRNG_START] = co.cstart;
  m_ppalrange[PRNG_STOP] = co.cstop;
}

/// m_countPortions === graphs
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new DrawGraph_Sheigen(graphopts_t::goInterp(0.0f, DE_LINTERP), copts), graphs, OR_LRBT, splitGraphs)
//  ,m_graphopts(graphopts_t::goInterp(0.0f, DE_LINTERP)), m_coloropts(copts)
{ reConstructor(samples); reXtractDynrange(copts); }
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new DrawGraph_Sheigen(graphopts, copts), graphs, OR_LRBT, splitGraphs)
//  ,m_graphopts(graphopts), m_coloropts(copts)
{ reConstructor(samples); reXtractDynrange(copts); }

const graphopts_t& DrawGraph::graphopts() const {  return ((DrawGraph_Sheigen*)m_pcsh)->graphopts;   }
const coloropts_t& DrawGraph::coloropts() const {  return ((DrawGraph_Sheigen*)m_pcsh)->coloropts;   }

void DrawGraph::setOpts(const graphopts_t& go)
{
  ((DrawGraph_Sheigen*)m_pcsh)->graphopts = go;
  vmanUpInit();
}

void DrawGraph::setOpts(const coloropts_t& co)
{
  ((DrawGraph_Sheigen*)m_pcsh)->coloropts = co;
  reXtractDynrange(co);
  vmanUpInit();
}

void DrawGraph::setOpts(const graphopts_t& go, const coloropts_t& co)
{
  ((DrawGraph_Sheigen*)m_pcsh)->graphopts = go;
  ((DrawGraph_Sheigen*)m_pcsh)->coloropts = co;
  reXtractDynrange(co);
  vmanUpInit();
}

void DrawGraph::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = m_scalingB;
  clampScaling(scalingA, scalingB);
  *matrixDimmB = sizeB / *scalingB;
  if (*matrixDimmB == 0)
    *matrixDimmB = 1;
}

unsigned int DrawGraph::colorBack() const
{
  unsigned int bc = ((DrawGraph_Sheigen*)m_pcsh)->coloropts.backcolor;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}

////////////////////////////////////////////////////////////////

void DrawGraphDyport::reConstructorEx(unsigned int samples)
{
  m_dataDimmA = samples;
  m_portionSize = samples;
}

DrawGraphDyport::DrawGraphDyport(unsigned int minsamples, unsigned int maxsamples, unsigned int graphs, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawGraph(maxsamples, graphs, copts, splitGraphs), m_minsamples(minsamples), m_maxsamples(maxsamples)
{
  reConstructorEx(minsamples);
}

DrawGraphDyport::DrawGraphDyport(unsigned int minsamples, unsigned int maxsamples, unsigned int graphs, const graphopts_t& graphopts, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawGraph(maxsamples, graphs, graphopts, copts, splitGraphs), m_minsamples(minsamples), m_maxsamples(maxsamples)
{
  reConstructorEx(minsamples);
}

void DrawGraphDyport::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *scalingA = m_scalingA;
  *scalingB = m_scalingB;
  *matrixDimmA = sizeA / m_scalingA;
  if (*matrixDimmA > m_maxsamples)      *matrixDimmA = m_maxsamples;
  else if (*matrixDimmA < m_minsamples) *matrixDimmA = m_minsamples;
  *matrixDimmB = sizeB / *scalingB;
  if (*matrixDimmB == 0)
    *matrixDimmB = 1;
}

int DrawGraphDyport::sizeAndScaleChanged(bool changedDimmA, bool /*changedDimmB*/)
{
  if (changedDimmA)
  {
    m_portionSize = sizeDataA();
  }
  return 0;
}

////////////////////////////////////////////////////////////////

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, coloropts_t copts, SPLITPORTIONS splitGraphs):
  DrawGraph(samples, graphs, copts, splitGraphs), m_stepSamples(stepsamples)
{
}

DrawGraphMove::DrawGraphMove(unsigned int samples, unsigned int stepsamples, unsigned int graphs, const graphopts_t& graphopts, coloropts_t copts, SPLITPORTIONS splitGraphs):
  DrawGraph(samples, graphs, graphopts, copts, splitGraphs), m_stepSamples(stepsamples)
{
}

void DrawGraphMove::setData(const float *data)
{
  for (unsigned int p=0; p<m_countPortions; p++)
  {
    unsigned int poffs = p*m_portionSize;
    for (unsigned int i=0; i<m_portionSize - m_stepSamples; i++)
      m_dataStorage[poffs + i] = m_dataStorage[poffs + i + m_stepSamples];
    for (unsigned int i=0; i<m_stepSamples; i++)
      m_dataStorage[poffs + m_portionSize - m_stepSamples + i] = data[p*m_stepSamples + i];
  }
  DrawGraph::vmanUpData();
}

void DrawGraphMove::setData(const float* data, DataDecimator* decim)
{
  for (unsigned int p=0; p<m_countPortions; p++)
  {
    unsigned int poffs = p*m_portionSize;
    for (unsigned int i=0; i<m_portionSize - m_stepSamples; i++)
      m_dataStorage[poffs + i] = m_dataStorage[poffs + i + m_stepSamples];
    for (unsigned int i=0; i<m_stepSamples; i++)
      m_dataStorage[poffs + m_portionSize - m_stepSamples + i] = decim->decimate(data, m_stepSamples, i, p);
  }
  DrawGraph::vmanUpData();
}

////////////////////////////////////////////////////////////////

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, 
                                 coloropts_t copts, bool resizeExpanding, SPLITPORTIONS splitGraphs):
  DrawGraph(samples, graphs, graphs*(resizeExpanding? 3840 : samples), graphopts_t::goInterp(0.0f, DE_LINTERP), copts, splitGraphs),
  m_stepSamples(stepsamples), m_filloffset(0), m_memory(graphs, samples, extmemory),
  m_resizeExpanding(resizeExpanding)
{
  if (extmemory)
    m_matrixLmSize = samples + extmemory;
}

DrawGraphMoveEx::DrawGraphMoveEx(unsigned int samples, unsigned int stepsamples, unsigned int extmemory, unsigned int graphs, 
                                 const graphopts_t& graphopts, coloropts_t copts, bool resizeExpanding, SPLITPORTIONS splitGraphs):
  DrawGraph(samples, graphs, graphs*(resizeExpanding? 3840 : samples), graphopts, copts, splitGraphs), 
  m_stepSamples(stepsamples), m_filloffset(0), m_memory(graphs, samples, extmemory),
  m_resizeExpanding(resizeExpanding)
{
  if (extmemory)
    m_matrixLmSize = samples + extmemory;
}

//void DrawGraphMoveEx::rescaleA(int v)
//{
//  m_portionSize = m_dataDimmA;
//  int w = c_width, h = c_height;
//  if (m_dataDimmSwitchAB)
//    adjustSizeAndScale(h, w);
//  else
//    adjustSizeAndScale(w, h);
//  setScalingLimitsA(v,v);
//}

void DrawGraphMoveEx::viewDiap(unsigned int* from, unsigned int* to)
{
  int f = int(m_memory.filled()) - m_filloffset - int(this->m_portionSize);
  if (f < 0)  f = 0;
  *from = f;
  int t = int(m_memory.filled()) - m_filloffset;
  if (t < this->m_portionSize)  t = this->m_portionSize;
  *to = t;
}

void DrawGraphMoveEx::memoryRangeSize(int mem)
{
  m_matrixLmSize = m_memory.rangeMemsize(mem);
  m_filloffset = 0;
}

void DrawGraphMoveEx::setMemfloatsUpdate()
{
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::setData(const float *data)
{
  m_memory.onSetData(data, m_stepSamples);
  
  if (m_sbStatic && m_filloffset != 0) m_filloffset += m_stepSamples;
  fillMatrix();
  
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::setData(const float* data, DataDecimator* decim)
{
  m_memory.onSetData(data, m_stepSamples, decim);
  
  if (m_sbStatic && m_filloffset != 0) m_filloffset += m_stepSamples;
  fillMatrix();
  
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::clearData()
{
  m_memory.onClearData();
  DrawQWidget::clearData();
}

void DrawGraphMoveEx::appendData(const float* data, unsigned int length)
{
//  m_memory.onSetData(data, length);
  
//  if (m_sbStatic && m_filloffset != 0) m_filloffset += m_countPortions*length;
  
//  int showlength = this->sizeDataA();
//  m_memory.onFillData(m_filloffset + showlength, showlength, m_dataStorage, 0);
  
//  DrawQWidget::vmanUpData();
  
  m_memory.onSetData(data, length);
  
  if (m_sbStatic && m_filloffset != 0) m_filloffset += length;
  fillMatrix();
  
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::resetData(const float* data, unsigned int length)
{
  m_filloffset = 0;
  m_memory.onClearData();
  appendData(data, length);
}

void DrawGraphMoveEx::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  if (m_resizeExpanding == false)
    DrawGraph::sizeAndScaleHint(sizeA, sizeB, matrixDimmA, matrixDimmB, scalingA, scalingB);
  else
  {
//    *scalingA = 1;
    *scalingA = m_scalingA;
    *scalingB = m_scalingB;
    clampScaling(scalingA, scalingB);
//    *matrixDimmA = sizeA;
    *matrixDimmA = sizeA / *scalingA;
    if (*matrixDimmA == 0)
      *matrixDimmA = 1;
    *matrixDimmB = sizeB / *scalingB;
    if (*matrixDimmB == 0)
      *matrixDimmB = 1;
  }
}

int DrawGraphMoveEx::sizeAndScaleChanged(bool changedDimmA, bool /*changedDimmB*/)
{
  if (changedDimmA)
  {
//    bool more = m_portionSize < m_dataDimmA;
    m_portionSize = m_dataDimmA;
//    if (more)
    {
      clampFilloffset();
      fillMatrix();
    }
    return PC_DATA;
  }
  return 0;
}

void DrawGraphMoveEx::fillMatrix()
{
  m_memory.onFillData(m_filloffset + this->m_portionSize, this->m_portionSize, m_dataStorage, 0);
}

void DrawGraphMoveEx::clampFilloffset()
{
  int m = int(m_memory.filled()), ps = int(this->m_portionSize);
  int a2 = m - m_filloffset;
  int a1 = a2 - ps;
  if (a1 < 0 || a2 < ps)
  {
    m_filloffset = m - ps;
    if (m_filloffset < 0)     ///  ntc!
      m_filloffset = 0;
  }
  else if (a2 > m)
    m_filloffset = 0;
}

int DrawGraphMoveEx::scrollValue() const
{
  return m_filloffset + this->m_portionSize;
}

void DrawGraphMoveEx::scrollDataTo(int pp)
{
  m_filloffset = int(((float)pp/m_memory.allowed())*(m_memory.allowed() - this->m_portionSize));
  clampFilloffset();
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::scrollDataToAbs(int v)
{
  if (v > m_memory.allowed() - this->m_portionSize)
    v = m_memory.allowed() - this->m_portionSize;
  m_filloffset = v;
  clampFilloffset();
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawGraphMoveEx::scrollRelativeTo(int pp)
{
  m_filloffset += pp;
  clampFilloffset();
  fillMatrix();
  DrawQWidget::vmanUpData();
}

