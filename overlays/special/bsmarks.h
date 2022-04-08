#ifndef BSMARKS_H
#define BSMARKS_H

/// Overlays:   set of little figures
///   OMarkDashs. View: little lines like | || | ||   | |
///   OCluster. View: little figures, types described by FFORM
/// Created By: Elijah Vlasov

#include "../../core/bsoverlay.h"

class OMarkDashs: public DrawOverlay_ColorThroughPalette, public OVLCoordsStatic, public OVLDimmsOff
{
protected:
  unsigned int      m_maxmarks;
  struct markinfo_t {
    float   tcolor;
    float   pos;
  }                 *m_marks;
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
private:
  dmtype_arr_t      m_dm_coords;
  COORDINATION      m_featcn;
  float             m_marksize;
public:
  OMarkDashs(unsigned int maxmarks, COORDINATION cn, float areaPos, COORDINATION featcn, float marksize, const IPalette* ipal, bool discrete);
  ~OMarkDashs();
public:
  void  updateMarkColor(unsigned int mark, float tcolor){ if (mark < m_maxmarks)  m_marks[mark].tcolor = tcolor; }
  void  updateMarkPos(unsigned int mark, float pos){ if (mark < m_maxmarks)  m_marks[mark].pos = pos; }
  void  updateMark(unsigned int mark, float tcolor, float pos){ if (mark < m_maxmarks){   m_marks[mark].tcolor = tcolor; m_marks[mark].pos = pos; } }
  void  updateFinished(){ _DrawOverlay::updateParameter(false, true); }
  unsigned int countMarks() const { return m_maxmarks; }
};



/// max figures count limited by videoadapter
class OCluster: public DrawOverlay_ColorThroughPalette, public OVLCoordsStatic, public OVLDimmsOff
{
public:
  struct    clusteritem_t
  {
    float   x, y;
    float   zoom;
    float   form_color;
  };
protected:
  bool              m_crossable;
  unsigned int      m_total;
  clusteritem_t*    m_items;
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
private:
  dmtype_arr_t      m_dm_coords;
  dmtype_arr_t      m_dm_form;
  COORDINATION      m_featcn;
  float             m_figsize, m_figopc;
public:
  enum  FFORM { F_OFF=0, F_SQUARE, F_CIRCLE, F_CROSS, F_RHOMB, 
                F_TGL_UP, F_TGL_DOWN, F_TGL_LEFT, F_TGL_RIGHT,
                _F_SPECIAL, _F_COUNT = _F_SPECIAL - 1
              };
public:
  OCluster(bool crossable, unsigned int maxfigures, COORDINATION featcn, float figsize, const IPalette* ipal, bool discrete, float figopacity=0.5f);
  ~OCluster();
  
  const clusteritem_t&  item(unsigned int idx) const {  return m_items[idx]; }
public:
  ///   Idx from zero
  void  updateItem(unsigned int idx, float x, float y);
  void  updateItemColor(unsigned int idx, float color);
  void  updateItemZoom(unsigned int idx, float zoom);
  void  updateItemForm(unsigned int idx, FFORM form);
  void  updateItem(unsigned int idx, float x, float y, float color);
  void  updateItem(unsigned int idx, float x, float y, float color, FFORM form, float zoom=1.0f);
  void  updateFinished();
public:
  unsigned  int count() const { return m_total; }
};



#endif // BSMARKS
