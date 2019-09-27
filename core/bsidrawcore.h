#ifndef BSDRAWDEF_H
#define BSDRAWDEF_H

/// Technical header

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

struct  DPostmask
{
  enum DPOVER   {  PO_OFF=0, PO_SIGNAL=1, PO_EMPTY=2, PO_ALL=3 };
  DPOVER        over;
  enum DPMASK   {  PM_CONTOUR, PM_LINELEFT, PM_LINERIGHT, PM_LINEBOTTOM, PM_LINETOP,
                   PM_LINELEFTBOTTOM, PM_LINERIGHTBOTTOM, PM_LINELEFTTOP, PM_LINERIGHTTOP,
                   PM_PSEUDOCIRCLE, PM_DOT, 
                   PM_DOTLEFTBOTTOM, PM_DOTCONTOUR,
                   PM_SHTRICHL, PM_SHTRICHR, PM_CROSS  };
  DPMASK        postmask;
  int           weight;
  int           colorManual;
  float         colorPalette; /// activates if colorManual == -1
  float         threshold;    /// for 2D draws
  DPostmask(DPOVER over_, DPMASK mask, int weight_, float r_, float g_, float b_, float emptyThreshold=0.0f):
  over(over_), postmask(mask), weight(weight_), threshold(emptyThreshold) { colorManual = bscolor3fToint(r_, g_, b_); }
  DPostmask(DPOVER over_, DPMASK mask, int weight_, int colorhex, float emptyThreshold=0.0f):
  over(over_), postmask(mask), weight(weight_), colorManual(colorhex), threshold(emptyThreshold){}
  DPostmask(DPOVER over_, DPMASK mask, int weight_, float colorByPalette=0.0f, float emptyThreshold=0.0f):
  over(over_), postmask(mask), weight(weight_), colorManual(-1), colorPalette(colorByPalette), threshold(emptyThreshold){}
};

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
  virtual   unsigned int  shfragment_pendingSize(unsigned int ovlscount) const =0;
  virtual   unsigned int  shfragment_store(unsigned int allocPortions, const DPostmask&, ORIENTATION orient, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const =0;
  
public:
  enum      { PMT_PSEUDO2D, PMT_FORCE1D }; /// PORTION_MESH_TYPE
  virtual   int           portionMeshType() const =0;
  virtual   ~ISheiGenerator(){}
};

enum  DTYPE { /// Trace/simple shader datatypes
              DT_1F, DT_2F, DT_3F, DT_4F, 
              DT_ARR,   DT_ARR2,  DT_ARR3, DT_ARR4, 
              DT_ARRI, DT_ARRI2, DT_ARRI3, DT_ARRI4, 
              DT_SAMP4, DT_1I, DT_2I, DT_3I, DT_4I,
              DT_TEX, DT_TEXA, DT_TEXT, 
              
              /// special Color shader datatypes (Hard)
              DT__HC_SPECIAL_TYPES=100, DT__HC_PALETTE };

inline bool dtIsTexture(DTYPE dtype) { return dtype == DT_SAMP4 || dtype == DT_TEX || dtype == DT_TEXA || dtype == DT_TEXT || dtype == DT__HC_PALETTE; }

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

class AbstractOverlay
{
public:
  enum { MAXUNIFORMS = 10 };
private:
  dmtype_t          m_uniforms[MAXUNIFORMS];
  unsigned int      m_uniformsCount;
public:
  AbstractOverlay(): m_uniformsCount(0) {}
  virtual ~AbstractOverlay(){}
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
  virtual void overlayUpdateParameter(bool recreate=false) =0;
  friend class IOverlayUpdater;
};

class IDrawOverlayFriendly
{
protected:
  IDrawOverlayFriendly(){}  // no vdestructor, closed constructor
  virtual void overlayUpdate(int overlay, bool internal, bool noupdate, bool recreate)=0;
  virtual void innerOverlayRemove(int overlay)=0;
  friend class IOverlay;
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
  int           inversive;  /// -1 - off, 0-4 algos
  OUTSIDELINE   outside;
  linestyle_t(unsigned int lstroke, unsigned int lspace, unsigned int dots, float red, float green, float blue, OUTSIDELINE ols): 
    lenstroke(lstroke), lenspace(lspace), countdot(dots), r(red), g(green), b(blue), inversive(-1), outside(ols){}
};
inline linestyle_t    linestyle_inverse_1(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ linestyle_t result(lstroke, lspace, dots, 0,0,0, ols); result.inversive = 0; return result; }
inline linestyle_t    linestyle_inverse_2(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ linestyle_t result(lstroke, lspace, dots, 0,0,0, ols); result.inversive = 1; return result; }
inline linestyle_t    linestyle_inverse_3(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ linestyle_t result(lstroke, lspace, dots, 0,0,0, ols); result.inversive = 2; return result; }

