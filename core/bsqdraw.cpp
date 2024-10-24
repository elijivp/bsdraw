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


struct  tftdesignedimage_t
{
  struct  design_t
  {
    char  text[TFT_TEXTMAXLEN+1];
    int   width;
  };
  QImage*       image;
  design_t*     designs;
  int           designscount;
};

class BSQSetup
{
public:
  QFontMetrics            fm;
  QPainter                painter;
  QBrush                  brush;
  QPen                    pen;
  BSQSetup(const QFont& f): fm(f) {}
};

enum  { TFT_MAXPAGES=256 };

struct  tftgeterbook_t: tftgetermetrics_t
{
  QFont                     font;
  BSQSetup                  ctx_setup;
  
  int                       limrows;   // by font
  int                       limcols;
  int                       c_total;
#ifndef BSGLSLOLD
  tftdesignedimage_t        dbookpages[TFT_MAXPAGES];
  unsigned int              dbookpagescount;
#else
  tftdesignedimage_t        dbookpages;
#endif
  
  tftgeterbook_t(const QFont& fnt): font(fnt), ctx_setup(fnt){}
};


inline QImage* _tftgeterbook_allocimage(int width, int height)
{
  QImage* img = new QImage(width, height, QImage::Format_ARGB32);
  img->fill(Qt::transparent);
  return img;
}


tftgeterbook_t*  tftgeterbook_alloc(QFont font, int maxtextlen, int limitcolumns, QColor color)
{
  tftgeterbook_t* result = new tftgeterbook_t(font);
  result->font = font;
  result->maxtextlen = maxtextlen < TFT_TEXTMAXLEN ? maxtextlen : TFT_TEXTMAXLEN;
  result->ctx_setup.brush = QBrush(color);
  result->ctx_setup.pen = QPen(color);
  
  result->design_width = result->ctx_setup.fm.averageCharWidth() * result->maxtextlen + 2;
  result->design_ht = result->ctx_setup.fm.ascent();
  result->design_hb = result->ctx_setup.fm.descent();
  result->design_ld = result->ctx_setup.fm.leading();
  result->design_height = result->ctx_setup.fm.height() + result->ctx_setup.fm.leading() + 2;
  
  result->limcols = limitcolumns;
  result->limrows = 1080 / result->design_height;
  result->c_total = result->limrows*result->limcols;
  
#ifndef BSGLSLOLD
  {
    result->dbookpages[0].image = _tftgeterbook_allocimage(result->design_width*result->limcols,
                                      result->design_height*result->limrows);
    result->dbookpages[0].designs = new tftdesignedimage_t::design_t[result->c_total];
    result->dbookpages[0].designscount = 0;
    result->dbookpagescount = 1;
  }
#else
  result->dbookpages.image = _tftgeterbook_allocimage( result->design_width*result->limcols,
                                                      result->design_height*result->limrows);
  result->dbookpages.records = new tftdesignedimage_t::design_t[result->c_total];
  result->dbookpages.recordscount = 0;
#endif
  return result;
}

int tftgeterbook_addtext(tftgeterbook_t* th, const char* text)
{
  const char* texts[] = {text};
  return tftgeterbook_addtexts(th, 1, texts);
}

int tftgeterbook_addtexts(tftgeterbook_t* th, int count, const char* texts[])
{
  QImage* imgprev = nullptr;
  int     result_rid = -1;
  for (int i=0; i<count; i++)
  {
#ifndef BSGLSLOLD
    if (th->dbookpages[th->dbookpagescount-1].designscount >= th->c_total)
    {
      if (th->dbookpagescount >= TFT_MAXPAGES)
        return -1;
      th->dbookpages[th->dbookpagescount].image = _tftgeterbook_allocimage(th->design_width*th->limcols, th->design_height*th->limrows);
      th->dbookpages[th->dbookpagescount].designs = new tftdesignedimage_t::design_t[th->c_total];
      th->dbookpages[th->dbookpagescount].designscount = 0;
      th->dbookpagescount++;
    }
    tftdesignedimage_t* tftar = &th->dbookpages[th->dbookpagescount-1];
#else
    if (th->dbookpages.recordscount >= th->c_total)
      return -1;
    tftdesignedimage_t* tftar = &th->dbookpages;
#endif
  
    int rid_loc = tftar->designscount++;
#ifndef BSGLSLOLD
    if (result_rid == -1)
      result_rid = (th->dbookpagescount-1)*(th->c_total) + rid_loc;
#else  
    if (result_rid == -1)
      result_rid = rid_loc;
#endif
    tftdesignedimage_t::design_t* design = &tftar->designs[rid_loc];
    strncpy(design->text, texts[i], th->maxtextlen+1);
    design->width = th->ctx_setup.fm.horizontalAdvance(design->text);
    
    if (imgprev != tftar->image)
    {
      if (imgprev != nullptr)
        th->ctx_setup.painter.end();
      
      th->ctx_setup.painter.begin(tftar->image);
      th->ctx_setup.painter.setBrush(th->ctx_setup.brush);
      th->ctx_setup.painter.setPen(th->ctx_setup.pen);
      th->ctx_setup.painter.setFont(th->font);
      imgprev = tftar->image;
    }
    
    int row = rid_loc % th->limrows;
    int col = rid_loc / th->limrows;
//    th->ctx_setup.painter.drawText(
//          1 + col*(th->design_width), 
//          th->design_height*th->limrows - row*th->design_height - th->design_hb, 
//          text);
    th->ctx_setup.painter.drawText(
          1 + col*(th->design_width) + th->design_width/2 - design->width/2, 
          th->design_height*th->limrows - row*th->design_height - th->design_hb, 
          texts[i]);
  } // for
  if (count > 0)
    th->ctx_setup.painter.end();
  return result_rid;
}

void  tftgeterbook_setup(tftgeterbook_t* th, const QColor& clr)
{
  th->ctx_setup.brush = QBrush(clr);
  th->ctx_setup.pen = QPen(clr);
}
inline int  tftgeterbook_designscount(tftgeterbook_t* th)
{
#ifndef BSGLSLOLD
  return (th->dbookpagescount-1)*th->c_total + th->dbookpages[th->dbookpagescount-1].designscount;
#else
  return th->dbookpages.recordscount;
#endif
}
inline int   tftgeterbook_designsperarea(tftgeterbook_t* th)
{
  return th->c_total;
}
inline const char* tftgeterbook_recordtext(tftgeterbook_t* th, int dsgid)
{
#ifndef BSGLSLOLD  
  return th->dbookpages[dsgid/th->c_total].designs[dsgid%th->c_total].text;
#else
  return th->dbookpages.records[dsgid].text;
#endif
}
  
void  tftgeterbook_release(tftgeterbook_t* th)
{
#ifndef BSGLSLOLD
  for (unsigned int i=0; i<th->dbookpagescount; i++)
  {
    delete th->dbookpages[i].image;
    delete []th->dbookpages[i].designs;
  }
#else
  delete th->dbookpages.image;
  delete []th->dbookpages.records;
#endif
  delete th;
}


tftgetermetrics_t tftgeterbook_metrics(const tftgeterbook_t* th)
{
  return *(tftgetermetrics_t*)th;
}

QFont tftgeterbook_font(const tftgeterbook_t* th)
{
  return th->font;
}



extern int msprintf(char* to, const char* format, ...);

int OvldrawEmpty::fshOVCoords(int overlay, bool /*switchedab*/, char* to) const
{
  return msprintf(to, "vec4 overlayOVCoords%d(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, "
                                             "in vec2 coords, in float dvalue, in float thick, "
                                             "in vec2 root_offset_px, in vec3 post_in, out vec2 this_offset_px)"
                                          "{ return vec4(0.0,0.0,0.0,0.0); }\n", overlay);
}

