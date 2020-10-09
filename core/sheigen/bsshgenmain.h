#ifndef FSHMAINGENERATOR_H
#define FSHMAINGENERATOR_H

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsidrawcore.h"

class VshMainGenerator1D
{
public:
  static unsigned int pendingSize();
  unsigned int operator()(char* to);
};

class VshMainGenerator2D
{
public:
  static unsigned int pendingSize();
  unsigned int operator()(char* to);
};

class FshMainGenerator
{
  const char*                         m_writebase;
  char* const                         m_to;
  unsigned int                        m_allocatedPortions;
  int                                 m_offset;
  ORIENTATION                         m_orient;
  SPLITPORTIONS                       m_splitPortions;
  
  unsigned int                        m_ovlscount;
  const ovlfraginfo_t*                m_ovls;
public:
  static unsigned int basePendingSize(unsigned int ovlscount);
public:
  FshMainGenerator(char* deststring, unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]);
  unsigned int  written() const { return (unsigned int)m_offset; }
public:
  enum  { 
    INITBACK_BYZERO,
    INITBACK_BYVALUE,
    INITBACK_BYPALETTE,
    INIT_BYZERO,
    INIT_BYVALUE,
    INIT_BYPALETTE
        };
  void  goto_func_begin(int initback, unsigned int backcolor, const DPostmask &fsp); /// initresult: 0-none, 1-by zero, 2-by backcolor
  void  goto_func_end(const DPostmask &fsp);
public:
  void  push(const char* text);
public:
  void  cfloatvar(const char* name, float value);
  void  ccolor(const char* name, unsigned int value);
  void  cintvar(const char* name, int value);
};

#endif // FSHMAINGENERATOR_H
