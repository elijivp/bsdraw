/// Overlays:   set of little figures
///   OMarkDashs. View: little lines like | || | ||   | |
///   OCluster. View: little figures, types described by FFORM
/// Created By: Elijah Vlasov
#include "bsmarks.h"

#include "../../core/sheigen/bsshgentrace.h"
#include <memory.h>

OMarkDashs::OMarkDashs(unsigned int maxmarks, COORDINATION cn, float areaPos, COORDINATION featcn, float marksize, const IPalette *ipal, bool discrete): Ovldraw_ColorThroughPalette(ipal, discrete), 
  OVLCoordsStatic(cn, 0.0f, areaPos),
  OVLDimmsOff(),
  m_maxmarks(maxmarks), m_featcn(featcn), m_marksize(marksize)
{
  m_marks = new markinfo_t[m_maxmarks];

  m_dm_coords.count = m_maxmarks;
  m_dm_coords.data = m_marks;
  appendUniform(DT_ARR2, &m_dm_coords);
  
  {
    m_dm_coords.count = m_maxmarks;
    for (unsigned int i=0; i<m_dm_coords.count; i++)
    {
      m_marks[i].tcolor = 0.0f;
      m_marks[i].pos = 0.0f;
    }
  }
}

OMarkDashs::~OMarkDashs()
{
  delete []m_marks;
}

int OMarkDashs::fshOVCoords(int overlay, bool switchedab, char *to) const
{ 
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("marksize", m_marksize);
    int fc = 0;
    if (m_featcn != CR_SAME)
      fc = ocg.register_xyscaler_pixel(m_featcn);
    ocg.xyscale_y_pixel("marksize", fc);
    
    ocg.param_for_arr_begin("mark");
    {
      ocg.push("if (mark[0] > 0.001){");
      {
        ocg.push("inormed.x = icoords.x - floor(mark[1]*ov_ibounds.x + 0.49);");
        ocg.var_static(DT_1F, "oldr = result[0]");
        ocg.trace_linevert_b("marksize", nullptr);
        ocg.push("mixwell = mixwell*sign(oldr) + (1-sign(oldr))*sign(result[0])*mark[0];");
      }
      ocg.push("}");
    }
    ocg.param_for_end();
  }
  ocg.push("inormed.x = ov_ibounds.x/2;");
  ocg.goto_func_end(false);
  return ocg.written();
}


/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/

OCluster::OCluster(bool crossable, unsigned int maxfigures, COORDINATION featcn, float figsize, const IPalette* ipal, bool discrete, float figopacity): 
  Ovldraw_ColorThroughPalette(ipal, discrete), OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f), OVLDimmsOff(),
  m_crossable(crossable), m_total(maxfigures), m_featcn(featcn), m_figsize(figsize), m_figopc(figopacity)
{
  m_items = new clusteritem_t[m_total];

  m_dm_coords.count = m_total;
  m_dm_coords.data = m_items;
  appendUniform(DT_ARR4, &m_dm_coords);
  
  {
    m_dm_coords.count = m_total;
    for (unsigned int i=0; i<m_dm_coords.count; i++)
    {
      m_items[i].x = -1.0f;
      m_items[i].y = -1.0f;
      m_items[i].form_color = 10.0f;
      m_items[i].zoom = 1.0f;
    }
  }
}

OCluster::~OCluster()
{
  delete []m_items;
}

