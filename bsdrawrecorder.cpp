/// DrawRecorder is a draw for 2D painting by 1D lines
/// Input: 1D array who moves in top of draw
/// Created By: Elijah Vlasov
#include "bsdrawrecorder.h"

#include "core/sheigen/bsshgenmain.h"

class SheiGeneratorRecorder: public ISheiGenerator
{
public:
  virtual const char*   shaderName() const {  return "2DREC"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 700 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         ovlfraginfo_t ovlsinfo[], unsigned int ovlscount, 
                                         locbackinfo_t locbackinfo[], unsigned int* locbackcount,
                                         char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, 0, nullptr, ovlscount, ovlsinfo);
    fmg.getLocbacks(locbackinfo, locbackcount);
    fmg.main_begin(FshDrawConstructor::INITBACK_BYPALETTE, 0, orient, fsp); //FshDrawConstructor::INITBACK_BYZERO
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);
    fmg.push( splitPortions == SP_NONE? "for (int i=0; i<dataportions; i++)" : "int i = explicitPortion;" );
    fmg.push("{");
    {
      fmg.value2D("float value");
      fmg.push("dvalue = max(dvalue, value);");
      fmg.push("value = paletrange[0] + (paletrange[1] - paletrange[0])*value;" );
      if ( splitPortions == SP_NONE )
        fmg.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" );
      else if (splitPortions & SPFLAG_COLORSPLIT)
        fmg.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" );
      else
        fmg.push("result.rgb = mix(texture(paletsampler, vec2(value, 0.0)).rgb, result.rgb, step(dataportions, float(explicitPortion)));" );
      
      fmg.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" );
    }
    fmg.push("}");
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};

DrawRecorder::DrawRecorder(unsigned int samplesHorz, unsigned int linesStart, unsigned int linesMemory, 
                           unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int resizeLimit): 
  DrawQWidget(DATEX_15D, new SheiGeneratorRecorder, portions, orient, splitPortions),
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

void DrawRecorderPaged::clearPage(int p)
{
  if (p != m_pagehole)
    m_pages[p].filled = 0;
  else
    DrawRecorder::clearData(); // for self
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