int OvldrawEmpty::fshColor(int overlay, char* to) const
{
  return msprintf(to, "vec3 overlayColor%d(in vec3 color_drawed, in vec3 color_ovled, in vec4 in_variant) { return color_ovled; }\n", overlay);
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
  
  m_tfth_current = -1;
  for (int i=0; i<TFT_HOLDINGS; i++)
    m_tfths[i] = nullptr;
  
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
  
  for (unsigned int t=0; t<TFT_HOLDINGS; t++)
    if (m_tfths[t] != nullptr)
    {
      if (m_tfths[t]->geterbookowner)
        tftgeterbook_release(m_tfths[t]->geterbook);
      delete m_tfths[t];
    }
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

void DrawQWidget::palettePrepare(const IPalette* ppal, bool discrete, int levels, float range0, float range1)
{
  const void*   palArr;
  unsigned int  palSize;
  unsigned int  palFormat;
  
  ppal->paletteData(&palArr, &palSize, &palFormat);
  
  if (palFormat == IPalette::FMT_UNKNOWN)
    return;
  
  const GLenum  formats[] = { GL_UNSIGNED_BYTE, GL_FLOAT };
  GLenum format = formats[palFormat-1];
  
  if (palSize > 0)
  {
    unsigned int   r0i = range0*palSize;  if (r0i >= palSize) r0i = palSize-1;
    unsigned int   r1o = range1*palSize;  if (r1o > palSize)  r1o = palSize;
    if (format == GL_UNSIGNED_BYTE)
    {
      const unsigned char* uiArr = (const unsigned char*)palArr;
      palArr = (const void*)&uiArr[4*r0i];
    }
    else if (format == GL_FLOAT)
    {
      const float* flArr = (const float*)palArr;
      palArr = (const void*)&flArr[r0i];
    }
    palSize = r1o - r0i;
  }
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(palSize) / levels, levels, 0, GL_RGBA, format, (const void*)palArr);
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
  for (unsigned int t=0; t<TFT_HOLDINGS; t++)
    if (m_tfths[t] != nullptr)
    {
      fshps += m_tfths[t]->wri_statics.count*1024;
      fshps += m_tfths[t]->wri_dynamics[TFTD_COMT].count*1024;
      if (m_tfths[t]->wri_enable_dynamic_RT)
        fshps += 1024;
    }
  if (fshps < 32768)
    fshps = 32768;

  if (m_fshalloc < fshps)
  {
    if (m_fshalloc)  delete []m_fshmem;
    m_fshalloc = fshps;
    m_fshmem = new char[m_fshalloc];
  }
  
  
  /// store
  FshDrawMain fdm(m_fshmem, m_splitPortions, m_datasubmesh);
  fdm.generic_decls_begin();
  unsigned int    additufm_count=0;
  shuniformdesc_t additufm_desc[96];
  {
    additufm_count = m_pcsh->shfragment_uniforms(additufm_desc, sizeof(additufm_desc)/sizeof(shuniformdesc_t));
    for (unsigned int i=0; i<additufm_count; i++)
      fdm.generic_decls_add(additufm_desc[i].type, additufm_desc[i].varname);
  }
  bool havetft = false;
  {
    for (unsigned int t=0; t<TFT_HOLDINGS; t++)
    {
      if (m_tfths[t] == nullptr)  continue;
      
      m_tfths[t]->_location_h = -1;
      fdm.generic_decls_add_tft_area(t, m_tfths[t]->_varname_h);
      
      m_tfths[t]->_location_dt[TFTD_COMT] = -1;
      if (m_tfths[t]->wri_dynamics[TFTD_COMT].count > 0)
        fdm.generic_decls_add_tft_dslots(t, m_tfths[t]->_varname_dt[TFTD_COMT]);
      
      m_tfths[t]->_location_dt[TFTD_RUNT] = m_tfths[t]->_location_ts = -1;
      if (m_tfths[t]->wri_enable_dynamic_RT)
        fdm.generic_decls_add_tft_tslots(t, m_tfths[t]->_varname_dt[TFTD_RUNT], m_tfths[t]->_varname_ts);
      havetft |= m_tfths[t]->wri_statics.count > 0 || m_tfths[t]->wri_dynamics[TFTD_COMT].count > 0 || m_tfths[t]->wri_enable_dynamic_RT;
    }
  }
  bool haveovl = m_overlaysCount > 0;
  {
    for (unsigned int i=0; i<m_overlaysCount; i++)
    {
      m_overlays[i]._location = -1;
      fdm.generic_decls_add_ovl_input(i, m_overlays[i]._varname);
      _Ovldraw::uniforms_t  uf = m_overlays[i].povl->uniforms();
      m_overlays[i].uf_count = uf.count;
      for (unsigned int j=0; j<uf.count; j++)
      {
        m_overlays[i].uf_arr[j]._location = -1;
        m_overlays[i].uf_arr[j].type = uf.arr[j].type;
        m_overlays[i].uf_arr[j].dataptr = uf.arr[j].dataptr;
        m_overlays[i].uf_arr[j].c_istex = dtIsTexture(m_overlays[i].uf_arr[j].type);
        m_overlays[i].uf_arr[j].c_istexarray = dtIsTextureArray(m_overlays[i].uf_arr[j].type);
        fdm.generic_decls_add_ovl_nameonly(i, j, m_overlays[i].uf_arr[j].type, m_overlays[i].uf_arr[j]._varname);
      }
    }
  }
  
  fdm.generic_main_begin(m_allocatedPortions, m_orient, m_emptycolor, m_overpattern);
  {
    m_pcsh->shfragment_store(fdm);
  }
  
  {
    fdm.generic_main_process_fsp(m_overpattern, m_overpatternOpacity);
  }
  if (haveovl)
  {
    if (m_overlaysCount > 0)
    {
      fdm.generic_main_prepare_ovl();
      for (unsigned int i=0; i<m_overlaysCount; i++)
        fdm.generic_main_process_ovl(m_orient, i, m_overlays[i].driven_id, m_overlays[i].povl->getOmode(), m_overlays[i].orient);
    }
  }
  if (havetft)
  {
    fdm.generic_main_prepare_tft();
    for (unsigned int t=0; t<TFT_HOLDINGS; t++)
    {
      if (m_tfths[t] == nullptr)
        continue;
      int c_total = tftgeterbook_designsperarea(m_tfths[t]->geterbook);
      
      
      tftholding_t::wrilink_t* wri[] = { &m_tfths[t]->wri_statics, &m_tfths[t]->wri_dynamics[TFTD_COMT] };
      for (int w=0; w<2; w++)
      {
        const float c_div = 1.0f/(wri[w]->limit-1);
        for (unsigned int i=0; i<wri[w]->count; i++)
        {
          int dsgid_short = wri[w]->array[i].frag.designid % c_total;
#ifndef BSGLSLOLD
          int areaid =  wri[w]->array[i].frag.designid / c_total;
          tftfraginfo_t tfi = {  int(t), int(m_tfths[t]->geterbook->dbookpagescount),
                                 m_tfths[t]->geterbook->limrows, m_tfths[t]->geterbook->limcols, 
                                 m_tfths[t]->geterbook->design_width, m_tfths[t]->geterbook->design_height, 
                                   m_tfths[t]->geterbook->dbookpages[areaid].designs[dsgid_short].width, 
                                 i*c_div, &wri[w]->array[i].frag
          };
#else
          tftfraginfo_t tfi = {  int(t), 1,
                                 m_tfths[t]->limrows, m_tfths[t]->limcols, 
                                 m_tfths[t]->design_width, m_tfths[t]->design_height, m_tfths[t]->dbookpages.records[dsgid_short].width, 
                                 i*c_div, &writings[i].frag
          };
#endif
          FshDrawMain::GROUPING gpmode = FshDrawMain::GP_OFF;
          if (wri[w]->array[i].grouped)
          {
            gpmode = FshDrawMain::GP_ON;
            if (i + 1 == wri[w]->count)
              gpmode = FshDrawMain::GP_ONCLOSE;
            else if (wri[w]->array[i + 1].grouped == false)
              gpmode = FshDrawMain::GP_ONCLOSE;
          }
          fdm.generic_main_process_tft_sd(w == 0, tfi, gpmode);
        } // for writings count
      } // for _TFT_COUNT
      
      if (m_tfths[t]->wri_enable_dynamic_RT)
      {
        tftbookinfo_t tbi;
        tbi.texid = t;
        tbi.limitcols = m_tfths[t]->geterbook->limcols;
        tbi.limitrows = m_tfths[t]->geterbook->limrows;
        tbi.recordwidth = m_tfths[t]->geterbook->design_width;
        tbi.recordheight = m_tfths[t]->geterbook->design_height;
        fdm.generic_main_process_tft_t(tbi, m_tfths[t]->wri_dynamics[TFTD_RUNT].limit, m_tfths[t]->wri_dynamic_rt_coordination, m_tfths[t]->wri_dynamic_rt_rotateangle);
      }
    }
  }
  
//    qDebug()<<m_pcsh->shaderName()<<" fragment size "<<fsh_written<<" (had"<<m_fshalloc<<")";
  
  fdm.generic_main_end();
  
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
  
  for (unsigned int i=0; i<m_overlaysCount; i++)
  {     
    int fshtResult = m_overlays[i].povl->fshOVCoords(i + 1, m_dataDimmSwitchAB, m_fshmem);
    if (fshtResult <= 0)
      qDebug()<<Q_FUNC_INFO<<"OVL fshOVCoords failure!";
    else
      m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, m_fshmem);
    
#ifdef BSSHADER_DUMP
    fout.write((const char*)m_fshmem);
    fout.write((const char*)"\n\n\n\n");
#endif
    
    int fshcResult = m_overlays[i].povl->fshColor(i + 1, m_fshmem);
    if (fshcResult <= 0)
      qDebug()<<Q_FUNC_INFO<<"OVL fshColor failure!";
    else
      m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,  m_fshmem);
        
