#ifndef BSDRAWDEF_H
#define BSDRAWDEF_H

/// This file contains useful/technical types, base classes, conversion functions for all types of draws
/// Created By: Elijah Vlasov

inline int bscolor3fToint(float r, float g, float b)
{
  return (int(b*255)<<16) + (int(g*255)<<8) + int(r*255);
}

inline int bscolor4fToint(float r, float g, float b, float a)
{
  return (int(a*255)<<24) + (int(b*255)<<16) + (int(g*255)<<8) + int(r*255);
}

inline void bsintTocolor3f(int clr, float rgb[])
{
  int b = (clr >> 16) & 0xFF, g = (clr >> 8) & 0xFF, r = (clr) & 0xFF;
  rgb[0] = b/255.0f;    rgb[1] = g/255.0f;    rgb[2] = r/255.0f;
}

inline void bsintTocolor4f(int clr, float rgb[])
{
  int a = (clr >> 24) & 0xFF, b = (clr >> 16) & 0xFF, g = (clr >> 8) & 0xFF, r = (clr) & 0xFF;
  rgb[0] = a/255.0f;    rgb[1] = b/255.0f;    rgb[2] = g/255.0f;    rgb[3] = r/255.0f;
}


enum  ORIENTATION       { OR_LRBT=0,  OR_RLBT,  OR_LRTB,  OR_RLTB,    // order for +4 work
                          OR_TBLR,    OR_BTLR,  OR_TBRL,  OR_BTRL 
                        };


inline bool orientationMirroredHorz(ORIENTATION ort)
{
  const bool mh[] = { false, true, false, true, false, false, true, true };
  return mh[ort];
}

inline bool orientationMirroredVert(ORIENTATION ort)
{
  const bool mv[] = { false, false, true, true, true, false, true, false };
  return mv[ort];
}

inline bool orientationTransposed(ORIENTATION ort)
{
  const bool mv[] = { false, false, false, false, true, true, true, true };
  return mv[ort];
}

enum  OVL_ORIENTATION   { OO_INHERITED=0,     // default
                          OO_INHERITED_MIRROR_HORZ,
                          OO_INHERITED_MIRROR_VERT,
                          OO_INHERITED_MIRROR_BOTH,
                          OO_AREA_LRBT, OO_AREA_RLBT, OO_AREA_LRTB, OO_AREA_RLTB, OO_AREA_TBLR, OO_AREA_BTLR, OO_AREA_TBRL, OO_AREA_BTRL,
                          OO_AREAOR,
                          OO_AREAOR_MIRROR_HORZ,
                          OO_AREAOR_MIRROR_VERT,
                          OO_AREAOR_MIRROR_BOTH,
                          OO_SAME // special orient when ovl replaced by another
                        };

enum  DRAWVIEWALIGN     { DVA_LEFT=0,  DVA_CENTER,  DVA_RIGHT
                        };

enum  { SPFLAG_COLORSPLIT = 0x10000000 };

