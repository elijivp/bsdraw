#ifndef BSBORDER_H
#define BSBORDER_H

#include "../core/bsoverlay.h"

class AbstractBorder: virtual public IOverlay
{
  bool    m_banclicks;
public:
  AbstractBorder(bool banclicks): m_banclicks(banclicks){}
protected:
  virtual bool  overlayReaction(OVL_REACTION, const void *dataptr, bool* doStop);
  virtual bool  clickBanned(float x, float y) const =0;
};


class OBorder: public IOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
  unsigned int  m_width;
public:
  OBorder(unsigned int widthpixels, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OToons: public IOverlayTraced, public OVLCoordsStatic, public OVLDimms1Static
{
  float         m_radius2, m_border;
  bool          m_banclicks;
public:
  OToons(COORDINATION cr, float diameter, float border, const linestyle_t& =linestyle_solid(0,0,0), bool banclikcks=true);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
//  virtual bool  clickBanned(float x, float y) const;
  virtual bool  overlayReaction(OVL_REACTION, const void *dataptr, bool* doStop);
};

#endif // BSBORDER_H

