#include "bsgrid.h"
#include "../core/sheigen/bsshgentrace.h"

enum  EXTENDED_REGULAR { EGO_REGULAR_H, EGO_REGULAR_V, EGO_RISK_H, EGO_RISK_V };

OGridRegular::OGridRegular(REGULAR go, COORDINATION cn, float startchannel, float stepsize, const linestyle_t& linestyle, int maxsteps): IOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, go == REGULAR_HORZ? 0.0f : startchannel, go == REGULAR_HORZ? startchannel : 0.0f),
  OVLDimmsOff(),
  m_gridtype((int)go), m_stepsize(stepsize), m_maxsteps(maxsteps)
{
}

OGridRegular::OGridRegular(RISK gr, COORDINATION cn, float startchannel, float stepsize, float mark_centeroffset, bool absolute_height, float risk_height, const linestyle_t& linestyle, int maxsteps): IOverlayTraced(linestyle),
  OVLCoordsDynamic(cn, gr == RISK_HORZ? mark_centeroffset : startchannel, gr == RISK_HORZ? startchannel : mark_centeroffset),
  OVLDimmsOff(),
  m_gridtype((int)gr + EGO_RISK_H), m_stepsize(stepsize), m_specheight(risk_height), m_babsheight(absolute_height), m_maxsteps(maxsteps)
{
}

int OGridRegular::fshTrace(int overlay, char *to) const
{
  if (m_stepsize == 0)  return -1;    
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    
    ocg.var_fixed("grid_step", m_stepsize);
    
    if (m_gridtype == EGO_REGULAR_H || m_gridtype == EGO_REGULAR_V)
    {
//      ocg.var_fixed("grid_step", m_stepsize);
//      ocg.var_fixed("grid_limit", m_maxsteps == -1? 65536:m_maxsteps);
//      ocg.param_alias(0, "grid_start");
//      ocg.goto_normed("grid_start");
//      if (m_gridtype == EGO_REGULAR_H){ ocg.movecs_pix_y("grid_step");  ocg.var_static(DT_1F, "crossed = inormed.y"); }
//      else{                             ocg.movecs_pix_x("grid_step");  ocg.var_static(DT_1F, "crossed = inormed.x"); }
      
//      ocg.push( "int optiid = floor(abs(crossed)/grid_step + 0.49);"
//                "vec2 offset = vec2(floor(grid_step * optiid + 0.49), -floor(grid_step * optiid + 0.49));" );

//      if (m_gridtype == EGO_REGULAR_H){  ocg.trace_2linehorz_c(nullptr, nullptr, "offset[0]", "(1.0 - step(grid_limit, optiid))");  ocg.trace_2linehorz_c(nullptr, nullptr, "offset[1]", "(1.0 - step(grid_limit, optiid))"); }
//      else                            {  ocg.trace_2linevert_c(nullptr, nullptr, "offset[0]", "(1.0 - step(grid_limit, optiid))");  ocg.trace_2linevert_c(nullptr, nullptr, "offset[1]", "(1.0 - step(grid_limit, optiid))"); }
      ocg.var_fixed("grid_limit", m_maxsteps == -1? 65536:m_maxsteps);
      int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
      if (m_gridtype == EGO_REGULAR_H){ ocg.push_cs_rel_y("grid_step", relstep);  ocg.var_static(DT_1F, "crossed = inormed.y/float(ibounds.y-1)"); }
      else{                             ocg.push_cs_rel_x("grid_step", relstep);  ocg.var_static(DT_1F, "crossed = inormed.x/float(ibounds.x-1)"); }
      
      ocg.push( "int optiid = int(floor(abs(crossed)/grid_step + 0.49));"
                "vec2 offset = vec2(grid_step * optiid, -(grid_step * optiid));" );

      if (m_gridtype == EGO_REGULAR_H){  ocg.pop_cs_rel_y("offset");  ocg.trace_2linehorz_c(nullptr, nullptr, "offset[0]", "(1.0 - step(float(grid_limit), float(optiid)))");  ocg.trace_2linehorz_c(nullptr, nullptr, "offset[1]", "(1.0 - step(float(grid_limit), float(optiid)))"); }
      else                            {  ocg.pop_cs_rel_x("offset");  ocg.trace_2linevert_c(nullptr, nullptr, "offset[0]", "(1.0 - step(float(grid_limit), float(optiid)))");  ocg.trace_2linevert_c(nullptr, nullptr, "offset[1]", "(1.0 - step(float(grid_limit), float(optiid)))"); }
    
    }
    else if (m_gridtype == EGO_RISK_H || m_gridtype == EGO_RISK_V)
    {
      int pixing_height;
      if (m_babsheight)
      {
        int sh = (int)(m_specheight/2) + 1;
        ocg.var_fixed("grid_height", sh == 0? 1 : sh/* + (1 - (sh % 2))*/);
        pixing_height = ocg.add_movecs_pixing(CR_ABSOLUTE_NOSCALED);
      }
      else
      {
        ocg.var_fixed("grid_height", m_specheight/2.0f);
        pixing_height = 0; //ocg.add_movecs_pixing(coords_type_t::getCoordination());
      }
      ocg.var_fixed("grid_limit", m_maxsteps == -1? 65536:m_maxsteps);
      
      int relstep = ocg.add_movecs_rel(coords_type_t::getCoordination());
      if (m_gridtype == EGO_RISK_H) { ocg.push_cs_rel_y("grid_step", relstep); ocg.movecs_pix_x("grid_height", pixing_height); ocg.var_static(DT_1F, "crossed = inormed.y/float(ibounds.y-1)"); }
      else {                          ocg.push_cs_rel_x("grid_step", relstep); ocg.movecs_pix_y("grid_height", pixing_height); ocg.var_static(DT_1F, "crossed = inormed.x/float(ibounds.x-1)"); }
      
      ocg.push( "int optiid = int(floor(abs(crossed)/grid_step + 0.49));"
                "vec2 offset = vec2(grid_step * optiid, -(grid_step * optiid));" );
      
      if (m_gridtype == EGO_RISK_H) { ocg.pop_cs_rel_y("offset"); ocg.trace_2linehorz_c("grid_height", nullptr, "offset[0]", "(1.0 - step(float(grid_limit), float(optiid)))"); ocg.trace_2linehorz_c("grid_height", nullptr, "offset[1]", "(1.0 - step(float(grid_limit), float(optiid)))"); }
      else                          { ocg.pop_cs_rel_x("offset"); ocg.trace_2linevert_c("grid_height", nullptr, "offset[0]", "(1.0 - step(float(grid_limit), float(optiid)))"); ocg.trace_2linevert_c("grid_height", nullptr, "offset[1]", "(1.0 - step(float(grid_limit), float(optiid)))"); }
      
    }    
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/********************************************/

OGridCircular::OGridCircular(COORDINATION cn, float center_x, float center_y, COORDINATION featcn, float stepsize, const linestyle_t& linestyle, int maxsteps): IOverlayTraced(linestyle), 
  OVLCoordsDynamic(cn, center_x, center_y),
  OVLDimmsOff(),
  m_featcn(featcn), m_stepsize(stepsize), m_maxsteps(maxsteps)
{
}

int OGridCircular::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("grid_step", m_stepsize);
    
    ocg.var_fixed("grid_limit", m_maxsteps == -1? 65535 : m_maxsteps);
    
    int fc = 0;
    if (m_featcn != CR_SAME)
      fc = ocg.add_movecs_pixing(m_featcn);
    ocg.movecs_pix_x("grid_step", fc);
    
    ocg.var_const_static(DT_1F, "border = 1.5");
    ocg.math_pi();
//    ocg.push( "float  d2 = dot(inormed, inormed);"
//              "int    optiid = int(floor(sqrt(d2)/grid_step + 0.49));"
//              "int    optistep = grid_step * optiid;"                   // 0 possible!
//              "vec3   r2 = vec3((optistep - border) * (optistep - border), optistep * optistep, (optistep + border) * (optistep + border));"
//              "float mixwell_before = (d2 - r2[0])/(r2[1]-r2[0])*(1 - step(r2[1], d2));"
//              "float mixwell_aftere = (1 - (d2 - r2[1])/(r2[2]-r2[1]))*(step(r2[1], d2));"
//              "float affirmative = step(1, optistep) * clamp(mixwell_before + mixwell_aftere, 0.0,1.0);"
//              "result += vec3((1 - step(grid_limit+1, optiid))*affirmative, atan(inormed.x, inormed.y)/(2*PI), 2*PI*optistep);");
    
    
    ocg.push( "float  d2 = inormed.x*inormed.x + inormed.y*inormed.y;"
              "int    optiid = int(floor(sqrt(d2)/grid_step + 0.49));"
              "int    optistep = int(grid_step * optiid);"                   // 0 possible!
              "vec3   r2 = vec3((optistep - border) * (optistep - border), optistep * optistep, (optistep + border) * (optistep + border));"
              "float mixwell_before = (d2 - r2[0])/(r2[1]-r2[0])*(1 - step(r2[1], d2));"
              "float mixwell_aftere = (1 - (d2 - r2[1])/(r2[2]-r2[1]))*(step(r2[1], d2));"
              "float affirmative = step(1.0, float(optistep)) * clamp(mixwell_before + mixwell_aftere, 0.0,1.0);"
              "result += vec3((1 - step(float(grid_limit+1), float(optiid)))*affirmative, atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*optistep);");
  }
  ocg.goto_func_end(true);
  return ocg.written();
}


