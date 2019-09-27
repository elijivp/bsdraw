#ifndef BSBLOCKER_H
#define BSBLOCKER_H

#include "../../core/bsoverlay.h"

class OBlocker: public IOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
  int     m_blockstate;
public:
  OBlocker();
  void    setLocked(bool locked);
  void    setUnlocked(bool unlocked);
  bool    isLocked() const {  return m_blockstate; }
protected:
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
  virtual bool overlayReaction(OVL_REACTION oreact, const void*, bool* doStop);
};

#endif // BSBLOCKER_H

