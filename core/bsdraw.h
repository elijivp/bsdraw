#ifndef DRAWCORE_H
#define DRAWCORE_H

#ifndef BSOVERLAYSLIMIT
// Using default bigsized overlay
#endif

#include "bsidrawcore.h"
#include "bsoverlay.h"

//#########################
struct  bounds_t
{
  float   LL, HL;             /// Lowest and highest colors from palette
  bounds_t(float ll=0, float hl=1): LL(ll), HL(hl){}
};
struct  contrast_t
{
  float   contrast, offset;   /// k*x + b
  contrast_t(float c=1, float o=0): contrast(c), offset(o){}
};
enum  ORIENTATION       { OR_LRBT=0, OR_RLBT, OR_LRTB, OR_RLTB };
//enum  INTERPOLATION     { IT_NONE, IT_STATIC, IT_DATALINE, IT_NEAREST  };

inline  void  _bsdraw_update_kb(const bounds_t& bnd, const contrast_t& cnt, float* k, float *b)
{
  // (x - bounds.x)/(bounds.y-bounds.x)*contrast + offset
  // k = contrast/(bounds.y-bounds.x); b = -bounds.x*contrast/(bounds.y-bounds.x) + offset
  *k = cnt.contrast/(bnd.HL - bnd.LL);
  *b = -bnd.LL*cnt.contrast/(bnd.HL - bnd.LL) + cnt.offset;
}

//#########################

class DrawCore: public IDrawOverlayFriendly
{
protected:
  bounds_t              m_bounds;
  contrast_t            m_contrast;
  float                 m_loc_k, m_loc_b;
  unsigned int          m_portionSize;        /// inner sizeof data
  unsigned int          m_countPortions;      /// count of parallel drawings in one
  
  ORIENTATION           m_orient;
  bool                  m_interpPal;
  
  float*                m_matrixData;
  float*                m_matrixDataCached;
  unsigned int          m_matrixWidth;
  unsigned int          m_matrixHeight;
  
  unsigned int          m_matrixScWidth;
  unsigned int          m_matrixScHeight;
  unsigned int          m_matrixMinScWidth, m_matrixMaxScWidth;
  unsigned int          m_matrixMinScHeight, m_matrixMaxScHeight;
  bool                  m_isSyncedSc;
protected:
  const IPalette*       m_ppal;
  bool                  m_ppaldiscretise;
  float                 m_clearcolor[3];
  bool                  m_clearbypalette;
protected:
  float*                m_dataDomains;
  bool                  m_dataDomainsFastFree;
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
  enum                  PCBM  { PC_INIT=1, PC_SIZE=2, PC_DOMAIN=4, PC_DATA=8, PC_PALETTE=16, PC_PARAMS=32, PC_PARAMSOVL=128  };
  bool                  havePendOn(PCBM bit) const {  return (m_bitmaskPendingChanges & bit) != 0; }
  bool                  havePending() const {  return m_bitmaskPendingChanges != 0; } 
  void                  unpend(PCBM bit) {  m_bitmaskPendingChanges &= ~bit; }
  void                  unpendAll() {  m_bitmaskPendingChanges = 0; }
  
  void                  pendResize(bool autorepaint){    m_bitmaskPendingChanges |= PC_SIZE;  if (!autorepaint) innerResize(); }
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
    IOverlay*           povl;
    int                 upcount;
    int                 outloc;
    unsigned int        uf_count;
    uniform_located_t*  uf_arr;
    msstruct_t          olinks;
    void                _reinit(IOverlay* p, unsigned int ufcount){ povl = p; upcount = 1; outloc = -1;  if (uf_count) delete []uf_arr;  uf_count = ufcount; if(uf_count) uf_arr = new uniform_located_t[ufcount]; olinks.type = msstruct_t::MS_SELF; }
    void                _setdriven(int driverid){ olinks.type = msstruct_t::MS_DRIVEN;  olinks.details.drivenid = driverid; }
    overlays_t(): povl(nullptr), uf_count(0){}
    ~overlays_t() { if (uf_count) delete[]uf_arr; }
    
  }                     m_overlays[OVLLIMIT];
  unsigned int          m_overlaysCount;  
  class OverlayEmpty: public IOverlay
  {
  public:
    virtual int  fshTrace(int, char*) const;
    virtual int  fshColor(int, char*) const;
  };
  OverlayEmpty          m_overlaySingleEmpty;
