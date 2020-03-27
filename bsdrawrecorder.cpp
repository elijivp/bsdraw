#include "bsdrawrecorder.h"
#include "core/sheigen/bsshei2d.h"

DrawRecorder::DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, unsigned int portions, ORIENTATION orient, unsigned int resizeLimit): DrawQWidget(new SheiGeneratorBright(SheiGeneratorBright::DS_NONE), portions, orient),
  m_stopped(0), m_memory(portions, samplesHorz, linesMemory), m_resizelim(resizeLimit)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = linesStart;
  m_portionSize = m_matrixDimmA;
  deployMemory(portions*m_portionSize*m_resizelim);
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

//void DrawRecorder::resizeEvent(QResizeEvent* event)
//{
//  getContentsMargins(&m_cttrLeft, &m_cttrTop, &m_cttrRight, &m_cttrBottom);
  
//  int w = event->size().width() - (m_cttrLeft + m_cttrRight);
//  int h = event->size().height() - (m_cttrTop + m_cttrBottom);
  
//  int& sizeA = m_matrixSwitchAB? h : w;
//  int& sizeB = m_matrixSwitchAB? w : h;
  
//  /*int differentAB = */clampScaling(, m_scalingB);
//  unsigned int old_dimmB = m_matrixDimmB;
//  m_matrixDimmB = sizeB / m_scalingB;
//  if (m_matrixDimmB > m_resizelim) m_matrixDimmB = m_resizelim;
//  if (m_matrixDimmB > old_dimmB  || (m_matrixDimmB < old_dimmB  && (m_countPortions > 1)))
//    fillMatrix();
////  pendResize(differentAB != 0);
//  pendResize(false);
  
//  DrawQWidget::resizeEvent(event);
//}

void DrawRecorder::fillMatrix()
{
  for (unsigned int p = 0; p < m_countPortions; ++p)
    for (unsigned int i = 0; i < m_matrixDimmB; ++i)
    {
      if (m_memory.onFillData(p, i + m_stopped, &m_matrixData[p*m_matrixDimmB*m_matrixDimmA +  i*m_matrixDimmA]) == false)
      {
        for (unsigned int j = 0; j < m_matrixDimmA; ++j)
        {
          m_matrixData[p*m_matrixDimmB*m_matrixDimmA + i*m_matrixDimmA + j] = 0;
        }
      }
    }
}

void DrawRecorder::slideLmHeight(int pp)
{
  if (m_matrixLmSize < m_matrixDimmB*m_scalingB)
    m_stopped = 0;
  else
    m_stopped = ((float)pp/m_matrixLmSize)*(m_matrixLmSize - m_matrixDimmB*m_scalingB);
  fillMatrix();
  DrawQWidget::vmanUpData();
}
