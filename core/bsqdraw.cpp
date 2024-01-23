/// DrawCore does not contain any Qt code. DrawQWidget is an arrangement of DrawCore on Qt
/// Based on QOpenGLWidget, DrawQWidget also rearrange DrawCore methods on Qt slots
/// Qt Events also appears here
/// 
/// If you want operate pure bsdraws interface, use DrawCore
/// But if you want operate a connective (bsdraws+QWidget), use DrawQWidget
/// 
/// Created By: Elijah Vlasov
#include "bsqdraw.h"

#include "../palettes/bspalettes_std.h"
#include "sheigen/bsshgenmain.h"

#if QT_VERSION >= 0x050000
#include <QApplication>
#include <QScreen>
#endif

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#if QT_CONFIG(wheelevent)
#include <QWheelEvent>
#endif

extern int msprintf(char* to, const char* format, ...);

int OvldrawEmpty::fshOVCoords(int overlay, bool /*switchedab*/, char* to) const
{
  return msprintf(to, "vec4 overlayOVCoords%d(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 selfposition){ return vec4(0.0,0.0,0.0,0.0); }\n", overlay);
}

int OvldrawEmpty::fshColor(int overlay, char* to) const
{
  return msprintf(to, "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) { return undercolor; }\n", overlay);
}

////////////////////////////////////////

//#define BSSHADER_DUMP     // write file fragshader.txt when compiling shader

///////////////////////////////////////

