/// DrawCore does not contain any Qt code. DrawQWidget is an arrangement of DrawCore on Qt
/// Based on QOpenGLWidget, DrawQWidget also rearrange DrawCore methods on Qt slots
/// Qt Events also appears here
/// 
/// If you want operate pure bsdraws interface, use DrawCore
/// But if you want operate a connective (bsdraws+QWidget), use DrawQWidget
/// 
/// Created By: Elijah Vlasov
#include "bsqdraw.h"

#include "../palettes/bsipalette.h"

#include <QMouseEvent>
#include <QResizeEvent>

const char*   DrawQWidget::vardesc(SHEIFIELD sf)
{
  if (sf == SF_DATA)            return "texData";
  if (sf == SF_PALETTE)         return "texPalette";
  if (sf == SF_DOMAIN)          return "texGround";
  if (sf == SF_PORTIONSIZE)     return "countGround";
  if (sf == SF_COUNTPORTIONS)   return "countPortions";
  if (sf == SF_DIMM_A)          return "viewdimm_a";
  if (sf == SF_DIMM_B)          return "viewdimm_b";
  if (sf == SF_CHNL_SCALING_A)  return "scaling_a";
  if (sf == SF_CHNL_SCALING_B)  return "scaling_b";
  if (sf == SF_DATABOUNDS)      return "databounds";
  return nullptr;
}

DrawQWidget::DrawQWidget(ISheiGenerator* pcsh, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawCore(portions, orient, splitPortions),
  m_compileOnInitializeGL(true), m_vshalloc(0), m_fshalloc(0), m_pcsh(pcsh), 
  m_matrixLmSize(0), m_sbStatic(false), 
  m_cttrLeft(0), m_cttrTop(0), m_cttrRight(0), m_cttrBottom(0), m_texOvlCount(0)
{
  for (int i=0; i<_SF_COUNT; i++)
    m_locations[i] = -1;
  
  m_portionMeshType = splitPortions == SL_NONE? m_pcsh->portionMeshType() : ISheiGenerator::PMT_FORCE1D;  // ntf: strange, non-intuitive
  
  setFocusPolicy(Qt::ClickFocus);
//  setMouseTracking(true);
}

DrawQWidget::~DrawQWidget()
{
  if (m_pcsh)
    delete m_pcsh;
  
  if (m_vshalloc)
    delete []m_vshmem;
  if (m_fshalloc)
    delete []m_fshmem;
}

void DrawQWidget::compileWhenInitializeGL(bool cflag)
{
  m_compileOnInitializeGL = cflag;
}

//OR_LRBT=0,  OR_RLBT,  OR_LRTB,  OR_RLTB
//OR_TBLR,    OR_BTLR,  OR_TBRL,  OR_BTRL 
//inline const char*  fragment_rotateLRBT(){  return  "vec2 rotate(vec2 coords){ return coords; }"; }
//inline const char*  fragment_rotateRLBT(){  return  "vec2 rotate(vec2 coords){ coords.x = 1.0-coords.x; return coords; }"; }
//inline const char*  fragment_rotateLRTB(){  return  "vec2 rotate(vec2 coords){ coords.y = 1.0-coords.y; return coords; }"; }
//inline const char*  fragment_rotateRLTB(){  return  "vec2 rotate(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords; }"; }
//inline const char*  fragment_rotateTBLR(){  return  "vec2 rotate(vec2 coords){ coords.y = 1.0-coords.y; return coords.yx; }"; }
//inline const char*  fragment_rotateBTLR(){  return  "vec2 rotate(vec2 coords){ return coords.yx; }"; }
//inline const char*  fragment_rotateTBRL(){  return  "vec2 rotate(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords.yx; }"; }
//inline const char*  fragment_rotateBTRL(){  return  "vec2 rotate(vec2 coords){ coords.x = 1.0-coords.x; return coords.yx; }"; }

inline float*   ccode_swap(float* arr){ float t=arr[0]; arr[0] = arr[1]; arr[1] = t; return arr;   }
inline float*  ccode_rotateLRBT(float *arr){  return  arr; }
inline float*  ccode_rotateRLBT(float *arr){  arr[0] = 1.0f - arr[0]; return arr; }
inline float*  ccode_rotateLRTB(float *arr){  arr[1] = 1.0f - arr[1]; return arr; }
inline float*  ccode_rotateRLTB(float *arr){  arr[0] = 1.0f - arr[0]; arr[1] = 1.0f - arr[1]; return arr; }
inline float*  ccode_rotateTBLR(float *arr){  arr[1] = 1.0f - arr[1]; return ccode_swap(arr); }
inline float*  ccode_rotateBTLR(float *arr){  return ccode_swap(arr); }
inline float*  ccode_rotateTBRL(float *arr){  arr[0] = 1.0f - arr[0]; arr[1] = 1.0f - arr[1]; return ccode_swap(arr); }
inline float*  ccode_rotateBTRL(float *arr){  arr[0] = 1.0f - arr[0]; return ccode_swap(arr); }


inline const char*  fastpaced_settings(char* tmpbuf, unsigned int ovl)
{
  /// EQuals: srintf(_tempvd, "ovl_exsettings%d", i + 1);
  static char chset[] = {'0','1','2','3','4','5','6','7','8','9'};
  const char* base = "ovl_exsettings";
  char* p = tmpbuf;
  while (*base)  *p++ = *base++;
  *(p + 3) = '\0';
  for (int i=2; i>=0; i--)
  {
    *(p+i) = chset[ovl % 10];
    ovl /= 10;
  }
  return tmpbuf;
}

inline const char*  fastpaced_opm(char* tmpbuf, unsigned int ovl, unsigned int subpos)
{
  /// EQuals: srintf(_tempvd, "opm%d_%d", i + 1, j);
  static char chset[] = {'0','1','2','3','4','5','6','7','8','9'};
  const char* base = "opm";
  char* p = tmpbuf;
  while (*base)  *p++ = *base++;
  static char fhset[] = { '_', '\0' };
  for (int j=0; j<2; j++)
  {
    for (int i=2; i>=0; i--)
    {
      *(p+i) = chset[ovl % 10];
      ovl /= 10;
    }
    p += 3;
    *p++ = fhset[j];
    ovl = subpos;
  }
  return tmpbuf;
}

void DrawQWidget::palettePrepare(const IPalette* ppal, bool discrete, int levels)
{
  const void*   palArr;
  unsigned int  palSize;
  unsigned int  palFormat;
  
  ppal->getPalette(&palArr, &palSize, &palFormat);
  
  if (palFormat == IPalette::FMT_UNKNOWN)
    return;
  
  GLenum  formats[] = { GL_UNSIGNED_BYTE, GL_FLOAT };
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(palSize) / levels, levels, 0, GL_RGBA, formats[palFormat - 1], (const void*)palArr);
  if (discrete)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
}


