#ifndef BSFIGURES_H
#define BSFIGURES_H

/// Overlays:   single simple figures
///   OFPoint: point,       OFCircle: circle,     OFSquare: square by leftbottom,     OFSquareCC: square by center 
///   OFTriangle: triangle,   OFLine: line with (x0,y0)(x1,y1) or horz/vert
///   OFArrow: line with arrow,   OFCross: square visir,  OFObjectif: like camera Objectif,   OFDouble: double lines 
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

class OFPoint: public DrawOverlay_ColorForegoing, public OVLCoordsDynamic, public OVLDimms1Static
{
  float r,g,b;
public:
  OFPoint(COORDINATION cn, float center_x, float center_y, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFPoint(OVLCoordsStatic* pcoords, float offset_x, float offset_y, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFCircle: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
  float  m_border;
public:
  OFCircle(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1), float border=1.0f);
  OFCircle(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float radius, const linestyle_t& kls=linestyle_solid(1,1,1), float border=1.0f);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFSquare: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFSquare(float fillopacity, COORDINATION cn, float leftbottom_x, float leftbottom_y, COORDINATION featcn, float aside, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFSquare(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFSquareCC: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFSquareCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFSquareCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFRhombCC: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float  m_fillcoeff;
public:
  OFRhombCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFRhombCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float halfside, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};


class OFTriangle: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
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
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFLine: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimmsOff
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
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFRay: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
public:
protected:
  COORDINATION  m_featcn;
  float         m_start_x, m_start_y;
  COORDINATION  m_additcn;
  float         m_length;
public:
  OFRay(COORDINATION cn, float start_x, float start_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFRay(COORDINATION cn, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFArrow: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimmsOff
{
protected:
  int           m_at;
  float         m_param1;
  float         m_param2;
public:
  OFArrow(COORDINATION cn, float arrow_x, float arrow_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFArrow(OVLCoordsStatic* pcoords, float offset_arrow_x, float offset_arrow_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFCross: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms1Static
{
protected:
  float         m_gap;
public:
  OFCross(COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFCross(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFAngle: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  int     m_orient;
public:
  enum  ORIENT { OR_LT, OR_RT, OR_LB, OR_RB };
  OFAngle(ORIENT orient, COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
  OFAngle(ORIENT orient, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFVisir: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  float         m_gap;
public:
  OFVisir(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float gap, float size, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFVisir(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFFactor: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  float         m_gap;
public:
  OFFactor(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float gap, float size, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFFactor(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size=-1, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFPointrun: public DrawOverlay_ColorForegoing, public OVLCoordsDynamic, public OVLDimmsOff
{
protected:
  COORDINATION  m_cr;
  float         m_gap;
  int           m_speed;
public:
  OFPointrun(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float gap_w, int speed);
  OFPointrun(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap_w, int speed);
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFSubjectif: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  float         m_gap;
public:
  OFSubjectif(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float width, float gap_w, float gap_h, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFSubjectif(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float width, float gap_w, float gap_h, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};

class OFObjectif: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Static
{
protected:
  float         m_gap[2];
public:
  OFObjectif(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFObjectif(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};


class OFDouble: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimms2Dynamic
{
protected:
  bool          m_horz;
  float         m_gap;
public:
  OFDouble(bool horz, COORDINATION cn, float center, COORDINATION featcn, float gap, const linestyle_t& kls=linestyle_solid(1,1,1));
  OFDouble(bool horz, OVLCoordsStatic* pcoords, float offset, COORDINATION featcn, float gap, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
};


//class OFLevels: public DrawOverlay_ColorTraced, public OVLCoordsDynamic, public OVLDimmsOff
//{
//public:
//protected:
//  int           m_lt;
//  COORDINATION  m_featcn;
//  float         m_param1;
//  float         m_param2;
//public:
//  enum  LINETYPE {  LT_HORZ=0, LT_VERT=1  };
//  OFLevels(LINETYPE linetype, COORDINATION cn, float base, const linestyle_t& kls=linestyle_solid(1,1,1));
////public:
////  OFLine(COORDINATION cn, float start_x, float start_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
////  OFLine(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float end_x, float end_y, const linestyle_t& linestyle=linestyle_solid(1,1,1));
//protected:
//  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const;
//};

#endif // BSFIGURES_H

