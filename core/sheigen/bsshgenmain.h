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
  SPLITPORTIONS                       m_splitPortions;
  
  impulsedata_t                       m_impulsegen;
  
  unsigned int                        m_ovlscount;
  const ovlfraginfo_t*                m_ovls;
  
  enum  DATAMAPPING { DM_OFF, DM_ON  }  m_datamapped;
public:
  static unsigned int basePendingSize(const impulsedata_t& imp, unsigned int ovlscount);
public:
  FshMainGenerator(char* deststring, unsigned int allocatedPortions, SPLITPORTIONS splitPortions, const impulsedata_t& imp, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]);
  unsigned int  written() const { return (unsigned int)m_offset; }
private:
  void  _main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask &fsp); /// initresult: 0-none, 1-by zero, 2-by backcolor
public:
  enum  { 
    INITBACK_BYZERO,
    INITBACK_BYVALUE,
    INITBACK_BYPALETTE,
    INIT_BYZERO,
    INIT_BYVALUE,
    INIT_BYPALETTE
        };
  void  main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask &fsp); /// initresult: 0-none, 1-by zero, 2-by backcolor
  void  main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask &fsp, unsigned int dboundsA, unsigned int dboundsB); /// initresult: 0-none, 1-by zero, 2-by backcolor
  void  main_end(const DPostmask &fsp);
public:
  void  push(const char* text);
public:
  void  ccolor(const char* name, unsigned int value);
  void  cfloatvar(const char* name, float value);
  void  cfloatvar(const char* name, float value1, float value2);
  void  cintvar(const char* name, int value);
  void  cintvar(const char* name, int value1, int value2);
public:
  void  value2D(const char* varname, const char* coordsname="relcoords", const char* portionname="i");
private:
};

#endif // FSHMAINGENERATOR_H
