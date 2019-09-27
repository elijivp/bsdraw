#ifndef SHEIGEN2D_H
#define SHEIGEN2D_H

#include "../bsidrawcore.h"
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
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, ORIENTATION orient, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, orient, ovlscount, ovlsinfo);

    if (dsup != DS_NONE)
      fmg.push( "uniform highp sampler2D domainarr;"
                "uniform highp int       domainscount;" );

    fmg.goto_func_begin(FshMainGenerator::INITBACK_BYPALETTE, 0, fsp);
    {
      fmg.cintvar("allocatedPortions", (int)allocatedPortions);
      const char fsh_bright[] =         "for (int i=0; i<countPortions; i++)"
                                        "{"
                                          "float value = getValue2D(i, relcoords);"
                                          "ovMix = max(ovMix, value);"
                                          "result = result + texture(texPalette, vec2(value, float(i) / float(allocatedPortions - 1))).rgb;"
                                          "ppb_sfp[0] = mix(1.0, ppb_sfp[0], step(value, ppb_sfp[1]));"
                                        "}";

      const char fsh_domain[] =         "for (int i=0; i<countPortions; i++)"
                                        "{"
                                          "float domain = texture(domainarr, relcoords).r;"
                                          "float value = texture(texData, vec2(domain, 0.0)).r;"  // domain /float(domainscount-1)
                                          "ovMix = max(ovMix, value);"
                                          "result = result + texture(texPalette, vec2(value, float(i) / (allocatedPortions - 1))).rgb;"
                                          "ppb_sfp[0] = mix(1.0, ppb_sfp[0], step(value, ppb_sfp[1]));"
                                        "}";

      const char fsh_domblack[] =       "for (int i=0; i<countPortions; i++)"
                                        "{"
                                          "float domain = texture(domainarr, relcoords).r;"
                                          "float value = texture(texData, vec2(domain, 0.0)).r * (1-step(domain, 0.0));"    // /float(domainscount-1)
                                          "ovMix = max(ovMix, value);"
                                          "result = result + texture(texPalette, vec2(value, float(i) / (allocatedPortions - 1))).rgb;"
                                          "ppb_sfp[0] = mix(1.0, ppb_sfp[0], step(value, ppb_sfp[1]));"
                                        "}";

      if (dsup == DS_NONE)          fmg.push(fsh_bright);
      else if (dsup == DS_DOMSTD)   fmg.push(fsh_domain);
      else if (dsup == DS_DOMBLACK) fmg.push(fsh_domblack);
    }
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};

#endif // SHEIGEN2D_H
