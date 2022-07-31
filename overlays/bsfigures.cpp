/// Overlays:   single simple figures
///   OFPoint: point,       OFCircle: circle,     OFSquare: square by leftbottom,     OFSquareCC: square by center 
///   OFTriangle: triangle,   OFLine: line with (x0,y0)(x1,y1) or horz/vert
///   OFArrow: line with arrow,   OFCross: square visir,  OFObjectif: like camera Objectif,   OFDouble: double lines 
/// Created By: Elijah Vlasov
#include "bsfigures.h"
#include "../core/sheigen/bsshgentrace.h"

OFPoint::OFPoint(COORDINATION cn, float center_x, float center_y, const linestyle_t& kls): Ovldraw_ColorForegoing(),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(CR_ABSOLUTE, 1)
{
  r = kls.r;  g = kls.g;  b = kls.b;
}

OFPoint::OFPoint(OVLCoordsStatic* pcoords, float offset_x, float offset_y, const linestyle_t& kls): Ovldraw_ColorForegoing(),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(CR_ABSOLUTE, 1)
{
  r = kls.r;  g = kls.g;  b = kls.b;
}

int   OFPoint::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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

OFCircle::OFCircle(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float radius, const linestyle_t &kls, float border): Ovldraw_ColorTraced(kls), 
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, radius), m_fillcoeff(fillopacity), m_border(border)
{
}

OFCircle::OFCircle(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float radius, const linestyle_t &kls, float border): Ovldraw_ColorTraced(kls), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination(): featcn, radius), m_fillcoeff(fillopacity), m_border(border)
{
}

int   OFCircle::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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


OFSquare::OFSquare(float fillopacity, COORDINATION cn, float leftbottom_x, float leftbottom_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(cn, leftbottom_x, leftbottom_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, aside), m_fillcoeff(fillopacity)
{
}

OFSquare::OFSquare(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, aside), m_fillcoeff(fillopacity)
{
}

int OFSquare::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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


OFSquareCC::OFSquareCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, aside), m_fillcoeff(fillopacity)
{
}

OFSquareCC::OFSquareCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, aside), m_fillcoeff(fillopacity)
{
}

int OFSquareCC::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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


///////////////////////////////////////
////////////////////////
/// 


OFRhombCC::OFRhombCC(float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, aside), m_fillcoeff(fillopacity)
{
}

OFRhombCC::OFRhombCC(float fillopacity, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float aside, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, aside), m_fillcoeff(fillopacity)
{
}

int OFRhombCC::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.trace_rhomb_cc("idimms1", m_fillcoeff);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}



/////////////////////////////////////////////
//////////////////////////
/// 

