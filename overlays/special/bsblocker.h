#ifndef BSBLOCKER_H
#define BSBLOCKER_H

/// Overlays:
///   Blocker. View: grey screen who disappears after one left mouse click
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OBlocker: public Ovldraw_ColorForegoing, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
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
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop);
};

class OBlockerDots: public Ovldraw_ColorForegoing, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
{
  int     m_mod[2];
  float   m_mixdot, m_mixback;
  float   m_clrdot[3], m_clrback[3];
public:
  OBlockerDots(unsigned int modX, unsigned int modY, unsigned int dotcolor=0x00EEEEEE, float dotmix=0.9f, unsigned int backcolor=0x00000000, float backmix=0.9f);
  void    setLocked(bool locked);
  void    setUnlocked(bool unlocked);
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop);
};


class OBlockerShirm: public Ovldraw_ColorForegoing, public OVLCoordsDynamic, public OVLDimmsOff
{
  int     m_setup;
  float   m_mixline, m_mixback;
  float   m_clrline[3], m_clrback[3];
public:
  enum  { OBS_HORZ2RIGHT, OBS_HORZ2LEFT, OBS_VERT2TOP, OBS_VERT2BOT };
  OBlockerShirm(float start, int setup, unsigned int colorLine=0x00FFFFFF, float mixLine=0.1f, unsigned int colorBack=0x00000000, float mixBack=0.9f);
protected:
  virtual int fshOVCoords(int overlay, bool /*switchedab*/, char* to) const;
};


#endif // BSBLOCKER_H

