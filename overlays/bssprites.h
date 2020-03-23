#ifndef BSSPRITES_H
#define BSSPRITES_H

#include "bsimage.h"


struct kpdc_t
{
  float   x, y;
  float   zoom;
  float   color;
};

class OSprites: public IOverlay, public OVLQImage, public OVLCoordsOff, public OVLDimmsOff
{
public:
  enum  CENTER_BY { CB_LEFTTOP, CB_CENTER };
  enum  COLOR_SPRITE  { CR_OPACITY, CR_DISABLE };
  OSprites(QImage* image, IMAGECONVERT icvt, float sizemultiplier, unsigned int count, COLOR_SPRITE cr=CR_OPACITY, CENTER_BY cb=CB_CENTER);
  OSprites(QImage* image, IMAGECONVERT icvt, float sizemultiplier, unsigned int count, const class IPalette* ipal, bool discrete, CENTER_BY cb=CB_CENTER);
  ~OSprites();
public:
  ///   Idx from zero
  void  setKPDC(unsigned int idx, float x, float y);
  void  setKPDCZoom(unsigned int idx, float zoom);
  void  setKPDCColor(unsigned int idx, float color);
  void  setKPDCZoomColor(unsigned int idx, float zoom, float color);
  void  setKPDC(unsigned int idx, float x, float y, float color);
  void  setKPDC(unsigned int idx, float x, float y, float zoom, float color);
  void  updateKPDC();
  
  const kpdc_t& at(int idx) const;
public:
  unsigned  int total() const { return m_countmax; }
  unsigned  int count() const { return m_countactive; }
  void          setActiveCount(unsigned int count);
protected:
  float         m_sm;
  unsigned int  m_countmax;
  int           m_cb, m_cr;
  unsigned int  m_countactive;
  dmtype_sampler_t  m_kpdc;
  dmtype_palette_t  m_dm_palette;
protected:
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
  virtual int fshColor(int overlay, char *to) const;
  bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*);
public:
  void  setPalette(const IPalette* ipal, bool discrete);
};

#endif // BSSPRITES_H

