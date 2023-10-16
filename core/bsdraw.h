#ifndef DRAWCORE_H
#define DRAWCORE_H

/// This file contains base class for all types of draws
/// You dont need to use classes from this file directly
/// For deriving your subclasses use DrawQWidget, this class better shows interface
/// Created By: Elijah Vlasov

#ifndef BSOVERLAYSLIMIT
// Using default bigsized overlay
#endif
#ifndef BSOVLUNIFORMSLIMIT
// Using default uniforms limit
#endif

#include "bsidrawcore.h"
#include "bsoverlay.h"
#include "../palettes/bsipalette.h"

//#########################
struct  bounds_t
{
  float   LL, HL;             /// Lowest and highest colors from palette
  bounds_t(float ll=0, float hl=1): LL(ll), HL(hl){}
};

inline float length(const bounds_t& bnd){ return bnd.HL - bnd.LL; }
inline float pos01(float value, const bounds_t& bnd){ return (value - bnd.LL)/(bnd.HL - bnd.LL); }

inline  void  _bsdraw_update_kb(const bounds_t& bnd, float* k, float *b)
{
  *k = 1.0f/(bnd.HL - bnd.LL);
  *b = -bnd.LL*1.0f/(bnd.HL - bnd.LL);
}
inline  void  _bsdraw_update_kinvb(const bounds_t& bnd, float* kinv, float *b)
{
  *kinv = (bnd.HL - bnd.LL);
  *b = -bnd.LL*1.0f/(bnd.HL - bnd.LL);
}
inline  void  _bsdraw_update_bnd(const float k, const float b, bounds_t* bnd)
{
  bnd->LL = -b/k;
  bnd->HL = (1.0f - b)/k;
}

//#########################

class OvldrawEmpty: public Ovldraw
{
public:
  OvldrawEmpty(): Ovldraw(true){}
  virtual int  fshOVCoords(int, bool, char*) const;
  virtual int  fshColor(int, char*) const;
};

//#########################

class DrawEventReactor
{
public:
  virtual bool  reactionMouse(class DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/){  return false; }
  virtual bool  reactionKey(class DrawQWidget*, int /*key*/, int /*modifiersOMK*/, bool* /*doStop*/){  return false; }
  virtual ~DrawEventReactor(){}
};

//#########################

struct dcsizecd_t        /// drawcore size components (for 1) direction
{
  unsigned int  dimm;
  unsigned int  scaling;
  unsigned int  splitter;
};

inline unsigned int length(const dcsizecd_t& s){ return s.dimm*s.scaling*s.splitter; }

//#########################


class DrawCore: public IOvldrawFriendly
{
protected:
  enum  DATAASTEXTURE  {  DATEX_2D, DATEX_15D, DATEX_1D, DATEX_DD, DATEX_POLAR };
  DATAASTEXTURE         m_datex;
  bounds_t              m_bounds;
  float                 m_loc_k, m_loc_b;
  unsigned int          m_portionSize;        /// inner sizeof data
  unsigned int          m_allocatedPortions;
  unsigned int          m_countPortions;      /// count of parallel drawings in one
  
  ORIENTATION           m_orient;
  SPLITPORTIONS         m_splitPortions;

  float*                m_dataStorage;
  float*                m_dataStorageCached;
  unsigned int          m_dataDimmA;
  unsigned int          m_dataDimmB;
  bool                  m_dataDimmSwitchAB;
  
  bool                  m_rawResizeModeNoScaled;

  unsigned int          m_scalingA;
  unsigned int          m_scalingB;
  unsigned int          m_scalingAMin, m_scalingAMax;
  unsigned int          m_scalingBMin, m_scalingBMax;
  bool                  m_scalingIsSynced;
  
  unsigned int          m_splitterA;
  unsigned int          m_splitterB;

  bool                  m_dataTextureInterp;
protected:
  const IPalette*       m_ppal;
  bool                  m_ppaldiscretise;       // inited by true if portions > 1
  float                 m_clearcolor[3];
  float                 m_ppalrange[2];
  enum                  { PRNG_START, PRNG_STOP };
  
