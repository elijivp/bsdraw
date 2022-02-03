/// Overlays:   single simple figures
///   OFPoint: point,       OFCircle: circle,     OFSquare: square by leftbottom,     OFSquareCC: square by center 
///   OFTriangle: triangle,   OFLine: line with (x0,y0)(x1,y1) or horz/vert
///   OFArrow: line with arrow,   OFCross: square visir,  OFObjectif: like camera Objectif,   OFDouble: double lines 
/// Created By: Elijah Vlasov
#include "bsfigures.h"
#include "../core/sheigen/bsshgentrace.h"

OFPoint::OFPoint(COORDINATION cn, float center_x, float center_y, const linestyle_t& kls): DrawOverlaySimple(),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(CR_ABSOLUTE, 1)
{
  r = kls.r;  g = kls.g;  b = kls.b;
}

OFPoint::OFPoint(OVLCoordsStatic* pcoords, float offset_x, float offset_y, const linestyle_t& kls): DrawOverlaySimple(),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(CR_ABSOLUTE, 1)
{
  r = kls.r;  g = kls.g;  b = kls.b;
}

int   OFPoint::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_const_fixed("pointcolor", r, g, b);
    ocg.push("result = pointcolor;");
    ocg.push("mixwell = step(length(vec2(inormed)), 0.0);");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

///////////////////////////////////////
////////////////////////
/// 

OFCircle::OFCircle(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float radius, const linestyle_t &kls, float border): DrawOverlayTraced(kls), 
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, radius), m_fillcoeff(fillopacity), m_border(border)
{
}

OFCircle::OFCircle(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float radius, const linestyle_t &kls, float border): DrawOverlayTraced(kls), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination(): featcn, radius), m_fillcoeff(fillopacity), m_border(border)
{
}

int   OFCircle::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("b", m_border);
//    ocg.var_const_static(DT_1F, "b = 1.0");
    ocg.trace_circle_cc_begin("idimms1", "b");
    ocg.trace_circle_cc_end(m_fillcoeff);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

//float OFCircle::radius() const
//{
//  return m_side;
//}

//void  OFCircle::updateRadius(float radius)
//{
//  OVLDimms1Static::Dimms(radius,radius,radius,radius);
//  updateParameter(false, update);
//}


///////////////////////////////////////
////////////////////////
/// 


OFSquare::OFSquare(float fillopacity, COORDINATION cn, float leftbottom_x, float leftbottom_y, COORDINATION featcn, float aside, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(cn, leftbottom_x, leftbottom_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, aside), m_fillcoeff(fillopacity)
{
}

OFSquare::OFSquare(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, aside), m_fillcoeff(fillopacity)
{
}

int OFSquare::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.trace_square_lb_begin("idimms1");
    ocg.trace_square_lb_end(m_fillcoeff);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

//float OFSquare::getSide() const
//{
//  return m_side;
//}

//void  OFSquare::updateSide(float aside)
//{
//  OVLDimms1Static::laterInitDimms(aside,aside,aside,aside);
//  updateParameter(false, update);
//}


///////////////////////////////////////
////////////////////////
/// 


OFSquareCC::OFSquareCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float aside, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, aside), m_fillcoeff(fillopacity)
{
}

OFSquareCC::OFSquareCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, aside), m_fillcoeff(fillopacity)
{
}

int OFSquareCC::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.trace_square_cc_begin("idimms1");
    ocg.trace_square_cc_end(m_fillcoeff);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

//float OFSquareCC::halfside() const
//{
//  return m_side;
//}

//void  OFSquareCC::updateHalfside(float aside)
//{
//  OVLDimms1Static::laterInitDimms(aside,aside,aside,aside);
//  updateParameter(false, update);
//}

/////////////////////////////////////////////
//////////////////////////
/// 

OFTriangle::OFTriangle(ORIENT orientation, float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float side, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, side), m_orientation(orientation), m_fillcoeff(fillopacity)
{
  
}

OFTriangle::OFTriangle(ORIENT orientation, float fillopacity, OVLCoordsStatic *pcoords, float offset_x, float offset_y, COORDINATION featcn, float side, const linestyle_t &kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, side), m_orientation(orientation), m_fillcoeff(fillopacity)
{
  
}

