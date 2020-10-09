#ifndef BSTESTPRECISION_H
#define BSTESTPRECISION_H

/// Overlays:
///   OTestPrecision. View: technical overlay, need to be upgraded
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OTestPrecision: public DrawOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
public:
  OTestPrecision();
protected:
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
//  virtual bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void*, bool* doStop);
};

#endif // BSTESTPRECISION_H

