/// This file contains special derived class for show images
/// DrawHint with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawHint with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
/// Created By: Elijah Vlasov
#include "bsdrawhint.h"

#include "core/sheigen/bsshgenmain.h"
#include "core/bsgraphopts.h"
#include "bsdrawgraph.h"

class ISheiGeneratorHint_Base: public ISheiGenerator
{
public:
  virtual const char*   shaderName() const {  return "HINT"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 750 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
};

#ifndef SHNL
#define SHNL "\n"
#endif

class SheiGeneratorHint_Graph: public ISheiGeneratorHint_Base
{
private:
  unsigned int     m_portion, m_allocatedPortions;
  BSCOLORPOLICY m_cpolicy;
private:
  int     m_automargin;
  DRAWHINT  m_type;
  int     m_subcount;
public:
  int     m_mindimmA, m_mindimmB;
  unsigned int  m_bckclr;
public:
  SheiGeneratorHint_Graph(int flags, unsigned int portion, unsigned int allocatedPortions, BSCOLORPOLICY cp, unsigned int backgroundColor): 
    m_portion(portion), m_allocatedPortions(allocatedPortions),
    m_cpolicy(cp), m_bckclr(backgroundColor)
  {
    m_automargin = (flags >> 8) & 0xF;
    m_type = DRAWHINT(flags & 0xF0);
    m_subcount = flags & 0x0F;
    if (m_subcount == 0)  m_subcount = 1;
    m_mindimmA = m_mindimmB = 1 + m_automargin*2;
    switch (m_type)
    {
    case DH_SAW: case DH_TRIANGLE: case DH_MEANDER:  m_mindimmB += 8; break;
    default: break;
    }
  }
  ~SheiGeneratorHint_Graph();
public:
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, m_bckclr, orient, fsp);
    fmg.push("ivec2 icoords = ivec2(abc_coords*(ab_ibounds));" SHNL);
    
    if (m_automargin)
    {
      fmg.cintvar("am", m_automargin);
      fmg.push(
                "ivec2 im = ivec2(min(am, ab_ibounds.x/2), min(am, ab_ibounds.y/2));" SHNL
                "ab_ibounds = ab_ibounds - 2*im;" SHNL
                "icoords = icoords - im;" SHNL
//                "mixwell = mixwell*step(0.0, float(icoords.x))*step(0.0, float(icoords.y))*step(float(icoords.x), float(ab_ibounds.x))*step(float(icoords.y), float(ab_ibounds.y));" SHNL
                "mixwell = mixwell*step(0.0, float(icoords.x))*step(0.0, float(icoords.y))*(1.0 - step(float(ab_ibounds.x), float(icoords.x)))*(1.0 - step(float(ab_ibounds.y), float(icoords.y)));" SHNL
            );
    }
    
    fmg.cintvar("i", (int)m_portion);
    
//    if (m_allocatedPortions > 1)
//    {
//      fmg.cintvar("allocatedPortions", (int)m_allocatedPortions);
//      switch (m_cpolicy)
//      {
//      case CP_MONO:               fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(i);" SHNL); break;
//      case CP_PAINTED:             fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(i + 1.0 - VALCLR);" SHNL); break;
//      case CP_PAINTED_GROSS:       fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(i + 1.0 - sqrt(VALCLR));" SHNL); break;
//      case CP_PAINTED_SYMMETRIC:   fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(i + 0.5 - abs(VALCLR - 0.5) );" SHNL); break;
//      case CP_REPAINTED:                fmg.push("float portionColor = (palrange[1] - (palrange[1] - palrange[0])*(float(i)/float(allocatedPortions)))*VALCLR;" SHNL); break;
//      case CP_PALETTE:           fmg.push("float portionColor = palrange[0] + (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
//      case CP_PALETTE_SPLIT:      fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*float(i)/float(allocatedPortions)*b_coord_ns/ab_fndimms.y;" SHNL); break;
//      }
//    }
//    else
//    {
//      switch (m_cpolicy)
//      {
//      case CP_MONO:               fmg.push("float portionColor = palrange[1];" SHNL); break;
//      case CP_PAINTED:             fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*VALCLR;" SHNL); break;
//      case CP_PAINTED_GROSS:       fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*sqrt(VALCLR);" SHNL); break;
//      case CP_PAINTED_SYMMETRIC:   fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*(1.0 - 0.5 + abs(VALCLR - 0.5));" SHNL); break;
//      case CP_REPAINTED:                fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*(1.0 - VALCLR);" SHNL); break;
//      case CP_PALETTE:           fmg.push("float portionColor = palrange[0] + (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
//      case CP_PALETTE_SPLIT:      fmg.push("float portionColor = palrange[1] - (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
//      }
//    }
    
