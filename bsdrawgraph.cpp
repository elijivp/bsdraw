/// DrawGraph is a graph for standart 1D functions 
/// Input: 1D array
/// Scaling: vertical scaling is 1 by default, so You can easily resize Draw
/// Created By: Elijah Vlasov
#include "bsdrawgraph.h" 

#include "core/sheigen/bsshgenmain.h"

#ifndef SHNL
#define SHNL "\n"
#endif

class SheiGeneratorGraph: public ISheiGenerator
{
public:
  graphopts_t   graphopts;
  coloropts_t   coloropts;
public:
  SheiGeneratorGraph(const graphopts_t& kgo, const coloropts_t& kco): graphopts(kgo), coloropts(kco)
  {}
  ~SheiGeneratorGraph();
//public:
//  const graphopts_t&      gopts() const { return graphopts; }
//  const coloropts_t&      copts() const { return coloropts; }
public:
  virtual const char*   shaderName() const {  return "GRAPH"; }
  virtual int           portionMeshType() const{  return PMT_FORCE1D; }
  virtual unsigned int  shvertex_pendingSize() const { return VshMainGenerator1D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const { return VshMainGenerator1D()(to); }
  virtual unsigned int  shfragment_pendingSize() const { return 8192; }
  virtual unsigned int  shfragment_uniforms(shuniformdesc_t*, unsigned int){ return 0; }
  virtual void          shfragment_store(FshDrawComposer& fdc) const
  {
    fdc.push("mixwell = 0.0;");
    
//    fdc.cfloatvar("specopc", 1.0f - (graphopts.opacity > 1.0f? 1.0f : graphopts.opacity < 0.0f? 0.0f : graphopts.opacity));
    fdc.cfloatvar("specopc", 1.0f - graphopts.opacity);
    
    {
      bool isDots = graphopts.graphtype == GT_DOTS;
      bool isHistogram = graphopts.graphtype == GT_HISTOGRAM || 
                         graphopts.graphtype == GT_HISTOGRAM_CROSSMAX || 
                         graphopts.graphtype == GT_HISTOGRAM_CROSSMIN || 
                         graphopts.graphtype == GT_HISTOGRAM_MESH || 
                         graphopts.graphtype == GT_HISTOGRAM_SUM || 
                         graphopts.graphtype == GT_HISTOGRAM_LASTBACK;
      bool isInterp = graphopts.graphtype == GT_LINTERP_B || graphopts.graphtype == GT_LINTERP_BSS;
      bool isInterpD = graphopts.graphtype == GT_LINTERP_D;
      
      
      if (isHistogram)   /// for histograms 0.0 is under area, 1.0 is full filled area
      { 
        static const char graph_locals[] = 
                                          "vec2  ab_datadm = vec2(ab_indimms);" SHNL     // ab - ABoriented, dm - dimm
                                          "vec2  ab_viewdm = vec2(ab_ibounds);" SHNL     // - vec2(1) here destroys overpatterns
                                          "float bself_datasc = 1.0 + floor(abc_coords.y*ab_datadm.y);" SHNL
                                          "float bself_viewsc = floor(abc_coords.y*ab_viewdm.y);" SHNL
                                        ;
        fdc.push(graph_locals);
        fdc.push("vec4 neib = vec4(0.0, 0.0, 0.0, 0.0);" SHNL);  /// min, max, max_allow
      }
      else   /// for graphs and dots [0.0,1.0] values (their visual repr) are inside draw area
      {
        if (isInterpD)
        {
          static const char graph_locals[] = 
                                            "vec2  ab_datadm = vec2(ab_indimms) - vec2(1);" SHNL     // ab - ABoriented, dm - dimm
                                            "vec2  ab_viewdm = vec2(ab_ibounds) - vec2(1);" SHNL     
                                            "float bself_datasc = abc_coords.y*ab_datadm.y;" SHNL
                                            "float bself_viewsc = abc_coords.y*ab_viewdm.y;" SHNL   /// ! no floor.  ???????????
                                          ;
          fdc.push(graph_locals);
        }
        else
        {
          static const char graph_locals[] = 
                                            "vec2  ab_datadm = vec2(ab_indimms);" SHNL     // ab - ABoriented, dm - dimm
                                            "vec2  ab_viewdm = vec2(ab_ibounds);" SHNL     
                                            "float bself_datasc = abc_coords.y*ab_datadm.y;" SHNL
                                            "float bself_viewsc = abc_coords.y*ab_viewdm.y;" SHNL   /// ! no floor.  ???????????
                                          ;
          fdc.push(graph_locals);
        }
      }
      
      if (isHistogram || isInterp || isInterpD)
        fdc.cfloatvar("specsmooth", graphopts.smooth < -0.5f? -0.5f : graphopts.smooth);
      
      bool deinterpSL = !isInterpD && (graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_SINTERP || graphopts.descaling == DE_QINTERP);
      bool deinterpSC = !isInterpD && (graphopts.descaling == DE_LINTERP_SCALINGCENTER);
      
      unsigned int needValues = 1;
      bool  strictDotsNearest = false;
      if (isInterpD)
      {
        needValues = 8;
        strictDotsNearest = true;
      }
      else if ((deinterpSC == false && deinterpSL == false && isInterp) || 
          graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_LINTERP_SCALINGCENTER ||
          graphopts.descaling == DE_SINTERP)
        needValues = 3;
      else if (graphopts.descaling == DE_QINTERP)
        needValues = 4;
      
      if (needValues == 1)
      {
        fdc.push( "vec3  ax = vec3(abc_coords.x, abc_coords.x, abc_coords.x);" SHNL );
        fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fdc.push(
                  "{" SHNL
                    "vec3   bv = vec3(getValue1D(i, ax[0]));" SHNL
                    "vec3   bv_datasc = floor(bv*ab_datadm.y);" SHNL
                    "vec3   bv_viewsc = floor(bv*ab_viewdm.y);" SHNL
              );
      }
      else if (needValues == 2)
      {
//        fdc.push( "vec2 ax = vec2(floor(abc_coords.x*ab_indimms.x)/ab_indimms.x, " SHNL      ///  FLOOR???????
//                                 "floor(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x);" SHNL      ///  FLOOR???????
//                  );
        fdc.push( "vec2 ax = vec2((abc_coords.x*ab_indimms.x)/ab_indimms.x, " SHNL      ///  FLOOR???????
                                 "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x);" SHNL      ///  FLOOR???????
                  );
        fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fdc.push(
                  "{" SHNL
                    "vec2  bv = vec2(getValue1D(i, ax[0]), getValue1D(i, ax[1]));" SHNL
//                    "vec2  bv_datasc = floor(bv*ab_indimms.y);" SHNL
//                    "vec2  bv_viewsc = bv*ab_viewdm.y;" SHNL
                    "vec2  bv_datasc = floor(bv*ab_datadm.y);" SHNL
                    "vec2  bv_viewsc = bv*ab_viewdm.y;" SHNL
              );
      }
      else if (needValues == 3)
      {
        fdc.push( "vec3 ax = vec3(float(max(abc_coords.x*ab_indimms.x, 1) - 1)/ab_indimms.x, "
                    "abc_coords.x, "
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x);" SHNL
                  );
        //        fdc.push( "vec3 ax = vec3(floor(max(abc_coords.x*ab_datadm.x, 1) - 1)/ab_datadm.x, " SHNL      ///  FLOOR???????
        //                    "floor(abc_coords.x*ab_datadm.x)/ab_datadm.x, " SHNL      ///  FLOOR???????
        //                    "floor(min(abc_coords.x*ab_datadm.x, ab_datadm.x-1)  + 1)/ab_datadm.x);" SHNL      ///  FLOOR???????
        //                  );
        fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fdc.push(
                  "{" SHNL
                    "vec3  bv = vec3(getValue1D(i, ax[0]), getValue1D(i, ax[1]), getValue1D(i, ax[2]));" SHNL
//                    "vec3  bv_datasc = floor(bv*ab_indimms.y);" SHNL
//                    "vec3  bv_viewsc = bv*ab_ibounds.y;" SHNL
//        "vec3  bv_datasc = floor(bv*ab_indimms.y);" SHNL
//        "vec3  bv_viewsc = floor(bv*ab_ibounds.y);" SHNL
                    "vec3  bv_datasc = floor(bv*ab_datadm.y);" SHNL
                    "vec3  bv_viewsc = floor(bv*ab_viewdm.y);" SHNL
//        "vec3  bv_datasc = floor(bv*ab_datadm.y);" SHNL
//        "vec3  bv_viewsc = floor(bv*ab_viewdm.y);" SHNL
              );
      }
      else if (needValues == 4)
      {
        fdc.push( "vec4 ax = vec4(float(max(abc_coords.x*ab_indimms.x, 1) - 1)/ab_indimms.x, abc_coords.x, " SHNL
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x, " SHNL
                    "float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-2)  + 2)/ab_indimms.x);" SHNL
                  );
        //        fdc.push( "vec4 ax = vec4(floor(max(abc_coords.x*ab_datadm.x, 1) - 1)/ab_datadm.x, " SHNL      ///  FLOOR???????
        //                    "floor(abc_coords.x*ab_datadm.x)/ab_datadm.x, " SHNL      ///  FLOOR???????
        //                    "floor(min(abc_coords.x*ab_datadm.x, ab_datadm.x-1)  + 1)/ab_datadm.x, " SHNL      ///  FLOOR???????
        //                    "floor(min(abc_coords.x*ab_datadm.x, ab_datadm.x-1)  + 2)/ab_datadm.x);" SHNL      ///  FLOOR???????
        //                  );
        fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fdc.push(
                  "{" SHNL
                    "vec4  bv = vec4(getValue1D(i, ax[0]), getValue1D(i, ax[1]), getValue1D(i, ax[2]), getValue1D(i, ax[3]));" SHNL
//                    "vec4  bv_datasc = floor(bv*ab_indimms.y);" SHNL
//                    "vec4  bv_viewsc = bv*ab_ibounds.y;" SHNL
                    "vec4  bv_datasc = floor(bv*ab_datadm.y);" SHNL
                    "vec4  bv_viewsc = floor(bv*ab_viewdm.y);" SHNL
              );
      }
      else if (needValues > 4) // strictDotsNearest!
      {
        fdc.cintvar("fcnt", needValues);
        fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
        fdc.push(
                  "{" SHNL
              );
        fdc.push(   
                    "float ax[fcnt], bv[fcnt];" SHNL
                    "for (int j=0; j<fcnt; j++)" SHNL
                    "{" SHNL
//                      "ax[j] = floor(abc_coords.x*ab_datadm.x - fcnt/2.0 + 1 + j)/ab_datadm.x;" SHNL      ///  FLOOR???????     I DONT UNDERSTAND!!
//                      "bv[j] = getValue1D(i, ax[j]);" SHNL
//                      "ax[j] = (abc_coords.x*ab_datadm.x - fcnt/2.0 + 1 + j)/ab_datadm.x;" SHNL      ///  FLOOR???????
                      "ax[j] = floor(abc_coords.x*ab_datadm.x - fcnt/2.0 + 1 + j)/ab_datadm.x;" SHNL      ///  FLOOR???????     I DONT UNDERSTAND!!
                      "bv[j] = getValue1D(i, ax[j]);" SHNL
                    "}" SHNL
//                    "vec2  bv_datasc = floor(bv*ab_datadm.y);" SHNL
//                    "vec2  bv_viewsc = bv*ab_viewdm.y;" SHNL
              );
      }
      
      
      
      
      if (deinterpSL || deinterpSC)
      {
        if (graphopts.descaling == DE_LINTERP_SCALINGLEFT || graphopts.descaling == DE_LINTERP_SCALINGCENTER)
        {
          if (graphopts.descaling == DE_LINTERP_SCALINGLEFT)
            fdc.push( "vec3 fds = vec3(immod.x/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);" SHNL);
          else if (graphopts.descaling == DE_LINTERP_SCALINGCENTER)
            fdc.push( "vec3 fds = vec3((immod.x - ab_iscaler.x/2)/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);" SHNL);
          
          fdc.push( "bv_viewsc = vec3(" SHNL    // bv_viewsc*step(float(ab_iscaler.x), 1.0) + (1 - step(float(ab_iscaler.x), 1.0))*"
                                      "mix(bv_viewsc[1] + (bv_viewsc[1] - bv_viewsc[0])*fds.x, bv_viewsc[1] + (bv_viewsc[2] - bv_viewsc[1])*fds.x, step(0.0, fds.x))," SHNL
                                      "mix(bv_viewsc[1] + (bv_viewsc[1] - bv_viewsc[0])*fds.y, bv_viewsc[1] + (bv_viewsc[2] - bv_viewsc[1])*fds.y, step(0.0, fds.y))," SHNL
                                      "mix(bv_viewsc[1] + (bv_viewsc[1] - bv_viewsc[0])*fds.z, bv_viewsc[1] + (bv_viewsc[2] - bv_viewsc[1])*fds.z, step(0.0, fds.z))" SHNL
                                  ");" SHNL
                    );
        }
        else if (graphopts.descaling == DE_SINTERP)
        {
          fdc.push( 
                    "float corrector = clamp(0.5 + 0.5*(bv_viewsc[1] - bv_viewsc[0])/(bv_viewsc[2] - bv_viewsc[1]) + 0.0*(bv_viewsc[2] - bv_viewsc[1]), -4.0, 4.0);"
                    "float surrector = 1.0;"
                    "vec3 fsteps = vec3(float(immod.x), bv_viewsc[2] - bv_viewsc[1], (bv_viewsc[1] - bv_viewsc[0])/float(ab_iscaler.x));"
                    "vec3 fcorr = vec3((fsteps.x - 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x - 1.0)/float(ab_iscaler.x))*surrector), "
                                      "(fsteps.x      )/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x      )/float(ab_iscaler.x))*surrector), "
                                      "(fsteps.x + 1.0)/float(ab_iscaler.x)*mix(1.0, corrector, (1.0 - (fsteps.x + 1.0)/float(ab_iscaler.x))*surrector)"
                                      ");"
                
                    "bv_viewsc = mix("
                      "vec3(bv_viewsc[1] + fsteps[1]*fcorr[0], bv_viewsc[1] + fsteps[1]*fcorr[1], bv_viewsc[1] + fsteps[1]*fcorr[2]),"
                      "vec3(bv_viewsc[1] - fsteps[2], bv_viewsc[1], bv_viewsc[1] + fsteps[1]*fcorr[2]),"
                      "step(fsteps.x, 0.0) );"
                    );
        }
        else if (graphopts.descaling == DE_QINTERP)
          fdc.push(              
                    "vec3 dd = vec3(bv_viewsc[1] - bv_viewsc[0], bv_viewsc[2] - bv_viewsc[1], bv_viewsc[3] - bv_viewsc[2]);" SHNL
                    "vec3 ss = vec3(sign(dd[0]), sign(dd[1]), sign(dd[2]));" SHNL
                    
                    "vec2 srs = vec2((1.0 + ss[0]*ss[1])/2.0, (1.0 + ss[1]*ss[2])/2.0);" SHNL
                    "ss[1] = ss[1] + (1.0 - abs(ss[1]))*ss[0];" SHNL
                    "ss = vec3(ss[0] + (1.0 - abs(ss[0]))*ss[1], ss[1], ss[2] + (1.0 - abs(ss[2]))*ss[1]);" SHNL
                    "vec2 corrector = vec2(  clamp(0.5 + ss[0]*ss[1]*mix(0.5, 0.15, srs[0])*sqrt(dd[0]*dd[0] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0),"
                                            "clamp(0.5 - ss[2]*ss[1]*mix(0.5, 0.15, srs[1])*sqrt(dd[2]*dd[2] + 1.0)/sqrt(dd[1]*dd[1] + 1.0), -3.0, 3.0)"
                                          ");" SHNL
    //                "vec2 corrector = vec2( 0.5, 0.5 );" SHNL
                    "vec3 fsteps = vec3(float(immod.x), bv_viewsc[2] - bv_viewsc[1], (bv_viewsc[1] - bv_viewsc[0])/float(ab_iscaler.x));" SHNL
                    "vec3 stepF = vec3((fsteps.x - 1.0)/float(ab_iscaler.x), (fsteps.x   )/float(ab_iscaler.x), (fsteps.x + 1.0)/float(ab_iscaler.x));" SHNL
                    
                    "dd = vec3(mix(corrector[0], corrector[1], stepF[0]), mix(corrector[0], corrector[1], stepF[1]), mix(corrector[0], corrector[1], stepF[2]));" SHNL
    
                    "vec3 fcorr = vec3("
                        "mix(mix(0.0, dd[0], stepF[0]), mix(dd[0], 1.0, stepF[0]), stepF[0]), " SHNL
                        "mix(mix(0.0, dd[1], stepF[1]), mix(dd[0], 1.0, stepF[1]), stepF[1]), " SHNL
                        "mix(mix(0.0, dd[2], stepF[2]), mix(dd[2], 1.0, stepF[2]), stepF[2]) " SHNL
                                      ");" SHNL
                    
                    "bv_viewsc = mix(" SHNL
                          "vec4(bv_viewsc[1] + fsteps[1]*fcorr[0], bv_viewsc[1] + fsteps[1]*fcorr[1], bv_viewsc[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
                          "vec4(bv_viewsc[1] - fsteps[2], bv_viewsc[1], bv_viewsc[1] + fsteps[1]*fcorr[2], 0.0)," SHNL
                          "step(fsteps.x, 0.0) );" SHNL
                    );
        
        fdc.push( "bv_datasc = floor(bv_viewsc/(ab_iscaler.y));" SHNL );
      } // if (deinterpSL || deinterpSC)
      
      
      
      
      if (isDots)
      {
        if (graphopts.dotsize == 0)
          fdc.push( 
//                      "float mixwellp = step(distance(fcoords_noscaled, vec2(fcoords_noscaled.x, bv_datasc[1])), 0.0);" SHNL // ??? 0.0?
//                      "float mixwellp = step(bself_datasc, bv_datasc[1])*step(bv_datasc[1], bself_datasc);" SHNL // ??? 0.0?
                      "float mixwellp = step(floor(bself_datasc), bv_datasc[1])*step(bv_datasc[1], floor(bself_datasc));" SHNL // ??? 0.0?
                      "post_mask[0] = post_mask[0]*(1.0 - mixwellp) + mixwellp;" SHNL
                    );
        else
          fdc.push(   "float mixwellp = 0.0;" SHNL );
        
        fdc.push("float VALCLR = bv[1];" SHNL);
      }
      else if (isInterp)
      {
        if (graphopts.smooth <= -0.99f)   /// special 8bit interp
        {
          fdc.push( 
                    "float fmix_self = 1.0 - abs(bself_datasc - bv_datasc[1]);" SHNL
                    "float fmix_prev = (bv_datasc[0] - bself_datasc)/(bv_datasc[0]-bv_datasc[1]);" SHNL
                    "float fmix_next = (bv_datasc[2] - bself_datasc)/(bv_datasc[2]-bv_datasc[1]);" SHNL
                    "fmix_prev = step(0.5, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL   /// shot is not in (->0)
                    "fmix_next = step(0.5, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL   /// shot is not in (->0)
                    );
        }
        else
        {
          if (graphopts.graphtype == GT_LINTERP_B)
          {
#if 0                   /// default simple algo, with bad V-effect
            fdc.push( 
                        "float fmix_self = 1.0 - abs(bself_datasc - bv_datasc[1])/(1.4*(1.0 + specsmooth));" SHNL
                  
                        "float fsig_prev = sign(bv_datasc[0] - bv_datasc[1]);" SHNL
                        "float fsig_next = sign(bv_datasc[2] - bv_datasc[1]);" SHNL
                        "float ffdist = (bself_datasc - bv_datasc[1])/(1.0 + specsmooth);"
                        "float fmix_prev = 1.0 - clamp(ffdist/(bv_datasc[0]-bv_datasc[1] + fsig_prev*1), 0.0, 1.0);" SHNL   /// fsig_prev*1 == addit. specsmooth level
                        "float fmix_next = 1.0 - clamp(ffdist/(bv_datasc[2]-bv_datasc[1] + fsig_next*1), 0.0, 1.0);" SHNL   /// fsig_next*2 == addit. specsmooth level
                  
                        "fmix_prev = fmix_prev*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*(1.0 - step(1.0, fmix_next));" SHNL
                      );
#elif 0                 /// advanced algo with nonsmooth line transition
            fdc.push( 
  //                      "float fmix_self = abs(bself_datasc - bv_datasc[1]);" SHNL
  //                      "fmix_self = 1.0 / (1.0 + fmix_self*2.0);" SHNL
  //                      "fmix_self = fmix_self + specsmooth*2.0*(0.25-(fmix_self-0.5)*(fmix_self-0.5));" SHNL    /// 2.0 - gap
                        "float fmix_self = abs(bself_datasc - bv_datasc[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL
                  
                        "float fsig_prev = sign(bv_datasc[0] - bv_datasc[1]);" SHNL
                        "float fsig_next = sign(bv_datasc[2] - bv_datasc[1]);" SHNL
  //                      "float fsig_self = sign(bself_datasc - bv_datasc[1]);" SHNL
  //                      "float fcoop_prev = fsig_prev*fsig_self;" SHNL
  //                      "float fcoop_next = fsig_next*fsig_self;" SHNL
                  
                        "float fmix_prev = (bv_datasc[0] - bself_datasc)/(bv_datasc[0]-bv_datasc[1]);" SHNL
                        "float fmix_next = (bv_datasc[2] - bself_datasc)/(bv_datasc[2]-bv_datasc[1]);" SHNL
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
  //                      "fmix_prev = fmix_prev + specsmooth*2.0*(0.36-(fmix_prev-0.6)*(fmix_prev-0.6));"    // 2.0 - gap
  //                      "fmix_next = fmix_next + specsmooth*2.0*(0.36-(fmix_next-0.6)*(fmix_next-0.6));"    // 2.0 - gap
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - gap
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - gap
                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                    
                        );
#elif 1                 /// advanced algo with smooth line transition
            fdc.push( 
                        "float fmix_self = abs(bself_datasc - bv_datasc[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL  /// 0.25 shading
                  
                        "float fsig_prev = sign(bv_datasc[0] - bv_datasc[1]);" SHNL
                        "float fsig_next = sign(bv_datasc[2] - bv_datasc[1]);" SHNL
  
                        "float fmix_prev = bv_datasc[0] + fsig_prev;" SHNL      /// adding 1 outside pixel for future shading
                        "float fmix_next = bv_datasc[2] + fsig_next;" SHNL      /// adding 1 outside pixel for future shading
                        "fmix_prev = (fmix_prev - bself_datasc)/(fmix_prev-bv_datasc[1]);" SHNL
                        "fmix_next = (fmix_next - bself_datasc)/(fmix_next-bv_datasc[1]);" SHNL
                  
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL   /// shot is not in (->0)
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL   /// shot is not in (->0)
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - gap
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - gap
                        "fmix_prev = mix(fmix_prev, 0.2, (1.0-step(fmix_prev, 0.0))*step(fmix_prev, 0.2));" SHNL    /// 0.2 level for all 0..0.2
                        "fmix_next = mix(fmix_next, 0.2, (1.0-step(fmix_next, 0.0))*step(fmix_next, 0.2));" SHNL    /// 0.2 level for all 0..0.2
//                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
//                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                        );
#else                 /// advanced algo with smooth line transition but bad V-effect destroys
            fdc.push( 
                        "float fmix_self = abs(bself_datasc - bv_datasc[1]);" SHNL
                        "fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);" SHNL
                  
                        "float fsig_prev = sign(bv_datasc[0] - bv_datasc[1]);" SHNL
                        "float fsig_next = sign(bv_datasc[2] - bv_datasc[1]);" SHNL
                        "float fmix_prev = bv_datasc[0] + (bv_datasc[0] - bv_datasc[1])/4 + fsig_prev;" SHNL
                        "float fmix_next = bv_datasc[2] + (bv_datasc[2] - bv_datasc[1])/4 + fsig_next;" SHNL
                        "fmix_prev = (fmix_prev - bself_datasc)/(fmix_prev-bv_datasc[1]);" SHNL
                        "fmix_next = (fmix_next - bself_datasc)/(fmix_next-bv_datasc[1]);" SHNL
                        "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                        "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                        "float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;" SHNL
                        "fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));" SHNL    /// 2.0 - gap
                        "fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));" SHNL    /// 2.0 - gap
                        "fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));" SHNL
                        "fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));" SHNL
                    
                        );
#endif
            }
            else if (graphopts.graphtype == GT_LINTERP_BSS)
            {
              fdc.push( 
                          "float fsig_prev = sign(bv_datasc[0] - bv_datasc[1]);" SHNL
                          "float fsig_next = sign(bv_datasc[2] - bv_datasc[1]);" SHNL
                          "float fmix_prev = smoothstep(bv_datasc[0] + fsig_prev, bv_datasc[1], bself_datasc);" SHNL
                          "float fmix_next = smoothstep(bv_datasc[2] + fsig_next, bv_datasc[1], bself_datasc);" SHNL
                          "fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));" SHNL
                          "fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));" SHNL
                          "float fmix_self = 1.0 - abs(bself_datasc - bv_datasc[1])/(1.4*(1.0 + specsmooth));"
                        );
            }
        }
        fdc.push( 
                  "vec3 fhit = vec3(0.0, step(1.0, fmix_self), 0.0);" SHNL
//                  "float mixwellp = max(fhit.y, specopc*max(max(fmix_prev, fmix_next), fmix_self ));" SHNL
                  "float mixwellp = max(fhit.y, min(specopc*max(max(fmix_prev, fmix_next), fmix_self), 1.0));" SHNL
                  
        
//              "float fmix_summ = max(max(fmix_prev, fmix_next), fmix_self);" SHNL
//              "float mixwellp = max(fhit.y, min((specopc + fmix_summ)*step, 1.0));" SHNL
        
//            "float mixwellp = max(fhit.y, min((specopc + fmix_summ)*step, 1.0));" SHNL

                  "fhit.x = 1.0 - step(mixwellp, 0.0);" SHNL
                  "float VALCLR = bself_datasc / ab_datadm.y;" SHNL
                 );
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fdc.push("ivec2 fhit_rect = ivec2(min(min(bv_datasc[0], bv_datasc[2]), bv_datasc[1]-1)*ab_iscaler.y, max(max(bv_datasc[0], bv_datasc[2]), bv_datasc[1]+1)*ab_iscaler.y + ab_iscaler.y - 1);" SHNL);
      } // interpB
      else if (isInterpD)
      {
//                 "float dz = step(length((ptb + pte)/2.0), length(pte-ptb)/2.0 + 1.0 + bold);" SHNL // center point and radius
//        fdc.push("vec2 pt  = vec2(abc_coords.x*ab_viewdm.x, abc_coords.y*ab_viewdm.y);" SHNL);
//        fdc.push("vec2 ptb = vec2(ax[0]*ab_viewdm.x, bv[0]*ab_viewdm.y);" SHNL
//                 "vec2 pte = vec2(ax[1]*ab_viewdm.x, bv[1]*ab_viewdm.y);");
//        fdc.push("vec2 bz = vec2(pte.x-ptb.x, pte.y-ptb.y);" SHNL
//                 "float znm = sqrt(bz[0]*bz[0] + bz[1]*bz[1]);" SHNL
//                 "vec3 ABC = vec3(bz.y/znm, -bz.x/znm, (pte.x*ptb.y - ptb.x*pte.y)/znm);"
//                 "float nrm = sqrt(ABC[0]*ABC[0] + ABC[1]*ABC[1]);" SHNL
//                 "float dist = abs( ABC[0]*pt.x + ABC[1]*pt.y + ABC[2] )/nrm;" SHNL
//                 );
        
        //        {
        //          fdc.push(/*"float lateral = max(distance(pt, pte), distance(pt,ptb));" SHNL
        //                   "float laton = lateral - length(bz);" SHNL
        //                   "laton = laton*step(0.0, laton);" SHNL*/
        //                   "float dist_ptb = abs(ABC[1]*(pt.x - ptb.x) - ABC[0]*(pt.y - ptb.y));" SHNL
        //                   "float dist_pte = abs(ABC[1]*(pt.x - pte.x) - ABC[0]*(pt.y - pte.y));" SHNL
        ////                   "float be_len = distance(ptb, pte);" SHNL
        //                   "float overlen = max(dist_ptb,dist_pte) - (distance(ptb, pte));" SHNL
        ////                   "dist = mix(dist, max(dist, overlen), step(0.0, overlen));" SHNL
        //                   "dist = mix(dist, mix(distance(pte, pt), distance(ptb, pt), step(dist_ptb, dist_pte)), step(0.0, overlen));" SHNL
                           
        ////                   "dist = max(dist, max(dist_ptb, dist_pte));" SHNL
        ////                   "dist = dist + abs(dist_ptb-dist_pte)*step(length(bz), max(dist_ptb,dist_pte));" SHNL
                           
        //                   );
        //        }
        //          {
        //            fdc.push("vec2 ptc = (ptb + pte)/vec2(2);" SHNL
        ////                     "float dist_ptc = abs(ABC[1]*(pt.x - ptc.x) - ABC[0]*(pt.y - ptc.y));" SHNL
        //                     "float dist_ptc = abs(ABC[1]*(pt.x - ptc.x) - ABC[0]*(pt.y - ptc.y))/sqrt(ABC[0]*ABC[0] + ABC[1]*ABC[1]);" SHNL
        //                     "float overlen = dist_ptc - distance(ptb, pte)/2.0;" SHNL
        ////                     "dist = mix(dist, min(dist, overlen), step(0.0, overlen));" SHNL
        //                     "dist = max(dist, min(distance(pte, pt), distance(ptb, pt))*step(0.0, overlen));" SHNL
        //                     );
        //          }
        
        
        fdc.push("vec2 pt  = abc_coords*ab_viewdm;" SHNL);
        fdc.push("float mixwellp = 0.0;" SHNL);
//        fdc.push("float cf_specsmooth = 4*(specsmooth*(specsmooth + 2.0) + 1.15);" SHNL);
        fdc.push("float cf_specsmooth = 3*(specsmooth*(0.25*specsmooth + 0.5) + 0.4);" SHNL);
//        fdc.push("float cf_specsmooth = 6*(0.5*(0.25*0.5 + 0.5) + 0.4);" SHNL);
        
        fdc.push("for (int j=0; j<fcnt-1; j++)" SHNL
                 "{" SHNL
                );
        {
          fdc.push("vec2 ptb = vec2(ax[j], bv[j])*ab_viewdm;" SHNL
                   "vec2 pte = vec2(ax[j+1], bv[j+1])*ab_viewdm;");
          fdc.push("vec2 bz = vec2(pte.x-ptb.x, pte.y-ptb.y);" SHNL
                   "vec3 ABC = vec3(-bz.y, +bz.x, ptb.x*pte.y - ptb.y*pte.x);"
                   "float nrm = sqrt(ABC[0]*ABC[0] + ABC[1]*ABC[1]);" SHNL
                   "float dist = abs( ABC[0]*pt.x + ABC[1]*pt.y + ABC[2] )/nrm;" SHNL
                   );
//                  {
//                    fdc.push(
//                             "float dist_ptb = abs(ABC[1]*(pt.x - ptb.x) - ABC[0]*(pt.y - ptb.y))/nrm;" SHNL
//                             "float dist_pte = abs(ABC[1]*(pt.x - pte.x) - ABC[0]*(pt.y - pte.y))/nrm;" SHNL
//                             "float overlen = max(dist_ptb,dist_pte) - (distance(ptb, pte));" SHNL
//                             "dist = mix(dist, mix(distance(pte, pt), distance(ptb, pt), step(dist_ptb, dist_pte)), step(0.0, overlen));" SHNL
//                             );
//                  }
          {
            fdc.push(
                     "vec3 dds = vec3(distance(ptb, pt), distance(pte, pt), distance(ptb, pte));" SHNL
                     "dist = mix(dist, max(dist, mix(dds[0], dds[1], step(dds[1], dds[0]))), step(dds[2], max(dds[0], dds[1])));" SHNL
                     );
          }
          
          fdc.push("dist = dist/(1.0 + (1.0 - specopc)*(1.0 - abs(fcnt/2.0-j)/fcnt));" SHNL
                   "float mixwellj = 1.0/(dist*dist*cf_specsmooth + 1.0);" SHNL
                   "mixwellp = max(mixwellp, mixwellj);" SHNL
                   );
        }
        fdc.push("}" SHNL
                 "float VALCLR = bself_datasc / ab_datadm.y;" SHNL
                 );
      }
      else if (isHistogram)
      {
        fdc.push(                               
                  "float fmix_self = bself_datasc - bv_datasc[1];" SHNL   // floor - floor
                  "vec3 fhit = vec3(step(fmix_self, 0.0), 0.0, 0.0);" SHNL
                  "fhit.y = fhit.x*step(0.0, fmix_self);" SHNL
//                  "fhit.z = (1.0 - fhit.x)*step(fmix_self, 1.0)*(0.25+specsmooth*0.375);" SHNL    FHIT.Z reserved! for future smooth
                  "float mixwellp =  max(fhit.y, specopc*fhit.x);" SHNL
                  "float fneiprec = floor(bv_datasc[1]);" SHNL
                  );
        
        if (graphopts.postrect == PR_VALUEAROUND || graphopts.postrect == PR_SUMMARY)
          fdc.push("ivec2 fhit_rect = ivec2(ab_iscaler.y, int(bv_datasc[1])*(ab_iscaler.y) + ab_iscaler.y - 1);" SHNL); // ab_iscaler.y compensates (0,1]
        
        if (graphopts.graphtype == GT_HISTOGRAM_MESH)
          fdc.push( "mixwellp = mixwellp*mix(0.4, mix(0.6, 1.0, step(neib[0], bself_datasc)), step(neib[0], fneiprec));" SHNL
                    "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec));" SHNL
                    );
        else if (graphopts.graphtype == GT_HISTOGRAM_CROSSMAX)
          fdc.push( "neib[0] = max(neib[0], fneiprec);" SHNL
                    "fneiprec = step(neib[0], fneiprec);" SHNL  /// reassign!!
                    "mixwellp = mixwellp*fneiprec;" SHNL
                    "fhit = fhit*fneiprec;" SHNL
                    );
        else if (graphopts.graphtype == GT_HISTOGRAM_CROSSMIN)
          fdc.push( 
                    "neib[0] = mix(neib[0], fneiprec, step(neib[0], fneiprec)*(1.0 - fhit.x));" SHNL
                    "neib[1] = mix(fneiprec, neib[1], neib[2]*(1.0 - step(fneiprec, neib[1])*fhit.x));" SHNL
                    "neib[2] = mix(fhit.x, 1.0, neib[2]);" SHNL   /// + 0.0*fhit.z

                    "fneiprec = (fhit.z + neib[2])*step(neib[0], fneiprec)*step(floor(fneiprec), neib[1]);" SHNL  /// reassign!!
                    "mixwellp = mixwellp*fneiprec;" SHNL
                    "fhit = fhit*fneiprec;" SHNL
                    );
          
        if (coloropts.cpolicy != CP_REPAINTED)
          fdc.push("float VALCLR = clamp(bself_datasc/bv_datasc[1], 0.0, 1.0);" SHNL);
        else
          fdc.push("float VALCLR = bv[1];" SHNL);
      } // histogram
      
      if (isInterp || isHistogram)
      {
        if (graphopts.postrect == PR_STANDARD)
          fdc.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL);
        if (graphopts.postrect == PR_VALUEONLY)
          fdc.push("post_mask[0] = post_mask[0]*(1.0 - fhit.y)*(1.0 - fhit.x) + fhit.y*fhit.x;" SHNL);
        else if (graphopts.postrect == PR_VALUEAROUND)
          fdc.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
//                   "imrect.ga = int(1.0 - fhit.x)*imrect.ga + int(fhit.x)*ivec2(bself_viewsc - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
                   "imrect.ga = int(1.0 - fhit.x)*imrect.ga + int(fhit.x)*ivec2(bself_viewsc + fhit_rect[0], fhit_rect[1]);" SHNL);
        else if (graphopts.postrect == PR_SUMMARY)
          fdc.push("post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);" SHNL
//                   "imrect.ga = int(1.0 - (fhit.x - fhit.y))*imrect.ga + int(fhit.x - fhit.y)*ivec2(bself_viewsc - fhit_rect[0], fhit_rect[1] - fhit_rect[0]);" SHNL);
                   "imrect.ga = int(1.0 - (fhit.x - fhit.y))*imrect.ga + int(fhit.x - fhit.y)*ivec2(bself_viewsc + fhit_rect[0], fhit_rect[1]);" SHNL);
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
          fdc.push(descaling[int(graphopts.descaling)]);
      }
      
      if (graphopts.dotsize)
      {       
        fdc.push( "{" SHNL
                    "float hscx = floor(ab_iscaler.x/2.0 + 0.49);" SHNL);
        if (graphopts.dotsize > 0)
          fdc.cintvar("dotsize", graphopts.dotsize);
        else
        {
          fdc.cintvar("godot", -graphopts.dotsize - 1);
          fdc.push("int dotsize = max(int(hscx) + godot, 0);" SHNL);
        }
        fdc.push("int dist_limit = int(max((dotsize-1) - hscx + 1, 0.0));" SHNL);
        fdc.cfloatvar("dotsmooth", graphopts.dotsmooth);
        
        if (deinterpSL)
        {
          fdc.push("float demoded_x = float(abc_coords.x*ab_ibounds.x - immod.x)/ab_viewdm.x*ab_indimms.x + (step(ab_iscaler.x/2.0, float(immod.x)));" SHNL
                   "float demoded_offs = ab_iscaler.x/2.0 - 0.5;" SHNL
                   );
        }
        else
          fdc.push("float demoded_x = float(abc_coords.x*ab_ibounds.x - immod.x)/ab_viewdm.x*ab_indimms.x;" SHNL
                   "float demoded_offs = 0.0;" SHNL
                   );
        
        fdc.push(   "for (int j=-dist_limit; j<=dist_limit; j++)" SHNL
                    "{" SHNL
                      "float ox = float(demoded_x + j)/ab_indimms.x;" SHNL
                      "float o_VALCLR = getValue1D(i, ox);" SHNL
//                      "float fdist_weight = (dotsize - distance(fcoords, vec2( floor(ox*ab_viewdm.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) ) / float(dotsize);" SHNL
//                      "float fdist_weight = (dotsize - distance(abc_coords*ab_viewdm, vec2( floor(ox*ab_viewdm.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) ) / float(dotsize);" SHNL
//                      "float fdist_weight = 1.0 - distance(abc_coords*ab_viewdm, vec2( floor(ox*ab_viewdm.x) - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) / float(dotsize);" SHNL
                      "float fdist_weight = 1.0 - distance(abc_coords*ab_viewdm, vec2( ox*ab_viewdm.x - demoded_offs + hscx, floor(o_VALCLR*(ab_ibounds.y-1)))) / float(dotsize);" SHNL
                      "fdist_weight = min(fdist_weight*(1.0 + dotsmooth), 1.0);" SHNL
                      "VALCLR = mix(o_VALCLR, VALCLR, step(fdist_weight, mixwellp));" SHNL // calc before new mixwellp
                      "mixwellp = mix(fdist_weight, mixwellp, step(fdist_weight, mixwellp));" SHNL
                    "}" SHNL                    
                  "}" SHNL);
      }
      
      {
        fdc.cfloatvar("paletrange", coloropts.cstart, coloropts.cstop);
        if ( (fdc.splits()  & SPFLAG_COLORSPLIT) == 0 )
        {
          switch (coloropts.cpolicy)
          {
          case CP_MONO:                 fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i);" SHNL); break;
          case CP_PAINTED:              fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 1.0 - VALCLR);" SHNL); break;
          case CP_PAINTED_GROSS:        fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 1.0 - sqrt(VALCLR));" SHNL); break;
          case CP_PAINTED_SYMMETRIC:    fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i + 0.5 - abs(VALCLR - 0.5));" SHNL); break;
          case CP_REPAINTED:            fdc.push("float porc = (paletrange[1] - (paletrange[1] - paletrange[0])*(float(allocatedPortions - 1 - i)/float(allocatedPortions)))*VALCLR;" SHNL); break;
            
