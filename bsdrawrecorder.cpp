/// DrawRecorder is a draw for 2D painting by 1D lines
/// Input: 1D array who moves in top of draw
/// Created By: Elijah Vlasov
#include "bsdrawrecorder.h"
#include "core/sheigen/bsshei2d.h"

DrawRecorder::DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, 
                           unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int resizeLimit): 
  DrawQWidget(DATEX_15D, new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient, splitPortions),
  m_filldirection(FILL_DEFAULT), m_stopped(0), m_memory(portions, samplesHorz, linesMemory), m_resizelim(resizeLimit)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = linesStart;
  m_portionSize = m_dataDimmA;
  deployMemory(portions*m_portionSize*m_resizelim);
  m_matrixLmSize = linesMemory;
}

void DrawRecorder::setFillDirection(int fd)
{
  m_filldirection = fd;
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawRecorder::setData(const float* data)
{  
  m_memory.onSetData(data);
  
  if (m_sbStatic && m_stopped != 0) m_stopped++;
  fillMatrix(); 
  
  DrawQWidget::vmanUpData();
}

void DrawRecorder::setData(const float* data, DataDecimator* decim)
{
  m_memory.onSetData(data, decim);
  
  if (m_sbStatic && m_stopped != 0) m_stopped++;
  fillMatrix(); 
  
  DrawQWidget::vmanUpData();
}

void DrawRecorder::clearData()
{  
  m_memory.onClearData();
  for (unsigned int p = 0; p < m_countPortions; ++p)
    for (unsigned int i = 0; i < m_dataDimmB; ++i)
    {
      for (unsigned int j = 0; j < m_dataDimmA; ++j)
      {
        m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = 0;
      }
    }
  DrawQWidget::vmanUpData();  // clearData?
}

void DrawRecorder::resizeGL(int w, int h)
{
  DrawQWidget::resizeGL(w,h);
//  if (m_matrixLmSize < m_dataDimmB*m_scalingB)
//    m_stopped = 0;
//  else
//    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_dataDimmB*m_scalingB);
  fillMatrix();
}

void DrawRecorder::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = m_scalingB;
  clampScaling(scalingA, scalingB);
  *matrixDimmB = sizeB / *scalingB;
  if (*matrixDimmB > m_resizelim)
    *matrixDimmB = m_resizelim;
}

void DrawRecorder::fillMatrix()
{
  if (m_filldirection == FILL_OUTSIDE)
  {
    for (unsigned int p = 0; p < m_countPortions; ++p)
      for (unsigned int i = 0; i < m_dataDimmB; ++i)
      {
        if (m_memory.onFillData(p, i + m_stopped, &m_dataStorage[p*m_dataDimmB*m_dataDimmA +  i*m_dataDimmA]) == false)
  //      if (m_memory.onFillData(p + m_portionSize, i + m_stopped, &m_dataStorage[p*m_dataDimmB*m_dataDimmA +  i*m_dataDimmA]) == false)
        {
          for (unsigned int j = 0; j < m_dataDimmA; ++j)
          {
            m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = 0;
          }
        }
      }
  }
  else if (m_filldirection == FILL_INSIDE)
  {
    for (unsigned int p = 0; p < m_countPortions; ++p)
      for (unsigned int i = 0; i < m_dataDimmB; ++i)
      {
        if (m_memory.onFillDataBackward(p, i + m_stopped, &m_dataStorage[p*m_dataDimmB*m_dataDimmA +  i*m_dataDimmA]) == false)
  //      if (m_memory.onFillData(p + m_portionSize, i + m_stopped, &m_dataStorage[p*m_dataDimmB*m_dataDimmA +  i*m_dataDimmA]) == false)
        {
          for (unsigned int j = 0; j < m_dataDimmA; ++j)
          {
            m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = 0;
          }
        }
      }
  }
}

int DrawRecorder::scrollValue() const
{
  return m_stopped;
}

