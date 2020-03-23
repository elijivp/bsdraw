#ifndef FSHCOLORGENERATOR_H
#define FSHCOLORGENERATOR_H

#include "../bsoverlay.h"

class FshColorGenerator
{
  int                                 m_overlay;
private:
  const char*                         m_writebase;
  char* const                         m_to;
  int                                 m_offset;

  int                                 m_paramsctr;
public:
  FshColorGenerator(int overlay, char *deststring, int ovlctr=0);
  unsigned int  written() const { return m_offset; }
public:
  enum  CGV { CGV_COLORED, CGV_TRACED, CGV_TEXTURED, CGV_EMPTY };
  void  goto_func_begin(CGV cgv);
public:
  
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