int OFTriangle::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.trace_triangle_cc("idimms1", m_orientation == ORIENT_UP? 0: m_orientation == ORIENT_DOWN? 1 : 0, m_fillcoeff);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

//float OFTriangle::fside() const
//{
//  return m_side;
//}

//void OFTriangle::updateSide(float side)
//{
//  OVLDimms1Static::laterInitDimms(side,side,side,side);
//  updateParameter(false, update);
//}



/////////////////////////////////////////////
//////////////////////////
/// 

OFLine::OFLine(LINETYPE linetype, COORDINATION cr, float start_x, float start_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cr, start_x, start_y),
  OVLDimmsOff(),
  m_lt(linetype), m_featcn(featcn), m_param1(gap), m_param2(size < 0 ? 5000.0f : size)
{
}

OFLine::OFLine(LINETYPE linetype, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_lt(linetype), m_featcn(featcn), m_param1(gap), m_param2(size < 0 ? 5000.0f : size)
{
}

OFLine::OFLine(COORDINATION cn, float start_x, float start_y, float end_x, float end_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, start_x, start_y),
  OVLDimmsOff(),
  m_lt(-1), m_param1(end_x), m_param2(end_y)
{
}

OFLine::OFLine(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float end_x, float end_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_lt(-1), m_param1(end_x), m_param2(end_y)
{
}

int   OFLine::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
    ocg.goto_normed();
    if (m_lt == -1)
    {
      ocg.var_fixed("endpoint", m_param1, m_param2);
      ocg.movecs_pix("endpoint", 0);
      ocg.push("endpoint = endpoint - ioffset;");
      ocg.trace_line_from_normed_to("endpoint");
    }
    else
    {
      ocg.var_fixed("cs_gap", (float)m_param1);
      ocg.var_fixed("cs_size", (float)m_param2);
      if (m_featcn != CR_SAME)
      {
        int fc = ocg.add_movecs_pixing(m_featcn);
        if (m_lt == LT_HORZ_SYMMETRIC || m_lt == LT_HORZ_BYLEFT || m_lt == LT_CROSS)
        {
          ocg.movecs_pix_x("cs_gap", fc);
          ocg.movecs_pix_x("cs_size", fc);
        }
        else
        {
          ocg.movecs_pix_y("cs_gap", fc);
          ocg.movecs_pix_y("cs_size", fc);
        }
      }
//      ocg.var_fixed("cs_size2", (float)(m_param2 - m_param1));
  
      if (m_lt == LT_HORZ_SYMMETRIC)      ocg.trace_2linehorz_c("cs_size", "cs_gap");
      else if (m_lt == LT_HORZ_BYLEFT)    ocg.trace_linehorz_l("cs_size", "cs_gap");
      else if (m_lt == LT_VERT_SYMMETRIC) ocg.trace_2linevert_c("cs_size", "cs_gap");
      else if (m_lt == LT_VERT_BYBOTTOM)  ocg.trace_linevert_b("cs_size", "cs_gap");
      else if (m_lt == LT_CROSS){ ocg.trace_2linehorz_c("cs_size", "cs_gap"); ocg.trace_2linevert_c("cs_size", "cs_gap"); }
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 

OFRay::OFRay(COORDINATION cn, float start_x, float start_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, 0.0f, 0.0f),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(CR_SAME), m_length(0)
{
}

OFRay::OFRay(COORDINATION cn, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, 0.0f, 0.0f),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(additcn), m_length(length)
{
  
}

OFRay::OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(CR_SAME), m_length(0)
{
}

OFRay::OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle)
  : DrawOverlayTraced(linestyle), 
    OVLCoordsDynamic(pcoords, offset_x, offset_y),
    OVLDimmsOff(),
    m_start_x(start_x), m_start_y(start_y), m_additcn(additcn), m_length(length)
{
  
}

int   OFRay::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
//    ocg.goto_normed();
//    ocg.var_fixed("endpoint", m_start_x, m_start_y);
//    ocg.movecs_pix("endpoint", 0);
//    ocg.push("endpoint = endpoint - ioffset;");
//    ocg.trace_ray_trough_normed_from("endpoint");
    ocg.var_fixed("center", m_start_x, m_start_y);
    ocg.goto_normed("center", 0);
    
    const char* limit = nullptr;
    if (m_length != 0.0f)
    {
      int relcn = ocg.add_movecs_pixing(m_additcn == CR_SAME? this->getCoordination() : m_additcn);
      ocg.var_fixed("limit", m_length);
      ocg.movecs_pix_x("limit", relcn);
      limit = "limit";
    }
    ocg.trace_ray_trough("icoords - ioffset - inormed;", limit);
  }
  ocg.goto_func_end(true);
  return ocg.written();
}
/////////////////////////////////////////////////////////
//////////////////////////////////////
///



