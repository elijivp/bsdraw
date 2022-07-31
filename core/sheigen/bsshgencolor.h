#ifndef FSHOVCOLORCONSTRUCTOR_H
#define FSHOVCOLORCONSTRUCTOR_H

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsoverlay.h"

class FshOVColorConstructor
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
  FshOVColorConstructor(int overlay, char *deststring, int ovlctr=0);
  FshOVColorConstructor(const _Ovldraw::uniforms_t& ufms, int overlay, char *deststring, int ovlctr=0);
  unsigned int  written() const { return (unsigned int)m_offset; }
public:
  void  goto_func_begin();
public:
  void  param_alias(const char *name);
  void  paramarr_alias(const char *name, const char* idxname);
  void  param_get();
public:
  void  mixwell_by_alpha(float alpha);
  void  mixwell_by_traced(const linestyle_t& kls);
public:
  void  brushResult(const color3f_t& clr);
  void  brushResult(float r, float g, float b);
  void  brushResult(const linestyle_t& kls);
  void  invertResult(int idx);
public:
  void  push(const char* text);
  
public:
  void  goto_func_end();
};

#endif // FSHOVCOLORCONSTRUCTOR_H
