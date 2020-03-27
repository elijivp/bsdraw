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
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int /*allocatedPortions*/, const DPostmask& fsp, ORIENTATION orient, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, orient, ovlscount, ovlsinfo);
    fmg.goto_func_begin(FshMainGenerator::INIT_BYVALUE, emptyclr, fsp);
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorEmpty::~SheiGeneratorEmpty(){}

DrawEmpty::DrawEmpty(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backcolor): DrawQWidget(new SheiGeneratorEmpty(backcolor), 1, OR_LRTB)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = samplesVert;
  m_portionSize = samplesHorz*samplesVert;
  deployMemory();
}

void DrawEmpty::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);
  clampScaling(scalingA, scalingB);
}
