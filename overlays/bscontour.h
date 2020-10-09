#ifndef BSCONTOUR_H
#define BSCONTOUR_H

/// Overlays:   Value-depended, useful for highlight data
///   OContour. View: a < data[x] < b: colored borders for data
///   OContourPal. View: a < data[x] < b: colored borders for data corresponding with palette
///   OCover. View: a < data[x] < b: fill by color
///   OSlice. View: ... idk 
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

/// Contour by Single samples with color from linestyle
class OContour: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_from, m_to;
  bool    m_noscaled_contour;
  unsigned char m_checkmask;
public:
  OContour(float from, float to, const linestyle_t& kls, bool noscaled_contour=true, unsigned char checkmask=0xFF); // checkmask: left-bottom to around
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

/// Contour by Single samples with color from palette
class OContourPal: public DrawOverlayHard, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_from, m_to;
  bool    m_noscaled_contour;
  unsigned char m_checkmask;
public:
  OContourPal(float from, float to, const IPalette* ipal, bool discrete, bool noscaled_contour=true, unsigned char checkmask=0xFF);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OCover: public DrawOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
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

class OSlice: public DrawOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
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