DrawQWidget::DrawQWidget(DATAASTEXTURE datex, ISheiGenerator* pcsh, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int emptycolor): 
  DrawCore(datex, portions, orient, splitPortions, emptycolor),
  m_compileOnInitializeGL(true), m_vshalloc(0), m_fshalloc(0), m_pcsh(pcsh), m_locationSecondaryCount(0),
  m_matrixLmSize(0), m_sbStatic(false), 
  m_cttrLeft(0), m_cttrTop(0), m_cttrRight(0), m_cttrBottom(0), c_width(0), c_height(0), 
  m_viewAlignHorz(0.0f), m_viewAlignVert(0.0f), 
  m_viewTurn(0),
  m_texturesCount(0)
{
  m_paletptr = &palette_creature;
  m_paletdiscretise = palette_creature.paletteDiscretion();
  
  for (int i=0; i<_SF_COUNT; i++)
    m_locationPrimary[i] = -1;
  
  for (int i=0; i<TFT_HOLDERS; i++)
    m_holders[i] = nullptr;
  
  m_portionMeshType = splitPortions == SP_NONE? m_pcsh->portionMeshType() : ISheiGenerator::PMT_FORCE1D;  // ntf: strange, non-intuitive
  
  setFocusPolicy(Qt::ClickFocus);
  
  QSurfaceFormat fmt = this->format();
#if QT_VERSION >= 0x050000
  fmt.setSwapInterval(0);
#endif
  fmt.setDepthBufferSize(0);
  fmt.setStencilBufferSize(0);
//  fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  this->setFormat(fmt);
  
#if QT_VERSION >= 0x050000
  c_dpr = (float)QApplication::primaryScreen()->devicePixelRatio();
#else
  c_dpr = 1.0f;
#endif
  c_dpr_inv = 1.0f / c_dpr;
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

void DrawQWidget::compileShaderNow()
{
//  makeCurrent();
//  initCollectAndCompileShader();
//  doneCurrent();
  m_compileOnInitializeGL = false;
}

void DrawQWidget::compileWhenInitializeGL(bool cflag)
{
  m_compileOnInitializeGL = cflag;
}


inline const char*  fastpaced_ovl_settings(char* tmpbuf, unsigned int ovl)
{
  /// EQuals: srintf(_tempvd, "ovl_otss_%d", i + 1);
  static char chset[] = {'0','1','2','3','4','5','6','7','8','9'};
  const char* base = "ovl_otss_";    // opacity, thickness, slice
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

inline const char*  fastpaced_ovl_opm(char* tmpbuf, unsigned int ovl, unsigned int subpos)
{
  /// EQuals: srintf(_tempvd, "opm%d_%d", i + 1, j);
  static char chset[] = {'0','1','2','3','4','5','6','7','8','9'};
  const char* base = "opm";
  char* p = tmpbuf;
  while (*base)  *p++ = *base++;
  static char fhset[] = { '_', '\0' };
  unsigned int ids[] = { ovl, subpos };
  for (int j=0; j<2; j++)
  {
    for (int i=2; i>=0; i--)
    {
      *(p+i) = chset[ids[j] % 10];
      ids[j] /= 10;
    }
    p += 3;
    *p++ = fhset[j];
  }
  return tmpbuf;
}

void DrawQWidget::palettePrepare(const IPalette* ppal, bool discrete, int levels)
{
  const void*   palArr;
  unsigned int  palSize;
  unsigned int  palFormat;
  
  ppal->paletteData(&palArr, &palSize, &palFormat);
  
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
  m_ShaderProgram.removeAllShaders();
     
  /// 1. Vertex shader
  /// mem alloc
  unsigned int vshps = m_pcsh->shvertex_pendingSize();
  if (m_vshalloc < vshps)
  {
    if (m_vshalloc) delete []m_vshmem;
    m_vshalloc = vshps;
    m_vshmem = new char[m_vshalloc];
  }
  
  /// store
  unsigned int vsh_written = m_pcsh->shvertex_store(m_vshmem);
  Q_ASSERT(vsh_written <= m_vshalloc);
//  Q_UNUSED(vsh_written);
//    qDebug()<<m_pcsh->shaderName()<<" vertex size "<<vsh_written<<" (had"<<m_vshalloc<<")";
  if (!m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, m_vshmem))
  {
    qDebug()<<Q_FUNC_INFO<<"... vertex shader failure!"<<m_ShaderProgram.log();
    return;
  }
  
  
  /// 2. Fragment shader
  /// mem alloc
  unsigned int fshps = m_pcsh->shfragment_pendingSize();
  fshps +=  FshDrawMain::basePendingSize(m_datasubmesh, m_overlaysCount);
  for (unsigned int t=0; t<TFT_HOLDERS; t++)
    if (m_holders[t] != nullptr)
      fshps += m_holders[t]->tftslots.size()*512;
        
  if (m_fshalloc < fshps)
  {
    if (m_fshalloc)  delete []m_fshmem;
    m_fshalloc = fshps;
    m_fshmem = new char[m_fshalloc];
  }
  
  
  /// store
  FshDrawMain fdm(m_fshmem, m_splitPortions, m_datasubmesh);
  fdm.generic_decls_begin(m_overlaysCount);
  unsigned int    additufm_count=0;
  shuniformdesc_t additufm_desc[96];
  {
    additufm_count = m_pcsh->shfragment_uniforms(additufm_desc, sizeof(additufm_desc)/sizeof(shuniformdesc_t));
    for (unsigned int i=0; i<additufm_count; i++)
      fdm.generic_decls_add(additufm_desc[i].type, additufm_desc[i].varname);
  }
  {
    for (unsigned int t=0; t<TFT_HOLDERS; t++)
    {
      if (m_holders[t] == nullptr)  continue;
      m_holders[t]->_location = -1;
      fdm.generic_decls_add_tftareas(t, m_holders[t]->_varname);
      
      for (unsigned int i=0; i<m_holders[t]->tftslots.size(); i++)
      {
        m_holders[t]->tftslots[i]._location_i = -1;
        m_holders[t]->tftslots[i]._location_c = -1;
        if (m_holders[t]->tftslots[i].isstatic)
        {
          m_holders[t]->tftslots[i]._varname_i[0] = '\0';
          m_holders[t]->tftslots[i]._varname_c[0] = '\0';
        }
        else
          fdm.generic_decls_add_tftdynamicvar(t, i, m_holders[t]->tftslots[i]._varname_i, m_holders[t]->tftslots[i]._varname_c);
      }
    }
  }
  
  fdm.generic_main_begin(m_allocatedPortions, m_orient, m_emptycolor, m_overpattern);
  
  m_pcsh->shfragment_store(fdm);
  
  {
    fdm.generic_main_process_fsp(m_overpattern, m_overpatternOpacity);
  }
  {
    fdm.generic_main_prepare_tft();
    for (unsigned int t=0; t<TFT_HOLDERS; t++)
    {
      if (m_holders[t] == nullptr)
        continue;
      for (unsigned int i=0; i<m_holders[t]->tftslots.size(); i++)
      {
        int recid_short = m_holders[t]->tftslots[i].recid % m_holders[t]->recordslimit;
#ifndef BSGLSLOLD
        int areaid =  m_holders[t]->tftslots[i].recid / m_holders[t]->recordslimit;
        tftfraginfo_t tfi = {  int(t), int(m_holders[t]->tftarea.size()),
                               m_holders[t]->recordslimit, m_holders[t]->record_width, m_holders[t]->record_height, m_holders[t]->tftarea[areaid].records[recid_short].width, 
                                        m_holders[t]->tftslots[i].recid,
                               int(i), m_holders[t]->tftslots[i].isstatic, m_holders[t]->tftslots[i].slotinfo
        };
#else
        tftfraginfo_t tfi = {  int(t), 1,
                               m_holders[t]->recordslimit, m_holders[t]->record_width, m_holders[t]->record_height, m_holders[t]->tftarea.records[recid_short].width, 
                                        m_holders[t]->tftslots[i].recid,
                               int(i), m_holders[t]->tftslots[i].isstatic, m_holders[t]->tftslots[i].slotinfo
        };
#endif
        fdm.generic_main_process_tft(tfi);
      }
    }
  }
  {
    ovlfraginfo_t ovlsinfo[OVLLIMIT];
    for (unsigned int i=0; i<m_overlaysCount; i++)
    {
      if (m_overlays[i].olinks.type != msstruct_t::MS_SELF && m_overlays[i].olinks.type != msstruct_t::MS_ROOT)
        ovlsinfo[i].link = m_overlays[i].olinks.details.drivenid;
      else
        ovlsinfo[i].link = -1;
      
      ovlsinfo[i].orient = m_overlays[i].orient;
    }
    fdm.generic_main_process_overlays(m_orient, m_overlaysCount, ovlsinfo);
  }
//    qDebug()<<m_pcsh->shaderName()<<" fragment size "<<fsh_written<<" (had"<<m_fshalloc<<")";
  Q_ASSERT(fdm.written() <= m_fshalloc);
  
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

    //////////////////////////////
//    qDebug()<<m_fshmem;
    //////////////////////////////
  
  char  ovlshaderbuf[8192*2];
  for (unsigned int i=0; i<m_overlaysCount; i++)
  {     
    int fshtResult = m_overlays[i].povl->fshOVCoords(i + 1, m_dataDimmSwitchAB, ovlshaderbuf);
    if (fshtResult <= 0)
      qDebug()<<Q_FUNC_INFO<<"OVL fshOVCoords failure!";
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
  
  m_texturesCount = 0;
  
  if (m_ShaderProgram.link())
  {
    static const char* vd_corresponding_array[] = 
    { 
      "datasampler", "datadimm_a", "datadimm_b", "dataportions", "dataportionsize", "datarange", 
      "scaler_a", "scaler_b", 
      "paletsampler", "paletrange"
    };
    
    Q_ASSERT(_SF_COUNT == sizeof(vd_corresponding_array)/sizeof(const char*));
    for (int i=0; i<_SF_COUNT; i++)
    {
      m_locationPrimary[i] = m_ShaderProgram.uniformLocation(vd_corresponding_array[i]);
//      qDebug()<<i<<"????? "<<m_locationPrimary[i];
    }
    
    if (m_locationPrimary[SF_DATASAMPLER] != -1)
      glGenTextures(1, &m_textures[m_texturesCount++]);
    if (m_locationPrimary[SF_PALETSAMPLER] != -1)
      glGenTextures(1, &m_textures[m_texturesCount++]);
    
    /// 2. Init additional locations and textures
    Q_ASSERT(additufm_count <= MAX_OPTIONALS);
    for (unsigned int i=0; i<additufm_count; i++)
    {
      m_locationSecondary[i].location = m_ShaderProgram.uniformLocation(additufm_desc[i].varname);
      m_locationSecondary[i].istexture = additufm_desc[i].type >= _DT_TEXTURES_BEGIN;
      //qDebug()<<additufm_desc[i].varname<<additufm_desc[i].istexture<<m_locationSecondary[i].location<<m_texturesCount;
      if (m_locationSecondary[i].istexture)
        glGenTextures(1, &m_textures[m_texturesCount++]);
    }
    m_locationSecondaryCount = additufm_count;
    
    
    /// 3. Init tft locations and textures
    {
      for (unsigned int t=0; t<TFT_HOLDERS; t++)
      {
        if (m_holders[t] == nullptr)  continue;
        
        glGenTextures(1, &m_textures[m_texturesCount++]);
        m_holders[t]->_location = m_ShaderProgram.uniformLocation(m_holders[t]->_varname);
        m_holders[t]->ponger = 0;
        
        for (unsigned int i=0; i<m_holders[t]->tftslots.size(); i++)
        {
          if (m_holders[t]->tftslots[i]._varname_i[0] != '\0')
            m_holders[t]->tftslots[i]._location_i = m_ShaderProgram.uniformLocation(m_holders[t]->tftslots[i]._varname_i);
          else
            m_holders[t]->tftslots[i]._location_i = -1;
          if (m_holders[t]->tftslots[i]._varname_c[0] != '\0')
            m_holders[t]->tftslots[i]._location_c = m_ShaderProgram.uniformLocation(m_holders[t]->tftslots[i]._varname_c);
          else
            m_holders[t]->tftslots[i]._location_c = -1;
          m_holders[t]->tftslots[i].ponger = 0;
        }
      }
    }
    
    /// 4. Init ovl locations and textures
    {
      char _tempvd[64];
      for (unsigned int i=0; i<m_overlaysCount; i++)
      {
        fastpaced_ovl_settings(_tempvd, i+1);
        m_overlays[i].outloc = m_ShaderProgram.uniformLocation(_tempvd);                  /// ! cannot hide by upcount
        m_overlays[i].texcount = 0;
        _Ovldraw::uniforms_t  uf = m_overlays[i].povl->uniforms();
        for (unsigned int j=0; j<uf.count; j++)
        {
          fastpaced_ovl_opm(_tempvd, i+1, j);
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
            m_overlays[i].texcount += 1;
            glGenTextures(1, &m_textures[m_texturesCount++]);
          }
        }
        m_overlays[i].ponger_reinit = m_overlays[i].ponger_update = 0;
      }
    } // init ovl
    
    unpend(PC_INIT);
    m_bitmaskPendingChanges |= PC_DATA | PC_DATADIMMS | PC_DATAPORTS | PC_DATARANGE | PC_PALETTE | PC_PALETTEPARAMS | PC_SEC | 
                              PC_PARAMSOVL | 
                              PC_TFT_TEXTURE | PC_TFT_PARAMS;
    
  } /// link
  
  
  if (!m_ShaderProgram.bind())
  {
    qDebug()<<Q_FUNC_INFO<<"... bind failure!";
  }
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

  if (m_clearsource == CS_WIDGET)
  {
    QColor clr_widget(palette().window().color());
    m_clearcolor[0] = clr_widget.redF();
    m_clearcolor[1] = clr_widget.greenF();
    m_clearcolor[2] = clr_widget.blueF();
  }
  glClearColor(m_clearcolor[0], m_clearcolor[1], m_clearcolor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  m_doclearbackground = false;
  
  if (m_compileOnInitializeGL)
    initCollectAndCompileShader();
  
  m_bitmaskPendingChanges |= PC_DATA | PC_DATADIMMS | PC_DATAPORTS | PC_DATARANGE | PC_PALETTE | PC_PALETTEPARAMS | PC_SEC | 
                              PC_PARAMSOVL |
                              PC_TFT_TEXTURE | PC_TFT_PARAMS; 
  //qDebug()<<"InitGL: "<<m_compileOnInitializeGL<<QString::number(m_bitmaskPendingChanges, 2);
  
  glDisable(GL_DEPTH_TEST);
  {
    glDisable(GL_BLEND);
//    glDisable(GL_COLOR_MATERIAL);
//    glDisable(GL_LINE_SMOOTH);
//    glDisable(GL_MINMAX);
//    glDisable(GL_POINT_SPRITE);
    glDisable(GL_STENCIL_TEST);
//    glDisable(GL_SCISSOR_TEST);
//    glDisable(GL_NORMALIZE);
//    glDisable();
    glDisable(GL_DITHER);
#ifdef GL_MULTISAMPLE
    glDisable(GL_MULTISAMPLE);
#endif
  }
}

void DrawQWidget::paintGL()
{ 
//  qDebug("blublublu %04x", m_bitmaskPendingChanges);

//  glDisable(GL_DEPTH_TEST);
//  {
//    glDisable(GL_BLEND);
////    glDisable(GL_COLOR_MATERIAL);
////    glDisable(GL_LINE_SMOOTH);
////    glDisable(GL_MINMAX);
////    glDisable(GL_POINT_SPRITE);
//    glDisable(GL_STENCIL_TEST);
////    glDisable(GL_SCISSOR_TEST);
////    glDisable(GL_NORMALIZE);
////    glDisable();
//    glDisable(GL_DITHER);
//#ifdef GL_MULTISAMPLE
//    glDisable(GL_MULTISAMPLE);
//#endif
//  }
  {
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//    glEnable(GL_TEXTURE_2D);
//    glDisable(GL_BLEND);
//    glDisable(GL_ALPHA_TEST);
  }
//    glEnable(GL_BLEND);
  
//  qDebug()<<"PaintGL: "<<m_compileOnInitializeGL<<QString::number(m_bitmaskPendingChanges, 2);
  bool forceSubPendOn = false;
  if (havePendOn(PC_INIT, m_bitmaskPendingChanges))
  {
    initCollectAndCompileShader();
    forceSubPendOn = true;
  }
  int bitmaskPendingChanges = m_bitmaskPendingChanges;
  unpendAll();
  
  if (m_doclearbackground || !havePending(bitmaskPendingChanges))
  {
    glClearColor(m_clearcolor[0], m_clearcolor[1], m_clearcolor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); // on initCollectAndCompileShader
    m_doclearbackground = false;
//    qDebug()<<"clearing clr"<<m_clearcolor[0]<<m_clearcolor[1]<<m_clearcolor[2];
  }
  
  int loc;
  unsigned int CORR_TEX = 0;
  
  if ((loc = m_locationPrimary[SF_DATASAMPLER]) != -1)
  {      
    glActiveTexture(GL_TEXTURE0 + CORR_TEX);
    glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
    if (havePendOn(PC_DATA, bitmaskPendingChanges) || havePendOn(PC_DATADIMMS, bitmaskPendingChanges))
    {
      
      GLsizei textureLengthA = m_dataDimmA;
      GLsizei textureLengthB = m_dataDimmB;
      if (m_datex == DATEX_1D || m_datex == DATEX_DD)
      {
        textureLengthB = 1;
        textureLengthA = m_portionSize;
      }
      else if (m_datex == DATEX_POLAR)
      {
        textureLengthB = textureLengthB / 2;
        textureLengthA = m_portionSize / textureLengthB;
      }
      if (havePendOn(PC_DATA, bitmaskPendingChanges) || m_datex == DATEX_15D)
      {
        unsigned int total = textureLengthA*textureLengthB*m_countPortions;
        for (unsigned int i=0; i<total; i++)
          m_dataStorageCached[i] = m_dataStorage[i] * m_loc_k + m_loc_b;
      }
      glTexImage2D(   GL_TEXTURE_2D, 0, 
#if QT_VERSION >= 0x050000
                      GL_R32F, 
#elif QT_VERSION >= 0x040000
                      GL_RED, 
#endif
                      textureLengthA, textureLengthB*m_countPortions, 0, GL_RED, GL_FLOAT, m_dataStorageCached);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_dataTextureInterp? GL_LINEAR : GL_NEAREST);  // GL_LINEAR
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_dataTextureInterp? GL_LINEAR : GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      
      m_ShaderProgram.setUniformValue(loc, CORR_TEX);
    }
    CORR_TEX++;
  }
  
  if (havePendOn(PC_DATADIMMS, bitmaskPendingChanges)/* || m_datex == DATEX_15D*/)
  {
    if ((loc = m_locationPrimary[SF_DATADIMM_A]) != -1)        m_ShaderProgram.setUniformValue(loc, m_dataDimmA);
    if ((loc = m_locationPrimary[SF_DATADIMM_B]) != -1)        m_ShaderProgram.setUniformValue(loc, m_dataDimmB);
    if ((loc = m_locationPrimary[SF_SCALER_A]) != -1)          m_ShaderProgram.setUniformValue(loc, m_scalingA);
    if ((loc = m_locationPrimary[SF_SCALER_B]) != -1)          m_ShaderProgram.setUniformValue(loc, m_scalingB);
  }
  if (havePendOn(PC_DATAPORTS, bitmaskPendingChanges))
  {
    if ((loc = m_locationPrimary[SF_DATAPORTIONS]) != -1)      m_ShaderProgram.setUniformValue(loc, m_countPortions);
    if ((loc = m_locationPrimary[SF_DATAPORTIONSIZE]) != -1)   m_ShaderProgram.setUniformValue(loc, m_portionSize);
  }
  if (havePendOn(PC_DATARANGE, bitmaskPendingChanges))
  {
    if ((loc = m_locationPrimary[SF_DATARANGE]) != -1)         m_ShaderProgram.setUniformValue(loc, (const QVector2D&)m_bounds);
  }
  
  if ((loc = m_locationPrimary[SF_PALETSAMPLER]) != -1)
  {
    glActiveTexture(GL_TEXTURE0 + CORR_TEX);
    glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
    if (m_paletptr && havePendOn(PC_PALETTE, bitmaskPendingChanges))
    {
//        palettePrepare(m_paletptr, m_paletdiscretise, m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D && m_countPortions != 0? m_countPortions : 1);
      palettePrepare(m_paletptr, m_paletdiscretise, m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D && m_allocatedPortions != 0? m_allocatedPortions : 1);
      if (m_clearsource == CS_PALETTE)
        _colorCvt(this->colorBack());
      m_ShaderProgram.setUniformValue(loc, CORR_TEX);
    }
    CORR_TEX++;
    
    if (havePendOn(PC_PALETTEPARAMS, bitmaskPendingChanges))
    {
      if ((loc = m_locationPrimary[SF_PALETRANGE]) != -1)     m_ShaderProgram.setUniformValue(loc, *(const QVector2D*)this->m_paletrange);
    }
  }
  
  for (unsigned int i=0; i<m_locationSecondaryCount; i++)
  {
    if (m_locationSecondary[i].istexture)
    {
      glActiveTexture(GL_TEXTURE0 + CORR_TEX);
      glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
//      qDebug()<<m_pcsh->shaderName()<<QString::number(bitmaskPendingChanges,16)<<havePendOn(PC_SEC, bitmaskPendingChanges)<<QString::number(secPendingFlags(bitmaskPendingChanges), 16);
      if (havePendOn(PC_SEC, bitmaskPendingChanges))
        processGlLocation(i, secPendingFlags(bitmaskPendingChanges), m_locationSecondary[i].location, CORR_TEX);
      CORR_TEX++;
    }
    else
    {
      if (havePendOn(PC_SEC, bitmaskPendingChanges))
        processGlLocation(i, secPendingFlags(bitmaskPendingChanges), m_locationSecondary[i].location, -1);
    }
  }
  
  for (unsigned int t=0; t<TFT_HOLDERS; t++)
  {
    if (m_holders[t] == nullptr)
      continue;
    
    if ((loc = m_holders[t]->_location) != -1)
    {
#ifndef BSGLSLOLD
      glActiveTexture(GL_TEXTURE0 + CORR_TEX);
      glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
#else
      glActiveTexture(GL_TEXTURE0 + CORR_TEX);
      glBindTexture(GL_TEXTURE, m_textures[CORR_TEX]);
#endif
    
      if (havePendOn(PC_TFT_TEXTURE, bitmaskPendingChanges))
      {
        const GLint   gl_internalFormat = GL_RGBA8;
        const GLenum  gl_format = GL_RGBA;
        const GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
        int width = m_holders[t]->record_width;
        int height = m_holders[t]->record_height * m_holders[t]->recordslimit;
//        qDebug()<<m_holders[t]->ponger<<m_holders[t]->pinger<<m_holders[t]->_location;
#ifndef BSGLSLOLD
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
        bool  subPendOn = forceSubPendOn || (m_holders[t]->ponger != m_holders[t]->pinger);
        if (subPendOn)
        {
          int LAY = m_holders[t]->tftarea.size();
          glTexStorage3D(  GL_TEXTURE_2D_ARRAY, LAY, gl_internalFormat, width, height, LAY);
          for (int r=0; r<LAY; r++)
          {
            const TFTarea&  area = m_holders[t]->tftarea[r];
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, r, width, height, 1, gl_format, gl_texture_type, area.img->constBits());
          }
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
  #if QT_VERSION >= 0x050000
          glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
          glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
  #endif
          glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
          
          m_holders[t]->ponger = m_holders[t]->pinger;
        }
#else
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
        bool  subPendOn = forceSubPendOn || (m_holders[t]->ponger != m_holders[t]->pinger);
        if (subPendOn)
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
          
  #if QT_VERSION >= 0x050000
          glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
          glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
  #endif
          const TFTarea&  area = m_holders[t]->tftarea;
          qDebug()<<"IMAGE ASSIGNED!"<<width<<height;
          glTexImage2D(GL_TEXTURE_2D, 0, gl_internalFormat, width, height, 0, gl_format, gl_texture_type, area.img->constBits());
          
          m_holders[t]->ponger = m_holders[t]->pinger;
        }
#endif
        m_ShaderProgram.setUniformValue(loc, CORR_TEX);
      } // if loc
    } // if havePend
    CORR_TEX++;
  }
  
  if (havePendOn(PC_TFT_PARAMS, bitmaskPendingChanges))
  {
    for (unsigned int t=0; t<TFT_HOLDERS; t++)
    {
      if (m_holders[t] == nullptr)
        continue;
      for (unsigned int i=0; i<m_holders[t]->tftslots.size(); i++)
      {
        bool  subPendOn = forceSubPendOn || (m_holders[t]->tftslots[i].ponger != m_holders[t]->tftslots[i].pinger);
        if (subPendOn)
        {
          if ((loc = m_holders[t]->tftslots[i]._location_i) != -1)
            m_ShaderProgram.setUniformValue(loc, GLint(m_holders[t]->tftslots[i].recid));
          
          if ((loc = m_holders[t]->tftslots[i]._location_c) != -1)
          {
            float arr[] = { m_holders[t]->tftslots[i].slotinfo.fx, m_holders[t]->tftslots[i].slotinfo.fy, 
                            m_holders[t]->tftslots[i].slotinfo.scale, m_holders[t]->tftslots[i].slotinfo.rotate };
            m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)arr);
          }
          m_holders[t]->tftslots[i].ponger = m_holders[t]->tftslots[i].pinger;
        }
      }
    } // for
  }
  
  
  if (!havePendOn(PC_PARAMSOVL, bitmaskPendingChanges))
  {
    for (unsigned int o=0; o<m_overlaysCount; o++)
    {
      for (unsigned int i=0; i<m_overlays[o].texcount; i++)
      {
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
        CORR_TEX++;
      }
    }
  }
  else
  {
    for (unsigned int o=0; o<m_overlaysCount; o++)
    {
      overlay_t& ovl = m_overlays[o];
      
      bool  subPendOn = forceSubPendOn || (ovl.ponger_reinit < ovl.povl->pingerReinit()) || (ovl.ponger_update < ovl.povl->pingerUpdate());
      if (subPendOn)
      {
        
        if ((loc = ovl.outloc) != -1)
          m_ShaderProgram.setUniformValue(loc, QVector4D(ovl.povl->isVisible()? ovl.povl->getOpacity() : 1.0f, 
                                                         ovl.povl->getThickness(), 
                                                         ovl.povl->getSliceLL(), ovl.povl->getSliceHL()));
        
        for (unsigned int i=0; i<ovl.uf_count; i++)
        {
          const uniform_located_t& ufm = ovl.uf_arr[i];
          loc = ufm.location;
          switch (ufm.type) {
            case DT_1F: m_ShaderProgram.setUniformValue(loc, *(const GLfloat*)ufm.dataptr); break;
            case DT_2F: m_ShaderProgram.setUniformValue(loc, *(const QVector2D*)ufm.dataptr); break;
            case DT_3F: m_ShaderProgram.setUniformValue(loc, *(const QVector3D*)ufm.dataptr); break;
            case DT_4F: m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)ufm.dataptr); break;
            case DT_1I: m_ShaderProgram.setUniformValue(loc, *(const GLint*)ufm.dataptr); break;
//            case DT_2I: m_ShaderProgram.setUniformValue(loc, *(const QPoint*)ufm.dataptr); break;
  //              case DT_3I: m_ShaderProgram.setUniformValue(loc, *(QSize*)ufm.dataptr); break;
  //                case DT_2I: m_ShaderProgram.setUniformValue(loc, *(QSize*)ufm.dataptr); break;
            case DT_ARR: case DT_ARR2: case DT_ARR3: case DT_ARR4:
            case DT_ARRI: case DT_ARRI2: case DT_ARRI3: case DT_ARRI4:
            {
              const dmtype_arr_t* parr = (const dmtype_arr_t*)ufm.dataptr;
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
            case DT_SAMP4: case DT_2D3F: case DT_TEXTURE: case DT_PALETTE:
            {
              glActiveTexture(GL_TEXTURE0 + CORR_TEX);
              glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
              
              if (ufm.type == DT_SAMP4)
              {
                const dmtype_sampler_t* psampler = (const dmtype_sampler_t*)ufm.dataptr;
                glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(psampler->count), 1, 0, GL_RGBA, GL_FLOAT, psampler->data);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
              }
              else if (ufm.type == DT_2D3F)
              {
                const dmtype_2d_t* pimage = (const dmtype_2d_t*)ufm.dataptr;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pimage->linsmooth ? GL_LINEAR : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pimage->linsmooth ? GL_LINEAR : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
                glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
                
                glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
                glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
                glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
                
                GLint   gl_internalFormat = GL_RGB32F;
                GLenum  gl_format = GL_RGB;
                GLenum  gl_texture_type = GL_FLOAT;
                glTexImage2D(  GL_TEXTURE_2D, 0, gl_internalFormat, pimage->w, pimage->len, 0, gl_format, gl_texture_type, pimage->data);
              }
              else if (ufm.type == DT_TEXTURE)
              {
                const dmtype_image_t* pimage = (const dmtype_image_t*)ufm.dataptr;
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
                } //if
              }
              else if (ufm.type == DT_PALETTE)
              {
                const dmtype_palette_t* cdp = (const dmtype_palette_t*)ufm.dataptr;
                palettePrepare(cdp->ppal, cdp->discrete, 1);
              }
              
              m_ShaderProgram.setUniformValue(loc, CORR_TEX);
              CORR_TEX++;
              break;
            }
            default:
            qDebug()<<"BSDraw failure: unknown parameter type!";
            break;
          }
        } // for ov_ufs
        ovl.ponger_reinit = ovl.povl->pingerReinit();
        ovl.ponger_update = ovl.povl->pingerUpdate();
      } // if upcount
      else
      {
        for (unsigned int i=0; i<ovl.texcount; i++)
        {
          glActiveTexture(GL_TEXTURE0 + CORR_TEX);
          glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
          CORR_TEX++;
        }
      }
    } // for overlays
  } // else
  

  m_ShaderProgram.enableAttributeArray(0);
  m_ShaderProgram.setAttributeArray(0, GL_FLOAT, m_SurfaceVertex, 2);
  
  if (!m_rawResizeModeNoScaled)
  {
    dcgeometry_t dch = this->geometryHorz();
    dcgeometry_t dcv = this->geometryVert();
    glViewport(dch.cttr_pre + dch.viewalign_pre, c_height - (dcv.cttr_pre + dcv.viewalign_pre + dcv.length), dch.length, dcv.length);
  }
  
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_ShaderProgram.disableAttributeArray(0);
  
