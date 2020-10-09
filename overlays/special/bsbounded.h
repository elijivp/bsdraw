#ifndef BSBOUNDED_H
#define BSBOUNDED_H

/// Overlays:   data bound-depended (low and high data bounds)
///   OValueLine. View: horizontal line across value
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OValueLine: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_value;
public:
  OValueLine(float value=0.0f);
  OValueLine(float value, const linestyle_t& linestyle);
protected:
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
};

#endif // BSBOUNDED_H
