#include "bsdrawintensity.h"

#include "core/sheigen/bsshei2d.h"

DrawIntensity::DrawIntensity(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient, splitPortions)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = samplesVert;
  m_portionSize = samplesHorz*samplesVert;
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