//    unpendAll();
  for (unsigned int i=0; i<m_texturesCount; i++)
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawQWidget::resizeGL(int w, int h)
{
  c_width = w = /*qRound*/(w * c_dpr);
  c_height = h = /*qRound*/(h * c_dpr);
  
#if 1
  QMargins ms(contentsMargins());
  if (ms.left() != m_cttrLeft || ms.top() != m_cttrTop || ms.right() != m_cttrRight || ms.bottom() != m_cttrBottom)
  {
    m_cttrLeft = ms.left();  m_cttrRight = ms.right();  m_cttrTop = ms.top();  m_cttrBottom = ms.bottom();
    updateGeometry();
    return;   /// ?????????
  }
#elif 0
  int ml, mt, mr, mb;
  getContentsMargins(&ml, &mt, &mr, &mb);
  if (ml != m_cttrLeft || mt != m_cttrTop || mr != m_cttrRight || mb != m_cttrBottom)
  {
    m_cttrLeft = ml;  m_cttrRight = mr;  m_cttrTop = mt;  m_cttrBottom = mb;
    updateGeometry();
    return;   /// ?????????
  }
#else
  getContentsMargins(&m_cttrLeft, &m_cttrTop, &m_cttrRight, &m_cttrBottom);
#endif
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  
//  qDebug()<<"DrawQWidget PRE resizeEv: "<<QSize(w,h)<<m_dataDimmA<<m_dataDimmB<<m_scalingA<<m_scalingB;
  if (m_dataDimmSwitchAB)
    adjustSizeAndScale(h, w);
  else
    adjustSizeAndScale(w, h);
//  qDebug()<<"DrawQWidget resizeEv: "<<m_dataDimmA<<m_dataDimmB<<m_scalingA<<m_scalingB;
  
  if (m_rawResizeModeNoScaled)
  {
    dcgeometry_t dch = this->geometryHorz();
    dcgeometry_t dcv = this->geometryVert();
    glViewport( dch.cttr_pre + dch.viewalign_pre, c_height - (dcv.cttr_pre + dcv.viewalign_pre + dcv.length), dch.length, dcv.length);
    
//    if (m_dataDimmSwitchAB)
//      glViewport(0 + m_cttrLeft + m_viewAlignHorz*(c_width - m_cttrLeft - m_cttrRight - (int)sizeB()), 
//                              c_height - (int)sizeA() - m_cttrTop, 
//                              (int)sizeB(), (int)sizeA());
//    else
//      glViewport(0 + m_cttrLeft + m_viewAlignHorz*(c_width - m_cttrLeft - m_cttrRight - (int)sizeA()), 
//                              c_height - (int)sizeB() - m_cttrTop, 
//                              (int)sizeA(), (int)sizeB());
  }
  m_doclearbackground = true;
  pendResize(false);
}