#ifdef BSSHADER_DUMP
    fout.write((const char*)m_fshmem);
    fout.write((const char*)"\n\n\n\n");
#endif
  }
  
  m_texturesCount = 0;
  
  if (m_ShaderProgram.link())
  {
    static const char* vd_corresponding_array[] = 
    { 
      "datasampler", "datadimm_a", "datadimm_b", "dataportions", "dataportionsize", "datarange", 
      "scaler_a", "scaler_b", 
      "paletsampler", 
    };
    
    Q_ASSERT(_SF_COUNT == sizeof(vd_corresponding_array)/sizeof(const char*));
    for (int i=0; i<_SF_COUNT; i++)
      m_locationPrimary[i] = m_ShaderProgram.uniformLocation(vd_corresponding_array[i]);
    
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
      if (m_locationSecondary[i].istexture)
        glGenTextures(1, &m_textures[m_texturesCount++]);
    }
    m_locationSecondaryCount = additufm_count;
    
    
    /// 3. Init tft locations and textures
    {
      /// 3.1. Textures for designs
      for (unsigned int t=0; t<TFT_HOLDINGS; t++)
      {
        if (m_tfths[t] == nullptr)  continue;
        glGenTextures(1, &m_textures[m_texturesCount++]);
        m_tfths[t]->_location_h = m_ShaderProgram.uniformLocation(m_tfths[t]->_varname_h);
        m_tfths[t]->ponger_h = 0;
      }
      
      /// 3.2. Variables and textures for writings
      for (unsigned int t=0; t<TFT_HOLDINGS; t++)
      {
        if (m_tfths[t] == nullptr)  continue;
        
        if (m_tfths[t]->wri_dynamics[TFTD_COMT].count == 0)
          m_tfths[t]->_location_dt[TFTD_COMT] = -1;
        else
        {
          glGenTextures(1, &m_textures[m_texturesCount++]);
          m_tfths[t]->_location_dt[TFTD_COMT] = m_ShaderProgram.uniformLocation(m_tfths[t]->_varname_dt[TFTD_COMT]);
        }
        m_tfths[t]->ponger_dt[TFTD_COMT] = 0;
        
        if (m_tfths[t]->wri_enable_dynamic_RT == false)
        {
          m_tfths[t]->_location_dt[TFTD_RUNT] = -1;
          m_tfths[t]->_location_ts = -1;
        }
        else
        {
          glGenTextures(1, &m_textures[m_texturesCount++]);
          m_tfths[t]->_location_dt[TFTD_RUNT] = m_ShaderProgram.uniformLocation(m_tfths[t]->_varname_dt[TFTD_RUNT]);
          m_tfths[t]->_location_ts = m_ShaderProgram.uniformLocation(m_tfths[t]->_varname_ts);
        }
        m_tfths[t]->ponger_dt[TFTD_RUNT] = 0;
      }
    }
    
    /// 4. Init ovl locations and textures
    {
      for (unsigned int i=0; i<m_overlaysCount; i++)
      {
        m_overlays[i]._location = m_ShaderProgram.uniformLocation(m_overlays[i]._varname);  /// ! cannot hide by upcount
        for (unsigned int j=0; j<m_overlays[i].uf_count; j++)
        {
          m_overlays[i].uf_arr[j]._location = m_ShaderProgram.uniformLocation(m_overlays[i].uf_arr[j]._varname);
          if (m_overlays[i].uf_arr[j].c_istex || m_overlays[i].uf_arr[j].c_istexarray)
            glGenTextures(1, &m_textures[m_texturesCount++]);
        }
        m_overlays[i].ponger_reinit = m_overlays[i].ponger_update = 0;
      }
    } // init ovl
    
    unpend(PC_INIT);
    m_bitmaskPendingChanges |= PC_DATA | PC_DATADIMMS | PC_DATAPORTS | PC_DATARANGE | PC_PALETTE | PC_SEC | 
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
  
  m_bitmaskPendingChanges |= PC_DATA | PC_DATADIMMS | PC_DATAPORTS | PC_DATARANGE | PC_PALETTE | PC_SEC | 
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
  
#if 0     /// glReadPixel Only??
  {
    glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    
    glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
#if QT_VERSION >= 0x050000
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
#endif
  }
#endif
  
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
      palettePrepare( m_paletptr, m_paletdiscretise, 
                      m_portionMeshType == ISheiGenerator::PMT_PSEUDO2D && m_allocatedPortions != 0? m_allocatedPortions : 1,
                      m_paletprerange[0], m_paletprerange[1]);
      if (m_clearsource == CS_PALETTE)
        _colorCvt(this->colorBack());
      m_ShaderProgram.setUniformValue(loc, CORR_TEX);
    }
    CORR_TEX++;
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
  
  for (unsigned int t=0; t<TFT_HOLDINGS; t++)
  {
    if (m_tfths[t] == nullptr)
      continue;
    loc = m_tfths[t]->_location_h;
#ifndef BSGLSLOLD
    glActiveTexture(GL_TEXTURE0 + CORR_TEX);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
#else
    glActiveTexture(GL_TEXTURE0 + CORR_TEX);
    glBindTexture(GL_TEXTURE, m_textures[CORR_TEX]);
#endif
  
    if (havePendOn(PC_TFT_TEXTURE, bitmaskPendingChanges) && m_tfths[t]->ponger_h != m_tfths[t]->pinger_h)
    {
      const GLint   gl_internalFormat = GL_RGBA8;
      const GLenum  gl_format = GL_RGBA;
      const GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
//        qDebug()<<m_tfths[t]->ponger<<m_tfths[t]->pinger<<m_tfths[t]->_location;
#ifndef BSGLSLOLD
      int width = m_tfths[t]->geterbook->design_width * m_tfths[t]->geterbook->limcols;
      int height = m_tfths[t]->geterbook->design_height * m_tfths[t]->geterbook->limrows;
      int LAY = m_tfths[t]->geterbook->dbookpagescount;
      glTexStorage3D(  GL_TEXTURE_2D_ARRAY, /*LAY*/1, gl_internalFormat, width, height, LAY);
      for (int r=0; r<LAY; r++)
      {
        const tftdesignedimage_t&  area = m_tfths[t]->geterbook->dbookpages[r];
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, r, width, height, 1, gl_format, gl_texture_type, area.image->constBits());
      }
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, /*LAY*/1);
#else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      const tftdesignedimage_t&  area = m_tfths[t]->dbookpages;
      glTexImage2D(GL_TEXTURE_2D, 0, gl_internalFormat, width, height, 0, gl_format, gl_texture_type, area.image->constBits());
