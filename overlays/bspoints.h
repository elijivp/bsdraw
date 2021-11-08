#ifndef POINTS_H
#define POINTS_H

/// Overlays:   lines and selectors, saved into array (care max 1024 size)
///   OPTFill: same as OFPoint but with another realization
///   ODropPoints: array of not linked points
///   OPolyLine: array of linked points, base class
///   ODropLine: click = new line from last point
///   OBrush: move = new line from last point
///   OSelector: rectangle-selector
///   OSelectorCirc: circle-selector
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

class OPTFill: public DrawOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
protected:
  linestyle_t   m_fill;
public:
  OPTFill(const linestyle_t& kls);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class ODropPoints: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
protected:
  unsigned int ptCountMax;
  ovlcoords_t*   ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptStart, ptCount;
public:
  ODropPoints(unsigned int ptlimit, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *, bool *doStop);
};

class OPolyLine: public DrawOverlayTraced, public OVLCoordsStatic, public OVLDimmsOff
{
protected:
  unsigned int  ptCountMax;
  ovlcoords_t*    ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptCount;
public:
  OPolyLine(unsigned int countPointsMax, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
public:
  unsigned int count() const { return ptCount; }
  void  setPointsCount(unsigned int newCount);
  void  setPoint(int idx, float x, float y);
  void  updatePoints();
};

class ODropLine: public OPolyLine
{
  bool          followMoving;
public:
  ODropLine(unsigned int maxpoints, bool lastFollowsMouse=true, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *, bool *doStop);
};

class OBrush: public OPolyLine
{
public:
  OBrush(unsigned int memoryPoints, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *, bool *doStop);
};



/////////// Selectors

class OSelectorReaction: public DrawOverlayTraced, public OVLCoordsDimmsLinked
{
protected:
  float     m_alpha;
  int       m_phase;
  bool      m_move, m_neg;
  ovlcoords_t m_dxy;
  OSelectorReaction(const linestyle_t& kls, float alpha, bool moveable, bool saveneg);
public:
  void    setMoveMode(bool mvm){  m_move = mvm; }
  bool    moveMode() const { return m_move; }
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *, bool *doStop);
};

class OSelector: public OSelectorReaction
{
public:
  OSelector(const linestyle_t& kls, float alpha=0.2f, bool moveable=false);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OSelectorCirc: public OSelectorReaction
{
public:
  OSelectorCirc(const linestyle_t& kls, float alpha=0.2f, bool moveable=false);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OSelectorBand: public OSelectorReaction
{
public:
  OSelectorBand(const linestyle_t& kls, float alpha=0.2f, bool moveable=false);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

#endif // POINTS_H