//void DrawQWidget::fitSize(int width_in, int height_in, int* actualwidth, int* actualheight) const
//{
//  width_in -= m_cttrLeft + m_cttrRight;
//  height_in -= m_cttrTop + m_cttrBottom;
  
//  int wsizeA = m_dataDimmSwitchAB? height_in : width_in;
//  int wsizeB = m_dataDimmSwitchAB? width_in : height_in;
  
//  unsigned int dimmA, dimmB, scalingA, scalingB;
//  sizeAndScaleHint(wsizeA/m_splitterA, wsizeB/m_splitterB, &dimmA, &dimmB, &scalingA, &scalingB);
//  if (m_dataDimmSwitchAB)
//  {
//    *actualwidth = dimmB*scalingB*m_splitterB;
//    *actualheight = dimmA*scalingA*m_splitterA;
//  }
//  else
//  {
//    *actualwidth = dimmA*scalingA*m_splitterA;
//    *actualheight = dimmB*scalingB*m_splitterB;
//  }
////  unsigned int dimmA, dimmB, scalingA, scalingB;
////  if (m_dataDimmSwitchAB)
////    sizeAndScaleHint(height_in, width_in, &dimmB, &dimmA, &scalingB, &scalingA);
////  else
////    sizeAndScaleHint(width_in, height_in, &dimmA, &dimmB, &scalingA, &scalingB);
////  *actualwidth = dimmA*scalingA;
////  *actualheight = dimmB*scalingB;
//}

void DrawQWidget::fitSize(int width_in, int height_in, dcsizecd_t* sz_horz, dcsizecd_t* sz_vert, dcgeometry_t* gm_horz, dcgeometry_t* gm_vert) const
{
  width_in *= c_dpr;
  height_in *= c_dpr;
  
  width_in -= m_cttrLeft + m_cttrRight;
  height_in -= m_cttrTop + m_cttrBottom;
  
  int wsizeA = m_dataDimmSwitchAB? height_in : width_in;
  int wsizeB = m_dataDimmSwitchAB? width_in : height_in;
 
  dcsizecd_t  dcA, dcB;
  sizeAndScaleHint(wsizeA/m_splitterA, wsizeB/m_splitterB, &dcA.dimm, &dcB.dimm, &dcA.scaling, &dcB.scaling);
  dcA.splitter = m_splitterA;
  dcB.splitter = m_splitterB;
  
  if (m_dataDimmSwitchAB)
  {
    if (sz_horz) *sz_horz = dcB;
    if (sz_vert) *sz_vert = dcA;
    if (gm_horz)
    {
      int rest = width_in - (int)length(dcB);
      if (rest <= 0)  *gm_horz = { m_cttrLeft, 0, int(length(dcB)), 0, m_cttrRight }; 
      else{ int rh = int(rest*m_viewAlignHorz); *gm_horz = { m_cttrLeft, rh, int(length(dcB)), rest - rh, m_cttrRight };  }
    }
    if (gm_vert)
    {
      int rest = height_in - (int)length(dcA);
      if (rest <= 0) *gm_vert = { m_cttrTop, 0, int(length(dcA)), 0, m_cttrBottom };
      else{ int rv = int(rest*m_viewAlignVert); *gm_vert = { m_cttrTop, rv, int(length(dcA)), rest - rv, m_cttrBottom };  }
    }
  }
  else
  {
    if (sz_horz) *sz_horz = dcA;
    if (sz_vert) *sz_vert = dcB;
    if (gm_horz)
    {
      int rest = width_in - (int)length(dcA);
      if (rest <= 0)  *gm_horz = { m_cttrLeft, 0, int(length(dcA)), 0, m_cttrRight }; 
      else{ int rh = int(rest*m_viewAlignHorz); *gm_horz = { m_cttrLeft, rh, int(length(dcA)), rest - rh, m_cttrRight };  }
    }
    if (gm_vert)
    {
      int rest = height_in - (int)length(dcB);
      if (rest <= 0) *gm_vert = { m_cttrTop, 0, int(length(dcB)), 0, m_cttrBottom };
      else{ int rv = int(rest*m_viewAlignVert); *gm_vert = { m_cttrTop, rv, int(length(dcB)), rest - rv, m_cttrBottom };  }
    }
  }
}

