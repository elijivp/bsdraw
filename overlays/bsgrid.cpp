/// Overlays:   grids
///   OGridRegular. View: classic regular grid
///   OGridCircular. View: circular
///   OGridDecart. View: classic decart
/// Created By: Elijah Vlasov
#include "bsgrid.h"
#include "../core/sheigen/bsshgentrace.h"

enum  EXTENDED_REGULAR { 
  EGO_REGULAR_H, EGO_REGULAR_V, EGO_REGULAR_B, 
  EGO_RISK_H, EGO_RISK_V, EGO_RISK_B
};

OGridRegular::OGridRegular(REGULAR go, COORDINATION cn, float startchannel, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showBorderGrids): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, go == REGULAR_HORZ? 0.0f : startchannel, go == REGULAR_HORZ? startchannel : 0.0f),
  OVLDimmsOff(),
  m_gridtype((int)go), m_stepsize(stepsize), m_maxsteps(maxsteps), m_bordergrids(showBorderGrids), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(REGULAR go, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showBorderGrids): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, go == REGULAR_HORZ? 0.0f : startchannel, go == REGULAR_HORZ? startchannel : 0.0f),
  OVLDimmsOff(),
  m_gridtype((int)go), m_stepsize(stepsize), m_maxsteps(maxsteps), m_bordergrids(showBorderGrids), m_additcn(cnstep)
{
}

OGridRegular::OGridRegular(float start, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showBorderGrids): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(CR_RELATIVE, 0.0f, start),
  OVLDimmsOff(),
  m_gridtype(EGO_REGULAR_B), m_stepsize(stepsize), m_maxsteps(maxsteps), m_bordergrids(showBorderGrids), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(RISK gr, COORDINATION cn, float startchannel, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps): Ovldraw_ColorTraced(linestyle),
  OVLCoordsDynamic(cn, gr == RISK_HORZ? mark_centeroffset : startchannel, gr == RISK_HORZ? startchannel : mark_centeroffset),
  OVLDimmsOff(),
  m_gridtype((int)gr + EGO_RISK_H), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(OGridRegular::RISK gr, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps)
  : Ovldraw_ColorTraced(linestyle),
    OVLCoordsDynamic(cn, gr == RISK_HORZ? mark_centeroffset : startchannel, gr == RISK_HORZ? startchannel : mark_centeroffset),
    OVLDimmsOff(),
    m_gridtype((int)gr + EGO_RISK_H), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(cnstep)
{
}

OGridRegular::OGridRegular(float start, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps)
  : Ovldraw_ColorTraced(linestyle),
    OVLCoordsDynamic(CR_RELATIVE, mark_centeroffset, start),
    OVLDimmsOff(),
    m_gridtype(EGO_RISK_B), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(CR_SAME)
{
}

