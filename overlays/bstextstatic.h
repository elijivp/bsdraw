#ifndef IMAGE_H
#define IMAGE_H

/// Overlays:   static text, not optimal, (no texture atlas)
///   OTextTraced: with traced algo for frame
///   OTextColored: simple colored
///   OTextPaletted: colorized by palette
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

class QImage;
class QFont;


struct otextopts_t
{
  const char* text;
  int         flags;
  int mnLeft, mnRight, mnTop, mnBottom;
  otextopts_t(const char* statictext, int alignflags=0, int left=2, int top=2, int right=2, int bottom=2):
  text(statictext), flags(alignflags), mnLeft(left), mnRight(right), mnTop(top), mnBottom(bottom) {}
};

class OITextStatic
{
protected:
  OvldrawUpdater m_ovllink;
  dmtype_image_t  dmti;
  QImage*         m_pImage;
  OITextStatic(Ovldraw* ovllink, const char* text, unsigned int fontSize);
  OITextStatic(Ovldraw* ovllink, const char* text, const QFont& font);
  OITextStatic(Ovldraw* ovllink, const otextopts_t& text, unsigned int fontSize);
  OITextStatic(Ovldraw* ovllink, const otextopts_t& text, const QFont& font);
public:
  virtual       ~OITextStatic();
public:
  void          setText(const char* text, unsigned int fontSize);
  void          setText(const char* text, const QFont& font);
protected:
  void          innerSetText(const otextopts_t& ot, const QFont& font);
};

class OTextTraced: public Ovldraw_ColorTraced, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos,
              unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
              unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos,
              const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
              const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const otextopts_t&, COORDINATION cn, float xpos, float ypos,
              const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const otextopts_t&, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
              const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
  bool m_rectangled;
};

class OTextColored: public Ovldraw_ColorForegoing, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextColored(const char* text, COORDINATION cn, float xpos, float ypos,
               unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
               unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, COORDINATION cn, float xpos, float ypos,
               const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
               const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  
  OTextColored(const otextopts_t&, COORDINATION cn, float xpos, float ypos,
               unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const otextopts_t&, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
               unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const otextopts_t&, COORDINATION cn, float xpos, float ypos,
               const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const otextopts_t&, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
               const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
  int m_argbTextColor, m_argbBackColor, m_argbBorderColor;
};


class OTextPaletted: public Ovldraw_ColorThroughPalette, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos,
                unsigned int fontSize, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y,
                unsigned int fontSize,                 bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos,
                const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, 
                const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const otextopts_t&, COORDINATION cn, float xpos, float ypos,
                const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const otextopts_t&, OVLCoordsStatic* pcoords, float offset_x, float offset_y, 
                const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
  bool m_rectangled;
};




//class OTextNumeration: public OSimpleImage
//{
//public:
//  enum DIRECTION  { DN_HORZ_RIGHT, DN_HORZ_LEFT, DN_HORZ,
//                    DN_VERT_TOP, DN_VERT_BOTTOM, DN_VERT };
//  OTextNumeration(DIRECTION dn, float start, float step, COORDINATION cn, float xpos, float ypos, unsigned int fontSize=12, const linestyle_t& kls = linestyle_solid(1,1,1), bool skipfirst=false);
//  OTextNumeration(DIRECTION dn, float start, float step, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize=12, const linestyle_t& kls = linestyle_solid(1,1,1), bool skipfirst=false);
//protected:
//  void          innerSetText(unsigned int fontSize);
//protected:
//  DIRECTION       m_dn;
//  float           m_start, m_step;
//  bool            m_skipfirst;
//};

#endif // IMAGE_H