enum  SPLITPORTIONS     { SP_NONE=0,
                          SP_COLUMN_TB      =0x10001,    SP_ROW_LR       =0x10101,  SP_COLUMN_BT    =0x0001,  SP_ROW_RL     =0x0101, 
                          SP_COLUMNS_TB_2   =0x10002,    SP_ROWS_LR_2    =0x10102,  SP_COLUMNS_BT_2 =0x0002,  SP_ROWS_RL_2  =0x0102,
                          SP_COLUMNS_TB_3   =0x10003,    SP_ROWS_LR_3    =0x10103,  SP_COLUMNS_BT_3 =0x0003,  SP_ROWS_RL_3  =0x0103,
                          SP_COLUMNS_TB_4   =0x10004,    SP_ROWS_LR_4    =0x10104,  SP_COLUMNS_BT_4 =0x0004,  SP_ROWS_RL_4  =0x0104,
                          SP_COLUMNS_TB_5   =0x10005,    SP_ROWS_LR_5    =0x10105,  SP_COLUMNS_BT_5 =0x0005,  SP_ROWS_RL_5  =0x0105,
                          SP_COLUMNS_TB_6   =0x10006,    SP_ROWS_LR_6    =0x10106,  SP_COLUMNS_BT_6 =0x0006,  SP_ROWS_RL_6  =0x0106,
                          SP_COLUMNS_TB_7   =0x10007,    SP_ROWS_LR_7    =0x10107,  SP_COLUMNS_BT_7 =0x0007,  SP_ROWS_RL_7  =0x0107,
                          SP_COLUMNS_TB_8   =0x10008,    SP_ROWS_LR_8    =0x10108,  SP_COLUMNS_BT_8 =0x0008,  SP_ROWS_RL_8  =0x0108,
                          
                          
                          SP_COLUMN_TB_COLORSPLIT     =SPFLAG_COLORSPLIT|0x10001,   SP_ROW_LR_COLORSPLIT      =SPFLAG_COLORSPLIT|0x10101,  
                           SP_COLUMN_BT_COLORSPLIT    =SPFLAG_COLORSPLIT|0x0001,    SP_ROW_RL_COLORSPLIT      =SPFLAG_COLORSPLIT|0x0101, 
                          SP_COLUMNS_TB_2_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10002,   SP_ROWS_LR_2_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10102,  
                           SP_COLUMNS_BT_2_COLORSPLIT =SPFLAG_COLORSPLIT|0x0002,    SP_ROWS_RL_2_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0102,
                          SP_COLUMNS_TB_3_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10003,   SP_ROWS_LR_3_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10103,  
                           SP_COLUMNS_BT_3_COLORSPLIT =SPFLAG_COLORSPLIT|0x0003,    SP_ROWS_RL_3_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0103,
                          SP_COLUMNS_TB_4_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10004,   SP_ROWS_LR_4_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10104,  
                           SP_COLUMNS_BT_4_COLORSPLIT =SPFLAG_COLORSPLIT|0x0004,    SP_ROWS_RL_4_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0104,
                          SP_COLUMNS_TB_5_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10005,   SP_ROWS_LR_5_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10105,  
                           SP_COLUMNS_BT_5_COLORSPLIT =SPFLAG_COLORSPLIT|0x0005,    SP_ROWS_RL_5_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0105,
                          SP_COLUMNS_TB_6_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10006,   SP_ROWS_LR_6_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10106,  
                           SP_COLUMNS_BT_6_COLORSPLIT =SPFLAG_COLORSPLIT|0x0006,    SP_ROWS_RL_6_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0106,
                          SP_COLUMNS_TB_7_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10007,   SP_ROWS_LR_7_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10107,  
                           SP_COLUMNS_BT_7_COLORSPLIT =SPFLAG_COLORSPLIT|0x0007,    SP_ROWS_RL_7_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0107,
                          SP_COLUMNS_TB_8_COLORSPLIT  =SPFLAG_COLORSPLIT|0x10008,   SP_ROWS_LR_8_COLORSPLIT   =SPFLAG_COLORSPLIT|0x10108,  
                           SP_COLUMNS_BT_8_COLORSPLIT =SPFLAG_COLORSPLIT|0x0008,    SP_ROWS_RL_8_COLORSPLIT   =SPFLAG_COLORSPLIT|0x0108
                        };


enum  DTYPE       /// Trace/simple shader datatypes
{
              DT_1F, DT_2F, DT_3F, DT_4F, 
              DT_ARR,   DT_ARR2,  DT_ARR3, DT_ARR4, 
              DT_ARRI, DT_ARRI2, DT_ARRI3, DT_ARRI4, 
              DT_1I, DT_2I, DT_3I, DT_4I,
              DT_SAMP4, DT_2D3F, DT_TEXTURE, DT_PALETTE    // >= DT_SAMP4 are textures
};

inline bool dtIsTexture(DTYPE dtype) { return dtype >= DT_SAMP4; }

struct dmtype_t
{
  DTYPE         type;
  const void*   dataptr;
};

struct dmtype_arr_t
{
  unsigned int  count;
  const void*   data;
};

struct dmtype_sampler_t
{
  unsigned int  count;
  const float*  data;
};

struct dmtype_2d_t
{
  unsigned int  w, len;
  const float*  data;
};

struct dmtype_image_t
{
  enum  { NONE, RGB, RGBA, FASTALPHA };
  int           type;
  unsigned int  w, h;
  const void*   data;
};