int OCluster::fshOVCoords(int overlay, bool switchedab, char *to) const
{   
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("fsize", m_figsize);
    int fc = 0;
    if (m_featcn != CR_SAME)
      fc = ocg.register_xyscaler_pixel(m_featcn);
    ocg.xyscale_x_pixel("fsize", fc);
    
//    ocg.var_fixed("opacity", m_figopc);
    
    if (m_crossable)
    {
      ocg.param_for_arr_begin("fig", "arrlen");
      ocg.push("float mixwell0 = mixwell;");
    }
    else
    {
      ocg.push("vec4 fig;");
      ocg.push("float bd=100000;");
      ocg.param_for_arr_begin("fig0", "arrlen");
//        ocg.push("float dd = distance(coords.ba, fig0.xy)/fig0[2];");
        ocg.push("float dd = distance(icoords.xy, fig0.xy*ov_ibounds.xy)/fig0[2];");
        ocg.push("fig = mix(fig, fig0, step(dd, bd));");
        ocg.push("bd = mix(bd, dd, step(dd, bd));");
      ocg.param_for_end();
    }
    
      ocg.push("inormed = icoords - ivec2(fig[0]*ov_ibounds.x + 0.49, fig[1]*ov_ibounds.y + 0.49);");
      ocg.push("int fig3type = int(fig[3]/10.0);");
      ocg.var_static(DT_2I, "ifigdimms = ivec2(fsize*fig[2], fsize*fig[2]);");
      
      ocg.push("if (fig3type == 1)"
               "{");
      {
                  ocg.simplemix_square_cc("ifigdimms[0]", m_figopc);
      }
      ocg.push("} else if (fig3type == 2)"
               "{");
      {
                  ocg.simplemix_circle_cc("ifigdimms[0]", m_figopc);
      }
      ocg.push("} else if (fig3type == 3)"
               "{");
      {
                  ocg.simplemix_cross_cc("ifigdimms[0]", m_figopc);
      }
      ocg.push("} else if (fig3type == 4)"
               "{");
      {
                  ocg.simplemix_rhomb_cc("ifigdimms[0]", m_figopc);
      }
      ocg.push("} else if (fig3type == 5)"
               "{");
      {
                  ocg.simplemix_triangle_cc("ifigdimms[0]", 0, m_figopc);
      }
      ocg.push("} else if (fig3type == 6)"
               "{");
      {
                  ocg.simplemix_triangle_cc("ifigdimms[0]", 1, m_figopc);
      }
      ocg.push("} else if (fig3type == 7)"
               "{");
      {
                  ocg.simplemix_triangle_cc("ifigdimms[0]", 2, m_figopc);
      }
      ocg.push("} else if (fig3type == 8)"
               "{");
      {
                  ocg.simplemix_triangle_cc("ifigdimms[0]", 3, m_figopc);
      }
      ocg.push("}");
      
    }
  
  if (m_crossable)
  {
    ocg.push("result[0] = mix(result[0], fig[3] - fig3type*10.0, step(mixwell0, mixwell));");
    ocg.push("mixwell = max(mixwell0, mixwell);");
    ocg.param_for_end();
  }
  else
  {
    ocg.push("result[0] = fig[3] - fig3type*10.0;");
  }
  ocg.push("inormed.x = ov_ibounds.x/2;");
  ocg.goto_func_end(false);
  return ocg.written();
}

inline float            makeFormAndColor(OCluster::FFORM form, float color){ return ((int)form)*10.0f + (color > 1.0f? 1.0f : color < 0.0f? 0.0f : color);  }
inline OCluster::FFORM takeForm(float form_color){ return (OCluster::FFORM)(form_color/10.0f);  }
inline float            takeColor(float form_color){ return form_color - ((int)(form_color/10.0f))*10;  }

void  OCluster::updateItem(unsigned int idx, float x, float y){ m_items[idx].x = x; m_items[idx].y = y; }
void  OCluster::updateItemColor(unsigned int idx, float color){ m_items[idx].form_color = makeFormAndColor(takeForm(m_items[idx].form_color), color); }
void  OCluster::updateItemZoom(unsigned int idx, float zoom){ m_items[idx].zoom = zoom; }
void  OCluster::updateItemForm(unsigned int idx, FFORM form){ m_items[idx].form_color = makeFormAndColor(form, takeColor(m_items[idx].form_color)); }
void  OCluster::updateItem(unsigned int idx, float x, float y, float color){ m_items[idx].x = x; m_items[idx].y = y; m_items[idx].form_color = makeFormAndColor(takeForm(m_items[idx].form_color), color); }
void  OCluster::updateItem(unsigned int idx, float x, float y, float color, FFORM form, float zoom)
{
  m_items[idx].x = x;
  m_items[idx].y = y;
  m_items[idx].form_color = makeFormAndColor(form, color);
  m_items[idx].zoom = zoom;
}

void OCluster::updateFinished()
{
  _Ovldraw::updateParameter(false, true);
}




/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/

OTrass::OTrass(unsigned int trasslimit, unsigned int linestotal, const IPalette* ipal, bool discrete, unsigned int linesframe): Ovldraw_ColorThroughPalette(ipal, discrete),
  trass_limit(trasslimit), tlines_total(linestotal), tlines_frame(linesframe), tline_current(linestotal-1), tline_skroll(0),
  vv_repeatcounter(0), vv_tline_repeated(0), vv_maxdistinterp(0.5f)
{
  tlines_texture = new float[TPS*trass_limit * (tlines_total + tlines_frame)];
  memset(tlines_texture, 0, sizeof(float)*TPS*trass_limit * (tlines_total + tlines_frame));
  dm_trass.w = trass_limit;
  dm_trass.len = tlines_frame;
  dm_trass.data = &tlines_texture[(TPS*trass_limit)*(tlines_total-1)];
  appendUniform(DT_2D3F, &dm_trass);
//  vv_markups = new int[trass_limit];
//  memset(vv_markups, 0, sizeof(int)*trasslimit);
}

OTrass::~OTrass()
{
//  delete []tlines_texture;
//  delete []vv_markups;
}

void OTrass::setTrail(int pxwidth, float lineary, bool update)
{
  trail_width_px = pxwidth;
  trail_lineary = lineary;
  updateParameter(true, update);
}

