#ifndef INTERACTIVE_H
#define INTERACTIVE_H

#include "../core/bsoverlay.h"

class OActivePoint: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActivePoint(COORDINATION cn, float center_x, float center_y);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  float   m_seed[2];
};

class OActiveCursor: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActiveCursor(bool linkToScaledCenter=false);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  virtual bool  overlayReaction(OVL_REACTION, const void *dataptr, bool*);
  bool  m_linked;
};

//////////////////////////////////

class OActiveRandom: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActiveRandom();
  void  update();
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  unsigned int m_ctr;
  float   m_seed[2];
};

//class Interactive1DMaxMin: public IOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
//{
//public:
//  enum POINTERTYPE {  PT_MAX_H, PT_MIN_H, PT_MAX_V, PT_MIN_V  };
//  Interactive1DMaxMin(POINTERTYPE vt);
//  ~Interactive1DMaxMin();
//protected:
//  virtual int   fshTrace(int overlay, char* to) const;
//  POINTERTYPE   m_pointerType;
//};


#endif // INTERACTIVE_H
