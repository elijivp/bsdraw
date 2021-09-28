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
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 600 + FshMainGenerator::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, ORIENTATION orient, 
                                         SPLITPORTIONS splitPortions, const impulsedata_t& imp, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);

    if (dsup != DS_NONE)
      fmg.push( "uniform highp sampler2D texGround;"
                "uniform highp int       countGround;" );

    fmg.main_begin(FshMainGenerator::INITBACK_BYPALETTE, 0, orient, fsp); //FshMainGenerator::INITBACK_BYZERO
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);
    fmg.push( splitPortions == SL_NONE? "for (int i=0; i<countPortions; i++)" : "int i=icell[0];" );
    fmg.push( "{" );
    {
      if (dsup == DS_NONE)
      {
        fmg.value2D("float value");
        fmg.push("ovMix = max(ovMix, value);");
      }
      else if (dsup == DS_DOMSTD)
        fmg.push(
                  "float domain = texture(texGround, relcoords).r;"
                  "float value = texture(texData, vec2(domain, 0.0)).r;"  // domain /float(countGround-1)
                  "ovMix = max(ovMix, value);"
                );
      else if (dsup == DS_DOMBLACK)
        fmg.push(
                  "float domain = texture(texGround, relcoords).r;"
                  "float value = texture(texData, vec2(domain, 0.0)).r * (1-step(domain, 0.0));"    // /float(countGround-1)
                  "ovMix = max(ovMix, value);"
                );
      
      if ( splitPortions == SL_NONE )
        fmg.push( "result = result + texture(texPalette, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" );
      else
        fmg.push( "result.rgb = mix(texture(texPalette, vec2(value, 0.0)).rgb, result.rgb, step(countPortions, float(icell[0])));" );
      
      fmg.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" );
    }
    fmg.push( "}" );
    
    fmg.main_end(fsp);
    return fmg.written();
  }
};

#endif // SHEIGEN2D_H