#endif
      m_ShaderProgram.setUniformValue(loc, CORR_TEX);
      
      m_tfths[t]->ponger_h = m_tfths[t]->pinger_h;
    } // if havePend
    CORR_TEX++;
  } // for TFT
  

  {
    float   _memory[2*(TFT_SLOTLIMIT_DYNAMIC_CT > TFT_SLOTLIMIT_DYNAMIC_RT? TFT_SLOTLIMIT_DYNAMIC_CT : TFT_SLOTLIMIT_DYNAMIC_RT)][4];
    for (unsigned int t=0; t<TFT_HOLDINGS; t++)
    {
      if (m_tfths[t] == nullptr)
        continue;
      
      for (unsigned int l=0; l<_TFTD_COUNT; l++)
      {
        if ((loc = m_tfths[t]->_location_dt[l]) != -1)
        {
          glActiveTexture(GL_TEXTURE0 + CORR_TEX);
          glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
          
          if (havePendOn(PC_TFT_PARAMS, bitmaskPendingChanges) && m_tfths[t]->ponger_dt[l] != m_tfths[t]->pinger_dt[l])
          {
            tftholding_t::wrilink_t* wri_dt = &m_tfths[t]->wri_dynamics[l];
            int offset0 = 0*wri_dt->limit;
            int offset1 = 1*wri_dt->limit;
            for (unsigned int i=0; i<wri_dt->count; i++)
            {
              _memory[offset0 + i][0] = wri_dt->array[i].frag.designid;
              _memory[offset0 + i][1] = _memory[offset0 + i][2] = _memory[offset0 + i][3] = 0;
          
              _memory[offset1 + i][0] = wri_dt->array[i].frag.slotdata.fx;
              _memory[offset1 + i][1] = wri_dt->array[i].frag.slotdata.fy;
              _memory[offset1 + i][2] = wri_dt->array[i].frag.slotdata.opacity;
              _memory[offset1 + i][3] = wri_dt->array[i].frag.slotdata.rotate;
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GLint   gl_internalFormat = GL_RGBA32F;
            GLenum  gl_format = GL_RGBA;
            GLenum  gl_texture_type = GL_FLOAT;
            glTexImage2D(GL_TEXTURE_2D, 0, gl_internalFormat, wri_dt->limit, 2, 0, gl_format, gl_texture_type, _memory);
            m_tfths[t]->ponger_dt[l] = m_tfths[t]->pinger_dt[l];
          }
          m_ShaderProgram.setUniformValue(loc, CORR_TEX);
          CORR_TEX++;
        } // if loc
        if (l == TFTD_RUNT)
        {
          if ((loc = m_tfths[t]->_location_ts) != -1)
          {
            m_ShaderProgram.setUniformValue(loc, m_tfths[t]->wri_dynamics[TFTD_RUNT].count);
//            qDebug()<<m_tfths[t]->wri_dynamics[TFTD_RUNT].count;
          }
        }
      }
    } // for
  }
  
  
  if (!havePendOn(PC_PARAMSOVL, bitmaskPendingChanges))
  {
    for (unsigned int o=0; o<m_overlaysCount; o++)
    {
      overlay_t& ovl = m_overlays[o];
      for (unsigned int i=0; i<ovl.uf_count; i++)
      {
        if (ovl.uf_arr[i].c_istex || ovl.uf_arr[i].c_istexarray)
        {
          glActiveTexture(GL_TEXTURE0 + CORR_TEX);
          glBindTexture(ovl.uf_arr[i].c_istex ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
          CORR_TEX++;
        }
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
        
        if ((loc = ovl._location) != -1)
        {
//          m_ShaderProgram.setUniformValue(loc, QVector4D(ovl.povl->isVisible()? ovl.povl->getOpacity() : 1.0f, 
//                                                         ovl.povl->getThickness(), 
//                                                         ovl.povl->getSliceLL(), ovl.povl->getSliceHL()));
          m_ShaderProgram.setUniformValue(loc, ovl.povl->isVisible()? ovl.povl->getOpacity() : 1.0f, 
                                                         ovl.povl->getThickness(), 
                                                         ovl.povl->getSliceLL(), ovl.povl->getSliceHL());
        }
        
        for (unsigned int i=0; i<ovl.uf_count; i++)
        {
          const uniform_located_t& ufm = ovl.uf_arr[i];
          loc = ufm._location;
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
              default: break;
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
                  GLint   gl_internalFormat;
                  GLenum  gl_format;
                  GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
  #if QT_VERSION >= 0x050000
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
                palettePrepare(cdp->ppal, cdp->discrete, 1, cdp->prerange[0], cdp->prerange[1]);
              }
              
              m_ShaderProgram.setUniformValue(loc, CORR_TEX);
              CORR_TEX++;
              break;
            }
            case DT_TEXTURE_2D_ARRAY:
            {
              glActiveTexture(GL_TEXTURE0 + CORR_TEX);
              glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
              
              const dmtype_samplerarray_t* psmparr = (const dmtype_samplerarray_t*)ufm.dataptr;
              const GLint   gl_internalFormat = GL_RGBA8;
              const GLenum  gl_format = GL_RGBA;
              const GLenum  gl_texture_type = GL_UNSIGNED_BYTE;
              int width = psmparr->w;
              int height = psmparr->h;
              
              {
                int LAY = psmparr->layers;
                glTexStorage3D(  GL_TEXTURE_2D_ARRAY, 1/*LAY*/, gl_internalFormat, width, height, LAY);
                for (int r=0; r<LAY; r++)
                  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, r, width, height, 1, gl_format, gl_texture_type, psmparr->data[r]);
//                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, psmparr->linsmooth ? GL_LINEAR : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, psmparr->linsmooth ? GL_LINEAR : GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, /*LAY-*/1);
                
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
//        for (unsigned int i=0; i<ovl.texcount; i++)
//        {
//          glActiveTexture(GL_TEXTURE0 + CORR_TEX);
//          glBindTexture(GL_TEXTURE_2D, m_textures[CORR_TEX]);
//          CORR_TEX++;
//        }
        for (unsigned int i=0; i<ovl.uf_count; i++)
        {
          if (ovl.uf_arr[i].c_istex || ovl.uf_arr[i].c_istexarray)
          {
            glActiveTexture(GL_TEXTURE0 + CORR_TEX);
            glBindTexture(ovl.uf_arr[i].c_istex ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY, m_textures[CORR_TEX]);
            CORR_TEX++;
          }
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
  
//  if (objectName() == "AAA")
//    qDebug()<<"DrawQWidget PRE resizeEv: "<<QSize(w,h)<<m_dataDimmA<<m_dataDimmB<<m_scalingA<<m_scalingB;
  if (m_dataDimmSwitchAB)
    adjustSizeAndScale(h, w);
  else
    adjustSizeAndScale(w, h);
//  if (objectName() == "AAA")
//    qDebug()<<"DrawQWidget resizeEv: "<<m_dataDimmA<<m_dataDimmB<<m_scalingA<<m_scalingB;
  
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
//  qDebug()<<"MEOW"<<this->isInitialized();
//  if (this->isInitialized())
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

void  DrawQWidget::_applyMouseEvents(OVL_REACTION_MOUSE oreact, int x, int y)
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

//  coordstriumv_t ct;
//  ct.fx_pix = fx;
//  ct.fy_pix = fy;
//  if (orientationTransposed(m_orient))
//  {
//    ct.fx_01 = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
//    ct.fy_01 = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
//    ct.fx_01 = singleDimmHeight <=1 ? 0 : (ct.fx_01 - int(ct.fx_01/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
//    ct.fy_01 = singleDimmWidth <= 1? 0 : (ct.fy_01 - int(ct.fy_01/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
//    ct.fx_01 = ct.fx_01;
//    ct.fy_01 = ct.fy_01;
//  }
//  else
//  {
//    ct.fx_01 = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
//    ct.fy_01 = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
//    ct.fx_01 = singleDimmWidth <= 1? 0 : (ct.fx_01 - int(ct.fx_01/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
//    ct.fy_01 = singleDimmHeight <= 1? 0 : (ct.fy_01 - int(ct.fy_01/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
//    ct.fx_01 = ct.fx_01;
//    ct.fy_01 = ct.fy_01;
//  }
  coordstriumv_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    float tx = !orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy;
    tx = tx - int(tx/singleDimmHeight)*singleDimmHeight;
    float ty = orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx;
    ty = ty - int(ty/singleDimmWidth)*singleDimmWidth;
    ct.fx_01  = singleDimmHeight <=1 ? 0 : tx / (singleDimmHeight - 1);
    ct.fy_01  = singleDimmWidth <= 1 ? 0 : ty / (singleDimmWidth - 1);
    ct.fx_0n1 = singleDimmHeight <=1 ? 0 : tx / singleDimmHeight;
    ct.fy_0n1 = singleDimmHeight <=1 ? 0 : ty / singleDimmWidth;
  }
  else
  {
    float tx = orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx;
    tx = tx - int(tx/singleDimmWidth)*singleDimmWidth;
    float ty = !orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy;
    ty = ty - int(ty/singleDimmHeight)*singleDimmHeight;
    ct.fx_01  = singleDimmWidth <= 1? 0 : tx / (singleDimmWidth - 1);
    ct.fy_01  = singleDimmHeight <= 1? 0 : ty / (singleDimmHeight - 1);
    ct.fx_0n1 = singleDimmWidth <= 1? 0 : tx / singleDimmWidth;
    ct.fy_0n1 = singleDimmHeight <= 1? 0 : ty / singleDimmHeight;
  }
  
  bool doStop = false, doUpdate = false;
  if (m_proactive)
    doUpdate |= m_proactive->reactionMouse(this, oreact, &ct, &doStop);
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
    if (!autoUpdateBanned(RD_BYINPUTEVENTS))
      callWidgetUpdate();
  }
}

void  DrawQWidget::_applyMouseTracking(int x, int y)
{
  dcgeometry_t dch = this->geometryHorz();
  dcgeometry_t dcv = this->geometryVert();
  
  float singleDimmWidth = dch.length;
  float singleDimmHeight = dcv.length;
  float totalDimmWidth = singleDimmWidth * (m_dataDimmSwitchAB? m_splitterB : m_splitterA);
  float totalDimmHeight = singleDimmHeight * (m_dataDimmSwitchAB? m_splitterA : m_splitterB);
  
  float fx = (x*c_dpr - (dch.cttr_pre + dch.viewalign_pre));
  float fy = (y*c_dpr - (dcv.cttr_pre + dcv.viewalign_pre));
  
#if 0
  if (isPress[oreact] == false)
  {
    if (fx < 0)  fx = 0; else if (fx >= totalDimmWidth) fx = totalDimmWidth - 1;
    if (fy < 0)  fy = 0; else if (fy >= totalDimmHeight) fy = totalDimmHeight - 1;
  }
  else if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;
#else
  if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;
#endif

  coordstriumv_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    float tx = !orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy;
    tx = tx - int(tx/singleDimmHeight)*singleDimmHeight;
    float ty = orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx;
    ty = ty - int(ty/singleDimmWidth)*singleDimmWidth;
    ct.fx_01  = singleDimmHeight <=1 ? 0 : tx / (singleDimmHeight - 1);
    ct.fy_01  = singleDimmWidth <= 1 ? 0 : ty / (singleDimmWidth - 1);
    ct.fx_0n1 = singleDimmHeight <=1 ? 0 : tx / singleDimmHeight;
    ct.fy_0n1 = singleDimmHeight <=1 ? 0 : ty / singleDimmWidth;
  }
  else
  {
    float tx = orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx;
    tx = tx - int(tx/singleDimmWidth)*singleDimmWidth;
    float ty = !orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy;
    ty = ty - int(ty/singleDimmHeight)*singleDimmHeight;
    ct.fx_01  = singleDimmWidth <= 1? 0 : tx / (singleDimmWidth - 1);
    ct.fy_01  = singleDimmHeight <= 1? 0 : ty / (singleDimmHeight - 1);
    ct.fx_0n1 = singleDimmWidth <= 1? 0 : tx / singleDimmWidth;
    ct.fy_0n1 = singleDimmHeight <= 1? 0 : ty / singleDimmHeight;
  }
  
  bool doStop;
  bool doUpdate = m_proactive->reactionTracking(this, &ct, &doStop);
  if (doUpdate)
  {
    m_bitmaskPendingChanges |= PC_PARAMSOVL;
    if (!autoUpdateBanned(RD_BYINPUTEVENTS))
      callWidgetUpdate();
  }
}



#define BUTTONCHECK_DOGSHIT(action) Qt::MouseButton btn = event->button(); \
                                    QPoint pos = event->pos(); \
                                    if (btn == Qt::LeftButton)        _applyMouseEvents(ORM_LM##action, pos.x(), pos.y()); \
                                    else if (btn == Qt::RightButton)  _applyMouseEvents(ORM_RM##action, pos.x(), pos.y());


void DrawQWidget::mousePressEvent(QMouseEvent *event)
{
  BUTTONCHECK_DOGSHIT(PRESS)
//  if (event->button() == Qt::LeftButton)  _applyMouseEvents(ORM_LMPRESS, event->pos().x(), event->pos().y());
//  else if (event->button() == Qt::RightButton)  _applyMouseEvents(ORM_RMPRESS, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseReleaseEvent(QMouseEvent *event)
{
  BUTTONCHECK_DOGSHIT(RELEASE)
//  if (event->button() == Qt::LeftButton)  _applyMouseEvents(ORM_LMRELEASE, event->pos().x(), event->pos().y());
}

void DrawQWidget::mouseMoveEvent(QMouseEvent *event)
{
  Qt::MouseButtons btn = event->buttons();
  QPoint pos = event->pos();
  if (btn & Qt::LeftButton)        _applyMouseEvents(ORM_LMMOVE, pos.x(), pos.y());
  else if (btn & Qt::RightButton)  _applyMouseEvents(ORM_RMMOVE, pos.x(), pos.y());
  else if (btn == Qt::NoButton && m_proactive != nullptr)
    if (hasMouseTracking())
      _applyMouseTracking(pos.x(), pos.y());
  
//  if (event->buttons() & Qt::LeftButton)  _applyMouseEvents(ORM_LMMOVE, event->pos().x(), event->pos().y());
//  _applyMouseEvents(ORM_LMMOVE, event->pos().x(), event->pos().y());
  //  setMouseTracking(tru!!);
}

void DrawQWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)  _applyMouseEvents(ORM_LMDOUBLE, event->pos().x(), event->pos().y());
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
  
  QPointF pos = event->position();
  float fx = (pos.x()*c_dpr - (dch.cttr_pre + dch.viewalign_pre));
  float fy = (pos.y()*c_dpr - (dcv.cttr_pre + dcv.viewalign_pre));
  if (fx >= totalDimmWidth || fy >= totalDimmHeight)
    return;

  coordstriumv_wheel_t ct;
  ct.fx_pix = fx;
  ct.fy_pix = fy;
  if (orientationTransposed(m_orient))
  {
    ct.fx_01 = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fy_01 = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fx_01 = singleDimmHeight <=1 ? 0 : (ct.fx_01 - int(ct.fx_01/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fy_01 = singleDimmWidth <= 1? 0 : (ct.fy_01 - int(ct.fy_01/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fx_01 = ct.fx_01;
    ct.fy_01 = ct.fy_01;
  }
  else
  {
    ct.fx_01 = (orientationMirroredHorz(m_orient)? totalDimmWidth - 1 - fx : fx);
    ct.fy_01 = (!orientationMirroredVert(m_orient)? totalDimmHeight - 1 - fy : fy);
    ct.fx_01 = singleDimmWidth <= 1? 0 : (ct.fx_01 - int(ct.fx_01/singleDimmWidth)*singleDimmWidth) / (singleDimmWidth - 1);
    ct.fy_01 = singleDimmHeight <= 1? 0 : (ct.fy_01 - int(ct.fy_01/singleDimmHeight)*singleDimmHeight) / (singleDimmHeight - 1);
    ct.fx_01 = ct.fx_01;
    ct.fy_01 = ct.fy_01;
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
    if (!autoUpdateBanned(RD_BYINPUTEVENTS))
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
    if (!autoUpdateBanned(RD_BYINPUTEVENTS))
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

void DrawQWidget::leaveEvent(QEvent* event)
{
  bool doUpdate = false;
  if (m_proactive)
    doUpdate |= m_proactive->reactionLeave(this);
  if (doUpdate)
  {
    m_bitmaskPendingChanges |= PC_PARAMSOVL;
    if (!autoUpdateBanned(RD_BYINPUTEVENTS))
      callWidgetUpdate();
  }
  QOpenGLWidget::leaveEvent(event);
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
#if QT_VERSION >= 0x050000
  QObject::connect(qsb, &QAbstractSlider::valueChanged, this, &DrawQWidget::scrollDataTo);
#else
  QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(scrollDataTo(int)));
#endif
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


int DrawQWidget::_tft_holding_alloc(tftgeterbook_t* th, bool isowner, bool enableDynamicRT)
{
  for (int i=0; i<TFT_HOLDINGS; i++)
    if (m_tfths[i] == nullptr)
    {
      m_tfths[i] = new tftholding_t;
      m_tfths[i]->geterbook = th;
      m_tfths[i]->geterbookowner = isowner;
      
      m_tfths[i]->_location_h = -1;
      m_tfths[i]->pinger_h = 1;
      m_tfths[i]->ponger_h = 1;
      
      m_tfths[i]->wri_statics.array = m_tfths[i]->wri_statics_buffer;
      m_tfths[i]->wri_statics.count = 0;
      m_tfths[i]->wri_statics.limit = sizeof(m_tfths[i]->wri_statics_buffer)/sizeof(m_tfths[i]->wri_statics_buffer[0]);
      
      m_tfths[i]->wri_dynamics[TFTD_COMT].array = m_tfths[i]->wri_dynamics_ct_buffer;
      m_tfths[i]->wri_dynamics[TFTD_COMT].count = 0;
      m_tfths[i]->wri_dynamics[TFTD_COMT].limit = sizeof(m_tfths[i]->wri_dynamics_ct_buffer)/sizeof(m_tfths[i]->wri_dynamics_ct_buffer[0]);
      
      m_tfths[i]->wri_dynamics[TFTD_RUNT].array = m_tfths[i]->wri_dynamics_rt_buffer;
      m_tfths[i]->wri_dynamics[TFTD_RUNT].count = 0;
      m_tfths[i]->wri_dynamics[TFTD_RUNT].limit = sizeof(m_tfths[i]->wri_dynamics_rt_buffer)/sizeof(m_tfths[i]->wri_dynamics_rt_buffer[0]);
      
      for (int j=0; j<_TFTR_COUNT; j++)
        m_tfths[i]->wri_grouping[j] = true;
      m_tfths[i]->wri_enable_dynamic_RT = enableDynamicRT;
          
      for (int d=0; d<_TFTD_COUNT; d++)
      {
        m_tfths[i]->pinger_dt[d] = 1;
        m_tfths[i]->ponger_dt[d] = 0;
        m_tfths[i]->_location_dt[d] = -1;
      }
      m_tfths[i]->_location_ts = -1;
      
      m_tfths[i]->wri_dynamic_rt_coordination = CR_ABSOLUTE_NOSCALED;
      m_tfths[i]->wri_dynamic_rt_rotateangle = 0.0f;
      return i;
    }
  return -1;
}

bool DrawQWidget::_tft_holding_release(int idx)
{
  if (m_tfths[idx] == nullptr)
    return false;
  
  if (m_tfths[idx]->geterbookowner)
    tftgeterbook_release(m_tfths[idx]->geterbook);
  delete m_tfths[idx];
  m_tfths[idx] = nullptr;
  return true;
}

//#define DBGLABELSHOW_1
//#define DBGLABELSHOW_2

#if defined DBGLABELSHOW_1 || defined DBGLABELSHOW_2
#include <QLabel>
#ifdef DBGLABELSHOW_1
QLabel* g_lbl = nullptr;
int     g_ctr = 0;
#endif
#endif

int DrawQWidget::_tft_holding_design(DrawQWidget::tftholding_t* holding, const char* text)
{
  int rid_loc = tftgeterbook_addtext(holding->geterbook, text);
  holding->pinger_h += 1;
  m_bitmaskPendingChanges |= PC_TFT_TEXTURE;
#ifdef DBGLABELSHOW_1
  if (g_lbl == nullptr || g_ctr != holding->geterbook->dbookpagescount)
  {
    g_lbl = new QLabel;
    g_ctr = holding->geterbook->dbookpagescount;
  }
  g_lbl->setPixmap(QPixmap::fromImage(*holding->geterbook->dbookpages[g_ctr-1].image));
  g_lbl->show();
#endif
  return rid_loc;
}

int DrawQWidget::_tft_holding_design(DrawQWidget::tftholding_t* holding, int count, const char* texts[])
{
  int rid_loc = tftgeterbook_addtexts(holding->geterbook, count, texts);
  holding->pinger_h += 1;
  m_bitmaskPendingChanges |= PC_TFT_TEXTURE;
#ifdef DBGLABELSHOW_1
  if (g_lbl == nullptr || g_ctr != holding->geterbook->dbookpagescount)
  {
    g_lbl = new QLabel;
    g_ctr = holding->geterbook->dbookpagescount;
  }
  g_lbl->setPixmap(QPixmap::fromImage(*holding->geterbook->dbookpages[g_ctr-1].image));
  g_lbl->show();
#endif
  return rid_loc;
}

int  DrawQWidget::_tft_add_writing(tftholding_t::wrilink_t* pwri, const DrawQWidget::tftwriting_t& ts, bool grouped)
{
  if (pwri->count >= pwri->limit)
    return -1;
  pwri->array[pwri->count] = ts;
  pwri->array[pwri->count].grouped = grouped;
  int sid = pwri->count++;
  if (ts.frag.designid != -1)
  {
    m_bitmaskPendingChanges |= PC_INIT | PC_TFT_PARAMS; //PC_TFT_TEXTURE | 
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  return sid;
}



int DrawQWidget::tftHoldingRegister(tftgeterbook_t* th, bool isowner, bool enableDynamicRT)
{
  int next_holding = _tft_holding_alloc(th, isowner, enableDynamicRT);
  if (next_holding != -1)
  {
    m_tfth_current = next_holding;
  }
#ifdef DBGLABELSHOW_2
  for (int i=0; i<th->dbookpagescount; i++)
  {
    QLabel* lbl = new QLabel;
    lbl->setPixmap(QPixmap::fromImage(*th->dbookpages[i].image));
    lbl->show();
  }
#endif
  return next_holding;
}

int DrawQWidget::tftHoldingRegister(const QFont& font, int maxtextlen, int limitcolumns, QColor color, bool enableDynamicRT)
{
  return tftHoldingRegister(tftgeterbook_alloc(font, maxtextlen, limitcolumns, color), true, enableDynamicRT);
}

tftgeterbook_t* DrawQWidget::tftHoldingGetbook(int hoid)
{
  if (hoid < 0 || hoid >= TFT_HOLDINGS)
    return nullptr;
  if (m_tfths[hoid] == nullptr)
    return nullptr;
  return m_tfths[hoid]->geterbook;
}

bool DrawQWidget::tftHoldingGetbookOwner(int hoid)
{
  if (hoid < 0 || hoid >= TFT_HOLDINGS)
    return true;
  if (m_tfths[hoid] == nullptr)
    return true;
  return m_tfths[hoid]->geterbookowner;
}

bool DrawQWidget::tftHoldingSwitch(int hoid)
{
  if (hoid < 0 || hoid >= TFT_HOLDINGS)
    return false;
  if (m_tfths[hoid] == nullptr)
    return false;
  m_tfth_current = hoid;
  return true;
}

int DrawQWidget::tftHoldingRelease()
{
  if (_tft_holding_release(m_tfth_current) == true)
  {
    for (int i=0; i<TFT_HOLDINGS; i++)
    {
      int nidx = (m_tfth_current + i) % TFT_HOLDINGS;
      if (m_tfths[nidx] != nullptr)
        m_tfth_current = nidx;
    }
    m_bitmaskPendingChanges |= PC_INIT | PC_TFT_TEXTURE | PC_TFT_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  return m_tfth_current;
}

void DrawQWidget::tftHoldingSetColor(QColor clr)
{
  if (m_tfth_current == -1)
    return;
  tftgeterbook_setup(m_tfths[m_tfth_current]->geterbook, clr);
}

int DrawQWidget::tftAddDesign(const char* text)
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], text);
  return dsgid;
}

int DrawQWidget::tftAddDesigns(int count, const char* texts[])
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], count, texts);
  return dsgid;
}



QSize DrawQWidget::tftHoldingDesignMaxSize() const
{
  if (m_tfth_current == -1)
    return QSize();
  return QSize(m_tfths[m_tfth_current]->geterbook->design_width, m_tfths[m_tfth_current]->geterbook->design_height);
}

int DrawQWidget::tftHoldingDesignMaxWidth() const
{
  if (m_tfth_current == -1)
    return 0;
  return m_tfths[m_tfth_current]->geterbook->design_width;
}

int DrawQWidget::tftHoldingDesignMaxHeight() const
{
  if (m_tfth_current == -1)
    return 0;
  return m_tfths[m_tfth_current]->geterbook->design_height;
}



/////

tftstatic_t DrawQWidget::tftPushStatic(int dsgid, COORDINATION cr, float fx, float fy, float rotate)
{
  return tftPushStatic(dsgid, -1, cr, fx, fy, rotate);
}

tftstatic_t DrawQWidget::tftPushStatic(const char* text, COORDINATION cr, float fx, float fy, float rotate)
{
  return tftPushStatic(text, -1, cr, fx, fy, rotate);
}

tftstatic_t DrawQWidget::tftPushStatic(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy, float rotate)
{
  tftwriting_t ts = { { dsgid, { cr, fx, fy, 0.0f, rotate }, false, ovlroot }, m_tfths[m_tfth_current]->wri_grouping[TFTR_STATIC] };
  {
    tftholding_t::wrilink_t* pwri = &m_tfths[m_tfth_current]->wri_statics;
    if (pwri->count >= pwri->limit)
      return tftstatic_t(m_tfth_current, -1);
    pwri->array[pwri->count++] = ts;
    if (dsgid != -1)
    {
      m_bitmaskPendingChanges |= PC_INIT;
      if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
    }
  }
  return tftstatic_t(m_tfth_current, dsgid);
}

tftstatic_t DrawQWidget::tftPushStatic(const char* text, int ovlroot, COORDINATION cr, float fx, float fy, float rotate)
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], text);
  return tftPushStatic(dsgid, ovlroot, cr, fx, fy, rotate);
}

void DrawQWidget::tftEnableStaticClosestMode()
{
  if (m_tfth_current != -1)
    m_tfths[m_tfth_current]->wri_grouping[TFTR_STATIC] = true;
}

void DrawQWidget::tftDisableStaticClosestMode()
{
  if (m_tfth_current != -1)
    m_tfths[m_tfth_current]->wri_grouping[TFTR_STATIC] = false;
}


/////

tftdynamic_t DrawQWidget::tftPushDynamicFA(int dsgid, COORDINATION cr, float fx, float fy)
{
  return tftPushDynamicFA(dsgid, -1, cr, fx, fy);
}

tftdynamic_t DrawQWidget::tftPushDynamicFA(const char* text, COORDINATION cr, float fx, float fy)
{
  return tftPushDynamicFA(text, -1, cr, fx, fy);
}

tftdynamic_t DrawQWidget::tftPushDynamicFA(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy)
{
  tftwriting_t ts = { { dsgid, { cr, fx, fy, 0.0f, 0.0f }, true, ovlroot }, m_tfths[m_tfth_current]->wri_grouping[TFTR_DYNAMIC] };
  int sloid = -1;
  {
    tftholding_t::wrilink_t* pwri = &m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT];
    if (pwri->count < pwri->limit)
    {
      pwri->array[pwri->count] = ts;
      sloid = pwri->count++;
      if (ts.frag.designid != -1)
      {
        m_tfths[m_tfth_current]->pinger_dt[TFTD_COMT] += 1;
        m_bitmaskPendingChanges |= PC_INIT | PC_TFT_PARAMS;
        if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
      }
    }
  }
  return tftdynamic_t(this, m_tfth_current, TFTD_COMT, sloid);
}

tftdynamic_t DrawQWidget::tftPushDynamicFA(const char* text, int ovlroot, COORDINATION cr, float fx, float fy)
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], text);
  return tftPushDynamicFA(dsgid, ovlroot, cr, fx, fy);
}


