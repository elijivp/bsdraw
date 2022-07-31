/// Overlays:   invisible reactions for user events (mouse clicks, kbd keys). Use as root overlays
///   OActivePointer. Effect: invisible source of coords, dynamic, movable
///   OActiveCursor. Effect: mouse clicks reaction
///   OActiveCursorCarrier. Effect: mouse clicks reaction with attached client
///   OActiveCursorCarrier2. Effect: mouse clicks reaction with attached 2 clients
///   OActiveRandom. Effect: point moves after update() calls
/// Created By: Elijah Vlasov
#include "bsinteractive.h"
#include "../core/sheigen/bsshgentrace.h"


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

_OActiveBase::_OActiveBase(bool linkToScaledCenter): Ovldraw_ColorForegoing(0, false),
  OVLCoordsDynamic(CR_RELATIVE, 0.5, 0.5), OVLDimmsOff(), m_linked(linkToScaledCenter) {}

_OActiveBase::_OActiveBase(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter): Ovldraw_ColorForegoing(0, true),
  OVLCoordsDynamic(cn, default_x, default_y), OVLDimmsOff(), m_linked(linkToScaledCenter) {}

int   _OActiveBase::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
//  if (m_linked)
//    ocg.push("ioffset = ivec2(ioffset/ov_iscaler)*ov_iscaler + ov_iscaler/2;");
  if (m_linked)
    ocg.push(
              "ioffset = clamp(ioffset, ivec2(0,0), ov_ibounds-ivec2(1,1));"     /// autoclamp
              "ioffset = ivec2(ioffset/ov_iscaler)*ov_iscaler + ov_iscaler/2;"
             );
//  ocg.goto_normed();
  ocg.goto_func_end(false);
  return ocg.written();
}


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/


OActivePointer::OActivePointer(COORDINATION cn, float center_x, float center_y): Ovldraw_ColorForegoing(),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimmsOff(){}

int   OActivePointer::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  ocg.goto_normed();
  ocg.goto_func_end(false);
  return ocg.written();
}


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/


OActiveCursor::OActiveCursor(bool linkToScaledCenter): _OActiveBase(linkToScaledCenter){}
OActiveCursor::OActiveCursor(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter): _OActiveBase(cn, default_x, default_y, linkToScaledCenter){}
bool OActiveCursor::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (isVisible() == false)
      setVisible(true, false);
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    return true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    if (isVisible() == true)
    {
      setVisible(false);
      return true;
    }
  }
  return false;
}

OActiveCursorR::OActiveCursorR(bool linkToScaledCenter): _OActiveBase(linkToScaledCenter){}
OActiveCursorR::OActiveCursorR(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter): _OActiveBase(cn, default_x, default_y, linkToScaledCenter){}
bool OActiveCursorR::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == ORM_RMPRESS || oreact == ORM_RMMOVE)
  {
    if (isVisible() == false)
      setVisible(true, false);
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    return true;
  }
  else if (oreact == ORM_LMPRESS)
  {
    if (isVisible() == true)
    {
      setVisible(false);
      return true;
    }
  }
  return false;
}



OActiveCursorCarrierL::OActiveCursorCarrierL(IOverlayReactor* iop, bool linkToScaledCenter): OActiveCursor(linkToScaledCenter),
  m_iop(iop)
{
  m_iop->overlayReactionVisible(false);
}

OActiveCursorCarrierL::OActiveCursorCarrierL(IOverlayReactor* iop, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop)
{
  m_iop->overlayReactionVisible(true);
}


bool OActiveCursorCarrierL::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  return result;
}

OActiveCursorCarrierL2::OActiveCursorCarrierL2(IOverlayReactor* iop, IOverlayReactor* iop2, bool linkToScaledCenter): OActiveCursor(linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
  m_iop->overlayReactionVisible(false);
  m_iop2->overlayReactionVisible(false);
}

OActiveCursorCarrierL2::OActiveCursorCarrierL2(IOverlayReactor* iop, IOverlayReactor* iop2, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
  m_iop->overlayReactionVisible(true);
  m_iop2->overlayReactionVisible(true);
}

bool OActiveCursorCarrierL2::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
      m_iop2->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      m_iop2->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop2->overlayReactionMouse(oreact, ct, doStop);
  return result;
}


OActiveCursorCarrierL3::OActiveCursorCarrierL3(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, bool linkToScaledCenter):
  OActiveCursor(linkToScaledCenter),
  m_iop(iop), m_iop2(iop2), m_iop3(iop3)
{
  m_iop->overlayReactionVisible(false);
  m_iop2->overlayReactionVisible(false);
  m_iop3->overlayReactionVisible(false);
}

OActiveCursorCarrierL3::OActiveCursorCarrierL3(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, 
                                               COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop), m_iop2(iop2), m_iop3(iop3)
{
  m_iop->overlayReactionVisible(true);
  m_iop2->overlayReactionVisible(true);
  m_iop3->overlayReactionVisible(true);
}

