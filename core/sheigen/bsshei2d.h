#ifndef SHEIGEN2D_H
#define SHEIGEN2D_H
/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "bsshgenmain.h"

class SheiGeneratorBright: public ISheiGenerator
{
public:
  enum            DOMAINSUPPORT { DS_NONE, DS_DOMSTD, DS_DOMBLACK };
protected:
  DOMAINSUPPORT   dsup;
public:
  SheiGeneratorBright(DOMAINSUPPORT ds): dsup(ds) {}
  virtual const char*   shaderName() const {  return "BRIGHT"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 700 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);

    if (dsup != DS_NONE)
      fmg.push( "uniform highp sampler2D texground;"
                "uniform highp int       lenground;" );

    fmg.main_begin(FshDrawConstructor::INITBACK_BYPALETTE, 0, orient, fsp); //FshDrawConstructor::INITBACK_BYZERO
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);
    fmg.push( splitPortions == SP_NONE? "for (int i=0; i<portions; i++)" : "int i = explicitPortion;" );
    fmg.push( "{" );
    {
      if (dsup == DS_NONE)
        fmg.value2D("float value");
      else if (dsup == DS_DOMSTD)
        fmg.push(
                  "float domain = texture(texground, abc_coords).r;"
                  "float value = texture(texdata, vec2(domain, 0.0)).r;"  // domain /float(lenground-1)
                );
      else if (dsup == DS_DOMBLACK)
        fmg.push(
                  "float domain = texture(texground, abc_coords).r;"
                  "float value = texture(texdata, vec2(domain, 0.0)).r * (1-step(domain, 0.0));"    // /float(lenground-1)
                );
      
      fmg.push(  "dvalue = max(dvalue, value);");
      fmg.push(  "value = palrange[0] + (palrange[1] - palrange[0])*value;" );
      
      if ( splitPortions == SP_NONE )
        fmg.push("result = result + texture(texpalette, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" );
      else if (splitPortions & SPFLAG_COLORSPLIT)
        fmg.push("result = result + texture(texpalette, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" );
//        fmg.push("result.rgb = mix(texture(texpalette, vec2(value, float(i)/(allocatedPortions-1))).rgb, result.rgb, step(countPortions, float(explicitPortion)));" );
      else
        fmg.push("result.rgb = mix(texture(texpalette, vec2(value, 0.0)).rgb, result.rgb, step(countPortions, float(explicitPortion)));" );
      
      fmg.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" );
    }
    fmg.push( "}" );
    
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};

#endif // SHEIGEN2D_H
