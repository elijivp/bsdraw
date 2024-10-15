/// DrawRecorder is a draw for 2D painting by 1D lines
/// Input: 1D array who moves in top of draw
/// Created By: Elijah Vlasov
#include "bsdrawrecorder.h"

#include "core/sheigen/bsshgenmain.h"

class SheiGeneratorRecorder: public ISheiGenerator
{
  SUMMODEPORTIONS       m_smp;
public:
  SheiGeneratorRecorder(SUMMODEPORTIONS smp): m_smp(smp){}
  virtual const char*   shaderName() const {  return "2DREC"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize() const { return 2048; }
  virtual unsigned int  shfragment_uniforms(shuniformdesc_t*, unsigned int){ return 0; }
  virtual void          shfragment_store(FshDrawComposer& fdc) const
  {
    fdc.push( fdc.splits() == SP_NONE? "for (int i=0; i<dataportions; i++)" : "int i = explicitPortion;" );
    fdc.push("{");
    {
      if ( fdc.splits() == SP_NONE )
      {
        fdc.value2D("float value");
        fdc.push("dvalue = max(dvalue, value);");
      }
      else
        fdc.value2D("dvalue");
      
      /// result=vec3(0) by DrawCore constructor and m_emptycolor=0
      if ( fdc.splits() == SP_NONE )
      {
        if (m_smp == SMP_SUM)
          fdc.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1))).rgb;" );
        else if (m_smp == SMP_MEANSUM)
          fdc.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1))).rgb/vec3(allocatedPortions);" );
      }
      else if (fdc.splits() & SPFLAG_COLORSPLIT)
      {
        if (m_smp == SMP_SUM)
          fdc.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" );
        else if (m_smp == SMP_MEANSUM)
          fdc.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb/vec3(allocatedPortions);" );
      }
      else
        fdc.push("result.rgb = mix(texture(paletsampler, vec2(value, 0.0)).rgb, result.rgb, step(dataportions, float(explicitPortion)));" );
      
      fdc.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" );
    }
    fdc.push("}");
  }
};

DrawRecorder::DrawRecorder( unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, 
                            unsigned int portions, ORIENTATION orient, 
                            SPLITPORTIONS splitPortions, SUMMODEPORTIONS summodePortions, unsigned int resizeLimit): 
  DrawQWidget(DATEX_15D, new SheiGeneratorRecorder(summodePortions), portions, orient, splitPortions, 0x00000000),
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
        m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = m_dataClearValue;
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
            m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = m_dataClearValue;
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
            m_dataStorage[p*m_dataDimmB*m_dataDimmA + i*m_dataDimmA + j] = m_dataClearValue;
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

DrawRecorderPaged::DrawRecorderPaged( unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, unsigned int portions, 
                                      unsigned int pages, ORIENTATION orient, 
                                      SPLITPORTIONS splitPortions, SUMMODEPORTIONS summodePortions, unsigned int resizeLimit):
  DrawRecorder(samplesHorz, linesStart, linesMemory, portions, orient, splitPortions, summodePortions, resizeLimit),
  m_pagescount(pages == 0? 1 : pages)
{
  m_pages = new MemExpand2D::mem_t[m_pagescount];
//  for (int i=m_pagescount-1; i>0;i--)
  for (unsigned int i=1; i<m_pagescount; i++)
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
void DrawRecorderPaged::implicitClearData()
{
  m_memory.onClearData();
//  if (m_sbStatic && m_stopped != 0) m_stopped++;
}

void DrawRecorderPaged::switchPage(int idx, bool currentPageImplicitlyModified)
{
  if (idx != m_pagehole)
  {
    m_pages[m_pagehole] = m_memory.extendeddataarr_replace(m_pages[idx]);
    m_pagehole = idx;
    currentPageImplicitlyModified |= true;
  }
  if (currentPageImplicitlyModified)
    updatePageModified();
}

void DrawRecorderPaged::switchPageNoUpdate(int idx)
{
  if (idx != m_pagehole)
  {
    m_pages[m_pagehole] = m_memory.extendeddataarr_replace(m_pages[idx]);
    m_pagehole = idx;
  }
}

void DrawRecorderPaged::updatePageModified()
{
  fillMatrix();
  DrawQWidget::vmanUpData();
}

void DrawRecorderPaged::swapPages(int p1, int p2)
{
  if (p1 == p2)
    return;
  
  MemExpand2D::mem_t tmp = m_pages[p1];
  m_pages[p1] = m_pages[p2];
  m_pages[p2] = tmp;
  
  if (m_pagehole == p1 || m_pagehole == p2)
    updatePageModified();
}

void DrawRecorderPaged::swapPagesNoUpdate(int p1, int p2)
{
  if (p1 == p2)
    return;
  MemExpand2D::mem_t tmp = m_pages[p1];
  m_pages[p1] = m_pages[p2];
  m_pages[p2] = tmp;
}

//void DrawRecorderPaged::clearPage(int p)
//{
//  if (p != m_pagehole)
//    m_pages[p].filled = 0;
//  else
//    DrawRecorder::clearData(); // for self
//}

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