class IPalette;
struct dmtype_palette_t
{
  const IPalette*   ppal;
  bool              discrete;
};

enum  COORDINATION      { CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                          CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED,
                          
                          CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, 
                          CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                          
                          CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED,
                          CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                          
                          /// SPECIAL COORDINATION for init overlays:
                          CR_SAME,
                          CR_PIXEL=CR_ABSOLUTE_NOSCALED
                        };

class Ovldraw;
class _Ovldraw;
class IOvldrawFriendly
{
protected:
  friend class _Ovldraw;
  IOvldrawFriendly(){}  // no vdestructor, closed constructor
  virtual void overlayUpdate(bool internal, bool noupdate)=0;
  virtual void innerOverlayReplace(int ovlid, Ovldraw* ovl, OVL_ORIENTATION orient, bool owner)=0;
  virtual void innerOverlayRemove(int ovlid)=0;
};
class _Ovldraw
{
public:
  enum { MAXUNIFORMS = 16, MAXDRAWERS = 32 };
private:
  dmtype_t          m_uniforms[MAXUNIFORMS];
  unsigned int      m_uniformsCount;
  unsigned int      m_uniformsCCdelim;  // color <-> coords uniform delimiter
private:
  struct  _ioverlay_repaintable_t
  {
    int                       idoverlay;
    IOvldrawFriendly*     repaintable;
    bool                      delowner;
  }                 m_drawers[MAXDRAWERS];
  unsigned int      m_drawersCount;
  unsigned int      m_pinger_reinit;
  unsigned int      m_pinger_update;
public:
  _Ovldraw(): m_uniformsCount(0), m_uniformsCCdelim(0), m_drawersCount(0), m_pinger_reinit(0), m_pinger_update(0) {}
  virtual ~_Ovldraw()
  {
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->innerOverlayRemove(m_drawers[i].idoverlay);
  }
  friend class OvldrawUpdater;
public:
  struct    uniforms_t
  {
    unsigned int    count;
    unsigned int    ccdelim;
    const dmtype_t* arr;
  };
  uniforms_t  uniforms() const {  uniforms_t result = { m_uniformsCount, m_uniformsCCdelim, m_uniforms }; return result;  }
protected:
  void appendUniform(DTYPE type, const void* value, bool preuniform=false)
  {
    m_uniforms[m_uniformsCount].type = type;
    m_uniforms[m_uniformsCount].dataptr = value;
    m_uniformsCount++;
    if (preuniform) m_uniformsCCdelim++;
  }
protected:
  void updateParameter(bool reinit, bool update)
  {
    if (reinit)   m_pinger_reinit++;
    m_pinger_update++;
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->overlayUpdate(reinit, !update);
  }
  unsigned int    pingerReinit() const { return m_pinger_reinit; }
  unsigned int    pingerUpdate() const { return m_pinger_update; }
  void            increasePingerReinit() { m_pinger_reinit++; }
  void            increasePingerUpdate() { m_pinger_update++; }
public:
  void  eject(Ovldraw* ovl, OVL_ORIENTATION orient=OO_SAME, bool owner=true)
  {
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->innerOverlayReplace(m_drawers[i].idoverlay, ovl, orient, owner); // this method calls assign for ovl
    m_drawersCount = 0;
  }
  void  eject()
  {
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->innerOverlayRemove(m_drawers[i].idoverlay);
    m_drawersCount = 0;
  }
//protected:
private:
  friend class DrawCore;
  bool  assign(int overlay, IOvldrawFriendly* idr, bool delowner)
  {
    if (m_drawersCount >= MAXDRAWERS) return false;
    m_drawers[m_drawersCount].idoverlay = overlay;
    m_drawers[m_drawersCount].repaintable = idr;
    m_drawers[m_drawersCount].delowner = delowner;
    m_drawersCount++;
    return true;
  }
  bool  preDelete(IOvldrawFriendly* idr)
  {
    int  ownermask = 0;
    bool move=false;
    for (unsigned int i=0; i<m_drawersCount; i++)
      if (m_drawers[i].repaintable == idr)
      {
        if (m_drawers[i].delowner == true)
          ownermask |= 2;
        move = true;
      }
      else 
      {
        if (m_drawers[i].delowner == true)
          ownermask |= 1;
        if (move)
          m_drawers[i-1] = m_drawers[i];
      }
    if (move)
      m_drawersCount -= 1;
    return ownermask == 2;
  }
protected:    /// EXTERNAL interface
  friend class DrawQWidget;
  virtual int   fshOVCoords(int overlay, bool switchedab, char* to) const =0;
  virtual int   fshColor(int overlay, char* to) const =0;
};



