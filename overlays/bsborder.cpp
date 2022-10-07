/// Overlays:     borders and other types of cover around draw
///   OBorder. View: classical border
///   OBorderSelected. View: border for subdraws, selectable 
///   OToons. View: rounded border
/// Created By: Elijah Vlasov
#include "bsborder.h"
#include "../core/sheigen/bsshgentrace.h"
#include "../core/sheigen/bsshgencolor.h"


OBorder::OBorder(const linestyle_t &kls, int lineset): Ovldraw_ColorTraced(kls), OVLDimmsOff(), m_lineset(lineset)
{
}

int OBorder::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    if (m_lineset & OBLINE_BOTTOM)
      ocg.trace_linehorz_l(nullptr, nullptr, nullptr, nullptr);
    if (m_lineset & OBLINE_LEFT)
      ocg.trace_linevert_b(nullptr, nullptr, nullptr, nullptr);
    ocg.push("inormed = icoords - ov_ibounds + ivec2(1,1);");
    if (m_lineset & OBLINE_TOP)
      ocg.trace_linehorz_r(nullptr, nullptr, nullptr, nullptr);
    if (m_lineset & OBLINE_RIGHT)
      ocg.trace_linevert_t(nullptr, nullptr, nullptr, nullptr);
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

/*******************************************************************************************************************************************************/


OShadow::OShadow(int lineset, unsigned int pxwidth, float curver, const color3f_t &clr): Ovldraw_ColorDomestic(clr), OVLCoordsOff(), OVLDimmsOff(), 
  m_curver(curver), m_lineset(pxwidth == 0? 0 : lineset), m_color(clr)
{
  for (int i=0; i<4; i++)
    m_pxwidth[i] = (int)pxwidth - 1;
}

OShadow::OShadow(int pxwidth_left, int pxwidth_top, int pxwidth_right, int pxwidth_bottom, float curver, const color3f_t& clr): Ovldraw_ColorDomestic(clr), OVLCoordsOff(), OVLDimmsOff(), 
  m_curver(curver), m_color(clr)
{
  m_lineset = 0;
  m_pxwidth[0] = pxwidth_left-1;      if (pxwidth_left > 0)  m_lineset |= OBLINE_LEFT;
  m_pxwidth[1] = pxwidth_top-1;       if (pxwidth_top > 0)  m_lineset |= OBLINE_TOP;
  m_pxwidth[2] = pxwidth_right-1;     if (pxwidth_right > 0)  m_lineset |= OBLINE_RIGHT;
  m_pxwidth[3] = pxwidth_bottom-1;    if (pxwidth_bottom > 0)  m_lineset |= OBLINE_BOTTOM;
}

void OShadow::setCurver(float curver, bool update)
{
  m_curver = curver;
  updateParameter(true, update);
}

int OShadow::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("weight", m_curver);
    
    if (m_lineset)
    {
      ocg.goto_normed();
      
      const char* dds[] = { "inormed.x",
                            "ov_ibounds.y - 1 - inormed.y",
                            "ov_ibounds.x - 1 - inormed.x",
                            "inormed.y",
                          };
      ocg.push("ivec2 dd = ivec2(9999, 0);");
      for (int i=0; i<4; i++)
      {
        if (m_lineset & (1 << i))
        {
          ocg.push("dd = ivec2(mix(dd, ivec2(");
          ocg.push(dds[i]);
          ocg.push(", ");
          ocg.push(m_pxwidth[i]);
          ocg.push("), step(float(");
          ocg.push(dds[i]);
          ocg.push("), float(dd[0]))));");
        }
      }      
//      ocg.push("mixwell = clamp(1.0 + dd[1] - dd[0], 0.0, 1.0 + dd[1])/(1.0 + dd[1]);");
//      ocg.push("mixwell = 1.22 - 1.22 / (1 + 4.5*mixwell);");
//      ocg.push("mixwell = clamp(weight*(mixwell*mixwell*mixwell), 0.0, 1.0)*step(0.0, mixwell);");
      
      ocg.construct_trail_vec2("dd[1]", "weight", "dd[0]", "tms");
      ocg.push("mixwell = tms[1];");
    }
  }  
  ocg.goto_func_end(false);
  return ocg.written();
}

/*******************************************************************************************************************************************************/

_OSelected::_OSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): Ovldraw_ColorTraced(kls), OVLDimmsOff(),
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

int OBorderSelected::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
//    ocg.var_const_fixed("border", (int)m_width);
//    ocg.push( "for (int i=0; i<2; i++){"
//                "result += (1.0 - step(float(border), float(icoords[i])))*insider(icoords[1-i], ivec2(0, ov_ibounds[1-i]));"
//                "result += step(float(ov_ibounds[i] - border), float(icoords[i]))*insider(icoords[1-i], ivec2(0, ov_ibounds[1-i]));"
//              "}"
//              "result[0] = result[0]*step(float(selected), float(ispcell[0]))*step(float(ispcell[0]), float(selected));"
//          );
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
                "result.xy += vec2(step(float(ov_ibounds.y-border-icoords.y),0.0), icoords.x);"
                "result.xy += vec2(step(float(icoords.y-border+1), 0.0), icoords.x);"
                "result.xy = result.xy*step(float(selected - ispcell[0]), 0.0)*step(float(ispcell[0] - selected), 0.0);"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


