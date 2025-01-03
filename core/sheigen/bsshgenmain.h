#ifndef FSHDRAWCONSTRUCTOR_H
#define FSHDRAWCONSTRUCTOR_H

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsidrawcore.h"

#if !defined SHNL && !defined NO_USE_SHNL
#define SHNL "\n"
#endif

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

//struct globvarinfo_t
//{
//  DTYPE           type;
//  const char*     name;
//};

class FshDrawComposer
{
protected:
  const char*                         m_writebase;
  char* const                         m_to;
  int                                 m_offset;
  
  SPLITPORTIONS                       m_split;
  datasubmesh_t                       m_datasubmesh;
  
  bool                                m_datamapped;
protected:
  FshDrawComposer(char* deststring, SPLITPORTIONS splits, const datasubmesh_t& imp);
public:
  SPLITPORTIONS splits() const { return m_split;  }
  unsigned int  written() const { return (unsigned int)m_offset; }
  const char*   code() const { return m_writebase; }
public:
  void  push(const char* text);
  void  pushin(const char* text); // guaranteed no SHNL at the end
public:
  void  ccolor(const char* name, unsigned int value);
  void  cfloatvar(const char* name, float value);
  void  cfloatvar(const char* name, float value1, float value2);
  void  cintvar(const char* name, int value);
  void  cintvar(const char* name, int value1, int value2);
public:
  void  value2D(const char* varname, const char* coordsname="abc_coords", const char* portionname="i");
};

class FshDrawMain: public FshDrawComposer
{
public:
  static unsigned int basePendingSize(const datasubmesh_t& imp, unsigned int ovlscount);
public:
  FshDrawMain(char* deststring, SPLITPORTIONS splits, const datasubmesh_t& imp);
  
  void          generic_decls_begin();
  void          generic_decls_add(DTYPE, const char* name);
  void          generic_decls_add(DTYPE, const char* name, int count);
  void          generic_decls_add_tft_area(int texid, char* result);
  void          generic_decls_add_tft_dslots(int texid, char* result);
  void          generic_decls_add_tft_tslots(int texid, char* resulttex, char* resultcount);
  void          generic_decls_add_ovl_input(int ovlid, char* result);
  void          generic_decls_add_ovl_nameonly(int ovlid, int ovlparamid, DTYPE type, char* result);
  
  void          generic_main_begin(int allocatedPortions, ORIENTATION orient, unsigned int emptycolor, const overpattern_t& fsp);
  
  void          generic_main_prepare_tft();
  enum GROUPING  { GP_OFF, GP_ON, GP_ONCLOSE };
  void          generic_main_process_tft_sd(bool isstatic, const tftfraginfo_t& tft, GROUPING);
  void          generic_main_process_tft_t(const tftbookinfo_t& tbi, unsigned int texlen, COORDINATION unicr, float unirotate);
  
  void          generic_main_process_fsp(const overpattern_t& fsp, float fspopacity);
  void          generic_main_prepare_ovl();
  void          generic_main_process_ovl(ORIENTATION orient, int i, int link, Ovldraw::OPC_MODE omode, OVL_ORIENTATION ovlorient);
  void          generic_main_end();
};

#endif // FSHDRAWCONSTRUCTOR_H