class DataDecimator
{
public:
  virtual float decimate(const float* srcdata, int dstrange, int i, int portion)=0;
};

typedef float (*datadecimator_func_t)(const float*, unsigned int);
class DataDecimatorFunc: public DataDecimator
{
  datadecimator_func_t  m_fn;
  unsigned int          m_srcsize;
public:
  DataDecimatorFunc(datadecimator_func_t dfn, unsigned int srcsize): m_fn(dfn), m_srcsize(srcsize){}
  virtual float decimate(const float* srcdata, int dstrange, int i, int portion)
  {
    unsigned int stepsize = m_srcsize % dstrange == 0 || i < dstrange - 1? m_srcsize / dstrange : m_srcsize % dstrange;
    return m_fn(&srcdata[portion*m_srcsize + stepsize*i], stepsize);
  }
};

template <typename T>
class DataDecimatorTemplated: public DataDecimator
{
  T             fn;
  unsigned int  m_srcsize;
public:
  DataDecimatorTemplated(unsigned int srcsize): m_srcsize(srcsize){}
  virtual float decimate(const float* srcdata, int dstrange, int i, int portion)
  {
    unsigned int stepsize = m_srcsize % dstrange == 0 || i < dstrange - 1? m_srcsize / dstrange : m_srcsize % dstrange;
    return fn(&srcdata[portion*m_srcsize + stepsize*i], stepsize);
  }
};

////////////////////////////////////////////////////////////////////////////////

struct  color3f_t
{
  float         r,g,b;
};


#if __cplusplus > 19971
#define __color3f__(red, green, blue) {red,green,blue}
#else
inline color3f_t    __color3f__(float red, float green, float blue)
{   color3f_t result = { red,green,blue };   return result;    }
#endif

inline color3f_t    color3f_inverse_1(){ return __color3f__(0,0,0); }
inline color3f_t    color3f_inverse_2(){ return __color3f__(0,0,0); }
inline color3f_t    color3f_inverse_3(){ return __color3f__(0,0,0); }

inline color3f_t    color3f_white(){ return __color3f__(1,1,1); }
inline color3f_t    color3f_yellow(){ return __color3f__(1,1,0); }
inline color3f_t    color3f_black(){ return __color3f__(0,0,0); }
inline color3f_t    color3f_red(){ return __color3f__(1,0,0); }
inline color3f_t    color3f_blue(){ return __color3f__(0,0,1); }
inline color3f_t    color3f_orange(){ return __color3f__(1,0.6f,0.3f); }
inline color3f_t    color3f_redlight(){ return __color3f__(1,0.6f,0.6f); }
inline color3f_t    color3f_green(){ return __color3f__(0,1,0); }
inline color3f_t    color3f_bluelight(){ return __color3f__(0.5f,0.75f,1); }
inline color3f_t    color3f_bluesoft(){ return __color3f__(0.35f,0.65f,1); }
inline color3f_t    color3f_purple(){ return __color3f__(1,0,1); }
inline color3f_t    color3f_grey(){ return __color3f__(0.5f,0.5f,0.5f); }
inline color3f_t    color3f_greylight(){ return __color3f__(0.7f,0.7f,0.7f); }
inline color3f_t    color3f_greydark(){ return __color3f__(0.2f,0.2f,0.2f); }

inline color3f_t    color3f(int colorhex){  return __color3f__((colorhex & 0xFF)/255.0f, (colorhex>>8 & 0xFF)/255.0f, (colorhex>>16 & 0xFF)/255.0f); }
inline color3f_t    color3f(float r, float g, float b){  return __color3f__(r,g,b); }
inline color3f_t    color3f(int r256, int g256, int b256){  return __color3f__(r256/255.0f, g256/255.0f, b256/255.0f); }