public:
  DrawCore(unsigned int portions, ORIENTATION orient): m_portionSize(0), m_countPortions(portions), m_orient(orient), m_interpPal(false), m_matrixData(nullptr), m_matrixDataCached(nullptr), m_matrixScWidth(1), m_matrixScHeight(1),
    m_matrixMinScWidth(1), m_matrixMaxScWidth(0), m_matrixMinScHeight(1), m_matrixMaxScHeight(0), m_isSyncedSc(false),
    m_ppal(nullptr), m_ppaldiscretise(false), m_clearbypalette(true), 
    m_dataDomains(nullptr), m_dataDomainsFastFree(true),
    m_bitmaskUpdateBan(0), m_bitmaskPendingChanges(PC_INIT), 
    m_postMask(DPostmask::PM_NONE, DPostmask::PO_OFF, 0, 0.0f, 0.0f, 0.0f),
    m_overlaysCount(0)
  {
    _bsdraw_update_kb(m_bounds, m_contrast, &m_loc_k, &m_loc_b);
  }
  ~DrawCore(){ _ovlRemoveAll(); if (m_matrixData) delete []m_matrixData; if (m_matrixDataCached)  delete []m_matrixDataCached; }
protected:
  void    deployMemory()
  {
    unsigned int total = m_countPortions * m_portionSize;
    m_matrixData = new float[total];
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = 0;
    m_matrixDataCached = new float[total];
  }
  void    deployMemory(unsigned int total) {  m_matrixData = new float[total]; for (unsigned int i=0; i<total; i++) m_matrixData[i] = 0; m_matrixDataCached = new float[total]; }
public:
  
public:
                    /// Access methods
  unsigned int          countPortions() const { return m_countPortions; }
  
  void                  setBounds(const bounds_t& d){ m_bounds = d; _bsdraw_update_kb(m_bounds, m_contrast, &m_loc_k, &m_loc_b);  m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYSETTINGS); }
  void                  setBoundLow(float LL){ m_bounds.LL = LL; _bsdraw_update_kb(m_bounds, m_contrast, &m_loc_k, &m_loc_b);  m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYSETTINGS); }
  void                  setBoundHigh(float HL){ m_bounds.HL = HL; _bsdraw_update_kb(m_bounds, m_contrast, &m_loc_k, &m_loc_b);  m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYSETTINGS); }
  bounds_t              bounds() const { return m_bounds; }
  
  void                  setContrast(const contrast_t& c){ m_contrast = c; _bsdraw_update_kb(m_bounds, m_contrast, &m_loc_k, &m_loc_b);  m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYSETTINGS); }
  contrast_t            contrast()const{ return m_contrast; }
public:
  void                  setScalingLimitsH(unsigned int scmin, unsigned int scmax=0){  m_matrixMinScWidth = scmin < 1? 1 : scmin; m_matrixMaxScWidth = scmax; m_isSyncedSc = false; clampScalingManually(); pendResize(false); }
  void                  setScalingLimitsV(unsigned int scmin, unsigned int scmax=0){  m_matrixMinScHeight = scmin < 1? 1 : scmin; m_matrixMaxScHeight = scmax; m_isSyncedSc = false; clampScalingManually(); pendResize(false); }
  void                  setScalingLimitsSynced(unsigned int scmin, unsigned int scmax=0){  m_isSyncedSc = true; m_matrixMinScWidth = m_matrixMinScHeight = scmin < 1? 1 : scmin; m_matrixMaxScWidth = m_matrixMaxScHeight = scmax; clampScalingManually(); pendResize(false); }
  
  void                  scalingLimitsH(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_matrixMinScWidth;  if (scmax) *scmax = m_matrixMaxScWidth; }
  void                  scalingLimitsV(unsigned int *scmin, unsigned int *scmax=nullptr) const { *scmin = m_matrixMinScHeight;  if (scmax) *scmax = m_matrixMaxScHeight; }
