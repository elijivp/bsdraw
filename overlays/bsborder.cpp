/// Overlays:     borders and other types of cover around draw
///   OBorder. View: classical border
///   OBorderSelected. View: border for subdraws, selectable 
///   OToons. View: rounded border
/// Created By: Elijah Vlasov
#include "bsborder.h"
#include "../core/sheigen/bsshgentrace.h"
#include "../core/sheigen/bsshgencolor.h"


OBorder::OBorder(const linestyle_t &kls, int lineset): DrawOverlay_ColorTraced(kls), OVLDimmsOff(), m_lineset(lineset)
{
}

int OBorder::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    if (m_lineset & OBLINE_BOTTOM)
      ocg.trace_linehorz_l(nullptr, nullptr, nullptr, nullptr);
    if (m_lineset & OBLINE_LEFT)
      ocg.trace_linevert_b(nullptr, nullptr, nullptr, nullptr);
    ocg.push("inormed = icoords - ibounds + ivec2(1,1);");
    if (m_lineset & OBLINE_TOP)
      ocg.trace_linehorz_r(nullptr, nullptr, nullptr, nullptr);
    if (m_lineset & OBLINE_RIGHT)
      ocg.trace_linevert_t(nullptr, nullptr, nullptr, nullptr);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/*******************************************************************************************************************************************************/


OShadow::OShadow(int lineset, unsigned int pxwidth, float weight, const color3f_t &clr): DrawOverlay_ColorDomestic(clr), OVLCoordsOff(), OVLDimmsOff(), 
  m_weight(weight), m_lineset(pxwidth == 0? 0 : lineset), m_color(clr)
{
  for (int i=0; i<4; i++)
    m_pxwidth[i] = (int)pxwidth - 1;
}

OShadow::OShadow(int pxwidth_left, int pxwidth_top, int pxwidth_right, int pxwidth_bottom, float weight, const color3f_t& clr): DrawOverlay_ColorDomestic(clr), OVLCoordsOff(), OVLDimmsOff(), 
  m_weight(weight), m_color(clr)
{
  m_lineset = 0;
  m_pxwidth[0] = pxwidth_left;      if (pxwidth_left > 0)  m_lineset |= OBLINE_LEFT;
  m_pxwidth[1] = pxwidth_top;       if (pxwidth_top > 0)  m_lineset |= OBLINE_TOP;
  m_pxwidth[2] = pxwidth_right;     if (pxwidth_right > 0)  m_lineset |= OBLINE_RIGHT;
  m_pxwidth[3] = pxwidth_bottom;    if (pxwidth_bottom > 0)  m_lineset |= OBLINE_BOTTOM;
}

int OShadow::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("weight", m_weight);
    
    if (m_lineset)
    {
      ocg.goto_normed();
      
      const char* dds[] = { "inormed.x",
                            "ibounds.y - 1 - inormed.y",
                            "ibounds.x - 1 - inormed.x",
                            "inormed.y",
                          };
      ocg.push("ivec2 dd = ivec2(9999, 0);");
      for (int i=0; i<4; i++)
      {
        if (m_lineset & (1 << i))
        {
          ocg.push("dd = (ivec2)mix(dd, ivec2(");
          ocg.push(dds[i]);
          ocg.push(", ");
          ocg.push(m_pxwidth[i]);
          ocg.push("), step(");
          ocg.push(dds[i]);
          ocg.push(", dd[0]));");
        }
      }
  //    ocg.push("int dd = inormed.x")
      ocg.push("mixwell = clamp(1.0 + dd[1] - dd[0], 0.0, 1.0 + dd[1])/(1.0 + dd[1]);");
      ocg.push("mixwell = 1.22 - 1.22 / (1 + 4.5*mixwell);");
      ocg.push("mixwell = clamp(weight*(mixwell*mixwell*mixwell), 0.0, 1.0)*step(0.0, mixwell);");
    }
  }  
  ocg.goto_func_end(false);
  return ocg.written();
}

/*******************************************************************************************************************************************************/

_OSelected::_OSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): DrawOverlay_ColorTraced(kls), OVLDimmsOff(),
  m_selected(default_selection), m_width(widthpixels)
{
  appendUniform(DT_1I, &m_selected);
}

void _OSelected::setSelection(int select, bool update)
{
  m_selected = select;
  updateParameter(false, update);
}


OBorderSelected::OBorderSelected(unsigned int widthpixels, int default_selection, const linestyle_t &kls): _OSelected(widthpixels, default_selection, kls)
{
}

