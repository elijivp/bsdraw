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
  DrawQWidget(new SheiGeneratorBright(isBckgrndDomain? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_matrixWidth = samplesHorz;
  m_matrixHeight = samplesVert;
  unsigned int total = m_matrixWidth*m_matrixHeight;
  deployMemory(total*m_countPortions);
  m_portionSize = 1;
  
  m_dataDomains = new float[total];
  memset(m_dataDomains, 0, total*sizeof(float));
  m_dataDomainsFastFree = !holdmemorytilltheend;
  
  m_domain._init(m_matrixWidth, m_matrixHeight, isBckgrndDomain, &m_portionSize, m_dataDomains);
}

DrawDomain::DrawDomain(const DIDomain &cpy, unsigned int portions, ORIENTATION orient, bool holdmemorytilltheend): 
  DrawQWidget(new SheiGeneratorBright(cpy.isBackgroundDomain()? SheiGeneratorBright::DS_DOMSTD : SheiGeneratorBright::DS_DOMBLACK), portions, orient)
{
  m_matrixWidth = cpy.m_width;
  m_matrixHeight = cpy.m_height;
  unsigned int total = m_matrixWidth*m_matrixHeight;
  deployMemory(total*m_countPortions);
  
  m_dataDomains = new float[total];
  memcpy(m_dataDomains, cpy.m_dataptr, total*sizeof(float));
  m_portionSize = *cpy.m_count;
  m_dataDomainsFastFree = !holdmemorytilltheend;
 
  m_domain._init(m_matrixWidth, m_matrixHeight, cpy.isBackgroundDomain(), &m_portionSize, m_dataDomains);
}

DrawDomain::~DrawDomain()
{
  if (m_dataDomains != nullptr)
    delete []m_dataDomains;
}

DIDomain *DrawDomain::domain()
{
  return m_dataDomains == NULL? 0 : &m_domain;  
}

const DIDomain *DrawDomain::domain() const
{
  return m_dataDomains == NULL? 0 : &m_domain;  
}

unsigned int DrawDomain::domainsCount() const
{
  return m_portionSize;
}

void DrawDomain::resizeGL(int w, int h)
{
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  m_matrixScWidth = (unsigned int)w <= m_matrixWidth? 1 : (w / m_matrixWidth);
  m_matrixScHeight = (unsigned int)h <= m_matrixHeight? 1 : (h / m_matrixHeight);
  clampScaling();
  pendResize(true);
}