tftdynamic_t DrawQWidget::tftPushDynamicDA(int dsgid, COORDINATION cr, float fx, float fy, float rotate)
{
  return tftPushDynamicDA(dsgid, -1, cr, fx, fy, rotate);
}

tftdynamic_t DrawQWidget::tftPushDynamicDA(const char* text, COORDINATION cr, float fx, float fy, float rotate)
{
  return tftPushDynamicDA(text, -1, cr, fx, fy, rotate);
}

tftdynamic_t DrawQWidget::tftPushDynamicDA(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy, float rotate)
{
  tftwriting_t ts = { { dsgid, { cr, fx, fy, 0.0f, rotate }, false, ovlroot }, m_tfths[m_tfth_current]->wri_grouping[TFTR_DYNAMIC] };
  int sloid = -1;
  {
    tftholding_t::wrilink_t* pwri = &m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT];
    if (pwri->count < pwri->limit)
    {
      pwri->array[pwri->count] = ts;
      sloid = pwri->count++;
      if (ts.frag.designid != -1)
      {
        m_tfths[m_tfth_current]->pinger_dt[TFTD_COMT] += 1;
        m_bitmaskPendingChanges |= PC_INIT | PC_TFT_PARAMS;
        if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
      }
    }
  }
  return tftdynamic_t(this, m_tfth_current, TFTD_COMT, sloid);
}

