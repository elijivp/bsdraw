#ifndef BSBLOCKER_H
#define BSBLOCKER_H

/// Overlays:
///   Blocker. View: grey screen who disappears after one left mouse click
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OBlocker: public DrawOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
{
  int     m_blockstate;
  float   m_clr[3];
public:
  OBlocker(unsigned int color=0x00EEEEEE);
  OBlocker(float r, float g, float b);
  void    setLocked(bool locked);
  void    setUnlocked(bool unlocked);
  bool    isLocked() const {  return m_blockstate; }
protected:
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
  virtual bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void*, bool* doStop);
};

#endif // BSBLOCKER_H

