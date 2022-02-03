#ifndef FSHCOLORGENERATOR_H
#define FSHCOLORGENERATOR_H

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsoverlay.h"

class FshColorGenerator
{
  int                                 m_overlay;
  unsigned int                        loc_uniformsCount;
  const dmtype_t*                     loc_uniforms;
private:
  const char*                         m_writebase;
  char* const                         m_to;
  int                                 m_offset;

  int                                 m_paramsctr;
public:
  FshColorGenerator(int overlay, char *deststring, int ovlctr=0);
  FshColorGenerator(const _DrawOverlay::uniforms_t& ufms, int overlay, char *deststring, int ovlctr=0);
  unsigned int  written() const { return m_offset; }
public:
  enum  CGV { CGV_COLORED, CGV_TRACED, CGV_TEXTURED, CGV_EMPTY };
  void  goto_func_begin(CGV cgv);
public:
  void  param_alias(const char *name);
  void  paramarr_alias(const char *name, const char* idxname);
  void  param_get();
public:
  void  mixwell_by_alpha(float alpha);
  void  mixwell_by_traced(const linestyle_t& kls);
public:
  void  color_by_rgb(float r, float g, float b);
  void  color_by_inversive(int idx);
  void  color_by_traced(const linestyle_t& kls);
public:
  void  push(const char* text);
  
public:
  void  goto_func_end();
};

#endif // FSHCOLORGENERATOR_H