OFTriangle::OFTriangle(ORIENT orientation, float fillopacity, COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float side, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(cn, center_x, center_y), OVLDimms1Static(featcn == CR_SAME? cn : featcn, side), m_orientation(orientation), m_fillcoeff(fillopacity)
{
  
}

OFTriangle::OFTriangle(ORIENT orientation, float fillopacity, OVLCoordsStatic *pcoords, float offset_x, float offset_y, COORDINATION featcn, float side, const linestyle_t &kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(pcoords, offset_x, offset_y), OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, side), m_orientation(orientation), m_fillcoeff(fillopacity)
{
  
}

int OFTriangle::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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

OFLine::OFLine(LINETYPE linetype, COORDINATION cr, float start_x, float start_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cr, start_x, start_y),
  OVLDimmsOff(),
  m_lt(linetype), m_featcn(featcn), m_param1(gap), m_param2(size < 0 ? 5000.0f : size)
{
}

OFLine::OFLine(LINETYPE linetype, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_lt(linetype), m_featcn(featcn), m_param1(gap), m_param2(size < 0 ? 5000.0f : size)
{
}

OFLine::OFLine(COORDINATION cn, float start_x, float start_y, float end_x, float end_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, start_x, start_y),
  OVLDimmsOff(),
  m_lt(-1), m_param1(end_x), m_param2(end_y)
{
}

OFLine::OFLine(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float end_x, float end_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_lt(-1), m_param1(end_x), m_param2(end_y)
{
}

int   OFLine::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
    ocg.goto_normed();
    if (m_lt == -1)
    {
      ocg.var_fixed("endpoint", m_param1, m_param2);
      ocg.xyscale_xy_pixel("endpoint", 0);
      ocg.push("endpoint = endpoint - ioffset;");
      ocg.trace_line_from_normed_to("endpoint");
    }
    else
    {
      ocg.var_fixed("cs_gap", (float)m_param1);
      ocg.var_fixed("cs_size", (float)m_param2);
      if (m_featcn != CR_SAME)
      {
        int fc = ocg.register_xyscaler_pixel(m_featcn);
        if (m_lt == LT_HORZ_SYMMETRIC || m_lt == LT_HORZ_BYLEFT || m_lt == LT_CROSS)
        {
          ocg.xyscale_x_pixel("cs_gap", fc);
          ocg.xyscale_x_pixel("cs_size", fc);
        }
        else
        {
          ocg.xyscale_y_pixel("cs_gap", fc);
          ocg.xyscale_y_pixel("cs_size", fc);
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

OFRay::OFRay(COORDINATION cn, float start_x, float start_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, 0.0f, 0.0f),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(CR_SAME), m_length(0)
{
}

OFRay::OFRay(COORDINATION cn, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, 0.0f, 0.0f),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(additcn), m_length(length)
{
  
}

OFRay::OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(),
  m_start_x(start_x), m_start_y(start_y), m_additcn(CR_SAME), m_length(0)
{
}

OFRay::OFRay(OVLCoordsStatic* pcoords, float offset_x, float offset_y, float start_x, float start_y, COORDINATION additcn, float length, const linestyle_t& linestyle)
  : Ovldraw_ColorTraced(linestyle), 
    OVLCoordsDynamic(pcoords, offset_x, offset_y),
    OVLDimmsOff(),
    m_start_x(start_x), m_start_y(start_y), m_additcn(additcn), m_length(length)
{
  
}

int   OFRay::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
//    ocg.goto_normed();
//    ocg.var_fixed("endpoint", m_start_x, m_start_y);
//    ocg.xyscale_xy_pixel("endpoint", 0);
//    ocg.push("endpoint = endpoint - ioffset;");
//    ocg.trace_ray_trough_normed_from("endpoint");
    ocg.var_fixed("center", m_start_x, m_start_y);
    ocg.goto_normed("center", 0);
    
    const char* limit = nullptr;
    if (m_length != 0.0f)
    {
      int relcn = ocg.register_xyscaler_pixel(m_additcn == CR_SAME? this->getCoordination() : m_additcn);
      ocg.var_fixed("limit", m_length);
      ocg.xyscale_x_pixel("limit", relcn);
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



OFArrow::OFArrow(COORDINATION cn, float arrow_x, float arrow_y, float end_x, float end_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, arrow_x, arrow_y),
  OVLDimmsOff(),
  m_at(-1), m_param1(end_x), m_param2(end_y)
{
}

OFArrow::OFArrow(OVLCoordsStatic* pcoords, float offset_arrow_x, float offset_arrow_y, float end_x, float end_y, const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_arrow_x, offset_arrow_y),
  OVLDimmsOff(),
  m_at(-1), m_param1(end_x), m_param2(end_y)
{
}

int   OFArrow::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  
  ocg.var_fixed("endpoint", m_param1, m_param2);
  int arrow_pixing = ocg.register_xyscaler_pixel(CR_RELATIVE_NOSCALED);
  ocg.push("float angle; {");
  {
    ocg.goto_normed();
    ocg.var_static(DT_2F, "norm_end = endpoint");
    ocg.xyscale_xy_pixel("norm_end", 0);
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
      ocg.xyscale_xy_pixel("arrow", arrow_pixing);
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

OFCross::OFCross(COORDINATION cn, float start_x, float start_y, COORDINATION featcn, float gap, float size, const linestyle_t &linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, start_x, start_y),
  OVLDimms1Static(featcn == CR_SAME? cn : featcn, size < 0 ? 5000.0f : size),
  m_gap(gap)
{
}

OFCross::OFCross(OVLCoordsStatic *pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t &linestyle): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms1Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size < 0 ? 5000.0f : size),
  m_gap(gap)
{
}

int OFCross::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("cs_gap", m_gap);
    ocg.xyscale_x_pixel("cs_gap", 1);
    ocg.trace_lines_x("idimms1", "cs_gap");
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/////////////////////////////////////////////
//////////////////////////
/// 

OFAngle::OFAngle(OFAngle::ORIENT orient, COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size_x, size_y), 
  m_orient(orient)
{
}

OFAngle::OFAngle(OFAngle::ORIENT orient, OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float size_x, float size_y, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size_x, size_y), 
  m_orient(orient)
{
}

int   OFAngle::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
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
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size, size),
  m_gap(gap)
{
}

OFVisir::OFVisir(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size, size),
  m_gap(gap)
{
}

int   OFVisir::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap, m_gap);
    ocg.xyscale_xy_pixel("gap", 1);

    ocg.push("vec2 nosig = vec2(-1.0 + 2.0*step(0.0, float(inormed.x)), -1.0 + 2.0*step(0.0, float(inormed.y)));");
    
    ocg.push("ivec2 isaved = inormed;");
//    ocg.push("inormed.x = inormed.x + mix(gap, -gap, step(0.0, float(inormed.x)));");
    ocg.push("inormed.x = int(inormed.x - gap.x*nosig.x);");
    ocg.trace_2linevert_c("float(idimms2.x)", "gap.x");
    ocg.push("inormed = isaved;");
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
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, size, size),
  m_gap(gap)
{
}

OFFactor::OFFactor(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap, float size, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, size, size),
  m_gap(gap)
{
}

int   OFFactor::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap, m_gap);
    ocg.xyscale_xy_pixel("gap", 1);
    
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

OFPointrun::OFPointrun(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float gap_w, int speed): 
  Ovldraw_ColorForegoing(1), OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimmsOff(), m_cr(featcn), m_gap(gap_w), m_speed(speed)
{
}

OFPointrun::OFPointrun(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float gap_w, int speed): 
  Ovldraw_ColorForegoing(1), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimmsOff(), m_cr(featcn), m_gap(gap_w), m_speed(speed)
{
}

int   OFPointrun::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    int amp = ocg.register_xyscaler_pixel(m_cr);
    ocg.var_fixed("gap", m_gap);
    ocg.xyscale_x_pixel("gap", amp);
    
    
    if (m_speed <= 0)
    {
      ocg.var_fixed("speed", -m_speed);
      ocg.push("int offsx = int(step(gap, abs(inormed.x)))*((inormed.x >> speed) << speed);");
    }
    else
    {
      ocg.var_fixed("speed", m_speed);
      ocg.push("int n = sign(inormed.x);");
      ocg.push("int x = n*inormed.x;");
      ocg.push("int s = 2 + speed;");
      ocg.push("while (x != 0){  x = x >> 1;   s = s + 1; }");
//      ocg.push("int offsx = 1 + ((n*inormed.x << (2 + speed)) & ( ((1 << speed)-1) << (s - speed))) / ((1 << (2 + speed)) - 1);");
      ocg.push("int offsx = ((n*inormed.x << (2 + speed)) & ( ((1 << speed)-1) << (s - speed))) / ((1 << (2 + speed)));");
      ocg.push("offsx = offsx*n * int(step(gap, float(offsx)));");
    }
    ocg.push("result = vec3(1.0, 1.0, 1.0);");
    ocg.push("mixwell = step(float(inormed.x-offsx),0.0)*step(float(offsx-inormed.x),0.0)*step(float(inormed.y), 0.0)*step(0.0, float(inormed.y));");
  }  
  ocg.goto_func_end(false);
  return ocg.written();
}

/////////////////////////////////////////////
//////////////////////////
/// 


OFSubjectif::OFSubjectif(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float width, float gap_w, float gap_h, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle),  OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, width*2 + gap_w, gap_h*2)
{
  m_gap = gap_w;
}

