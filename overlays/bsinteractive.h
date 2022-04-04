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

class OActiveCursorCarrierL3: public OActiveCursorL
{
public:
  OActiveCursorCarrierL3(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, bool linkToScaledCenter=false);
  OActiveCursorCarrierL3(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter=false);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
  IOverlayReactor*  m_iop;
  IOverlayReactor*  m_iop2;
  IOverlayReactor*  m_iop3;
};


typedef OActiveCursorCarrierL OActiveCursorCarrier;
typedef OActiveCursorCarrierL2 OActiveCursorCarrier2;
typedef OActiveCursorCarrierL3 OActiveCursorCarrier3;

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

///////////////////////////////////////

class OActiveCell: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
{
protected:
  int             m_rows, m_columns;
  int             m_selrow, m_selcolumn;
  float           m_selfloat[2];
  int             m_margin;
public:
  OActiveCell(int rows, int columns, const linestyle_t& linestyle, int margin=0);
public:
  void          setCell(int r, int c, bool update=true){ m_selrow = r; m_selcolumn = c;   m_selfloat[0] = c/float(m_columns); m_selfloat[1] = r/float(m_rows); updateParameter(false, update); }
  void          setCellColumn(int c, bool update=true){ m_selcolumn = c;   m_selfloat[0] = c/float(m_columns); updateParameter(false, update); }
  void          setCellRow(int r, bool update=true){ m_selrow = r;   m_selfloat[1] = r/float(m_rows); updateParameter(false, update); }
public:
  void          moveCell(int dr, int dc, bool update=true){ m_selrow += dr; m_selcolumn += dc; m_selfloat[0] = m_selcolumn/float(m_columns); m_selfloat[1] = m_selrow/float(m_rows); updateParameter(false, update); }
  int           moveCellColumn(int dc, bool update=true){ m_selcolumn += dc; m_selfloat[0] = m_selcolumn/float(m_columns); updateParameter(false, update);  return m_selcolumn;   }
  int           moveCellRow(int dr, bool update=true){ m_selrow += dr; m_selfloat[1] = m_selrow/float(m_rows); updateParameter(false, update);  return m_selrow;   }
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
};

class OActiveCellCarrier: public OActiveCell
{
  coordstriumv_ex_t   c_ctex;   // cached value for non-local calls
public:
  OActiveCellCarrier(IOverlayReactor* iop, int rows, int columns, const linestyle_t& linestyle, int margin=0);
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*);
  IOverlayReactor*  m_iop;
};

////


#endif // INTERACTIVE_H