    if (m_allocatedPortions > 1 && (splitPortions & SPFLAG_COLORSPLIT) == 0)
    {
      fmg.cintvar("allocatedPortions", (int)m_allocatedPortions);
      switch (m_cpolicy)
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
      switch (m_cpolicy)
      {
      case CP_MONO:                 fmg.push("float porc = palrange[1];" SHNL); break;
      case CP_PAINTED:              fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - VALCLR);" SHNL); break;
      case CP_PAINTED_GROSS:        fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*sqrt(1.0 - VALCLR);" SHNL); break;
      case CP_PAINTED_SYMMETRIC:    fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - 0.5 + abs(VALCLR - 0.5));" SHNL); break;
      case CP_REPAINTED:            fmg.push("float porc = palrange[1] - (palrange[1] - palrange[0])*(1.0 - VALCLR);" SHNL); break;
        
      case CP_PALETTE: case CP_PALETTE_SPLIT:      fmg.push("float porc = palrange[0] + (palrange[1] - palrange[0])*b_coord_ns/ab_fndimms.y;" SHNL); break;
      }
    }
    
    fmg.push( "float distwell = 0.0;"  SHNL );
    
    if (m_type == DH_LINE)
    {
      fmg.push(   
                  "float center = float(ab_ibounds.y/2);" SHNL
                  "distwell = abs(icoords.y - center);" SHNL
                  );
    }
    else if (m_type == DH_SAW || m_type == DH_TRIANGLE || m_type == DH_MEANDER)
    {
      fmg.cintvar("icnt", m_subcount);
      fmg.push(   "if (ab_ibounds.x > 1 && ab_ibounds.y > 1)" SHNL
                  "{" SHNL
                    "float window = ab_ibounds.x/float(icnt);" SHNL
                    "float fx = mod(float(icoords.x), window);" SHNL
      );
      {
        if (m_type == DH_SAW)
        {
          fmg.push( "float nl = length(vec2(window, ab_ibounds.y));" SHNL
                    "distwell = abs(fx*float(ab_ibounds.y)/nl - icoords.y*float(window)/nl);" SHNL
                    "distwell = min(distwell, abs(fx));" SHNL
          );
        }
        else if (m_type == DH_TRIANGLE)
        {
          fmg.push( "float nl = length(vec2(window, 2*ab_ibounds.y));" SHNL
                    "distwell = abs((fx-window/2)*2*float(ab_ibounds.y)/nl - icoords.y*float(window)/nl);" SHNL
                    "distwell = min(distwell, abs(fx*2*float(ab_ibounds.y)/nl - (ab_ibounds.y - icoords.y)*float(window)/nl));" SHNL
          );
        }
        else if (m_type == DH_MEANDER)
        {
          fmg.push( 
                    "float fodd = mod(float(int(float(icoords.x)/window)), 2.0);" SHNL
                    "distwell = mix(abs(icoords.y - 0), abs(ab_ibounds.y - 1 - icoords.y), fodd);" SHNL
                    "distwell = min(distwell, abs(fx));" SHNL
          );
        }
      }
      fmg.push(   "}" SHNL 
      );
    }
    else if (m_type == DH_DIAGONAL)
    {
      fmg.push(   "if (ab_ibounds.x > 1 && ab_ibounds.y > 1)" SHNL
                  "{" SHNL
//                    "float k = float(ab_ibounds.y)/ab_ibounds.x;" SHNL
//                    "distwell = abs(icoords.y - k*icoords.x);" SHNL
                    "float nl = length(vec2(ab_ibounds-ivec2(1,1)));" SHNL
                    "distwell = abs(icoords.x*float(ab_ibounds.y-1)/nl - icoords.y*float(ab_ibounds.x-1)/nl);" SHNL
                  "}" SHNL
            );
    }
    else if (m_type == DH_FILL) 
      ; /// distwell == 0
    
    fmg.push(   
                "mixwell = mixwell*step(distwell, 7.0)*(1.0-distwell/1.5);" SHNL
                "mixwell = max(0.0, mixwell);" SHNL
                "vec3  colorGraph = texture(texpalette, vec2(porc, 0.0)).rgb;" SHNL
                "result = mix(result, colorGraph, mixwell);" SHNL
    );
    
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorHint_Graph::~SheiGeneratorHint_Graph(){}

