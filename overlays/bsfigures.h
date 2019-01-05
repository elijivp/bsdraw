#ifndef BSFIGURES_H
#define BSFIGURES_H

#include "../core/bsoverlay.h"

class OFPoint: public IOverlaySimple, public OVLCoordsDynamic, public OVLDimms1Static
{
  float r,g,b;
public:
  OFPoint(COORDINATION cn, float center_x, float center_y, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFPoint(OVLCoordsStatic* pcoords, float offset_x, float offset_y, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

class OFCircle: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFCircle(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFCircle(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
//public:
//  float radius() const;
//  void  updateRadius(float radius);
};

class OFSquare: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFSquare(float fillopacity, COORDINATION cn, float leftbottom_x, float leftbottom_y, COORDINATION featcn, float aside, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFSquare(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
//public:
//  float getSide() const;
//  void  updateSide(float aside);
};

class OFSquareCC: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFSquareCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFSquareCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
public:
  float halfside() const;
  void  updateHalfside(float aside);
};

class OFTriangle: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
public:
  enum  ORIENT { ORIENT_UP, ORIENT_DOWN };
protected:
  ORIENT  m_orientation;
  float   m_fillcoeff;
public:
  OFTriangle(ORIENT orientation, float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float getSide, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFTriangle(ORIENT orientation, float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float getSide, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
//public:
//  float fside() const;
//  void  updateSide(float getSide);
};

class OFLine: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
protected:
  int           m_lt;
  COORDINATION  m_featcn;
  float         m_param1;
  float         m_param2;
public:
  enum  LINETYPE {  LT_HORZ_SYMMETRIC=0, LT_HORZ_BYLEFT=1, LT_VERT_SYMMETRIC=2, LT_VERT_BYBOTTOM=3, LT_CROSS=4  };
  OFLine(LINETYPE linetype, COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFLine(LINETYPE linetype, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
public:
  OFLine(COORDINATION cn, float start_x, float start_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFLine(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

class OFArrow: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
  enum  ARROWTYPE {  AT_SUKA, AT_BLET, AT_SUKABLET  };
protected:
  int           m_at;
  float         m_param1;
  float         m_param2;
public:
  OFArrow(COORDINATION cn, float arrow_x, float arrow_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFArrow(OVLCoordsStatic* pcoords, float offset_arrow_x, float offset_arrow_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

class OFCross: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float         m_gap;
public:
  OFCross(COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFCross(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

class OFFactor: public IOverlayTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  float         m_gap;
public:
  OFFactor(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float gap, float size, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFFactor(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, char* to) const;
};

#endif // BSFIGURES_H