void DrawQWidget::slot_compileShader()
{
  compileShaderNow();
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
  int w = c_width, h = c_height;
  {
    QMargins ms(contentsMargins());
    m_cttrLeft = ms.left();
    m_cttrTop = ms.top();
    m_cttrRight = ms.right();
    m_cttrBottom = ms.bottom();
  }
//  getContentsMargins(&m_cttrLeft, &m_cttrTop, &m_cttrRight, &m_cttrBottom);
  w -= m_cttrLeft + m_cttrRight;
  h -= m_cttrTop + m_cttrBottom;
  
  if (m_dataDimmSwitchAB)
    adjustSizeAndScale(h, w);
  else
    adjustSizeAndScale(w, h);
}

void DrawQWidget::innerUpdateGeometry()
{
//  int sizeA = m_scalingA * m_dataDimmA * m_splitterA * c_dpr_inv;       if (sizeA < 1)  sizeA = 1;
//  int sizeB = m_scalingB * (m_datex == DATEX_1D? 1 : m_dataDimmB) * m_splitterB * c_dpr_inv;      if (sizeB < 1)  sizeB = 1;
//  this->resize(sizeA, sizeB);
  this->updateGeometry();
}

QSize DrawQWidget::minimumSizeHint() const
{
  int sizeA = m_scalingAMin * m_dataDimmA * m_splitterA * c_dpr_inv;    if (sizeA < 1)  sizeA = 1;
  int sizeB = m_scalingBMin * (m_datex == DATEX_1D? 1 : m_dataDimmB) * m_splitterB * c_dpr_inv;   if (sizeB < 1)  sizeB = 1;
  return m_dataDimmSwitchAB ? QSize( sizeB + m_cttrLeft + m_cttrRight, sizeA + m_cttrTop + m_cttrBottom ) : QSize( sizeA + m_cttrLeft + m_cttrRight, sizeB + m_cttrTop + m_cttrBottom );
}

QSize DrawQWidget::sizeHint() const
{ 
  int sizeA = m_scalingA * m_dataDimmA * m_splitterA * c_dpr_inv;       if (sizeA < 1)  sizeA = 1;
  int sizeB = m_scalingB * (m_datex == DATEX_1D? 1 : m_dataDimmB) * m_splitterB * c_dpr_inv;      if (sizeB < 1)  sizeB = 1;
  return m_dataDimmSwitchAB ? QSize( sizeB + m_cttrLeft + m_cttrRight, sizeA + m_cttrTop + m_cttrBottom ) : QSize( sizeA + m_cttrLeft + m_cttrRight, sizeB + m_cttrTop + m_cttrBottom );
}

//bool DrawQWidget::event(QEvent* event)
//{
//  qDebug()<<(event->type());
//  return QOpenGLWidget::event(event);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const bool isPress[] = { true, false, false, true, true, false, true };

