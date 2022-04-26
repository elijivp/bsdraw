#ifndef BSTESTPRECISION_H
#define BSTESTPRECISION_H

/// Overlays:
///   OTestPrecision. View: technical overlay, need to be upgraded
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OTestPrecision: public DrawOverlay_ColorForegoing, public OVLCoordsOff, public OVLDimmsOff
{
public:
  OTestPrecision();
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
//  virtual bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop);
};

#endif // BSTESTPRECISION_H

