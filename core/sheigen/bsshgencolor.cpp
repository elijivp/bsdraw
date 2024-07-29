/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgencolor.h"

#include <memory.h>
#include <stdio.h>
#include "bsshgenparams.h"

FshOVColorConstructor::FshOVColorConstructor(int overlay, char* deststring, int ovlctr): m_overlay(overlay), m_writebase(deststring), m_to(deststring), m_offset(0), m_paramsctr(ovlctr)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  loc_uniformsCount = 0;
  loc_uniforms = nullptr;
}

FshOVColorConstructor::FshOVColorConstructor(const _Ovldraw::uniforms_t& ufms, int overlay, char* deststring, int ovlctr): m_overlay(overlay), m_writebase(deststring), m_to(deststring), m_offset(0), m_paramsctr(ovlctr)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  loc_uniformsCount = ufms.count;
  loc_uniforms = ufms.arr;
  
  m_offset += msexpandParams(&m_to[m_offset], m_overlay, loc_uniformsCount, loc_uniforms);
}

void FshOVColorConstructor::goto_func_begin()
{
  m_offset += msprintf(&m_to[m_offset],   "vec3 overlayColor%d(in vec3 color_drawed, in vec3 color_ovled, in vec4 in_variant) " SHNL
                                          "{" SHNL
                                            "vec3 result;" SHNL
                                            "float mixwell = 0.0;" SHNL,
                       m_overlay
                       );
}

void FshOVColorConstructor::param_alias(const char *name)
{
//  Q_ASSERT(loc_uniforms != nullptr);
  m_offset += msprintf(&m_to[m_offset], "%s %s = ovlprm%d_%d;" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshOVColorConstructor::paramarr_alias(const char* name, const char* idxname)
{
  m_offset += msprintf(&m_to[m_offset], "%s %s = ovlprm%d_%d[%s];" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr, idxname);
  m_paramsctr++;
}

void FshOVColorConstructor::param_get()
{
  m_offset += msprintf(&m_to[m_offset], "ovlprm%d_%d", m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshOVColorConstructor::mixwell_by_alpha(float alpha)
{
  m_offset += msprintf(&m_to[m_offset], "mixwell = %F;" SHNL, alpha);
}

void FshOVColorConstructor::mixwell_by_traced(const linestyle_t &kls)
{
  /// 1. Mixwell
  {
    const char* decl_mixwell[] =          {   "mixwell = step(1.0, float(in_variant[0]));" SHNL, 
                                              "mixwell = 1.0 - step(float(in_variant[0]), 0.0);" SHNL,
                                              "mixwell = in_variant[0];" SHNL,
                                              "mixwell = (1 - in_variant[0]*in_variant[0]);" SHNL,
                                              "mixwell = in_variant[0];" SHNL,
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
                                            "int pos = int(mod(in_variant[1], float(2*countdot - 1 + lenspace)));" SHNL
                                            "int mixdot = int((1.0 - step(2.0*countdot, float(pos)))*step(1.0, mod(pos - (2.0*countdot-1.0), 2.0)));" SHNL
                                            "mixwell = mixwell*mixdot;" SHNL,
                          kls.lenspace, kls.countdot);
    else
      m_offset += msprintf(&m_to[m_offset], "const int lenstroke = %d;" SHNL
                                            "const int lenspace = %d;" SHNL
                                            "const int countdot = %d;" SHNL
                                            "int pos = int(mod(in_variant[1], lenstroke + lenspace + (1 + lenspace) * countdot));" SHNL
                                            "int mixban = int(step(float(lenstroke), float(pos))*step(1.0, mod(pos - lenstroke + 1, lenspace + 1)));" SHNL
                                            "mixwell = mixwell*(1 - mixban);" SHNL, 
                          kls.lenstroke, kls.lenspace, kls.countdot);
  }
}

void FshOVColorConstructor::brushResult(const color3f_t& clr)
{
  m_offset += msprintf(&m_to[m_offset], "result = vec3(%F,%F,%F);", clr.r, clr.g, clr.b);
}

void FshOVColorConstructor::brushResult(float r, float g, float b)
{
  m_offset += msprintf(&m_to[m_offset], "result = vec3(%F,%F,%F);", r, g, b);
}

void FshOVColorConstructor::brushResult(const linestyle_t &kls)
{
  if (kls.inversive == 0)
    brushResult(kls.r, kls.g, kls.b);
  else
    invertResult(kls.inversive);
}

void FshOVColorConstructor::invertResult(int idx)
{
  static const char* decl_inversive[] = {     "result = 1.0 - color_ovled;" SHNL,
                                              "result = step(0.5, color_ovled); result = (vec3(0.5,0.5,0.5) - result) + color_ovled;" SHNL,
                                              "result.x = step(0.5, (color_ovled.r + color_ovled.g + color_ovled.b)/3.0); result = (1.0 - result.x) * vec3(1.0,1.0,1.0);" SHNL, 
                                              "result = vec3(1.0 - color_ovled.g, 1.0 - color_ovled.b, 1.0 - color_ovled.r);" SHNL, 
                                              "result = step(0.5, color_ovled); result = (vec3(0.5,0.5,0.5) - result) + vec3(1.0 - color_ovled.g, 1.0 - color_ovled.b, 1.0 - color_ovled.r);" SHNL,
                                        };
  
  const int limidx = sizeof(decl_inversive)/sizeof(const char*);
  if (idx <= 0 || idx > limidx)
    m_offset += msprintf(&m_to[m_offset], "result = color_ovled;");  // fake eba
  else
    m_offset += msprintf(&m_to[m_offset], "%s", decl_inversive[idx-1]);
}

void FshOVColorConstructor::push(const char *text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
}

void FshOVColorConstructor::goto_func_end()
{
  m_offset += msprintf(&m_to[m_offset],   "return mix(color_ovled, result, mixwell);" SHNL
                                        "}" SHNL);
}

