#include "bsqdraw.h"

#include "../palettes/bsipalette.h"

const char*   DrawQWidget::vardesc(SHEIFIELD sf)
{
  if (sf == SF_DATA)                return "texData";
//  if (sf == SF_BOUNDS)              return "bounds";
//  if (sf == SF_CONTRAST)            return "contrast";
  if (sf == SF_PALETTE)             return "texPalette";
  if (sf == SF_DOMAIN)              return "domainarr";
  if (sf == SF_PORTIONSIZE)         return "domainscount";
  if (sf == SF_COUNTPORTIONS)       return "countPortions";
  if (sf == SF_DIMM_A)              return "datadimm_a";
  if (sf == SF_DIMM_B)              return "datadimm_b";
  if (sf == SF_CHNL_HORZSCALING)    return "scaling_a";
  if (sf == SF_CHNL_VERTSCALING)    return "scaling_b";
  return nullptr;
}

DrawQWidget::DrawQWidget(ISheiGenerator* pcsh, unsigned int portions, ORIENTATION orient): DrawCore(portions, orient), m_vshalloc(0), m_fshalloc(0), m_pcsh(pcsh), 
  m_matrixLmSize(0), m_sbStatic(false), 
  m_cttrLeft(0), m_cttrTop(0), m_cttrRight(0), m_cttrBottom(0), m_texOvlCount(0)
{
  for (int i=0; i<_SF_COUNT; i++)
    m_locations[i] = -1;
  
  m_portionMeshType = m_pcsh->portionMeshType();  
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

//OR_LRBT=0,  OR_RLBT,  OR_LRTB,  OR_RLTB
//OR_TBLR,    OR_BTLR,  OR_TBRL,  OR_BTRL 
inline const char*  fragment_rotateLRBT(){  return  "vec2 rotate(vec2 coords){ return coords; }"; }
inline const char*  fragment_rotateRLBT(){  return  "vec2 rotate(vec2 coords){ coords.x = 1.0-coords.x; return coords; }"; }
inline const char*  fragment_rotateLRTB(){  return  "vec2 rotate(vec2 coords){ coords.y = 1.0-coords.y; return coords; }"; }
inline const char*  fragment_rotateRLTB(){  return  "vec2 rotate(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords; }"; }
inline const char*  fragment_rotateTBLR(){  return  "vec2 rotate(vec2 coords){ coords.y = 1.0-coords.y; return coords.yx; }"; }
inline const char*  fragment_rotateBTLR(){  return  "vec2 rotate(vec2 coords){ return coords.yx; }"; }
inline const char*  fragment_rotateTBRL(){  return  "vec2 rotate(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords.yx; }"; }
inline const char*  fragment_rotateBTRL(){  return  "vec2 rotate(vec2 coords){ coords.x = 1.0-coords.x; return coords.yx; }"; }

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
void DrawQWidget::applyHardPendings()
{
  if (m_pcsh && havePendOn(PC_INIT))
  {
    glClear(GL_COLOR_BUFFER_BIT);
    
#ifdef BSSHADER_DUMP
    QFile fout("fragshader.txt");
    fout.open(QFile::WriteOnly | QFile::Text);
#endif
    
    if (m_vshalloc == 0)
    {
      m_vshalloc = m_pcsh->shvertex_pendingSize();
      m_vshmem = new char[m_vshalloc];
    }
    unsigned int vsh_written = m_pcsh->shvertex_store(m_vshmem);
    Q_UNUSED(vsh_written);
//    qDebug()<<m_pcsh->shaderName()<<" vertex size "<<vsh_written<<" (had"<<m_vshalloc<<")";
    
    if (m_fshalloc == 0 || m_fshalloc < m_pcsh->shfragment_pendingSize(m_overlaysCount + 2))
    {
      if (m_fshalloc != 0)
        delete []m_fshmem;
      m_fshalloc = m_pcsh->shfragment_pendingSize(m_overlaysCount + 2);
      m_fshmem = new char[m_fshalloc];
    }
    
    {
      ovlfraginfo_t ovlsinfo[OVLLIMIT];
      for (unsigned int i=0; i<m_overlaysCount; i++)
      {
        if (m_overlays[i].olinks.type != msstruct_t::MS_SELF && m_overlays[i].olinks.type != msstruct_t::MS_ROOT)
          ovlsinfo[i].link = m_overlays[i].olinks.details.drivenid;
        else
          ovlsinfo[i].link = -1;
      }
      unsigned int fsh_written = m_pcsh->shfragment_store(m_postMask, m_matrixSwitchAB, m_overlaysCount, ovlsinfo, m_fshmem);
      Q_UNUSED(fsh_written);
//      qDebug()<<m_pcsh->shaderName()<<" fragment size "<<fsh_written<<" (had"<<m_fshalloc<<")";
      
#ifdef BSSHADER_DUMP
      fout.write((const char*)m_fshmem);
      fout.write((const char*)"\n\n\n\n");
#endif
    }
    
    m_ShaderProgram.removeAllShaders();
    if (!m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, m_vshmem))
      qDebug()<<"Vertex failure!";

    
    if (!m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, m_fshmem))
      qDebug()<<"Fragment0 failure!";
    
    {
      const char* pfragment_rotate_enumed[] = { 
        fragment_rotateLRBT(), fragment_rotateRLBT(), fragment_rotateLRTB(), fragment_rotateRLTB(),
        fragment_rotateTBLR(), fragment_rotateBTLR(), fragment_rotateTBRL(), fragment_rotateBTRL()
      };
      m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, pfragment_rotate_enumed[m_orient]);
    }
    
    char  ovlshaderbuf[8192*2];
    for (unsigned int i=0; i<m_overlaysCount; i++)
    {     
      int fshtResult = m_overlays[i].povl->fshTrace(i + 1, ovlshaderbuf);
      if (fshtResult == -1)
        qDebug()<<"OVL fshTrace failure!";
      else if (fshtResult > 0)
        m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, ovlshaderbuf);
      
