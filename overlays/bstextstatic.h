#ifndef IMAGE_H
#define IMAGE_H

#include "../core/bsoverlay.h"

class QImage;
class QFont;

class OITextStatic
{
protected:
  IOverlayUpdater m_ovllink;
  dmtype_image_t  dmti;
  QImage*         m_pImage;
  
  OITextStatic(IOverlay* ovllink, const char* text, unsigned int fontSize);
  OITextStatic(IOverlay* ovllink, const char* text, const QFont& font);
public:
  ~OITextStatic();
public:
  void          setText(const char* text, unsigned int fontSize);
  void          setText(const char* text, const QFont& font);
protected:
  void          innerSetText(const char* text, const QFont& font);
};

class OTextTraced: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, COORDINATION cn, float xpos, float ypos, const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextTraced(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  bool m_rectangled;
};

class OTextColored: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextColored(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize=12, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, COORDINATION cn, float xpos, float ypos, const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
  OTextColored(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont&, int argbTextColor=0x00000000, int argbBackColor=0x00FFFFFF, int argbBorderColor=0x00FFFFFF);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  int m_argbTextColor, m_argbBackColor, m_argbBorderColor;
};


class OTextPaletted: public IOverlayHard, public OVLCoordsDynamic, public OVLDimmsStatic, public OITextStatic
{
public:
  OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, COORDINATION cn, float xpos, float ypos, const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
  OTextPaletted(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont&, bool rectangled, const IPalette* ipal, bool discrete=false);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
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

