/// Overlays:   set of little figures
///   OMarkDashs. View: little lines like | || | ||   | |
///   OMarkFigures. View: little figures, types described by FFORM
/// Created By: Elijah Vlasov
#include "bsmarks.h"

#include "../../core/sheigen/bsshgentrace.h"

OMarkDashs::OMarkDashs(unsigned int maxmarks, COORDINATION cn, float areaPos, COORDINATION featcn, float marksize, const IPalette *ipal, bool discrete): DrawOverlayHard(ipal, discrete), 
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

struct    OMarkFigures::figinfo_t
{
  float   x, y;
  float   zoom;
  float   form_color;
};

OMarkFigures::OMarkFigures(unsigned int maxfigures, COORDINATION featcn, float figsize, const IPalette* ipal, bool discrete, float figopacity): DrawOverlayHard(ipal, discrete), 
  OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f),
  OVLDimmsOff(),
  m_maxfigures(maxfigures), m_featcn(featcn), m_figsize(figsize), m_figopc(figopacity)
{
  m_figures = new figinfo_t[m_maxfigures];

  m_dm_coords.count = m_maxfigures;
  m_dm_coords.data = m_figures;
  appendUniform(DT_ARR4, &m_dm_coords);
  
  {
    m_dm_coords.count = m_maxfigures;
    for (unsigned int i=0; i<m_dm_coords.count; i++)
    {
      m_figures[i].x = 0.5;
      m_figures[i].y = 0.5;
      m_figures[i].form_color = 10.0f;
      m_figures[i].zoom = 1.0f;
    }
  }
}

OMarkFigures::~OMarkFigures()
{
  delete []m_figures;
}

int OMarkFigures::fshTrace(int overlay, bool rotated, char *to) const
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
    
    ocg.push("vec3 resultold;");
    
    ocg.param_for_arr_begin("fig", "arrlen");
    {
      ocg.push("inormed = ivec2(icoords.x - floor(fig[0]*ibounds.x + 0.49), icoords.y - floor(fig[1]*ibounds.y + 0.49));");
      ocg.push("resultold = result; result = vec3(0.0, 0.0, 0.0);");
      
      ocg.push("int figtype = int(fig[3]/10.0);"
               "fig[3] = fig[3] - figtype*10.0;");
      
      ocg.push("if (figtype == 1){");
      {
        ocg.var_static(DT_2I, "ifigdimms = ivec2(fsize*fig[2], fsize*fig[2]);");
        ocg.trace_rect_cc_begin("");
        ocg.trace_rect_cc_end("ifigdimms", m_figopc);
      }
      ocg.push("} else if (figtype == 2){");
      {
        ocg.trace_circle_cc_begin("fsize*fig[2]", "2");
        ocg.trace_circle_cc_end(m_figopc);
      }
      ocg.push("} else if (figtype == 3){");
      {
        ocg.trace_lines_x("fsize*fig[2]");
      }
      ocg.push("} else if (figtype == 4){");
      {
        ocg.trace_triangle_cc_begin("fsize*fig[2]");
        ocg.trace_triangle_cc_end("fsize*fig[2]", 0, m_figopc);
      }
      ocg.push("} else if (figtype == 5){");
      {
        ocg.trace_triangle_cc_begin("fsize*fig[2]");
        ocg.trace_triangle_cc_end("fsize*fig[2]", 1, m_figopc);
      }
      ocg.push("}");
      
      
      ocg.push("_fvar = sign(result[0]);");
      ocg.push("result = mix(resultold, result, _fvar);");
      ocg.push("mixwell = mix(mixwell, fig[3], _fvar);");
    }
    ocg.param_for_end();
  }
  ocg.push("inormed.x = ibounds.x/2;");
  ocg.goto_func_end(false);
  return ocg.written();
}

inline float            makeFormAndColor(OMarkFigures::FFORM form, float color){ return ((int)form)*10.0f + (color > 1.0f? 1.0f : color < 0.0f? 0.0f : color);  }
inline OMarkFigures::FFORM takeForm(float form_color){ return (OMarkFigures::FFORM)(form_color/10.0f);  }
inline float            takeColor(float form_color){ return form_color - ((int)(form_color/10.0))*10;  }

void  OMarkFigures::updateFigure(unsigned int idx, float x, float y){ m_figures[idx].x = x; m_figures[idx].y = y; }
void  OMarkFigures::updateFigureColor(unsigned int idx, float color){ m_figures[idx].form_color = makeFormAndColor(takeForm(m_figures[idx].form_color), color); }
void  OMarkFigures::updateFigureZoom(unsigned int idx, float zoom){ m_figures[idx].zoom = zoom; }
void  OMarkFigures::updateFigureForm(unsigned int idx, FFORM form){ m_figures[idx].form_color = makeFormAndColor(form, takeColor(m_figures[idx].form_color)); }
void  OMarkFigures::updateFigure(unsigned int idx, float x, float y, float color){ m_figures[idx].x = x; m_figures[idx].y = y; m_figures[idx].form_color = makeFormAndColor(takeForm(m_figures[idx].form_color), color); }
void  OMarkFigures::updateFigure(unsigned int idx, float x, float y, float color, FFORM form, float zoom)
{
  m_figures[idx].x = x;
  m_figures[idx].y = y;
  m_figures[idx].form_color = makeFormAndColor(form, color);
  m_figures[idx].zoom = zoom;
}

void OMarkFigures::updateFinished()
{
  DrawOverlay::overlayUpdateParameter();
}