protected:
  void                  clampScaling()
  {
    if (m_matrixScWidth < m_matrixMinScWidth) m_matrixScWidth = m_matrixMinScWidth;
    if (m_matrixMaxScWidth && m_matrixScWidth > m_matrixMaxScWidth) m_matrixScWidth = m_matrixMaxScWidth;
    if (m_matrixScHeight < m_matrixMinScHeight) m_matrixScHeight = m_matrixMinScHeight;
    if (m_matrixMaxScHeight && m_matrixScHeight > m_matrixMaxScHeight)  m_matrixScHeight = m_matrixMaxScHeight;
    
    if (m_isSyncedSc)
    {
      if (m_matrixScHeight > m_matrixScWidth || (getDataDimmUsage() != DDU_2D && getDataDimmUsage() != DDU_DD)) m_matrixScHeight = m_matrixScWidth;
      else m_matrixScWidth = m_matrixScHeight;
    }
  }
private:
  void                  clampScalingManually()
  {
    unsigned int old_matrixScHeight = m_matrixScHeight;
    clampScaling();
    if (m_matrixScHeight != old_matrixScHeight && (getDataDimmUsage() == DDU_1D))
    {
      float coeff = float(old_matrixScHeight) / m_matrixScHeight; 
      m_matrixHeight = (unsigned int)(m_matrixHeight*coeff + 0.5f);
    }
  }
public:
  void  setDataPalette(const IPalette* ppal)
  {
    m_ppal = ppal;
    m_bitmaskPendingChanges |= PC_PALETTE; innerUpdate(RD_BYSETTINGS);
  }
  void  setDataPaletteDiscretion(bool d)
  {
    m_ppaldiscretise = d;
    m_bitmaskPendingChanges |= PC_PALETTE; innerUpdate(RD_BYSETTINGS);
  }
public:
  /// 1. Data methods
  virtual void setData(const float* data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = data[i]; 
    
    m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYDATA);
  }
  virtual void setData(const float* data, DataDecimator* decim)
  {
    for (unsigned int p=0; p<m_countPortions; p++)
      for (unsigned int i=0; i<m_portionSize; i++)
        m_matrixData[p*m_portionSize + i] = decim->decimate(data, m_portionSize, i, p);
    
    m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYDATA);
  }
  virtual void fillData(float data)
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = data; 
    
    m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYDATA);
  }
  virtual void clearData()
  {
    unsigned int total = m_countPortions * m_portionSize;
    for (unsigned int i=0; i<total; i++)
      m_matrixData[i] = 0;
    m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYDATA);
  }
protected:
  void  vmanUpData(){ m_bitmaskPendingChanges |= PC_DATA; innerUpdate(RD_BYDATA);  }
  enum  DATADIMMUSAGE { DDU_2D, DDU_15D, DDU_1D, DDU_DD };
  virtual DATADIMMUSAGE   getDataDimmUsage() const =0;
public:
  unsigned int directions() const { DATADIMMUSAGE ddu = getDataDimmUsage(); if (ddu == DDU_2D || ddu == DDU_DD) return 2; return 1; }
public:
  /// 2. Delegated methods
public:
  void  setOrientation(ORIENTATION orient)
  { 
    m_orient = orient;
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYSETTINGS);
  }
  ORIENTATION     orientation() const { return m_orient; }
public:
  /// 3. Overlays
  int             ovlPushBack(IOverlay* povl, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT) return -1;
    _ovlSet(m_overlaysCount, povl, owner, false, 0);
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    return ++m_overlaysCount;
  }
  int             ovlPushBack(IOverlay* povl, int ovlroot, bool owner=true)
  {
    if (m_overlaysCount == OVLLIMIT || ovlroot <= 0 || ovlroot > (int)m_overlaysCount) return -1;
    _ovlSet(m_overlaysCount, povl, owner, true, ovlroot-1);
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    return ++m_overlaysCount;
  }
  int             ovlReplace(int ovl, IOverlay* povl, bool owner, IOverlay** old=nullptr, bool followPrevRoot=true)
  {
    ovl--;
    msstruct_t prevroot = m_overlays[ovl].olinks;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, owner, followPrevRoot & (prevroot.type == msstruct_t::MS_DRIVEN), prevroot.details.drivenid);
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    return ovl + 1;
  }
  int             ovlReplace(int ovl, IOverlay* povl, bool owner, IOverlay** old, int ovlroot)
  {
    ovl--;  ovlroot--;
    if (ovlroot >= ovl) return -1;
    if (_ovlRemove(ovl, old) == false)
      return -1;
    _ovlSet(ovl, povl, owner, true, ovlroot);
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    return ovl + 1;
  }
  IOverlay*       ovlRemove(int ovl)
  {
    ovl--;
    IOverlay* result = nullptr;
    if (_ovlRemove(ovl, &result) == false)
      return nullptr;
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    if (ovl+1 == (int)m_overlaysCount)
      while (m_overlays[ovl--].povl == &m_overlaySingleEmpty)
        m_overlaysCount--;
    return result;
  }
  IOverlay* ovlGet(int ovl) const
  {
    ovl -= 1;
    if (ovl < 0 || (unsigned int)ovl >= m_overlaysCount) return nullptr;
    if (m_overlays[ovl].povl == &m_overlaySingleEmpty)  return nullptr;
    return m_overlays[ovl].povl;
  }
  IOverlay* ovlPopBack()
  {
    IOverlay* result = nullptr;
    if (_ovlRemove((int)m_overlaysCount - 1, &result))
    {
      m_overlaysCount--;
      m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
    }
    return result;
  }
  void      ovlClearAll()
  {
    _ovlRemoveAll();
    m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYOVL_ADDREMOVE);
  }
