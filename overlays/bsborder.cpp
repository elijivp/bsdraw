#include "bsborder.h"
#include "../core/sheigen/bsshgentrace.h"

bool  AbstractBorder::overlayReaction(OVL_REACTION, const void *dataptr, bool* doStop)
{
  if (m_banclicks)
    *doStop = clickBanned(((float*)dataptr)[0], ((float*)dataptr)[1]);
  return m_banclicks;
}



OBorder::OBorder(unsigned int widthpixels, const linestyle_t &kls): IOverlayTraced(kls), OVLDimmsOff(),
  m_width(widthpixels)
{
}

int OBorder::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator ocg(this->uniforms(), overlay, to);
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


/*******************************************************************************************************************************************************/

OToons::OToons(COORDINATION cr, float diameter, float border, const linestyle_t &kls, bool banclikcks): IOverlayTraced(kls),
  OVLCoordsStatic(CR_RELATIVE, 0.5f, 0.5f),
  OVLDimms1Static(cr, diameter/2.0f),
  m_radius2(diameter*diameter/4.0f), m_border(border), m_banclicks(banclikcks)
{
}

int OToons::fshTrace(int overlay, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this, false);
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

bool OToons::overlayReaction(OVL_REACTION, const void*, bool*)
{
//  if (m_banclicks)
//  {
//    float x = ((float*)dataptr)[0] - 0.5f;
//    float y = ((float*)dataptr)[1] - 0.5f;
//    *doStop = x*x + y*y > m_radius2;
//  }
  return m_banclicks;
}
