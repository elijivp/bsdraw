#ifndef BSBOUNDED_H
#define BSBOUNDED_H

/// Overlays:   data bound-depended (low and high data bounds)
///   OLevel. View: horizontal line across value
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OLevel: public Ovldraw_ColorTraced, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_value;
public:
  OLevel(float value, const linestyle_t& linestyle=linestyle_inverse_1(0,1,1));
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
};


class OLevelVariable: public Ovldraw_ColorTraced, public OVLCoordsOff, public OVLDimmsOff
{
  float   m_value;
public:
  OLevelVariable(float value, const linestyle_t& linestyle=linestyle_inverse_1(0,1,1));
  void    setLevel(float v, bool update=true);
  float   level() const { return m_value; }
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
};


//////////////////////


// recorder automovable line
class ORecLine: public Ovldraw_ColorTraced, public OVLCoordsOff, public OVLDimmsOff
{
  enum  { TOTAL = 33, LAST = 32 };
  int           ctr;
  int           links[TOTAL];
  dmtype_arr_t _dm_coords;
public:
  ORecLine(const linestyle_t& linestyle);
public:
  void      increment(bool activate, bool update=true);
protected:
  virtual int fshOVCoords(int overlay, bool switchedab, char* to) const;
};


//////////////////////

class _OLevelSet: public Ovldraw, public OVLCoordsOff, public OVLDimmsOff
{
protected:
  struct    line_t
  {
    float activated;
    float offset;
  };
private:
  enum  LINETYPE {  LT_HORZ, LT_VERT  };
  unsigned int  m_activecount;
  COORDINATION  m_cn;
  bool          m_static;
  float         m_mr1, m_mr2;
  LINETYPE      m_lt;
  
  linestyle_t*    m_kls;
  dmtype_arr_t    dtarr;
  line_t*         m_data;
public:
  _OLevelSet(unsigned int maxcount, linestyle_t* pkls, line_t* poffsets, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f);
  
  unsigned int    count() const { return m_activecount; }
  unsigned int    total() const { return dtarr.count; }
  bool            empty() const { return m_activecount == 0; }
  float           offset(int idx) const { return m_data[idx].offset; }
  linestyle_t     linestyle(int idx) const { return m_kls[idx]; }
  
  int   lineAdd(float offset, bool update=true);
  int   lineNextFreeIndex() const;
  
  void  lineShow(int idx, float offset, bool update=true);
  void  lineShow(int idx, bool update=true);
  
  void  lineHide(int idx, bool update=true);
  bool  lineVisible(int idx) const { return m_data[idx].activated > 0; }
  void  clear(bool update=true);
protected:
  int   fshOVCoords(int overlay, bool switchedab, char* to) const;
  int   fshColor(int overlay, char* to) const;
};

template <int N>
class _OLevelSetMemory: public _OLevelSet
{
protected:
  linestyle_t   transfer_kls[N];
  line_t        transfer_offsets[N];
public:
  _OLevelSetMemory(COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f):
    _OLevelSet(N, transfer_kls, transfer_offsets, cn, isstatic, margin1, margin2)
  {
  }
};

class OLevelSet: public _OLevelSetMemory<5>
{
public:
  enum  
  {
    FL_RGBOP, FL_GBOPR, FL_BOPRG, FL_OPRGB, FL_PRGBO,
    FL_GBROP, FL_BGROP, FL_BRGOP, FL_GRBOP, FL_RBGOP,
    FL_GBORP, FL_BGORP, FL_BOGRP, FL_GOBRP, FL_OBGRP,
    _FL_SCHEMAS_COUNT,
    
    FL_CS_0_STANDARD = 0x0000,    // 1..F
    FL_CS_0_BRIGHTMED = 0x8000,    // 1..F
    FL_CS_0_BRIGHTFULL = 0xF000,    // 1..F
    FL_CS_1_STANDARD = 0x00000,    // 1..F
    FL_CS_1_BRIGHTMED = 0x80000,    // 1..F
    FL_CS_1_BRIGHTFULL = 0xF0000,    // 1..F
    FL_CS_2_STANDARD = 0x000000,    // 1..F
    FL_CS_2_BRIGHTMED = 0x800000,    // 1..F
    FL_CS_2_BRIGHTFULL = 0xF00000,    // 1..F
    FL_CS_3_STANDARD = 0x0000000,    // 1..F
    FL_CS_3_BRIGHTMED = 0x8000000,    // 1..F
    FL_CS_3_BRIGHTFULL = 0xF000000,    // 1..F
    FL_CS_4_STANDARD = 0x00000000,    // 1..F
    FL_CS_4_BRIGHTMED = 0x80000000,    // 1..F
    FL_CS_4_BRIGHTFULL = 0xF0000000,    // 1..F
  };
  OLevelSet(int flags, int lstroke, int lspace, int ldots, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f);
};

class OLevelSet1: public _OLevelSetMemory<1>
{
public:
  OLevelSet1(linestyle_t kls, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    transfer_kls[0] = kls;  }
};

class OLevelSet2: public _OLevelSetMemory<2>
{
public:
  OLevelSet2(linestyle_t kls1, linestyle_t kls2, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    transfer_kls[0] = kls1; transfer_kls[1] = kls2;  }
  OLevelSet2(linestyle_t* pkls, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    for (unsigned int i=0; i<sizeof(transfer_kls)/sizeof(linestyle_t); i++)  transfer_kls[i] = pkls[i];  }
};

class OLevelSet3: public _OLevelSetMemory<3>
{
public:
  OLevelSet3(linestyle_t kls1, linestyle_t kls2, linestyle_t kls3, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    transfer_kls[0] = kls1; transfer_kls[1] = kls2; transfer_kls[2] = kls3;  }
  OLevelSet3(linestyle_t* pkls, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    for (unsigned int i=0; i<sizeof(transfer_kls)/sizeof(linestyle_t); i++)  transfer_kls[i] = pkls[i];  }
};

class OLevelSet4: public _OLevelSetMemory<4>
{
public:
  OLevelSet4(linestyle_t kls1, linestyle_t kls2, linestyle_t kls3, linestyle_t kls4, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    transfer_kls[0] = kls1; transfer_kls[1] = kls2; transfer_kls[2] = kls3; transfer_kls[3] = kls4;  }
  OLevelSet4(linestyle_t* pkls, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    for (unsigned int i=0; i<sizeof(transfer_kls)/sizeof(linestyle_t); i++)  transfer_kls[i] = pkls[i];  }
};

class OLevelSet5: public _OLevelSetMemory<5>
{
public:
  OLevelSet5(linestyle_t kls1, linestyle_t kls2, linestyle_t kls3, linestyle_t kls4, linestyle_t kls5, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    transfer_kls[0] = kls1; transfer_kls[1] = kls2; transfer_kls[2] = kls3; transfer_kls[3] = kls4; transfer_kls[4] = kls5;  }
  OLevelSet5(linestyle_t* pkls, COORDINATION cn, bool isstatic=false, float margin1=0.0f, float margin2=0.0f): _OLevelSetMemory(cn, isstatic, margin1, margin2)
  {    for (unsigned int i=0; i<sizeof(transfer_kls)/sizeof(linestyle_t); i++)  transfer_kls[i] = pkls[i];  }
};


#endif // BSBOUNDED_H