#ifdef BSSHADER_DUMP
#include <QFile>
#endif
void DrawQWidget::initCollectAndCompileShader()
{
  glClearColor(m_clearcolor[0], m_clearcolor[1], m_clearcolor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  m_ShaderProgram.removeAllShaders();
     
  /// 1. Vertex shader
  /// mem alloc
  if (m_vshalloc == 0)
  {
    m_vshalloc = m_pcsh->shvertex_pendingSize();
    m_vshmem = new char[m_vshalloc];
  }
  
  /// store
  unsigned int vsh_written = m_pcsh->shvertex_store(m_vshmem);
  Q_ASSERT(vsh_written <= m_vshalloc);
//  Q_UNUSED(vsh_written);
//    qDebug()<<m_pcsh->shaderName()<<" vertex size "<<vsh_written<<" (had"<<m_vshalloc<<")";
  
  if (!m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, m_vshmem))
  {
    qDebug()<<Q_FUNC_INFO<<"... vertex shader failure!";
    return;
  }
  
  
  /// 2. Fragment shader
  /// mem alloc
  if (m_fshalloc == 0 || m_fshalloc < m_pcsh->shfragment_pendingSize(m_impulsedata, m_overlaysCount))
  {
    if (m_fshalloc != 0)
      delete []m_fshmem;
    m_fshalloc = m_pcsh->shfragment_pendingSize(m_impulsedata, m_overlaysCount);
    m_fshmem = new char[m_fshalloc];
  }
  
  /// store
  {
    ovlfraginfo_t ovlsinfo[OVLLIMIT];
    for (unsigned int i=0; i<m_overlaysCount; i++)
    {
      if (m_overlays[i].olinks.type != msstruct_t::MS_SELF && m_overlays[i].olinks.type != msstruct_t::MS_ROOT)
        ovlsinfo[i].link = m_overlays[i].olinks.details.drivenid;
      else
        ovlsinfo[i].link = -1;
    }
    unsigned int fsh_written = m_pcsh->shfragment_store(m_allocatedPortions, m_postMask, m_orient, m_splitPortions, 
                                                        m_impulsedata, m_overlaysCount, ovlsinfo, m_fshmem);
    
//    qDebug()<<m_pcsh->shaderName()<<" fragment size "<<fsh_written<<" (had"<<m_fshalloc<<")";
    Q_ASSERT(fsh_written <= m_fshalloc);
//    Q_UNUSED(fsh_written);
  }
  if (!m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, m_fshmem))
  {
    qDebug()<<Q_FUNC_INFO<<"... fragment shader failure!";
    return;
  }
  
  
#ifdef BSSHADER_DUMP
    QFile fout("fragshader.txt");
    fout.open(QFile::WriteOnly | QFile::Text);
    fout.write((const char*)m_fshmem);
    fout.write((const char*)"\n\n\n\n");
#endif
  
  char  ovlshaderbuf[8192*2];
  for (unsigned int i=0; i<m_overlaysCount; i++)
  {     
    int fshtResult = m_overlays[i].povl->fshTrace(i + 1, m_matrixSwitchAB, ovlshaderbuf);
    if (fshtResult <= 0)
      qDebug()<<Q_FUNC_INFO<<"OVL fshTrace failure!";
    else
      m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, ovlshaderbuf);
    
#ifdef BSSHADER_DUMP
    fout.write((const char*)ovlshaderbuf);
    fout.write((const char*)"\n\n\n\n");
#endif
    
    int fshcResult = m_overlays[i].povl->fshColor(i + 1, ovlshaderbuf);
    if (fshcResult <= 0)
      qDebug()<<Q_FUNC_INFO<<"OVL fshColor failure!";
    else
      m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,  ovlshaderbuf);
        
#ifdef BSSHADER_DUMP
    fout.write((const char*)ovlshaderbuf);
    fout.write((const char*)"\n\n\n\n");
#endif
  }
  
//  m_ShaderProgram.bindAttributeLocation("vertices", 0); ???
  
  if (m_ShaderProgram.link())
  {
    for (int i=0; i<_SF_COUNT; i++)
    {
      const char* vd = vardesc((SHEIFIELD)i);
      m_locations[i] = vd != 0? m_ShaderProgram.uniformLocation(vd) : -1;
    }    
    
    /// 2. Init ovl locations and textures
    {
      char _tempvd[32];
      unsigned int  texNew[96 - HT_OVERLAYSSTART];
      unsigned int  texNewCount=0;
      for (unsigned int i=0; i<m_overlaysCount; i++)
      {
        fastpaced_settings(_tempvd, i+1);
        m_overlays[i].outloc = m_ShaderProgram.uniformLocation(_tempvd);                  /// ! cannot hide by upcount
        AbstractDrawOverlay::uniforms_t  uf = m_overlays[i].povl->uniforms();
        for (unsigned int j=0; j<uf.count; j++)
        {
          fastpaced_opm(_tempvd, i+1, j);
          m_overlays[i].uf_arr[j].location = m_ShaderProgram.uniformLocation(_tempvd);
          if (m_overlays[i].uf_arr[j].location == -1)
          {
//            qDebug()<<"Location of var"<<_tempvd<<"wasnt found";
          }
          DTYPE dtype = uf.arr[j].type;
          m_overlays[i].uf_arr[j].type = dtype;
          m_overlays[i].uf_arr[j].dataptr = uf.arr[j].dataptr;
          if (dtIsTexture(dtype))
          {
            if (m_overlays[i].upcount > 0)
            {
              m_overlays[i].uf_arr[j].tex_idx = HT_OVERLAYSSTART + texNewCount;
              glGenTextures(1, &texNew[texNewCount]);
              texNewCount++;
            }
            else
              texNew[texNewCount++] = m_texAll[m_overlays[i].uf_arr[j].tex_idx];
          }
        }
        m_overlays[i].upcount = 1001;
      }
      m_texOvlCount = HT_OVERLAYSSTART;
      for (unsigned int i=0; i<texNewCount; i++)
        m_texAll[m_texOvlCount++] = texNew[i];
    } // init ovl
    
    unpend(PC_INIT);
    m_bitmaskPendingChanges |= (m_groundData == nullptr? 0 : PC_GROUND) | PC_SIZE | PC_DATA | PC_PARAMS | PC_PARAMSOVL;
    
  } /// link
  