OFArrow::OFArrow(COORDINATION cn, float arrow_x, float arrow_y, float end_x, float end_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, arrow_x, arrow_y),
  OVLDimmsOff(),
  m_at(-1), m_param1(end_x), m_param2(end_y)
{
}

OFArrow::OFArrow(OVLCoordsStatic* pcoords, float offset_arrow_x, float offset_arrow_y, float end_x, float end_y, const linestyle_t& linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_arrow_x, offset_arrow_y),
  OVLDimmsOff(),
  m_at(-1), m_param1(end_x), m_param2(end_y)
{
}

int   OFArrow::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  
  ocg.var_fixed("endpoint", m_param1, m_param2);
  int arrow_pixing = ocg.add_movecs_pixing(CR_RELATIVE_NOSCALED);
  ocg.push("float angle; {");
  {
    ocg.goto_normed();
    ocg.var_static(DT_2F, "norm_end = endpoint");
    ocg.movecs_pix("norm_end", 0);
    ocg.push("norm_end = norm_end - ioffset;");
//    ocg.push("angle = atan2(norm_end.x, norm_end.y);");
    ocg.push("angle = atan(norm_end.x, norm_end.y);");
    ocg.trace_line_from_normed_to("norm_end");
  }
  ocg.ban_trace(true);
  {
    ocg.push("vec2 arrow; float newangle;");
    for (int i=0; i<2; i++)
    {
      if (i == 0) ocg.push("newangle = angle - 0.25;");
      else ocg.push("newangle = angle + 0.25;");
      ocg.push("arrow = 1.0*vec2(sin(newangle), cos(newangle));");
      ocg.movecs_pix("arrow", arrow_pixing);
      ocg.trace_line_from_normed_to("arrow");
    }
  }
  ocg.ban_trace(false);
  ocg.push("}");
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 

OFCross::OFCross(COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float gap, float size, const linestyle_t &linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, start_x, start_y),
  OVLDimms1Static(featcn == CR_SAME? cn : featcn, size < 0 ? 5000.0f : size),
  m_gap(gap)
{
}

OFCross::OFCross(OVLCoordsStatic *pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t &linestyle): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size < 0 ? 5000.0f : size),
  m_gap(gap)
{
}

int OFCross::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("cs_gap", m_gap);
    ocg.movecs_pix_x("cs_gap", 1);
    ocg.trace_lines_x("idimms1", "cs_gap");
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 

OFAngle::OFAngle(OFAngle::ORIENT orient, COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size_x, size_y), 
  m_orient(orient)
{
}

OFAngle::OFAngle(OFAngle::ORIENT orient, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size_x, size_y), 
  m_orient(orient)
{
}

int   OFAngle::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    
    if (m_orient == OR_LT || m_orient == OR_LB)
      ocg.trace_linehorz_l("float(idimms2.x)");
    else if (m_orient == OR_RT || m_orient == OR_RB)
      ocg.trace_linehorz_r("float(idimms2.x)");
    
    if (m_orient == OR_LT || m_orient == OR_RT)
      ocg.trace_linevert_t("float(idimms2.y)");
    else if (m_orient == OR_LB || m_orient == OR_RB)
      ocg.trace_linevert_b("float(idimms2.y)");
  }      
  ocg.goto_func_end(true);
  return ocg.written();
}



/////////////////////////////////////////////
//////////////////////////
/// 

OFVisir::OFVisir(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size, size),
  m_gap(gap)
{
}

OFVisir::OFVisir(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size, size),
  m_gap(gap)
{
}

int   OFVisir::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap, m_gap);
    ocg.movecs_pix("gap", 1);

    ocg.push("vec2 nosig = vec2(-1.0 + 2.0*step(0.0, float(inormed.x)), -1.0 + 2.0*step(0.0, float(inormed.y)));");
    
    ocg.push("ivec2 isaved = inormed;");
