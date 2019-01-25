#ifndef FSHMAINGENERATOR_H
#define FSHMAINGENERATOR_H

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
  int                                 m_offset;
  bool                                m_rotated;
  
  unsigned int                        m_ovlscount;
  const ovlfraginfo_t*                m_ovls;
public:
  static unsigned int basePendingSize(unsigned int ovlscount);
public:
  FshMainGenerator(char* deststring, bool rotated, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]);
  unsigned int  written() const { return m_offset; }
public:
  void  goto_func_begin(const DPostmask &fsp);
  void  goto_func_end(const DPostmask &fsp);
public:
  void  push(const char* text);
public:
  void  cfloatvar(const char* name, float value);
  void  ccolor(const char* name, unsigned int value);
  void  cintvar(const char* name, int value);
};

#endif // FSHMAINGENERATOR_H
