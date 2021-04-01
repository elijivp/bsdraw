/// King of draws!
/// DrawDomain its a draw, who allow you to mark a canvas by markers
/// which will be paitned on colors, corresponding with data
/// Created By: Elijah Vlasov
#include "bsdrawdomain.h"

#include <memory.h>
#include "core/sheigen/bsshei2d.h"

void DIDomain::_init(unsigned int width, unsigned int height, bool incbackground, unsigned int* count, float* dataptr)
{
  m_width = width;
  m_height = height;
  m_incBackground = incbackground;
  m_count = count;
  m_dataptr = dataptr;
}

void DIDomain::start()
{
}

void DIDomain::finish()
{
  (*m_count)++;
}

void DIDomain::includeRow(int row)
{
  Q_ASSERT(row >= 0 && row < (int)m_height);
  for (unsigned int i=0; i<m_width; i++)
    m_dataptr[m_width*row + i] = *m_count;
}

void DIDomain::includeColumn(int column)
{
  Q_ASSERT(column >= 0 && column < (int)m_width);
  for (unsigned int i=0; i<m_height; i++)
    m_dataptr[m_width*i + column] = *m_count;
}

void DIDomain::includeRect(int left, int top, int width, int height)
{
//  for (int i=left; i<left+width; i++)
//    for (int j=top; j<top+height; j++)
//      m_dataptr[m_height*j + i] = *m_count;
  for (int i=top; i<top+height; i++)
    for (int j=left; j<left+width; j++)
      m_dataptr[m_width*i + j] = *m_count;
}

void DIDomain::includePixel(int r, int c)
{
  Q_ASSERT(r >= 0 && c >= 0 && r < (int)m_height && c < (int)m_width);
  m_dataptr[m_width*r + c] = *m_count;
}



void DIDomain::includeRowFree(int row)
{
  Q_ASSERT(row >= 0 && row < (int)m_height);
  for (unsigned int i=0; i<m_width; i++)
    if (m_dataptr[m_width*row + i] == 0.0f)
      m_dataptr[m_width*row + i] = *m_count;
}

void DIDomain::includeColumnFree(int column)
{
  Q_ASSERT(column >= 0 && column < (int)m_width);
  for (unsigned int i=0; i<m_height; i++)
    if (m_dataptr[m_width*i + column] == 0.0f)
      m_dataptr[m_width*i + column] = *m_count;
}

void DIDomain::includeRectFree(int left, int top, int width, int height)
{
//  for (int i=left; i<left+width; i++)
//    for (int j=top; j<top+height; j++)
//      m_dataptr[m_height*j + i] = *m_count;
  for (int i=top; i<top+height; i++)
    for (int j=left; j<left+width; j++)
      if (m_dataptr[m_width*i + j] == 0.0f)
        m_dataptr[m_width*i + j] = *m_count;
}

void DIDomain::includePixelFree(int r, int c)
{
  Q_ASSERT(r >= 0 && c >= 0 && r < (int)m_height && c < (int)m_width);
  if (m_dataptr[m_width*r + c] == 0.0f)
    m_dataptr[m_width*r + c] = *m_count;
}

bool DIDomain::isFree(int r, int c) const
{
  return m_dataptr[m_width*r + c] == 0.0f;
}



void DIDomain::excludeRow(int row)
{
  Q_ASSERT(row >= 0 && row < (int)m_height);
  for (unsigned int i=0; i<m_width; i++)
    m_dataptr[m_width*row + i] = 0.0f;
}

void DIDomain::excludeColumn(int column)
{
  Q_ASSERT(column >= 0 && column < (int)m_width);
  for (unsigned int i=0; i<m_height; i++)
    m_dataptr[m_width*i + column] = 0.0f;
}

void DIDomain::excludeRect(int left, int top, int width, int height)
{
  for (int i=top; i<top+height; i++)
    for (int j=left; j<left+width; j++)
      m_dataptr[m_width*i + j] = 0.0f;
}

void DIDomain::excludePixel(int r, int c)
{
  Q_ASSERT(r >= 0 && c >= 0 && r < (int)m_height && c < (int)m_width);
  m_dataptr[m_width*r + c] = 0.0f;
}


/////////////////////////////////////////////////////////////////////////

DrawDomain::DrawDomain(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, bool isBckgrndDomain, ORIENTATION orient, bool holdmemorytilltheend): 
  DrawQWidget(DATEX_DD, new SheiGeneratorBright(isBckgrndDomain? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = samplesVert;
  unsigned int total = m_matrixDimmA*m_matrixDimmB;
  deployMemory(total*portions);
  m_portionSize = 1;
  
  m_groundType = GND_DOMAIN;
  m_groundData = new float[total];
  memset(m_groundData, 0, total*sizeof(float));
  m_groundDataFastFree = !holdmemorytilltheend;
  
  m_domain._init(m_matrixDimmA, m_matrixDimmB, isBckgrndDomain, &m_portionSize, (float*)m_groundData);
}

DrawDomain::DrawDomain(const DIDomain &cpy, unsigned int portions, ORIENTATION orient, bool holdmemorytilltheend): 
  DrawQWidget(DATEX_DD, new SheiGeneratorBright(cpy.isBackgroundDomain()? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_matrixDimmA = cpy.m_width;
  m_matrixDimmB = cpy.m_height;
  unsigned int total = m_matrixDimmA*m_matrixDimmB;
  deployMemory(total*portions);
  
  m_groundType = GND_DOMAIN;
  m_groundData = new float[total];
  memcpy(m_groundData, cpy.m_dataptr, total*sizeof(float));
  m_portionSize = *cpy.m_count;
  m_groundDataFastFree = !holdmemorytilltheend;
 
  m_domain._init(m_matrixDimmA, m_matrixDimmB, cpy.isBackgroundDomain(), &m_portionSize, (float*)m_groundData);
}

DrawDomain::~DrawDomain()
{
  if (m_groundData != nullptr)
    delete [](float*)m_groundData;
}

DIDomain *DrawDomain::domain()
{
  return m_groundData == NULL? 0 : &m_domain;  
}

const DIDomain *DrawDomain::domain() const
{
  return m_groundData == NULL? 0 : &m_domain;  
}

unsigned int DrawDomain::domainsCount() const
{
  return m_portionSize;
}

void DrawDomain::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);
  clampScaling(scalingA, scalingB);
}




