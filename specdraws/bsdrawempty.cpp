/// This file contains special derived class for trivial cases
/// DrawEmpty does nothing, just holds actual for 2D draws space (throught sizeAndScaleHint)
/// Created By: Elijah Vlasov
#include "bsdrawempty.h"

#include "../core/sheigen/bsshgenmain.h"

class SheiGeneratorEmpty: public ISheiGenerator
{
  unsigned int emptyclr;
public:
  SheiGeneratorEmpty(unsigned int color): emptyclr(color){}
  ~SheiGeneratorEmpty();
public:
  virtual const char*   shaderName() const {  return "EMPTY"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         ovlfraginfo_t ovlsinfo[], unsigned int ovlscount,
                                         locbackinfo_t locbackinfo[], unsigned int* locbackcount,
                                         char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, 0, nullptr, ovlscount, ovlsinfo);
    fmg.getLocbacks(locbackinfo, locbackcount);
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, emptyclr, orient, fsp);
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};
SheiGeneratorEmpty::~SheiGeneratorEmpty(){}

DrawEmpty::DrawEmpty(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backcolor, bool allowNoscaledResize): 
  DrawQWidget(DATEX_2D, new SheiGeneratorEmpty(backcolor), 1, OR_LRTB), m_allowNoscaledResize(allowNoscaledResize)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = samplesVert;
  m_portionSize = 0;
  deployMemory();
}

void DrawEmpty::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  if (m_allowNoscaledResize)
  {
    *matrixDimmA = sizeA / m_scalingA;
    *matrixDimmB = sizeB / m_scalingB;
    *scalingA = m_scalingA;
    *scalingB = m_scalingB;
  }
  else
  {
    *matrixDimmA = m_dataDimmA;
    *matrixDimmB = m_dataDimmB;
    *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
    *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  }
  clampScaling(scalingA, scalingB);
}