#ifdef BSSHADER_DUMP
      fout.write((const char*)ovlshaderbuf);
      fout.write((const char*)"\n\n\n\n");
#endif
      
      int fshcResult = m_overlays[i].povl->fshColor(i + 1, ovlshaderbuf);
      if (fshcResult == -1)
        qDebug()<<"OVL fshColor failure!";
      else if (fshcResult > 0)
        m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,  ovlshaderbuf);
          
#ifdef BSSHADER_DUMP
      fout.write((const char*)ovlshaderbuf);
      fout.write((const char*)"\n\n\n\n");
#endif
    }
    
    m_ShaderProgram.bindAttributeLocation("vertices", 0);
    m_ShaderProgram.link();
    
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
        AbstractOverlay::uniforms_t  uf = m_overlays[i].povl->uniforms();
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
    m_bitmaskPendingChanges |= (m_dataDomains == nullptr? 0 : PC_DOMAIN) | PC_SIZE | PC_DATA | PC_PARAMS | PC_PARAMSOVL;
#ifdef BSSHADER_DUMP
    fout.close();
#endif
  }
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
  glGenTextures(1, &m_texAll[HT_DMN]);  /// domain
  applyHardPendings();
  
  m_bitmaskPendingChanges |= PC_SIZE | PC_DATA | PC_PARAMS | PC_PARAMSOVL; 
//  qDebug("GL: initialized. %04x", m_bitmaskPendingChanges);
}