  enum  CLEARSOURCE     { CS_WIDGET, CS_PALETTE, CS_MANUAL };
  bool                  m_doclearbackground;
  CLEARSOURCE           m_clearsource;
protected:
  enum  GROUNDTYPE      { GND_NONE, GND_DOMAIN, GND_SDP, GND_ASSISTFLOATTABLE };
  GROUNDTYPE            m_groundType;
  void*                 m_groundData;
  unsigned int          m_groundDataWidth, m_groundDataHeight;
  bool                  m_groundDataFastFree, m_groundMipMapping;
protected:
  int                   m_bitmaskUpdateBan;
  int                   m_bitmaskPendingChanges;
protected:
  impulsedata_t         m_postImpulse;
  overpattern_t         m_postOverpattern;
  float                 m_postOverpatternOpacity;
public:
                    /// Redraw control. Which actions will cause repaint
  enum  REDRAWBY        { RD_BYDATA, RD_BYSETTINGS, RD_BYOVL_ADDREMOVE, RD_BYOVL_ACTIONS };   // special now used on overlays removing
  void                  banAutoUpdate(REDRAWBY rb, bool ban){ if (ban) m_bitmaskUpdateBan |= (1 << (int)rb); else m_bitmaskUpdateBan &= ~(1 << (int)rb); }
  void                  banAutoUpdate(bool ban){ m_bitmaskUpdateBan = ban? 0xF : 0; }
  bool                  autoUpdateBanned(REDRAWBY rdb) const {  return ((1 << (int)rdb) & m_bitmaskUpdateBan) != 0;  }
protected:
  /// inner. Pending changes bitmask
  enum                  PCBM  { PC_INIT=1, PC_SIZE=2, PC_GROUND=4, PC_DATA=8, PC_PALETTE=16, PC_PARAMS=64, PC_PARAMSOVL=128  };
  bool                  havePendOn(PCBM bit) const {  return (m_bitmaskPendingChanges & bit) != 0; }
  bool                  havePending() const {  return m_bitmaskPendingChanges != 0; } 
  bool                  havePendOn(PCBM bit, int someBPCcopy) const {  return (someBPCcopy & bit) != 0; }
  bool                  havePending(int someBPCcopy) const {  return someBPCcopy != 0; } 
  void                  unpend(PCBM bit) {  m_bitmaskPendingChanges &= ~bit; }
  void                  unpendAll() {  m_bitmaskPendingChanges = 0; }
  
  void                  pendResize(bool doUpdateGeometry){    m_bitmaskPendingChanges |= PC_SIZE;  if (doUpdateGeometry) innerUpdateGeometry(); }
public:  
#ifdef BSOVERLAYSLIMIT
  enum                    { OVLLIMIT=BSOVERLAYSLIMIT };
#else
  enum                    { OVLLIMIT=32 };
#endif
#ifdef BSOVLUNIFORMSLIMIT
  enum                    { OVLUFLIMIT=BSOVLUNIFORMSLIMIT };
#else
  enum                    { OVLUFLIMIT=16 };
#endif
protected:
  /// inner. Overlays storage
  struct uniform_located_t
  {
    int                 location;
    int                 type;
    const void*         dataptr;
    unsigned int        tex_idx;
  };
  struct msstruct_t
  {
    enum  MSTYPE        { MS_SELF, MS_ROOT, MS_DRIVEN };
    int                 type;
    union {     int     drivenid;    }  details;
  };
  enum  REACTOR_BANS    { ORB_KEYBOARD=0x1, ORB_MOUSE=0x2 };
  
  struct overlays_t
  {
    Ovldraw*          povl;
    OVL_ORIENTATION       orient;
    IOverlayReactor*      prct;
    unsigned int          prct_bans;
    unsigned int          ponger_reinit;
    unsigned int          ponger_update;
    int                   outloc;
    unsigned int          uf_count;
    uniform_located_t     uf_arr[OVLUFLIMIT];
    msstruct_t            olinks;
    void      _reinit(Ovldraw* p, OVL_ORIENTATION o, unsigned int ufcount)
    {
      povl = p;
      orient = o;
      prct = povl->reactor();
      prct_bans = 0;
      ponger_reinit = ponger_update = 0;
      outloc = -1;
//      if (uf_count) delete []uf_arr;
      uf_count = ufcount;
//      if(uf_count) uf_arr = new uniform_located_t[ufcount];
      olinks.type = msstruct_t::MS_SELF;
    }
    void                _setdriven(int driverid){ olinks.type = msstruct_t::MS_DRIVEN;  olinks.details.drivenid = driverid; }
    overlays_t(): povl(nullptr), prct(nullptr), prct_bans(0), uf_count(0){}
//    ~overlays_t() { if (uf_count) delete[]uf_arr; }
  }                     m_overlays[OVLLIMIT];
  unsigned int          m_overlaysCount;
  DrawEventReactor*           m_proactive;
  bool                  m_proactiveOwner;
  OvldrawEmpty      m_overlaySingleEmpty;
public:
  DrawCore(DATAASTEXTURE datex, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): m_datex(datex), 
                                                        m_portionSize(0), m_allocatedPortions(portions), 
                                                        m_countPortions(portions), m_orient(orient), m_splitPortions(splitPortions),
                                                        m_dataStorage(nullptr), m_dataStorageCached(nullptr),
                                                        m_dataDimmSwitchAB(orient > OR_RLTB),
                                                        m_rawResizeModeNoScaled(false),
                                                        m_scalingA(1), m_scalingB(1),
                                                        m_scalingAMin(1), m_scalingAMax(0),
                                                        m_scalingBMin(1), m_scalingBMax(0), m_scalingIsSynced(false),
                                                        m_dataTextureInterp(false),
                                                        m_ppal(nullptr), m_ppaldiscretise(false), m_doclearbackground(true), m_clearsource(CS_WIDGET), 
                                                        m_groundType(GND_NONE), m_groundData(nullptr), m_groundDataFastFree(true),
                                                        m_groundMipMapping(false), m_bitmaskUpdateBan(0), m_bitmaskPendingChanges(PC_INIT), 
                                                        m_postOverpattern(overpattern_off()), m_postOverpatternOpacity(0.0f), 
                                                        m_overlaysCount(0), m_proactive(nullptr), m_proactiveOwner(true)
  {
    _bsdraw_update_kb(m_bounds, &m_loc_k, &m_loc_b);
    
    m_postImpulse.type = impulsedata_t::IR_OFF;
    m_postImpulse.count = 0;
    
    m_ppalrange[PRNG_START] = 0.0f;
    m_ppalrange[PRNG_STOP] = 1.0f;
        
    unsigned int spDivider = m_splitPortions&0xFF;
    unsigned int divider2 = spDivider == 0? 1 : m_allocatedPortions / spDivider + (m_allocatedPortions % spDivider? 1 : 0);
    if (((m_splitPortions >> 8)&0xFF) == 0)
    {
      m_splitterA = spDivider == 0? 1 : spDivider;
      m_splitterB = divider2;
    }
    else
    {
      m_splitterB = spDivider == 0? 1 : spDivider;
      m_splitterA = divider2;
    }
  }
  virtual ~DrawCore()
  {
    if (m_proactiveOwner && m_proactive)
      delete m_proactive;
    _ovlRemoveAll();
    if (m_dataStorage)
      delete []m_dataStorage;
    if (m_dataStorageCached)
      delete []m_dataStorageCached;
  }
protected:
  void    deployMemory()
  {
    unsigned int total = m_allocatedPortions * m_portionSize;
    m_dataStorage = new float[total];
    for (unsigned int i=0; i<total; i++)
      m_dataStorage[i] = 0;
    m_dataStorageCached = new float[total];
  }
  void    deployMemory(unsigned int total) {  m_dataStorage = new float[total]; for (unsigned int i=0; i<total; i++) m_dataStorage[i] = 0; m_dataStorageCached = new float[total]; }
public:
  unsigned int directions() const { if (m_datex == DATEX_2D || m_datex == DATEX_DD || m_datex == DATEX_POLAR) return 2; return 1; }
public:
                    /// Access methods
  unsigned int          allocatedPortions() const { return m_allocatedPortions; }
  unsigned int          countPortions() const { return m_countPortions; }
  unsigned int          portionSize() const { return m_portionSize; }
  
