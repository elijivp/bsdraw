/// This file contains useful/technical types, base classes, conversion functions for all types of draws
/// Midway realisations for Overlays
/// If you need to create your own Overlay, use other overlays as examples
/// Created By: Elijah Vlasov
#include "bsoverlay.h"
#include "sheigen/bsshgencolor.h"

/*
  if (cgv == CGV_EMPTY)
                        , m_overlay);
  else if (cgv == CGV_COLORED)
    m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) {" SHNL
                                            "vec3 result = overcolor.rgb;" SHNL
                                            "float mixwell = overcolor[3];" SHNL
                        , m_overlay);
  else if (cgv == CGV_TRACED)
    m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec4 in_variant, in vec3 undercolor) {" SHNL
                                            "vec3 result;" SHNL
                                            "float mixwell = 0.0;" SHNL
                        , m_overlay);
  else if (cgv == CGV_TEXTURED)
  {
    m_offset += msprintf(&m_to[m_offset],   "uniform highp sampler2D opm%D_%D;" SHNL
                                            "vec3 overlayColor%d(in vec4 txtm, in vec3 undercolor) {" SHNL
                                            "vec3 result = texture(opm%D_%D, vec2(txtm[0], 0.0)).rgb;" SHNL
                                            "float mixwell = txtm[3];" SHNL
                        , m_overlay, m_paramsctr
                        , m_overlay
                        , m_overlay, m_paramsctr
                        );
    m_paramsctr++;
  }
*/

int DrawOverlay_ColorForegoing::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin();
  ocg.push("result = in_variant.rgb;"
           "mixwell = in_variant.a;");
  if (m_inversive)
    ocg.invertResult(m_inversive);
  ocg.goto_func_end();
  return ocg.written();
}

int DrawOverlay_ColorDomestic::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin();
  ocg.push("mixwell = in_variant.a;");
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
  ocg.push("result = texture("); ocg.param_get(); ocg.push(", vec2(in_variant[0], 0.0)).rgb;"
           "mixwell = in_variant[3];"
    );
  ocg.goto_func_end();
  return ocg.written();
}