void DrawQWidget::paintGL()
{
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
  glClearColor(m_clearcolor[0], m_clearcolor[1], m_clearcolor[2], 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT); // on applyHardPendings
//    glEnable(GL_BLEND);
  
  applyHardPendings();
  
  m_ShaderProgram.bind();
  {
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
        if (havePendOn(PC_DATA) || ddu == DDU_15D)
        {
          unsigned int total = dataDimmA*dataDimmB*m_countPortions;
          for (unsigned int i=0; i<total; i++)
            m_matrixDataCached[i] = m_matrixData[i] * m_loc_k + m_loc_b;
        }
        glTexImage2D(   GL_TEXTURE_2D, 0, GL_RED, dataDimmA, dataDimmB*m_countPortions, 0, GL_RED, GL_FLOAT, m_matrixDataCached);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_interpPal? GL_LINEAR : GL_NEAREST);  // GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_interpPal? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
      m_ShaderProgram.setUniformValue(loc, HT_MATRIX);
    }
         
    if ((loc = m_locations[SF_PALETTE]) != -1)
    {
      glActiveTexture(GL_TEXTURE0 + HT_PAL);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_PAL]);
      if (m_ppal && havePendOn(PC_PALETTE))
      {
        palettePrepare(m_ppal, m_ppaldiscretise, m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D? m_countPortions : 1);
        if (m_clearbypalette)
          _colorCvt(m_ppal->firstColor());
      }
      m_ShaderProgram.setUniformValue(loc, HT_PAL);
    }
    
    if ((loc = m_locations[SF_DOMAIN]) != -1)
    {
      glActiveTexture(GL_TEXTURE0 + HT_DMN);
      glBindTexture(GL_TEXTURE_2D, m_texAll[HT_DMN]);
      
      if (havePendOn(PC_DOMAIN))
      {
        float* dataDomainsCached=m_dataDomainsFastFree? m_dataDomains : new float[m_matrixDimmA*m_matrixDimmB];
        for (unsigned int i=0; i<m_matrixDimmA*m_matrixDimmB; i++)
          dataDomainsCached[i] = m_dataDomains[i] / (m_portionSize+1);
        
        glTexImage2D(   GL_TEXTURE_2D, 0, 
#if QT_VERSION >= 0x050000
                        GL_R32F, 
#elif QT_VERSION >= 0x040000
                        GL_RED, 
#endif
                        m_matrixDimmA, m_matrixDimmB, 0, GL_RED, GL_FLOAT, dataDomainsCached);
//          glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        delete []dataDomainsCached;
        if (m_dataDomainsFastFree)
          m_dataDomains = nullptr;
      }
      m_ShaderProgram.setUniformValue(loc, HT_DMN);
      
    }
    
    if (havePendOn(PC_DOMAIN))
    {
      if ((loc = m_locations[SF_PORTIONSIZE]) != -1)
      {
        m_ShaderProgram.setUniformValue(loc, m_portionSize);
      }
    }
    
    if (havePendOn(PC_SIZE))
    {
      if ((loc = m_locations[SF_DIMM_A]) != -1)                   m_ShaderProgram.setUniformValue(loc, m_matrixDimmA);
      if ((loc = m_locations[SF_DIMM_B]) != -1)                   m_ShaderProgram.setUniformValue(loc, m_matrixDimmB);
      if ((loc = m_locations[SF_CHNL_HORZSCALING]) != -1)         m_ShaderProgram.setUniformValue(loc, m_matrixSwitchAB? m_scalingHeight : m_scalingWidth);
      if ((loc = m_locations[SF_CHNL_VERTSCALING]) != -1)         m_ShaderProgram.setUniformValue(loc, m_matrixSwitchAB? m_scalingWidth : m_scalingHeight);
    }
    
    if (havePendOn(PC_PARAMS))
    {
      if ((loc = m_locations[SF_COUNTPORTIONS]) != -1)            m_ShaderProgram.setUniformValue(loc, this->m_countPortions);
//        if ((loc = m_locations[SF_BOUNDS]) != -1)                   m_ShaderProgram.setUniformValue(loc, (const QVector2D&)m_bounds);
//        if ((loc = m_locations[SF_CONTRAST]) != -1)                 m_ShaderProgram.setUniformValue(loc, (const QVector2D&)m_contrast);
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
            m_ShaderProgram.setUniformValue(loc, QVector4D(m_overlays[i].povl->getOpacity(), m_overlays[i].povl->getDensity(), m_overlays[i].povl->getSlice(), 0.0f));
          
          for (unsigned int j=0; j<m_overlays[i].uf_count; j++)
          {
            uniform_located_t& ufm = m_overlays[i].uf_arr[j];
            int         loc = ufm.location;
            const void* data = ufm.dataptr;
            switch (ufm.type) {
              case DT_1F: m_ShaderProgram.setUniformValue(loc, *(GLfloat*)data); break;
              case DT_2F: m_ShaderProgram.setUniformValue(loc, *(QVector2D*)data); break;
              case DT_3F: m_ShaderProgram.setUniformValue(loc, *(QVector3D*)data); break;
              case DT_4F: m_ShaderProgram.setUniformValue(loc, *(QVector4D*)data); break;
              case DT_1I: m_ShaderProgram.setUniformValue(loc, *(GLint*)data); break;
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
                  glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(psampler->count), 1, 0, GL_RGBA, GL_FLOAT, psampler->data);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  border = need special color
  //                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  m_ShaderProgram.setUniformValue(loc, ufm.tex_idx);
                }
                break;
              }
              case DT_TEX:  case DT_TEXA: case DT_TEXT:
              {
                if (m_overlays[i].upcount >= 1001)
                {
                  glActiveTexture(GL_TEXTURE0 + ufm.tex_idx);
//                  glBindTexture(GL_TEXTURE_2D, m_textures[ufm.tex_idx]);
                  const dmtype_image_t* pimage = (const dmtype_image_t*)data;
                  
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
//                  GLint tex_internalFormat = GL_RGB8;       GLenum tex_format = GL_RGB;       GLenum texture_type = GL_UNSIGNED_BYTE;
//                  GLint texa_internalFormat = GL_RGBA8;     GLenum texa_format = GL_RGBA;
//                  GLint string_internalFormat = GL_ALPHA8;  GLenum string_format = GL_ALPHA;  GLenum string_type = GL_UNSIGNED_BYTE;
                  if (ufm.type == DT_TEX)       {    gl_internalFormat = GL_RGB;    gl_format = GL_RGB;   }
                  else if (ufm.type == DT_TEXA) {    gl_internalFormat = GL_RGBA8;   gl_format = GL_RGBA;   }
                  else if (ufm.type == DT_TEXT) {    gl_internalFormat = GL_ALPHA8;   gl_format = GL_ALPHA;   }
#elif QT_VERSION >= 0x040000
//                  GLint tex_internalFormat = GL_RGB;        GLenum tex_format = GL_RGB;       GLenum texture_type = GL_UNSIGNED_BYTE;
//                  GLint texa_internalFormat = GL_RGBA;      GLenum texa_format = GL_RGBA;           
//                  GLint string_internalFormat = GL_RGBA;  GLenum string_format = GL_RGBA;  GLenum string_type = GL_UNSIGNED_BYTE;
                  if (ufm.type == DT_TEX)       {    gl_internalFormat = GL_RGB;    gl_format = GL_RGB;   }
                  else if (ufm.type == DT_TEXA) {    gl_internalFormat = GL_RGBA;   gl_format = GL_RGBA;   }
                  else if (ufm.type == DT_TEXT) {    gl_internalFormat = GL_RGBA;   gl_format = GL_RGBA;   }
#endif
                  glTexImage2D(  GL_TEXTURE_2D, 0, gl_internalFormat, pimage->w, pimage->h, 0, gl_format, gl_texture_type, pimage->data);
                  m_ShaderProgram.setUniformValue(loc, ufm.tex_idx);
                }
                break;
              }
              case DT__HC_PALETTE:
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
    
    if (m_matrixSwitchAB)
    {
      glViewport(0 + m_cttrLeft, height() - (m_matrixDimmA*m_scalingHeight) - m_cttrTop, 
                 m_matrixDimmB*m_scalingWidth,
                 m_matrixDimmA*m_scalingHeight
                 );
    }
    else
    {
      glViewport(0 + m_cttrLeft, height() - (m_matrixDimmB*m_scalingHeight) - m_cttrTop, 
                 m_matrixDimmA*m_scalingWidth,
                 m_matrixDimmB*m_scalingHeight
                 );
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_ShaderProgram.disableAttributeArray(0);
    
  }
  unpendAll();
  m_ShaderProgram.release();
  
  for (unsigned int i=0; i<m_texOvlCount; i++)
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawQWidget::innerUpdate(REDRAWBY redrawby)
{
  if (!autoUpdateBanned(redrawby))
  {
    update();
  }
}