          case CP_PALETTE:              fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])*bself_datasc/ab_datadm.y;" SHNL); break;
          case CP_PALETTE_SPLIT:        fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])/float(allocatedPortions)*(i + bself_datasc/ab_datadm.y);" SHNL); break;
          }
        }
        else
        {
          switch (coloropts.cpolicy)
          {
          case CP_MONO:                 fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*0;" SHNL); break;
          case CP_PAINTED:              fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(1.0 - VALCLR);" SHNL); break;
          case CP_PAINTED_GROSS:        fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(1.0 - sqrt(VALCLR));" SHNL); break;
          case CP_PAINTED_SYMMETRIC:    fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(0.5 - abs(VALCLR - 0.5));" SHNL); break;
          case CP_REPAINTED:            fdc.push("float porc = (paletrange[1] - (paletrange[1] - paletrange[0])*(0)*VALCLR;" SHNL); break;
            
          case CP_PALETTE:              fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])*bself_datasc/ab_datadm.y;" SHNL); break;
          case CP_PALETTE_SPLIT:        fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 + bself_datasc/ab_datadm.y);" SHNL); break;
          }
        }
//        {
//          switch (coloropts.cpolicy)
//          {
//          case CP_MONO:                 fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1);" SHNL); break;
//          case CP_PAINTED:              fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 + 1.0 - VALCLR);" SHNL); break;
//          case CP_PAINTED_GROSS:        fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 + 1.0 - sqrt(VALCLR));" SHNL); break;
//          case CP_PAINTED_SYMMETRIC:    fdc.push("float porc = paletrange[1] - (paletrange[1] - paletrange[0])/float(allocatedPortions)*(allocatedPortions - 1 + 0.5 - abs(VALCLR - 0.5));" SHNL); break;
//          case CP_REPAINTED:            fdc.push("float porc = (paletrange[1] - (paletrange[1] - paletrange[0])*(float(allocatedPortions - 1)/float(allocatedPortions)))*VALCLR;" SHNL); break;
            
