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

enum  DRAWVIEWALIGN     { DVA_LEFT=0,  DVA_CENTER,  DVA_RIGHT
                        };

enum  SPLITPORTIONS     { SL_NONE=0,
                          SL_VERT =0x0001,  SL_HORZ =0x0101,  SL_VERT2=0x0002,  SL_HORZ2=0x0102,
                          SL_VERT3=0x0003,  SL_HORZ3=0x0103,  SL_VERT4=0x0004,  SL_HORZ4=0x0104,
                          SL_VERT5=0x0005,  SL_HORZ5=0x0105,  SL_VERT6=0x0006,  SL_HORZ6=0x0106,
                          SL_VERT7=0x0007,  SL_HORZ7=0x0107,  SL_VERT8=0x0008,  SL_HORZ8=0x0108,
                          SL_VERT9=0x0009,  SL_HORZ9=0x0109,  SL_VERT10=0x000A,  SL_HORZ10=0x010A,
                          SL_VERT11=0x000B,  SL_HORZ11=0x010B,  SL_VERT12=0x000C,  SL_HORZ12=0x010C,
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



enum BSPOSTMASKOVER {  PO_OFF=0, PO_SIGNAL=1, PO_EMPTY=2, PO_ALL=3 };
enum BSPOSTMASKTYPE {  
                  PM_CONTOUR, PM_LINELEFT, PM_LINERIGHT, PM_LINEBOTTOM, PM_LINETOP,
                  PM_LINELEFTBOTTOM, PM_LINERIGHTBOTTOM, PM_LINELEFTTOP, PM_LINERIGHTTOP,
                  PM_CIRCLESMOOTH, PM_CIRCLEBORDERED, PM_DOT, 
                  PM_DOTLEFTBOTTOM, PM_DOTCONTOUR,
                  PM_SHTRICHL, PM_SHTRICHR, PM_CROSS, PM_GRID, PM_FILL, PM_SQUARES  
};

struct  DPostmask
{
  BSPOSTMASKOVER    over;
  BSPOSTMASKTYPE    mask;
  int               weight;
  int               colorManual;
  float             colorPalette; /// activates if colorManual == -1
  float             threshold;    /// for 2D draws
  static DPostmask  postmask(BSPOSTMASKOVER over, BSPOSTMASKTYPE mask, int weight, float r, float g, float b, float emptyThreshold=0.0f)
  { int clr = bscolor3fToint(r, g, b);  DPostmask result = { over, mask, weight, clr, 0.0f, emptyThreshold }; return result;  }
  static DPostmask  postmask(BSPOSTMASKOVER over, BSPOSTMASKTYPE mask, int weight, int colorhex, float emptyThreshold=0.0f)
  { DPostmask result = { over, mask, weight, colorhex, 0.0f, emptyThreshold };      return result;  }
  static DPostmask  postmask(BSPOSTMASKOVER over, BSPOSTMASKTYPE mask, int weight, float colorByPalette=0.0f, float emptyThreshold=0.0f)
  { DPostmask result = { over, mask, weight, -1, colorByPalette, emptyThreshold };  return result;  }
  
  static DPostmask  empty()
  { DPostmask result = { PO_OFF, PM_CONTOUR, 0, 0, 0.0f, 0.0f };  return result;  }
};

enum  DTYPE       /// Trace/simple shader datatypes
{
              DT_1F, DT_2F, DT_3F, DT_4F, 
              DT_ARR,   DT_ARR2,  DT_ARR3, DT_ARR4, 
              DT_ARRI, DT_ARRI2, DT_ARRI3, DT_ARRI4, 
              DT_SAMP4, DT_1I, DT_2I, DT_3I, DT_4I,
              DT_TEXTURE, DT_PALETTE,
              