void DrawQWidget::innerResize()
{
  this->updateGeometry();
}

QSize DrawQWidget::minimumSizeHint() const
{
  if (m_matrixSwitchAB)
    return QSize( (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB)*m_scalingWidthMin + m_cttrLeft + m_cttrRight,
                  m_matrixDimmA*m_scalingHeightMin + m_cttrTop + m_cttrBottom
                  );
  else
    return QSize( m_matrixDimmA*m_scalingWidthMin + m_cttrLeft + m_cttrRight,
                  (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB)*m_scalingHeightMin + m_cttrTop + m_cttrBottom
                  );
        //            ((getDataDimmUsage() != DDU_2D && getDataDimmUsage() != DDU_DD)? 1 : m_matrixDimmB)*m_scalingHeightMin + m_cttrTop + m_cttrBottom
}

QSize DrawQWidget::sizeHint() const
{ 
  if (m_matrixSwitchAB)
    return QSize( (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB)*m_scalingWidth + m_cttrLeft + m_cttrRight, 
                  m_matrixDimmA*m_scalingHeight + m_cttrTop + m_cttrBottom
                  );
  else
    return QSize( m_matrixDimmA*m_scalingWidth + m_cttrLeft + m_cttrRight, 
                  (getDataDimmUsage() == DDU_1D? 1 : m_matrixDimmB)*m_scalingHeight + m_cttrTop + m_cttrBottom
                  );
        //            ((getDataDimmUsage() != DDU_2D && getDataDimmUsage() != DDU_DD)? 1 : m_matrixDimmB)*m_scalingHeight + m_cttrTop + m_cttrBottom
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  DrawQWidget::store_crd_clk(OVL_REACTION oreact, unsigned int x, unsigned int y)
{
  const unsigned int& dimmWidth = m_matrixSwitchAB? m_matrixDimmB : m_matrixDimmA;
  const unsigned int& dimmHeight = m_matrixSwitchAB? m_matrixDimmA : m_matrixDimmB;
  if (x < m_cttrLeft + dimmWidth*m_scalingWidth && y < m_cttrTop + dimmHeight*m_scalingHeight)
  {
    float dataptr[] = { float(x - m_cttrLeft) / (dimmWidth*m_scalingWidth), 1.0f - float(y - m_cttrTop) / (dimmHeight*m_scalingHeight) };
    bool doStop = false, doUpdate = false;
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
    {
      if (m_overlays[i].povl->overlayReaction(oreact, dataptr, &doStop))
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
      innerUpdate(RD_BYOVL_ACTIONS);
    }
  }
}

#include <QMouseEvent>
#include <QResizeEvent>

void DrawQWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)  store_crd_clk(OR_LMPRESS, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)  store_crd_clk(OR_LMRELEASE, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton)  store_crd_clk(OR_LMMOVE, event->pos().x(), event->pos().y());
//  store_crd_clk(OR_LMMOVE, event->pos().x(), event->pos().y());
//  setMouseTracking(tru!!);
}

