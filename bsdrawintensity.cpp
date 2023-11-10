#include "bsdrawintensity.h"

#include "core/sheigen/bsshgenmain.h"
#ifndef SHNL
#define SHNL "\n"
#endif

class SheiGeneratorIntenisty: public ISheiGenerator
{
public:
  virtual const char*   shaderName() const {  return "2D"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 700 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         ovlfraginfo_t ovlsinfo[], unsigned int ovlscount, 
                                         locbackinfo_t locbackinfo[], unsigned int* locbackcount,
                                         char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, 0, nullptr, ovlscount, ovlsinfo);
    fmg.getLocbacks(locbackinfo, locbackcount);
    fmg.main_begin(FshDrawConstructor::INITBACK_BYPALETTE, 0, orient, fsp); //FshDrawConstructor::INITBACK_BYZERO
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);
    fmg.push( splitPortions == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
    fmg.push("{");
    {
//      fmg.value2D("float value");
//      fmg.push("dvalue = max(dvalue, value);");
      fmg.value2D("dvalue");
      fmg.push("float value = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
      if ( splitPortions == SP_NONE )
        fmg.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" SHNL);
      else if (splitPortions & SPFLAG_COLORSPLIT)
        fmg.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" SHNL);
      else
        fmg.push("result.rgb = mix(texture(paletsampler, vec2(value, 0.0)).rgb, result.rgb, step(dataportions, float(explicitPortion)));" SHNL);
      
      fmg.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" SHNL);
    }
    fmg.push("}");
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};

DrawIntensity::DrawIntensity(unsigned int samplesA, unsigned int samplesB, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorIntenisty, portions, orient, splitPortions)
{
  m_dataDimmA = samplesA;
  m_dataDimmB = samplesB;
  m_portionSize = samplesA*samplesB;
  deployMemory();
}

void DrawIntensity::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
///

DrawIntensityUpsizeA::DrawIntensityUpsizeA(unsigned int samplesAmin, unsigned int samplesAmax, unsigned int samplesB, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorIntenisty, portions, orient, splitPortions), m_minA(samplesAmin), m_maxA(samplesAmax)
{
  m_dataDimmA = samplesAmax;
  m_dataDimmB = samplesB;
  m_portionSize = m_dataDimmA*m_dataDimmB;
  deployMemory();
  m_dataDimmA = samplesAmin;
  m_portionSize = m_dataDimmA*m_dataDimmB;
}

void DrawIntensityUpsizeA::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = sizeA / m_scalingA;
  *matrixDimmB = m_dataDimmB;
  if (*matrixDimmA > m_maxA)      *matrixDimmA = m_maxA;
  else if (*matrixDimmA < m_minA) *matrixDimmA = m_minA;
  *scalingA = m_scalingA;
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}

int DrawIntensityUpsizeA::sizeAndScaleChanged(bool changedDimmA, bool /*changedDimmB*/, bool changedScalingA, bool /*changedScalingB*/)
{
  if (changedDimmA || changedScalingA)
  {
    m_portionSize = sizeDataA()*sizeDataB();
    return PC_DATA | PC_DATADIMMS;
  }
  return 0;
}



DrawIntensityUpsizeB::DrawIntensityUpsizeB(unsigned int samplesA, unsigned int samplesBmin, unsigned int samplesBmax, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorIntenisty, portions, orient, splitPortions), m_minB(samplesBmin), m_maxB(samplesBmax)
{
  m_dataDimmA = samplesA;
  m_dataDimmB = samplesBmax;
  m_portionSize = m_dataDimmA*m_dataDimmB;
  deployMemory();
  m_dataDimmB = samplesBmin;
  m_portionSize = m_dataDimmA*m_dataDimmB;
}

void DrawIntensityUpsizeB::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = sizeB / m_scalingB;
  if (*matrixDimmB > m_maxB)      *matrixDimmB = m_maxB;
  else if (*matrixDimmB < m_minB) *matrixDimmB = m_minB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = m_scalingB;
  clampScaling(scalingA, scalingB);
}

int DrawIntensityUpsizeB::sizeAndScaleChanged(bool /*changedDimmA*/, bool changedDimmB, bool /*changedScalingA*/, bool changedScalingB)
{
  if (changedDimmB || changedScalingB)
  {
    m_portionSize = sizeDataA()*sizeDataB();
    return PC_DATA | PC_DATADIMMS;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
///

DrawIntensePoints::DrawIntensePoints(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions, int dcip):
  DrawIntensity(samplesHorz, samplesVert, portions, orient, splitPortions), m_dcip(dcip), m_clearBuf(nullptr)
{
}

/////////
/// no clearData() cause of possible changed LL and!! innerupdate by data
#define DCIP_DATA_CLEAR {   unsigned int total = m_countPortions * m_dataDimmA * m_dataDimmB;    for (unsigned int _i=0; _i<total; _i++)  m_dataStorage[_i] = 0;  }
#define DCIP_MULS(cr)       static int crcast[] = { 0, 1, 2, 3, 0, 1, 2, 2, 3, 3 }; \
                            static unsigned int crmuls[][2] = { {1,1}, {m_dataDimmA-1, m_dataDimmB-1}, {1, m_dataDimmB-1}, {m_dataDimmA-1, 1} }; \
                            unsigned int (&muls)[2] = crmuls[crcast[cr]];

#define DCIP_2D_FOR_BEGIN   unsigned int n=0; \
                            for (unsigned int p = 0; p < m_countPortions; p++) \
                              for (unsigned int d = 0; d<dataCountByPortions[p]; ++d, ++n) \
                              {
//#define DCIP_2D_FOR_STORE(value)  m_dataStorage[p*m_dataDimmB*m_dataDimmA + j*m_dataDimmA + i] = value;

#define DCIP_2D_FOR_END       }

void DrawIntensePoints::setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, float value)
{
  DCIP_DATA_CLEAR
  DCIP_MULS(cr)
  DCIP_2D_FOR_BEGIN
    int i = coordsXY[n*2] * muls[0], j = coordsXY[n*2 + 1] * muls[1];
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + j*m_dataDimmA + i] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_MULS(cr)
  DCIP_2D_FOR_BEGIN
    int i = coordsXY[n*2] * muls[0], j = coordsXY[n*2 + 1] * muls[1];
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + j*m_dataDimmA + i] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

/////////////////////////

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, float value)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + int(relYs[n]*m_dataDimmB)*m_dataDimmA + int(relXs[n]*m_dataDimmA)] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + int(relYs[n]*(m_dataDimmB-1))*m_dataDimmA + int(relXs[n]*(m_dataDimmA-1))] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}


/////////////////////////

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const unsigned int* absXs, const unsigned int* absYs, float value)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + absYs[n]*m_dataDimmA + absXs[n]] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const unsigned int* Xs, const unsigned int* absYs, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_dataStorage[p*m_dataDimmB*m_dataDimmA + absYs[n]*m_dataDimmA + Xs[n]] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}
