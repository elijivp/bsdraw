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


OITextStatic::OITextStatic(IOverlay* ovllink, const char* text, unsigned int fontSize): m_ovllink(ovllink), m_pImage(nullptr)
{
  m_ovllink.appendUniform(DT_TEXT, &dmti);
  
  QFont f1(QApplication::font());
  f1.setPointSize(fontSize);
  innerSetText(text, f1);
}

OITextStatic::OITextStatic(IOverlay* ovllink, const char* text, const QFont& font): m_ovllink(ovllink), m_pImage(nullptr)
{
  m_ovllink.appendUniform(DT_TEXT, &dmti);
  innerSetText(text, font);
}

OITextStatic::~OITextStatic()
{
  if (m_pImage)
    delete m_pImage;
}


void  OITextStatic::setText(const char* text, unsigned int fontSize)
{
  QFont f1(QApplication::font());
  f1.setPointSize(fontSize);
  innerSetText(text, f1);
  m_ovllink.updatePublic();
}

void  OITextStatic::setText(const char* text, const QFont& font)
{
  innerSetText(text, font);
  m_ovllink.updatePublic();
}

void  OITextStatic::innerSetText(const char* text, const QFont &font)
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
  
//  laterInitDimms(0, height, width, 0);
  
  dmti.w = width;
  dmti.h = height;
  dmti.data = m_pImage->constBits();
}

///////////////////////////////////////////////////////////////


OTextTraced::OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, bool rectangled, const linestyle_t& kls): 
  IOverlayTraced(kls), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_rectangled(rectangled)
{
}

OTextTraced::OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize, bool rectangled, const linestyle_t& kls): 
  IOverlayTraced(kls), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_rectangled(rectangled)
{
}

OTextTraced::OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos, const QFont& font, bool rectangled, const linestyle_t& kls): 
  IOverlayTraced(kls), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_rectangled(rectangled)
{
}

OTextTraced::OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont& font, bool rectangled, const linestyle_t& kls):
  IOverlayTraced(kls), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_rectangled(rectangled)
{
}

int OTextTraced::fshTrace(int overlay, char *to) const
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

OTextColored::OTextColored(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, int argbTextColor, int argbBackColor, int argbBorderColor): 
  IOverlaySimple(), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_argbTextColor(argbTextColor), m_argbBackColor(argbBackColor), m_argbBorderColor(argbBorderColor)
{
}

OTextColored::OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize, int argbTextColor, int argbBackColor, int argbBorderColor): 
  IOverlaySimple(), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_argbTextColor(argbTextColor), m_argbBackColor(argbBackColor), m_argbBorderColor(argbBorderColor)
{
}

OTextColored::OTextColored(const char* text, COORDINATION cn, float xpos, float ypos, const QFont& font, int argbTextColor, int argbBackColor, int argbBorderColor): 
  IOverlaySimple(), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_argbTextColor(argbTextColor), m_argbBackColor(argbBackColor), m_argbBorderColor(argbBorderColor)
{
}

OTextColored::OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont& font, int argbTextColor, int argbBackColor, int argbBorderColor):
  IOverlaySimple(), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_argbTextColor(argbTextColor), m_argbBackColor(argbBackColor), m_argbBorderColor(argbBorderColor)
{
}

int OTextColored::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_const_fixed("rect_size", (int)dmti.w, (int)dmti.h);
    float clr[4];
    bsintTocolor4f(m_argbTextColor, clr);
    ocg.var_const_fixed("clrtext", clr[1], clr[2], clr[3], clr[0]);
    bsintTocolor4f(m_argbBackColor, clr);
    ocg.var_const_fixed("clrquad", clr[1], clr[2], clr[3], clr[0]);
    bsintTocolor4f(m_argbBorderColor, clr);
    ocg.var_const_fixed("clrbord", clr[1], clr[2], clr[3], clr[0]);
    
    ocg.push(
          "ivec2 inside = ivec2(step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1-step(rect_size.x, float(inormed.x)))*(1-step(rect_size.y, float(inormed.y))),"
          "                     step(1.0,float(inormed.x))*step(1.0,float(inormed.y))*(1-step(rect_size.x-1, float(inormed.x)))*(1-step(rect_size.y-1, float(inormed.y))));"
          
          );
//    ocg.inside_begin2("rect_size");
    {
      ocg.push( "vec2  tcoords = inormed/vec2(rect_size.x-1, rect_size.y-1);"
                "float textpixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, 1 - tcoords.y)).a;"
                                                                          
                "result = mix(result, clrquad.rgb, (1.0 - clrquad.a)*inside[1]);"
                "mixwell = mix(mixwell, (1.0 - clrquad.a), inside[1]);"

                "result = mix(result, clrtext.rgb, (1.0 - clrtext.a)*textpixel*inside[1]);"
                "mixwell = mix(mixwell, 1.0 - clrtext.a, textpixel*inside[1]);"

                "result = mix(result, clrbord.rgb, inside[0]*(1.0-inside[1]));"
                "mixwell = mix(mixwell, 1.0 - clrbord.a, inside[0]*(1.0-inside[1]));"
                );
    }
//    ocg.inside_end();
  }
  ocg.goto_func_end(false);
  return ocg.written();
}


//////////////////////////////////////////

OTextPaletted::OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, bool rectangled, const IPalette* ipal, bool discrete): 
  IOverlayHard(ipal, discrete), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_rectangled(rectangled)
{
}

OTextPaletted::OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize, bool rectangled, const IPalette* ipal, bool discrete): 
  IOverlayHard(ipal, discrete), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, fontSize),
  m_rectangled(rectangled)
{
}

OTextPaletted::OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos, const QFont& font, bool rectangled, const IPalette* ipal, bool discrete): 
  IOverlayHard(ipal, discrete), OVLCoordsDynamic(cn, xpos, ypos), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_rectangled(rectangled)
{
}

OTextPaletted::OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont& font, bool rectangled, const IPalette* ipal, bool discrete):
  IOverlayHard(ipal, discrete), OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimmsStatic(CR_ABSOLUTE), OITextStatic(this, text, font),
  m_rectangled(rectangled)
{
}

int OTextPaletted::fshTrace(int overlay, char* to) const
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
                "float textmixwell = inside*textpixel;"
                );
      if (m_rectangled)
      {
        ocg.trace_rect_xywh_begin("rect_size");
        ocg.trace_rect_xywh_end("rect_size");
        ocg.push("mixwell = result[0] + textmixwell;");
      }
      else
        ocg.push("mixwell = textmixwell;");
    }
    ocg.inside_end();
  }
  ocg.goto_func_end(false);
  return ocg.written();
}












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


