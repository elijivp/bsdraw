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
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 750 + FshMainGenerator::basePendingSize(imp, ovlscount); }
};

#ifndef SHNL
#define SHNL "\n"
#endif

class SheiGeneratorHint_Single: public ISheiGeneratorHint_Base
{
private:
  float   m_start, m_stop;
  int     m_portion, m_allocatedPortions;
  BSCOLORPOLICY m_cpolicy;
private:
  int     m_automargin;
  DRAWHINT  m_type;
  int     m_subcount;
public:
  int     m_mindimmA, m_mindimmB;
  unsigned int  m_bckclr;
public:
  SheiGeneratorHint_Single(int flags, float start, float stop, int portion, int allocatedPortions, BSCOLORPOLICY cp, unsigned int backgroundColor): 
    m_start(start), m_stop(stop), 
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
  ~SheiGeneratorHint_Single();
public:
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, 
                                         ORIENTATION orient, SPLITPORTIONS splitPortions, const impulsedata_t& imp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.main_begin(FshMainGenerator::INIT_BYVALUE, m_bckclr, orient, fsp);
    
    
    fmg.push( "ivec2 icoords = ivec2(relcoords*(ibounds));" SHNL);
    fmg.push( "float mixwell = 1.0;"  SHNL );
    
    if (m_automargin)
    {
      fmg.cintvar("am", m_automargin);
      fmg.push(
                "ivec2 im = ivec2(min(am, ibounds.x/2), min(am, ibounds.y/2));" SHNL
                "ibounds = ibounds - 2*im;" SHNL
                "icoords = icoords - im;" SHNL
//                "mixwell = mixwell*step(0.0, float(icoords.x))*step(0.0, float(icoords.y))*step(float(icoords.x), float(ibounds.x))*step(float(icoords.y), float(ibounds.y));" SHNL
                "mixwell = mixwell*step(0.0, float(icoords.x))*step(0.0, float(icoords.y))*(1.0 - step(float(ibounds.x), float(icoords.x)))*(1.0 - step(float(ibounds.y), float(icoords.y)));" SHNL
            );
    }
    
    float base = m_start;
    float interval = m_stop - m_start;
    fmg.cfloatvar("pcBase", base);
    fmg.cfloatvar("pcInterval", interval);
    fmg.cintvar("i", m_portion);
    
