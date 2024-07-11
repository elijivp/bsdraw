#ifndef OVERLAY_H
#define OVERLAY_H

/// This file contains useful/technical types, base classes, conversion functions for all types of draws
/// Midway realisations for Overlays
/// If you need to create your own Overlay, use other overlays as examples
/// Created By: Elijah Vlasov

#include "bsidrawcore.h"


/// Shader takes color and mixwell from fshOVCoords result: in_variant[0..4] = [r g b mixwell]
class Ovldraw_ColorForegoing: public Ovldraw
{
  int   m_inversive;
public:
  Ovldraw_ColorForegoing(int inversive_algo=0, bool visible=true): Ovldraw(visible), m_inversive(inversive_algo){}
protected:
  virtual int fshColor(int overlay, char *to) const;
};


/// Shader takes only mixwell from fshOVCoords result: in_variant[0..4] = [- - - mixwell]
class Ovldraw_ColorDomestic: public Ovldraw
{
  color3f_t         m_color;
public:
  Ovldraw_ColorDomestic(const color3f_t& color, bool visible=true): Ovldraw(visible), m_color(color){}
  void  setColor(color3f_t color, bool update=true);
  color3f_t color() const { return m_color; }
protected:
  virtual int fshColor(int overlay, char *to) const;
};

/// Shader takes outsideline, path position and mixwell from fshOVCoords result: in_variant[0..4] = [ols path - mixwell]
class Ovldraw_ColorTraced: public Ovldraw
{
protected:
  linestyle_t       m_linestyle;
public:
  Ovldraw_ColorTraced(const linestyle_t& linestyle, bool visible=true): Ovldraw(visible), m_linestyle(linestyle){}
  void              setLineStyle(const linestyle_t& linestyle, bool update=true){  m_linestyle = linestyle; updateParameter(true, update); }
  linestyle_t       getLineStyle() const {  return m_linestyle;  }
protected:
  virtual int fshColor(int overlay, char *to) const;
};

