#ifndef BSMARKS_H
#define BSMARKS_H

#include "../../core/bsoverlay.h"

class OMarkDashs: public IOverlayHard, public OVLCoordsStatic, public OVLDimmsOff
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
  void  updateMark(unsigned int mark, float tcolor, float pos){ if (mark < m_maxmarks) m_marks[mark] = { tcolor, pos}; }
  void  updateFinished(){ IOverlay::overlayUpdateParameter(); }
  unsigned int countMarks() const { return m_maxmarks; }
};



/// max figures count limited by videoadapter
class OMarkFigures: public IOverlayHard, public OVLCoordsStatic, public OVLDimmsOff
{
protected:
  unsigned int      m_maxfigures;
  struct            figinfo_t;
  struct figinfo_t  *m_figures;
  virtual int fshTrace(int overlay, bool rotated, char* to) const;
private:
  dmtype_arr_t      m_dm_coords;
  dmtype_arr_t      m_dm_form;
  COORDINATION      m_featcn;
  float             m_figsize, m_figopc;
public:
  enum  FFORM { F_OFF=0, F_SQUARE, F_CIRCLE, F_CROSS, F_TGL_UP, F_TGL_DOWN };
public:
  OMarkFigures(unsigned int maxfigures, COORDINATION featcn, float figsize, const IPalette* ipal, bool discrete, float figopacity=0.5f);
  ~OMarkFigures();
public:
  ///   Idx from zero
  void  updateFigure(unsigned int idx, float x, float y);
  void  updateFigureColor(unsigned int idx, float color);
  void  updateFigureZoom(unsigned int idx, float zoom);
  void  updateFigureForm(unsigned int idx, FFORM form);
  void  updateFigure(unsigned int idx, float x, float y, float color);
  void  updateFigure(unsigned int idx, float x, float y, float color, FFORM form, float zoom=1.0f);
  void  updateFinished();
public:
  unsigned  int countFigures() const { return m_maxfigures; }
public:
//  void  updatePalette(const IPalette* ipal, bool discrete);
};



#endif // BSMARKS
