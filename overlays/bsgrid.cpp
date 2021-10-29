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

OGridRegular::OGridRegular(REGULAR go, COORDINATION cn, float startchannel, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showGridAtZero): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, go == REGULAR_HORZ? 0.0f : startchannel, go == REGULAR_HORZ? startchannel : 0.0f),
  OVLDimmsOff(),
  m_gridtype((int)go), m_stepsize(stepsize), m_maxsteps(maxsteps), m_zeroreg(showGridAtZero), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(OGridRegular::REGULAR go, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showGridAtZero): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, go == REGULAR_HORZ? 0.0f : startchannel, go == REGULAR_HORZ? startchannel : 0.0f),
  OVLDimmsOff(),
  m_gridtype((int)go), m_stepsize(stepsize), m_maxsteps(maxsteps), m_zeroreg(showGridAtZero), m_additcn(cnstep)
{
}

OGridRegular::OGridRegular(COORDINATION cn, float start, float stepsize, const linestyle_t& linestyle, int maxsteps, bool showGridAtZero): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, 0.0f, start),
  OVLDimmsOff(),
  m_gridtype(EGO_REGULAR_B), m_stepsize(stepsize), m_maxsteps(maxsteps), m_zeroreg(showGridAtZero), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(RISK gr, COORDINATION cn, float startchannel, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps): DrawOverlayTraced(linestyle),
  OVLCoordsDynamic(cn, gr == RISK_HORZ? mark_centeroffset : startchannel, gr == RISK_HORZ? startchannel : mark_centeroffset),
  OVLDimmsOff(),
  m_gridtype((int)gr + EGO_RISK_H), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(CR_SAME)
{
}

OGridRegular::OGridRegular(OGridRegular::RISK gr, COORDINATION cn, float startchannel, COORDINATION cnstep, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps)
  : DrawOverlayTraced(linestyle),
    OVLCoordsDynamic(cn, gr == RISK_HORZ? mark_centeroffset : startchannel, gr == RISK_HORZ? startchannel : mark_centeroffset),
    OVLDimmsOff(),
    m_gridtype((int)gr + EGO_RISK_H), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(cnstep)
{
}

OGridRegular::OGridRegular(COORDINATION cn, float start, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps)
  : DrawOverlayTraced(linestyle),
    OVLCoordsDynamic(cn, start, mark_centeroffset),
    OVLDimmsOff(),
    m_gridtype(EGO_RISK_B), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps), m_additcn(CR_SAME)
{
}

//#define OGRID_SYMMETRY "+0.49f"
//#define OGRID_SYMMETRY ""

int OGridRegular::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, m_gridtype == EGO_REGULAR_B || m_gridtype == EGO_RISK_B? 
                           FshTraceGenerator::OINC_DATABOUNDS : 0);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    if (m_maxsteps == 0 || m_stepsize < 0.00001f)
      ;
    else
    {
      ocg.var_fixed("grid_step", m_stepsize);
      if (m_gridtype == EGO_REGULAR_B || m_gridtype == EGO_RISK_B)
        ocg.push("grid_step = grid_step/(databounds[1]-databounds[0]);");
      
      int relstep = ocg.add_movecs_rel(m_additcn == CR_SAME? coords_type_t::getCoordination() : m_additcn);
      
      if (m_gridtype == EGO_REGULAR_H || m_gridtype == EGO_REGULAR_V || m_gridtype == EGO_REGULAR_B)
      {
        ocg.var_fixed("grid_limit", m_maxsteps == -1? 65536:m_maxsteps);
//        int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
//        if ()
        if (m_gridtype == EGO_REGULAR_H || m_gridtype == EGO_REGULAR_B)
        {
          ocg.push_cs_rel_y("grid_step", relstep);
          ocg.var_static(DT_1F, "crossed = inormed.y/float(ibounds.y-1)");
        }
        else if (m_gridtype == EGO_REGULAR_V)
        {
          ocg.push_cs_rel_x("grid_step", relstep);
          ocg.var_static(DT_1F, "crossed = inormed.x/float(ibounds.x-1)"); 
        }

        ocg.push( "int optiid = int(crossed/grid_step + sign(crossed)*0.49);"
                  "float offset = grid_step*optiid;" );
  
        if (m_gridtype == EGO_REGULAR_H || m_gridtype == EGO_REGULAR_B)
        { 
          ocg.pop_cs_rel_y("offset");  
          ocg.trace_2linehorz_c(nullptr, nullptr, "offset", m_zeroreg ? "(1.0 - step(float(grid_limit), float(optiid)))":
                                                                        "(1.0 - step(float(grid_limit), float(optiid)))*sign(icoords.y)");
        }
        else
        {
          ocg.pop_cs_rel_x("offset");
          ocg.trace_2linevert_c(nullptr, nullptr, "offset", m_zeroreg? "(1.0 - step(float(grid_limit), float(optiid)))":
                                                                       "(1.0 - step(float(grid_limit), float(optiid)))*sign(icoords.x)");
        }
      
      }
      else if (m_gridtype == EGO_RISK_H || m_gridtype == EGO_RISK_V || m_gridtype == EGO_RISK_B)
      {
        int pixing_height;
        if (m_babsheight)
        {
          int sh = (int)(m_specheight/2) + 1;
          ocg.var_fixed("grid_height", sh == 0? 1.0f : float(sh));
          pixing_height = ocg.add_movecs_pixing(CR_ABSOLUTE_NOSCALED);
        }
        else
        {
          ocg.var_fixed("grid_height", m_specheight/2.0f);
          pixing_height = 0; //ocg.add_movecs_pixing(coords_type_t::getCoordination());
        }
        ocg.var_fixed("grid_limit", m_maxsteps == -1? 65536:m_maxsteps);
        
//        int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
        if (m_gridtype == EGO_RISK_H || m_gridtype == EGO_RISK_B)
        {
          ocg.push_cs_rel_y("grid_step", relstep); 
          ocg.movecs_pix_x("grid_height", pixing_height);
          ocg.var_static(DT_1F, "crossed = inormed.y/float(ibounds.y)"); 
        }
        else
        {
          ocg.push_cs_rel_x("grid_step", relstep);
          ocg.movecs_pix_y("grid_height", pixing_height);
          ocg.var_static(DT_1F, "crossed = inormed.x/float(ibounds.x)");
        }
        
        ocg.push( "int optiid = int(crossed/grid_step + sign(crossed)*0.49);"
                  "float offset = grid_step*optiid;" );
        if (m_gridtype == EGO_RISK_H || m_gridtype == EGO_RISK_B)
        {
          ocg.pop_cs_rel_y("offset");
          ocg.trace_2linehorz_c("grid_height", nullptr, "offset", "(1.0 - step(float(grid_limit), float(optiid)))"); // no *sign(icoords.x), cos of little risks
        }
        else
        {
          ocg.pop_cs_rel_x("offset");
          ocg.trace_2linevert_c("grid_height", nullptr, "offset", "(1.0 - step(float(grid_limit), float(optiid)))"); // no *sign(icoords.x), cos of little risks
          
        }
      }
    }
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/********************************************/

