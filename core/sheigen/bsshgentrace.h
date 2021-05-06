#ifndef BSSHGENTRACE
#define BSSHGENTRACE

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsoverlay.h"

struct _bs_unzip_t
{
  int           type;     /// 0 - off, 1 - static, 2 - dynamic
  COORDINATION  cr;
  float         ffs[4];
};

template <class Coords>
void _bs_unzip_coords(const Coords* c, _bs_unzip_t* rslt);
template <> inline void _bs_unzip_coords(const OVLCoordsOff*, _bs_unzip_t* rslt){ rslt->type = 0; }
template <> inline void _bs_unzip_coords(const OVLCoordsStatic* c, _bs_unzip_t* rslt){ rslt->type = 1; rslt->cr = c->getCoordination(); c->getCoordinates(&rslt->ffs[0], &rslt->ffs[1]); }
template <> inline void _bs_unzip_coords(const OVLCoordsDynamic* c, _bs_unzip_t* rslt){ rslt->type = 2; rslt->cr = c->getCoordination(); }
template <> inline void _bs_unzip_coords(const OVLCoordsDimmsLinked* c, _bs_unzip_t* rslt){ rslt->type = 2; rslt->cr = c->getCoordination(); }

template <class Dimms>
void _bs_unzip_dimms(const Dimms* d, _bs_unzip_t* rslt);
template <> inline void _bs_unzip_dimms(const OVLDimmsOff*, _bs_unzip_t* rslt){ rslt->type = 0; }
template <> inline void _bs_unzip_dimms(const OVLDimms1Static* d, _bs_unzip_t* rslt){ rslt->type = 1; rslt->cr = d->getCoordination(); rslt->ffs[0] = d->getSide(); }
template <> inline void _bs_unzip_dimms(const OVLDimms1Dynamic* d, _bs_unzip_t* rslt){ rslt->type = 2; rslt->cr = d->getCoordination(); }
template <> inline void _bs_unzip_dimms(const OVLDimms2Static* d, _bs_unzip_t* rslt){ rslt->type = 3; rslt->cr = d->getCoordination(); d->getDimms(&rslt->ffs[0], &rslt->ffs[1]); }
template <> inline void _bs_unzip_dimms(const OVLDimms2Dynamic* d, _bs_unzip_t* rslt){ rslt->type = 4; rslt->cr = d->getCoordination(); }
template <> inline void _bs_unzip_dimms(const OVLDimmsStatic* d, _bs_unzip_t* rslt){ rslt->type = 5; rslt->cr = d->getCoordination(); d->dimms(&rslt->ffs[0], &rslt->ffs[1], &rslt->ffs[2], &rslt->ffs[3]); }
template <> inline void _bs_unzip_dimms(const OVLDimmsDynamic* d, _bs_unzip_t* rslt){ rslt->type = 6; rslt->cr = d->getCoordination(); }
template <> inline void _bs_unzip_dimms(const OVLCoordsDimmsLinked* d, _bs_unzip_t* rslt){ rslt->type = 7; rslt->cr = d->getCoordination(); }


class FshTraceGenerator
{
  int                                 m_overlay;
  bool                                m_rotated;
  const char*                         m_writebase;
  char* const                         m_to;
  int                                 m_offset;
  
  unsigned int                        loc_uniformsCount;
  const dmtype_t*                     loc_uniforms;
  
  enum                                { MM_PI=0, MM_EPS=1 };
  int                                 m_pixingsctr, m_relingsctr;
  int                                 m_maths;
  int                                 m_paramsctr;
  int                                 m_prmmemory[10];
  int                                 m_prmmemoryiter;
public:
  enum  OCG_INCLUDE_BITS  {  OINC_NONE=0, OINC_GETVALUE=1, OINC_RANDOM=2, OINC_DATABOUNDS=4 /*, OINC_PORTIONS=4*/ };
public:
  FshTraceGenerator(const AbstractDrawOverlay::uniforms_t& ufms, int overlay, bool rotated, char* deststring, int ocg_include_bits = 0);
  int  written() const { return m_offset; }
private:
  void  _gtb(OVL_ORIENTATION orient);
  void  _gtb_coords(const _bs_unzip_t& bsu);
  void  _gtb_dimms(const _bs_unzip_t& bsu);
public:
  template <class Coords, class Dimms>
  void  goto_func_begin(const Coords* coords, const Dimms* dimms, OVL_ORIENTATION orient=OO_INHERITED) // -1 for auto, enum OR_... for other
  {
    _gtb(orient);
    {
      _bs_unzip_t cu;
      _bs_unzip_coords(coords, &cu);
      _gtb_coords(cu);
    }
    {
      _bs_unzip_t cu;
      _bs_unzip_dimms(dimms, &cu);
      _gtb_dimms(cu);
    }
  }
public:
  int   add_movecs_pixing(COORDINATION con);
  int   add_movecs_rel(COORDINATION con);
public:
  void  math_pi();
public:
  void  param_alias(const char* name, int memslot=-1);
  int   param_push();
  void  param_get();
  void  param_mem(int memslot);
  void  param_pass();
  void  param_peek();
  
  void  param_for_arr_begin(const char* name, const char* arrlengthname="arrlen", const char *additname=nullptr);
  void  param_for_rarr_begin(const char* name);
  void  param_for_oarr_begin(const char* name_cur, const char* name_next="next", const char* arrlengthname="arrlen");
  void  param_for_end();
public:
  void  goto_normed();
  void  goto_normed(const char* someparam, int pixing, bool saveasoffset = false);
  void  goto_normed_empty();
  void  goto_normed_rotated(const char* angleRadName);
public:
  void  var_fixed(const char* name, float value);
  void  var_const_fixed(const char* name, float value);
  void  var_inline(const char* name, float v);
  void  var_fixed(const char* name, int value);
  void  var_const_fixed(const char* name, int value);
  void  var_inline(const char* name, int v);
  