void OTrass::setMaxInterpDistance(float mid)
{
  vv_maxdistinterp = mid;
}

void OTrass::_nladded(bool update)
{
  if (vv_repeatcounter)
  {
    if (vv_repeatcounter < 30)
    {
      float* tline_origin_specmod = &tlines_texture[TPS*trass_limit*vv_tline_repeated];
      const float* tline_origin = tline_origin_specmod;
      float* tline_update_specmod = &tlines_texture[TPS*trass_limit*tline_current];
      const float* tline_update = tline_update_specmod;
      
      int backwalk[30];
      for (int i=0; i<vv_repeatcounter; i++)
        backwalk[i] = (tline_current + 1 + i) % tlines_total;
        
      for (int j=0; j<trass_limit; j++)
      {
        if (tline_origin[TPS*j + 0] <= 0.0f || tline_update[TPS*j + 0] <= 0.0f)
        {
          if (tline_origin[TPS*j + 0] < 0.0f)
            tline_origin_specmod[TPS*j + 0] = -tline_origin_specmod[TPS*j + 0];
          if (tline_update[TPS*j + 0] < 0.0f)
            tline_update_specmod[TPS*j + 0] = -tline_update_specmod[TPS*j + 0];
          continue;
        }
        if (tline_update[TPS*j + 1] - tline_origin[TPS*j + 1] > vv_maxdistinterp || tline_update[TPS*j + 1] - tline_origin[TPS*j + 1] < -vv_maxdistinterp)
          continue;
        for (int i=0; i<vv_repeatcounter; i++)
        {
          tlines_texture[TPS*trass_limit*backwalk[i] + TPS*j + 1] += (tline_update[TPS*j + 1] - tline_origin[TPS*j + 1])*(vv_repeatcounter - 1 - i + 1)/(vv_repeatcounter+1);
        }
      }
    }
    vv_repeatcounter = 0;
//    memset(vv_markups, 0, sizeof(int)*trass_limit);
  }
  
  _nlup(update);
}

void OTrass::_nlup(bool update)
{
  dm_trass.data = &tlines_texture[TPS*trass_limit*((tline_current + tline_skroll)%tlines_total) ];
  updateParameter(false, update);
}

void OTrass::appendTrassline(const trasspoint_t tps[], bool update)
{
  if (--tline_current < 0) tline_current = tlines_total - 1;
  memcpy(&tlines_texture[TPS*trass_limit*tline_current], tps, sizeof(float)*TPS*trass_limit);
  if (tline_current < tlines_frame)
    memcpy(&tlines_texture[TPS*trass_limit*(tlines_total + tline_current)], tps, sizeof(float)*TPS*trass_limit);
  
  _nladded(update);
}

void OTrass::appendEmptyline(bool update)
{
  vv_repeatcounter = 0;
  
  if (--tline_current < 0) tline_current = tlines_total - 1;
  memset(&tlines_texture[TPS*trass_limit*tline_current], 0, sizeof(float)*TPS*trass_limit);
  if (tline_current < tlines_frame)
    memset(&tlines_texture[TPS*trass_limit*(tlines_total + tline_current)], 0, sizeof(float)*TPS*trass_limit);
  
  _nladded(update);
}

void OTrass::repeatTrassline(bool interpolate, bool update)
{
  if (vv_repeatcounter == 0)
    vv_tline_repeated = tline_current;
  if (interpolate)
    vv_repeatcounter++;
  
  if (--tline_current < 0) tline_current = tlines_total - 1;
  memcpy(&tlines_texture[TPS*trass_limit*tline_current], &tlines_texture[TPS*trass_limit*vv_tline_repeated], sizeof(float)*TPS*trass_limit);
  if (tline_current < tlines_frame)
    memcpy(&tlines_texture[TPS*trass_limit*(tlines_total + tline_current)], &tlines_texture[TPS*trass_limit*vv_tline_repeated], sizeof(float)*TPS*trass_limit);
  
  _nlup(update);
}

void OTrass::skipLines(int count, bool update)
{
  vv_repeatcounter = 0;
  tline_current -= count;
  if (tline_current < 0) tline_current += tlines_total;
  _nlup(update);
}

void OTrass::clearTrasses(bool update)
{
  memset(tlines_texture, 0, sizeof(float)*TPS*trass_limit * (tlines_total + tlines_frame));
  tline_current = tlines_total - 1;
  updateParameter(false, update);
}

void OTrass::scroll(int offset, bool update)
{
  if (offset < 0)
    offset = 0;
  tline_skroll = offset;
  _nlup(update);
}

void OTrass::update()
{
  _nlup(true);
}

/******************************************************************************************************************/


