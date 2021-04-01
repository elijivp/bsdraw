/// Overlays:   invisible reactions for user events (mouse clicks, kbd keys). Use as root overlays
///   OActivePoint. Effect: invisible source of coords, dynamic, movable
///   OActiveCursor. Effect: mouse clicks reaction
///   OActiveCursorCarrier. Effect: mouse clicks reaction with attached client
///   OActiveCursorCarrier2. Effect: mouse clicks reaction with attached 2 clients
///   OActiveRandom. Effect: point moves after update() calls
/// Created By: Elijah Vlasov
#include "bsinteractive.h"
#include "../core/sheigen/bsshgentrace.h"

OActivePoint::OActivePoint(COORDINATION cn, float center_x, float center_y): DrawOverlaySimple(),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimmsOff(){}

int   OActivePoint::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  ocg.goto_normed();
  ocg.goto_func_end(false);
  return ocg.written();
}




/********************************************************************************************************************************************/
/********************************************************************************************************************************************/
OActiveCursor::OActiveCursor(bool linkToScaledCenter): DrawOverlaySimple(),
  OVLCoordsDynamic(CR_RELATIVE, 0.5, 0.5), OVLDimmsOff(), m_linked(linkToScaledCenter) {}

OActiveCursor::OActiveCursor(COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter): DrawOverlaySimple(),
  OVLCoordsDynamic(cn, default_x, default_y), OVLDimmsOff(), m_linked(linkToScaledCenter) {}

int   OActiveCursor::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
//  if (m_linked)
//    ocg.push("ioffset = ivec2(ioffset/iscaling)*iscaling + iscaling/2;");
  if (m_linked)
    ocg.push(
              "ioffset = clamp(ioffset, ivec2(0,0), ibounds-ivec2(1,1));"     /// autoclamp
              "ioffset = ivec2(ioffset/iscaling)*iscaling + iscaling/2;"
             );
//  ocg.goto_normed();
  ocg.goto_func_end(false);
  return ocg.written();
}

bool OActiveCursor::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*)
{
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    setCoordinates(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
    return true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    setCoordinates(-1, -1);
    return true;
  }
  return false;
}


OActiveCursorCarrier::OActiveCursorCarrier(DrawOverlayProactive* iop, bool linkToScaledCenter): OActiveCursor(linkToScaledCenter),
  m_iop(iop)
{
}

OActiveCursorCarrier::OActiveCursorCarrier(DrawOverlayProactive* iop, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop)
{
}

bool OActiveCursorCarrier::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  m_iop->overlayReactionMouse(oreact, dataptr, doStop);
  
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    setCoordinates(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
    return true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    setCoordinates(-1, -1);
    return true;
  }
  return false;
}

OActiveCursorCarrier2::OActiveCursorCarrier2(DrawOverlayProactive* iop, DrawOverlayProactive* iop2, bool linkToScaledCenter): OActiveCursor(linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
}

OActiveCursorCarrier2::OActiveCursorCarrier2(DrawOverlayProactive* iop, DrawOverlayProactive* iop2, COORDINATION cn, float default_x, float default_y, bool linkToScaledCenter):
  OActiveCursor(cn, default_x, default_y, linkToScaledCenter),
  m_iop(iop), m_iop2(iop2)
{
  
}

bool OActiveCursorCarrier2::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  m_iop->overlayReactionMouse(oreact, dataptr, doStop);
  m_iop2->overlayReactionMouse(oreact, dataptr, doStop);
  
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    setCoordinates(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
    return true;
  }
  else if (oreact == ORM_RMPRESS)
  {
    setCoordinates(-1, -1);
    return true;
  }
  return false;
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

#include <time.h>

OActiveRandom::OActiveRandom(): DrawOverlaySimple(), OVLCoordsDynamic(CR_RELATIVE, 0.0, 0.0)
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
  updateParameter(false);
}

int   OActiveRandom::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  ocg.goto_normed();
  ocg.param_alias("myseed");
  ocg.push("float r1 = fract(sin(dot(myseed.xy, vec2(12.9898, 78.233)))*43758.5453123);");
  ocg.push("vec2 randomed = vec2(r1, fract(sin(dot(vec2(myseed.x, r1), vec2(12.9898, 78.233)))*43758.5453123));");
  ocg.push("ioffset = ivec2(randomed*ibounds);");
  ocg.goto_func_end(false);
  return ocg.written();
}


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

//Interactive1DMaxMin::Interactive1DMaxMin(POINTERTYPE pt): DrawOverlaySimple(), m_pointerType(pt)
//{
//}

//Interactive1DMaxMin::~Interactive1DMaxMin()
//{
//}

//int   Interactive1DMaxMin::fshTrace(int overlay, bool rotated, char* to) const
//{
//  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_GETVALUE | FshTraceGenerator::OINC_BOUNDS);
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
    
//    int pixing_pos = ocg.add_movecs_pixing(CR_RELATIVE);
//    ocg.goto_normed("saved.xy", pixing_pos, true);
    
////    int pixing_size = ocg.add_movecs_pixing(m_cn);
////    ocg.var_fixed("cs_gap", m_size_h != 0? m_gap_h : 0, m_size_v != 0? m_gap_v : 0);
////    ocg.var_fixed("cs_size", m_size_h != 0? m_size_h : 0, m_size_v != 0? m_size_v : 0);
////    ocg.movecs_pix("cs_gap", pixing_size);
////    ocg.movecs_pix("cs_size", pixing_size);
////    if (m_size_h != 0)
////      ocg.trace_2linehorz_c("cs_size[0]", "cs_gap[0]");
////    if (m_size_v != 0)
////      ocg.trace_2linevert_c("cs_size[1]", "cs_gap[1]");
//  }  
//  ocg.goto_func_end(false);
//  return ocg.written();
//}

