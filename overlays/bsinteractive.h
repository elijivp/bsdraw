#ifndef INTERACTIVE_H
#define INTERACTIVE_H

/// Overlays:   invisible reactions for user events (mouse clicks, kbd keys). Use as root overlays
///   OActivePointer. Effect: invisible source of coords, dynamic, movable
///   OActiveCursor. Effect: mouse clicks reaction
///   OActiveCursorCarrier. Effect: mouse clicks reaction with attached client
///   OActiveCursorCarrier2. Effect: mouse clicks reaction with attached 2 clients
///   OActiveRandom. Effect: point moves after update() calls
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

class _OActiveBase: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff, public IOverlayReactor
{
  bool  m_linked;
public:
  _OActiveBase(bool linkToScaledCenter=false);
  _OActiveBase(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
};


////

class OActivePointer: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OActivePointer(COORDINATION cn, float center_x, float center_y);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  float   m_seed[2];
};

////

class OActiveCursor: public _OActiveBase
{
public:
  OActiveCursor(bool linkToScaledCenter=false);
  OActiveCursor(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
};

class OActiveCursorR: public _OActiveBase
{
public:
  OActiveCursorR(bool linkToScaledCenter=false);
  OActiveCursorR(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
};

typedef OActiveCursor OActiveCursorL;


////


class OActiveCursorCarrierL: public OActiveCursorL
{
public:
  OActiveCursorCarrierL(IOverlayReactor* iop, bool linkToScaledCenter=false);
  OActiveCursorCarrierL(IOverlayReactor* iop, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
  IOverlayReactor*  m_iop;
};

class OActiveCursorCarrierL2: public OActiveCursorL
{
public:
  OActiveCursorCarrierL2(IOverlayReactor* iop, IOverlayReactor* iop2, bool linkToScaledCenter=false);
  OActiveCursorCarrierL2(IOverlayReactor* iop, IOverlayReactor* iop2, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
  IOverlayReactor*  m_iop;
  IOverlayReactor*  m_iop2;
};


typedef OActiveCursorCarrierL OActiveCursorCarrier;
typedef OActiveCursorCarrierL2 OActiveCursorCarrier2;

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