  void  var_fixed(const char* name, float v1, float v2);
  void  var_const_fixed(const char* name, float v1, float v2);
  void  var_inline(const char* name, float v1, float v2);
  void  var_fixed(const char* name, int v1, int v2);
  void  var_const_fixed(const char* name, int v1, int v2);
  void  var_inline(const char* name, int v1, int v2);
  
  void  var_fixed(const char* name, float v1, float v2, float v3);
  void  var_const_fixed(const char* name, float v1, float v2, float v3);
  void  var_inline(const char* name, float v1, float v2, float v3);
  void  var_fixed(const char* name, int v1, int v2, int v3);
  void  var_const_fixed(const char* name, int v1, int v2, int v3);
  void  var_inline(const char* name, int v1, int v2, int v3);
  
  void  var_fixed(const char* name, float v1, float v2, float v3, float v4);
  void  var_const_fixed(const char* name, float v1, float v2, float v3, float v4);
  
  void  var_array_f_empty(const char* name, int size);
  void  var_array_ff_empty(const char* name, int size);
  void  var_array_fff_empty(const char* name, int size);
  
  void  var_array(const char* name, float v1);
  void  var_array(const char* name, float v1, float v2);
  void  var_array(const char* name, float v1, float v2, float v3);
  void  var_array(const char* name, float v1, float v2, float v3, float v4);
  void  var_array(const char* name, float v1, float v2, float v3, float v4, float v5);
  
public:
  void  var_static(DTYPE type, const char* name_eq_value);
  void  var_static(const char* name, const char* value);
  void  var_const_static(DTYPE type, const char* name_eq_value);
  
  void  movecs_pix_x(const char* name, int resc_idx);
  void  movecs_pix_y(const char* name, int resc_idx);
  void  movecs_pix(const char* name, int resc_idx);
  
  void  push_cs_rel_x(const char* name, int resc_idx);
  void  push_cs_rel_y(const char* name, int resc_idx);
  void  push_cs_rel(const char* name, int resc_idx);
  void  pop_cs_rel_x(const char* name);
  void  pop_cs_rel_y(const char* name);
  void  pop_cs_rel(const char* name);
public:
  void  push(const char* sztext);
  void  push(const char* text, unsigned int len);
public:
  void  inside_begin1(const char* limrad1);
  void  inside_begin2(const char* limits2);
  void  inside_end();

public:           /// PLACES  from inormed
//  void  place_rect_lb(const char* rdimms, const char* inside_name);
//  void  place_rect_cc(const char* r2dimms, const char* inside_name);
//  void  place_line(const char* ABC, const char* border, const char* inside_name);

public:
  void  ban_trace(bool);
public:           /// TRACES  from inormed
  void  trace_triangle_cc(const char *side, int direction, float fillcoeff=0.0f);   /// 0 - UP, 1 - DOWN
  
  
  void  trace_rect_xywh(const char* wh, float fillcoeff=0.0f, const char* crosslimit=nullptr);
  void  trace_rect_cc(const char* rdimms, float fillcoeff=0.0f, const char* crosslimit=nullptr);
  
  void  trace_square_lb_begin(const char* aside);
  void  trace_square_lb_end(float fillcoeff=0.0f);
  
  void  trace_square_cc_begin(const char* halfside);
  void  trace_square_cc_end(float fillcoeff=0.0f);

  void  trace_circle_cc_begin(const char* radius, const char* border);
  void  trace_circle_cc_end(float fillcoeff/*, bool notraceinside=false*/);
  
  void  trace_2linehorz_c(const char* isize = nullptr, const char* igap = nullptr, const char* ioffset = nullptr, const char* icrosslimit=nullptr);
  void  trace_2linevert_c(const char* size = nullptr, const char* gap = nullptr, const char* ioffset = nullptr, const char* crosslimit=nullptr);
  void  trace_linehorz_l(const char* isize = nullptr, const char* igap = nullptr, const char* ioffset = nullptr, const char* icrosslimit=nullptr);
  void  trace_linehorz_r(const char* isize = nullptr, const char* igap = nullptr, const char* ioffset = nullptr, const char* icrosslimit=nullptr);
  void  trace_linevert_t(const char* isize = nullptr, const char* igap = nullptr, const char* ioffset = nullptr, const char* icrosslimit=nullptr);
  void  trace_linevert_b(const char* isize = nullptr, const char* igap = nullptr, const char* ioffset = nullptr, const char* icrosslimit=nullptr);
  
  void  trace_lines_x(const char* size = nullptr, const char* igap = nullptr, const char* icrosslimit=nullptr);
public:
  void  trace_line_from_normed_to(const char* inormedendpoint);
//  void  trace_ray_trough_normed_from(const char* inormedstartpoint);
  void  trace_ray_trough(const char* somepoint, const char* size);
public:
  void  tex_pickcolor(int palette_param_idx, const char* pickvalue, const char* result="result");
  void  tex_addcolor(int palette_param_idx, const char* pickvalue, const char* weight, const char* result="result");
  void  tex_meshcolor(int palette_param_idx, const char* pickvalue, const char* mesh, const char* result="result");
public:
  void  goto_func_end(bool traced);
};

#endif // BSSHGENTRACE

