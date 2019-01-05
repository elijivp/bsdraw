#include "bsdrawrecorder.h"
#include "core/sheigen/bsshei2d.h"


DrawRecorder::DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, unsigned int portions, ORIENTATION orient, unsigned int resizeLimit): DrawQWidget(new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient),
  m_stopped(0), m_memory(portions, samplesHorz, linesMemory), m_resizelim(resizeLimit)
{
  m_matrixWidth = samplesHorz;
  m_matrixHeight = linesStart;
  m_portionSize = m_matrixWidth;
  deployMemory(m_countPortions*m_portionSize*m_resizelim);
  m_matrixLmSize = linesMemory;
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
    for (unsigned int i = 0; i < m_matrixHeight; ++i)
    {
      for (unsigned int j = 0; j < m_matrixWidth; ++j)
      {
        m_matrixData[p*m_matrixHeight*m_matrixWidth + i*m_matrixWidth + j] = 0;
      }
    }
  DrawQWidget::vmanUpData();  // clearData?
}

void DrawRecorder::resizeGL(int w, int h)
{
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  m_matrixScWidth = (unsigned int)w <= m_matrixWidth? 1 : (w / m_matrixWidth);
  clampScaling();
  unsigned int old_matrixHeight = m_matrixHeight;
  m_matrixHeight = h / m_matrixScHeight;
  if (m_matrixHeight > m_resizelim) m_matrixHeight = m_resizelim;
  if (m_matrixHeight > old_matrixHeight || (m_matrixHeight < old_matrixHeight && (m_countPortions > 1)))
    fillMatrix();
  pendResize(true);
}

void DrawRecorder::fillMatrix()
{
  for (unsigned int p = 0; p < m_countPortions; ++p)
    for (unsigned int i = 0; i < m_matrixHeight; ++i)
    {
      if (m_memory.onFillData(p, i + m_stopped, &m_matrixData[p*m_matrixHeight*m_matrixWidth +  i*m_matrixWidth]) == false)
      {
        for (unsigned int j = 0; j < m_matrixWidth; ++j)
        {
          m_matrixData[p*m_matrixHeight*m_matrixWidth + i*m_matrixWidth + j] = 0;
        }
      }
    }
}

void DrawRecorder::slideLmHeight(int pp)
{
  if (m_matrixLmSize < m_matrixHeight*m_matrixScHeight)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixHeight*m_matrixScHeight);
  fillMatrix();
  DrawQWidget::vmanUpData();
}