OGridCircular::OGridCircular(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float stepsize, const linestyle_t& linestyle, int maxsteps, float border): DrawOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, center_x, center_y),
  OVLDimmsOff(),
  m_featcn(featcn), m_stepsize(stepsize), m_maxsteps(maxsteps), m_border(border)
{
}

int OGridCircular::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
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
        fc = ocg.add_movecs_pixing(m_featcn);
      ocg.movecs_pix_x("grid_step", fc);
      
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
  DrawOverlayTraced(linestyle),
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

int OGridDecart::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
   
    ocg.var_fixed("cs_lims_lr", m_limits[0], m_limits[1]);
    ocg.var_fixed("cs_lims_tb", m_limits[2], m_limits[3]);
    ocg.movecs_pix_x("cs_lims_lr", 0);
    ocg.movecs_pix_y("cs_lims_tb", 0);
    if (m_limits[0] != 0.0f)  ocg.trace_linehorz_r(m_limits[0] < 0? nullptr:"cs_lims_lr[0]");
    if (m_limits[1] != 0.0f)  ocg.trace_linehorz_l(m_limits[1] < 0? nullptr:"cs_lims_lr[1]");
    
    if (m_limits[2] != 0.0f)  ocg.trace_linevert_t(m_limits[2] < 0? nullptr:"cs_lims_tb[0]");
    if (m_limits[3] != 0.0f)  ocg.trace_linevert_b(m_limits[3] < 0? nullptr:"cs_lims_tb[1]");
    
    if (m_riskheight > 0)
    {
      ocg.ban_trace(true);
      ocg.var_fixed("cs_steps", m_step[0], m_step[1]);
      int sh = (int)(m_riskheight/2) + 1;
      ocg.var_fixed("risk_height", sh == 0? 1 : sh);
      int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
      ocg.push_cs_rel("cs_steps", relstep);
      
      ocg.var_static(DT_2F, "crossed = inormed/vec2(ibounds)");
      
      ocg.push( "ivec2 optiid = ivec2(crossed.x/cs_steps.x + sign(crossed.x)*0.49, crossed.y/cs_steps.y + sign(crossed.y)*0.49);"
                "vec2 offset = cs_steps * optiid;" );
      {
        ocg.pop_cs_rel_x("offset.x");
        ocg.trace_2linevert_c("risk_height", "2", "offset.x",
                                m_limits[0] < 0?
                                  m_limits[1] < 0? nullptr : "mix(step(offset.x, cs_lims_lr[1]), 1.0, step(offset.x, 0.0))" :
                                  m_limits[1] < 0? "mix(step(-offset.x, cs_lims_lr[0]), 1.0, step(0.0, offset.x))" : 
                                                   "step(abs(offset.x), mix(cs_lims_lr[0], cs_lims_lr[1], step(offset.x, 0)))"
                                               );
      }
      {
        ocg.pop_cs_rel_y("offset.y");
        ocg.trace_2linehorz_c("risk_height", "2", "offset.y", 
                                m_limits[2] < 0?
                                  m_limits[3] < 0? nullptr : "mix(step(offset.y, cs_lims_tb[1]), 1.0, step(offset.y, 0.0))" :
                                  m_limits[3] < 0? "mix(step(-offset.y, cs_lims_tb[0]), 1.0, step(0.0, offset.y))" : 
                                                   "step(abs(offset.y), mix(cs_lims_tb[0], cs_lims_tb[1], step(offset.y, 0)))"
                                             );
      }
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