/// Shader takes nondirect color and mixwell from fshOVCoords result: in_variant[0..4] = [ptr_to_color inversation - mixwell]
class Ovldraw_ColorThroughPalette: public Ovldraw
{
private:
  dmtype_palette_t    m_dm_palette;
public:
  Ovldraw_ColorThroughPalette(const IPalette* ipal, bool discrete, bool visible=true):  Ovldraw(visible)
  {
    m_dm_palette.ppal = ipal;
    m_dm_palette.discrete = discrete;
    m_dm_palette.prerange[0] = 0.0f;
    m_dm_palette.prerange[1] = 1.0f;
    appendUniform(DT_PALETTE, &m_dm_palette, true);
  }
  void  setPalette(const IPalette* ipal, bool discrete, bool update=true)
  {
    m_dm_palette.ppal = ipal;
    m_dm_palette.discrete = discrete;
    _Ovldraw::updateParameter(false, update); 
  }
  void  setPaletteRange(float range_min, float range_max, bool update=true)
  {
    _bsdraw_clamp(&range_min, &range_max);
    m_dm_palette.prerange[0] = range_min;
    m_dm_palette.prerange[1] = range_max;
    _Ovldraw::updateParameter(false, update); 
  }
protected:
  virtual int fshColor(int overlay, char *to) const;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// COINSIDE B: overlay form features
///
///

struct  ovlcoords_t
{
  float x,y;
  ovlcoords_t(){}
  ovlcoords_t(float _x, float _y): x(_x), y(_y){}
  ovlcoords_t(const float xy[]): x(xy[0]), y(xy[1]){}
};

typedef float ovldimms1_t;

struct  ovldimms2_t
{
  float w,h;
  ovldimms2_t(){}
  ovldimms2_t(float _w, float _h): w(_w), h(_h){}
  ovldimms2_t(const float wh[]): w(wh[0]), h(wh[1]){}
};

struct  ovldimms4_t
{
  float l,t,r,b;
  ovldimms4_t(){}
  ovldimms4_t(float _l, float _t, float _r, float _b): l(_l), t(_t), r(_r), b(_b) {}
  ovldimms4_t(const float ltrb[]): l(ltrb[0]), t(ltrb[1]), r(ltrb[2]), b(ltrb[3]){}
};


class OVLCoordsOff
{
public:
  typedef   OVLCoordsOff      coords_type_t;
};

class OVLCoordsStatic
{
protected:
  COORDINATION  m_cn;
  ovlcoords_t   m_coords;
public:
  OVLCoordsStatic(COORDINATION cn, float x, float y): m_cn(cn), m_coords(x,y){}
  OVLCoordsStatic(OVLCoordsStatic* cpy, float x=0.0f, float y=0.0f): m_cn(cpy->m_cn){  m_coords.x = cpy->m_coords.x + x; m_coords.y = cpy->m_coords.y + y; }
  COORDINATION  getCoordination() const { return m_cn; }
  void          getCoordinates(float* x, float* y) const { *x = m_coords.x; *y = m_coords.y;  }
  float         getCoordX() const { return m_coords.x; }
  float         getCoordY() const { return m_coords.y; }
  void          getCoordinates(ovlcoords_t* coords) const { *coords = m_coords; }
public:
  typedef   OVLCoordsStatic   coords_type_t;
};

class OVLCoordsDynamic: public OVLCoordsStatic, virtual protected _Ovldraw
{
public:
  OVLCoordsDynamic(COORDINATION cn, float x, float y): OVLCoordsStatic(cn, x, y) {  appendUniform(DT_2F, (const void*)&m_coords); }
  OVLCoordsDynamic(OVLCoordsStatic* cpy, float x, float y): OVLCoordsStatic(cpy, x, y) { appendUniform(DT_2F, (const void*)&m_coords); }
  void          setCoordinates(float x, float y, bool update=true){ m_coords.x = x; m_coords.y = y; updateParameter(false, update); }
  void          setCoordX(float x, bool update=true){ m_coords.x = x; updateParameter(false, update); }
  void          setCoordY(float y, bool update=true){ m_coords.y = y; updateParameter(false, update); }
public:
  void          moveCoordinates(float dx, float dy, bool update=true){ m_coords.x += dx; m_coords.y += dy; updateParameter(false, update); }
  float         moveCoordX(float dx, bool update=true){ m_coords.x += dx; updateParameter(false, update); return m_coords.x; }
  float         moveCoordY(float dy, bool update=true){ m_coords.y += dy; updateParameter(false, update); return m_coords.y; }
public:
  typedef   OVLCoordsDynamic  coords_type_t;
};

////////////////////////////////////////////////////////


class OVLDimmsOff
{
public:
  typedef OVLDimmsOff       dimms_type_t;
};

class OVLDimms1Static
{
protected:
  COORDINATION  m_cn;
  ovldimms1_t   m_side;
public:
  OVLDimms1Static(COORDINATION cn, float aside): m_cn(cn){  m_side = aside; }
  COORDINATION  getCoordination() const { return m_cn; }
  float         getSide() const {  return m_side; }
  typedef OVLDimms1Static   dimms_type_t;
};

class OVLDimms1Dynamic: public OVLDimms1Static, virtual protected _Ovldraw
{
public:
  void          setSide(float side, bool update=true) { m_side = side; updateParameter(false, update); }
  OVLDimms1Dynamic(COORDINATION cn, float aside): OVLDimms1Static(cn, aside) { appendUniform(DT_1F, (const void*)&m_side); }
  typedef OVLDimms1Dynamic  dimms_type_t;
};

/////////


class OVLDimms2Static
{
protected:
  COORDINATION  m_cn;
  ovldimms2_t   m_sides;
public:
  OVLDimms2Static(COORDINATION cn, float width, float height): m_cn(cn), m_sides(width, height){}
  COORDINATION  getCoordination() const { return m_cn; }
  ovldimms2_t   getDimms() const {  return m_sides; }
  void          getDimms(float *width, float* height) const {  *width = m_sides.w; *height = m_sides.h; }
  typedef OVLDimms2Static   dimms_type_t;
};

class OVLDimms2Dynamic: public OVLDimms2Static, virtual protected _Ovldraw
{
public:
  OVLDimms2Dynamic(COORDINATION cn, float width, float height): OVLDimms2Static(cn, width, height){ appendUniform(DT_2F, (const void*)&m_sides); }
  void          setDimms(float width, float height, bool update=true){ m_sides.w = width; m_sides.h = height; updateParameter(false, update); }
  typedef OVLDimms2Dynamic   dimms_type_t;
};

/////////


class OVLDimmsStatic
{
protected:
  COORDINATION  m_cn;
  float         m_dimms[4];
  void    laterInitDimms(float left, float top, float right, float bottom){ m_dimms[0] = left; m_dimms[1] = top; m_dimms[2] = right; m_dimms[3] = bottom; }
public:
  OVLDimmsStatic(COORDINATION cn, float aside = 0.0f): m_cn(cn){  m_dimms[0] = m_dimms[1] = m_dimms[2] = m_dimms[3] = aside; }
  OVLDimmsStatic(COORDINATION cn, float left, float top, float right, float bottom): m_cn(cn){ laterInitDimms(left, top, right, bottom); }
  COORDINATION  getCoordination() const { return m_cn; }
  void dimms(float* left, float* top, float* right, float* bottom) const
  {
    *left = m_dimms[0]; *top = m_dimms[1];  *right = m_dimms[2];  *bottom = m_dimms[3];
  }
  typedef OVLDimmsStatic     dimms_type_t;
};
class OVLDimmsDynamic: virtual protected _Ovldraw
{
protected:
  COORDINATION  m_cn;
  float         m_dimms[4];
  bool          m_dimmsready;
  void    laterInitDimms(float left, float top, float right, float bottom){ m_dimms[0] = left; m_dimms[1] = top; m_dimms[2] = right; m_dimms[3] = bottom; }
public:
  OVLDimmsDynamic(): m_cn(CR_RELATIVE), m_dimmsready(true) { m_dimms[0] = m_dimms[1] = m_dimms[2] = m_dimms[3] = 0; appendUniform(DT_4F, m_dimms); }
  OVLDimmsDynamic(COORDINATION cn, float aside = 0.0f): m_cn(cn), m_dimmsready(true) {  m_dimms[0] = m_dimms[1] = m_dimms[2] = m_dimms[3] = aside;  appendUniform(DT_4F, m_dimms); }
  OVLDimmsDynamic(COORDINATION cn, float left, float top, float right, float bottom): m_cn(cn), m_dimmsready(true) { laterInitDimms(left, top, right, bottom);  appendUniform(DT_4F, m_dimms); }
  COORDINATION  getCoordination() const { return m_cn; }
  bool dimms(float* left, float* top, float* right, float* bottom) const
  {
    *left = m_dimms[0]; *top = m_dimms[1];  *right = m_dimms[2];  *bottom = m_dimms[3];
    return m_dimmsready;
  }
  typedef OVLDimmsDynamic     dimms_type_t;
};

/////////

class OVLCoordsDimmsLinked: virtual protected _Ovldraw
{
protected:
  COORDINATION  m_cn;
  ovlcoords_t   m_xy;
  ovldimms2_t   m_wh;
  bool          m_dimmsready;
public:
  OVLCoordsDimmsLinked(COORDINATION cn, float x, float y, float w, float h): m_cn(cn), m_xy(x,y), m_wh(w,h), m_dimmsready(true) { appendUniform(DT_2F, (const void*)&m_xy);   appendUniform(DT_2F, (const void*)&m_wh); }
  COORDINATION  getCoordination() const { return m_cn; }
  bool dimms(float* left, float* top, float* right, float* bottom) const
  {
    *left  = m_wh.w < 0? m_xy.x + m_wh.w : m_xy.x;
    *right = m_wh.w < 0? m_xy.x : m_xy.x + m_wh.w;
    *top   = m_wh.h < 0? m_xy.y + m_wh.h : m_xy.y;
    *bottom= m_wh.h < 0? m_xy.y : m_xy.y + m_wh.h;
    return m_dimmsready;
  }
  bool inrect(ovlcoords_t xy) const
  {
    return  (m_wh.w < 0? m_xy.x + m_wh.w  : m_xy.x) < xy.x && xy.x < (m_wh.w < 0? m_xy.x : m_xy.x + m_wh.w) &&
            (m_wh.h < 0? m_xy.y + m_wh.h  : m_xy.y) < xy.y && xy.y < (m_wh.h < 0? m_xy.y : m_xy.y + m_wh.h);
  }
public:
  void          setCoordinates(float x, float y, bool update=true){ m_xy.x = x; m_xy.y = y; updateParameter(false, update); }
  void          setCoordX(float x, bool update=true){ m_xy.x = x; updateParameter(false, update); }
  void          setCoordY(float y, bool update=true){ m_xy.y = y; updateParameter(false, update); }
public:
  void          moveCoordinates(float dx, float dy, bool update=true){ m_xy.x += dx; m_xy.y += dy; updateParameter(false, update); }
  float         moveCoordX(float dx, bool update=true){ m_xy.x += dx; updateParameter(false, update); return m_xy.x; }
  float         moveCoordY(float dy, bool update=true){ m_xy.y += dy; updateParameter(false, update); return m_xy.y; }
public:
  typedef OVLCoordsDimmsLinked     coords_type_t;
  typedef OVLCoordsDimmsLinked     dimms_type_t;
};

/////////

//class OVLDimms22Linked: virtual protected _Ovldraw
//{
//protected:
//  COORDINATION  m_cn;
//  ovlcoords_t   m_x1y1;
//  ovlcoords_t   m_x2y2;
//  bool          m_dimmsready;
//public:
//  OVLDimms22Linked(COORDINATION cn, float x1, float y1, float x2, float y2): m_cn(cn), m_x1y1(x1,y1), m_x2y2(x2,y2), m_dimmsready(true) 
//  { appendUniform(DT_2F, (const void*)&m_x1y1);   appendUniform(DT_2F, (const void*)&m_x2y2); }
//  COORDINATION  getCoordination() const { return m_cn; }
////  bool dimms(float* left, float* top, float* right, float* bottom) const
////  {
////    *left  = m_wh.w < 0? m_xy.x + m_wh.w : m_xy.x;
////    *right = m_wh.w < 0? m_xy.x : m_xy.x + m_wh.w;
////    *top   = m_wh.h < 0? m_xy.y + m_wh.h : m_xy.y;
////    *bottom= m_wh.h < 0? m_xy.y : m_xy.y + m_wh.h;
////    return m_dimmsready;
////  }
////  bool inrect(ovlcoords_t xy) const
////  {
////    return  (m_wh.w < 0? m_xy.x + m_wh.w  : m_xy.x) < xy.x && xy.x < (m_wh.w < 0? m_xy.x : m_xy.x + m_wh.w) &&
////            (m_wh.h < 0? m_xy.y + m_wh.h  : m_xy.y) < xy.y && xy.y < (m_wh.h < 0? m_xy.y : m_xy.y + m_wh.h);
////  }
//  typedef OVLDimms22Linked     coords_type_t;
//  typedef OVLDimms22Linked     dimms_type_t;
//};


#endif // OVERLAY_H