void  DrawQWidget::store_crd_clk(OVL_REACTION_MOUSE oreact, int x, int y)
{
  dcgeometry_t dch = this->geometryHorz();
  dcgeometry_t dcv = this->geometryVert();
  
  float singleDimmWidth = dch.length;
  float singleDimmHeight = dcv.length;
  float totalDimmWidth = singleDimmWidth * (m_dataDimmSwitchAB? m_splitterB : m_splitterA);
  float totalDimmHeight = singleDimmHeight * (m_dataDimmSwitchAB? m_splitterA : m_splitterB);
  
  float fx = (x*c_dpr - (dch.cttr_pre + dch.viewalign_pre));
  float fy = (y*c_dpr - (dcv.cttr_pre + dcv.viewalign_pre));
  
  if (isPress[oreact] == false)
  {
    if (fx < 0)  fx = 0; else if (fx >= totalDimmWidth) fx = totalDimmWidth - 1;
    if (fy < 0)  fy = 0; else if (fy >= totalDimmHeight) fy = totalDimmHeight - 1;
  }
  else if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;

  coordstriumv_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    ct.fx_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fy_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fx_rel = singleDimmHeight <=1 ? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fy_rel = singleDimmWidth <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  else
  {
    ct.fx_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fy_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fx_rel = singleDimmWidth <= 1? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fy_rel = singleDimmHeight <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  
  bool doStop = false, doUpdate = false;
  if (m_proactive)  m_proactive->reactionMouse(this, oreact, &ct, &doStop);
  if (!doStop)
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
      if (m_overlays[i].prct && (m_overlays[i].prct_bans & ORB_MOUSE) == 0)
      {
        if (m_overlays[i].prct->overlayReactionMouse(oreact, &ct, &doStop))
        {
          m_overlays[i].povl->increasePingerUpdate();
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
}

/*
//  float fx, fy;
//  if (orientationTransposed(m_orient))
//  {
//    fx = float((!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - y : y) % singleDimmHeight) / singleDimmHeight;
//    fy = float((orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - x : x) % singleDimmWidth) / singleDimmWidth;
//  }
//  else
//  {
//    fx = float((orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - x : x) % singleDimmWidth) / singleDimmWidth;
//    fy = float((!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - y : y) % singleDimmHeight) / singleDimmHeight;
//  }
*/

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

#if QT_CONFIG(wheelevent)
void DrawQWidget::wheelEvent(QWheelEvent* event)
{
  dcgeometry_t dch = this->geometryHorz();
  dcgeometry_t dcv = this->geometryVert();
  
  float singleDimmWidth = dch.length;
  float singleDimmHeight = dcv.length;
  float totalDimmWidth = singleDimmWidth * (m_dataDimmSwitchAB? m_splitterB : m_splitterA);
  float totalDimmHeight = singleDimmHeight * (m_dataDimmSwitchAB? m_splitterA : m_splitterB);
  
#if QT_VERSION < 6
  QPoint pos = event->pos();
#else
  QPoint pos = event->angleDelta();
#endif
  float fx = (pos.x()*c_dpr - (dch.cttr_pre + dch.viewalign_pre));
  float fy = (pos.y()*c_dpr - (dcv.cttr_pre + dcv.viewalign_pre));
  if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;

  coordstriumv_wheel_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    ct.fx_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fy_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fx_rel = singleDimmHeight <=1 ? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fy_rel = singleDimmWidth <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  else
  {
    ct.fx_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fy_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fx_rel = singleDimmWidth <= 1? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fy_rel = singleDimmHeight <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  ct.angle = event->angleDelta().y()/8.0f;
  ct.delta_x = event->angleDelta().x();
  ct.delta_y = event->angleDelta().y();
  
  bool doStop = false, doUpdate = false;
  if (m_proactive)  m_proactive->reactionWheel(this, ct.angle > 0 ? ORW_AWAY : ORW_TOWARD, &ct, &doStop);
  if (!doStop)
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
      if (m_overlays[i].prct && (m_overlays[i].prct_bans & ORB_MOUSE) == 0)
      {
        if (m_overlays[i].prct->overlayReactionWheel(ct.angle > 0 ? ORW_AWAY : ORW_TOWARD, &ct, &doStop))
        {
          m_overlays[i].povl->increasePingerUpdate();
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
}
#endif


void DrawQWidget::keyPressEvent(QKeyEvent* event)
{
  int modifiers = int(event->modifiers()) >> 24;
  int key = event->key();
  bool doStop = false, doUpdate = false;
  if (m_proactive)  m_proactive->reactionKey(this, key, modifiers, &doStop);
  if (!doStop)
    for (int i=int(m_overlaysCount)-1; i>=0; i--)
      if (m_overlays[i].prct && (m_overlays[i].prct_bans & ORB_KEYBOARD) == 0)
      {
        if (m_overlays[i].prct->overlayReactionKey(key, modifiers, &doStop))
        {
          m_overlays[i].povl->increasePingerUpdate();
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

void DrawQWidget::showEvent(QShowEvent* ev)
{
  QOpenGLWidget::showEvent(ev);
  if (havePending())
    if (!autoUpdateBanned(RD_BYDATA) || !autoUpdateBanned(RD_BYSETTINGS))
      callWidgetUpdate();
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

void DrawQWidget::slot_setScalingA(int s){  setScalingLimitsA((unsigned int)s, (unsigned int)s); }
void DrawQWidget::slot_setScalingB(int s){  setScalingLimitsB((unsigned int)s, (unsigned int)s); }
void DrawQWidget::slot_setScalingH(int s){  setScalingLimitsHorz((unsigned int)s, (unsigned int)s); }
void DrawQWidget::slot_setScalingV(int s){  setScalingLimitsVert((unsigned int)s, (unsigned int)s); }

void DrawQWidget::slot_setBounds(float low, float high){  setBounds(low, high); }
void DrawQWidget::slot_setBoundLow(float value){  setBoundLow(value); }
void DrawQWidget::slot_setBoundHigh(float value){ setBoundHigh(value);  }
void DrawQWidget::slot_setBounds01(){ setBounds(0.0f, 1.0f); }
void DrawQWidget::slot_setContrast(float k, float b){ setContrast(k, b);  }
void DrawQWidget::slot_setContrastK(float k){ setContrastK(k);  }
void DrawQWidget::slot_setContrastKinv(float k){ setContrastKinv(k); }
void DrawQWidget::slot_setContrastB(float b){ setContrastB(b);  }
void DrawQWidget::slot_setDataTextureInterpolation(bool d){ setDataTextureInterpolation(d); }
void DrawQWidget::slot_setDataPalette(const IPalette* ppal){ setDataPalette(ppal); }
void DrawQWidget::slot_setDataPaletteDiscretion(bool d){ setDataPaletteDiscretion(d); }
void DrawQWidget::slot_setDataPaletteRangeStart(float v){ setDataPaletteRangeStart(v); }
void DrawQWidget::slot_setDataPaletteRangeStop(float v){ setDataPaletteRangeStop(v); }
void DrawQWidget::slot_setDataPaletteRange(float start, float stop){  setDataPaletteRange(start, stop);  }
void DrawQWidget::slot_setData(const float* data){ setData(data); }
void DrawQWidget::slot_setData(QVector<float> data){ setData(data.constData()); }
void DrawQWidget::slot_fillData(float data){ fillData(data); }
void DrawQWidget::slot_clearData(){ clearData(); }

void DrawQWidget::slot_adjustBounds(){  adjustBounds();  }
void DrawQWidget::slot_adjustBounds(unsigned int portion){ adjustBounds(portion); }
void DrawQWidget::slot_adjustBounds(unsigned int start, unsigned int stop){ adjustBounds(start, stop); }
void DrawQWidget::slot_adjustBounds(unsigned int start, unsigned int stop, unsigned int portion){ adjustBounds(start, stop, portion); }
void DrawQWidget::slot_adjustBoundsWithSpacingAdd(float add2min, float add2max){  adjustBoundsWithSpacingAdd(add2min, add2max); }
void DrawQWidget::slot_adjustBoundsWithSpacingMul(float mul2min, float mul2max){  adjustBoundsWithSpacingMul(mul2min, mul2max); }

void DrawQWidget::slot_setMirroredHorz(){ setMirroredHorz(); }
void DrawQWidget::slot_setMirroredVert(){ setMirroredVert(); }
void DrawQWidget::slot_setPortionsCount(int count){  setPortionsCount(count); }

void DrawQWidget::slot_ovlReplace(int idx, Ovldraw* ovl){ ovlReplace(idx, ovl, OO_SAME, false); }

void DrawQWidget::slot_enableAutoUpdate(bool enabled){  banAutoUpdate(!enabled); }
void DrawQWidget::slot_disableAutoUpdate(bool disabled){  banAutoUpdate(disabled); }
void DrawQWidget::slot_enableAutoUpdateByData(bool enabled){  banAutoUpdate(RD_BYDATA, !enabled); }
void DrawQWidget::slot_disableAutoUpdateByData(bool disabled){  banAutoUpdate(RD_BYDATA, disabled); }


////////////////////////////////////////////////////////////////////////
void DrawQWidget::slot_setBoundLow_dbl(double value){  setBoundLow((float)value); }
void DrawQWidget::slot_setBoundHigh_dbl(double value){ setBoundHigh((float)value);  }
void DrawQWidget::slot_setContrast_dbl(double k, double b){ setContrast((float)k, (float)b);  }
void DrawQWidget::slot_setContrastK_dbl(double k){ setContrastK((float)k);  }
void DrawQWidget::slot_setContrastKinv_dbl(double k){ setContrastKinv((float)k); }
void DrawQWidget::slot_setContrastB_dbl(double b){ setContrastB((float)b);  }
void DrawQWidget::slot_setDataPaletteRangeStart_dbl(double s){  setDataPaletteRangeStart((float)s); }
void DrawQWidget::slot_setDataPaletteRangeStop_dbl(double s){  setDataPaletteRangeStart((float)s); }
////////////////////////////////////////////////////////////////////////

int   DrawQWidget::scrollValue() const
{
  return 0;
}

unsigned int DrawQWidget::lmSize() const
{
  return m_matrixLmSize;
}

dcgeometry_t DrawQWidget::geometryHorz() const
{
  int rest = c_width - m_cttrLeft - m_cttrRight - (int)sizeHorz();
  if (rest <= 0) return { m_cttrLeft, 0, int(sizeHorz()), 0, m_cttrRight }; 
  int rh = int(rest*m_viewAlignHorz);
  return { m_cttrLeft, rh, int(sizeHorz()), rest - rh, m_cttrRight }; 
}

dcgeometry_t DrawQWidget::geometryVert() const
{
  int rest = c_height - m_cttrTop - m_cttrBottom - (int)sizeVert();
  if (rest <= 0) return { m_cttrTop, 0, int(sizeVert()), 0, m_cttrBottom }; 
  int rv = int(rest*m_viewAlignVert);
  return { m_cttrTop, rv, int(sizeVert()), rest - rv, m_cttrBottom }; 
}

void  DrawQWidget::scrollDataTo(int)
{
}

void DrawQWidget::scrollDataToAbs(int)
{
}



////////////////////////////////////////////////////////////////////////////////

int DrawQWidget::_tft_allocHolder(QFont font)
{
  for (int i=0; i<TFT_HOLDERS; i++)
    if (m_holders[i] == nullptr)
    {
      m_holders[i] = new TFTholder;
      m_holders[i]->font = font;
      m_holders[i]->ponger = 0;
      m_holders[i]->pinger = 1;
      m_holders[i]->_location = -1;
      
      QFontMetrics qfm(font);
      m_holders[i]->record_width = qfm.averageCharWidth() * TFT_TEXTMAXLEN + 2;
      m_holders[i]->record_ht = qfm.ascent();
      m_holders[i]->record_hb = qfm.descent();
      m_holders[i]->record_ld = qfm.leading();
      m_holders[i]->record_height = qfm.height() + qfm.leading() + 2;
      
      m_holders[i]->recordslimit = 1080 / m_holders[i]->record_height;
#ifndef BSGLSLOLD
      {
        TFTarea area;
        area.img = _tft_allocateImage(m_holders[i]->record_width, m_holders[i]->record_height*m_holders[i]->recordslimit);
        area.records = new TFTrecord[m_holders[i]->recordslimit];
        area.recordscount = 0;
        m_holders[i]->tftarea.push_back(area);
      }
#else
      m_holders[i]->tftarea.img = _tft_allocateImage(m_holders[i]->record_width, m_holders[i]->record_height*m_holders[i]->recordslimit);
      m_holders[i]->tftarea.records = new TFTrecord[m_holders[i]->recordslimit];
      m_holders[i]->tftarea.recordscount = 0;
#endif
      return i;
    }
  return -1;
}

#include <QLabel>

int DrawQWidget::_tft_pushRecord(DrawQWidget::TFTholder* holder, const char* text)
{
#ifndef BSGLSLOLD
  if (holder->tftarea.back().recordscount >= holder->recordslimit)
  {
    TFTarea area;
    area.img = _tft_allocateImage(holder->record_width, holder->record_height*holder->recordslimit);
    area.records = new TFTrecord[holder->recordslimit];
    area.recordscount = 0;
    holder->tftarea.push_back(area);
  }
  TFTarea& tftar = holder->tftarea.back();
#else
  if (holder->tftarea.recordscount >= holder->recordslimit)
    return -1;
  TFTarea& tftar = holder->tftarea;
#endif
  int rid_loc = tftar.recordscount++;
  TFTrecord* tftrec = &tftar.records[rid_loc];
  strncpy(tftrec->text, text, TFT_TEXTMAXLEN);
  QFontMetrics qfm(holder->font);
  tftrec->width = qfm.horizontalAdvance(tftrec->text);
  {
    QPainter ptr(tftar.img);
//    ptr.begin()
    ptr.setBrush(QBrush(QColor(0,0,0)));
    ptr.setPen(QColor(0,0,0));
    ptr.setFont(holder->font);
    //QFont("Ubuntu", 24)
//    qDebug()<<rid_loc*holder->c_record_height;
    ptr.drawText(1, holder->record_height*holder->recordslimit - rid_loc*holder->record_height - holder->record_hb, /*Qt::AlignLeft | Qt::AlignBottom, */text);
//    ptr.drawText(10, 10, text);
  }
  holder->pinger += 1;
  
//  QLabel* lbl = new QLabel();
//  lbl->setPixmap(QPixmap::fromImage(*tftar.img));
//  lbl->show();
#ifndef BSGLSLOLD
  return (holder->tftarea.size()-1)*holder->recordslimit + rid_loc;
#else  
  return rid_loc;
#endif
}

QImage* DrawQWidget::_tft_allocateImage(int width, int height)
{
  QImage* img = new QImage(width, height, QImage::Format_ARGB32);
//  img->detach();
  img->fill(Qt::transparent);
//  img->fill(0xFFFFFFFF);
  return img;
}

DrawQWidget::TFTslotpass DrawQWidget::tftPushBack(const char* text, COORDINATION cr, float fx, float fy, bool isstatic)
{
  if (m_holder_current == -1)
    m_holder_current = _tft_allocHolder(font());
  
  int recid = _tft_pushRecord(m_holders[m_holder_current], text);
  TFTslot ts;
  ts.isstatic = isstatic;
  ts.recid = recid;
  ts.slotinfo.cr = cr;
  ts.slotinfo.fx = fx;
  ts.slotinfo.fy = fy;
  ts.slotinfo.rotate = 0.0f;
  ts.slotinfo.scale = 1.0f;
  if (ts.isstatic)
  {
    ts.pinger = 0;
    ts.ponger = 0;
  }
  else
  {
    ts.pinger = 1;
    ts.ponger = 0;
  }
  m_holders[m_holder_current]->tftslots.push_back(ts);
  int sid = m_holders[m_holder_current]->tftslots.size()-1;  
  if (recid != -1)
  {
    m_bitmaskPendingChanges |= PC_INIT | PC_TFT_TEXTURE | PC_TFT_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  return TFTslotpass(this, m_holder_current, sid);
}

int DrawQWidget::tftCountRecords(int hoid) const
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return 0;
#ifndef BSGLSLOLD  
  return (m_holders[hoid]->tftarea.size()-1)*m_holders[hoid]->recordslimit + m_holders[hoid]->tftarea.back().recordscount;
#else
  return m_holders[hoid]->tftarea.recordscount;
#endif
}

int DrawQWidget::tftCountSlots(int hoid) const
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return 0;
  return m_holders[hoid]->tftslots.size();
}

int DrawQWidget::tftRecordsPerArea(int hoid)
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return 0;
  return m_holders[hoid]->recordslimit;
}

int DrawQWidget::tftRecordForSlot(int hoid, int sloid) const
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return -1;
  if (sloid >= m_holders[hoid]->tftslots.size())
    return -1;
  return m_holders[hoid]->tftslots[sloid].recid;
}

bool DrawQWidget::tftMove(int hoid, int sloid, float fx, float fy)
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return false;
  if (sloid >= m_holders[hoid]->tftslots.size())
    return false;
  if (m_holders[hoid]->tftslots[sloid].isstatic)
    return false;
  m_holders[hoid]->tftslots[sloid].slotinfo.fx = fx;
  m_holders[hoid]->tftslots[sloid].slotinfo.fy = fy;
  {
    m_holders[hoid]->tftslots[sloid].pinger += 1;
    m_bitmaskPendingChanges |= PC_TFT_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  return true;
}

bool DrawQWidget::tftSwitch(int hoid, int sloid, int recid)
{
  Q_ASSERT(hoid < TFT_HOLDERS);
  if (m_holders[hoid] == nullptr)
    return false;
  if (sloid >= m_holders[hoid]->tftslots.size())
    return false;
  if (m_holders[hoid]->tftslots[sloid].isstatic)
    return false;
  m_holders[hoid]->tftslots[sloid].recid = recid;
  {
    m_holders[hoid]->tftslots[sloid].pinger += 1;
    m_bitmaskPendingChanges |= PC_TFT_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////



DrawQWidget::MemExpand2D::MemExpand2D(unsigned int portionsCount, unsigned int portionSize, unsigned int linesMemory): pc(portionsCount), ps(portionSize), memoryLines(linesMemory)
{
  mb.current = mb.filled = 0;
  mb.extendeddataarr = new float[memoryLines*pc*ps];
}

DrawQWidget::MemExpand2D::~MemExpand2D()
{
  delete []mb.extendeddataarr;
}

void DrawQWidget::MemExpand2D::onSetData(const float *data)
{
  if (++mb.current >= memoryLines)    mb.current = 0;
  memcpy(&mb.extendeddataarr[mb.current*pc*ps], data, pc*ps*sizeof(float));
  if (mb.filled < memoryLines)    mb.filled++;
}

void DrawQWidget::MemExpand2D::onSetData(const float* data, DataDecimator* decim)
{
  if (++mb.current >= memoryLines)    mb.current = 0;
  for (unsigned int p=0; p<pc; p++)
    for (unsigned int i=0; i<ps; i++)
      mb.extendeddataarr[mb.current*pc*ps + p*ps + i] = decim->decimate(data, ps, i, p);
  if (mb.filled < memoryLines)    mb.filled++;
}

void DrawQWidget::MemExpand2D::onClearData()
{
  mb.filled = 0;
}

bool DrawQWidget::MemExpand2D::onFillData(unsigned int portion, int pos, float *rslt) const
{
  if (pos < (int)mb.filled)
  {
    int j = (int)mb.current - (int)pos;
    if (j < 0)  j += memoryLines;
    memcpy(rslt, &mb.extendeddataarr[j*pc*ps + portion*ps], ps*sizeof(float));
    return true;
  }
  return false;
}

bool DrawQWidget::MemExpand2D::onFillDataBackward(unsigned int portion, int pos, float *rslt) const
{
  if (pos < (int)mb.filled)
  {
    int j = (int)mb.current - ((int)mb.filled - 1 - (int)pos);
    if (j < 0)  j += memoryLines;
    memcpy(rslt, &mb.extendeddataarr[j*pc*ps + portion*ps], ps*sizeof(float));
    return true;
  }
  return false;
}

unsigned int DrawQWidget::MemExpand2D::onCollectData(unsigned int portion, int pos, unsigned int sampleHorz, float* result, unsigned int countVerts, bool reverse) const
{
  if (pos < (int)mb.filled)
  {
    if (pos + countVerts > mb.filled)
      countVerts = mb.filled - pos;
    if (reverse == false)
    {
      for (unsigned int i=0; i<countVerts; i++)
      {
        int j = (int)mb.current - (int)(pos + i)/*((int)mb.filled - 1 - (int)i)*/;
        if (j < 0)  j += memoryLines;
        result[i] = mb.extendeddataarr[j*pc*ps + portion*ps + sampleHorz];
      }
    }
    else
    {
      for (unsigned int i=0; i<countVerts; i++)
      {
        int j = (int)mb.current - (int)(pos + i)/*((int)mb.filled - 1 - (int)i)*/;
        if (j < 0)  j += memoryLines;
        result[countVerts - 1 - i] = mb.extendeddataarr[j*pc*ps + portion*ps + sampleHorz];
      }
    }
    return countVerts;
  }
  return 0;
}

DrawQWidget::MemExpand2D::mem_t DrawQWidget::MemExpand2D::extendeddataarr_replicate()
{
  DrawQWidget::MemExpand2D::mem_t result = mb;
  mb.extendeddataarr = new float[memoryLines*pc*ps];
  mb.current = mb.filled = 0;
  return result;
}

DrawQWidget::MemExpand2D::mem_t DrawQWidget::MemExpand2D::extendeddataarr_replace(mem_t md)
{
  DrawQWidget::MemExpand2D::mem_t result = mb;
  mb = md;
  return result;
}

void DrawQWidget::MemExpand2D::extendeddataarr_release(mem_t md) const
{
  delete []md.extendeddataarr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawQWidget::MemExpand1D::MemExpand1D(unsigned int portionsCount, unsigned int portionSize, unsigned int additionalMemorySizeFor1Portion, bool anchor): 
  pc(portionsCount), pt(portionSize + additionalMemorySizeFor1Portion),
  pm(portionSize + additionalMemorySizeFor1Portion), anchoring(anchor),
  rounded(false), current(0)
{
  m_extendeddataarr = new float[pc*pt];
}

DrawQWidget::MemExpand1D::~MemExpand1D()
{
  delete []m_extendeddataarr;
}

unsigned int DrawQWidget::MemExpand1D::rangeMemsize(unsigned int memorySize)
{
  if (memorySize > pt)  memorySize = pt;
  if (pm != memorySize)
  {
    pm = memorySize;
    current = 0;    // strict here
    rounded = false;
  }
  return pm;
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

void DrawQWidget::MemExpand1D::onFillData(int offsetBack, unsigned int samples, float *rslt, float emptyfill) const
{
  int pos = current/* - ps*/ - offsetBack;
  if (pos < 0)
  {
    if (rounded)
      while (pos < 0)
        pos += pm;
    else
      pos = 0;
  }
  unsigned int fillSize[] = { samples, 0 };  /// first copy, second copy (by loop or emptyfill)
  bool sndIsEmpty = false;
  int  anchoro1 = 0;
  int  anchoro2 = 0;
  
  if (rounded == false && pos > (int)current)
  {
    fillSize[0] = 0;
    fillSize[1] = samples;
    sndIsEmpty = true;
  }
  else if (rounded == false && current - pos < samples)
  {
    fillSize[0] = current - pos;
    fillSize[1] = samples - fillSize[0];
    sndIsEmpty = true;
    if (anchoring)
    {
      anchoro1 = samples - fillSize[0];
      anchoro2 = -fillSize[0];
    }
  }
  else if (rounded == true && pos + samples > pm)
  {
    fillSize[0] = pm - pos;
    fillSize[1] = samples - fillSize[0];
  }
  
  for (unsigned int p=0; p<pc; p++)
  {
    memcpy(rslt + p*samples + anchoro1, &m_extendeddataarr[p*pm + pos], fillSize[0]*sizeof(float));
    if (sndIsEmpty == false)
      memcpy(rslt + p*samples + fillSize[0] + anchoro2, &m_extendeddataarr[p*pm + 0], fillSize[1]*sizeof(float));
    else
      for (unsigned int i=0; i<fillSize[1]; i++)
        *(rslt + i + p*samples + fillSize[0] + anchoro2) = emptyfill;
//        rslt[int(i + p*samples + fillSize[0]) + anchoro2] = emptyfill;
  }
}

////////////////////////////////////////////////////////////////////////////////


BSQProactiveSelectorBase::~BSQProactiveSelectorBase(){}



BSQClickerXY::BSQClickerXY(QObject* parent): QObject(parent)
{
}

bool BSQClickerXY::reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == ORM_LMPRESS)
  {
    emit clicked(ct->fx_rel, ct->fy_rel);
    return true;
  }
  if (oreact == ORM_RMPRESS)
  {
    emit released();
    return true;
  }
  return false;
}


BSQClickerPoint::BSQClickerPoint(OVL_REACTION_MOUSE em, QObject* parent): QObject(parent), emitter(em)
{
}

bool BSQClickerPoint::reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == emitter)
  {
    emit clicked(QPoint(ct->fx_pix, ct->fy_pix));
    return true;
  }
  return false;
}



BSQMousePoint::BSQMousePoint(BSQMousePoint::MOUSEBUTTON btn, QObject* parent): QObject(parent)
{
  if (btn == MSP_LEFTBUTTON)
  {
    emitset[0] = ORM_LMPRESS; emitset[1] = ORM_LMMOVE; emitset[2] = ORM_LMRELEASE;
  }
  else if (btn == MSP_RIGHTBUTTON)
  {
    emitset[0] = ORM_RMPRESS; emitset[1] = ORM_RMMOVE; emitset[2] = ORM_RMRELEASE;
  }
}

bool BSQMousePoint::reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == emitset[0] || oreact == emitset[1] || oreact == emitset[2])
  {
    emit active(QPoint(ct->fx_pix, ct->fy_pix));
    emit active(QPointF(ct->fx_rel, ct->fy_rel));
    return true;
  }
  return false;
}


bool BSQDoubleClicker::reactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == ORM_LMDOUBLE)
  {
    *doStop = true;
    emit doubleClicked();
//    emit doubleClicked(pwdg->mapToGlobal(QPoint(((const float*)dataptr)[0], ((const float*)dataptr)[1])));
//    emit doubleClicked(pwdg->mapToGlobal(QPoint(((const float*)dataptr)[2], ((const float*)dataptr)[3])));
    emit doubleClicked(QPoint(ct->fx_pix, ct->fy_pix));
  }
  return false;
}

bool BSQProactiveSelector::reactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
//    if (pwdg->splitterA() > 1 || pwdg->splitterB() > 1)  ???
    {
      unsigned int ssA = pwdg->sizeA()/pwdg->splitterA(), ssB = pwdg->sizeB()/pwdg->splitterB();
      int x = ct->fx_pix, y = ct->fy_pix;
      int portion = 0;
      if (pwdg->isSplittedA())
        portion = pwdg->allocatedPortions() - 1 - y / ssB;
      else if (pwdg->isSplittedB())
        portion = x / ssA;
      emit selectionChanged(portion + m_startswith);
    }
  }
  else if (oreact == m_drop)  emit selectionDropped();
  return false;
}

bool BSQCellSelector::reactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
    ORIENTATION orient = pwdg->orientation();
    {
      unsigned int ssA = pwdg->sizeA()/pwdg->sizeDataA(), ssB = pwdg->sizeB()/pwdg->sizeDataB();
      int x = ct->fx_pix, y = ct->fy_pix;
      unsigned int cellA = (orientationTransposed(orient)? y : x) / ssA;
      unsigned int cellB = (orientationTransposed(orient)? x : y) / ssB;
      if (orientationMirroredHorz(orient))
        cellA = pwdg->sizeDataA() - 1 - cellA;
      if (orientationMirroredVert(orient))
        cellB = pwdg->sizeDataB() - 1 - cellB;
      emit selectionChanged(cellA + m_startswith, cellB + m_startswith);
    }
  }
  else if (oreact == m_drop)  emit selectionDropped();
  return false;
}