////////////////////////////////////////////////////////////////////////////////

enum OUTSIDELINE {  OLS_OFF, OLS_FLAT, OLS_OPACITY_LINEAR, OLS_OPACITY_QUAD, OLS_OUTLINE  };

struct  linestyle_t
{
  unsigned int  lenstroke;
  unsigned int  lenspace;
  unsigned int  countdot;
  float         r,g,b;
  int           inversive;  /// 0 - off, 1-5 algos
  OUTSIDELINE   outside;
};

#if __cplusplus > 19971
#define __linestyle__(lstroke, lspace, dots, red, green, blue, inversive, ols) { lstroke, lspace, dots, red,green,blue, inversive, ols }
#else
inline linestyle_t    __linestyle__(unsigned int lstroke, unsigned int lspace, unsigned int dots, float red, float green, float blue, int inversive, OUTSIDELINE ols)
{   linestyle_t result = {lstroke, lspace, dots, red,green,blue, inversive, ols};   return result;    }
#endif

inline linestyle_t    linestyle_inverse_1(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,0,0, 1, ols); }
inline linestyle_t    linestyle_inverse_2(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,0,0, 2, ols); }
inline linestyle_t    linestyle_inverse_3(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,0,0, 3, ols); }

inline linestyle_t    linestyle_white(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,1,1, 0, ols); }
inline linestyle_t    linestyle_yellow(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,1,0, 0, ols); }
inline linestyle_t    linestyle_black(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,0,0, 0, ols); }
inline linestyle_t    linestyle_red(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,0,0, 0, ols); }
inline linestyle_t    linestyle_blue(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,0,1, 0, ols); }
inline linestyle_t    linestyle_orange(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,0.6f,0.3f, 0, ols); }
inline linestyle_t    linestyle_redlight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,0.6f,0.6f, 0, ols); }
inline linestyle_t    linestyle_green(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0,1,0, 0, ols); }
inline linestyle_t    linestyle_bluelight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0.5f,0.75f,1, 0, ols); }
inline linestyle_t    linestyle_bluesoft(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0.35f,0.65f,1, 0, ols); }
inline linestyle_t    linestyle_purple(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 1,0,1, 0, ols); }
inline linestyle_t    linestyle_grey(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0.5f,0.5f,0.5f, 0, ols); }
inline linestyle_t    linestyle_greylight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0.7f,0.7f,0.7f, 0, ols); }
inline linestyle_t    linestyle_greydark(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(lstroke, lspace, dots, 0.2f,0.2f,0.2f, 0, ols); }

