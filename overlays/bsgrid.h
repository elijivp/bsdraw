#ifndef GRID_H
#define GRID_H

#include "../core/bsoverlay.h"

/// Standard regular grid
class OGridRegular: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  enum REGULAR  {  REGULAR_HORZ, REGULAR_VERT };
  OGridRegular(REGULAR  go, COORDINATION cn, float startchannel, float stepsize, const linestyle_t& linestyle, int maxsteps=-1);
  enum RISK     {  RISK_HORZ, RISK_VERT };
  OGridRegular(RISK     gr, COORDINATION cn, float startchannel, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps=-1);  
protected:
  int           m_gridtype;
  float         m_stepsize;
  float         m_specheight;
  bool          m_babsheight;
  int           m_maxsteps;
protected:
  int fshTrace(int overlay, bool rotated, char *to) const;
};

/// Circular grid
class OGridCircular: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OGridCircular(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float stepsize, const linestyle_t& linestyle, int maxsteps=-1);
protected:
  COORDINATION  m_featcn;
  float         m_stepsize;
  int           m_maxsteps;
protected:
  int fshTrace(int overlay, bool rotated, char *to) const;
};

/// Dekart grid
class OGridDecart: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OGridDecart(COORDINATION cn, float center_x, float center_y, float step_x, float step_y, int absolute_risk_height, float limit_x_left=-1, float limit_x_right=-1, float limit_y_top=-1, float limit_y_bottom=-1, const linestyle_t& linestyle=linestyle_white(1,0,0));
protected:
  float         m_step[2];
  float         m_limits[4];
  int           m_riskheight;
protected:
  int fshTrace(int overlay, bool rotated, char *to) const;
};

#endif // GRID_H