#ifdef BSSHADER_DUMP
  fout.close();
#endif
}

void DrawQWidget::initializeGL()
{
  m_SurfaceVertex[0] = -1.0f; m_SurfaceVertex[1] = -1.0f;
  m_SurfaceVertex[2] =  1.0f; m_SurfaceVertex[3] = -1.0f;
  m_SurfaceVertex[4] = -1.0f; m_SurfaceVertex[5] =  1.0f;
  m_SurfaceVertex[6] =  1.0f; m_SurfaceVertex[7] =  1.0f;
  
  initializeOpenGLFunctions();
  glGenTextures(1, &m_texAll[HT_MATRIX]); /// matrix
  glGenTextures(1, &m_texAll[HT_PAL]);  /// palette
  glGenTextures(1, &m_texAll[HT_GND]);  /// ground: domain/SDP(specialy destroyed picture)

  if (m_compileOnInitializeGL)
    initCollectAndCompileShader();
  
  m_bitmaskPendingChanges |= PC_SIZE | PC_DATA | PC_PARAMS | PC_PARAMSOVL; 
//  qDebug("GL: initialized. %04x", m_bitmaskPendingChanges);
}

void DrawQWidget::paintGL()
{ 
//  qDebug("blublublu %04x", m_bitmaskPendingChanges);
//  if (!havePending())
//    return;
  
  
  glDisable(GL_DEPTH_TEST);
  {
//    glDisable(GL_BLEND);
//    glDisable(GL_COLOR_MATERIAL);
//    glDisable(GL_LINE_SMOOTH);
//    glDisable(GL_MINMAX);
//    glDisable(GL_POINT_SPRITE);
//    glDisable(GL_STENCIL_TEST);
//    glDisable(GL_SCISSOR_TEST);
//    glDisable(GL_NORMALIZE);
//    glDisable();
    glDisable(GL_DITHER);
#ifdef GL_MULTISAMPLE
    glDisable(GL_MULTISAMPLE);
#endif
  }
  {
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//    glEnable(GL_TEXTURE_2D);
//    glDisable(GL_BLEND);
//    glDisable(GL_ALPHA_TEST);
  }
  
//  if (!havePending())
//  {
//    static int ctr=0;
//    qDebug()<<"EMMM"<<ctr++;
//  }
//    glEnable(GL_BLEND);
  
  if (havePendOn(PC_INIT))
    initCollectAndCompileShader();
  else
  {
    if (m_clearupdated || !havePending())
    {
      glClearColor(m_clearcolor[0], m_clearcolor[1], m_clearcolor[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT); // on initCollectAndCompileShader
      m_clearupdated = false;
    }
  }
  
  if (m_ShaderProgram.bind())
  {
//    qDebug()<<"paintGL binded:"<<m_ShaderProgram.isLinked()<<m_ShaderProgram.programId();
    
    int loc;
    
    if ((loc = m_locations[SF_DATA]) != -1)
    {      
      glActiveTexture(GL_TEXTURE0 + HT_MATRIX);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_MATRIX]);
      if (havePendOn(PC_DATA) || havePendOn(PC_SIZE))
      {
        DATADIMMUSAGE ddu = this->getDataDimmUsage();
        GLsizei dataDimmA = m_matrixDimmA;
        GLsizei dataDimmB = m_matrixDimmB;
        if (ddu == DDU_1D || ddu == DDU_DD)
        {
          dataDimmB = 1;
          dataDimmA = m_portionSize;
        }
        else if (ddu == DDU_POLAR)
        {
          dataDimmB = dataDimmB / 2;
          dataDimmA = m_portionSize / dataDimmB;
        }
        if (havePendOn(PC_DATA) || ddu == DDU_15D)
        {
          unsigned int total = dataDimmA*dataDimmB*m_countPortions;
          for (unsigned int i=0; i<total; i++)
            m_matrixDataCached[i] = m_matrixData[i] * m_loc_k + m_loc_b;
        }
        glTexImage2D(   GL_TEXTURE_2D, 0, 
#if QT_VERSION >= 0x050000
                        GL_R32F, 
#elif QT_VERSION >= 0x040000
                        GL_RED, 
#endif
                        dataDimmA, dataDimmB*m_countPortions, 0, GL_RED, GL_FLOAT, m_matrixDataCached);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_dataTextureInterp? GL_LINEAR : GL_NEAREST);  // GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_dataTextureInterp? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        
        if ((loc = m_locations[SF_DATABOUNDS]) != -1)     m_ShaderProgram.setUniformValue(loc, (const QVector2D&)m_bounds);
        
      }
      m_ShaderProgram.setUniformValue(loc, HT_MATRIX);
    }
         
    if ((loc = m_locations[SF_PALETTE]) != -1)
    {
      glActiveTexture(GL_TEXTURE0 + HT_PAL);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_PAL]);
      if (m_ppal && havePendOn(PC_PALETTE))
      {
//        palettePrepare(m_ppal, m_ppaldiscretise, m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D && m_countPortions != 0? m_countPortions : 1);
        palettePrepare(m_ppal, m_ppaldiscretise, m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D && m_allocatedPortions != 0? m_allocatedPortions : 1);
        if (m_clearbypalette)
          _colorCvt(this->colorBack());
      }
      m_ShaderProgram.setUniformValue(loc, HT_PAL);
    }
    
    if ((loc = m_locations[SF_GROUND]) != -1)
    {
      glActiveTexture(GL_TEXTURE0 + HT_GND);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_GND]);
      
      if (havePendOn(PC_GROUND))
      {
        switch (m_groundType)
        {
        case GND_DOMAIN:
        {
          float* groundData = (float*)m_groundData;
          float* groundDataCached=m_groundDataFastFree? groundData : new float[m_matrixDimmA*m_matrixDimmB];
          for (unsigned int i=0; i<m_matrixDimmA*m_matrixDimmB; i++)
            groundDataCached[i] = groundData[i] / (m_portionSize+1);
        
          glTexImage2D(   GL_TEXTURE_2D, 0, 
#if QT_VERSION >= 0x050000
                          GL_R32F, 
#elif QT_VERSION >= 0x040000
                          GL_RED, 
#endif
                          m_matrixDimmA, m_matrixDimmB, 0, GL_RED, GL_FLOAT, groundDataCached);
  //          glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          
          if (m_groundMipMapping) glGenerateMipmap( GL_TEXTURE_2D );
          
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_groundMipMapping? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          
          delete []groundDataCached;
          if (m_groundDataFastFree)
            m_groundData = nullptr;
          break;
        }
        case GND_SDP: 
        {
          glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, m_groundDataWidth, m_groundDataHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_groundData);
          if (m_groundMipMapping) glGenerateMipmap( GL_TEXTURE_2D );
          
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_groundMipMapping? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
          break;
        }
        case GND_ASSISTFLOATTABLE:
        {
          glTexImage2D(  GL_TEXTURE_2D, 0, 
 #if QT_VERSION >= 0x050000
                           GL_R32F, 
 #elif QT_VERSION >= 0x040000
                           GL_RED, 
 #endif
                         m_groundDataWidth, m_groundDataHeight, 0, GL_RGBA, GL_FLOAT, m_groundData);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
          break;
        }
        default:  Q_ASSERT(havePendOn(PC_GROUND) && m_groundType != GND_NONE); break;
        }
      } // if pend on ground
      m_ShaderProgram.setUniformValue(loc, HT_GND);
    }
    
    if (havePendOn(PC_GROUND))
    {
      if ((loc = m_locations[SF_PORTIONSIZE]) != -1)
      {
        m_ShaderProgram.setUniformValue(loc, m_portionSize);
      }
    }
    
    if (havePendOn(PC_SIZE))
    {
      if ((loc = m_locations[SF_DIMM_A]) != -1)         m_ShaderProgram.setUniformValue(loc, m_matrixDimmA);
      if ((loc = m_locations[SF_DIMM_B]) != -1)         m_ShaderProgram.setUniformValue(loc, m_matrixDimmB);
      if ((loc = m_locations[SF_CHNL_SCALING_A]) != -1) m_ShaderProgram.setUniformValue(loc, m_scalingA);
      if ((loc = m_locations[SF_CHNL_SCALING_B]) != -1) m_ShaderProgram.setUniformValue(loc, m_scalingB);
    }
    
    if (havePendOn(PC_PARAMS))
    {
      if ((loc = m_locations[SF_COUNTPORTIONS]) != -1)  m_ShaderProgram.setUniformValue(loc, this->m_countPortions);
    }
    
    for (unsigned int i=0; i<m_texOvlCount; i++)
    {
      glActiveTexture(GL_TEXTURE0 + HT_OVERLAYSSTART + i);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_OVERLAYSSTART + i]);
    }
    
    if (havePendOn(PC_PARAMSOVL))
    {        
      for (unsigned int i=0; i<m_overlaysCount; i++)
      {
        if (m_overlays[i].upcount > 0)
        {
          if ((loc = m_overlays[i].outloc) != -1)
            m_ShaderProgram.setUniformValue(loc, QVector4D(m_overlays[i].povl->getOpacity(), m_overlays[i].povl->getThickness(), m_overlays[i].povl->getSlice(), 0.0f));
          
          for (unsigned int j=0; j<m_overlays[i].uf_count; j++)
          {
            uniform_located_t& ufm = m_overlays[i].uf_arr[j];
            int         loc = ufm.location;
            const void* data = ufm.dataptr;
            switch (ufm.type) {
              case DT_1F: m_ShaderProgram.setUniformValue(loc, *(const GLfloat*)data); break;
              case DT_2F: m_ShaderProgram.setUniformValue(loc, *(const QVector2D*)data); break;
              case DT_3F: m_ShaderProgram.setUniformValue(loc, *(const QVector3D*)data); break;
              case DT_4F: m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)data); break;
              case DT_1I: m_ShaderProgram.setUniformValue(loc, *(const GLint*)data); break;
//              case DT_3I: m_ShaderProgram.setUniformValue(loc, *(QSize*)data); break;
//                case DT_2I: m_ShaderProgram.setUniformValue(loc, *(QSize*)data); break;
              case DT_ARR: case DT_ARR2: case DT_ARR3: case DT_ARR4:
              case DT_ARRI: case DT_ARRI2: case DT_ARRI3: case DT_ARRI4:
              {
                const dmtype_arr_t* parr = (const dmtype_arr_t*)data;
                switch (ufm.type)
                {
                case DT_ARR : m_ShaderProgram.setUniformValueArray(loc, (const GLfloat*)parr->data, parr->count, 1); break;
                case DT_ARR2: m_ShaderProgram.setUniformValueArray(loc, (const QVector2D*)parr->data, parr->count); break;
                case DT_ARR3: m_ShaderProgram.setUniformValueArray(loc, (const QVector3D*)parr->data, parr->count); break;
                case DT_ARR4: m_ShaderProgram.setUniformValueArray(loc, (const QVector4D*)parr->data, parr->count); break;
                case DT_ARRI : m_ShaderProgram.setUniformValueArray(loc, (const int*)parr->data, parr->count); break;
//                  case DT_ARRI2: m_ShaderProgram.setUniformValueArray(loc, (const QVector2D*)parr->data, parr->count); break;
//                  case DT_ARRI3: m_ShaderProgram.setUniformValueArray(loc, (const QVector3D*)parr->data, parr->count); break;
//                  case DT_ARRI4: m_ShaderProgram.setUniformValueArray(loc, (const QVector4D*)parr->data, parr->count); break;
                }
                break;
              }
              case DT_SAMP4:
              {
                const dmtype_sampler_t* psampler = (const dmtype_sampler_t*)data;
                if (m_overlays[i].upcount >= 1001)
                {
                  glActiveTexture(GL_TEXTURE0 + ufm.tex_idx);
//                  glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(psampler->count), 1, 0, GL_RGBA, GL_FLOAT, psampler->data);
                  glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(psampler->count), 1, 0, GL_RGBA, GL_FLOAT, psampler->data);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  m_ShaderProgram.setUniformValue(loc, ufm.tex_idx);
                }
                break;
              }
              case DT_TEXTURE:
              {
                if (m_overlays[i].upcount >= 1001)
                {
                  glActiveTexture(GL_TEXTURE0 + ufm.tex_idx);
//                  glBindTexture(GL_TEXTURE_2D, m_textures[ufm.tex_idx]);
                  const dmtype_image_t* pimage = (const dmtype_image_t*)data;
                  
                  if (pimage->type != dmtype_image_t::NONE)
                  {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
                    glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
                    
                    glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
                    glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
                    glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
                    glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
                    
                    GLint   gl_internalFormat;
                    GLenum  gl_format;
                    GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
                    
#if QT_VERSION >= 0x050000
                    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
                    glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
                    if (pimage->type == dmtype_image_t::RGB)            {    gl_internalFormat = GL_RGB;    gl_format = GL_RGB;   }
                    else if (pimage->type == dmtype_image_t::RGBA)      {    gl_internalFormat = GL_RGBA8;   gl_format = GL_RGBA;   }
                    else if (pimage->type == dmtype_image_t::FASTALPHA) {    gl_internalFormat = GL_ALPHA8;   gl_format = GL_ALPHA;   }
#elif QT_VERSION >= 0x040000
                    if (pimage->type == dmtype_image_t::RGB)            {    gl_internalFormat = GL_RGB;    gl_format = GL_RGB;   }
                    else if (pimage->type == dmtype_image_t::RGBA)      {    gl_internalFormat = GL_RGBA;   gl_format = GL_RGBA;   }
                    else if (pimage->type == dmtype_image_t::FASTALPHA) {    gl_internalFormat = GL_RGBA;   gl_format = GL_RGBA;   }
#endif
                    glTexImage2D(  GL_TEXTURE_2D, 0, gl_internalFormat, pimage->w, pimage->h, 0, gl_format, gl_texture_type, pimage->data);
                  }
                  m_ShaderProgram.setUniformValue(loc, ufm.tex_idx);
                }
                break;
              }
              case DT_PALETTE: case DT__HC_PALETTE:
              {
                if (m_overlays[i].upcount >= 1001)
                {
                  glActiveTexture(GL_TEXTURE0 + ufm.tex_idx);
                  const dmtype_palette_t* cdp = (const dmtype_palette_t*)ufm.dataptr;
                  palettePrepare(cdp->ppal, cdp->discrete, 1);
                  m_ShaderProgram.setUniformValue(loc, ufm.tex_idx);
                }
                break;
              }
              default:
              qDebug()<<"BSDraw failure: unknown parameter type!";
              break;
            }
          } // for ov_ufs
          m_overlays[i].upcount = 0;
        } // if upcount
      } // for overlays
    }
    m_ShaderProgram.enableAttributeArray(0);
    m_ShaderProgram.setAttributeArray(0, GL_FLOAT, m_SurfaceVertex, 2);
    
    if (!m_rawResizeModeNoScaled)
    {
//      int sizeA = m_matrixDimmA*m_scalingA*(m_spDivider == 0? 1 : m_spDirection == 1? _divider2() : m_spDivider);
//      int sizeB = m_matrixDimmB*m_scalingB*(m_spDivider == 0? 1 : m_spDirection == 1? m_spDivider : _divider2());
      if (m_matrixSwitchAB)
        glViewport(0 + m_cttrLeft, height() - sizeA() - m_cttrTop, sizeB(), sizeA());
      else
        glViewport(0 + m_cttrLeft, height() - sizeB() - m_cttrTop, sizeA(), sizeB());
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_ShaderProgram.disableAttributeArray(0);
    m_ShaderProgram.release();
    
    unpendAll();
    for (unsigned int i=0; i<m_texOvlCount; i++)
      glBindTexture(GL_TEXTURE_2D, 0);
  } /// if bind
}