tftdynamic_t DrawQWidget::tftPushDynamicDA(const char* text, int ovlroot, COORDINATION cr, float fx, float fy, float rotate)
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], text);
  return tftPushDynamicDA(dsgid, ovlroot, cr, fx, fy, rotate);
}

void DrawQWidget::tftEnableDynamicClosestMode()
{
  if (m_tfth_current != -1)
    m_tfths[m_tfth_current]->wri_grouping[TFTD_COMT] = true;
}

void DrawQWidget::tftDisableDynamicClosestMode()
{
  if (m_tfth_current != -1)
    m_tfths[m_tfth_current]->wri_grouping[TFTD_COMT] = false;
}

////////



void DrawQWidget::tftSetHoldingDynamicRTCoordination(COORDINATION cr)
{
  if (m_tfth_current == -1)
    return;
  m_tfths[m_tfth_current]->wri_dynamic_rt_coordination = cr;
}

COORDINATION DrawQWidget::tftGetHoldingDynamicRTCoordination() const
{
  if (m_tfth_current == -1)
    return CR_SAME;
  return m_tfths[m_tfth_current]->wri_dynamic_rt_coordination;
}

void DrawQWidget::tftSetHoldingDynamicRTRotateAngle(float angle)
{
  if (m_tfth_current == -1)
    return;
  m_tfths[m_tfth_current]->wri_dynamic_rt_rotateangle = angle;
}

