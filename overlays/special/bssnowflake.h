#ifndef BSSNOWFLAKE_H
#define BSSNOWFLAKE_H

/// Overlays:
///   OSnowflake. View: snowflake lol
/// Created By: Elijah Vlasov

#include "../bsimage.h"

class OSnowflake: public Ovldraw_ColorForegoing, public OVLQImage, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
{
public:
  OSnowflake(QImage* image, IMAGECONVERT icvt, unsigned int count, float sizemultiplier);
  ~OSnowflake();
  void    update();
protected:
  unsigned int m_count;
  float   m_sm;
  int     m_counter;
  dmtype_sampler_t  m_randomer;
  float   m_click[4];
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*);
};

#endif // BSSNOWFLAKE_H