private:
  void _ovlSet(int idx, IOverlay* povl, bool owner, bool doRoot, int rootidx)
  {
    if (povl == nullptr)  povl = &m_overlaySingleEmpty;
    m_overlays[idx]._reinit(povl, povl->uniforms().count);
    if (doRoot)
      m_overlays[idx]._setdriven(rootidx);
    povl->assign(idx, this, owner);
  }
  void _ovlRemoveAll()
  {
    for (unsigned int i=0; i<m_overlaysCount; i++)
      if (m_overlays[i].povl != &m_overlaySingleEmpty)
        if (m_overlays[i].povl->overlayPreDelete(this) == true)
          delete m_overlays[i].povl;
    m_overlaysCount = 0;
  }
  bool  _ovlRemove(int idx, IOverlay** old)
  {
    if (idx < 0 || idx >= (int)m_overlaysCount) return false;
    IOverlay* removable = m_overlays[idx].povl;
    if (removable == &m_overlaySingleEmpty)
      removable = nullptr;
    else
    {
      m_overlays[idx]._reinit(&m_overlaySingleEmpty, 0);
      if (removable->overlayPreDelete(this) == true)
      {
        delete removable;
        removable = nullptr;
      }
    }
    if (old) *old = removable;
    return true;
  }
public:
  /// 4. Optimized/Native interpolation or posteffect methods
  void            setPaletteInterpolation(bool interp)
  {
//    bool dataup = (m_interpPal == IT_STATIC || interp == IT_STATIC) && m_interpPal != interp;
    m_interpPal = interp;
//    m_bitmaskPendingChanges |= dataup? PC_DATA : PC_PARAMS; innerUpdate(RD_BYSETTINGS);
    m_bitmaskPendingChanges = PC_DATA; innerUpdate(RD_BYSETTINGS);
  }
  bool            paletteInterpolation()const { return m_interpPal; }
public:
  void            setPostMask(const DPostmask& fsp){ m_postMask = fsp; m_bitmaskPendingChanges |= PC_INIT; innerUpdate(RD_BYSETTINGS); }
  DPostmask    postMask() const { return m_postMask; }
protected:
  virtual void            overlayUpdate(int ovl, bool internal, bool noupdate, bool recreate)
  {
    m_overlays[ovl].upcount += recreate? 1001 : 1;
    m_bitmaskPendingChanges |= (internal? PC_INIT : PC_PARAMSOVL); if (internal == true || (!internal && noupdate == false)) innerUpdate(RD_BYOVL_ACTIONS);
  }
  virtual void            innerOverlayRemove(int ovl)
  {
    m_overlays[ovl]._reinit(&m_overlaySingleEmpty, 0); m_bitmaskPendingChanges |= PC_INIT;  innerUpdate(RD_BYOVL_ADDREMOVE);
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
    m_clearbypalette = false;
    _colorCvt(clearcolor);
    innerUpdate(RD_BYSETTINGS);
  }
  void            setClearByPalette()
  {
    m_clearbypalette = true;
    innerUpdate(RD_BYSETTINGS);
  }
protected:
  virtual void    innerUpdate(REDRAWBY)=0;
  virtual void    innerResize()=0;
};

#endif // DRAWCORE_H