float DrawQWidget::tfGetHoldingDynamicRTRotateAngle() const
{
  if (m_tfth_current == -1)
    return CR_SAME;
  return m_tfths[m_tfth_current]->wri_dynamic_rt_rotateangle;
}

tftdynamic_t DrawQWidget::tftPushDynamicRT(int dsgid, float fx, float fy, float opacity)
{
  tftwriting_t ts = { { dsgid, { CR_SAME, fx, fy, opacity, 0.0f }, true, -1 }, true };
  int sloid = -1;
  {
    tftholding_t::wrilink_t* pwri = &m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT];
    if (pwri->count < pwri->limit)
    {
      pwri->array[pwri->count] = ts;
      sloid = pwri->count++;
      if (ts.frag.designid != -1)
      {
        m_tfths[m_tfth_current]->pinger_dt[TFTD_RUNT] += 1;
        m_bitmaskPendingChanges |= PC_TFT_PARAMS;
        if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
      }
    }
  }
  return tftdynamic_t(this, m_tfth_current, TFTD_RUNT, sloid);
}

tftdynamic_t DrawQWidget::tftPushDynamicRT(const char* text, float fx, float fy, float opacity)
{
  if (m_tfth_current == -1)
    m_tfth_current = _tft_holding_alloc(tftgeterbook_alloc(font()), true, TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int dsgid = _tft_holding_design(m_tfths[m_tfth_current], text);
  return tftPushDynamicRT(dsgid, fx, fy, opacity);
}

int DrawQWidget::tftDesignsCount() const
{
  if (m_tfth_current == -1) return 0;
  return tftgeterbook_designscount(m_tfths[m_tfth_current]->geterbook);
}

int DrawQWidget::tftDynamicsCTCount() const
{
  if (m_tfth_current == -1) return 0;
  return m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].count;
}

int DrawQWidget::tftDynamicsRTCount() const
{
  if (m_tfth_current == -1) return 0;
  return m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].count;
}

//int DrawQWidget::tftDesignsPerArea() const
//{
//  if (m_tfth_current == -1) return 0;
//  return tftgeterbook_designsperarea(m_tfths[m_tfth_current]->geterbook);
//}

#define TFT_CHECK_HOID(hoid, badret)      if (hoid < 0 || hoid >= TFT_HOLDINGS) return badret;

#define TFT_CHECK_SHORT(hoid, badret)     if (hoid == -1) return badret; \
                                          if (sloid >= int(m_tfths[hoid]->wri_dynamics[wriid].count)) \
                                            return badret;

#define TFT_CHECK_FULL(hoid, wriid, sloid, badret) \
                                          Q_ASSERT(hoid >= 0 && hoid < TFT_HOLDINGS); \
                                          if (m_tfths[hoid] == nullptr/* || wriid == -1*/) return badret; \
                                          if (sloid < 0 || sloid >= int(m_tfths[hoid]->wri_dynamics[wriid].count)) \
                                            return badret;


tftdynamic_t DrawQWidget::tftGetDynamicCT(int sloid)
{
  return tftdynamic_t(this, m_tfth_current, TFTD_COMT, sloid);
}

tftdynamic_t DrawQWidget::tftGetDynamicRT(int sloid)
{
  return tftdynamic_t(this, m_tfth_current, TFTD_RUNT, sloid);
}

int DrawQWidget::tftDesignIndex(const tftdynamic_t& sp) const
{
  return m_tfths[sp.hoid]->wri_dynamics[sp.wriid].array[sp.sloid].frag.designid;
}

int DrawQWidget::tftDesignIndex(int sloid) const
{
  int wriid = TFTD_COMT;
  TFT_CHECK_SHORT(m_tfth_current, -1)
  return m_tfths[m_tfth_current]->wri_dynamics[wriid].array[sloid].frag.designid;
}

const char* DrawQWidget::tftText(const tftdynamic_t& sp) const
{
  int dsgid = m_tfths[sp.hoid]->wri_dynamics[sp.wriid].array[sp.sloid].frag.designid;
  if (dsgid == -1)
    return nullptr;
  return tftgeterbook_recordtext(m_tfths[sp.hoid]->geterbook, dsgid);
}