//          case CP_PALETTE:              fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])*bself_datasc/ab_datadm.y;" SHNL); break;
//          case CP_PALETTE_SPLIT:        fdc.push("float porc = paletrange[0] + (paletrange[1] - paletrange[0])/float(allocatedPortions)*(bself_datasc/ab_datadm.y);" SHNL); break;
//          }
//        }
          
      }

      fdc.push(  "vec3  colorGraph = texture(paletsampler, vec2(porc, 0.0)).rgb;" SHNL );
      
      if (graphopts.graphtype == GT_HISTOGRAM_LASTBACK)
        fdc.push("result = mix(result, mix(colorGraph, backcolor, step(float(dataportions), float(i+1))), mixwellp);" SHNL );
      else if (graphopts.graphtype == GT_HISTOGRAM_SUM)
        fdc.push("result = result + colorGraph*vec3(mixwellp);" SHNL );
      else
        fdc.push("result = mix(result, colorGraph, mixwellp);" SHNL );
      
      fdc.push(  "mixwell = max(mixwell, mixwellp);" SHNL);
      
      fdc.push(  "dvalue = mix(bv[1], dvalue, step(abs(bself_viewsc - dvalue), abs(bself_viewsc - bv[1])) );" SHNL);
      fdc.push("}" SHNL ); // for
      