bool OActiveCursorCarrierL3::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
      m_iop2->overlayReactionVisible(true);
      m_iop3->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      m_iop2->overlayReactionVisible(false);
      m_iop3->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop2->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop3->overlayReactionMouse(oreact, ct, doStop);
  return result;
}


OActiveCursorCarrierL4::OActiveCursorCarrierL4(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, 
                                               IOverlayReactor* iop4, bool linkToScaledCenter): 
  OActiveCursor(linkToScaledCenter),
  m_iop(iop), m_iop2(iop2), m_iop3(iop3), m_iop4(iop4)
{
  m_iop->overlayReactionVisible(false);
  m_iop2->overlayReactionVisible(false);
  m_iop3->overlayReactionVisible(false);
  m_iop4->overlayReactionVisible(false);
}

OActiveCursorCarrierL4::OActiveCursorCarrierL4(IOverlayReactor* iop, IOverlayReactor* iop2, IOverlayReactor* iop3, 
                                               IOverlayReactor* iop4, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop), m_iop2(iop2), m_iop3(iop3), m_iop4(iop4)
{
  m_iop->overlayReactionVisible(true);
  m_iop2->overlayReactionVisible(true);
  m_iop3->overlayReactionVisible(true);
  m_iop4->overlayReactionVisible(true);
}

bool OActiveCursorCarrierL4::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
      m_iop2->overlayReactionVisible(true);
      m_iop3->overlayReactionVisible(true);
      m_iop4->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      m_iop2->overlayReactionVisible(false);
      m_iop3->overlayReactionVisible(false);
      m_iop4->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop2->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop3->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop4->overlayReactionMouse(oreact, ct, doStop);
  return result;
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

OActiveCursorCarrierR::OActiveCursorCarrierR(IOverlayReactor* iop, bool linkToScaledCenter): OActiveCursorR(linkToScaledCenter),
  m_iop(iop)
{
  m_iop->overlayReactionVisible(false);
}

OActiveCursorCarrierR::OActiveCursorCarrierR(IOverlayReactor* iop, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursorR(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop)
{
  m_iop->overlayReactionVisible(true);
}


bool OActiveCursorCarrierR::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_RMPRESS || oreact == ORM_RMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_LMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  return result;
}

OActiveCursorCarrierR2::OActiveCursorCarrierR2(IOverlayReactor* iop, IOverlayReactor* iop2, bool linkToScaledCenter): OActiveCursorR(linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
  m_iop->overlayReactionVisible(false);
  m_iop2->overlayReactionVisible(false);
}

OActiveCursorCarrierR2::OActiveCursorCarrierR2(IOverlayReactor* iop, IOverlayReactor* iop2, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursorR(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
  m_iop->overlayReactionVisible(true);
  m_iop2->overlayReactionVisible(true);
}

bool OActiveCursorCarrierR2::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = false;
  if (oreact == ORM_RMPRESS || oreact == ORM_RMMOVE)
  {
    if (isVisible() == false)
    {
      setVisible(true, false);
      m_iop->overlayReactionVisible(true);
      m_iop2->overlayReactionVisible(true);
    }
    setCoordinates(ct->fx_ovl, ct->fy_ovl);
    result = true;
  }
  else if (oreact == ORM_LMPRESS)
  {
    if (isVisible())
    {
      setVisible(false);
      m_iop->overlayReactionVisible(false);
      m_iop2->overlayReactionVisible(false);
      result = true;
    }
  }
  result |= m_iop->overlayReactionMouse(oreact, ct, doStop);
  result |= m_iop2->overlayReactionMouse(oreact, ct, doStop);
  return result;
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

#include <time.h>

OActiveRandom::OActiveRandom(): Ovldraw_ColorForegoing(), OVLCoordsDynamic(CR_RELATIVE, 0.0, 0.0)
{
  m_ctr = 10;
  m_seed[0] = m_seed[1] = clock();
  appendUniform(DT_2F, m_seed);
}

void OActiveRandom::update()
{
  m_seed[0] = m_ctr;
  m_seed[1] = m_ctr/2;
  m_ctr++;
  updateParameter(false, true);
}

int   OActiveRandom::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  ocg.goto_normed();
  ocg.param_alias("myseed");
  ocg.push("float r1 = fract(sin(dot(myseed.xy, vec2(12.9898, 78.233)))*43758.5453123);");
  ocg.push("vec2 randomed = vec2(r1, fract(sin(dot(vec2(myseed.x, r1), vec2(12.9898, 78.233)))*43758.5453123));");
  ocg.push("ioffset = ivec2(randomed*ov_ibounds);");
  ocg.goto_func_end(false);
  return ocg.written();
}


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

//Interactive1DMaxMin::Interactive1DMaxMin(POINTERTYPE pt): Ovldraw_ColorForegoing(), m_pointerType(pt)
//{
//}

//Interactive1DMaxMin::~Interactive1DMaxMin()
//{
//}