bool BSQSelectorA::reactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
    ORIENTATION orient = pwdg->orientation();
    {
      unsigned int ssA = pwdg->sizeA()/pwdg->sizeDataA();
      int posA = orientationTransposed(orient) ? ct->fy_pix : ct->fx_pix;
      unsigned int cellA = posA / ssA;
      bool mirrored = (orientationTransposed(orient) ? orientationMirroredHorz(orient) : orientationMirroredVert(orient)) ^ m_inversed;
      if (mirrored)
        cellA = pwdg->sizeDataA() - 1 - cellA;
      emit selectionChanged(cellA + m_startswith);
    }
  }
  else if (oreact == m_drop)  emit selectionDropped();
  return false;
}

bool BSQSelectorB::reactionMouse(DrawQWidget* pwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
    ORIENTATION orient = pwdg->orientation();
    {
      unsigned int ssB = pwdg->sizeB()/pwdg->sizeDataB();
      int posB = orientationTransposed(orient) ? ct->fx_pix : ct->fy_pix;
      unsigned int cellB = posB / ssB;
      bool mirrored = (orientationTransposed(orient) ? orientationMirroredVert(orient) : orientationMirroredHorz(orient)) ^ m_inversed;
      if (mirrored == false)
        cellB = pwdg->sizeDataB() - 1 - cellB;
      emit selectionChanged(cellB + m_startswith);
    }
  }
  else if (oreact == m_drop)  emit selectionDropped();
  return false;
}




