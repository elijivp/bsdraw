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

OBorder::OBorder(unsigned int widthpixels, const linestyle_t &kls): DrawOverlayTraced(kls), OVLDimmsOff(),
  m_width(widthpixels)
{
}

int OBorder::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed_empty();
    ocg.var_const_fixed("border", (int)m_width);
    ocg.push( "for (int i=0; i<2; i++){"
                "result += (1.0 - step(float(border), float(icoords[i])))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
                "result += step(float(ibounds[i] - border), float(icoords[i]))*insider(icoords[1-i], ivec2(0, ibounds[1-i]));"
              "}"
          );
  }  
  ocg.goto_func_end(true);
  return ocg.written();
}


OBorderSelected::OBorderSelected(unsigned int widthpixels, int default_selection, const linestyle_t &kls): DrawOverlayTraced(kls), OVLDimmsOff(),
  m_selected(default_selection), m_width(widthpixels)
{
  appendUniform(DT_1I, &m_selected);
}

void OBorderSelected::setSelection(int select)
{
  m_selected = select;
  overlayUpdateParameter();
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
