#ifndef POINTS_H
#define POINTS_H

#include "../core/bsoverlay.h"

class OPTFill: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimmsOff
{
protected:
  linestyle_t   m_fill;
public:
  OPTFill(const linestyle_t& kls);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

class ODropPoints: public IOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
protected:
  unsigned int ptCountMax;
  ovlcoords_t*   ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptStart, ptCount;
public:
  ODropPoints(unsigned int ptlimit, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  virtual bool  overlayReaction(OVL_REACTION, const void *, bool *doStop);
};

class DropsBase_: public IOverlayTraced, public OVLCoordsStatic, public OVLDimmsOff
{
protected:
  unsigned int  ptCountMax;
  ovlcoords_t*    ptdrops;
  dmtype_arr_t _dm_coords;
  unsigned int  ptCount;
protected:
  DropsBase_(unsigned int countPointsMax, const linestyle_t& kls);
  virtual int   fshTrace(int overlay, char* to) const;
};

class ODropLine: public DropsBase_
{
  bool          followMoving;
public:
  ODropLine(unsigned int maxpoints, bool lastFollowsMouse=true, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual bool  overlayReaction(OVL_REACTION, const void *, bool *doStop);
};

class OBrush: public DropsBase_
{
public:
  OBrush(unsigned int memoryPoints, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual bool  overlayReaction(OVL_REACTION, const void *, bool *doStop);
};

class OSelector: public IOverlayTraced, public OVLCoordsDimmsLinked
{
protected:
  float     m_alpha;
  int       m_phase;
public:
  OSelector(const linestyle_t& kls, float alpha=0.0f);
protected:
  virtual int   fshTrace(int overlay, char* to) const;
  virtual bool  overlayReaction(OVL_REACTION, const void *, bool *doStop);
};

#endif // POINTS_H
