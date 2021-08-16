#ifndef DRAWCORE_H
#define DRAWCORE_H

/// This file contains base class for all types of draws
/// You dont need to use classes from this file directly
/// For deriving your subclasses use DrawQWidget, this class better shows interface
/// Created By: Elijah Vlasov

#ifndef BSOVERLAYSLIMIT
// Using default bigsized overlay
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

inline  void  _bsdraw_update_kb(const bounds_t& bnd, float* k, float *b)
{
  *k = 1.0f/(bnd.HL - bnd.LL);
  *b = -bnd.LL*1.0f/(bnd.HL - bnd.LL);
}
inline  void  _bsdraw_update_bnd(const float k, const float b, bounds_t* bnd)
{
  bnd->LL = -b/k;
  bnd->HL = (1.0f - b)/k;
}

//#########################
class IProactive
{
public:
  virtual bool  overlayReactionMouse(class DrawQWidget*, OVL_REACTION_MOUSE, const void*, bool* /*doStop*/){  return false; }
  virtual bool  overlayReactionKey(class DrawQWidget*, int /*key*/, int /*modifiersOMK*/, bool* /*doStop*/){  return false; }
  virtual ~IProactive(){}
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


class DrawCore: public IDrawOverlayFriendly
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

  float*                m_matrixData;
  float*                m_matrixDataCached;
  unsigned int          m_matrixDimmA;
  unsigned int          m_matrixDimmB;
  bool                  m_matrixSwitchAB;
  
  impulsedata_t         m_impulsedata;
  DRAWVIEWALIGN         m_viewAlign;
  bool                  m_dataTextureInterp;
  
  bool                  m_rawResizeModeNoScaled;

  unsigned int          m_scalingA;
  unsigned int          m_scalingB;
  unsigned int          m_scalingAMin, m_scalingAMax;
  unsigned int          m_scalingBMin, m_scalingBMax;
  bool                  m_scalingIsSynced;
  
  unsigned int          m_splitterA;
  unsigned int          m_splitterB;
protected:
  const IPalette*       m_ppal;
  bool                  m_ppaldiscretise;
  float                 m_clearcolor[3];
  
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
  DPostmask             m_postMask;
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
  