    if (m_allocatedPortions > 1)
    {
      fmg.cintvar("allocatedPortions", m_allocatedPortions);
      if (m_cpolicy == CP_SINGLE)                   fmg.push("float portionColor = pcBase + pcInterval*(float(i)/float(allocatedPortions-1));" SHNL);                   /// -1!
      else if (m_cpolicy == CP_OWNRANGE)            fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 1.0 - VALCLR);" SHNL);             /// not -1!
      else if (m_cpolicy == CP_OWNRANGE_GROSS)      fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 1.0 - sqrt(VALCLR));" SHNL);       /// not -1!
      else if (m_cpolicy == CP_OWNRANGE_SYMMETRIC)  fmg.push("float portionColor = pcBase + pcInterval/float(allocatedPortions)*(i + 0.5 - abs(VALCLR - 0.5) );" SHNL); /// not -1!
      else if (m_cpolicy == CP_RANGE)               fmg.push("float portionColor = (pcBase + pcInterval*(float(i)/float(allocatedPortions)))*VALCLR;" SHNL);            /// not -1!
      else if (m_cpolicy == CP_SUBPAINTED)          fmg.push("float portionColor = fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
      else if (m_cpolicy == CP_RANGESUBPAINTED)     fmg.push("float portionColor = pcBase + pcInterval*float(i)/float(allocatedPortions)*fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
    }
    else
    {
      if (m_cpolicy == CP_SINGLE)                   fmg.push("float portionColor = pcBase;" SHNL);
      else if (m_cpolicy == CP_OWNRANGE)            fmg.push("float portionColor = pcBase + pcInterval*VALCLR;" SHNL);
      else if (m_cpolicy == CP_OWNRANGE_GROSS)      fmg.push("float portionColor = pcBase + pcInterval*sqrt(VALCLR);" SHNL);
      else if (m_cpolicy == CP_OWNRANGE_SYMMETRIC)  fmg.push("float portionColor = pcBase + pcInterval*(1.0 - 0.5 + abs(VALCLR - 0.5));" SHNL);
      else if (m_cpolicy == CP_RANGE)               fmg.push("float portionColor = pcBase + pcInterval*VALCLR;" SHNL);
      else if (m_cpolicy == CP_SUBPAINTED)          fmg.push("float portionColor = fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
      else if (m_cpolicy == CP_RANGESUBPAINTED)     fmg.push("float portionColor = pcBase + pcInterval*fcoords_noscaled.y/fbounds_noscaled.y;" SHNL);
    }
    
    fmg.push( "float distwell = 0.0;"  SHNL );
    
    if (m_type == DH_LINE)
    {
      fmg.push(   
                  "float center = float(ibounds.y/2);" SHNL
                  "distwell = abs(icoords.y - center);" SHNL
                  );
    }
    else if (m_type == DH_SAW || m_type == DH_TRIANGLE || m_type == DH_MEANDER)
    {
      fmg.cintvar("icnt", m_subcount);
      fmg.push(   "if (ibounds.x > 1 && ibounds.y > 1)" SHNL
                  "{" SHNL
                    "float window = ibounds.x/float(icnt);" SHNL
                    "float fx = mod(float(icoords.x), window);" SHNL
      );
      {
        if (m_type == DH_SAW)
        {
          fmg.push( "float nl = length(vec2(window, ibounds.y));" SHNL
                    "distwell = abs(fx*float(ibounds.y)/nl - icoords.y*float(window)/nl);" SHNL
                    "distwell = min(distwell, abs(fx));" SHNL
          );
        }
        else if (m_type == DH_TRIANGLE)
        {
          fmg.push( "float nl = length(vec2(window, 2*ibounds.y));" SHNL
                    "distwell = abs((fx-window/2)*2*float(ibounds.y)/nl - icoords.y*float(window)/nl);" SHNL
                    "distwell = min(distwell, abs(fx*2*float(ibounds.y)/nl - (ibounds.y - icoords.y)*float(window)/nl));" SHNL
          );
        }
        else if (m_type == DH_MEANDER)
        {
          fmg.push( 
                    "float fodd = mod(float(int(float(icoords.x)/window)), 2.0);" SHNL
                    "distwell = mix(abs(icoords.y - 0), abs(ibounds.y - 1 - icoords.y), fodd);" SHNL
                    "distwell = min(distwell, abs(fx));" SHNL
          );
        }
      }
      fmg.push(   "}" SHNL 
      );
    }
    else if (m_type == DH_DIAGONAL)
    {
      fmg.push(   "if (ibounds.x > 1 && ibounds.y > 1)" SHNL
                  "{" SHNL
//                    "float k = float(ibounds.y)/ibounds.x;" SHNL
//                    "distwell = abs(icoords.y - k*icoords.x);" SHNL
                    "float nl = length(vec2(ibounds-ivec2(1,1)));" SHNL
                    "distwell = abs(icoords.x*float(ibounds.y-1)/nl - icoords.y*float(ibounds.x-1)/nl);" SHNL
                  "}" SHNL
            );
    }
    else if (m_type == DH_FILL) 
      ; /// distwell == 0
    
    fmg.push(   
                "mixwell = mixwell*step(distwell, 7.0)*(1.0-distwell/1.5);" SHNL
                "mixwell = max(0.0, mixwell);" SHNL
                "vec3  colorGraph = texture(texPalette, vec2(portionColor, 0.0)).rgb;" SHNL
                "result = mix(result, colorGraph, mixwell);" SHNL
    );
    
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorHint_Single::~SheiGeneratorHint_Single(){}

DrawHint::DrawHint(const DrawGraph* pdg, int portion, int flags, ORIENTATION orient, unsigned int backgroundColor):
  DrawQWidget(DATEX_2D, 
              new SheiGeneratorHint_Single(flags, pdg->coloropts().cstart, pdg->coloropts().cstop, 
                                           portion, pdg->allocatedPortions(), pdg->coloropts().cpolicy, 
                                           backgroundColor == 0xFFFFFFFF? pdg->coloropts().backcolor : backgroundColor)
              , 1, orient)
{
  m_matrixDimmA = 1;
  m_matrixDimmB = 1;
  m_portionSize = 1;
  
  SheiGeneratorHint_Single* shs = (SheiGeneratorHint_Single*)m_pcsh;
  if (m_matrixSwitchAB)
    setMinimumSize(shs->m_mindimmB, shs->m_mindimmA);
  else
    setMinimumSize(shs->m_mindimmA, shs->m_mindimmB);
  
//  int automargin = 32;
//  if (automargin) this->setContentsMargins(automargin, automargin, automargin, automargin);
  deployMemory();
}

void DrawHint::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);
  clampScaling(scalingA, scalingB);
}

unsigned int DrawHint::colorBack() const
{
  unsigned int bc = ((SheiGeneratorHint_Single*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}