//      fdc.push("dvalue = mix(dvalue, bself_viewsc, step(mixwell, 0.0));" SHNL);
    }
  }
};


SheiGeneratorGraph::~SheiGeneratorGraph()
{
}

DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, unsigned int memForDeploy, const graphopts_t& graphopts, const coloropts_t& coloropts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new SheiGeneratorGraph(graphopts, coloropts), graphs, OR_LRBT, splitGraphs, coloropts.backcolor)
{
  m_dataDimmA = samples;
  m_dataDimmB = 1;
  m_portionSize = samples;
  deployMemory(memForDeploy);
}


void DrawGraph::reConstructor(unsigned int samples)
{
  m_dataDimmA = samples;
  m_dataDimmB = 1;
  m_portionSize = samples;
  deployMemory();
}

/// m_countPortions === graphs
DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new SheiGeneratorGraph(graphopts_t::goInterp(0.0f, DE_LINTERP), copts), graphs, OR_LRBT, splitGraphs, copts.backcolor)
{ reConstructor(samples); }

DrawGraph::DrawGraph(unsigned int samples, unsigned int graphs, const graphopts_t& graphopts, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_1D, new SheiGeneratorGraph(graphopts, copts), graphs, OR_LRBT, splitGraphs, copts.backcolor)
{ reConstructor(samples); }

