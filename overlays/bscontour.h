#ifndef BSCONTOUR_H
#define BSCONTOUR_H

#include "../core/bsoverlay.h"

/// Contour by Single samples with color from linestyle
class OContour: public IOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_from, m_to;
  bool    m_noscaled_contour;
public:
  OContour(float from, float to, const linestyle_t& kls, bool noscaled_contour=true);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

/// Contour by Single samples with color from palette
class OContourPal: public IOverlayHard, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_from, m_to;
  bool    m_noscaled_contour;
public:
  OContourPal(float from, float to, const IPalette* ipal, bool discrete, bool noscaled_contour=true);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OCover: public IOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
public:
  enum  COVER_OTHER_PORTIONS { COP_COVER, COP_SAVEUPPER, COP_SAVELOWER, COP_SAVEALL  };
private:
  float   m_from, m_to;
  float   m_cover_r, m_cover_g, m_cover_b;
  COVER_OTHER_PORTIONS  m_cop;
public:
  OCover(float from, float to, float r, float g, float b, COVER_OTHER_PORTIONS = COP_SAVEUPPER);
  OCover(float from, float to, int inversive_algo, COVER_OTHER_PORTIONS = COP_SAVEUPPER);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
}; 

class OSlice: public IOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
private:
  float   m_cover;
  float   m_slice_r, m_slice_g, m_slice_b;
public:
  OSlice(float cover, float r, float g, float b);
  OSlice(float cover, int inversive_algo);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
}; 


#endif // BSCONTOUR_H