#if 0     /// Integer calculatons
  unsigned int singleDimmWidth = sizeHorz();
  unsigned int singleDimmHeight = sizeVert();
  unsigned int totalDimmWidth = singleDimmWidth * (m_dataDimmSwitchAB? m_splitterB : m_splitterA);
  unsigned int totalDimmHeight = singleDimmHeight * (m_dataDimmSwitchAB? m_splitterA : m_splitterB);
  x -= m_cttrLeft;    y -= m_cttrTop;
  x *=  c_dpr;        y *=  c_dpr;

  if (isPress[oreact] == false)
  {
    if (x < 0)  x = 0; else if (x >= totalDimmWidth) x = int(totalDimmWidth) - 1;
    if (y < 0)  y = 0; else if (y >= totalDimmHeight) y = int(totalDimmHeight) - 1;
  }
  else if (x >= totalDimmWidth || y >= totalDimmHeight)
    return;

  coordstriumv_t ct;
  ct.fx_pix = x;
  ct.fy_pix = y;
  if (orientationTransposed(m_orient))
  {
    ct.fx_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - y : y);
    ct.fy_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - x : x);
    ct.fx_rel = singleDimmHeight <=1 ? 0 : float(int(ct.fx_ovl) % singleDimmHeight) / (singleDimmHeight - 1);
    ct.fy_rel = singleDimmWidth <= 1? 0 : float(int(ct.fy_ovl) % singleDimmWidth) / (singleDimmWidth - 1);
//    ct.fx_ovl = singleDimmHeight <=1 ? 0 : float(int(ct.fx_ovl) % singleDimmHeight) / singleDimmHeight;
//    ct.fy_ovl = singleDimmWidth <= 1? 0 : float(int(ct.fy_ovl) % singleDimmWidth) / singleDimmWidth;
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  else
  {
    ct.fx_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - x : x);
    ct.fy_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - y : y);
    ct.fx_rel = singleDimmWidth <= 1? 0 : float(int(ct.fx_ovl) % singleDimmWidth) / (singleDimmWidth - 1);
    ct.fy_rel = singleDimmHeight <= 1? 0 : float(int(ct.fy_ovl) % singleDimmHeight) / (singleDimmHeight - 1);
//    ct.fx_ovl = singleDimmWidth <= 1? 0 : float(int(ct.fx_ovl) % singleDimmWidth) / singleDimmWidth;
//    ct.fy_ovl = singleDimmHeight <= 1? 0 : float(int(ct.fy_ovl) % singleDimmHeight) / singleDimmHeight;
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
#elif 0   /// floating calculations
  float singleDimmWidth = sizeHorz();
  float singleDimmHeight = sizeVert();
  float totalDimmWidth = singleDimmWidth * (m_dataDimmSwitchAB? m_splitterB : m_splitterA);
  float totalDimmHeight = singleDimmHeight * (m_dataDimmSwitchAB? m_splitterA : m_splitterB);
  
  float fx = (x - m_cttrLeft) * c_dpr;
  float fy = (y - m_cttrTop) * c_dpr;
  
  if (isPress[oreact] == false)
  {
    if (fx < 0)  fx = 0; else if (fx >= totalDimmWidth) fx = totalDimmWidth - 1;
    if (fy < 0)  fy = 0; else if (fy >= totalDimmHeight) fy = totalDimmHeight - 1;
  }
  else if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;

  coordstriumv_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    ct.fx_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fy_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fx_rel = singleDimmHeight <=1 ? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fy_rel = singleDimmWidth <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
  else
  {
    ct.fx_ovl = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fy_ovl = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fx_rel = singleDimmWidth <= 1? 0 : (ct.fx_ovl - int(ct.fx_ovl/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fy_rel = singleDimmHeight <= 1? 0 : (ct.fy_ovl - int(ct.fy_ovl/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fx_ovl = ct.fx_rel;
    ct.fy_ovl = ct.fy_rel;
  }
#endif

    
#if 0
    qDebug()<<"CTX: "<<CORR_TEX;
    if (!havePendOn(PC_TFT_TEXTURE, bitmaskPendingChanges))
    {
      qDebug()<<"!hpo";
      for (unsigned int t=0; t<TFT_HOLDERS; t++)
      {
        if (m_holders[t] == nullptr)
          continue;
        qDebug()<<m_holders[t]->ponger<<m_holders[t]->pinger<<m_holders[t]->_location;
  #ifndef BSGLSLOLD
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
  #else
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE, m_textures[CORR_TEX]);
  #endif
        CORR_TEX++;
      }
    }
    else
    {
      qDebug()<<"hpo";
      for (unsigned int t=0; t<TFT_HOLDERS; t++)
      {
        if (m_holders[t] == nullptr)
          continue;
        const GLint   gl_internalFormat = GL_RGBA8;
        const GLenum  gl_format = GL_RGBA;
        const GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
        int width = m_holders[t]->record_width;
        int height = m_holders[t]->record_height * m_holders[t]->recordslimit;
        qDebug()<<m_holders[t]->ponger<<m_holders[t]->pinger<<m_holders[t]->_location;
      
  #ifndef BSGLSLOLD
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
        if (m_holders[t]->ponger != m_holders[t]->pinger)
        {
          int LAY = m_holders[t]->tftarea.size();
          glTexStorage3D(  GL_TEXTURE_2D_ARRAY, LAY, gl_internalFormat, width, height, LAY);
          for (int r=0; r<LAY; r++)
          {
            const TFTarea&  area = m_holders[t]->tftarea[r];
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, r, width, height, 1, gl_format, gl_texture_type, area.img->constBits());
          }
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
  #if QT_VERSION >= 0x050000
          glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
          glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
  #endif
          glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
          
          m_holders[t]->ponger = m_holders[t]->pinger;
        }
  #else
        glActiveTexture(GL_TEXTURE0 + CORR_TEX);
        glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
        if (m_holders[t]->ponger != m_holders[t]->pinger)
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
          glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
          glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
          
          glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
          glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
          
  #if QT_VERSION >= 0x050000
          glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
          glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
  #endif
          const TFTarea&  area = m_holders[t]->tftarea;
          qDebug()<<"IMAGE ASSIGNED!"<<width<<height;
          glTexImage2D(GL_TEXTURE_2D, 0, gl_internalFormat, width, height, 0, gl_format, gl_texture_type, area.img->constBits());
          
          m_holders[t]->ponger = m_holders[t]->pinger;
        }
  #endif
        m_ShaderProgram.setUniformValue(m_holders[t]->_location, CORR_TEX);
        CORR_TEX++;
      }
    }
#endif
