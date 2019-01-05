#ifndef IMAGE_H
#define IMAGE_H

#include "../core/bsoverlay.h"

class QImage;
class QFont;

class OSimpleImage: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsStatic
{
public:
  OSimpleImage(COORDINATION cn, float xpos, float ypos, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OSimpleImage(OVLCoordsStatic* pcoords, float offset_x, float offset_y, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  ~OSimpleImage();
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  dmtype_image_t  dmti;
  QImage*         m_pImage;
  bool            m_rectangled;
};

class OTextStatic: public OSimpleImage
{
public:
  OTextStatic(const char* text, COORDINATION cn, float xpos, float ypos, unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextStatic(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, unsigned int fontSize=12, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextStatic(const char* text, COORDINATION cn, float xpos, float ypos, const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
  OTextStatic(const char* text, OVLCoordsStatic* pcoords, float offset_x, float offset_y, const QFont&, bool rectangled=false, const linestyle_t& kls = linestyle_solid(1,1,1));
public:
  void          setText(const char* text, unsigned int fontSize, bool rectangled=false);
  void          setText(const char* text, const QFont& font, bool rectangled=false);
protected:
  void          innerSetText(const char* text, const QFont& font);
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

