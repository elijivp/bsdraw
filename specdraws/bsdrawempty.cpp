/// This file contains special derived class for trivial cases
/// DrawEmpty does nothing, just holds actual for 2D draws space (throught sizeAndScaleHint)
/// Created By: Elijah Vlasov
#include "bsdrawempty.h"

#include "../core/sheigen/bsshgenmain.h"

class SheiGeneratorEmpty: public ISheiGenerator
{
public:
  virtual const char*   shaderName() const {  return "EMPTY"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize() const { return 0; }
  virtual unsigned int  shfragment_uniforms(shuniformdesc_t*, unsigned int){ return 0; }
  virtual void          shfragment_store(FshDrawComposer&) const {}
};

DrawEmpty::DrawEmpty(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backcolor, bool allowNoscaledResize): 
  DrawQWidget(DATEX_2D, new SheiGeneratorEmpty(), 1, OR_LRTB, SP_NONE, backcolor), m_allowNoscaledResize(allowNoscaledResize)
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