/********************************************/

OGridDecart::OGridDecart(COORDINATION cn, float center_x, float center_y, float step_x, float step_y, int absolute_risk_height, float limit_x_left, float limit_x_right, float limit_y_top, float limit_y_bottom, const linestyle_t &linestyle):
  IOverlayTraced(linestyle),
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

int OGridDecart::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
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
//      ocg.var_static(DT_2F, "crossed = inormed/vec2(float(ibounds.x), float(ibounds.y))");
      ocg.var_static(DT_2F, "crossed = inormed/vec2(ibounds)");
      ocg.push( 
                "ivec2 optiid = ivec2(floor(abs(crossed)/cs_steps + 0.49));"
                "float offset_x = cs_steps.x * optiid.x;"
                "float offset_y = cs_steps.y * optiid.y;"
                );
      {
        ocg.pop_cs_rel_x("offset_x");
        ocg.trace_2linevert_c("risk_height", "2", "-offset_x", m_limits[0] < 0? nullptr:"step(offset_x, cs_lims_lr[0])");
        ocg.trace_2linevert_c("risk_height", "2", "offset_x",  m_limits[1] < 0? nullptr:"step(offset_x, cs_lims_lr[1])");
      }
      {
        ocg.pop_cs_rel_y("offset_y");
        ocg.trace_2linehorz_c("risk_height", "2", "-offset_y", m_limits[2] < 0? nullptr:"step(offset_y, cs_lims_tb[0])");
        ocg.trace_2linehorz_c("risk_height", "2", "offset_y",  m_limits[3] < 0? nullptr:"step(offset_y, cs_lims_tb[1])");
      }
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