//    ocg.push("inormed.x = inormed.x + mix(gap, -gap, step(0.0, float(inormed.x)));");
    ocg.push("inormed.x = int(inormed.x - gap.x*nosig.x);");
    ocg.trace_2linevert_c("float(idimms2.x)", "gap.x");
    ocg.push("inormed = isaved;");
//    ocg.push("inormed.y = inormed.y + mix(gap, -gap, step(0.0, inormed.y));");
    ocg.push("inormed.y = int(inormed.y - gap.y*nosig.y);");
    ocg.trace_2linehorz_c("float(idimms2.y)", "gap.y");
    
//    ocg.var_static(DT_2F, "cs_sizegapped = idimms2 - gap");
//    ocg.trace_2linehorz_c("cs_sizegapped.x", "gap.x");
//    ocg.trace_2linevert_c("cs_sizegapped.y", "gap.y");
//    ocg.trace_rect_cc("idimms2", 0.0f);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}



/////////////////////////////////////////////
//////////////////////////
/// 

OFFactor::OFFactor(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size, size),
  m_gap(gap)
{
}

OFFactor::OFFactor(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size, size),
  m_gap(gap)
{
}

int   OFFactor::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap, m_gap);
    ocg.movecs_pix("gap", 1);
    
    ocg.var_static(DT_2F, "cs_sizegapped = idimms2 - gap");
    ocg.trace_2linehorz_c("cs_sizegapped.x", "gap.x");
    ocg.trace_2linevert_c("cs_sizegapped.y", "gap.y");
    ocg.trace_rect_cc("idimms2", 0.0f);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 


OFObjectif::OFObjectif(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle),  OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, width, height)
{
  m_gap[0] = gap_w;
  m_gap[1] = gap_h;
}

OFObjectif::OFObjectif(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, width, height)
{
  m_gap[0] = gap_w;
  m_gap[1] = gap_h;
}

int   OFObjectif::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap[0], m_gap[1]);
    ocg.movecs_pix("gap", 1);
    
//    ocg.push("ivec2 ofs = ivec2(sign(inormed.x)*idimms2.x, sign(inormed.y)*idimms2.y);");   // symmetric cross in center
    ocg.push("ivec2 ofs = ivec2(mix(idimms2.x, -idimms2.x, step(float(inormed.x), 0)), mix(idimms2.y, -idimms2.y, step(float(inormed.y), 0)));");
    ocg.trace_2linevert_c("idimms2.y - gap.y", "gap.y", "ofs[0]");
    ocg.trace_2linehorz_c("idimms2.x - gap.x", "gap.x", "ofs[1]");
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/////////////////////////////////////////////
//////////////////////////
/// 



OFDouble::OFDouble(bool horz, COORDINATION cn, float center, COORDINATION featcn, float gap, const linestyle_t& kls): DrawOverlayTraced(kls), 
  OVLCoordsDynamic(cn, horz? 0.0f : center, horz? center: 0.0f),
  OVLDimms2Dynamic(featcn == CR_SAME? cn : featcn, horz? 0.0f : gap, horz? gap : 0.0f),
  m_horz(horz), m_gap(gap)
{
}

OFDouble::OFDouble(bool horz, OVLCoordsStatic* pcoords, float center, COORDINATION featcn, float gap, const linestyle_t& kls): DrawOverlayTraced(kls),
  OVLCoordsDynamic(pcoords, horz? 0.0f : center, horz? center: 0.0f),
  OVLDimms2Dynamic(featcn == CR_SAME? pcoords->getCoordination() : featcn, horz? gap : 0.0f, horz? 0.0f : gap),
  m_horz(horz), m_gap(gap)
{
}

int OFDouble::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    {
      int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
      if (m_horz) ocg.push("int double_step = int(mix(idimms2[1], -idimms2[1], step(float(abs(inormed.y + idimms2[1])), float(abs(inormed.y - idimms2[1])))));");
      else        ocg.push("int double_step = int(mix(idimms2[0], -idimms2[0], step(float(abs(inormed.x + idimms2[0])), float(abs(inormed.x - idimms2[0])))));");
      if (m_horz){  ocg.trace_2linehorz_c(nullptr, nullptr, "double_step"); }
      else {        ocg.trace_2linevert_c(nullptr, nullptr, "double_step"); }
    }
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 