  struct overlays_t
  {
    DrawOverlay*           povl;
    int                 upcount;
    int                 outloc;
    unsigned int        uf_count;
    uniform_located_t*  uf_arr;
    msstruct_t          olinks;
    void                _reinit(DrawOverlay* p, unsigned int ufcount){ povl = p; upcount = 1; outloc = -1;  if (uf_count) delete []uf_arr;  uf_count = ufcount; if(uf_count) uf_arr = new uniform_located_t[ufcount]; olinks.type = msstruct_t::MS_SELF; }
    void                _setdriven(int driverid){ olinks.type = msstruct_t::MS_DRIVEN;  olinks.details.drivenid = driverid; }
    overlays_t(): povl(nullptr), uf_count(0){}
    ~overlays_t() { if (uf_count) delete[]uf_arr; }
    
  }                     m_overlays[OVLLIMIT];
  unsigned int          m_overlaysCount;
  IProactive*           m_proactive;
  bool                  m_proactiveOwner;
  class OverlayEmpty: public DrawOverlay
  {
  public:
    virtual int  fshTrace(int, bool, char*) const;
    virtual int  fshColor(int, char*) const;
  };
  OverlayEmpty          m_overlaySingleEmpty;
public:
  DrawCore(DATAASTEXTURE datex, unsigned int portions, ORIENTATION orient, SPLITPORTIONS splitPortions): m_datex(datex), 
                                                        m_portionSize(0), m_allocatedPortions(portions), 
                                                        m_countPortions(portions), m_orient(orient), m_splitPortions(splitPortions),
                                                        m_matrixData(nullptr), m_matrixDataCached(nullptr),
                                                        m_matrixSwitchAB(orient > OR_RLTB),
                                                        m_viewAlign(DVA_LEFT), m_dataTextureInterp(false), m_rawResizeModeNoScaled(false),
                                                        m_scalingA(1), m_scalingB(1),
                                                        m_scalingAMin(1), m_scalingAMax(0),
                                                        m_scalingBMin(1), m_scalingBMax(0), m_scalingIsSynced(false),
                                                        m_ppal(nullptr), m_ppaldiscretise(false), m_doclearbackground(true), m_clearsource(CS_WIDGET), 
                                                        m_groundType(GND_NONE), m_groundData(nullptr), m_groundDataFastFree(true),
                                                        m_groundMipMapping(false), m_bitmaskUpdateBan(0), m_bitmaskPendingChanges(PC_INIT), 
                                                        m_postMask(DPostmask::empty()),
                                                        m_overlaysCount(0), m_proactive(nullptr), m_proactiveOwner(true)
  {
    _bsdraw_update_kb(m_bounds, &m_loc_k, &m_loc_b);
    
    m_impulsedata.type = impulsedata_t::IR_OFF;
    m_impulsedata.count = 0;
        
    int spDivider = m_splitPortions&0xFF;
    int divider2 = spDivider == 0? 1 : m_allocatedPortions / spDivider + (m_allocatedPortions % spDivider? 1 : 0);
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
    if (m_matrixData)
      delete []m_matrixData;
    if (m_matrixDataCached)
      delete []m_matrixDataCached;
  }
protected:
  void    deployMemory()
  {
    unsigned int total = m_allocatedPortions * m_portionSize;
    m_matrixData = new float[total];
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = 0;
    m_matrixDataCached = new float[total];
  }
  void    deployMemory(unsigned int total) {  m_matrixData = new float[total]; for (unsigned int i=0; i<total; i++) m_matrixData[i] = 0; m_matrixDataCached = new float[total]; }
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
  void                  setContrastB(float b){ m_loc_b = b; _bsdraw_update_bnd(m_loc_k, m_loc_b, &m_bounds);            m_bitmaskPendingChanges |= PC_DATA; if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate(); }
  float                 contrastK() const { return m_loc_k; }
  float                 contrastB() const { return m_loc_b; }
public:
  ///             size components
  unsigned int          sizeDataA() const { return m_matrixDimmA; }
  unsigned int          sizeDataB() const { return m_matrixDimmB; }
  unsigned int          sizeDataHorz() const { return m_matrixSwitchAB? m_matrixDimmB : m_matrixDimmA; }
  unsigned int          sizeDataVert() const { return m_matrixSwitchAB? m_matrixDimmA : m_matrixDimmB; }

  unsigned int          scalingA() const { return m_scalingA; }
  unsigned int          scalingB() const { return m_scalingB; }
  unsigned int          scalingHorz() const { return m_matrixSwitchAB? m_scalingB : m_scalingA; }
  unsigned int          scalingVert() const { return m_matrixSwitchAB? m_scalingA : m_scalingB; }

  unsigned int          splitterA() const { return m_splitterA; }
  unsigned int          splitterB() const { return m_splitterB; }
  unsigned int          splitterHorz() const { return m_matrixSwitchAB? m_splitterB : m_splitterA; }
  unsigned int          splitterVert() const { return m_matrixSwitchAB? m_splitterA : m_splitterB; }
  
  ///             size total
  unsigned int          sizeA() const { return m_matrixDimmA*m_scalingA*m_splitterA; }
  unsigned int          sizeB() const { return m_matrixDimmB*m_scalingB*m_splitterB; }
  unsigned int          sizeHorz() const { return m_matrixSwitchAB? sizeB() : sizeA(); }
  unsigned int          sizeVert() const { return m_matrixSwitchAB? sizeA() : sizeB(); }
  
  dcsizecd_t            sizeComponentsA() const { return { m_matrixDimmA, m_scalingA, m_splitterA }; }
  dcsizecd_t            sizeComponentsB() const { return { m_matrixDimmB, m_scalingB, m_splitterB }; }
  dcsizecd_t            sizeComponentsHorz() const { return m_matrixSwitchAB? sizeComponentsB() : sizeComponentsA(); }
  dcsizecd_t            sizeComponentsVert() const { return m_matrixSwitchAB? sizeComponentsA() : sizeComponentsB(); }


  
  bool                  isSplittedA() const { return ((m_splitPortions >> 8)&0xFF) == 0; }
  bool                  isSplittedB() const { return ((m_splitPortions >> 8)&0xFF) != 0; }
  