int OBorderSelected::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
//    ocg.var_const_fixed("border", (int)m_width);
//    ocg.push( "for (int i=0; i<2; i++){"
//                "result += (1.0 - step(float(border), float(icoords[i])))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
//                "result += step(float(ibounds[i] - border), float(icoords[i]))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
//              "}"
//              "result[0] = result[0]*step(float(selected), float(ispcell[0]))*step(float(ispcell[0]), float(selected));"
//          );
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
                "result.xy += vec2(step(ibounds.y-border, icoords.y), icoords.x);"
                "result.xy += vec2(step(icoords.y, border - 1), icoords.x);"
                "result.xy = result.xy*step(float(selected), float(ispcell[0]))*step(float(ispcell[0]), float(selected));"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


ORowSelected::ORowSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): _OSelected(widthpixels, default_selection, kls)
{
}

int ORowSelected::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
                "result.xy += vec2(step(iscaling.x*selected-border-1, icoords.x)*step(icoords.x, iscaling.x*selected-1), icoords.y);"
                "result.xy += vec2(step(iscaling.x*selected + iscaling.x , icoords.x)*step(icoords.x, iscaling.x*selected + iscaling.x + border ), icoords.y);"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


OColumnSelected::OColumnSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): _OSelected(widthpixels, default_selection, kls)
{
}

int OColumnSelected::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
                "result.xy += vec2(step(iscaling.y*selected-border-1, icoords.y)*step(icoords.y, iscaling.y*selected-1), icoords.x);"
                "result.xy += vec2(step(iscaling.y*selected + iscaling.y , icoords.y)*step(icoords.y, iscaling.y*selected + iscaling.y + border ), icoords.x);"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

OColumnsSelected::OColumnsSelected(unsigned int widthpixels, int default_selection, int addLeft, int addRight, bool _cyclic, const linestyle_t& kls): 
  _OSelected(widthpixels, default_selection, kls), al(addLeft), ar(addRight), cyclic(_cyclic)
{
}

int OColumnsSelected::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
    ocg.var_fixed("border", (int)m_width);
    ocg.var_const_fixed("ad", al, ar);
    ocg.push( "border = int(border/2.0 + 0.5);");
    ocg.push( "int pos_l = (selected - ad[0]);"
              "int pos_r = (selected + 1 + ad[1]);"
              );
    if (cyclic)
    {
      ocg.push("pos_l = pos_l + int(1.0 - step(0.0, float(pos_l)))*viewdimm_b;");
      ocg.push("pos_r = int(-1.0*step(float(viewdimm_b), float(pos_r)))*viewdimm_b + pos_r;");
    }
    ocg.push( 
              "result.xy += vec2(step(iscaling.y*pos_l-border-1, icoords.y)*step(icoords.y, iscaling.y*pos_l-1), icoords.x);"
              "result.xy += vec2(step(iscaling.y*pos_r + 1, icoords.y)*step(icoords.y, iscaling.y*pos_r + border + 1), icoords.x);"
            );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}



/*******************************************************************************************************************************************************/

OToons::OToons(COORDINATION cr, float diameter, float border, const linestyle_t &kls, bool banclikcks): DrawOverlay_ColorTraced(kls),
  OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f),
  OVLDimms1Static(cr, diameter/2.0f),
  m_radius2(diameter*diameter/4.0f), m_border(border), m_banclicks(banclikcks)
{
}

int OToons::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push( "int diameter = idimms1;" );
    ocg.var_const_fixed("border", m_border);
    
    ocg.push( "float d2 = inormed.x*inormed.x + inormed.y*inormed.y;" //floor(dot(inormed, inormed) + 0.49);" // TODO: test
              "vec3  r2 = vec3((diameter - border) * (diameter - border), diameter*diameter, (diameter + border) * (diameter + border));"
              "float mixwell_before = smoothstep(r2[0], r2[1], d2)*(1 - step(r2[1], d2));"
              "float mixwell_aftere = (step(r2[1], d2));"
              "float fmax = max(ibounds.x, ibounds.y);"
              "result += vec3(clamp(mixwell_before + mixwell_aftere, 0.0, 1.0), (sqrt(d2) - diameter)/(fmax - diameter), fmax - diameter);"
              );
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

//bool  OToons::clickBanned(float x, float y) const
//{
//  return false;
//}

//bool OToons::overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*)
//{
////  if (m_banclicks)
////  {
////    float x = ((float*)dataptr)[0] - 0.5f;
////    float y = ((float*)dataptr)[1] - 0.5f;
////    *doStop = x*x + y*y > m_radius2;
////  }
//  return m_banclicks;
//}


