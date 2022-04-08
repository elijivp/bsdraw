/// Overlays:   set of little figures
///   OMarkDashs. View: little lines like | || | ||   | |
///   OCluster. View: little figures, types described by FFORM
/// Created By: Elijah Vlasov
#include "bsmarks.h"

#include "../../core/sheigen/bsshgentrace.h"

OMarkDashs::OMarkDashs(unsigned int maxmarks, COORDINATION cn, float areaPos, COORDINATION featcn, float marksize, const IPalette *ipal, bool discrete): DrawOverlay_ColorThroughPalette(ipal, discrete), 
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

int OMarkDashs::fshTrace(int overlay, bool rotated, char *to) const
{ 
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.param_pass(); // Kostyl
  {
    ocg.goto_normed();
    ocg.var_fixed("marksize", m_marksize);
    int fc = 0;
    if (m_featcn != CR_SAME)
      fc = ocg.add_movecs_pixing(m_featcn);
    ocg.movecs_pix_y("marksize", fc);
    
    ocg.param_for_arr_begin("mark");
    {
      ocg.push("if (mark[0] > 0.001){");
      {
        ocg.push("inormed.x = icoords.x - floor(mark[1]*ibounds.x + 0.49);");
        ocg.var_static(DT_1F, "oldr = result[0]");
        ocg.trace_linevert_b("marksize", nullptr);
        ocg.push("mixwell = mixwell*sign(oldr) + (1-sign(oldr))*sign(result[0])*mark[0];");
      }
      ocg.push("}");
    }
    ocg.param_for_end();
  }
  ocg.push("inormed.x = ibounds.x/2;");
  ocg.goto_func_end(false);
  return ocg.written();
}


/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/

OCluster::OCluster(bool crossable, unsigned int maxfigures, COORDINATION featcn, float figsize, const IPalette* ipal, bool discrete, float figopacity): 
  DrawOverlay_ColorThroughPalette(ipal, discrete), OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f), OVLDimmsOff(),
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

int OCluster::fshTrace(int overlay, bool rotated, char *to) const
{   
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.param_pass(); // Kostyl
  {
    ocg.goto_normed();
    ocg.var_fixed("fsize", m_figsize);
    int fc = 0;
    if (m_featcn != CR_SAME)
      fc = ocg.add_movecs_pixing(m_featcn);
    ocg.movecs_pix_x("fsize", fc);
    
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
        ocg.push("float dd = distance(icoords.xy, fig0.xy*ibounds.xy)/fig0[2];");
        ocg.push("fig = mix(fig, fig0, step(dd, bd));");
        ocg.push("bd = mix(bd, dd, step(dd, bd));");
      ocg.param_for_end();
    }
    
      ocg.push("inormed = icoords - ivec2(fig[0]*ibounds.x + 0.49, fig[1]*ibounds.y + 0.49);");
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
  ocg.push("inormed.x = ibounds.x/2;");
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
  _DrawOverlay::updateParameter(false, true);
}
