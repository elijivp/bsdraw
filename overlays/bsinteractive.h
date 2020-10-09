#ifndef INTERACTIVE_H
#define INTERACTIVE_H

/// Overlays:   invisible reactions for user events (mouse clicks, kbd keys). Use as root overlays
///   OActivePoint. Effect: invisible source of coords, dynamic, movable
///   OActiveCursor. Effect: mouse clicks reaction
///   OActiveCursorCarrier. Effect: mouse clicks reaction with attached client
///   OActiveCursorCarrier2. Effect: mouse clicks reaction with attached 2 clients
///   OActiveRandom. Effect: point moves after update() calls
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

class OActivePoint: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActivePoint(COORDINATION cn, float center_x, float center_y);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  float   m_seed[2];
};

class OActiveCursor: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActiveCursor(bool linkToScaledCenter=false);
  OActiveCursor(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool*);
  bool  m_linked;
};

class OActiveCursorCarrier: public OActiveCursor
{
public:
  OActiveCursorCarrier(DrawOverlayProactive* iop, bool linkToScaledCenter=false);
  OActiveCursorCarrier(DrawOverlayProactive* iop, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool*);
  DrawOverlayProactive*  m_iop;
};

class OActiveCursorCarrier2: public OActiveCursor
{
public:
  OActiveCursorCarrier2(DrawOverlayProactive* iop, DrawOverlayProactive* iop2, bool linkToScaledCenter=false);
  OActiveCursorCarrier2(DrawOverlayProactive* iop, DrawOverlayProactive* iop2, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool*);
  DrawOverlayProactive*  m_iop;
  DrawOverlayProactive*  m_iop2;
};

//////////////////////////////////

class OActiveRandom: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActiveRandom();
  void  update();
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  unsigned int m_ctr;
  float   m_seed[2];
};

//class Interactive1DMaxMin: public DrawOverlaySimple, public OVLCoordsOff, public OVLDimmsOff
//{
//public:
//  enum POINTERTYPE {  PT_MAX_H, PT_MIN_H, PT_MAX_V, PT_MIN_V  };
//  Interactive1DMaxMin(POINTERTYPE vt);
//  ~Interactive1DMaxMin();
//protected:
//  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
//  POINTERTYPE   m_pointerType;
//};


#endif // INTERACTIVE_H