inline linestyle_t    linestyle_solid(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(1, 0, 0, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_slimstroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(3, 1, 0, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_stroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(5, 2, 0, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_widestroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(10, 2, 0, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_dots(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(1, 1, 0, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_strdot(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(4, 2, 1, red,green,blue, 0, ols); }
inline linestyle_t    linestyle_strdotdot(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return __linestyle__(4, 2, 2, red,green,blue, 0, ols); }

inline linestyle_t&   linestyle_update(linestyle_t* ls, float red, float green, float blue){ ls->r = red; ls->g = green; ls->b = blue; ls->inversive = 0; return *ls; }
inline linestyle_t    linestyle_update(linestyle_t ls, float red, float green, float blue){ ls.r = red; ls.g = green; ls.b = blue; ls.inversive = 0; return ls; }
inline linestyle_t    linestyle_update(linestyle_t ls, unsigned int clr){   ls.b = ((clr >> 16) & 0xFF)/255.0f;    ls.g = ((clr >> 8) & 0xFF)/255.0f; ls.r = ((clr) & 0xFF)/255.0f; ls.inversive = 0; return ls; }
inline linestyle_t    linestyle_update_inverse(linestyle_t ls, int inv){   ls.r = ls.g = ls.b = 0; ls.inversive = inv; return ls; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct impulsedata_t
{
  enum { MAXCOEFFS = 31 };
  enum IMPULSE_TYPE {     IR_OFF, IR_A_COEFF, IR_A_COEFF_NOSCALED, IR_A_BORDERS, IR_A_BORDERS_FIXEDCOUNT,
                                  IR_B_COEFF, IR_B_COEFF_NOSCALED, IR_B_BORDERS, IR_B_BORDERS_FIXEDCOUNT
                    };
  int type;
  int count;      // for IR_X_COEFF and IR_X_COEFF_NOSCALED - count of coeffs. For IR_X_BORDERS+ - minimal scaling for activation
  int central;    // for IR_X_COEFF and IR_X_COEFF_NOSCALED - central coeff. For IR_X_BORDERS+ - starter count for activation scaling
  
  enum  FLAGS { F_DEFAULT=0, F_CYCLED=0x1, F_CLAMPTOP=0x2, F_CLAMPBOT=0x4 };
  int flags;
  float coeff[MAXCOEFFS]; // for IR_X_COEFF and IR_X_COEFF_NOSCALED - coeffs pack. For IR_X_BORDERS - only 1st special coeff
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


enum BSOVERPATTERN_INT
{  
    OP_CONTOUR, OP_LINELEFT, OP_LINERIGHT, OP_LINEBOTTOM, OP_LINETOP, 
    OP_LINELEFTRIGHT, OP_LINEBOTTOMTOP, OP_LINELEFTBOTTOM, OP_LINERIGHTBOTTOM, OP_LINELEFTTOP, OP_LINERIGHTTOP,
    OP_GRID, OP_DOT, OP_DOTLEFTBOTTOM, OP_DOTCONTOUR,  OP_SHTRICHL, OP_SHTRICHR, 
    OP_CROSS, OP_FILL, OP_SQUARES,
    _OP_TOTAL
};

enum BSOVERPATTERN_FLOAT
{
    OPF_CIRCLE, OPF_CIRCLE_REV, OPF_CROSSPUFF, OPF_RHOMB, OPF_SURIKEN, OPF_SURIKEN_REV, OPF_DONUT, OPF_CROSS, 
    OPF_UMBRELLA, OPF_HOURGLASS, OPF_STAR, OPF_BULL, OPF_BULR, OPF_CROSSHAIR,
    _OPF_TOTAL
};

enum BSOVERPATTERN_ANGLEFIGURES
{
    OPA_PUFFHYPERB, OPA_PUFFCIRCLE, OPA_PUFFCIRCLE_SPACED125, OPA_PUFFCIRCLE_SPACED15, OPA_PUFFCIRCLE_SPACED2, OPA_PUFFCIRCLE_SPACED25,
    _OPA_TOTAL
};

struct  overpattern_t     // Brush over scaling
{
  enum {  OALG_OFF=0, OALG_THRS_PLUS=1, OALG_THRS_MINUS=2, OALG_ANY=3 };
  int               algo;         // OALG_...
  enum {  OMASK_INT, OMASK_FLOAT, OMASK_ANGLEFIGURES };
  int               masktype;     // true if OP_ else OPF_
  int               mask;         // OP_... or OPF_...
  float             threshold;    /// for 2D draws
  float             weight;
  float             smooth;       /// 0..1, only for OPF_
  bool              colorByPalette;
  color3f_t         color; /// activates if colorByPalette == false
};

#if __cplusplus > 19971
#define __overpattern_cp__(algo, masktype, mask, thrs, weight, smooth, colorByPalette) { algo, masktype, mask, thrs, weight, smooth, true, {colorByPalette,colorByPalette,colorByPalette} }
#define __overpattern_cf__(algo, masktype, mask, thrs, weight, smooth, color3f) { algo, masktype, mask, thrs, weight, smooth, false, color3f }
#else
inline overpattern_t    __overpattern_cp__(int algo, bool masktype, int mask, float threshold, int weight, float smooth, float colorByPalette)
{   overpattern_t result = { algo, bool masktype, mask, thrs, float(weight), smooth, true, {colorByPalette,colorByPalette,colorByPalette} };   return result;    }
inline overpattern_t    __overpattern_cf__(int algo, int mask, float threshold, int weight, float smooth, color3f_t color3f)
{   overpattern_t result = { algo, bool masktype, mask, thrs, float(weight), smooth, false, color3f };   return result;    }
#endif

inline overpattern_t    overpattern_off(){ return __overpattern_cf__(overpattern_t::OALG_OFF, true, 0, 0, 0, 0, color3f_white()); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_INT OP_type, int colorhex, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_INT, OP_type, 0.0f, float(weight), 0.0f, color3f(colorhex)); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_INT OP_type, const color3f_t& color, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_INT, OP_type, 0.0f, float(weight), 0.0f, color); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_INT OP_type, float colorByPalette, int weight=0){ return __overpattern_cp__(overpattern_t::OALG_ANY, overpattern_t::OMASK_INT, OP_type, 0.0f, float(weight), 0.0f, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_INT OP_type, float threshold, int colorhex, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_INT OP_type, float threshold, const color3f_t& color, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, color); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_INT OP_type, float threshold, float colorByPalette, int weight=0){ return __overpattern_cp__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_INT OP_type, float threshold, int colorhex, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_INT OP_type, float threshold, const color3f_t& color, int weight=0){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, color); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_INT OP_type, float threshold, float colorByPalette, int weight=0){ return __overpattern_cp__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_INT, OP_type, threshold, float(weight), 0.0f, colorByPalette); }

inline overpattern_t    overpattern_any(BSOVERPATTERN_FLOAT OP_type, int colorhex, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_FLOAT, OP_type, 0.0f, float(weight01), smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_FLOAT OP_type, const color3f_t& color, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_FLOAT, OP_type, 0.0f, float(weight01), smooth01, color); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_FLOAT OP_type, float colorByPalette, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_ANY, overpattern_t::OMASK_FLOAT, OP_type, 0.0f, float(weight01), smooth01, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_FLOAT OP_type, float threshold, int colorhex, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_FLOAT OP_type, float threshold, const color3f_t& color, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, color); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_FLOAT OP_type, float threshold, float colorByPalette, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_FLOAT OP_type, float threshold, int colorhex, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_FLOAT OP_type, float threshold, const color3f_t& color, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, color); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_FLOAT OP_type, float threshold, float colorByPalette, float weight01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_FLOAT, OP_type, threshold, weight01, smooth01, colorByPalette); }

inline overpattern_t    overpattern_any(BSOVERPATTERN_ANGLEFIGURES OP_type, int colorhex, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_ANGLEFIGURES, OP_type, 0.0f, float(figdist01), smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_ANGLEFIGURES OP_type, const color3f_t& color, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_ANY, overpattern_t::OMASK_ANGLEFIGURES, OP_type, 0.0f, float(figdist01), smooth01, color); }
inline overpattern_t    overpattern_any(BSOVERPATTERN_ANGLEFIGURES OP_type, float colorByPalette, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_ANY, overpattern_t::OMASK_ANGLEFIGURES, OP_type, 0.0f, float(figdist01), smooth01, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, int colorhex, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, const color3f_t& color, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, color); }
inline overpattern_t    overpattern_thrs_plus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, float colorByPalette, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_THRS_PLUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, colorByPalette); }
///
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, int colorhex, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, color3f(colorhex)); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, const color3f_t& color, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cf__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, color); }
inline overpattern_t    overpattern_thrs_minus(BSOVERPATTERN_ANGLEFIGURES OP_type, float threshold, float colorByPalette, float figdist01=0.5f, float smooth01=0.5f){ return __overpattern_cp__(overpattern_t::OALG_THRS_MINUS, overpattern_t::OMASK_ANGLEFIGURES, OP_type, threshold, figdist01, smooth01, colorByPalette); }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


