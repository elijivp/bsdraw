#ifndef BSBORDER_H
#define BSBORDER_H

/// Overlays:     borders and other types of cover around draw
///   OBorder. View: classical border
///   OBorderSelected. View: border for subdraws, selectable 
///   OToons. View: rounded border
/// Created By: Elijah Vlasov

#include "../core/bsoverlay.h"

enum  { OBLINE_LEFT=1, OBLINE_TOP=2, OBLINE_RIGHT=4, OBLINE_BOTTOM=8 };

class OBorder: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
  unsigned int  m_width;
  unsigned int  m_lineset;
public:
  OBorder(unsigned int widthpixels, const linestyle_t& kls=linestyle_solid(1,1,1), int lineset=OBLINE_LEFT|OBLINE_TOP|OBLINE_RIGHT|OBLINE_BOTTOM);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class _OSelected: public DrawOverlayTraced, public OVLCoordsOff, public OVLDimmsOff
{
protected:
  int  m_selected;
  unsigned int  m_width;
public:
  _OSelected(unsigned int widthpixels, int default_selection=0, const linestyle_t& kls=linestyle_solid(1,1,1));
  void  setSelection(int select);
  int selection() const { return m_selected; }  
};

class OBorderSelected: public _OSelected
{
public:
  OBorderSelected(unsigned int widthpixels, int default_selection=0, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class ORowSelected: public _OSelected
{
public:
  ORowSelected(unsigned int widthpixels, int default_selection=0, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OColumnSelected: public _OSelected
{
public:
  OColumnSelected(unsigned int widthpixels, int default_selection=0, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};

class OColumnsSelected: public _OSelected
{
  int   al,ar;
  bool  cyclic;
public:
  OColumnsSelected(unsigned int widthpixels, int default_selection=0, int addLeft=0, int addRight=0, bool cyclic=false, const linestyle_t& kls=linestyle_solid(1,1,1));
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
};


class OToons: public DrawOverlayTraced, public OVLCoordsStatic, public OVLDimms1Static
{
  float         m_radius2, m_border;
  bool          m_banclicks;
public:
  OToons(COORDINATION cr, float diameter, float border, const linestyle_t& =linestyle_solid(0,0,0), bool banclikcks=true);
protected:
  virtual int   fshTrace(int overlay, bool rotated, char* to) const;
//  virtual bool  clickBanned(float x, float y) const;
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool* doStop);
};

#endif // BSBORDER_H

