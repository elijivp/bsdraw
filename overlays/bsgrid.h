#ifndef GRID_H
#define GRID_H

/// Overlays:   grids
///   OGridRegular. View: classic regular grid
///   OGridCircular. View: circular
///   OGridDecart. View: classic decart
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

/// Standard regular grid
class OGridRegular: public DrawOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  enum REGULAR  {  REGULAR_HORZ, REGULAR_VERT };
  OGridRegular(REGULAR  go, COORDINATION cn, float startchannel, float stepsize, const linestyle_t& linestyle, int maxsteps=-1, bool showGridAtZero=false);
  OGridRegular(REGULAR  go, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, const linestyle_t& linestyle, int maxsteps=-1, bool showGridAtZero=false);
  /// bounded regular variant
  OGridRegular(float start, float stepsize, const linestyle_t& linestyle, int maxsteps=-1, bool showGridAtZero=false);
  enum RISK     {  RISK_HORZ, RISK_VERT };
  OGridRegular(RISK     gr, COORDINATION cn, float startchannel, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps=-1);  
  OGridRegular(RISK     gr, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps=-1);  
  /// bounded regular variant
  OGridRegular(float start, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps=-1);  
protected:
  int           m_gridtype;
  float         m_stepsize;
  float         m_specheight;
  bool          m_babsheight;
  int           m_maxsteps;
  bool          m_zeroreg;
  COORDINATION  m_additcn;
protected:
  int fshTrace(int overlay, bool rotated, char *to) const;
};

/// Circular grid
class OGridCircular: public DrawOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  OGridCircular(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float stepsize, const linestyle_t& linestyle, int maxsteps=-1, float border=1.5f);
protected:
  COORDINATION  m_featcn;
  float         m_stepsize;
  int           m_maxsteps;
  float         m_border;
protected:
  int fshTrace(int overlay, bool rotated, char *to) const;
};

/// Dekart grid
class OGridDecart: public DrawOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
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


// Cells
class OGridCells: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
protected:
  int             m_rows, m_columns;
public:
  OGridCells(int rows, int columns, const linestyle_t& linestyle);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};


#endif // GRID_H