void DrawQWidget::resizeEvent(QResizeEvent *event)
{
  int l,t,r,b;
  getContentsMargins(&l,&t,&r,&b);
  if (l != m_cttrLeft || t != m_cttrTop || r != m_cttrRight || b != m_cttrBottom)
  {
    m_cttrLeft = l; m_cttrTop = t; m_cttrRight = r; m_cttrBottom = b;
//    updateGeometry(); // no need? strange...
  }
//  else  // nothing to say here...
  QOpenGLWidget::resizeEvent(event);
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
  QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(slideLmHeight(int)));
}

void DrawQWidget::slot_setBoundLow(float value){  setBoundLow(value); }
void DrawQWidget::slot_setBoundHigh(float value){ setBoundHigh(value);  }
void DrawQWidget::slot_setContrast(float k, float b){ setContrast(contrast_t(k, b));  }
void DrawQWidget::slot_setDataPalette(const IPalette* ppal){ setDataPalette(ppal); }
void DrawQWidget::slot_setData(const float* data){ setData(data); }
void DrawQWidget::slot_setData(QVector<float> data){ setData(data.constData()); }
void DrawQWidget::slot_fillData(float data){ fillData(data); }
void DrawQWidget::slot_clearData(){ clearData(); }

void DrawQWidget::slot_enableAutoUpdate(bool enabled){  banAutoUpdate(!enabled); }
void DrawQWidget::slot_disableAutoUpdate(bool disabled){  banAutoUpdate(disabled); }
void DrawQWidget::slot_enableAutoUpdateByData(bool enabled){  banAutoUpdate(RD_BYDATA, !enabled); }
void DrawQWidget::slot_disableAutoUpdateByData(bool disabled){  banAutoUpdate(RD_BYDATA, disabled); }

////////////////////////////////////////////////////////////////////////
void  DrawQWidget::slideLmHeight(int)
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


class OverlayEmpty: public IOverlay
{
public:
  virtual int  fshTrace(int, char*) const;
  virtual int  fshColor(int, char*) const;
};

///////////////////////////////////////
extern int msprintf(char* to, const char* format, ...);

int DrawCore::OverlayEmpty::fshTrace(int overlay, char* to) const
{
  return msprintf(to, "vec4 overlayTrace%d(in vec2 coords, in float density, in ivec2 mastercoords, out ivec2 selfposition){ return vec4(0.0,0.0,0.0,0.0); }\n", overlay);
}

int DrawCore::OverlayEmpty::fshColor(int overlay, char* to) const
{
  return msprintf(to, "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) { return undercolor; }\n", overlay);
}