  void                  scalingLimitsA(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_scalingAMin;  if (scmax) *scmax = m_scalingAMax; }
  void                  scalingLimitsB(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_scalingBMin;  if (scmax) *scmax = m_scalingBMax; }
  void                  scalingLimitsHorz(unsigned int *scmin, unsigned int *scmax=nullptr) const { if (!m_matrixSwitchAB) scalingLimitsA(scmin, scmax); else scalingLimitsB(scmin, scmax); }
  void                  scalingLimitsVert(unsigned int *scmin, unsigned int *scmax=nullptr) const { if (!m_matrixSwitchAB) scalingLimitsA(scmin, scmax); else scalingLimitsB(scmin, scmax); }
  
  void                  setScalingLimitsA(unsigned int scmin, unsigned int scmax=0){  m_scalingAMin = scmin < 1? 1 : scmin; m_scalingAMax = scmax;  m_scalingIsSynced = false; clampScalingManually(); pendResize(true); }
  void                  setScalingLimitsB(unsigned int scmin, unsigned int scmax=0){  m_scalingBMin = scmin < 1? 1 : scmin; m_scalingBMax = scmax;  m_scalingIsSynced = false; clampScalingManually(); pendResize(true); }
  void                  setScalingLimitsHorz(unsigned int scmin, unsigned int scmax=0){ if (!m_matrixSwitchAB) setScalingLimitsA(scmin, scmax); else setScalingLimitsB(scmin, scmax); }
  void                  setScalingLimitsVert(unsigned int scmin, unsigned int scmax=0){ if (!m_matrixSwitchAB) setScalingLimitsB(scmin, scmax); else setScalingLimitsA(scmin, scmax); }
  void                  setScalingLimitsSynced(unsigned int scmin, unsigned int scmax=0){ m_scalingIsSynced = true; m_scalingAMin = m_scalingBMin = scmin < 1? 1 : scmin; m_scalingAMax = m_scalingBMax = scmax; clampScalingManually(); pendResize(true); }
public:
  virtual void          sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const =0;
  void                  adjustSizeAndScale(int sizeA, int sizeB)
  {
    unsigned int matrixDimmA, matrixDimmB, scalingA, scalingB;
    sizeAndScaleHint(sizeA/m_splitterA, sizeB/m_splitterB, &matrixDimmA, &matrixDimmB, &scalingA, &scalingB);
    bool  changedDimmA = m_matrixDimmA != matrixDimmA || m_scalingA != scalingA;
    m_matrixDimmA = matrixDimmA;
    m_scalingA = scalingA;
    bool  changedDimmB = m_matrixDimmB != matrixDimmB || m_scalingB != scalingB;
    m_matrixDimmB = matrixDimmB;
    m_scalingB = scalingB;
    m_bitmaskPendingChanges |= sizeAndScaleChanged(changedDimmA, changedDimmB);
  }
protected:
  virtual int           sizeAndScaleChanged(bool changedDimmA, bool changedDimmB) { return 0; }
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
    unsigned int old_scaling_A = m_scalingA;
    unsigned int old_scaling_B = m_scalingB;
    clampScaling(&m_scalingA, &m_scalingB);
    if (m_scalingB != old_scaling_B  && m_datex == DATEX_1D)
    {
      float coeff = float(old_scaling_B) / m_scalingB;
      m_matrixDimmB = (unsigned int)(m_matrixDimmB*coeff + 0.5f);
    }
//    m_bitmaskPendingChanges |= sizeAndScaleChanged(old_scaling_A != m_scalingA, old_scaling_B != m_scalingB);
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
  bool            paletteInterpolation()const { return m_dataTextureInterp; }
public:
  /// 1. Data methods
  virtual void setData(const float* data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = data[i]; 
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA))   callWidgetUpdate();
  }
  virtual void setData(const float* data, DataDecimator* decim)
  {
    for (unsigned int p=0; p<m_countPortions; p++)
      for (unsigned int i=0; i<m_portionSize; i++)
        m_matrixData[p*m_portionSize + i] = decim->decimate(data, (int)m_portionSize, (int)i, (int)p);
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  virtual void fillData(float data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = data; 
    
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  virtual void clearData()
  {
    unsigned int total = m_allocatedPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = 0;
    m_bitmaskPendingChanges |= PC_DATA;
    if (!autoUpdateBanned(RD_BYDATA)) callWidgetUpdate();
  }
  
  void getData(float* result) const
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      result[i] = m_matrixData[i];
  }
  const float*  getDataPtr() const {   return m_matrixData;    }
  float*        getDataPtr() {    return m_matrixData;   }     // You need manual update after data change
  void  manualDataPtrUpdate(){  vmanUpData(); }
public:
  bool  getMinMax(float* rmin, float* rmax)
  {
    if (m_portionSize <= 1) return false;
    float min = m_matrixData[0], max = m_matrixData[0];
    for (unsigned int p=0; p<m_countPortions; p++)
      for (unsigned int i=0; i<m_portionSize; i++)
      {
        if (min > m_matrixData[p*m_portionSize + i])
          min = m_matrixData[p*m_portionSize + i];
        if (max < m_matrixData[p*m_portionSize + i])
          max = m_matrixData[p*m_portionSize + i];
      }
    if (rmin) *rmin = min;
    if (rmax) *rmax = max;
    return true;
  }
  void  adjustBounds()
  {
    float min,max;
    if (getMinMax(&min, &max))
      setBounds(min, max);
  }
  void  adjustBoundsWithSpacingAdd(float add2min, float add2max)
  {
    float min,max;
    if (getMinMax(&min, &max))
      setBounds(min + add2min, max + add2max);
  }
  void  adjustBoundsWithSpacingMul(float mul2min, float mul2max)
  {
    float min,max;
    if (getMinMax(&min, &max))
      setBounds(min*mul2min, max*mul2max);
  }
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
    m_impulsedata = id;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  void  setImpulseOff()
  {
    if (m_impulsedata.type != impulsedata_t::IR_OFF)
    {
      m_impulsedata.type = impulsedata_t::IR_OFF;
      m_impulsedata.count = m_impulsedata.central = 0;
      m_bitmaskPendingChanges |= PC_INIT;
      if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
    }
  }
  void  setImpulseCoeffA(int count, const float coeffs[], int central, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_A_COEFF_NOSCALED : impulsedata_t::IR_A_COEFF, count, central, cycled? 1 : 0, {} };
    for (int i=0; i<count; i++)
      m_impulsedata.coeff[i] = coeffs[i];
    setImpulse(idt);
  }
  void  setImpulseCoeffB(int count, const float coeffs[], int central, bool noscaled=false, bool cycled=false)
  {
    impulsedata_t idt = { noscaled? impulsedata_t::IR_B_COEFF_NOSCALED : impulsedata_t::IR_B_COEFF, count, central, cycled? 1 : 0, {} };
    for (int i=0; i<count; i++)
      m_impulsedata.coeff[i] = coeffs[i];
    setImpulse(idt);
  }
  void  setImpulseBordersA(int minscaling, int bordersize, bool fixed=true)
  {
    impulsedata_t idt = { fixed? impulsedata_t::IR_A_BORDERS_FIXEDCOUNT : impulsedata_t::IR_A_BORDERS, minscaling, bordersize, 0, {} };
    setImpulse(idt);
  }
  void  setImpulseBordersB(int minscaling, int bordersize, bool fixed=true)
  {
    impulsedata_t idt = { fixed? impulsedata_t::IR_B_BORDERS_FIXEDCOUNT : impulsedata_t::IR_B_BORDERS, minscaling, bordersize, 0, {} };
    setImpulse(idt);
  }
  const impulsedata_t& impulse() const { return m_impulsedata; }
  
  void  setViewAlign(DRAWVIEWALIGN dva){  m_viewAlign = dva; callWidgetUpdate(); }
  DRAWVIEWALIGN  viewAlign() const {  return m_viewAlign; }