inline linestyle_t    linestyle_white(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,1,1, ols); }
inline linestyle_t    linestyle_yellow(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,1,0, ols); }
inline linestyle_t    linestyle_black(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0,0,0, ols); }
inline linestyle_t    linestyle_red(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,0,0, ols); }
inline linestyle_t    linestyle_orange(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,0.6,0.3, ols); }
inline linestyle_t    linestyle_redlight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,0.6,0.6, ols); }
inline linestyle_t    linestyle_green(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0,1,0, ols); }
inline linestyle_t    linestyle_bluelight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0.6,1,1, ols); }
inline linestyle_t    linestyle_purple(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 1,0,1, ols); }
inline linestyle_t    linestyle_grey(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0.5,0.5,0.5, ols); }
inline linestyle_t    linestyle_greylight(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0.7,0.7,0.7, ols); }
inline linestyle_t    linestyle_greydark(unsigned int lstroke, unsigned int lspace, unsigned int dots, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(lstroke, lspace, dots, 0.2,0.2,0.2, ols); }

inline linestyle_t    linestyle_solid(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(1, 0, 0, red,green,blue, ols); }
inline linestyle_t    linestyle_slimstroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(3, 1, 0, red,green,blue, ols); }
inline linestyle_t    linestyle_stroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(5, 2, 0, red,green,blue, ols); }
inline linestyle_t    linestyle_widestroks(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(10, 2, 0, red,green,blue, ols); }
inline linestyle_t    linestyle_dots(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(1, 1, 0, red,green,blue, ols); }
inline linestyle_t    linestyle_strdot(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(4, 2, 1, red,green,blue, ols); }
inline linestyle_t    linestyle_strdotdot(float red, float green, float blue, OUTSIDELINE ols=OLS_OPACITY_LINEAR){ return linestyle_t(4, 2, 2, red,green,blue, ols); }

inline linestyle_t&   linestyle_update(linestyle_t* ls, float red, float green, float blue){ ls->r = red; ls->g = green; ls->b = blue; return *ls; }
inline linestyle_t    linestyle_update(linestyle_t ls, float red, float green, float blue){ ls.r = red; ls.g = green; ls.b = blue; return ls; }



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum  OVL_REACTION { OR_LMPRESS, OR_LMMOVE, OR_LMRELEASE };

class IOverlay: virtual public AbstractOverlay
{
public:
  enum    { MAXDRAWERS = 50 };
private:
  struct  _ioverlay_repaintable_t
  {
    int                   idoverlay;
    IDrawOverlayFriendly*     repaintable;
    bool                  delowner;
  } m_drawers[MAXDRAWERS];
  unsigned int            m_drawersCount;
  float                   m_opacity, m_density;
  float                   m_slice;                // big float by default
  bool                    m_repaintban;
protected:
  IOverlay(): m_drawersCount(0), m_opacity(0.0f), m_density(0.0f), 
    m_slice(1.0f), //m_slice(1e+9f), 
    m_repaintban(false) {}
public:
  virtual ~IOverlay(){ for (unsigned int i=0; i<m_drawersCount; i++)  m_drawers[i].repaintable->innerOverlayRemove(m_drawers[i].idoverlay); }
private:
  friend class DrawCore;
  bool  assign(int overlay, IDrawOverlayFriendly* idr, bool delowner)
  {
    if (m_drawersCount >= MAXDRAWERS) return false;
    
//    overlayAttaching(m_drawersCount);
    
    m_drawers[m_drawersCount].idoverlay = overlay;
    m_drawers[m_drawersCount].repaintable = idr;
    m_drawers[m_drawersCount].delowner = delowner;
    m_drawersCount++;
    return true;
  }
  bool  overlayPreDelete(IDrawOverlayFriendly* idr)
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
          m_drawers[i - 1] = m_drawers[i];
      }
    if (move)
      m_drawersCount -= 1;
    return ownermask == 2;
  }
public:
  void  setOpacity(float opacity){ m_opacity = opacity; updateParameter(false); } /// 0 for invisible
  float getOpacity() const { return m_opacity; }
  bool  opaque() const {  return m_opacity >= 1.0f; }
  void  setDensity(float density){ m_density = density; updateParameter(false); }
  float getDensity() const { return m_density; }
  void  setSlice(float value) { m_slice = value;  updateParameter(false); }
  float getSlice() const { return m_slice; }
public:
  void  setUpdateBan(bool updateban) { m_repaintban = updateban; }
protected:
  void updatePublic(){ for (unsigned int i=0; i<m_drawersCount; i++)  m_drawers[i].repaintable->overlayUpdate(m_drawers[i].idoverlay, true, false, false); }
  void updateParameter(bool recreate){ for (unsigned int i=0; i<m_drawersCount; i++)  m_drawers[i].repaintable->overlayUpdate(m_drawers[i].idoverlay, false, m_repaintban, recreate); }
  virtual void overlayUpdateParameter(bool recreate=false){ updateParameter(recreate); }
  friend class IOverlayUpdater;
protected:    /// EXTERNAL interface
  friend class DrawQWidget;
  virtual int   fshTrace(int overlay, bool rotated, char* to) const =0;
  virtual int   fshColor(int overlay, char* to) const =0;
  virtual bool  overlayReaction(OVL_REACTION, const void*, bool*){  return false; }
protected:
//  virtual void  overlayAttaching(unsigned int /*ctr*/){}
};

class IOverlayUpdater
{
  IOverlay*   pOvl;
public:
  IOverlayUpdater(IOverlay* povl): pOvl(povl){}
  void updatePublic(){  pOvl->updatePublic(); }
  void updateParameter(bool recreate){ pOvl->updateParameter(recreate); }
  void appendUniform(DTYPE type, const void* value){  pOvl->appendUniform(type, value); }
};

#endif // BSDRAWDEF_H