void DrawQWidget::resizeGL(int w, int h)
{
  getContentsMargins(&m_cttrLeft, &m_cttrTop, &m_cttrRight, &m_cttrBottom);
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  
//  qDebug()<<"DrawQWidget PRE resizeEv: "<<QSize(w,h)<<m_matrixDimmA<<m_matrixDimmB<<m_scalingA<<m_scalingB;
  if (m_matrixSwitchAB)
    adjustSizeAndScale(h, w);
  else
    adjustSizeAndScale(w, h);
  
  
  if (m_rawResizeModeNoScaled)
  {
    if (m_matrixSwitchAB)
      glViewport(0 + m_cttrLeft, height() - sizeA() - m_cttrTop, sizeB(), sizeA());
    else
      glViewport(0 + m_cttrLeft, height() - sizeB() - m_cttrTop, sizeA(), sizeB());
  }
  
  m_clearupdated = true;
  pendResize(false);
}


void DrawQWidget::fitSize(int width_in, int height_in, int* actualwidth, int* actualheight) const
{
  width_in -= m_cttrLeft + m_cttrRight;
  height_in -= m_cttrTop + m_cttrBottom;
  
  unsigned int dimmA, dimmB, scalingA, scalingB;
  if (m_matrixSwitchAB)
    sizeAndScaleHint(height_in, width_in, &dimmB, &dimmA, &scalingB, &scalingA);
  else
    sizeAndScaleHint(width_in, height_in, &dimmA, &dimmB, &scalingA, &scalingB);
  *actualwidth = dimmA*scalingA;
  *actualheight = dimmB*scalingB;
}