int OGridRegular::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, m_gridtype == EGO_REGULAR_B || m_gridtype == EGO_RISK_B? 
                           FshOVCoordsConstructor::OINC_DATABOUNDS : 0);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    
    int step_xys_px = ocg.register_xyscaler_pixel(CR_RELATIVE);
    
    if (m_gridtype == EGO_REGULAR_B || m_gridtype == EGO_RISK_B)
    {
      if (switchedab)
        ocg.goto_normed("vec2((ioffset[1]-databounds[0])/(databounds[1]-databounds[0]), 0.0)", step_xys_px);
      else
        ocg.goto_normed("vec2(0.0, (ioffset[1]-databounds[0])/(databounds[1]-databounds[0]))", step_xys_px);
    }
    else
      ocg.goto_normed();
    
    bool ishorz_regular = m_gridtype == EGO_REGULAR_H || (!switchedab && m_gridtype == EGO_REGULAR_B);
    bool ishorz_risk = m_gridtype == EGO_RISK_H || (!switchedab && m_gridtype == EGO_RISK_B);
    bool isvert_regular = m_gridtype == EGO_REGULAR_V || (switchedab && m_gridtype == EGO_REGULAR_B);
    bool isvert_risk = m_gridtype == EGO_RISK_V || (switchedab && m_gridtype == EGO_RISK_B);
    
    int risk_pixing_height = 0;
    if (ishorz_risk || isvert_risk)
    {
      if (m_babsheight)
      {
        int sh = (int)(m_specheight/2) + 1;
        ocg.var_fixed("grid_height", sh == 0? 1.0f : float(sh));
        risk_pixing_height = ocg.register_xyscaler_pixel(CR_ABSOLUTE_NOSCALED);
      }
      else
        ocg.var_fixed("grid_height", m_specheight/2.0f);
    }
    

    if (ishorz_regular || ishorz_risk)
    {
      int xyscross = ocg.register_xyscaler_01(CR_PIXEL);
      ocg.var_static(DT_1F, "crossed = inormed.y");
      ocg.xyscale_y_01("crossed", xyscross);
    }
    else if (isvert_regular || isvert_risk)
    {
      int xyscross = ocg.register_xyscaler_01(CR_PIXEL);
      ocg.var_static(DT_1F, "crossed = inormed.x");
      ocg.xyscale_x_01("crossed", xyscross);
    }
        
    if (m_maxsteps && m_stepsize > 0.00001f)
    {
      ocg.var_fixed("grid_step", m_stepsize);
      if (m_gridtype == EGO_REGULAR_B || m_gridtype == EGO_RISK_B)
        ocg.push("grid_step = grid_step/(databounds[1]-databounds[0]);");
      
      int step_xys_01 = ocg.register_xyscaler_01(m_additcn == CR_SAME? coords_type_t::getCoordination() : m_additcn);
      
      if (ishorz_regular)       ocg.xyscale_y_01("grid_step", step_xys_01);
      else if (isvert_regular)  ocg.xyscale_x_01("grid_step", step_xys_01);
      else if (ishorz_risk){    ocg.xyscale_y_01("grid_step", step_xys_01);   ocg.xyscale_x_pixel("grid_height", risk_pixing_height);   }
      else if (isvert_risk){    ocg.xyscale_x_01("grid_step", step_xys_01);   ocg.xyscale_y_pixel("grid_height", risk_pixing_height);   }
      
//      ocg.push( "int optiid = int(crossed/grid_step + sign(crossed)*0.49);"
      ocg.push( "int optiid = int(crossed/grid_step + sign(crossed)*0.5);"
                "float offset = grid_step*optiid;" );
        
      ocg.push( "float limiter = 1.0;" );
      if (m_maxsteps > 0)
      {
        ocg.var_fixed("grid_limit", m_maxsteps);
        ocg.push("limiter = limiter*(1.0 - step(float(grid_limit), float(optiid)));");
      }
      if (m_bordergrids == false)
      {
        if (ishorz_regular || ishorz_risk)
          ocg.push("limiter = limiter*step(1.0, float(icoords.y))*step(float(icoords.y), float(ov_ibounds.y-1-1));");
        else if (isvert_regular || isvert_risk)
          ocg.push("limiter = limiter*step(1.0, float(icoords.x))*step(float(icoords.x), float(ov_ibounds.x-1-1));");
      }
  
      if (ishorz_regular || ishorz_risk)
      {
        ocg.xyscale_y_pixel("offset", step_xys_px);
        ocg.trace_2linehorz_c(ishorz_risk? "grid_height" : nullptr, nullptr, "offset", "limiter");
      }
      else if (isvert_regular || isvert_risk)
      {
        ocg.xyscale_x_pixel("offset", step_xys_px);
        ocg.trace_2linevert_c(isvert_risk? "grid_height" : nullptr, nullptr, "offset", "limiter");
      }
    }
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/********************************************/

OGridCircular::OGridCircular(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float stepsize, const linestyle_t& linestyle, int maxsteps, float border): Ovldraw_ColorTraced(linestyle), 
  OVLCoordsDynamic(cn, center_x, center_y),
  OVLDimmsOff(),
  m_featcn(featcn), m_stepsize(stepsize), m_maxsteps(maxsteps), m_border(border)
{
}

