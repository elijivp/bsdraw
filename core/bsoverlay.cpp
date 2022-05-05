/// This file contains useful/technical types, base classes, conversion functions for all types of draws
/// Midway realisations for Overlays
/// If you need to create your own Overlay, use other overlays as examples
/// Created By: Elijah Vlasov
#include "bsoverlay.h"
#include "sheigen/bsshgencolor.h"


int DrawOverlay_ColorForegoing::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin();
  ocg.push("result = in_variant.rgb;"
           "mixwell = in_variant[3];");
  if (m_inversive)
    ocg.invertResult(m_inversive);
  ocg.goto_func_end();
  return ocg.written();
}

void DrawOverlay_ColorDomestic::setColor(color3f_t color, bool update)
{
  m_color = color;
  updateParameter(true, update);
}

int DrawOverlay_ColorDomestic::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin();
  ocg.push("mixwell = in_variant[3];");
  ocg.brushResult(m_color);
  ocg.goto_func_end();
  return ocg.written();
}

int DrawOverlay_ColorTraced::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin();
  {
    ocg.mixwell_by_traced(m_linestyle);
    ocg.brushResult(m_linestyle);
  }
  ocg.goto_func_end();
  return ocg.written();
}

int DrawOverlay_ColorThroughPalette::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(uniforms(), overlay, to, 0);
  ocg.goto_func_begin();
  ocg.push("result = texture("); ocg.param_get(); ocg.push(", vec2(in_variant[0], 0.0)).rgb;");
//  ocg.push("result = mix(result, vec3(0.5,0.5,0.5) - step(0.5, undercolor) + undercolor, in_variant[1]);");
//  ocg.push("result = mix(result, vec3(0.5 - step(0.5, undercolor.r), 0.5 - step(0.5, undercolor.g), 0.5 - step(0.5, undercolor.b)) + undercolor, in_variant[1]);");
  
  ocg.push("vec3 ddss = vec3(abs(undercolor.b - undercolor.g), abs(undercolor.b - undercolor.r), abs(undercolor.g - undercolor.r));");
  ocg.push("vec3 ddssmask = mix(mix( vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0), step(ddss[1], ddss[2]) ), vec3(1.0, 0.0, 0.0), step(ddss[0], ddss[2]));");
  ocg.push("ddss = undercolor*ddssmask;");
  
//  ocg.push("vec3 doubles = vec3(ddss.r + ddss.g + ddss.b, 0.0, 0.0);");
//  ocg.push("doubles[1] = (undercolor.r + undercolor.g + undercolor.b - doubles[0])/2.0;");
//  ocg.push("doubles[2] = step(abs(doubles[0] - doubles[1]), 0.45);");
  ocg.push("ddss = vec3(ddss.r + ddss.g + ddss.b, 0.0, 0.0);");
  ocg.push("ddss[1] = (undercolor.r + undercolor.g + undercolor.b - ddss[0])/2.0;");
  ocg.push("ddss[2] = step(abs(ddss[0] - ddss[1]), 0.45);");
  ocg.push("vec3 inversed = mix(vec3(0.0) + ddssmask*step(ddss[2], 0.45), vec3(1.0) - ddssmask*step(ddss[2], 0.45), step(ddss[1], 0.65) );"); 
  
  ocg.push("result = mix(result, inversed, in_variant[1]);");
  ocg.push("mixwell = in_variant[3];");
  ocg.goto_func_end();
  return ocg.written();
}