void DrawQWidget::callWidgetUpdate()
{
//  qDebug()<<"callWidgetUpdate called...";
  QOpenGLWidget::update();
//  setUpdatesEnabled(false);
//  QOpenGLWidget::repaint();   /// better way: call banAutoUpdate for data, then explicitly call repaint in your GUI thread
  //  setUpdatesEnabled(true);
}

void DrawQWidget::innerRescale()
{
  int w = width(), h = height();
  getContentsMargins(&m_cttrLeft, &m_cttrTop, &m_cttrRight, &m_cttrBottom);
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  
  if (m_matrixSwitchAB)
    adjustSizeAndScale(h, w);
  else
    adjustSizeAndScale(w, h);
}

void DrawQWidget::innerUpdateGeometry()
{
  this->updateGeometry();
}

QSize DrawQWidget::minimumSizeHint() const
{
  int sizeA = m_scalingAMin * m_matrixDimmA * m_splitterA;
  int sizeB = m_scalingBMin * (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB) * m_splitterB;
  return m_matrixSwitchAB ? QSize( sizeB + m_cttrLeft + m_cttrRight, sizeA + m_cttrTop + m_cttrBottom ) : QSize( sizeA + m_cttrLeft + m_cttrRight, sizeB + m_cttrTop + m_cttrBottom );
}