int OTrass::fshOVCoords(int overlay, bool /*switchedab*/, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_fixed("line", (int)trass_limit);
    ocg.var_fixed("frame", (int)tlines_frame);
    
    ocg.push("float ty=float(inormed.y)/float(frame);");
    ocg.push("vec4 niph=vec4(0.0);"
             "float dist=9999;"
             );
    ocg.push("for (int i=0; i<line; i++){"
               "float tx = i/float(line-1);"
               "vec3 iph = texture("); ocg.param_get(); ocg.push(", vec2(tx, ty)).rgb;"
               "float dd = abs(float(inormed.x) - iph[1]*ov_ibounds.x);"
               "float good = (1.0 - step(iph[0], 0.0))*step(dd, dist);"
               "niph = mix(niph, vec4(iph, float(i)), good);"
               "dist = mix(dist, dd, good);"
             "}"
             );
    
    ocg.var_fixed("trw", trail_width_px);
    ocg.var_fixed("trl", trail_lineary);
    
    // mathf = [  (0 if c >= 0 else 1)*(2.0 - x*2) + (1 if c >= 0 else -1)*(1.0 - x)*(1.0 / (1 + 4.2*abs(c)*x)) for c in [ -1.0, -0.5, 0, 0.5, 1.0 ] ]
    ocg.push(
             "int ipos = int(niph[1]*ov_ibounds.x);"
             "vec3 tms = vec3(step(float(abs(ipos - inormed.x)), 0.0), 0.0, 0.0);"
             "tms[1] = clamp((1+trw - abs(ipos - inormed.x))/float(1+trw), 0.0, 1.0);"
             "tms[2] = tms[1]*(1.0 / (1 + 6.2*abs(trl)*(1.0 - tms[1])) );"
             "tms[2] = mix(2.0*tms[1] - tms[2], tms[2], step(0.0, trl));"
             "tms[1] = clamp(tms[2], 0.0, 1.0) - tms[0];"
          
             "mixwell = (1.0 - step(niph[0], 0.0))*(tms[0] + tms[1]);"
          );
    
    finalizeOVCoords(ocg);
  }
  ocg.push("inormed.x = ov_ibounds.x/2;");
  ocg.goto_func_end(false);
  return ocg.written();
}

void OTrass::finalizeOVCoords(FshOVCoordsConstructor& ocg) const
{
  ocg.push(  "result[0] = niph[0];" );
}

/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/



OTrassSelectable::OTrassSelectable(unsigned int trasslimit, unsigned int linestotal, const IPalette* ipal, bool discrete, float selcolor_bypalette, unsigned int linesframe): 
  OTrass(trasslimit, linestotal, ipal, discrete, linesframe), selectcolor(selcolor_bypalette), selectidx(-1)
{
  appendUniform(DT_1I, &selectidx);
}

void OTrassSelectable::select(int trassidx, bool update)
{
  selectidx = trassidx;
  updateParameter(false, update);
}

void OTrassSelectable::finalizeOVCoords(FshOVCoordsConstructor& ocg) const
{
  ocg.var_const_fixed("sclr", selectcolor);
  ocg.param_alias("sidx");
  ocg.push(
            "result[0] = mix(0.0, mix(0.01, mix(niph[0], sclr, step(niph[3], float(sidx))*step(float(sidx), niph[3])) , tms[0]), tms[0] + tms[1] );"
            "result[1] = mix(0.0, 0.35, 1.0 - step(tms[1], 0.0));"
           );
}



/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/



OTrassMultiSelectable::OTrassMultiSelectable(unsigned int trasslimit, unsigned int linestotal, const IPalette* ipal, bool discrete, float selcolor_bypalette, unsigned int linesframe): 
  OTrass(trasslimit, linestotal, ipal, discrete, linesframe), selectcolor(selcolor_bypalette)
{
  selectarr = new int[trasslimit];
  memset(selectarr, 0, sizeof(int)*trasslimit);
  _dm_select.data = selectarr;
  _dm_select.count = trasslimit;
  appendUniform(DT_ARRI, &_dm_select);
}

OTrassMultiSelectable::~OTrassMultiSelectable()
{
  delete []selectarr;
}

void OTrassMultiSelectable::select(int trassidx, bool selected, bool update)
{
  if (trassidx >= 0 && trassidx < trass_limit)
  {
    selectarr[trassidx] = selected? 1 : 0;
    updateParameter(false, update);
  }
}

void OTrassMultiSelectable::finalizeOVCoords(FshOVCoordsConstructor& ocg) const
{
  ocg.var_const_fixed("sclr", selectcolor);
  ocg.push("int selected = ");    ocg.param_get();  ocg.push("[int(niph[3])];");
  ocg.push(
            "result[0] = mix(0.0, mix(0.01, mix(niph[0], sclr, selected) , tms[0]), tms[0] + tms[1] );"
            "result[1] = mix(0.0, 0.35, 1.0 - step(tms[1], 0.0));"
           );
}
