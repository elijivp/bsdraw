/// Overlays:     borders and other types of cover around draw
///   OBorder. View: classical border
///   OBorderSelected. View: border for subdraws, selectable 
///   OToons. View: rounded border
/// Created By: Elijah Vlasov
#include "bsborder.h"
#include "../core/sheigen/bsshgentrace.h"


//class AbstractBorder: virtual public DrawOverlay
//{
//  bool    m_banclicks;
//public:
//  AbstractBorder(bool banclicks): m_banclicks(banclicks){}
//protected:
//  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool* doStop);
//  virtual bool  clickBanned(float x, float y) const =0;
//};


//bool  AbstractBorder::overlayReactionMouse(OVL_REACTION_MOUSE, const void *dataptr, bool* doStop)
//{
//  if (m_banclicks)
//    *doStop = clickBanned(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
//  return m_banclicks;
//}

OBorder::OBorder(unsigned int widthpixels, const linestyle_t &kls, int lineset): DrawOverlayTraced(kls), OVLDimmsOff(),
  m_width(widthpixels), m_lineset(lineset)
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

_OSelected::_OSelected(unsigned int widthpixels, int default_selection, const linestyle_t& kls): DrawOverlayTraced(kls), OVLDimmsOff(),
  m_selected(default_selection), m_width(widthpixels)
{
  appendUniform(DT_1I, &m_selected);
}

void _OSelected::setSelection(int select)
{
  m_selected = select;
  overlayUpdateParameter();
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
    ocg.var_const_fixed("border", (int)m_width);
    ocg.push( "for (int i=0; i<2; i++){"
                "result += (1.0 - step(float(border), float(icoords[i])))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
                "result += step(float(ibounds[i] - border), float(icoords[i]))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
              "}"
              "result[0] = result[0]*step(float(selected), float(icell[0]))*step(float(icell[0]), float(selected));"
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

OToons::OToons(COORDINATION cr, float diameter, float border, const linestyle_t &kls, bool banclikcks): DrawOverlayTraced(kls),
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

bool OToons::overlayReactionMouse(OVL_REACTION_MOUSE, const void*, bool*)
{
//  if (m_banclicks)
//  {
//    float x = ((float*)dataptr)[0] - 0.5f;
//    float y = ((float*)dataptr)[1] - 0.5f;
//    *doStop = x*x + y*y > m_radius2;
//  }
  return m_banclicks;
}