unsigned int DrawRecorder::filled() const
{
  return m_memory.filled();
}

unsigned int DrawRecorder::collectVertData(unsigned int portion, int pos, unsigned int sampleHorz, float* result, unsigned int countVerts, bool reverse) const
{
  return m_memory.onCollectData(portion, pos, sampleHorz, result, countVerts, reverse);
}

bool DrawRecorder::getHistoryData(int offset, float* result) const
{
  for (unsigned int p = 0; p < m_countPortions; ++p)
  {
    if (m_memory.onFillData(p, offset, &result[p*m_portionSize]) == false)
      return false;
  }
  return true;
}

bool DrawRecorder::getHistoryData(int offset, unsigned int portion, float* result) const
{
  return m_memory.onFillData(portion, offset, result);
}

//const float* DrawRecorder::getHistoryDataPtr(int offset, int portion) const {   return &m_dataStorage[portion*m_dataDimmB*m_dataDimmA + offset*m_dataDimmA];    }

//float* DrawRecorder::getHistoryDataPtr(int offset, int portion) {    return &m_dataStorage[portion*m_dataDimmB*m_dataDimmA + offset*m_dataDimmA];   }


void DrawRecorder::scrollDataTo(int pp)
{
  if (m_matrixLmSize < m_dataDimmB*m_scalingB)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_dataDimmB*m_scalingB);
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawRecorder::scrollDataToAbs(int v)
{
  if (v > (int)m_matrixLmSize - int(m_dataDimmB*m_scalingB))
    v = (int)m_matrixLmSize - int(m_dataDimmB*m_scalingB);
  m_stopped = v;
  fillMatrix();
  DrawQWidget::vmanUpData();
}



/**********************************************************************************************************************/

DrawRecorderPaged::DrawRecorderPaged(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, unsigned int portions, 
                                 unsigned int pages, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int resizeLimit):
  DrawRecorder(samplesHorz, linesStart, linesMemory, portions, orient, splitPortions, resizeLimit),
  m_pagescount(pages == 0? 1 : pages)
{
  m_pages = new MemExpand2D::mem_t[m_pagescount];
  for (int i=m_pagescount-1; i>0; i--)
    m_pages[i] = m_memory.extendeddataarr_replicate();    /// after cycle 0idx is replicated
  m_pagehole = 0;
}

DrawRecorderPaged::~DrawRecorderPaged()
{
  for (unsigned int i=0; i<m_pagescount; i++)
    if (i != m_pagehole)
      m_memory.extendeddataarr_release(m_pages[i]);
  delete []m_pages;
}

void DrawRecorderPaged::implicitSetData(const float* data)
{
  m_memory.onSetData(data);
  if (m_sbStatic && m_stopped != 0) m_stopped++;
}

void DrawRecorderPaged::switchPage(int idx, bool currentPageImplicitlyUpdated)
{
  if (idx != m_pagehole)
  {
    m_pages[m_pagehole] = m_memory.extendeddataarr_replace(m_pages[idx]);
    m_pagehole = idx;
    currentPageImplicitlyUpdated |= true;
  }
  if (currentPageImplicitlyUpdated)
  {
    fillMatrix();
    DrawQWidget::vmanUpData();
  }
}

void DrawRecorderPaged::switchPageNoUpdate(int idx)
{
  if (idx != m_pagehole)
  {
    m_pages[m_pagehole] = m_memory.extendeddataarr_replace(m_pages[idx]);
    m_pagehole = idx;
  }
}

void DrawRecorderPaged::clearAllPages()
{
  for (int i=0; i<m_pagescount; i++)
    if (i != m_pagehole)
      m_pages[i].filled = 0;
  DrawRecorder::clearData(); // for self
}

void DrawRecorderPaged::slt_switchPage(int idx)
{
  switchPage(idx, true);
}

void DrawRecorderPaged::slt_switchPageNoUpdate(int idx)
{
  switchPageNoUpdate(idx);
}
