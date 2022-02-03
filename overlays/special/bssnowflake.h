#ifndef BSSNOWFLAKE_H
#define BSSNOWFLAKE_H

/// Overlays:
///   OSnowflake. View: snowflake lol
/// Created By: Elijah Vlasov

#include "../bsimage.h"

class OSnowflake: public DrawOverlaySimple, public OVLQImage, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
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
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*);
};

#endif // BSSNOWFLAKE_H