enum  OVL_REACTION_MOUSE { 
  ORM_LMPRESS, ORM_LMMOVE, ORM_LMRELEASE, ORM_LMDOUBLE,
  ORM_RMPRESS, ORM_RMMOVE, ORM_RMRELEASE
};
enum  OVL_MODIFIER_KEYBOARD {  OMK_NONE=0, OMK_SHIFT=2, OMK_CONTROL=4, OMK_ALT=8 };

struct coordstriumv_t
{
  float   fx_ovl, fy_ovl;   // coords [0..1)    (for overlay evals)
  float   fx_pix, fy_pix;   // screen coords -margins *devicePixelRatio
  float   fx_rel, fy_rel;   // coords [0..1]    (for strict calculations)
};

struct coordstriumv_ex_t: coordstriumv_t
{
  int     ex_r, ex_c;
//  float   ex_fx_inside, ex_fy_inside;
  int     ex_count_rows, ex_count_columns;
};


class IOverlayReactor
{
public:
  virtual void  overlayReactionVisible(bool /*visible*/){}    // nothrow
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/){  return false; }   // return doUPDATE
  virtual bool  overlayReactionKey(int /*key*/, int /*modifiersOMK*/, bool* /*doStop*/){  return false; }             // return doUPDATE
  virtual ~IOverlayReactor(){}
};