int OGridCircular::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    
    if (m_maxsteps == 0 || m_stepsize < 0.00001f)
      ;
    else
    {
      ocg.var_fixed("grid_step", m_stepsize);
      
      ocg.var_fixed("grid_limit", m_maxsteps == -1? 65535 : m_maxsteps);
      
      int fc = 0;
      if (m_featcn != CR_SAME)
        fc = ocg.register_xyscaler_pixel(m_featcn);
      ocg.xyscale_x_pixel("grid_step", fc);
      
//      ocg.var_const_static(DT_1F, "border = 1.5");
      ocg.var_fixed("border", m_border);
      ocg.math_pi();
      ocg.push( "float  d2 = inormed.x*inormed.x + inormed.y*inormed.y;"
                "int    optiid = int(floor(sqrt(d2)/grid_step + 0.49));"
                "int    optistep = int(grid_step * optiid);"                   // 0 possible!
                "vec3   r2 = vec3((optistep - border) * (optistep - border), optistep * optistep, (optistep + border) * (optistep + border));"
                "float mixwell_before = (d2 - r2[0])/(r2[1]-r2[0])*(1 - step(r2[1], d2));"
                "float mixwell_aftere = (1 - (d2 - r2[1])/(r2[2]-r2[1]))*(step(r2[1], d2));"
                "float affirmative = step(1.0, float(optistep)) * clamp(mixwell_before + mixwell_aftere, 0.0,1.0);"
//                "affirmative = affirmative*step(1.0, post_in[1]);"
                "result.xy = result.xy + vec2((1 - step(float(grid_limit+1), float(optiid)))*affirmative, atan(float(inormed.x), float(inormed.y))*optistep);"
                );
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/********************************************/

OGridDecart::OGridDecart(COORDINATION cn, float center_x, float center_y, float step_x, float step_y, int absolute_risk_height, float limit_x_left, float limit_x_right, float limit_y_top, float limit_y_bottom, const linestyle_t &linestyle):
  Ovldraw_ColorTraced(linestyle),
  OVLCoordsDynamic(cn, center_x, center_y),
  OVLDimmsOff(),
  m_riskheight(absolute_risk_height)
{
  m_step[0] = step_x;
  m_step[1] = step_y;
  m_limits[0] = limit_x_left;
  m_limits[1] = limit_x_right;
  m_limits[2] = limit_y_top;
  m_limits[3] = limit_y_bottom;
}

int OGridDecart::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {   
    ocg.goto_normed();
    
    ocg.var_fixed("cs_lims_lr", m_limits[0], m_limits[1]);
    ocg.var_fixed("cs_lims_tb", m_limits[2], m_limits[3]);
    ocg.xyscale_xy_pixel("cs_lims_lr", 0);
    ocg.xyscale_xy_pixel("cs_lims_tb", 0);
    if (m_limits[0] != 0.0f)  ocg.trace_linehorz_r(m_limits[0] < 0? nullptr:"cs_lims_lr[0]");
    if (m_limits[1] != 0.0f)  ocg.trace_linehorz_l(m_limits[1] < 0? nullptr:"cs_lims_lr[1]");
    
    if (m_limits[2] != 0.0f)  ocg.trace_linevert_t(m_limits[2] < 0? nullptr:"cs_lims_tb[0]");
    if (m_limits[3] != 0.0f)  ocg.trace_linevert_b(m_limits[3] < 0? nullptr:"cs_lims_tb[1]");
    
    if (m_riskheight > 0)
    {
      ocg.ban_trace(true);
      
      int sn01 = ocg.register_xyscaler_01(CR_PIXEL);
      ocg.var_static(DT_2F, "crossed = inormed");
      ocg.xyscale_xy_01("crossed", sn01);
      
      ocg.var_fixed("cs_steps", m_step[0], m_step[1]);
      int steps_xys_01 = ocg.register_xyscaler_01(coords_type_t::getCoordination());
      int steps_xys_px = ocg.register_xyscaler_pixel(coords_type_t::getCoordination());
      ocg.xyscale_xy_01("cs_steps", steps_xys_01);
      
      ocg.push( "ivec2 optiid = ivec2(crossed.x/cs_steps.x + sign(crossed.x)*0.49, crossed.y/cs_steps.y + sign(crossed.y)*0.49);"
                "vec2 offset = cs_steps * optiid;" );
      
      int sh = (int)(m_riskheight/2) + 1;
      ocg.var_fixed("risk_height", sh == 0? 1 : sh);
      ocg.xyscale_xy_pixel("offset", steps_xys_px);
      ocg.trace_2linevert_c("risk_height", "2", "offset.x",
                              m_limits[0] < 0?
                                m_limits[1] < 0? nullptr : "mix(step(offset.x, cs_lims_lr[1]), 1.0, step(offset.x, 0.0))" :
                                m_limits[1] < 0? "mix(step(-offset.x, cs_lims_lr[0]), 1.0, step(0.0, offset.x))" : 
                                                 "step(abs(offset.x), mix(cs_lims_lr[0], cs_lims_lr[1], step(offset.x, 0)))"
                                             );
      ocg.trace_2linehorz_c("risk_height", "2", "offset.y", 
                              m_limits[2] < 0?
                                m_limits[3] < 0? nullptr : "mix(step(offset.y, cs_lims_tb[1]), 1.0, step(offset.y, 0.0))" :
                                m_limits[3] < 0? "mix(step(-offset.y, cs_lims_tb[0]), 1.0, step(0.0, offset.y))" : 
                                                 "step(abs(offset.y), mix(cs_lims_tb[0], cs_lims_tb[1], step(offset.y, 0)))"
                                           );
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/********************************************/


OGridCells::OGridCells(int rows, int columns, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), m_rows(rows), m_columns(columns)
{
}

int         OGridCells::fshOVCoords(int overlay, bool switchedab, char* to) const
{  
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, 0);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  
  ocg.goto_normed();
  ocg.var_fixed("cr", m_columns, m_rows);
  
  ocg.push( "vec2   cellsizepix = vec2(float(ov_ibounds.x-1) / cr.x, float(ov_ibounds.y-1) / cr.y);");
  ocg.push( "ivec2  optiid = ivec2(inormed.x / cellsizepix.x + 0.49, inormed.y / cellsizepix.y + 0.49);");
  
  {
    ocg.push( "ioffset = ivec2(cellsizepix.x*optiid.x, 0);");
    ocg.push( "inormed = icoords - ioffset;");
    ocg.trace_linevert_b(nullptr, nullptr, nullptr, nullptr);
  }
  
  {
    ocg.push( "ioffset = ivec2(0, cellsizepix.y*optiid.y);");
    ocg.push( "inormed = icoords - ioffset;");
    ocg.trace_linehorz_l(nullptr, nullptr, nullptr, nullptr);
  }
  
  ocg.goto_func_end(true);
  return ocg.written();
}

/********************************************/

OChannelSeparator::OChannelSeparator(color3f_t color, float curver, int channels):  Ovldraw_ColorDomestic (color),
  m_curver(curver), m_count(channels)
{
}

int OChannelSeparator::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, 0);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.goto_normed();
  ocg.var_fixed("chns", m_count);
  ocg.var_fixed("weight", m_curver);
  
//  ocg.var_static(DT_1F, "crossed = inormed.x/float(ov_ibounds.x-1)");
//  ocg.var_static(DT_1F, "chnstep = float(ov_ibounds.x-1)/chns");
//  ocg.push( "int optiid = int(crossed/chnstep + sign(crossed)*0.49);"
//            "int pxoffset = int(chnstep*optiid);" );
  
  ocg.var_static(DT_1F, "chnstep = float(ov_ibounds.x-1)/chns");
  ocg.var_static(DT_1I, "pxoffset = int(int((inormed.x + chnstep/2)/chnstep)*chnstep);");
  ocg.construct_trail_vec2("chnstep/2", "weight", "inormed.x - pxoffset", "tms");
  ocg.push("mixwell = tms[0];");
  
  ocg.goto_func_end(false);
  return ocg.written();
}
