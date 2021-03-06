/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgencolor.h"

#include <memory.h>
#include <stdio.h>
#include "bsshgenparams.h"

FshColorGenerator::FshColorGenerator(int overlay, char* deststring, int ovlctr): m_overlay(overlay), m_writebase(deststring), m_to(deststring), m_offset(0), m_paramsctr(ovlctr)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  loc_uniformsCount = 0;
  loc_uniforms = nullptr;
}

FshColorGenerator::FshColorGenerator(const AbstractDrawOverlay::uniforms_t& ufms, int overlay, char* deststring, int ovlctr): m_overlay(overlay), m_writebase(deststring), m_to(deststring), m_offset(0), m_paramsctr(ovlctr)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  loc_uniformsCount = ufms.count;
  loc_uniforms = ufms.arr;
  
  m_offset += msexpandParams(&m_to[m_offset], m_overlay, loc_uniformsCount, loc_uniforms);
}

void FshColorGenerator::goto_func_begin(CGV cgv)
{
  if (cgv == CGV_EMPTY)
    m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) {" SHNL
                                            "vec3 result;" SHNL
                                            "float mixwell = 0.0;" SHNL
                        , m_overlay);
  else if (cgv == CGV_COLORED)
    m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec4 overcolor, in vec3 undercolor) {" SHNL
                                            "vec3 result = overcolor.rgb;" SHNL
                                            "float mixwell = overcolor[3];" SHNL
                        , m_overlay);
  else if (cgv == CGV_TRACED)
    m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec4 trace_on_pix, in vec3 undercolor) {" SHNL
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
}

void FshColorGenerator::param_alias(const char *name)
{
//  Q_ASSERT(loc_uniforms != nullptr);
  m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D;" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshColorGenerator::paramarr_alias(const char* name, const char* idxname)
{
  m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D[%s];" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr, idxname);
  m_paramsctr++;
}

void FshColorGenerator::param_get()
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshColorGenerator::mixwell_by_alpha(float alpha)
{
  m_offset += msprintf(&m_to[m_offset], "mixwell = %F;" SHNL, alpha);
}

void FshColorGenerator::mixwell_by_traced(const linestyle_t &kls)
{
  /// 1. Mixwell
  {
    const char* decl_mixwell[] =          {   "mixwell = step(1.0, float(trace_on_pix[0]));" SHNL, 
                                              "mixwell = 1.0 - step(float(trace_on_pix[0]), 0.0);" SHNL,
                                              "mixwell = trace_on_pix[0];" SHNL,
                                              "mixwell = (1 - trace_on_pix[0]*trace_on_pix[0]);" SHNL,
                                              "mixwell = trace_on_pix[0];" SHNL,
                                                       };
    m_offset += msprintf(&m_to[m_offset], "%s", decl_mixwell[kls.outside]);
  }
  /// 2. Trace
  {
    if (kls.lenspace == 0)
      ;
    else if (kls.lenstroke == 0)
      m_offset += msprintf(&m_to[m_offset], "const int lenspace = %d;" SHNL
                                            "const int countdot = %d;" SHNL
//                                            "int rounded = int(floor(trace_on_pix[1] * trace_on_pix[2] + 0.49));" SHNL
                                            "int pos = int(mod(trace_on_pix[1], float(2*countdot - 1 + lenspace)));" SHNL
                                            "int mixdot = int((1.0 - step(2.0*countdot, float(pos)))*step(1.0, mod(pos - (2.0*countdot-1.0), 2.0)));" SHNL
                                            "mixwell = mixwell*mixdot;" SHNL,
                          kls.lenspace, kls.countdot);
    else
      m_offset += msprintf(&m_to[m_offset], "const int lenstroke = %d;" SHNL
                                            "const int lenspace = %d;" SHNL
                                            "const int countdot = %d;" SHNL
//                                            "int rounded = int(floor(trace_on_pix[1] * trace_on_pix[2]));" SHNL
                                            "int pos = int(mod(trace_on_pix[1], lenstroke + lenspace + (1 + lenspace) * countdot));" SHNL
                                            "int mixban = int(step(float(lenstroke), float(pos))*step(1.0, mod(pos - lenstroke + 1, lenspace + 1)));" SHNL
                                            "mixwell = mixwell*(1 - mixban);" SHNL, 
                          kls.lenstroke, kls.lenspace, kls.countdot);
  }
}

void FshColorGenerator::color_by_rgb(float r, float g, float b)
{
  m_offset += msprintf(&m_to[m_offset], "result = vec3(%F,%F,%F);", r, g, b);
}

void FshColorGenerator::color_by_inversive(int idx)
{
  static const char* decl_inversive[] = {     "result = 1.0 - undercolor;" SHNL,
                                              "result = step(0.5, undercolor); result = (vec3(0.5,0.5,0.5) - result) + undercolor;" SHNL,
                                              "result.x = step(0.5, (undercolor.r + undercolor.g + undercolor.b)/3.0); result = (1.0 - result.x) * vec3(1.0,1.0,1.0);" SHNL, 
                                              "result = vec3(1.0 - undercolor.g, 1.0 - undercolor.b, 1.0 - undercolor.r);" SHNL, 
                                              "result = step(0.5, undercolor); result = (vec3(0.5,0.5,0.5) - result) + vec3(1.0 - undercolor.g, 1.0 - undercolor.b, 1.0 - undercolor.r);" SHNL,
                                        };
  
  const int limidx = sizeof(decl_inversive)/sizeof(const char*);
  if (idx <= 0 || idx > limidx)
    m_offset += msprintf(&m_to[m_offset], "result = undercolor;");  // fake eba
  else
    m_offset += msprintf(&m_to[m_offset], "%s", decl_inversive[idx-1]);
}

void FshColorGenerator::color_by_traced(const linestyle_t &kls)
{
  if (kls.inversive == 0)
    color_by_rgb(kls.r, kls.g, kls.b);
  else
    color_by_inversive(kls.inversive);
}

void FshColorGenerator::push(const char *text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
}

void FshColorGenerator::goto_func_end()
{
  m_offset += msprintf(&m_to[m_offset],  "return mix(undercolor, result, mixwell);" SHNL
                                        "}" SHNL);
}