ORowSelected::ORowSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): _OSelected(widthpixels, default_selection, kls)
{
}

int ORowSelected::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
//                "result.xy += vec2(step(ov_iscaler.x*selected-border-1, icoords.x)*step(icoords.x, ov_iscaler.x*selected-1), icoords.y);"
//                "result.xy += vec2(step(ov_iscaler.x*selected + ov_iscaler.x , icoords.x)*step(icoords.x, ov_iscaler.x*selected + ov_iscaler.x + border ), icoords.y);"
//            "result.xy += vec2(step(float(ov_iscaler.x*selected-border), float(icoords.x))*step(float(icoords.x), float(ov_iscaler.x*selected)), icoords.y);"
//            "result.xy += vec2(step(float(ov_iscaler.x*selected+ov_iscaler.x-1), float(icoords.x))*step(float(icoords.x), float(ov_iscaler.x*selected+ov_iscaler.x+border-1)), icoords.y);"
          "result.xy += vec2(step(float(ov_iscaler.x*selected-border-icoords.x), 0.0)*step(float(icoords.x-ov_iscaler.x*selected), 0.0), icoords.y);"
          "result.xy += vec2(step(float(ov_iscaler.x*selected+ov_iscaler.x-1-icoords.x), 0.0)*step(float(icoords.x-ov_iscaler.x*selected-ov_iscaler.x-border+1), 0.0), icoords.y);"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


OColumnSelected::OColumnSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): _OSelected(widthpixels, default_selection, kls)
{
}

int OColumnSelected::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.param_alias("selected");
    ocg.var_fixed("border", (int)m_width);
    ocg.push("border = int(border/2.0 + 0.5);");
    ocg.push( 
//                "result.xy += vec2(step(ov_iscaler.y*selected-border-1, icoords.y)*step(icoords.y, ov_iscaler.y*selected-1), icoords.x);"
//                "result.xy += vec2(step(ov_iscaler.y*selected + ov_iscaler.y , icoords.y)*step(icoords.y, ov_iscaler.y*selected + ov_iscaler.y + border ), icoords.x);"
//          "result.xy += vec2(step(ov_iscaler.y*selected-border, icoords.y)*step(icoords.y, ov_iscaler.y*selected), icoords.x);"
//          "result.xy += vec2(step(ov_iscaler.y*selected+ov_iscaler.y-1, icoords.y)*step(icoords.y, ov_iscaler.y*selected+ov_iscaler.y+border-1), icoords.x);"
          "result.xy += vec2(step(float(ov_iscaler.y*selected-border-icoords.y),0.0)*step(float(icoords.y-ov_iscaler.y*selected), 0.0), icoords.x);"
          "result.xy += vec2(step(float(ov_iscaler.y*selected+ov_iscaler.y-1-icoords.y),0.0)*step(float(icoords.y-ov_iscaler.y*selected-ov_iscaler.y-border+1), 0.0), icoords.x);"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}

OColumnsSelected::OColumnsSelected(unsigned int widthpixels, int default_selection, int addLeft, int addRight, bool _cyclic, const linestyle_t& kls): 
  _OSelected(widthpixels, default_selection, kls), al(addLeft), ar(addRight), cyclic(_cyclic)
{
}

int OColumnsSelected::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor ocg(this->uniforms(), overlay, to);
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
      ocg.push("pos_l = pos_l + int(1.0 - step(0.0, float(pos_l)))*ov_indimms.y;");
      ocg.push("pos_r = int(-1.0*step(float(ov_indimms.y), float(pos_r)))*ov_indimms.y + pos_r;");
    }
    ocg.push( 
              "result.xy += vec2(step(float(ov_iscaler.y*pos_l-border-1-icoords.y),0.0)*step(float(icoords.y-ov_iscaler.y*pos_l+1), 0.0), icoords.x);"
              "result.xy += vec2(step(float(ov_iscaler.y*pos_r+1-icoords.y), 0.0)*step(float(icoords.y-ov_iscaler.y*pos_r - border - 1), 0.0), icoords.x);"
            );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}



/*******************************************************************************************************************************************************/

OToons::OToons(COORDINATION cr, float diameter, float border, const linestyle_t &kls, bool banclikcks): Ovldraw_ColorTraced(kls),
  OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f),
  OVLDimms1Static(cr, diameter/2.0f),
  m_radius2(diameter*diameter/4.0f), m_border(border), m_banclicks(banclikcks)
{
}

int OToons::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push( "int diameter = idimms1;" );
    ocg.var_const_fixed("border", m_border);
    
    ocg.push( "float d2 = inormed.x*inormed.x + inormed.y*inormed.y;"
              "vec3  r2 = vec3((diameter - border) * (diameter - border), diameter*diameter, (diameter + border) * (diameter + border));"
              "float mixwell_before = smoothstep(r2[0], r2[1], d2)*(1 - step(r2[1], d2));"
              "float mixwell_aftere = (step(r2[1], d2));"
              "float fmax = max(ov_ibounds.x, ov_ibounds.y);"
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