struct ovlbasics_t
{
  float   opacity;
  float   thickness;
  float   slice_ll;
  float   slice_hl;
};

class Ovldraw: virtual public _Ovldraw
{
private:
  bool                    m_visible;
  ovlbasics_t             m_ots;
protected:
  Ovldraw(bool visible): m_visible(visible)
  {
    m_ots.opacity = 0.0f;
    m_ots.thickness = 0.0f;
    m_ots.slice_ll = -1e+8f;
    m_ots.slice_hl =  1e+8f;
  }
public:
  void  setOpacity(float opacity, bool update=true){ m_ots.opacity = opacity; updateParameter(false, update); }  /// 1.0f for invisible
  float getOpacity() const { return m_ots.opacity; }
  bool  opaque() const {  return m_ots.opacity >= 1.0f; }
  void  setThickness(float thickness, bool update=true){ m_ots.thickness = thickness; updateParameter(false, update); }
  float getThickness() const { return m_ots.thickness; }
  void  setSlice(float level_low, float level_high, bool update=true) { m_ots.slice_ll = level_low; m_ots.slice_hl = level_high; updateParameter(false, update); }
  void  setSlice(float level_high, bool update=true) { m_ots.slice_hl = level_high; updateParameter(false, update); }
  void  setSliceLL(float level, bool update=true) { m_ots.slice_ll = level; updateParameter(false, update); }
  void  setSliceHL(float level, bool update=true) { m_ots.slice_hl = level; updateParameter(false, update); }
  float getSliceLL() const { return m_ots.slice_ll; }
  float getSliceHL() const { return m_ots.slice_hl; }
public:
  void  setVisible(bool visible, bool update=true){ m_visible = visible;  updateParameter(false, update); }
  bool  isVisible() const { return m_visible; }
public:
  void  setOTS(const ovlbasics_t& ob, bool update=true){ m_ots = ob; updateParameter(false, update); }
  void  setOTS(float opacity, float thickness, float slice_low, float slice_high, bool update=true)
  { m_ots.opacity = opacity; m_ots.thickness = thickness; m_ots.slice_ll = slice_low; m_ots.slice_hl = slice_high; updateParameter(false, update); }
protected:
  friend class DrawCore;
  virtual IOverlayReactor*  reactor() { return nullptr; }
};

class OvldrawUpdater
{
  _Ovldraw* pOvl;
public:
  OvldrawUpdater(_Ovldraw* povl): pOvl(povl){}
  void updateParameter(bool recreate, bool update){ pOvl->updateParameter(recreate, update); }
  void appendUniform(DTYPE type, const void* value){  pOvl->appendUniform(type, value); }
};


/////////////////////////////////////////////
/////////////////////////////////////////////


struct ovlfraginfo_t
{
  int               link;
  OVL_ORIENTATION   orient;
};

class ISheiGenerator
{
public:
  virtual   const char*   shaderName() const =0;
  virtual   unsigned int  shvertex_pendingSize() const =0;
  virtual   unsigned int  shvertex_store(char* to) const =0;
  virtual   unsigned int  shfragment_pendingSize(const impulsedata_t&, unsigned int ovlscount) const =0;
  virtual   unsigned int  shfragment_store(unsigned int allocPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                           const impulsedata_t&, const overpattern_t&, float, 
                                           unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const =0;
  
public:
  enum      { PMT_PSEUDO2D, PMT_FORCE1D }; /// PORTION_MESH_TYPE
  virtual   int           portionMeshType() const =0;
  virtual   ~ISheiGenerator(){}
};

#endif // BSDRAWDEF_H