              /// special Color shader datatypes (Hard)
              DT__HC_SPECIAL_TYPES=100, DT__HC_PALETTE
};

inline bool dtIsTexture(DTYPE dtype) { return dtype == DT_SAMP4 || dtype == DT_TEXTURE || dtype == DT_PALETTE || dtype == DT__HC_PALETTE; }

struct dmtype_t
{
  DTYPE         type;
  const void*   dataptr;
};

struct dmtype_sampler_t
{
  unsigned int  count;
  float*        data;
};

struct dmtype_arr_t
{
  unsigned int  count;
  const void*   data;
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

class DrawOverlay;
class _DrawOverlay;
class IDrawOverlayFriendly
{
protected:
  friend class _DrawOverlay;
  IDrawOverlayFriendly(){}  // no vdestructor, closed constructor
  virtual void overlayUpdate(bool internal, bool noupdate)=0;
  virtual void innerOverlayReplace(int ovlid, DrawOverlay* ovl, bool owner)=0;
  virtual void innerOverlayRemove(int ovlid)=0;
};

class _DrawOverlay
{
public:
  enum { MAXUNIFORMS = 10, MAXDRAWERS = 50 };
private:
  dmtype_t          m_uniforms[MAXUNIFORMS];
  unsigned int      m_uniformsCount;
private:
  struct  _ioverlay_repaintable_t
  {
    int                       idoverlay;
    IDrawOverlayFriendly*     repaintable;
    bool                      delowner;
  }                 m_drawers[MAXDRAWERS];
  unsigned int      m_drawersCount;
  unsigned int      m_pinger_reinit;
  unsigned int      m_pinger_update;
public:
  _DrawOverlay(): m_uniformsCount(0), m_drawersCount(0), m_pinger_reinit(1), m_pinger_update(1) {}
  virtual ~_DrawOverlay()
  {
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->innerOverlayRemove(m_drawers[i].idoverlay);
  }
  friend class DrawOverlayUpdater;
public:
  struct    uniforms_t
  {
    unsigned int    count;
    const dmtype_t* arr;
  };
  uniforms_t  uniforms() const {  uniforms_t result = { m_uniformsCount, m_uniforms }; return result;  }
protected:
  void appendUniform(DTYPE type, const void* value)
  {
    m_uniforms[m_uniformsCount].type = type;
    m_uniforms[m_uniformsCount].dataptr = value;
    m_uniformsCount++;
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
  void  eject(DrawOverlay* ovl, bool owner)
  {
    for (unsigned int i=0; i<m_drawersCount; i++)
      m_drawers[i].repaintable->innerOverlayReplace(m_drawers[i].idoverlay, ovl, owner); // this method calls assign for ovl
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
  bool  assign(int overlay, IDrawOverlayFriendly* idr, bool delowner)
  {
    if (m_drawersCount >= MAXDRAWERS) return false;
    m_drawers[m_drawersCount].idoverlay = overlay;
    m_drawers[m_drawersCount].repaintable = idr;
    m_drawers[m_drawersCount].delowner = delowner;
    m_drawersCount++;
    return true;
  }
  bool  preDelete(IDrawOverlayFriendly* idr)
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
  virtual int   fshTrace(int overlay, bool rotated, char* to) const =0;
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
#define __linestyle__(lstroke, lspace, dots, red, green, blue, inversive, ols) {lstroke, lspace, dots, red,green,blue, inversive, ols}
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
  int cycled;
  float coeff[MAXCOEFFS]; // for IR_X_COEFF and IR_X_COEFF_NOSCALED - coeffs pack. For IR_X_BORDERS - only 1st special coeff
};

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
  float   slice;
};

class DrawOverlay: virtual public _DrawOverlay
{
private:
  bool                    m_visible;
  ovlbasics_t             m_ots;
protected:
  DrawOverlay(bool visible): m_visible(visible)
  {
    m_ots.opacity = 0.0f;
    m_ots.thickness = 0.0f;
    m_ots.slice = 1.0f;
  }
public:
  void  setOpacity(float opacity, bool update=true){ m_ots.opacity = opacity; updateParameter(false, update); }  /// 1.0f for invisible
  float getOpacity() const { return m_ots.opacity; }
  bool  opaque() const {  return m_ots.opacity >= 1.0f; }
  void  setThickness(float thickness, bool update=true){ m_ots.thickness = thickness; updateParameter(false, update); }
  float getThickness() const { return m_ots.thickness; }
  void  setSlice(float value, bool update=true) { m_ots.slice = value; updateParameter(false, update); }
  float getSlice() const { return m_ots.slice; }
public:
  void  setVisible(bool visible, bool update=true){ m_visible = visible;  updateParameter(false, update); }
  bool  isVisible() const { return m_visible; }
public:
  void  setOTS(const ovlbasics_t& ob, bool update=true){ m_ots = ob; updateParameter(false, update); }
  void  setOTS(float opacity, float thickness, float slice, bool update=true){ m_ots.opacity = opacity; m_ots.thickness = thickness; m_ots.slice = slice; updateParameter(false, update); }
protected:
  friend class DrawCore;
  virtual IOverlayReactor*  reactor() { return nullptr; }
};

class DrawOverlayUpdater
{
  _DrawOverlay* pOvl;
public:
  DrawOverlayUpdater(_DrawOverlay* povl): pOvl(povl){}
  void updateParameter(bool recreate, bool update){ pOvl->updateParameter(recreate, update); }
  void appendUniform(DTYPE type, const void* value){  pOvl->appendUniform(type, value); }
};


/////////////////////////////////////////////
/////////////////////////////////////////////


struct ovlfraginfo_t
{
  int   link;
};

class ISheiGenerator
{
public:
  virtual   const char*   shaderName() const =0;
  virtual   unsigned int  shvertex_pendingSize() const =0;
  virtual   unsigned int  shvertex_store(char* to) const =0;
  virtual   unsigned int  shfragment_pendingSize(const impulsedata_t&, unsigned int ovlscount) const =0;
  virtual   unsigned int  shfragment_store(unsigned int allocPortions, const DPostmask&, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                           const impulsedata_t&, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const =0;
  
public:
  enum      { PMT_PSEUDO2D, PMT_FORCE1D }; /// PORTION_MESH_TYPE
  virtual   int           portionMeshType() const =0;
  virtual   ~ISheiGenerator(){}
};

#endif // BSDRAWDEF_H