class SheiGeneratorHint_Intensity: public ISheiGeneratorHint_Base
{
  float   m_value;
public:
  int     m_mindimmA, m_mindimmB;
  unsigned int  m_bckclr;
public:
  SheiGeneratorHint_Intensity(float value, unsigned int backgroundColor): 
    m_value(value), m_bckclr(backgroundColor)
  {
  }
  ~SheiGeneratorHint_Intensity();
public:
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, m_bckclr, orient, fsp);
    fmg.push( "ivec2 icoords = ivec2(abc_coords*(ab_ibounds));" SHNL);
    
    fmg.cfloatvar("portionColor", m_value);
    fmg.push( "float distwell = 0.0;"  SHNL );
    fmg.push( "vec3  colorGraph = texture(texpalette, vec2(palrange[0] + (palrange[1] - palrange[0])*portionColor, 0.0)).rgb;" SHNL
              "result = mix(result, colorGraph, mixwell);" SHNL
    );
    
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorHint_Intensity::~SheiGeneratorHint_Intensity(){}

DrawHint::DrawHint(const DrawGraph* pdg, int portion, int flags, ORIENTATION orient, unsigned int backgroundColor):
  DrawQWidget(DATEX_2D, 
              new SheiGeneratorHint_Graph(flags, (unsigned int)portion, pdg->allocatedPortions(), pdg->coloropts().cpolicy, 
                                           backgroundColor == 0xFFFFFFFF? pdg->coloropts().backcolor : backgroundColor)
              , 1, orient)
{
  m_dataDimmA = 1;
  m_dataDimmB = 1;
  m_portionSize = 1;
  
  SheiGeneratorHint_Graph* shs = (SheiGeneratorHint_Graph*)m_pcsh;
  if (m_dataDimmSwitchAB)
    setMinimumSize(shs->m_mindimmB, shs->m_mindimmA);
  else
    setMinimumSize(shs->m_mindimmA, shs->m_mindimmB);
  
//  int automargin = 32;
//  if (automargin) this->setContentsMargins(automargin, automargin, automargin, automargin);
  deployMemory();
  setDataPaletteRange(pdg->coloropts().cstart, pdg->coloropts().cstop);
}

DrawHint::DrawHint(float value, ORIENTATION orient, unsigned int backgroundColor):
  DrawQWidget(DATEX_2D, 
              new SheiGeneratorHint_Intensity(value, backgroundColor)
              , 1, orient)
{
  m_dataDimmA = 1;
  m_dataDimmB = 1;
  m_portionSize = 1;
  deployMemory();
}

void DrawHint::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}

unsigned int DrawHint::colorBack() const
{
  unsigned int bc = ((SheiGeneratorHint_Graph*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}
