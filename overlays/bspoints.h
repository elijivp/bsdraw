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

class OPTFill: public Ovldraw_ColorForegoing, public OVLCoordsDynamic, public OVLDimmsOff
{
protected:
  linestyle_t   m_fill;
public:
  OPTFill(const linestyle_t& kls);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class ODropPoints: public Ovldraw_ColorTraced, public OVLCoordsOff, public OVLDimmsOff, public IOverlayReactor
{
protected:
  unsigned int ptCountMax;
  ovlcoords_t*   ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptStart, ptCount;
public:
  ODropPoints(unsigned int ptlimit, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool *doStop);
};

class OPolyLine: public Ovldraw_ColorTraced, public OVLCoordsStatic, public OVLDimmsOff
{
protected:
  unsigned int  ptCountMax;
  ovlcoords_t*    ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptCount;
public:
  OPolyLine(unsigned int countPointsMax, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
public:
  unsigned int count() const { return ptCount; }
  void  clear(bool update=true);
  void  setPointsCount(unsigned int newCount, bool update=true);
  void  setPoint(int idx, float x, float y, bool update=true);
  void  point(int idx, float* x, float* y) const;
  void  updatePoints(bool update=true);
};

class ODropLine: public OPolyLine, public IOverlayReactor
{
  bool          followMoving;
  bool          firstFixed;
public:
  ODropLine(unsigned int maxpoints, bool lastFollowsMouse=true, const linestyle_t& kls=linestyle_solid(1,1,1));
  ODropLine(unsigned int maxpoints, float start_x, float start_y, bool lastFollowsMouse=true, const linestyle_t& kls=linestyle_solid(1,1,1));
public:
  void    fixStartPoint(float start_x, float start_y, bool update=true);
  void    unfixStartPoint();
protected:
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool *doStop);
};

class OBrush: public OPolyLine, public IOverlayReactor
{
public:
  OBrush(unsigned int memoryPoints, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool *doStop);
};

/////////// Selectors && Highlights

class OSelectorReaction: public Ovldraw_ColorTraced, public OVLCoordsDimmsLinked, public IOverlayReactor
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
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool *doStop);
};

class OHighlight: public OSelectorReaction
{
  COORDINATION    m_cr;
  float           m_gap;
  float           m_limit;
public:
  OHighlight(const linestyle_t& kls, bool moveable=false, COORDINATION cr=CR_RELATIVE, float gap=0.0f, float limitsize=-1.0f);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OSelector: public OSelectorReaction
{
public:
  OSelector(const linestyle_t& kls, float alpha=0.2f, bool moveable=false);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OSelectorCirc: public OSelectorReaction
{
public:
  OSelectorCirc(const linestyle_t& kls, float alpha=0.2f, bool moveable=false);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OSelectorBand: public OSelectorReaction
{
  bool      m_dir_horz;
public:
  OSelectorBand(const linestyle_t& kls, float alpha=0.2f, bool dir_horz=true, bool moveable=false);
  bool    getInterval(float* imin, float* imax) const ;
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};


/////////// Segments

class OSegmentReaction: public Ovldraw_ColorTraced, public OVLCoordsDimmsLinked, public IOverlayReactor
{
protected:
  int       m_phase;
  ovlcoords_t m_dxy;
  OSegmentReaction(const linestyle_t& kls);
protected:
  virtual IOverlayReactor*  reactor() { return this; }
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool *doStop);
};

class OSegment: public OSegmentReaction
{
  COORDINATION    m_cr;
  float           m_gap;
  float           m_size;
public:
  OSegment(const linestyle_t& kls, COORDINATION cr, float gap, float size);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};



#endif // POINTS_H