QSize DrawQWidget::sizeHint() const
{ 
  int sizeA = m_scalingA * m_matrixDimmA * m_splitterA;
  int sizeB = m_scalingB * (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB) * m_splitterB;
  return m_matrixSwitchAB ? QSize( sizeB + m_cttrLeft + m_cttrRight, sizeA + m_cttrTop + m_cttrBottom ) : QSize( sizeA + m_cttrLeft + m_cttrRight, sizeB + m_cttrTop + m_cttrBottom );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const bool isPress[] = { true, false, false, true, true, false, true };

void  DrawQWidget::store_crd_clk(OVL_REACTION_MOUSE oreact, int x, int y)
{
  int singleDimmWidth = sizeHorz();
  int singleDimmHeight = sizeVert();
  int totalDimmWidth = singleDimmWidth * (m_matrixSwitchAB? m_splitterB : m_splitterA);
  int totalDimmHeight = singleDimmHeight * (m_matrixSwitchAB? m_splitterA : m_splitterB);

  if (isPress[oreact] == false)
  {
    if (x < 0)  x = 0; else if (x >= m_cttrLeft + totalDimmWidth) x = m_cttrLeft + totalDimmWidth - 1;
    if (y < 0)  y = 0; else if (y >= m_cttrTop + totalDimmHeight) y = m_cttrTop + totalDimmHeight - 1;
  }
  else if (x >= m_cttrLeft + totalDimmWidth || y >= m_cttrTop + totalDimmHeight)
    return;

//  float dataptr[] = { float(x - m_cttrLeft) / (dimmWidth-1), 1.0f - float(y - m_cttrTop) / (dimmHeight-1), float(x), float(y) };
  float dataptr[] = { float((x - m_cttrLeft)%singleDimmWidth) / (singleDimmWidth-1), 
                      1.0f - float((y - m_cttrTop)%singleDimmHeight) / (singleDimmHeight-1), 
                      float(x - m_cttrLeft), float(y - m_cttrTop)
                    };
  float* (*pfns[])(float*) = {  ccode_rotateLRBT, ccode_rotateRLBT, ccode_rotateLRTB, ccode_rotateRLTB,
                                ccode_rotateTBLR, ccode_rotateBTLR, ccode_rotateTBRL, ccode_rotateBTRL
                             };
  float* rslt = (*pfns[m_orient])(dataptr);
  
  bool doStop = false, doUpdate = false;
  
  if (m_proactive)  m_proactive->overlayReactionMouse(this, oreact, rslt, &doStop);
  if (!doStop)
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
    {
      if (m_overlays[i].povl->overlayReactionMouse(oreact, rslt, &doStop))
      {
        m_overlays[i].upcount++;
        doUpdate = true;
      }
      if (doStop)
        break;
    }
  
//    if (!havePendOn(PC_PARAMSOVL) && doUpdate)
  if (doUpdate)
  {
    m_bitmaskPendingChanges |= PC_PARAMSOVL;
    if (!autoUpdateBanned(RD_BYOVL_ACTIONS))
      callWidgetUpdate();
  }
}

#define BUTTONCHECK_DOGSHIT(action) Qt::MouseButton btn = event->button(); \
                                    QPoint pos = event->pos(); \
                                    if (btn == Qt::LeftButton)        store_crd_clk(ORM_LM##action, pos.x(), pos.y()); \
                                    else if (btn == Qt::RightButton)  store_crd_clk(ORM_RM##action, pos.x(), pos.y());

#define BUTTONCHECK_BULLSHIT(action)  Qt::MouseButtons btn = event->buttons(); \
                                      QPoint pos = event->pos(); \
                                      if (btn & Qt::LeftButton)        store_crd_clk(ORM_LM##action, pos.x(), pos.y()); \
                                      else if (btn & Qt::RightButton)  store_crd_clk(ORM_RM##action, pos.x(), pos.y());


void DrawQWidget::mousePressEvent(QMouseEvent *event)
{
  BUTTONCHECK_DOGSHIT(PRESS)
//  if (event->button() == Qt::LeftButton)  store_crd_clk(ORM_LMPRESS, event->pos().x(), event->pos().y());
//  else if (event->button() == Qt::RightButton)  store_crd_clk(ORM_RMPRESS, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseReleaseEvent(QMouseEvent *event)
{
  BUTTONCHECK_DOGSHIT(RELEASE)
//  if (event->button() == Qt::LeftButton)  store_crd_clk(ORM_LMRELEASE, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseMoveEvent(QMouseEvent *event)
{
  BUTTONCHECK_BULLSHIT(MOVE)
//  if (event->buttons() & Qt::LeftButton)  store_crd_clk(ORM_LMMOVE, event->pos().x(), event->pos().y());
//  store_crd_clk(ORM_LMMOVE, event->pos().x(), event->pos().y());
  //  setMouseTracking(tru!!);
}

void DrawQWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)  store_crd_clk(ORM_LMDOUBLE, event->pos().x(), event->pos().y());
}

void DrawQWidget::keyPressEvent(QKeyEvent* event)
{
  int modifiers = int(event->modifiers()) >> 24;
  int key = event->key();
  bool doStop = false, doUpdate = false;
  if (m_proactive)  m_proactive->overlayReactionKey(this, key, modifiers, &doStop);
  if (!doStop)
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
    {
      if (m_overlays[i].povl->overlayReactionKey(key, modifiers, &doStop))
      {
        m_overlays[i].upcount++;
        doUpdate = true;
      }
      if (doStop)
        break;
    }
  
  if (doUpdate)
  {
    m_bitmaskPendingChanges |= PC_PARAMSOVL;
    if (!autoUpdateBanned(RD_BYOVL_ACTIONS))
      callWidgetUpdate();
  }
  QWidget::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QScrollBar>

void DrawQWidget::connectScrollBar(QScrollBar *qsb, bool staticView, bool setOrientation)
{
  if (m_matrixLmSize == 0)  return;
  qsb->setRange(0, m_matrixLmSize-1);
  
  if (setOrientation)
  {
    if ((qsb->orientation() == Qt::Vertical   && (this->m_orient == OR_LRBT || this->m_orient == OR_RLBT)) ||
        (qsb->orientation() == Qt::Horizontal && (this->m_orient == OR_LRTB || this->m_orient == OR_LRBT)) )
    {
      qsb->setInvertedAppearance(true);
//      qsb->setInvertedControls(true);
//      qsb->setValue(qsb->minimum());
    }
  }
  m_sbStatic = staticView;
  QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(scrollDataTo(int)));
}


void DrawQWidget::slot_setScalingH(int s){  setScalingLimitsHorz(s, s); }
void DrawQWidget::slot_setScalingV(int s){  setScalingLimitsVert(s, s); }

void DrawQWidget::slot_setBounds(float low, float high){  setBounds(low, high); }
void DrawQWidget::slot_setBoundLow(float value){  setBoundLow(value); }
void DrawQWidget::slot_setBoundHigh(float value){ setBoundHigh(value);  }
void DrawQWidget::slot_setContrast(float k, float b){ setContrast(k, b);  }
void DrawQWidget::slot_setContrastK(float k){ setContrastK(k);  }
void DrawQWidget::slot_setContrastB(float b){ setContrastB(b);  }
void DrawQWidget::slot_setDataTextureInterpolation(bool d){ setDataTextureInterpolation(d); }
void DrawQWidget::slot_setDataPalette(const IPalette* ppal){ setDataPalette(ppal); }
void DrawQWidget::slot_setDataPaletteDiscretion(bool d){ setDataPaletteDiscretion(d); }
void DrawQWidget::slot_setData(const float* data){ setData(data); }
void DrawQWidget::slot_setData(QVector<float> data){ setData(data.constData()); }
void DrawQWidget::slot_fillData(float data){ fillData(data); }
void DrawQWidget::slot_clearData(){ clearData(); }

void DrawQWidget::slot_setMirroredHorz(){ setMirroredHorz(); }
void DrawQWidget::slot_setMirroredVert(){ setMirroredVert(); }
void DrawQWidget::slot_setPortionsCount(int count){  setPortionsCount(count); }

void DrawQWidget::slot_enableAutoUpdate(bool enabled){  banAutoUpdate(!enabled); }
void DrawQWidget::slot_disableAutoUpdate(bool disabled){  banAutoUpdate(disabled); }
void DrawQWidget::slot_enableAutoUpdateByData(bool enabled){  banAutoUpdate(RD_BYDATA, !enabled); }
void DrawQWidget::slot_disableAutoUpdateByData(bool disabled){  banAutoUpdate(RD_BYDATA, disabled); }

////////////////////////////////////////////////////////////////////////

int   DrawQWidget::scrollValue() const
{
  return 0;
}

void  DrawQWidget::scrollDataTo(int)
{
}

////////////////////////////////////////////////////////////////////////////////

DrawQWidget::MemExpand2D::MemExpand2D(unsigned int portionsCount, unsigned int portionSize, unsigned int linesMemory): pc(portionsCount), ps(portionSize), memoryLines(linesMemory), 
  filled(0), current(0)
{
  m_extendeddataarr = new float[memoryLines*pc*ps];
}

DrawQWidget::MemExpand2D::~MemExpand2D()
{
  delete []m_extendeddataarr;
}

void DrawQWidget::MemExpand2D::onSetData(const float *data)
{
  if (++current >= memoryLines)    current = 0;
  memcpy(&m_extendeddataarr[current*pc*ps], data, pc*ps*sizeof(float));
  if (filled < memoryLines)    filled++;
}

void DrawQWidget::MemExpand2D::onSetData(const float* data, DataDecimator* decim)
{
  if (++current >= memoryLines)    current = 0;
  for (unsigned int p=0; p<pc; p++)
    for (unsigned int i=0; i<ps; i++)
      m_extendeddataarr[current*pc*ps + p*ps + i] = decim->decimate(data, ps, i, p);
  if (filled < memoryLines)    filled++;
}

void DrawQWidget::MemExpand2D::onClearData()
{
  filled = 0;
}

bool DrawQWidget::MemExpand2D::onFillData(int portion, int pos, float *rslt) const
{
  if (pos < (int)filled)
  {
    int j = (int)current - (int)pos;
    if (j < 0)  j += memoryLines;
    memcpy(rslt, &m_extendeddataarr[j*pc*ps + portion*ps], ps*sizeof(float));
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawQWidget::MemExpand1D::MemExpand1D(unsigned int portionsCount, unsigned int portionSize, unsigned int memorySize): pc(portionsCount), ps(portionSize), pm(portionSize + memorySize), 
  rounded(false), current(0)
{
  m_extendeddataarr = new float[pc*pm];
}

DrawQWidget::MemExpand1D::~MemExpand1D()
{
  delete []m_extendeddataarr;
}

void DrawQWidget::MemExpand1D::reinit(unsigned int portionSize)
{
  ps = portionSize;
}

void DrawQWidget::MemExpand1D::onSetData(const float *data, unsigned int size)
{
  int left = pm - current;
  if (left > (int)size)
  {
    for (unsigned int p=0; p<pc; p++)
      memcpy(&m_extendeddataarr[p*pm + current], data + p*size, size*sizeof(float));
    current += size;
  }
  else
  {
    for (unsigned int p=0; p<pc; p++)
    {
      memcpy(&m_extendeddataarr[p*pm + current], data + p*size, left*sizeof(float));
      memcpy(&m_extendeddataarr[p*pm + 0], data + p*size + left, (size - left)*sizeof(float));
    }
    current = size - left;
    rounded = true;
  }
}

void DrawQWidget::MemExpand1D::onSetData(const float* data, unsigned int size, DataDecimator* decim)
{
  int left = pm - current;
  if (left > (int)size)
  {
    for (unsigned int p=0; p<pc; p++)
      for (unsigned int i=0; i<size; i++)
        m_extendeddataarr[p*pm + current] = decim->decimate(data + p*size, size, i, p);
    current += size;
  }
  else
  {
    for (unsigned int p=0; p<pc; p++)
    {
      for (int i=0; i<left; i++)
        m_extendeddataarr[p*pm + current] = decim->decimate(data + p*size, left, i, p);
      for (unsigned int i=0; i<size-left; i++)
        m_extendeddataarr[p*pm + 0] = decim->decimate(data + p*size + left, (size - left), i, p);
    }
    current = size - left;
    rounded = true;
  }
}

void DrawQWidget::MemExpand1D::onClearData()
{
  current = 0;
  rounded = false;
}

void DrawQWidget::MemExpand1D::onFillData(int pos, float *rslt, float emptyfill) const
{
  pos = current - ps - pos;
  if (pos < 0)
  {
    if (rounded)
      while (pos < 0)
        pos += pm;
    else
      pos = 0;
  }
  unsigned int fillSize[] = { ps, 0 };  /// first copy, second copy (by loop or emptyfill)
  bool sndIsEmpty = false;
  
  if (rounded == false && pos > (int)current)
  {
    fillSize[0] = 0;
    fillSize[1] = ps;
    sndIsEmpty = true;
  }
  else if (rounded == false && current - pos < ps)
  {
    fillSize[0] = current - pos;
    fillSize[1] = ps - fillSize[0];
    sndIsEmpty = true;
  }
  else if (rounded == true && pos + ps > pm)
  {
    fillSize[0] = pm - pos;
    fillSize[1] = ps - fillSize[0];
  }
  
  for (unsigned int p=0; p<pc; p++)
  {
    memcpy(rslt + p*ps, &m_extendeddataarr[p*pm + pos], fillSize[0]*sizeof(float));
    if (sndIsEmpty == false)
      memcpy(rslt  + p*ps + fillSize[0], &m_extendeddataarr[p*pm + 0], fillSize[1]*sizeof(float));
    else
      for (unsigned int i=0; i<fillSize[1]; i++)
        rslt[fillSize[0]  + p*ps + i] = emptyfill;
  }
}

////////////////////////////////////////////////////////////////////////////////


class OverlayEmpty: public DrawOverlay
{
public:
  virtual int  fshTrace(int, char*) const;
  virtual int  fshColor(int, char*) const;
};

///////////////////////////////////////
extern int msprintf(char* to, const char* format, ...);

int DrawCore::OverlayEmpty::fshTrace(int overlay, bool /*rotated*/, char* to) const
{
  return msprintf(to, "vec4 overlayTrace%d(in ivec2 icell, in vec4 coords, in float thick, in ivec2 mastercoords, out ivec2 selfposition){ return vec4(0.0,0.0,0.0,0.0); }\n", overlay);
}

int DrawCore::OverlayEmpty::fshColor(int overlay, char* to) const
{
  return msprintf(to, "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) { return undercolor; }\n", overlay);
}


///////////////////////////////////////

bool BSQDoubleClicker::overlayReactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  if (oreact == ORM_LMDOUBLE)
  {
    *doStop = true;
    emit doubleClicked();
//    emit doubleClicked(pwdg->mapToGlobal(QPoint(((const float*)dataptr)[0], ((const float*)dataptr)[1])));
//    emit doubleClicked(pwdg->mapToGlobal(QPoint(((const float*)dataptr)[2], ((const float*)dataptr)[3])));
    emit doubleClicked(QPoint(((const float*)dataptr)[2], ((const float*)dataptr)[3]));
  }
  return false;
}

bool BSQProactiveSelector::overlayReactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  if (oreact == ORM_LMDOUBLE)
  {
    *doStop = true;
//    if (pwdg->splitterA() > 1 || pwdg->splitterB() > 1)  ???
    {
      unsigned int ssA = pwdg->sizeA()/pwdg->splitterA(), ssB = pwdg->sizeB()/pwdg->splitterB();
      int x = ((const float*)dataptr)[2], y = ((const float*)dataptr)[3];
      int portion = 0;
      if (pwdg->isSplittedA())
        portion = pwdg->allocatedPortions() - 1 - y / ssB;
      else if (pwdg->isSplittedB())
        portion = x / ssA;
      emit portionSelected(portion);
    }
  }
  return false;
}

bool BSQCellSelector::overlayReactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  if (oreact == ORM_LMPRESS)
  {
    *doStop = true;
    ORIENTATION orient = pwdg->orientation();
    {
      unsigned int ssA = pwdg->sizeA()/pwdg->sizeDataA(), ssB = pwdg->sizeB()/pwdg->sizeDataB();
      int x = ((const float*)dataptr)[2], y = ((const float*)dataptr)[3];
      unsigned int cellA = (orientationTransposed(orient)? y : x) / ssA;
      unsigned int cellB = (orientationTransposed(orient)? x : y) / ssB;
      if (orientationMirroredHorz(orient))
        cellA = pwdg->sizeDataA() - 1 - cellA;
      if (orientationMirroredVert(orient))
        cellB = pwdg->sizeDataB() - 1 - cellB;
      emit cellSelected(cellA, cellB);
    }
  }
  return false;
}
