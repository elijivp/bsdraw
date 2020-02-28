#ifndef OVERLAY_H
#define OVERLAY_H

#include "bsidrawcore.h"


enum  OVL_ORIENTATION   {  OO_INHERITED=0, OO_DEFAULT=1,
                           OO_LRBT=1, OO_RLBT, OO_LRTB, OO_RLTB, OO_TBLR, OO_BTLR, OO_TBRL, OO_BTRL,
                           OO_IHBT  , OO_IHTB, OO_LRIH, OO_RLIH
                        };


class IOverlaySimple: public IOverlay
{
  int   m_inversive;
public:
  IOverlaySimple(int inversive_algo=-1): m_inversive(inversive_algo){}
protected:
  virtual int fshColor(int overlay, char *to) const;
};

class _IOverlayLined: public IOverlay
{
protected:
  linestyle_t       m_linestyle;
//  linestyle_t       m_linestyle_outside;
public:
  void              setLineStyle(const linestyle_t& linestyle){  m_linestyle = linestyle; updatePublic(); }
  linestyle_t       getLineStyle() const {  return m_linestyle;  }
public:
  _IOverlayLined(): m_linestyle(linestyle_solid(1,1,1)){}
  _IOverlayLined(const linestyle_t& linestyle): m_linestyle(linestyle){}
};

class IOverlayTraced: public _IOverlayLined
{
public:
  IOverlayTraced(): _IOverlayLined(){}
  IOverlayTraced(const linestyle_t& linestyle): _IOverlayLined(linestyle){}
protected:
  virtual int fshColor(int overlay, char *to) const;
};

class IOverlayHard: public _IOverlayLined
{
private:
  dmtype_palette_t    m_dm_palette;
public:
  IOverlayHard(const IPalette* ipal, bool discrete, const linestyle_t& linestyle=linestyle_solid(1,1,1)): _IOverlayLined(linestyle)
  {
    m_dm_palette.ppal = ipal;
    m_dm_palette.discrete = discrete;
    appendUniform(DT__HC_PALETTE, &m_dm_palette);
  }
  void  setPalette(const IPalette* ipal, bool discrete)
  {
    m_dm_palette.ppal = ipal;
    m_dm_palette.discrete = discrete;
    IOverlay::overlayUpdateParameter(true); 
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
  void          getCoordinates(ovlcoords_t* coords) const { *coords = m_coords; }
public:
  typedef   OVLCoordsStatic   coords_type_t;
};

class OVLCoordsDynamic: public OVLCoordsStatic, virtual protected AbstractOverlay
{
public:
  OVLCoordsDynamic(COORDINATION cn, float x, float y): OVLCoordsStatic(cn, x, y) {  appendUniform(DT_2F, (const void*)&m_coords); }
  OVLCoordsDynamic(OVLCoordsStatic* cpy, float x, float y): OVLCoordsStatic(cpy, x, y) { appendUniform(DT_2F, (const void*)&m_coords); }
  void          setCoordinates(float x, float y){ m_coords.x = x; m_coords.y = y; overlayUpdateParameter(); }
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

class OVLDimms1Dynamic: public OVLDimms1Static, virtual protected AbstractOverlay
{
public:
  void          setSide(float side) { m_side = side; overlayUpdateParameter(); }
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

class OVLDimms2Dynamic: public OVLDimms2Static, virtual protected AbstractOverlay
{
public:
  OVLDimms2Dynamic(COORDINATION cn, float width, float height): OVLDimms2Static(cn, width, height){ appendUniform(DT_2F, (const void*)&m_sides); }
  void          setDimms(float width, float height){ m_sides.w = width; m_sides.h = height; overlayUpdateParameter(false); }
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
class OVLDimmsDynamic: virtual protected AbstractOverlay
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

class OVLCoordsDimmsLinked: virtual protected AbstractOverlay
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
  typedef OVLCoordsDimmsLinked     coords_type_t;
  typedef OVLCoordsDimmsLinked     dimms_type_t;
};


#endif // OVERLAY_H
