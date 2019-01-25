#include "bstextstatic.h"
#include "../core/sheigen/bsshgentrace.h"

#include <QImage>
#include <QApplication>
#include <QPainter>

#if QT_VERSION >= 0x050000
#define QTCOMPAT_FORMAT QImage::Format_Alpha8
#elif QT_VERSION >= 0x040000
#define QTCOMPAT_FORMAT QImage::Format_ARGB32
#endif


OSimpleImage::OSimpleImage(COORDINATION cn, float xpos, float ypos, bool rectangled, const linestyle_t& kls): 
  IOverlayTraced(kls), 
  OVLCoordsDynamic(cn, xpos, ypos),
  OVLDimmsStatic(CR_ABSOLUTE),
  m_pImage(nullptr), m_rectangled(rectangled)
{
  appendUniform(DT_TEXT, &dmti);
}

OSimpleImage::OSimpleImage(OVLCoordsStatic* pcoords, float offset_x, float offset_y, bool rectangled, const linestyle_t& kls):
  IOverlayTraced(kls), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsStatic(CR_ABSOLUTE),
  m_pImage(nullptr), m_rectangled(rectangled)
{
  appendUniform(DT_TEXT, &dmti);
}

OSimpleImage::~OSimpleImage()
{
  if (m_pImage)
    delete m_pImage;
}

int OSimpleImage::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_const_fixed("rect_size", (int)dmti.w, (int)dmti.h);
    
    ocg.inside_begin2("rect_size");
    {
      ocg.push( "vec2  tcoords = inormed/vec2(rect_size.x-1, rect_size.y-1);"
                "float textpixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, 1 - tcoords.y)).a;"
                "result += vec3(inside*textpixel, 0, 1);"
                );
      if (m_rectangled)
      {
        ocg.trace_rect_xywh_begin("rect_size");
        ocg.trace_rect_xywh_end("rect_size");
      }
    }
    ocg.inside_end();
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////////////////



OTextStatic::OTextStatic(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, bool rectangled, const linestyle_t& kls): OSimpleImage(cn, xpos, ypos, rectangled, kls)
{
  QFont f1(QApplication::font());
  f1.setPointSize(fontSize);
  innerSetText(text, f1);
}

OTextStatic::OTextStatic(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize, bool rectangled, const linestyle_t& kls): OSimpleImage(pcoords, offset_x, offset_y, rectangled, kls)
{
  QFont f1(QApplication::font());
  f1.setPointSize(fontSize);
  innerSetText(text, f1);
}

OTextStatic::OTextStatic(const char* text, COORDINATION cn, float xpos, float ypos, const QFont& font, bool rectangled, const linestyle_t& kls): OSimpleImage(cn, xpos, ypos, rectangled, kls)
{
  innerSetText(text, font);
}

OTextStatic::OTextStatic(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont& font, bool rectangled, const linestyle_t& kls): OSimpleImage(pcoords, offset_x, offset_y, rectangled, kls)
{
  innerSetText(text, font);
}

void  OTextStatic::setText(const char* text, unsigned int fontSize, bool rectangled)
{
  m_rectangled = rectangled;
  QFont f1(QApplication::font());
  f1.setPointSize(fontSize);
  innerSetText(text, f1);
  updatePublic();
}

void  OTextStatic::setText(const char* text, const QFont& font, bool rectangled)
{
  m_rectangled = rectangled;  
  innerSetText(text, font);
  updatePublic();
}

void  OTextStatic::innerSetText(const char* text, const QFont &font)
{
  unsigned int fs = font.pointSize();
  unsigned int textlen = strlen(text);
  unsigned int height = fs*1.3f + 2;
  unsigned int width = 2;
  if (m_pImage)
    delete m_pImage;
  if (textlen == 0)
  {
    m_pImage = new QImage(width, height, QTCOMPAT_FORMAT);
  }
  else
  {
    width = (textlen*fs)*0.69 + ((textlen-1)*4)*0.85 + 5;
//    width = (textlen*fs)*0.74 + ((textlen-1)*4)*0.8 + 5;
    m_pImage = new QImage(width, height, QTCOMPAT_FORMAT);
    {
      m_pImage->fill(0);
      QPainter ptr(m_pImage);
      ptr.setFont(font);
      ptr.drawText(2, fs + 2, text);
    }
  }
  
  laterInitDimms(0, height, width, 0);
  
  dmti.w = width;
  dmti.h = height;
  dmti.data = m_pImage->constBits();
}



//////////////////////////////////////////
/*
OTextNumeration::OTextNumeration(OTextNumeration::DIRECTION dn, float start, float step, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, const linestyle_t &kls, bool skipfirst):
  OSimpleImage(cn, xpos, ypos, false, kls),
  m_dn(dn), m_start(start), m_step(step), m_skipfirst(skipfirst)
{
  innerSetText(fontSize);
}

OTextNumeration::OTextNumeration(OTextNumeration::DIRECTION dn, float start, float step, OVLCoordsStatic *pcoords, float offset_x, float offset_y, unsigned int fontSize, const linestyle_t &kls, bool skipfirst): 
  OSimpleImage(pcoords, offset_x, offset_y, false, kls),
  m_dn(dn), m_start(start), m_step(step), m_skipfirst(skipfirst)
{
  innerSetText(fontSize);
}

void OTextNumeration::innerSetText(unsigned int fontSize)
{
  if (m_pImage)
    delete m_pImage;
  
  unsigned int count = 5, precision = 4;
  float    foffs = 30;
  
  unsigned int textlen = precision;
  unsigned int height = fontSize*1.3f + 2;
  unsigned int width = count*foffs + (textlen*fontSize)*0.75 + ((textlen-1)*4)*0.85 + 5;
//  bool         symmetric = m_dn == DN_HORZ || m_dn == DN_VERT? true : false; 
  m_pImage = new QImage(width, height, QTCOMPAT_FORMAT);
  {
    m_pImage->fill(0);
    QPainter ptr(m_pImage);
    ptr.setFont(QApplication::font());
    
    if (m_dn == DN_HORZ || m_dn == DN_HORZ_LEFT || m_dn == DN_HORZ_RIGHT)
    {
      for (unsigned int i=0; i<count; i++)
      {
        float newnumber = m_start + m_step*i;
        QString strnumber = QString::number(newnumber);
        unsigned int size = strnumber.length();
        ptr.drawText(1 + foffs*i - ((size*fontSize)*0.75 + ((size-1)*4)*0.85 + 5) / 2,
                     fontSize + 1, strnumber);
      }
    }
    else if (m_dn == DN_VERT || m_dn == DN_VERT_TOP || m_dn == DN_VERT_BOTTOM)
    {
      
    }
      
  }
  laterInitDimms(0, height, width, 0);
  dmti.w = width;
  dmti.h = height;
  dmti.data = m_pImage->constBits();
}

*/