const char* DrawQWidget::tftText(int sloid) const
{
  int wriid = TFTD_COMT;
  TFT_CHECK_SHORT(m_tfth_current, nullptr)
  int dsgid = m_tfths[m_tfth_current]->wri_dynamics[wriid].array[sloid].frag.designid;
  if (dsgid == -1)
    return nullptr;
  return tftgeterbook_recordtext(m_tfths[m_tfth_current]->geterbook, dsgid);
}

//int DrawQWidget::tftRecordForSlot(int hoid, int sloid) const
//{
//  Q_ASSERT(hoid < TFT_HOLDINGS);
//  if (m_tfths[hoid] == nullptr)
//    return -1;
//  if (sloid >= m_tfths[hoid]->tftslots.size())
//    return -1;
//  return m_tfths[hoid]->tftslots[sloid].dsgid;
//}

#define TFT_GOTO_SLOT_PINGUP(hoid, wriid)  \
                                m_tfths[hoid]->pinger_dt[wriid] += 1; \
                                { \
                                  m_bitmaskPendingChanges |= PC_TFT_PARAMS; \
                                  if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); \
                                } \
                                return true;



tftdynamic_t DrawQWidget::tftDCT(int sloid)
{
  return tftdynamic_t(this, m_tfth_current, TFTD_COMT, sloid);
}

tftdynamic_t DrawQWidget::tftDynamicCT(int sloid)
{ 
  return tftdynamic_t(this, m_tfth_current, TFTD_COMT, sloid);
}


bool DrawQWidget::tftDynamicCTMove(int sloid, float fx, float fy)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fx = fx;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTMoveX(int sloid, float fx)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fx = fx;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTMoveY(int sloid, float fy)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTRotate(int sloid, float anglerad)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.rotate = anglerad;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTOpacity(int sloid, float opacity)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTSwitchTo(int sloid, int dsgid)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.designid = dsgid;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}

bool DrawQWidget::tftDynamicCTSetup(int sloid, int dsgid, float fx, float fy, float opacity)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.designid = dsgid;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fx = fx;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.fy = fy;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_COMT)
}




tftdynamic_t DrawQWidget::tftDRT(int sloid)
{
  return tftdynamic_t(this, m_tfth_current, TFTD_RUNT, sloid);
}

tftdynamic_t DrawQWidget::tftDynamicRT(int sloid)
{
  return tftdynamic_t(this, m_tfth_current, TFTD_RUNT, sloid);
}

bool DrawQWidget::tftDynamicRTMove(int sloid, float fx, float fy)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fx = fx;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}

bool DrawQWidget::tftDynamicRTMoveX(int sloid, float fx)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fx = fx;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}

bool DrawQWidget::tftDynamicRTMoveY(int sloid, float fy)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}

//bool DrawQWidget::tftDynamicRTRotate(int sloid, float anglerad)
//{
//  TFT_CHECK_FULL(m_tfth_current, sloid, TFTD_RUNT, false)
//  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.rotate = anglerad;
//  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
//}

bool DrawQWidget::tftDynamicRTOpacity(int sloid, float opacity)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}

bool DrawQWidget::tftDynamicRTSwitchTo(int sloid, int dsgid)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.designid = dsgid;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}

bool DrawQWidget::tftDynamicRTSetup(int sloid, int dsgid, float fx, float fy, float opacity)
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, false)
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.designid = dsgid;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fx = fx;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.fy = fy;
  m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(m_tfth_current, TFTD_RUNT)
}


////////////

int DrawQWidget::tftGetDesignWidth(int dsgid) const
{
  TFT_CHECK_HOID(m_tfth_current, 0)
  int c_total = m_tfths[m_tfth_current]->geterbook->c_total;
  int dsgid_short = dsgid % c_total;
  int areaid =  dsgid / c_total;
  return m_tfths[m_tfth_current]->geterbook->dbookpages[areaid].designs[dsgid_short].width;
}

int DrawQWidget::tftGetDesignHeight(int /*dsgid*/) const
{
  TFT_CHECK_HOID(m_tfth_current, 0)
  return m_tfths[m_tfth_current]->geterbook->design_height;
}

QSize DrawQWidget::tftGetDesignSize(int dsgid) const
{
  return QSize(tftGetDesignWidth(dsgid), tftGetDesignHeight(dsgid));
}

int DrawQWidget::tftGetDynamicWidth(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, 0)
  return tftGetDesignWidth(m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.designid);
}

int DrawQWidget::tftGetDynamicHeight(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, 0)
  return tftGetDesignHeight(m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.designid);
}

QSize DrawQWidget::tftGetDynamicSize(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_COMT, sloid, QSize(0,0))
  return tftGetDesignSize(m_tfths[m_tfth_current]->wri_dynamics[TFTD_COMT].array[sloid].frag.designid);
}

int DrawQWidget::tftGetDynamicRTWidth(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, 0)
  return tftGetDesignWidth(m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.designid);
}

int DrawQWidget::tftGetDynamicRTHeight(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, 0)
  return tftGetDesignHeight(m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.designid);
}

QSize DrawQWidget::tftGetDynamicRTSize(int sloid) const
{
  TFT_CHECK_FULL(m_tfth_current, TFTD_RUNT, sloid, QSize(0,0))
      return tftGetDesignSize(m_tfths[m_tfth_current]->wri_dynamics[TFTD_RUNT].array[sloid].frag.designid);
}




bool DrawQWidget::tftMove(const tftdynamic_t& d, float fx, float fy)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fx = fx;
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftMoveX(const tftdynamic_t& d, float fx)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fx = fx;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftMoveY(const tftdynamic_t& d, float fy)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fy = fy;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftRotate(const tftdynamic_t &d, float anglerad)
{
  if (d.wriid == TFTD_RUNT)
    return false;
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.rotate = anglerad;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftSetOpacity(const tftdynamic_t &d, float opacity)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftSwitchTo(const tftdynamic_t& d, int dsgid)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.designid = dsgid;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

bool DrawQWidget::tftSetup(const tftdynamic_t& d, int dsgid, float fx, float fy, float opacity)
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, false)
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.designid = dsgid;
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fx = fx;
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.fy = fy;
  m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.slotdata.opacity = opacity;
  TFT_GOTO_SLOT_PINGUP(d.hoid, d.wriid)
}

int DrawQWidget::tftGetWidth(const tftdynamic_t& d) const
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, 0)
  return tftGetDesignWidth(m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.designid);
}

int DrawQWidget::tftGetHeight(const tftdynamic_t& d) const
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, 0)
  return tftGetDesignHeight(m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.designid);
}

QSize DrawQWidget::tftGetSize(const tftdynamic_t& d) const
{
  TFT_CHECK_FULL(d.hoid, d.wriid, d.sloid, QSize())
  return tftGetDesignSize(m_tfths[d.hoid]->wri_dynamics[d.wriid].array[d.sloid].frag.designid);
}




bool tftdynamic_t::move(float fx, float fy){ return pdraw->tftMove(*this, fx, fy); }
bool tftdynamic_t::move_x(float fx){ return pdraw->tftMoveX(*this, fx); }
bool tftdynamic_t::move_y(float fy){ return pdraw->tftMoveY(*this, fy); }
bool tftdynamic_t::rotate(float anglerad){ return pdraw->tftRotate(*this, anglerad); }
bool tftdynamic_t::setopacity(float opacity){ return pdraw->tftSetOpacity(*this, opacity); }
bool tftdynamic_t::switchto(int dsgid){ return pdraw->tftSwitchTo(*this, dsgid); }
bool tftdynamic_t::setup(int dsgid, float fx, float fy, float opacity){ return pdraw->tftSetup(*this, dsgid, fx, fy, opacity); }
int tftdynamic_t::width() const { return pdraw->tftGetWidth(*this); }
int tftdynamic_t::height() const { return pdraw->tftGetHeight(*this); }
QSize tftdynamic_t::size() const { return pdraw->tftGetSize(*this); }

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
  if (oreact == ORM_LMPRESS || oreact == ORM_RMPRESS)
  {
    emit clicked(ct->fx_01, ct->fy_01);
    return true;
  }
  return false;
}

BSQClickerXY_WR::BSQClickerXY_WR(QObject* parent): QObject(parent)
{
}

bool BSQClickerXY_WR::reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == ORM_LMPRESS)
  {
    emit clicked(ct->fx_01, ct->fy_01);
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
    emit active(QPointF(ct->fx_01, ct->fy_01));
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




BSQTrackerXY::BSQTrackerXY(QObject* parent)
{
  
}

bool BSQTrackerXY::reactionTracking(DrawQWidget*, const coordstriumv_t* ct, bool*)
{
  emit tracked(ct->fx_0n1, ct->fy_0n1, ct->fx_pix, ct->fy_pix);
  return false; 
}