public:
  
  void  setOrientation(ORIENTATION orient)
  { 
    m_orient = orient;
    m_matrixSwitchAB = orient > OR_RLTB;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  ORIENTATION     orientation() const { return m_orient; }
  void  setTransponed(bool r)
  {
    m_orient = (ORIENTATION)((int)m_orient + (m_orient > OR_RLTB? ( r? 0 : -4 ) : ( r? 4 : 0 )));
    m_matrixSwitchAB = r;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  bool  transponed() const {  return m_matrixSwitchAB;  }
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
  IProactive*  setProactive(IProactive* op, bool owner=true)
  {
    m_proactiveOwner = owner;
    IProactive* result = m_proactive; m_proactive = op; return result;
  }
public:
  /// 3. Overlays
  int             ovlPushBack(DrawOverlay* povl, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT) return -1;
    _ovlSet(m_overlaysCount, povl, owner, false, 0);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ++m_overlaysCount;
  }
  int             ovlPushBack(DrawOverlay* povl, int ovlroot, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT || ovlroot <= 0 || ovlroot > (int)m_overlaysCount) return -1;
    _ovlSet(m_overlaysCount, povl, owner, true, ovlroot-1);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ++m_overlaysCount;
  }
  int             ovlReplace(int ovl, DrawOverlay* povl, bool owner, DrawOverlay** old=nullptr, bool followPrevRoot=true)
  {
    ovl--;
    msstruct_t prevroot = m_overlays[ovl].olinks;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, owner, followPrevRoot & (prevroot.type == msstruct_t::MS_DRIVEN), prevroot.details.drivenid);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ovl + 1;
  }
  int             ovlReplace(int ovl, DrawOverlay* povl, bool owner, DrawOverlay** old, int ovlroot)
  {
    ovl--;  ovlroot--;
    if (ovlroot >= ovl) return -1;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, owner, true, ovlroot);
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    return ovl + 1;
  }
  DrawOverlay*       ovlRemove(int ovl)
  {
    ovl--;
    DrawOverlay* result = nullptr;
    if (_ovlRemove(ovl, &result) == false)
      return nullptr;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
    if (ovl+1 == (int)m_overlaysCount)
      while (m_overlays[ovl--].povl == &m_overlaySingleEmpty)
        m_overlaysCount--;
    return result;
  }
  DrawOverlay* ovlGet(int ovl) const
  {
    ovl -= 1;
    if (ovl < 0 || (unsigned int)ovl >= m_overlaysCount) return nullptr;
    if (m_overlays[ovl].povl == &m_overlaySingleEmpty)  return nullptr;
    return m_overlays[ovl].povl;
  }
  DrawOverlay* ovlPopBack()
  {
    DrawOverlay* result = nullptr;
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
  void _ovlSet(int idx, DrawOverlay* povl, bool owner, bool doRoot, int rootidx)
  {
    if (povl == nullptr){  povl = &m_overlaySingleEmpty;  owner = false; }
    m_overlays[idx]._reinit(povl, povl->uniforms().count);
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
  bool  _ovlRemove(int idx, DrawOverlay** old)
  {
    if (idx < 0 || idx >= (int)m_overlaysCount) return false;
    DrawOverlay* removable = m_overlays[idx].povl;
    if (removable == &m_overlaySingleEmpty)
      removable = nullptr;
    else
    {
      m_overlays[idx]._reinit(&m_overlaySingleEmpty, 0);
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
  /// 4. Optimized/Native posteffect methods
  void            setPostMask(const DPostmask& fsp)
  {
    m_postMask = fsp;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
  DPostmask    postMask() const { return m_postMask; }
protected:
  virtual void            overlayUpdate(int ovl, bool internal, bool noupdate, bool recreate)
  {
    m_overlays[ovl].upcount += recreate? 1001 : 1;
    m_bitmaskPendingChanges |= (internal? PC_INIT : PC_PARAMSOVL);
    if (internal == true || (!internal && noupdate == false))
      if (!autoUpdateBanned(RD_BYOVL_ACTIONS))
        callWidgetUpdate();
  }
  virtual void            innerOverlayReplace(int ovlid, DrawOverlay* novl, bool owner)
  {
    m_overlays[ovlid]._reinit(novl, novl->uniforms().count);  ///_setdriven derived
    if (novl != &m_overlaySingleEmpty)
    {
      novl->assign(ovlid, this, owner);
    }
    m_overlays[ovlid].upcount = 1001;
    m_bitmaskPendingChanges |= PC_INIT;
    if (!autoUpdateBanned(RD_BYOVL_ADDREMOVE)) callWidgetUpdate();
  }
  virtual void            innerOverlayRemove(int ovlid)
  {
    m_overlays[ovlid]._reinit(&m_overlaySingleEmpty, 0);
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
};


#endif // DRAWCORE_H
