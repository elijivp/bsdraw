#ifndef BSSPRITES_H
#define BSSPRITES_H

#include "bsimage.h"

class OSprites: public IOverlaySimpleImage, public OVLCoordsOff, public OVLDimmsOff
{
public:
  OSprites(QImage* image, IMAGECONVERT icvt, unsigned int count, float sizemultiplier);
  ~OSprites();
//public slots:
  void    update();
protected:
  unsigned int m_count;
  float   m_sm;
  int     m_counter;
  dmtype_sampler_t  m_randomer;
  float   m_click[4];
protected:
  virtual int fshTrace(int overlay, char* to) const;
  bool overlayReaction(OVL_REACTION oreact, const void* dataptr, bool*);
};

#endif // BSSPRITES_H