//int   Interactive1DMaxMin::fshOVCoords(int overlay, bool switchedab, char* to) const
//{
//  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE | FshOVCoordsConstructor::OINC_BOUNDS);
//  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
//  {
//    {
//      if (m_pointerType == PT_MAX_H || m_pointerType == PT_MAX_V)   ocg.push("vec2 saved = vec2(0.0, bounds.x*contrast.x + contrast.y);");
//      else                                                          ocg.push("vec2 saved = vec2(0.0, bounds.y*contrast.x + contrast.y);");
//    }
//    ocg.push( "for (int i=0; i<countPortions; i++){"
//                "for (int h=0; h<viewdimm_a; h++){"
//                  "float fpos = float(h)/(viewdimm_a-1);"
//                  "vec2 current = vec2((fpos*(viewdimm_a-1) + 0.5)/viewdimm_a, getValue1D(i, fpos));" );
//    {
//      if (m_pointerType == PT_MAX_H || m_pointerType == PT_MAX_V)   ocg.push("float itis = step(saved[1], current[1]);");
//      else                                                          ocg.push("float itis = step(current[1], saved[1]);");
//    }
//    ocg.push(     "saved = mix(saved, current, itis);"
//                "}"
//              "}"
//             );
    
//    int pixing_pos = ocg.register_xyscaler_pixel(CR_RELATIVE);
//    ocg.goto_normed("saved.xy", pixing_pos, true);
    
////    int pixing_size = ocg.register_xyscaler_pixel(m_cn);
////    ocg.var_fixed("cs_gap", m_size_h != 0? m_gap_h : 0, m_size_v != 0? m_gap_v : 0);
////    ocg.var_fixed("cs_size", m_size_h != 0? m_size_h : 0, m_size_v != 0? m_size_v : 0);
////    ocg.xyscale_xy_pixel("cs_gap", pixing_size);
////    ocg.xyscale_xy_pixel("cs_size", pixing_size);
////    if (m_size_h != 0)
////      ocg.trace_2linehorz_c("cs_size[0]", "cs_gap[0]");
////    if (m_size_v != 0)
////      ocg.trace_2linevert_c("cs_size[1]", "cs_gap[1]");
//  }  
//  ocg.goto_func_end(false);
//  return ocg.written();
//}


OActiveCell::OActiveCell(int rows, int columns, const linestyle_t& linestyle, int margin): 
  Ovldraw_ColorTraced(linestyle), m_rows(rows), m_columns(columns), m_selrow(-1), m_selcolumn(-1), m_margin(margin)
{
  m_selfloat[0] = m_selfloat[1] = -1.0f;
  appendUniform(DT_2F, m_selfloat);
}

int         OActiveCell::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, 0);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  
  ocg.goto_normed();
  
  ocg.var_fixed("cr", m_columns, m_rows);
  ocg.var_fixed("mgn", m_margin);
  
  ocg.push( "vec2   cellsizepix = vec2(float(ov_ibounds.x) / cr.x, float(ov_ibounds.y) / cr.y);");
  
  ocg.param_alias("click");
  ocg.push( "ivec2  optiid = ivec2((click.x*ov_ibounds.x) / cellsizepix.x, (click.y*ov_ibounds.y) / cellsizepix.y);");
  
  ocg.push( "ioffset = ivec2(cellsizepix.x*optiid.x + mgn, cellsizepix.y*optiid.y + mgn);");
  ocg.push( "inormed = icoords - ioffset;");
  ocg.push( "ivec2 iwh = ivec2(cellsizepix.x - 2*mgn, cellsizepix.y - 2*mgn);");
  
  ocg.trace_rect_xywh("iwh", 0.0f);
  
  
  ocg.goto_func_end(true);
  return ocg.written();
}

bool        OActiveCell::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool up=false;
  if (oreact == ORM_LMPRESS)
  {
    *doStop = true;
    
    int r = int(ct->fy_rel * m_rows), c = int(ct->fx_rel * m_columns);
    
    if (r >= 0 && r < m_rows && c >= 0 && c < m_columns)
    {
      m_selrow = r;
      m_selcolumn = c;
      up = true;
      m_selfloat[0] = ct->fx_ovl;
      m_selfloat[1] = ct->fy_ovl;
    }
  }
  else if (oreact == ORM_RMPRESS)
  {
    up = m_selrow != -1 || m_selcolumn != -1;
    m_selrow = m_selcolumn = -1;
    
    m_selfloat[0] = -1;
    m_selfloat[1] = -1;
  }
  if (up)
    updateParameter(false, true);
  return up;
}

OActiveCellCarrier::OActiveCellCarrier(IOverlayReactor* iop, int rows, int columns, const linestyle_t& linestyle, int margin):
  OActiveCell(rows, columns, linestyle, margin), m_iop(iop)
{
}

bool OActiveCellCarrier::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  bool result = OActiveCell::overlayReactionMouse(oreact, ct, doStop);
  if (result)
  {
    (coordstriumv_t&)c_ctex = *ct;
    c_ctex.ex_r = m_selrow;
    c_ctex.ex_c = m_selcolumn;
    c_ctex.ex_count_rows = m_rows;
    c_ctex.ex_count_columns = m_columns;
    result |= m_iop->overlayReactionMouse(oreact, &c_ctex, doStop);
  }
  return result;
}


