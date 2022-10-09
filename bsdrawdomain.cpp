/// King of draws!
/// DrawDomain its a draw, who allow you to mark a canvas by markers
/// which will be paitned on colors, corresponding with data
/// Created By: Elijah Vlasov
#include "bsdrawdomain.h"

#include <memory.h>
#include "core/sheigen/bsshei2d.h"

#if !defined DIDOMAIN_CHECKBOUNDS_ASSERT && !defined DIDOMAIN_CHECKBOUNDS_CONDITION && !defined DIDOMAIN_CHECKBOUNDS_IGNORE
#define DIDOMAIN_CHECKBOUNDS_ASSERT
#endif


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
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(row >= 0 && row < (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (row < 0 || row >= (int)m_height)
    return;
#endif
  for (unsigned int i=0; i<m_width; i++)
    m_dataptr[m_width*row + i] = *m_count;
}

void DIDomain::includeColumn(int column)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(column >= 0 && column < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (column < 0 || column >= (int)m_width)
    return;
#endif
  for (unsigned int i=0; i<m_height; i++)
    m_dataptr[m_width*i + column] = *m_count;
}

void DIDomain::includeRect(int left, int top, int width, int height)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(width >= 0 && height >= 0);
  Q_ASSERT(left >= 0 && left + width <= (int)m_width);
  Q_ASSERT(top >= 0 && top + height <= (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (width < 0 || height < 0)  return;
  if (left < 0) left = 0;
  if (top < 0)  top = 0;
  if (left + width > (int)m_width)   width = (int)m_width - left;
  if (top + height > (int)m_height)  height = (int)m_height - top;
#endif
//  for (int i=left; i<left+width; i++)
//    for (int j=top; j<top+height; j++)
//      m_dataptr[m_height*j + i] = *m_count;
  for (int i=top; i<top+height; i++)
    for (int j=left; j<left+width; j++)
      m_dataptr[m_width*i + j] = *m_count;
}

void DIDomain::includePixel(int r, int c)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(r >= 0 && r < (int)m_height);
  Q_ASSERT(c >= 0 && c < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (r < 0 || r >= (int)m_height) return;
  if (c < 0 || c >= (int)m_width) return;
#endif
  m_dataptr[m_width*r + c] = *m_count;
}



void DIDomain::includeRowFree(int row)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(row >= 0 && row < (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (row < 0 || row >= (int)m_height)
    return;
#endif
  for (unsigned int i=0; i<m_width; i++)
    if (m_dataptr[m_width*row + i] == 0.0f)
      m_dataptr[m_width*row + i] = *m_count;
}

void DIDomain::includeColumnFree(int column)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(column >= 0 && column < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (column < 0 || column >= (int)m_width)
    return;
#endif
  for (unsigned int i=0; i<m_height; i++)
    if (m_dataptr[m_width*i + column] == 0.0f)
      m_dataptr[m_width*i + column] = *m_count;
}

void DIDomain::includeRectFree(int left, int top, int width, int height)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(width >= 0 && height >= 0);
  Q_ASSERT(left >= 0 && left + width <= (int)m_width);
  Q_ASSERT(top >= 0 && top + height <= (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (width < 0 || height < 0)  return;
  if (left < 0) left = 0;
  if (top < 0)  top = 0;
  if (left + width > (int)m_width)   width = (int)m_width - left;
  if (top + height > (int)m_height)  height = (int)m_height - top;
#endif
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
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(r >= 0 && r < (int)m_height);
  Q_ASSERT(c >= 0 && c < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (r < 0 || r >= (int)m_height) return;
  if (c < 0 || c >= (int)m_width) return;
#endif
  if (m_dataptr[m_width*r + c] == 0.0f)
    m_dataptr[m_width*r + c] = *m_count;
}

bool DIDomain::isFree(int r, int c) const
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(r >= 0 && r < (int)m_height);
  Q_ASSERT(c >= 0 && c < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (r < 0 || r >= (int)m_height) return false;
  if (c < 0 || c >= (int)m_width) return false;
#endif
  return m_dataptr[m_width*r + c] == 0.0f;
}



void DIDomain::excludeRow(int row)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(row >= 0 && row < (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (row < 0 || row >= (int)m_height)
    return;
#endif
  for (unsigned int i=0; i<m_width; i++)
    m_dataptr[m_width*row + i] = 0.0f;
}

void DIDomain::excludeColumn(int column)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(column >= 0 && column < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (column < 0 || column >= (int)m_width)
    return;
#endif
  for (unsigned int i=0; i<m_height; i++)
    m_dataptr[m_width*i + column] = 0.0f;
}

void DIDomain::excludeRect(int left, int top, int width, int height)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(width >= 0 && height >= 0);
  Q_ASSERT(left >= 0 && left + width <= (int)m_width);
  Q_ASSERT(top >= 0 && top + height <= (int)m_height);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (width < 0 || height < 0)  return;
  if (left < 0) left = 0;
  if (top < 0)  top = 0;
  if (left + width > (int)m_width)   width = (int)m_width - left;
  if (top + height > (int)m_height)  height = (int)m_height - top;
#endif
  for (int i=top; i<top+height; i++)
    for (int j=left; j<left+width; j++)
      m_dataptr[m_width*i + j] = 0.0f;
}

void DIDomain::excludePixel(int r, int c)
{
#ifdef DIDOMAIN_CHECKBOUNDS_ASSERT
  Q_ASSERT(r >= 0 && r < (int)m_height);
  Q_ASSERT(c >= 0 && c < (int)m_width);
#elif defined DIDOMAIN_CHECKBOUNDS_CONDITION
  if (r < 0 || r >= (int)m_height) return;
  if (c < 0 || c >= (int)m_width) return;
#endif
  m_dataptr[m_width*r + c] = 0.0f;
}


/////////////////////////////////////////////////////////////////////////

DrawDomain::DrawDomain(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, bool isBckgrndDomain, ORIENTATION orient, bool holdmemorytilltheend): 
  DrawQWidget(DATEX_DD, new SheiGeneratorBright(isBckgrndDomain? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = samplesVert;
  unsigned int total = m_dataDimmA*m_dataDimmB;
  deployMemory(total*portions);
  m_portionSize = 1;
  
  m_groundType = GND_DOMAIN;
  m_groundData = new float[total];
  memset(m_groundData, 0, total*sizeof(float));
  m_groundDataFastFree = !holdmemorytilltheend;
  
  m_domain._init(m_dataDimmA, m_dataDimmB, isBckgrndDomain, &m_portionSize, (float*)m_groundData);
}

DrawDomain::DrawDomain(const DIDomain &cpy, unsigned int portions, ORIENTATION orient, bool holdmemorytilltheend): 
  DrawQWidget(DATEX_DD, new SheiGeneratorBright(cpy.isBackgroundDomain()? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_dataDimmA = cpy.m_width;
  m_dataDimmB = cpy.m_height;
  unsigned int total = m_dataDimmA*m_dataDimmB;
  deployMemory(total*portions);
  
  m_groundType = GND_DOMAIN;
  m_groundData = new float[total];
  memcpy(m_groundData, cpy.m_dataptr, total*sizeof(float));
  m_portionSize = *cpy.m_count;
  m_groundDataFastFree = !holdmemorytilltheend;
 
  m_domain._init(m_dataDimmA, m_dataDimmB, cpy.isBackgroundDomain(), &m_portionSize, (float*)m_groundData);
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
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}