const graphopts_t& DrawGraph::graphopts() const {  return ((SheiGeneratorGraph*)m_pcsh)->graphopts;   }
const coloropts_t& DrawGraph::coloropts() const {  return ((SheiGeneratorGraph*)m_pcsh)->coloropts;   }

void DrawGraph::setOpts(const graphopts_t& go)
{
  ((SheiGeneratorGraph*)m_pcsh)->graphopts = go;
  vmanUpInit();
}

void DrawGraph::setOpts(const coloropts_t& co)
{
  ((SheiGeneratorGraph*)m_pcsh)->coloropts = co;
  vmanUpInit();
}

void DrawGraph::setOpts(const graphopts_t& go, const coloropts_t& co)
{
  ((SheiGeneratorGraph*)m_pcsh)->graphopts = go;
  ((SheiGeneratorGraph*)m_pcsh)->coloropts = co;
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

////////////////////////////////////////////////////////////////

void DrawGraphUpsizeA::reConstructorEx(unsigned int samples)
{
  m_dataDimmA = samples;
  m_portionSize = samples;
}

DrawGraphUpsizeA::DrawGraphUpsizeA(unsigned int minsamples, unsigned int maxsamples, unsigned int graphs, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawGraph(maxsamples, graphs, copts, splitGraphs), m_minsamples(minsamples), m_maxsamples(maxsamples)
{
  reConstructorEx(minsamples);
}

DrawGraphUpsizeA::DrawGraphUpsizeA(unsigned int minsamples, unsigned int maxsamples, unsigned int graphs, const graphopts_t& graphopts, const coloropts_t& copts, SPLITPORTIONS splitGraphs):
  DrawGraph(maxsamples, graphs, graphopts, copts, splitGraphs), m_minsamples(minsamples), m_maxsamples(maxsamples)
{
  reConstructorEx(minsamples);
}

void DrawGraphUpsizeA::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
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

int DrawGraphUpsizeA::sizeAndScaleChanged(bool changedDimmA, bool /*changedDimmB*/, bool changedScalingA, bool /*changedScalingB*/)
{
  if (changedDimmA || changedScalingA)
  {
    unsigned int new_portionSize = sizeDataA();
    if (m_portionSize != new_portionSize)
    {
      m_portionSize = new_portionSize;
      emit sig_portionDimmChanged();
      return PC_DATADIMMS;
    }
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

int DrawGraphMoveEx::sizeAndScaleChanged(bool changedDimmA, bool /*changedDimmB*/, bool changedScalingA, bool /*changedScalingB*/)
{
  if (changedDimmA || changedScalingA)
  {
//    bool more = m_portionSize < m_dataDimmA;
    m_portionSize = m_dataDimmA;
//    if (more)
    {
      clampFilloffset();
      fillMatrix();
    }
    return PC_DATA | PC_DATADIMMS;
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

