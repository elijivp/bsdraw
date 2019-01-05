#ifndef BSTESTPRECISION_H
#define BSTESTPRECISION_H

#include "../../core/bsoverlay.h"

class OTestPrecision: public IOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
public:
  OTestPrecision();
protected:
  virtual int fshTrace(int overlay, char* to) const;
//  virtual bool overlayReaction(OVL_REACTION oreact, const void*, bool* doStop);
};

#endif // BSTESTPRECISION_H

