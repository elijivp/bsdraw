/// DrawRecorder is a draw for 2D painting by 1D lines
/// Input: 1D array who moves in top of draw
/// Created By: Elijah Vlasov
#include "bsdrawrecorder.h"
#include "core/sheigen/bsshei2d.h"

DrawRecorder::DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, 
                           unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int resizeLimit): 
  DrawQWidget(DATEX_15D, new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient, splitPortions),
  m_stopped(0), m_memory(portions, samplesHorz, linesMemory), m_resizelim(resizeLimit)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = linesStart;
  m_portionSize = m_matrixDimmA;
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
    for (unsigned int i = 0; i < m_matrixDimmB; ++i)
    {
      for (unsigned int j = 0; j < m_matrixDimmA; ++j)
      {
        m_matrixData[p*m_matrixDimmB*m_matrixDimmA + i*m_matrixDimmA + j] = 0;
      }
    }
  DrawQWidget::vmanUpData();  // clearData?
}

void DrawRecorder::resizeGL(int w, int h)
{
  DrawQWidget::resizeGL(w,h);
  fillMatrix();
//  if (m_matrixDimmB > old_dimmB  || (m_matrixDimmB < old_dimmB  && (m_countPortions > 1)))
//    fillMatrix();
}

void DrawRecorder::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
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
      for (unsigned int i = 0; i < m_matrixDimmB; ++i)
      {
        if (m_memory.onFillData(p, i + m_stopped, &m_matrixData[p*m_matrixDimmB*m_matrixDimmA +  i*m_matrixDimmA]) == false)
  //      if (m_memory.onFillData(p + m_portionSize, i + m_stopped, &m_matrixData[p*m_matrixDimmB*m_matrixDimmA +  i*m_matrixDimmA]) == false)
        {
          for (unsigned int j = 0; j < m_matrixDimmA; ++j)
          {
            m_matrixData[p*m_matrixDimmB*m_matrixDimmA + i*m_matrixDimmA + j] = 0;
          }
        }
      }
  }
  else if (m_filldirection == FILL_INSIDE)
  {
    for (unsigned int p = 0; p < m_countPortions; ++p)
      for (unsigned int i = 0; i < m_matrixDimmB; ++i)
      {
        if (m_memory.onFillDataBackward(p, i + m_stopped, &m_matrixData[p*m_matrixDimmB*m_matrixDimmA +  i*m_matrixDimmA]) == false)
  //      if (m_memory.onFillData(p + m_portionSize, i + m_stopped, &m_matrixData[p*m_matrixDimmB*m_matrixDimmA +  i*m_matrixDimmA]) == false)
        {
          for (unsigned int j = 0; j < m_matrixDimmA; ++j)
          {
            m_matrixData[p*m_matrixDimmB*m_matrixDimmA + i*m_matrixDimmA + j] = 0;
          }
        }
      }
  }
}

int DrawRecorder::scrollValue() const
{
  return m_stopped;
}

void DrawRecorder::scrollDataTo(int pp)
{
  if (m_matrixLmSize < m_matrixDimmB*m_scalingB)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixDimmB*m_scalingB);
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
  for (int i=0; i<m_pagescount; i++)
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

void DrawRecorderPaged::slt_switchPage(int idx)
{
  switchPage(idx, true);
}

void DrawRecorderPaged::slt_switchPageNoUpdate(int idx)
{
  switchPageNoUpdate(idx);
}