OFSubjectif::OFSubjectif(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float width, float gap_w, float gap_h, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, width*2 + gap_w, gap_h*2)
{
  m_gap = gap_w;
}

int   OFSubjectif::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap);
    ocg.xyscale_x_pixel("gap", 1);
    
//    ocg.push("ivec2 ofs = ivec2(sign(inormed.x)*idimms2.x, sign(inormed.y)*idimms2.y);");   // symmetric cross in center
    
    ocg.trace_2linehorz_c("idimms2.x", "gap", nullptr);
//    ocg.push("int ofs = int(mix(idimms2.y, -idimms2.y, step(float(inormed.x), 0)));");
//    ocg.trace_2linehorz_c("idimms2.x", nullptr, "ofs");
    ocg.push("inormed.y = inormed.y - int(mix(idimms2.y, -idimms2.y, step(float(inormed.y), 0)));");
    ocg.trace_2linehorz_c("idimms2.x - gap", nullptr, nullptr);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/////////////////////////////////////////////
//////////////////////////
/// 


OFObjectif::OFObjectif(COORDINATION cr, float center_x, float center_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle),  OVLCoordsDynamic(cr, center_x, center_y),
  OVLDimms2Static(featcn == CR_SAME? cr : featcn, width, height)
{
  m_gap[0] = gap_w;
  m_gap[1] = gap_h;
}

OFObjectif::OFObjectif(OVLCoordsStatic* pcoords, float offset_x, float offset_y, COORDINATION featcn, float width, float height, float gap_w, float gap_h, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsDynamic(pcoords, offset_x, offset_y),
  OVLDimms2Static(featcn == CR_SAME? pcoords->getCoordination() : featcn, width, height)
{
  m_gap[0] = gap_w;
  m_gap[1] = gap_h;
}

int   OFObjectif::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("gap", m_gap[0], m_gap[1]);
    ocg.xyscale_xy_pixel("gap", 1);
    
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



OFDouble::OFDouble(bool horz, COORDINATION cn, float center, COORDINATION featcn, float gap, const linestyle_t& kls): Ovldraw_ColorTraced(kls), 
  OVLCoordsDynamic(cn, horz? 0.0f : center, horz? center: 0.0f),
  OVLDimms2Dynamic(featcn == CR_SAME? cn : featcn, horz? 0.0f : gap, horz? gap : 0.0f),
  m_horz(horz), m_gap(gap)
{
}

OFDouble::OFDouble(bool horz, OVLCoordsStatic* pcoords, float center, COORDINATION featcn, float gap, const linestyle_t& kls): Ovldraw_ColorTraced(kls),
  OVLCoordsDynamic(pcoords, horz? 0.0f : center, horz? center: 0.0f),
  OVLDimms2Dynamic(featcn == CR_SAME? pcoords->getCoordination() : featcn, horz? gap : 0.0f, horz? 0.0f : gap),
  m_horz(horz), m_gap(gap)
{
}

int OFDouble::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    {
      int relstep = ocg.register_xyscaler_01(coords_type_t::getCoordination());
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
