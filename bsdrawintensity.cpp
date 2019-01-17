#include "bsdrawintensity.h"
#include "core/sheigen/bsshei2d.h"

DrawIntensity::DrawIntensity(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, ORIENTATION orient): DrawQWidget(new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient)
{
  m_matrixWidth = samplesHorz;
  m_matrixHeight = samplesVert;
  m_portionSize = samplesHorz*samplesVert;
  deployMemory();
}

void DrawIntensity::resizeGL(int w, int h)
{
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  m_matrixScWidth = (unsigned int)w <= m_matrixWidth? 1 : (w / m_matrixWidth);
  m_matrixScHeight = (unsigned int)h <= m_matrixHeight? 1 : (h / m_matrixHeight);
  clampScaling();
  pendResize(true);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
///

DrawIntensePoints::DrawIntensePoints(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, ORIENTATION orient, int dcip):
  DrawIntensity(samplesHorz, samplesVert, portions, orient), m_dcip(dcip), m_clearBuf(nullptr)
{
}

/////////
/// no clearData() cause of possible changed LL and!! innerupdate by data
#define DCIP_DATA_CLEAR {   unsigned int total = m_countPortions * m_matrixWidth * m_matrixHeight;    for (unsigned int _i=0; _i<total; _i++)  m_matrixData[_i] = 0;  }
#define DCIP_MULS(cr)       static int crcast[] = { 0, 1, 2, 3, 0, 1, 2, 2, 3, 3 }; \
                            static unsigned int crmuls[][2] = { {1,1}, {m_matrixWidth-1, m_matrixHeight-1}, {1, m_matrixHeight-1}, {m_matrixWidth-1, 1} }; \
                            unsigned int (&muls)[2] = crmuls[crcast[cr]];

#define DCIP_2D_FOR_BEGIN   unsigned int n=0; \
                            for (unsigned int p = 0; p < m_countPortions; p++) \
                              for (unsigned int d = 0; d<dataCountByPortions[p]; ++d, ++n) \
                              {
//#define DCIP_2D_FOR_STORE(value)  m_matrixData[p*m_matrixHeight*m_matrixWidth + j*m_matrixWidth + i] = value;

#define DCIP_2D_FOR_END       }

void DrawIntensePoints::setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, float value)
{
  DCIP_DATA_CLEAR
  DCIP_MULS(cr)
  DCIP_2D_FOR_BEGIN
    int i = coordsXY[n*2] * muls[0], j = coordsXY[n*2 + 1] * muls[1];
    m_matrixData[p*m_matrixHeight*m_matrixWidth + j*m_matrixWidth + i] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(COORDINATION cr, const unsigned int* dataCountByPortions, const float* coordsXY, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_MULS(cr)
  DCIP_2D_FOR_BEGIN
    int i = coordsXY[n*2] * muls[0], j = coordsXY[n*2 + 1] * muls[1];
    m_matrixData[p*m_matrixHeight*m_matrixWidth + j*m_matrixWidth + i] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

/////////////////////////

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, float value)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_matrixData[p*m_matrixHeight*m_matrixWidth + int(relYs[n]*m_matrixHeight)*m_matrixWidth + int(relXs[n]*m_matrixWidth)] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const float* relXs, const float* relYs, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_matrixData[p*m_matrixHeight*m_matrixWidth + int(relYs[n]*(m_matrixHeight-1))*m_matrixWidth + int(relXs[n]*(m_matrixWidth-1))] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}


/////////////////////////

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const unsigned int* absXs, const unsigned int* absYs, float value)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_matrixData[p*m_matrixHeight*m_matrixWidth + absYs[n]*m_matrixWidth + absXs[n]] = value;
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}

void DrawIntensePoints::setData(const unsigned int* dataCountByPortions, const unsigned int* Xs, const unsigned int* absYs, const float* datavalues)
{
  DCIP_DATA_CLEAR
  DCIP_2D_FOR_BEGIN
    m_matrixData[p*m_matrixHeight*m_matrixWidth + absYs[n]*m_matrixWidth + Xs[n]] = datavalues[n];
  DCIP_2D_FOR_END
  DrawCore::vmanUpData();
}