  void                  setBounds(const bounds_t& d){ m_bounds = d; _bsdraw_update_kb(m_bounds, &m_loc_k, &m_loc_b);    m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setBounds(float LL, float HL){ setBounds(bounds_t(LL, HL)); }
  void                  setBoundLow(float LL){ m_bounds.LL = LL; _bsdraw_update_kb(m_bounds, &m_loc_k, &m_loc_b);       m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setBoundHigh(float HL){ m_bounds.HL = HL; _bsdraw_update_kb(m_bounds, &m_loc_k, &m_loc_b);      m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setBounds01(){ m_bounds.LL = m_loc_b = 0.0f; m_bounds.HL = m_loc_k = 1.0f;                      m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  bounds_t              bounds() const { return m_bounds; }
  float                 boundLow() const { return m_bounds.LL; }
  float                 boundHigh() const { return m_bounds.HL; }
  float                 boundLength() const { return m_bounds.HL - m_bounds.LL; }
  
  void                  setContrast(float k, float b){ m_loc_k = k; m_loc_b = b; _bsdraw_update_bnd(m_loc_k, m_loc_b, &m_bounds); m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setContrastK(float k){ m_loc_k = k; _bsdraw_update_bnd(m_loc_k, m_loc_b, &m_bounds);            m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setContrastKinv(float k){ m_loc_k = 1.0f/k; _bsdraw_update_bnd(m_loc_k, m_loc_b, &m_bounds);    m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  void                  setContrastB(float b){ m_loc_b = b; _bsdraw_update_bnd(m_loc_k, m_loc_b, &m_bounds);            m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  float                 contrastK()     const { return m_loc_k; }
  float                 contrastKinv()  const { return 1.0f/m_loc_k; }
  float                 contrastB()     const { return m_loc_b; }
public:
  ///             size components
  unsigned int          sizeDataA() const { return m_dataDimmA; }
  unsigned int          sizeDataB() const { return m_dataDimmB; }
  unsigned int          sizeDataHorz() const { return m_dataDimmSwitchAB? m_dataDimmB : m_dataDimmA; }
  unsigned int          sizeDataVert() const { return m_dataDimmSwitchAB? m_dataDimmA : m_dataDimmB; }

  unsigned int          scalingA() const { return m_scalingA; }
  unsigned int          scalingB() const { return m_scalingB; }
  unsigned int          scalingHorz() const { return m_dataDimmSwitchAB? m_scalingB : m_scalingA; }
  unsigned int          scalingVert() const { return m_dataDimmSwitchAB? m_scalingA : m_scalingB; }

  unsigned int          splitterA() const { return m_splitterA; }
  unsigned int          splitterB() const { return m_splitterB; }
  unsigned int          splitterHorz() const { return m_dataDimmSwitchAB? m_splitterB : m_splitterA; }
  unsigned int          splitterVert() const { return m_dataDimmSwitchAB? m_splitterA : m_splitterB; }
  
  ///             size total
  unsigned int          sizeA() const { return m_dataDimmA*m_scalingA*m_splitterA; }
  unsigned int          sizeB() const { return m_dataDimmB*m_scalingB*m_splitterB; }
  unsigned int          sizeHorz() const { return m_dataDimmSwitchAB? sizeB() : sizeA(); }
  unsigned int          sizeVert() const { return m_dataDimmSwitchAB? sizeA() : sizeB(); }
  
  dcsizecd_t            sizeComponentsA() const { return { m_dataDimmA, m_scalingA, m_splitterA }; }
  dcsizecd_t            sizeComponentsB() const { return { m_dataDimmB, m_scalingB, m_splitterB }; }
  dcsizecd_t            sizeComponentsHorz() const { return m_dataDimmSwitchAB? sizeComponentsB() : sizeComponentsA(); }
  dcsizecd_t            sizeComponentsVert() const { return m_dataDimmSwitchAB? sizeComponentsA() : sizeComponentsB(); }
  
  bool                  isSplittedA() const { return ((m_splitPortions >> 8)&0xFF) == 0; }
  bool                  isSplittedB() const { return ((m_splitPortions >> 8)&0xFF) != 0; }
  
  void                  scalingLimitsA(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_scalingAMin;  if (scmax) *scmax = m_scalingAMax; }
  void                  scalingLimitsB(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_scalingBMin;  if (scmax) *scmax = m_scalingBMax; }
  void                  scalingLimitsHorz(unsigned int *scmin, unsigned int *scmax=nullptr) const { if (!m_dataDimmSwitchAB) scalingLimitsA(scmin, scmax); else scalingLimitsB(scmin, scmax); }
  void                  scalingLimitsVert(unsigned int *scmin, unsigned int *scmax=nullptr) const { if (!m_dataDimmSwitchAB) scalingLimitsA(scmin, scmax); else scalingLimitsB(scmin, scmax); }
  
  void                  setScalingLimitsA(unsigned int scmin, unsigned int scmax=0){  m_scalingAMin = scmin < 1? 1 : scmin; m_scalingAMax = scmax;  m_scalingIsSynced = false; clampScalingManually(); pendResize(true); }
  void                  setScalingLimitsB(unsigned int scmin, unsigned int scmax=0){  m_scalingBMin = scmin < 1? 1 : scmin; m_scalingBMax = scmax;  m_scalingIsSynced = false; clampScalingManually(); pendResize(true); }
  void                  setScalingLimitsHorz(unsigned int scmin, unsigned int scmax=0){ if (!m_dataDimmSwitchAB) setScalingLimitsA(scmin, scmax); else setScalingLimitsB(scmin, scmax); }
  void                  setScalingLimitsVert(unsigned int scmin, unsigned int scmax=0){ if (!m_dataDimmSwitchAB) setScalingLimitsB(scmin, scmax); else setScalingLimitsA(scmin, scmax); }
  void                  setScalingLimitsSynced(unsigned int scmin, unsigned int scmax=0){ m_scalingIsSynced = true; m_scalingAMin = m_scalingBMin = scmin < 1? 1 : scmin; m_scalingAMax = m_scalingBMax = scmax; clampScalingManually(); pendResize(true); }
  
//  void                  setScalingDefaultA(unsigned int sc){  if (sc < m_scalingAMin) sc = m_scalingAMin;   m_scalingA = sc;  innerUpdateGeometry();  }
//  void                  setScalingDefaultB(unsigned int sc){  if (sc < m_scalingBMin) sc = m_scalingBMin;   m_scalingB = sc;  innerUpdateGeometry();  }
public:
  virtual void          sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const =0;
  void                  adjustSizeAndScale(int sizeA, int sizeB)
  {
    unsigned int matrixDimmA, matrixDimmB, scalingA, scalingB;
    sizeAndScaleHint(sizeA/(int)m_splitterA, sizeB/(int)m_splitterB, &matrixDimmA, &matrixDimmB, &scalingA, &scalingB);
    bool  changedDimmA = m_dataDimmA != matrixDimmA || m_scalingA != scalingA;
    m_dataDimmA = matrixDimmA;
    m_scalingA = scalingA;
    bool  changedDimmB = m_dataDimmB != matrixDimmB || m_scalingB != scalingB;
    m_dataDimmB = matrixDimmB;
    m_scalingB = scalingB;
    m_bitmaskPendingChanges |= sizeAndScaleChanged(changedDimmA, changedDimmB);
  }
protected:
  virtual int           sizeAndScaleChanged(bool /*changedDimmA*/, bool /*changedDimmB*/) { return 0; }
protected:
  void  clampScaling(unsigned int* scalingA, unsigned int* scalingB) const
  {
    if (*scalingA < m_scalingAMin) *scalingA = m_scalingAMin;
    if (m_scalingAMax && *scalingA > m_scalingAMax) *scalingA = m_scalingAMax;
    if (*scalingB < m_scalingBMin) *scalingB = m_scalingBMin;
    if (m_scalingBMax && *scalingB > m_scalingBMax)  *scalingB = m_scalingBMax;
    
    if (m_scalingIsSynced)
    {
      if (*scalingB > *scalingA || (m_datex != DATEX_2D && m_datex != DATEX_DD && m_datex != DATEX_POLAR)) *scalingB = *scalingA;
      else *scalingA = *scalingB;
    }
  }
private:
  void  clampScalingManually()
  {
    //unsigned int old_scaler_a = m_scalingA;
    unsigned int old_scaler_b = m_scalingB;
    clampScaling(&m_scalingA, &m_scalingB);
    if (m_scalingB != old_scaler_b  && m_datex == DATEX_1D)
    {
      float coeff = float(old_scaler_b) / m_scalingB;
      m_dataDimmB = (unsigned int)(m_dataDimmB*coeff + 0.5f);
    }
//    m_bitmaskPendingChanges |= sizeAndScaleChanged(old_scaler_a != m_scalingA, old_scaler_b != m_scalingB);
  }
public:
  void  setDataTextureInterpolation(bool interp)
  {
    m_dataTextureInterp = interp;
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  
  void  setRawResizeModeNoScaled(bool rawmode)
  {
    m_rawResizeModeNoScaled = rawmode;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  bool  rawResizeModeNoScaled() const { return m_rawResizeModeNoScaled; }
  
  void  setGroundMipMapping(bool v)
  { 
    m_groundMipMapping = v;
    m_bitmaskPendingChanges |= PC_GROUND;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  bool  groundMipMapping() const { return m_groundMipMapping; }
public:
  void  setDataPalette(const IPalette* ppal)
  {
    m_ppal = ppal;
    m_ppaldiscretise = ppal->paletteDiscretion();
    m_bitmaskPendingChanges |= PC_PALETTE;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setDataPaletteDiscretion(bool d)
  {
    m_ppaldiscretise = d;
    m_bitmaskPendingChanges |= PC_PALETTE;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setDataPalette(const IPalette* ppal, bool discrete)
  {
    m_ppal = ppal;
    m_ppaldiscretise = discrete;
    m_bitmaskPendingChanges |= PC_PALETTE;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  const IPalette* dataPalette() const { return m_ppal; }
  bool            dataPaletteInterpolation()const { return m_dataTextureInterp; }
  
  void  setDataPaletteRange(float start=0.0f, float stop=1.0f)
  {
    m_ppalrange[PRNG_START] = start;
    m_ppalrange[PRNG_STOP] = stop;
    m_bitmaskPendingChanges |= PC_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setDataPaletteRangeStart(float start)
  {
    m_ppalrange[PRNG_START] = start;
    m_bitmaskPendingChanges |= PC_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setDataPaletteRangeStop(float stop)
  {
    m_ppalrange[PRNG_STOP] = stop;
    m_bitmaskPendingChanges |= PC_PARAMS;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  float   dataPaletteRangeStart() const {  return m_ppalrange[PRNG_START]; }
  float   dataPaletteRangeStop() const {  return m_ppalrange[PRNG_STOP]; }
public:
  /// 1. Data methods
  virtual void setData(const float* data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_dataStorage[i] = data[i];
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA))   callWidgetUpdate();
  }
  virtual void setData(const float* data, DataDecimator* decim)
  {
    for (unsigned int p=0; p<m_countPortions; p++)
      for (unsigned int i=0; i<m_portionSize; i++)
        m_dataStorage[p*m_portionSize + i] = decim->decimate(data, (int)m_portionSize, (int)i, (int)p);
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  virtual void fillData(float data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_dataStorage[i] = data;
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  virtual void clearData()
  {
    unsigned int total = m_allocatedPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_dataStorage[i] = 0;
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  
  void getData(float* result) const
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      result[i] = m_dataStorage[i];
  }
  const float*  getDataPtr() const {   return m_dataStorage;    }
  float*        getDataPtr() {    return m_dataStorage;   }     // You need manual update after data change
  virtual const float*  getDataPtr(unsigned int portion) const { if (portion >= m_allocatedPortions)  return nullptr; return &m_dataStorage[portion*m_portionSize]; }
  virtual float*        getDataPtr(unsigned int portion) { if (portion >= m_allocatedPortions)  return nullptr; return &m_dataStorage[portion*m_portionSize]; }     // You need manual update after data change
  void  manualDataPtrUpdate(){  vmanUpData(); }
  void  updateData(){ vmanUpData(); }
protected:
  void  vmanUpInit(){ m_bitmaskPendingChanges |= PC_INIT; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();  }
  void  vmanUpData(){ m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();  }
public:
  /// 2. Delegated methods
  void  setPortionsCount(unsigned int portionsLessThanAlocated)
  {
    if (portionsLessThanAlocated > m_allocatedPortions)
      portionsLessThanAlocated = m_allocatedPortions;
    m_countPortions = (unsigned int)portionsLessThanAlocated;
    m_bitmaskPendingChanges |= PC_DATA | PC_SIZE | PC_PARAMS;
//    if (m_spDivider != 0)
    if (m_splitterA > 1 || m_splitterB > 1)
      innerRescale();
    if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
public:
  void  setImpulse(const impulsedata_t& id)
  {
    m_postImpulse = id;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setImpulseOff()
  {
    if (m_postImpulse.type != impulsedata_t::IR_OFF)
    {
      m_postImpulse.type = impulsedata_t::IR_OFF;
      m_postImpulse.count = m_postImpulse.central = 0;
      m_bitmaskPendingChanges |= PC_INIT;
      if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
    }
  }
  void  setImpulseCoeffA(int count, const float coeffs[], int central, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_A_COEFF_NOSCALED : impulsedata_t::IR_A_COEFF, count, central, cycled? 1 : 0, {} };
    for (int i=0; i<count; i++)
      m_postImpulse.coeff[i] = coeffs[i];
    setImpulse(idt);
  }
  void  setImpulseCoeffA(float coeff_left, float coeff_central, float coeff_right, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_A_COEFF_NOSCALED : impulsedata_t::IR_A_COEFF, 3, 1, cycled? 1 : 0, {coeff_left, coeff_central, coeff_right} };
    setImpulse(idt);
  }
  void  setImpulseCoeffA(float coeff_left2, float coeff_left1, float coeff_central, float coeff_right1, float coeff_right2, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_A_COEFF_NOSCALED : impulsedata_t::IR_A_COEFF, 5, 2, cycled? 1 : 0, {coeff_left2, coeff_left1, coeff_central, coeff_right1, coeff_right2} };
    setImpulse(idt);
  }
  void  setImpulseCoeffB(int count, const float coeffs[], int central, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_B_COEFF_NOSCALED : impulsedata_t::IR_B_COEFF, count, central, cycled? 1 : 0, {} };
    for (int i=0; i<count; i++)
      m_postImpulse.coeff[i] = coeffs[i];
    setImpulse(idt);
  }
  void  setImpulseCoeffB(float coeff_left, float coeff_central, float coeff_right, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_B_COEFF_NOSCALED : impulsedata_t::IR_B_COEFF, 3, 1, cycled? 1 : 0, {coeff_left, coeff_central, coeff_right} };
    setImpulse(idt);
  }
  void  setImpulseCoeffB(float coeff_left2, float coeff_left1, float coeff_central, float coeff_right1, float coeff_right2, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_B_COEFF_NOSCALED : impulsedata_t::IR_B_COEFF, 5, 2, cycled? 1 : 0, {coeff_left2, coeff_left1, coeff_central, coeff_right1, coeff_right2} };
    setImpulse(idt);
  }
  void  setImpulseBordersA(int minscaling, float coeff, bool smart=false)
  {
    impulsedata_t idt = { smart? impulsedata_t::IR_A_BORDERS_SMART : impulsedata_t::IR_A_BORDERS, minscaling, 0, 0, { coeff } };
    setImpulse(idt);
  }
  void  setImpulseBordersB(int minscaling, float coeff, bool smart=false)
  {
    impulsedata_t idt = { smart? impulsedata_t::IR_B_BORDERS_SMART : impulsedata_t::IR_B_BORDERS, minscaling, smart, 0, { coeff } };
    setImpulse(idt);
  }
  const impulsedata_t& impulse() const { return m_postImpulse; }
public:
  
  void  setOrientation(ORIENTATION orient)
  { 
    m_orient = orient;
    m_dataDimmSwitchAB = orient > OR_RLTB;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  ORIENTATION     orientation() const { return m_orient; }
  void  setTransponed(bool r)
  {
    m_orient = (ORIENTATION)((int)m_orient + (m_orient > OR_RLTB? ( r? 0 : -4 ) : ( r? 4 : 0 )));
    m_dataDimmSwitchAB = r;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  bool  transponed() const {  return m_dataDimmSwitchAB;  }
  void  setMirroredHorz()
  {
    const ORIENTATION mh[] = { OR_RLBT, OR_LRBT, OR_RLTB, OR_LRTB, OR_TBRL, OR_BTRL, OR_TBLR, OR_BTLR };
    setOrientation(mh[m_orient]);
  }
  void  setMirroredVert()
  {
    const ORIENTATION mv[] = { OR_LRTB, OR_RLTB, OR_LRBT, OR_RLBT, OR_BTLR, OR_TBLR, OR_BTRL, OR_TBRL };
    setOrientation(mv[m_orient]);
  }
  bool  mirroredHorz() const { return orientationMirroredHorz(m_orient); }
  bool  mirroredVert() const { return orientationMirroredVert(m_orient); }
public:
  DrawEventReactor*  setProactive(DrawEventReactor* op, bool owner=true)
  {
    m_proactiveOwner = owner;
    DrawEventReactor* result = m_proactive; m_proactive = op; return result;
  }
public:
  /// 3. Overlays
  int             ovlPushBack(Ovldraw* povl, OVL_ORIENTATION orient=OO_INHERITED, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT) return -1;
    _ovlSet(m_overlaysCount, povl, orient, owner, false, 0);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ++m_overlaysCount;
  }
  int             ovlPushBack(Ovldraw* povl, int ovlroot, OVL_ORIENTATION orient=OO_INHERITED, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT || ovlroot <= 0 || ovlroot > (int)m_overlaysCount) return -1;
    _ovlSet(m_overlaysCount, povl, orient, owner, true, ovlroot-1);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ++m_overlaysCount;
  }
  int             ovlReplace(int ovl, Ovldraw* povl, OVL_ORIENTATION orient=OO_SAME, bool owner=true, Ovldraw** old=nullptr)
  {
    ovl--;
    OVL_ORIENTATION c_orient = orient == OO_SAME? m_overlays[ovl].orient : orient;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, c_orient, owner, false, 0);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ovl + 1;
  }
  int             ovlReplace(int ovl, Ovldraw* povl, int ovlroot, OVL_ORIENTATION orient=OO_SAME, bool owner=true, Ovldraw** old=nullptr)
  {
    ovl--;
    OVL_ORIENTATION c_orient = orient == OO_SAME? m_overlays[ovl].orient : orient;
    if (ovlroot == 0)
      ovlroot = m_overlays[ovl].olinks.details.drivenid;
    else
      ovlroot--;
    if (ovlroot < 0 || ovlroot >= ovl) return -1;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, c_orient, owner, true, ovlroot);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ovl + 1;
  }
  Ovldraw*       ovlRemove(int ovl)
  {
    ovl--;
    Ovldraw* result = nullptr;
    if (_ovlRemove(ovl, &result) == false)
      return nullptr;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    if (ovl+1 == (int)m_overlaysCount)
      while (m_overlays[ovl--].povl == &m_overlaySingleEmpty)
        m_overlaysCount--;
    return result;
  }
  Ovldraw* ovlGet(int ovl) const
  {
    ovl -= 1;
    if (ovl < 0 || (unsigned int)ovl >= m_overlaysCount) return nullptr;
    if (m_overlays[ovl].povl == &m_overlaySingleEmpty)  return nullptr;
    return m_overlays[ovl].povl;
  }
  Ovldraw* ovlLast() const
  {
    if (m_overlaysCount == 0)
      return nullptr;
    return m_overlays[m_overlaysCount - 1].povl;
  }
  void     ovlReactionsEnableAll(int ovl)
  {
    ovl -= 1;
    if (ovl < 0 || (unsigned int)ovl >= m_overlaysCount) return;
    m_overlays[ovl].prct_bans = 0;
  }
  void     ovlReactionsDisableAll(int ovl)
  {
    ovl -= 1;
    if (ovl < 0 || (unsigned int)ovl >= m_overlaysCount) return;
    m_overlays[ovl].prct_bans = 0xFFFFFFFF;
  }
  Ovldraw* ovlPopBack()
  {
    Ovldraw* result = nullptr;
    if (_ovlRemove((int)m_overlaysCount - 1, &result))
    {
      m_overlaysCount--;
      m_bitmaskPendingChanges |= PC_INIT;
      if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    }
    return result;
  }
  void      ovlClearAll()
  {
    _ovlRemoveAll();
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
  }
private:
  void _ovlSet(int idx, Ovldraw* povl, OVL_ORIENTATION orient, bool owner, bool doRoot, int rootidx)
  {
    if (povl == nullptr){   povl = &m_overlaySingleEmpty;  owner = false; }
    povl->increasePingerReinit();
    povl->increasePingerUpdate();
    m_overlays[idx]._reinit(povl, orient, povl->uniforms().count);
    if (doRoot)   m_overlays[idx]._setdriven(rootidx);
    if (povl != &m_overlaySingleEmpty)
      povl->assign(idx, this, owner);
  }
  void _ovlRemoveAll()
  {
    for (unsigned int i=0; i<m_overlaysCount; i++)
      if (m_overlays[i].povl != &m_overlaySingleEmpty)
        if (m_overlays[i].povl->preDelete(this) == true)
          delete m_overlays[i].povl;
    m_overlaysCount = 0;
  }
  bool  _ovlRemove(int idx, Ovldraw** old)
  {
    if (idx < 0 || idx >= (int)m_overlaysCount) return false;
    Ovldraw* removable = m_overlays[idx].povl;
    if (removable == &m_overlaySingleEmpty)
      removable = nullptr;
    else
    {
      m_overlays[idx]._reinit(&m_overlaySingleEmpty, OO_INHERITED, 0);
      if (removable->preDelete(this) == true)
      {
        delete removable;
        removable = nullptr;
      }
    }
    if (old) *old = removable;
    return true;
  }
public:
  /// 4. Optimized/Native posteffect methods on scaling
  void            setOverpattern(const overpattern_t& fsp)
  {
    m_postOverpattern = fsp;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void            setOverpattern(const overpattern_t& fsp, float opacity)
  {
    m_postOverpattern = fsp;
    m_postOverpatternOpacity = opacity;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  const overpattern_t&  overpattern() const { return m_postOverpattern; }
  float                 overpatternOpacity() const { return m_postOverpatternOpacity; }
protected:
  virtual void            overlayUpdate(bool reinit, bool pendonly)
  {
    m_bitmaskPendingChanges |= reinit? PC_INIT : PC_PARAMSOVL;
    if (pendonly) return;
    if (!autoUpdateBanned(RD_BYOVL_ACTIONS))  callWidgetUpdate();
  }
  virtual void            innerOverlayReplace(int ovlid, Ovldraw* novl, OVL_ORIENTATION orient, bool owner)
  {
    m_overlays[ovlid]._reinit(novl, orient == OO_SAME? m_overlays[ovlid].orient : orient, novl->uniforms().count);  ///_setdriven derived
    if (novl != &m_overlaySingleEmpty)
    {
      novl->assign(ovlid, this, owner);
    }
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
  }
  virtual void            innerOverlayRemove(int ovlid)
  {
    m_overlays[ovlid]._reinit(&m_overlaySingleEmpty, OO_INHERITED, 0);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
  }
  void                    _colorCvt(unsigned int clr)
  {
    m_clearcolor[0] = ((clr &0xFF)/255.0f);
    m_clearcolor[1] = ((clr >> 8 &0xFF)/255.0f);
    m_clearcolor[2] = ((clr >> 16 &0xFF)/255.0f);
  }
public:
  void            setClearColor(unsigned int clearcolor)
  {
    m_clearsource = CS_MANUAL;
    _colorCvt(clearcolor);
    m_doclearbackground = true;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void            setClearByPalette()
  {
    m_clearsource = CS_PALETTE;
    m_doclearbackground = true;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  bool            isClearedByPalette() const { return m_clearsource == CS_PALETTE; }
  unsigned int    clearColor() const {  return (unsigned int)
        (int(m_clearcolor[0]*255.0f) + (int(m_clearcolor[1]*255.0f)<<8) + (int(m_clearcolor[2]*255.0f)<<16));
                                     }
  virtual unsigned int colorBack() const { return m_ppal? m_ppal->first() : 0x00000000; }
protected:
  virtual void    callWidgetUpdate()=0;
  virtual void    innerRescale()=0;
  virtual void    innerUpdateGeometry()=0;
public:
  bool  getMinMax(unsigned int start, unsigned int stop, float* rmin, float* rmax)
  {
    if (m_portionSize == 0) return false;
    if (start > m_portionSize || start > stop)  return false;
    if (stop > m_portionSize) stop = m_portionSize;
    float min = m_dataStorage[start], max = m_dataStorage[start];
    for (unsigned int p=0; p<m_countPortions; p++)
      for (unsigned int i=start+1; i<stop; i++)
      {
        if (min > m_dataStorage[p*m_portionSize + i])
          min = m_dataStorage[p*m_portionSize + i];
        if (max < m_dataStorage[p*m_portionSize + i])
          max = m_dataStorage[p*m_portionSize + i];
      }
    if (rmin) *rmin = min;
    if (rmax) *rmax = max;
    return true;
  }
  bool  getMinMax(unsigned int start, unsigned int stop, unsigned int portion, float* rmin, float* rmax)
  {
    if (m_portionSize == 0) return false;
    if (start > m_portionSize || start > stop)  return false;
    if (stop > m_portionSize) stop = m_portionSize;
    const unsigned int pjump = portion*m_portionSize;
    float min = m_dataStorage[pjump + start], max = m_dataStorage[pjump + start];
    for (unsigned int i=start+1; i<stop; i++)
    {
      if (min > m_dataStorage[pjump + i])
        min = m_dataStorage[pjump + i];
      if (max < m_dataStorage[pjump + i])
        max = m_dataStorage[pjump + i];
    }
    if (rmin) *rmin = min;
    if (rmax) *rmax = max;
    return true;
  }
  void  adjustBounds()
  {
    float min,max;
    if (getMinMax(0, m_portionSize, &min, &max))
      setBounds(min, max);
  }
  void  adjustBounds(unsigned int portion)
  {
    float min,max;
    if (getMinMax(0, m_portionSize, portion, &min, &max))
      setBounds(min, max);
  }
  void  adjustBounds(unsigned int start, unsigned int stop)
  {
    float min,max;
    if (getMinMax(start, stop, &min, &max))
      setBounds(min, max);
  }
  void  adjustBounds(unsigned int start, unsigned int stop, unsigned int portion)
  {
    float min,max;
    if (getMinMax(start, stop, portion, &min, &max))
      setBounds(min, max);
  }
  void  adjustBoundsWithSpacingAdd(float add2min, float add2max)
  {
    float min,max;
    if (getMinMax(0, m_portionSize, &min, &max))
      setBounds(min + add2min, max + add2max);
  }
  void  adjustBoundsWithSpacingAdd(float add2min, float add2max, unsigned int start, unsigned int stop)
  {
    float min,max;
    if (getMinMax(start, stop, &min, &max))
      setBounds(min + add2min, max + add2max);
  }
  void  adjustBoundsWithSpacingMul(float mul2min, float mul2max)
  {
    float min,max;
    if (getMinMax(0, m_portionSize, &min, &max))
      setBounds(min*mul2min, max*mul2max);
  }
  void  adjustBoundsWithSpacingMul(float mul2min, float mul2max, unsigned int start, unsigned int stop)
  {
    float min,max;
    if (getMinMax(start, stop, &min, &max))
      setBounds(min*mul2min, max*mul2max);
  }
};


#endif // DRAWCORE_H
