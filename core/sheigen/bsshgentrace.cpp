/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgentrace.h"

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

#include "bsshgenparams.h"

//#define _PLUS_049 " + 0.4999"
//#define _MINUS_049 " - 0.4999"
//#define _PLUS_VEC2049 " + vec2(0.4999)"
//#define _PLUS_VEC4049 " + vec4(0.4999)"

//#define _PLUS_049 " + 0.499"
//#define _MINUS_049 " - 0.499"
//#define _PLUS_VEC2049 " + vec2(0.499)"
//#define _PLUS_VEC4049 " + vec4(0.499)"

//#define _PLUS_049 " + 0.49"
//#define _MINUS_049 " - 0.49"
//#define _PLUS_VEC2049 " + vec2(0.49)"
//#define _PLUS_VEC4049 " + vec4(0.49)"

#define _PLUS_049 
#define _MINUS_049 
#define _PLUS_VEC2049 
#define _PLUS_VEC4049 


FshOVCoordsConstructor::FshOVCoordsConstructor(const _Ovldraw::uniforms_t &ufms, int overlay, char *deststring, int ocg_include_bits): 
  m_overlay(overlay), m_writebase(deststring), m_to(deststring), m_offset(0), 
  m_pixingsctr(0), m_relingsctr(0), m_maths(0), m_paramsctr(0), m_prmmemoryiter(0)
{
  loc_uniformsCount = ufms.count;
  loc_uniforms = ufms.arr;
  m_paramsctr += ufms.ccdelim;
  
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  static const char _overstart[] =        "vec3 insider(int i, ivec2 ifromvec);" SHNL;
  
  /// HEAD
  memcpy(&m_to[m_offset], _overstart, sizeof(_overstart)-1);  m_offset += sizeof(_overstart) - 1;
  
  {
    static const char _overinc_getvalue[] = "uniform highp int countPortions;"  SHNL
                                            "float getValue1D(in int portion, in float x);" SHNL
                                            "float getValue2D(in int portion, in vec2 x);" SHNL;
    if (ocg_include_bits & OINC_GETVALUE){    memcpy(&m_to[m_offset], _overinc_getvalue, sizeof(_overinc_getvalue)-1);  m_offset += sizeof(_overinc_getvalue) - 1;   }
    
    static const char _overinc_random[] =   "float randfloat(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }" SHNL
                                            "vec2  randvec2(vec2 co){ return vec2(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453), fract(sin(dot(co.yx ,vec2(12.9898,78.233))) * 43758.5453)); }" SHNL;
    if (ocg_include_bits & OINC_RANDOM){      memcpy(&m_to[m_offset], _overinc_random, sizeof(_overinc_random)-1);  m_offset += sizeof(_overinc_random) - 1;   }
    
    static const char _overinc_bounds[] =   "uniform highp vec2   databounds;"  SHNL;
    if (ocg_include_bits & OINC_DATABOUNDS){  memcpy(&m_to[m_offset], _overinc_bounds, sizeof(_overinc_bounds)-1);  m_offset += sizeof(_overinc_bounds) - 1;   }
  }
    
  m_offset += msexpandParams(&m_to[m_offset], m_overlay, loc_uniformsCount, loc_uniforms);
}

void FshOVCoordsConstructor::_gtb()
{ 
  m_offset += msprintf(&m_to[m_offset], "vec4 overlayOVCoords%d(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 selfposition){" SHNL, 
                       m_overlay);
  m_offset += msprintf(&m_to[m_offset],   "ivec2 icoords = ivec2(coords*ov_ibounds);" SHNL );
  
  static const char _vars[] =             "vec3 result = vec3(0.0);" SHNL
                                          "float mixwell = 0.0;" SHNL
                                          "vec2 _mvar;  float _fvar;" SHNL
                                          "vec2 _tracepass = vec2(1.0,1.0);" SHNL;
  memcpy(&m_to[m_offset], _vars, sizeof(_vars)-1); m_offset += sizeof(_vars) - 1;
}

void FshOVCoordsConstructor::_gtb_coords(const _bs_unzip_t &bsu)
{
  if (bsu.type)
  {
    int coordspixing = register_xyscaler_pixel(bsu.cr);
    m_offset += msprintf(&m_to[m_offset], "ivec2 ioffset = ivec2(");
    if (bsu.type == 1)
      m_offset += msprintf(&m_to[m_offset], "vec2(%f, %f)", bsu.ffs[0], bsu.ffs[1]);
    else if (bsu.type >= 2)
      m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
    m_offset += msprintf(&m_to[m_offset],  " * xyscaler_px_%d" _PLUS_VEC2049 ");" SHNL, coordspixing);
  }
  else
    m_offset += msprintf(&m_to[m_offset],  "ivec2 ioffset = ivec2(0,0);\n");
  
  m_offset += msprintf(&m_to[m_offset],  "ioffset = ioffset + mastercoords;" SHNL);
}

void FshOVCoordsConstructor::_gtb_dimms(const _bs_unzip_t &bsu)
{
  if (bsu.type)
  {
    int dimmpixing = register_xyscaler_pixel(bsu.cr);
    
    if (bsu.type == 1 || bsu.type == 2)
    {
      if (bsu.cr <= CR_XREL_YABS)               /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,   (SCALED variants)
      {
        m_offset += msprintf(&m_to[m_offset], "_fvar = mix(xyscaler_px_%d.x, xyscaler_px_%d.y, step(float(xyscaler_px_%d.y), float(xyscaler_px_%d.x)));" SHNL, dimmpixing,dimmpixing,dimmpixing,dimmpixing);
      }
      else if (bsu.cr >= CR_XABS_YABS_NOSCALED_SCALED)   /// all NOSCALED_SCALED or SCALED_NOSCALED variants
      {
        const char endings[] = {  'y', 'x', 
                                  'y', 'x',
                                  'y', 'x', 
                                  'y', 'x'
                               };
        m_offset += msprintf(&m_to[m_offset], "_fvar = xyscaler_px_%d.%c;" SHNL, dimmpixing, endings[bsu.cr - CR_XABS_YABS_NOSCALED_SCALED]);
      }
      else                                      /// last NOSCALED variants
        m_offset += msprintf(&m_to[m_offset], "_fvar = 1;" SHNL);
      
      m_offset += msprintf(&m_to[m_offset],  "int idimms1 = int(floor(");
      if (bsu.type == 1)
        m_offset += msprintf(&m_to[m_offset], "%f", bsu.ffs[0]);
      else if (bsu.type == 2)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * _fvar" _PLUS_049 "));" SHNL, dimmpixing);
    }
    else if (bsu.type == 3 || bsu.type == 4)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec2 idimms2 = ivec2(floor(");
      if (bsu.type == 3)
        m_offset += msprintf(&m_to[m_offset], "vec2(%f, %f)", bsu.ffs[0], bsu.ffs[1]);
      else if (bsu.type == 4)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * vec2(xyscaler_px_%d.x, xyscaler_px_%d.y) " _PLUS_VEC2049 "));" SHNL, dimmpixing,dimmpixing);
    }
    else if (bsu.type == 5 || bsu.type == 6)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec4 idimms4 = ivec4(floor((");
      if (bsu.type == 5)
      {
        m_offset += msprintf(&m_to[m_offset], "vec4(%f, %f, %f, %f)", bsu.ffs[0], bsu.ffs[1], bsu.ffs[2], bsu.ffs[3]);
      }
      else if (bsu.type == 6)
      {
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      }
      m_offset += msprintf(&m_to[m_offset],  " * vec4(xyscaler_px_%d.x, xyscaler_px_%d.y, xyscaler_px_%d.x, xyscaler_px_%d.y)) " _PLUS_VEC4049 "));" SHNL, dimmpixing,dimmpixing,dimmpixing,dimmpixing);
    }
    else if (bsu.type == 7)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec2 idimms2 = ivec2(floor(opm%D_%D * vec2(xyscaler_px_%d.x, xyscaler_px_%d.y) " _PLUS_VEC2049 "));" SHNL, m_overlay, m_paramsctr++, dimmpixing, dimmpixing);
    }
  }
}


void FshOVCoordsConstructor::math_pi()
{
  static const char _overmath[] = "const float PI = 3.1415926;" SHNL;
  memcpy(&m_to[m_offset], _overmath, sizeof(_overmath)-1); m_offset += sizeof(_overmath) - 1;
  m_maths |= (1 << (int)MM_PI);
}

#if 0

int FshOVCoordsConstructor::register_xyscaler_pixel(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                                         "ivec2 xyscaler_px_%d = ov_iscaler;" SHNL,
                                         "ivec2 xyscaler_px_%d = ov_ibounds;" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, ov_ibounds.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x, ov_iscaler.y);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "ivec2 xyscaler_px_%d = ivec2(1, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x, ov_indimms.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_indimms.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x, 1);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_iscaler.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x, ov_ibounds.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x, ov_indimms.y);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_ibounds.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, ov_indimms.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x, ov_iscaler.y);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_pixingsctr);
  return m_pixingsctr++;
}

int FshOVCoordsConstructor::register_xyscaler_01(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
//                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x-1, ov_indimms.y-1);" SHNL,   // ntf!
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x, ov_indimms.y);" SHNL,   // ntf!
                                         "vec2 xyscaler_01_%d = vec2(1.0, 1.0);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x, 1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_indimms.y);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_ibounds.y);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x, ov_ibounds.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x, ov_indimms.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, 1);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x, 1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_ibounds.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_indimms.y);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_relingsctr);
  return m_relingsctr++;
}

#else


int FshOVCoordsConstructor::register_xyscaler_pixel(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                                         "ivec2 xyscaler_px_%d = ov_iscaler;" SHNL,
                                         "ivec2 xyscaler_px_%d = ov_ibounds - ivec2(1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, ov_ibounds.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x-1, ov_iscaler.y);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "ivec2 xyscaler_px_%d = ivec2(1, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x-1, ov_indimms.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_indimms.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x-1, 1);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_iscaler.y);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x-1, ov_ibounds.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x-1, ov_indimms.y-1);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "ivec2 xyscaler_px_%d = ivec2(1, ov_ibounds.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_iscaler.x, ov_indimms.y-1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_ibounds.x-1, 1);" SHNL,
                                         "ivec2 xyscaler_px_%d = ivec2(ov_indimms.x-1, ov_iscaler.y);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_pixingsctr);
  return m_pixingsctr++;
}

int FshOVCoordsConstructor::register_xyscaler_01(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x-1, ov_indimms.y-1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1.0, 1.0);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x-1, 1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_indimms.y-1);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds - ivec2(1));" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x-1, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_ibounds.y-1);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x-1, ov_ibounds.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x-1, ov_indimms.y-1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, 1);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "vec2 xyscaler_01_%d = vec2(ov_ibounds.x-1, 1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_indimms.x-1, ov_iscaler.y);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(1, ov_ibounds.y-1);" SHNL,
                                         "vec2 xyscaler_01_%d = vec2(ov_iscaler.x, ov_indimms.y-1);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_relingsctr);
  return m_relingsctr++;
}

#endif

void FshOVCoordsConstructor::param_alias(const char *name, int memslot)
{
  if (memslot == -1)
  {
    m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D;" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
    m_paramsctr++;
  }
  else
    m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D;" SHNL, glsl_types[loc_uniforms[m_prmmemory[memslot]].type], name, m_overlay, m_prmmemory[memslot]);
}

int FshOVCoordsConstructor::param_push()
{
  int memslot = m_prmmemoryiter;
  m_prmmemory[m_prmmemoryiter++] = m_paramsctr++;
  return memslot;
}

void FshOVCoordsConstructor::param_get()
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshOVCoordsConstructor::param_mem(int memslot)
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_prmmemory[memslot]);
}

void FshOVCoordsConstructor::param_pass()
{
  m_paramsctr++;
}

void FshOVCoordsConstructor::param_peek()
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr);
}

void FshOVCoordsConstructor::param_for_arr_begin(const char *name, const char *arrlengthname, const char* additname)
{
  m_offset += msprintf(&m_to[m_offset],   "int %s = int(opm%D_%D.length());" SHNL
                                          "for (int i=0; i<%s; i++){" SHNL
                                          "%s %s = opm%D_%D[i];" SHNL,
                      arrlengthname, m_overlay, m_paramsctr, arrlengthname, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
  m_paramsctr++;
  if (additname)
  {
    m_offset += msprintf(&m_to[m_offset],  "%s %s = opm%D_%D[i];" SHNL,
                      glsl_types[loc_uniforms[m_paramsctr].type], additname, m_overlay, m_paramsctr);
    m_paramsctr++;
  }
}

void FshOVCoordsConstructor::param_for_rarr_begin(const char *name)  // DT_ARR, DT_1I, DT_1I
{
  m_offset += msprintf(&m_to[m_offset],  "ivec2 rarr_rnds = ivec2(opm%D_%D,opm%D_%D);" SHNL
                      "int rarr_len = opm%D_%D.length();" SHNL
                      "for (int i=0; i<rarr_rnds[1]; i++){" SHNL
                      "int rarr_idx = rarr_rnds[0] + i;" SHNL
                      "if (rarr_idx >= rarr_len) rarr_idx = rarr_idx - rarr_len;" SHNL
                      "%s %s = opm%D_%D[i];" SHNL,  m_overlay, m_paramsctr+1, m_overlay, m_paramsctr+2, 
                      m_overlay, m_paramsctr, 
                      glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
  m_paramsctr += 3;
}

void FshOVCoordsConstructor::param_for_oarr_begin(const char *name_cur, const char *name_next, const char *arrlengthname)  // DT_ARR, DT_1I
{
  m_offset += msprintf(&m_to[m_offset],  "%s %s = opm%D_%D[0]; %s %s; " SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name_cur, m_overlay, m_paramsctr, glsl_types[loc_uniforms[m_paramsctr].type], name_next);
  m_offset += msprintf(&m_to[m_offset],  "int %s = opm%D_%D;" SHNL
                      "for (int i=1; i<%s; i++){" SHNL
                      "%s = opm%D_%D[i];" SHNL,  arrlengthname, m_overlay, m_paramsctr + 1,
                      arrlengthname, 
                      name_next, m_overlay, m_paramsctr);
  m_paramsctr += 2;
}

void FshOVCoordsConstructor::param_for_carr_begin(const char* name, const char* arrlengthname)
{
  m_offset += msprintf(&m_to[m_offset],   "int %s = opm%D_%D;" SHNL
                                          "for (int i=0; i<%s; i++){" SHNL
                                            "%s %s = opm%D_%D[i];" SHNL,  arrlengthname, m_overlay, m_paramsctr + 1,
                                            arrlengthname, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
  m_paramsctr += 2;
}

void FshOVCoordsConstructor::param_for_end() {  m_offset += msprintf(&m_to[m_offset], "}");  }

void FshOVCoordsConstructor::goto_normed()
{
  m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ioffset;" SHNL);
}

void FshOVCoordsConstructor::goto_normed(const char *someparam, int pixing, bool saveasoffset)
{
  if (saveasoffset)
  {
    m_offset += msprintf(&m_to[m_offset], "ioffset = ivec2(floor((%s * xyscaler_px_%d) " _PLUS_VEC2049 "));" SHNL, someparam, pixing);
    m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ioffset;" SHNL);
  }
  else
    m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ivec2(floor((%s * xyscaler_px_%d) " _PLUS_VEC2049 "));" SHNL, someparam, pixing);
}

void FshOVCoordsConstructor::goto_normed_empty()
{
  m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = ivec2(0,0);" SHNL);
}

void FshOVCoordsConstructor::goto_normed_rotated(const char *angleRadName)
{
  m_offset += msprintf(&m_to[m_offset], "mat2 rotationMatrix = mat2 (cos(%s), -sin(%s), sin(%s), cos(%s));  inormed = inormed*rotationMatrix;" SHNL, angleRadName, angleRadName, angleRadName, angleRadName);
}

void FshOVCoordsConstructor::var_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "float %s = %f;" SHNL, name, value); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %f;" SHNL, name, value); }

void FshOVCoordsConstructor::var_inline(const char* name, float v){  m_offset += msprintf(&m_to[m_offset], name, v); }

void FshOVCoordsConstructor::var_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "int %s = %d;" SHNL, name, value); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;" SHNL, name, value); }

void FshOVCoordsConstructor::var_inline(const char* name, int v){  m_offset += msprintf(&m_to[m_offset], name, v); }

void FshOVCoordsConstructor::var_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "vec2 %s = vec2(%f, %f);" SHNL, name, v1, v2); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%f, %f);" SHNL, name, v1, v2); }

void FshOVCoordsConstructor::var_inline(const char* name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], name, v1, v2); }

void FshOVCoordsConstructor::var_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshOVCoordsConstructor::var_inline(const char* name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], name, v1, v2); }

void FshOVCoordsConstructor::var_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "vec3 %s = vec3(%f, %f, %f);" SHNL, name, v1, v2, v3); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%f, %f, %f);" SHNL, name, v1, v2, v3); }

void FshOVCoordsConstructor::var_inline(const char* name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], name, v1, v2, v3); }

void FshOVCoordsConstructor::var_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "const ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshOVCoordsConstructor::var_inline(const char* name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], name, v1, v2, v3); }

void FshOVCoordsConstructor::var_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "vec4 %s = vec4(%f, %f, %f, %f);" SHNL, name, v1, v2, v3, v4); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "const vec4 %s = vec4(%f, %f, %f, %f);" SHNL, name, v1, v2, v3, v4); }


void FshOVCoordsConstructor::var_fixed(const char *name, const color3f_t& v){  m_offset += msprintf(&m_to[m_offset], "vec3 %s = vec3(%f, %f, %f);" SHNL, name, v.r, v.g, v.b); }

void FshOVCoordsConstructor::var_const_fixed(const char *name, const color3f_t& v){  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%f, %f, %f);" SHNL, name, v.r, v.g, v.b); }

void FshOVCoordsConstructor::var_inline(const char* name, const color3f_t& v){  m_offset += msprintf(&m_to[m_offset], name, v.r, v.g, v.b); }



void FshOVCoordsConstructor::var_array_f_empty(const char* name, int size){  m_offset += msprintf(&m_to[m_offset], "float %s[%d];" SHNL, name, size);   }
void FshOVCoordsConstructor::var_array_ff_empty(const char* name, int size){  m_offset += msprintf(&m_to[m_offset], "vec2 %s[%d];" SHNL, name, size);   }
void FshOVCoordsConstructor::var_array_fff_empty(const char* name, int size){  m_offset += msprintf(&m_to[m_offset], "vec3 %s[%d];" SHNL, name, size);   }

void FshOVCoordsConstructor::var_array(const char* name, float v1){  m_offset += msprintf(&m_to[m_offset], "float %s[1] = { %f };" SHNL, name, v1);   }

void FshOVCoordsConstructor::var_array(const char* name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "float %s[2] = { %f, %f };" SHNL, name, v1, v2);   }

void FshOVCoordsConstructor::var_array(const char* name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "float %s[3] = { %f, %f, %f };" SHNL, name, v1, v2, v3);   }

void FshOVCoordsConstructor::var_array(const char* name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "float %s[4] = { %f, %f, %f, %f };" SHNL, name, v1, v2, v3, v4);   }

void FshOVCoordsConstructor::var_array(const char* name, float v1, float v2, float v3, float v4, float v5){  m_offset += msprintf(&m_to[m_offset], "float %s[5] = { %f, %f, %f, %f, %f };" SHNL, name, v1, v2, v3, v4, v5);   }




void FshOVCoordsConstructor::var_static(DTYPE type, const char *name_eq_value){  m_offset += msprintf(&m_to[m_offset], "%s %s;" SHNL, glsl_types[(int)type], name_eq_value); }

void FshOVCoordsConstructor::var_static(const char *name, const char *value){  m_offset += msprintf(&m_to[m_offset], "%s = %s;" SHNL, name, value); }

void FshOVCoordsConstructor::var_const_static(DTYPE type, const char *name_eq_value){  m_offset += msprintf(&m_to[m_offset], "const %s %s;" SHNL, glsl_types[(int)type], name_eq_value); }




void FshOVCoordsConstructor::xyscale_x_pixel(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor(float(%s*xyscaler_px_%d.x)" _PLUS_049 ");" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::xyscale_y_pixel(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor(float(%s*xyscaler_px_%d.y)" _PLUS_049 ");" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::xyscale_xy_pixel(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor(vec2(%s*xyscaler_px_%d)" _PLUS_049 ");" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::xyscale_x_01(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/xyscaler_01_%d.x;" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::xyscale_y_01(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/xyscaler_01_%d.y;" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::xyscale_xy_01(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/xyscaler_01_%d;" SHNL, name, name, resc_idx); }

void FshOVCoordsConstructor::push(const char *sztext)
{
  while (*sztext != '\0')
    m_to[m_offset++] = *sztext++;
}

void FshOVCoordsConstructor::push(const char* text, unsigned int len)
{
  if (len > 0)
  {
    memcpy(&m_to[m_offset], text, len - 1);
    m_offset += len - 1;
  }
}

void FshOVCoordsConstructor::push(int v){  m_offset += msprintf(&m_to[m_offset], "%d", v); }

void FshOVCoordsConstructor::push(float v){  m_offset += msprintf(&m_to[m_offset], "%f", v); }


//void FshOVCoordsConstructor::inside_begin1(const char *limrad1){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(-%s,float(inormed.x))*step(-%s,float(inormed.y))*(1-step(%s + 1.0, float(inormed.x)))*(1-step(%s + 1.0, float(inormed.y))));   if (inside != 0){" SHNL,
//                                                                              limrad1, limrad1, limrad1, limrad1); }

//void FshOVCoordsConstructor::inside_begin2(const char *limits2){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1-step(%s[0], float(inormed.x)))*(1-step(%s[1], float(inormed.y))));   if (inside != 0){" SHNL, limits2, limits2); }

//void FshOVCoordsConstructor::inside_begin4(const char *limits4){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(%s[0],float(inormed.x))*step(%s[2],float(inormed.y))*(1-step(%s[1] + 1.0, float(inormed.x)))*(1-step(%s[3] + 1.0, float(inormed.y))));   if (inside != 0){" SHNL, 
//                                                                              limits4, limits4, limits4, limits4); }

void FshOVCoordsConstructor::inside_begin1(const char *limrad1){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(-%s,float(inormed.x))*step(-%s,float(inormed.y))*step(float(inormed.x), %s + 1.0)*step(float(inormed.y), %s + 1.0));   if (inside != 0){" SHNL,
                                                                              limrad1, limrad1, limrad1, limrad1); }

void FshOVCoordsConstructor::inside_begin2(const char *limits2){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*step(float(inormed.x), %s[0])*step(float(inormed.y), %s[1]));   if (inside != 0){" SHNL, limits2, limits2); }

//void FshOVCoordsConstructor::inside_begin4(const char *limits4){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(%s[0],float(inormed.x))*step(%s[2],float(inormed.y))*(1-step(%s[1] + 1.0, float(inormed.x)))*(1-step(%s[3] + 1.0, float(inormed.y))));   if (inside != 0){" SHNL, 
//                                                                              limits4, limits4, limits4, limits4); }

void FshOVCoordsConstructor::inside_end(){ m_offset += msprintf(&m_to[m_offset], "}"); }

void FshOVCoordsConstructor::ban_trace(bool ban)
{
  if (ban)
    m_offset += msprintf(&m_to[m_offset], "_tracepass = vec2(1.0, 0.0);");
  else
    m_offset += msprintf(&m_to[m_offset], "_tracepass = vec2(1.0, 1.0);");
}

void FshOVCoordsConstructor::construct_trail_vec2(int pxwidth, float curver, const char* pxdistance_int, const char* result_vec2)
{
   m_offset += msprintf(&m_to[m_offset], 
           "vec2 %1 = vec2(abs(%s), 0.0);" SHNL
           "%1[1] = clamp((1+%d - %1[0])/float(1+%d), 0.0, 1.0);" SHNL
           "%1[0] = %1[1]*(1.0 / (1 + 6.2*abs(%F)*(1.0 - %1[1])) );" SHNL
           "%1[1] = mix(2.0*%1[1] - %1[0], %1[0], step(0.0, %F));" SHNL
           "%1[0] = clamp(%1[1], 0.0, 1.0);" SHNL,
           result_vec2, pxdistance_int, pxwidth, pxwidth, curver, curver
        );
}

void FshOVCoordsConstructor::construct_trail_vec2(const char*  pxwidth, const char*  curver, const char* pxdistance_int, const char* result_vec2)
{
   m_offset += msprintf(&m_to[m_offset], 
           "vec2 %1 = vec2(abs(%s), 0.0);" SHNL
           "%1[1] = clamp((1+%3 - %1[0])/float(1+%3), 0.0, 1.0);" SHNL
           "%1[0] = %1[1]*(1.0 / (1 + 6.2*abs(%4)*(1.0 - %1[1])) );" SHNL
           "%1[1] = mix(2.0*%1[1] - %1[0], %1[0], step(0.0, %4));" SHNL
           "%1[0] = clamp(%1[1], 0.0, 1.0);" SHNL,
           result_vec2, pxdistance_int, pxwidth, curver
        );
}





#define TRACE_MIX_MVAR_WITH_RESULT "result = mix(result, vec3(_mvar*_tracepass, 0.0), 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );" SHNL

void FshOVCoordsConstructor::trace_triangle_cc(const char *side, int direction, float fillcoeff)
{
  char rd[48];    msprintf(rd, "(%s)", side);
  char rd_plus[48];     msprintf(rd_plus, "(0.707*%s)", side);
  char rd_minus[48];    msprintf(rd_minus, "(-0.707*%s)", side);
  
  m_offset += msprintf(&m_to[m_offset],   "vec3 _tri = vec3(float(inormed.y) + %s" _PLUS_049 " - 1.0, 0.0, 0.0);" SHNL
                                          "_tri[1] = %s;" SHNL, 
                      direction == 0 || direction == 2? rd_plus :
                      direction == 1 || direction == 3? rd_minus : "",
                      direction == 0 || direction == 2? "1.0 - step(_tri[0], 0.0)" :
                      direction == 1 || direction == 3? "1.0 - step(0.0, _tri[0])" :
                                      "0.0"
                                      );
  m_offset += msprintf(&m_to[m_offset],
                          "_tri[0] = 1.0 - (abs(_tri[0]))/(%s*0.866*2.0);"
                          "_tri[2] = _tri[1]*_tri[0]*%s " _MINUS_049";" SHNL    /// otrezok
                          "_mvar[0] = (1.0+thick - clamp(float(abs(inormed.x) - int(_tri[2])), 0.0, 1.0+thick))/(1.0+thick) * (1.0-step(_tri[2], 0.1));" SHNL
                          "_tri[2] = step(float(abs(inormed.x)), _tri[2]-1.0);"
                          "_mvar[0] = mix(_mvar[0], %F, _tri[2]);" SHNL
                          "_mvar = vec2(_tri[1]*_mvar[0], _tri[0]*(1-_tri[2])*%s*2.31);" SHNL
                          TRACE_MIX_MVAR_WITH_RESULT
                      ,
                      side, side, fillcoeff, side
                      );
  trace_2linehorz_c(rd, nullptr, (direction == 0 || direction == 2? rd_minus:
                                  direction == 1 || direction == 3? rd_plus:
                                                                    ""));
}

void FshOVCoordsConstructor::simplemix_triangle_cc(const char* side, int direction, float fillcoeff)
{  
  if (direction == 0 || direction == 2)
  {
    m_offset += msprintf(&m_to[m_offset], "float sg2 = -step(float(-%1-inormed.%2), 0.0);" SHNL
                                          "sg2 = -(inormed.%2 + sg2*%1);" SHNL
                                          "mixwell = step(float(abs(inormed.%3)-sg2), 0.0)*step(float(-%1/2-inormed.%2),0.0) - "
                                          "step(float(abs(inormed.%3)-sg2+1), 0.0)*step(float(-%1/2+1-inormed.%2), 0.0)*%F;" SHNL
                         , side, 
                         direction == 0? "y" : "x", 
                         direction == 0? "x" : "y", 
                         1.0f - fillcoeff);
  }
  else if (direction == 1 || direction == 3)
  {
    m_offset += msprintf(&m_to[m_offset], "float sg2 = step(float(-%1-inormed.%2), 0.0);" SHNL
                                          "sg2 = (inormed.%2 + sg2*%1);" SHNL
                                          "mixwell = step(float(abs(inormed.%3)-sg2), 0.0)*step(float(inormed.%2-%1/2), 0.0) - "
                                                    "step(float(abs(inormed.%3)-sg2+1), 0.0)*step(float(inormed.%2-%1/2+1), 0.0)*%F;" SHNL
                         , side, 
                         direction == 1? "y" : "x", 
                         direction == 1? "x" : "y", 
                         1.0f - fillcoeff);
  }
}


void FshOVCoordsConstructor::trace_rect_xywh(const char *wh, float fillcoeff, const char *crosslimit)
{
//  char r0[48], r1[48], r01[64], r11[64]; msprintf(r0, "%s[0]", wh); msprintf(r1, "%s[1]", wh); msprintf(r01, "%s[0]-sign(%s[0])", wh,wh); msprintf(r11, "%s[1]-sign(%s[1])", wh,wh); 
  char r0[48], r1[48], r01[64], r11[64]; msprintf(r0, "%s[0]", wh); msprintf(r1, "%s[1]", wh); msprintf(r01, "(%s[0])", wh); msprintf(r11, "(%s[1])", wh); 
  trace_linehorz_l(r0, nullptr, nullptr, crosslimit); trace_linehorz_l(r0, nullptr, r11, crosslimit);
  trace_linevert_b(r1, nullptr, nullptr, crosslimit); trace_linevert_b(r1, nullptr, r01, crosslimit);
  if (fillcoeff > 0.0f)
  {
    m_offset += msprintf(&m_to[m_offset],
                        "_mvar[0] = step(float(abs(inormed[0])), float(abs(%s)))*(1.0-step(float(sign(inormed[0])*sign(%s)),0.0))*"
                        "step(float(abs(inormed[1])), float(abs(%s)))*(1.0-step(float(sign(inormed[1])*sign(%s)),0.0));" SHNL
                        "result = mix(result, vec3(_mvar[0]*%F*_tracepass[0], 0, 0), _mvar[0] * (1.0-step(1.0, result[0])) );" SHNL
                        ,
                        r01, r01, r11, r11,
//                         r01, r11, 
                         fillcoeff);
  }
}

void FshOVCoordsConstructor::trace_rect_cc(const char *rdimms, float fillcoeff, const char *crosslimit)
{   
//  char r0[48], r1[48], r01[64], r11[64]; msprintf(r0, "%s[0]", rdimms); msprintf(r1, "%s[1]", rdimms); msprintf(r01, " (%s[0]-sign(%s[0]))", rdimms,rdimms); msprintf(r11, " (%s[1]-sign(%s[1]))", rdimms,rdimms); 
  char r0[48], r1[48], r01[64], r11[64]; msprintf(r0, "%s[0]", rdimms); msprintf(r1, " %s[1]", rdimms); msprintf(r01, " (%s[0])", rdimms); msprintf(r11, " (%s[1])", rdimms); 
  trace_2linehorz_c(r0, nullptr, &r11[1], crosslimit); r11[0] = '-'; trace_2linehorz_c(r0, nullptr, r11, crosslimit);
  trace_2linevert_c(r1, nullptr, &r01[1], crosslimit); r01[0] = '-'; trace_2linevert_c(r1, nullptr, r01, crosslimit);
  if (fillcoeff > 0.0f)
  {
    m_offset += msprintf(&m_to[m_offset],
                        "_mvar = vec2(step(float(abs(inormed[0])), abs(%s)) * step(float(abs(inormed[1])), float(abs(%s))), 0.0);" SHNL
                        "result = mix(result, vec3(_mvar[0] * %F * _tracepass[0], 0,0), _mvar[0] * (1.0 - step(1.0, result[0])) );" SHNL,
                        r11, r01, fillcoeff);
  }
}

void FshOVCoordsConstructor::trace_square_lb_begin(const char* aside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sqdimms = vec2(%s, %s);", aside, aside);  trace_rect_xywh("_sqdimms"); }

void FshOVCoordsConstructor::trace_square_lb_end(float fillcoeff) {  trace_rect_xywh("_sqdimms", fillcoeff); }

void FshOVCoordsConstructor::trace_square_cc_begin(const char* halfside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sq2dimms = vec2(%s, %s);", halfside, halfside);  trace_rect_cc("_sq2dimms"); }

void FshOVCoordsConstructor::trace_square_cc_end(float fillcoeff) {  trace_rect_cc("_sq2dimms", fillcoeff); }

void FshOVCoordsConstructor::simplemix_square_cc(const char* halfside, float fillcoeff)
{
//  m_offset += msprintf(&m_to[m_offset], "mixwell = step(int(abs(inormed.x)), %1)*step(%1, int(abs(inormed.x)))*step(int(abs(inormed.y)), %1-1) + "
//                                                  "step(int(abs(inormed.y)), %1)*step(%1, int(abs(inormed.y)))*step(int(abs(inormed.x)), %1) + "
//                                                  "(step(int(abs(inormed.x)), %1-1)*step(int(abs(inormed.y)), %1-1)) * %F;", halfside, fillcoeff);
  m_offset += msprintf(&m_to[m_offset], "mixwell = step(float(abs(inormed.x)-%1), 0.0)*step(float(%1-abs(inormed.x)), 0.0)*"
                                                  "step(float(abs(inormed.y)-%1+1), 0.0) + " SHNL
                                                  "step(float(abs(inormed.y)-%1), 0.0)*step(float(%1-abs(inormed.y)), 0.0)*step(float(abs(inormed.x)-%1), 0.0) + " SHNL
                                                  "step(float(abs(inormed.x)-%1+1), 0.0)*step(float(abs(inormed.y)-%1+1), 0.0)*%F;" SHNL, halfside, fillcoeff);
}

void FshOVCoordsConstructor::trace_rhomb_cc(const char* side2side, float fillcoeff)
{   
  m_offset += msprintf(&m_to[m_offset], "vec2 sg2 = vec2(1 - 2*step(0.0, float(inormed.x)), 1 - 2*step(0.0, float(inormed.y)));" SHNL
                                        "sg2 = vec2(sg2[0]*(inormed.x - (-1 + 2*step(0.0, float(inormed.x)))*%s), sg2[1]*inormed.y);" SHNL
                                        "_mvar[0] = (1.0 + thick - clamp(abs(sg2[0] + sg2[1]), 0.0, 1.0 + thick))/(1.0 + thick);" SHNL
                       , side2side, side2side);
  
  m_offset += msprintf(&m_to[m_offset], "_mvar[1] = distance(vec2(0,0), vec2(sg2[0], sg2[1]));" SHNL,
                                          side2side);
  
  m_offset += msprintf(&m_to[m_offset], "result = mix(result, vec3(_mvar*_tracepass, 0.0), 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );" SHNL);
  
  if (fillcoeff > 0.0f)
  {
    m_offset += msprintf(&m_to[m_offset],
                        "_mvar = vec2(step(abs(inormed.y), sg2[0]), 0.0);" SHNL
                        "result = mix(result, vec3(_mvar[0] * %F * _tracepass[0], 0,0), _mvar[0] * (1.0 - step(1.0, result[0])) );" SHNL,
                        fillcoeff);
  }
}

void FshOVCoordsConstructor::simplemix_rhomb_cc(const char* side2side, float fillcoeff)
{
  m_offset += msprintf(&m_to[m_offset], "float sg2 = 1 - 2*step(0.0, float(inormed.x));" SHNL
                                        "sg2 = sg2*(inormed.x - (-1 + 2*step(0.0, float(inormed.x)))*%s);" SHNL
                                        "mixwell = step(float(abs(inormed.y)-sg2), 0.0)*step(float(sg2-abs(inormed.y)), 0.0) + step(float(abs(inormed.y)-sg2+1), 0.0)*%F;" SHNL
                       , side2side, fillcoeff);
}

void FshOVCoordsConstructor::trace_circle_cc_begin(const char *radius, const char *border) { m_offset += msprintf(&m_to[m_offset], "vec4 _cd = vec4(%s-1, %s + thick, inormed.x*inormed.x + inormed.y*inormed.y, %s);", radius, border, border); }

void FshOVCoordsConstructor::trace_circle_cc_end(float fillcoeff/*, bool notraceinside*/)
{
  if ((m_maths & (1 << MM_PI)) == 0)  math_pi();
  if (fillcoeff > 0.0f)
    m_offset += msprintf(&m_to[m_offset],  "vec3 r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3 r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_fill = 1-step(r2[1], _cd[2]);" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*mixwell_fill;" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
//                                          "_mvar = vec3(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0]);" SHNL
//                         "_mvar = vec3(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))/(2*PI)*step(mixwell_fill, 0.0), 2*PI*_cd[0]);" SHNL
                         "_mvar = vec2(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))*_cd[0]*step(mixwell_fill, 0.0));" SHNL
                                          TRACE_MIX_MVAR_WITH_RESULT
                        , fillcoeff
                        );    
  else
    m_offset += msprintf(&m_to[m_offset],  "vec3  r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3  r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_onborder = (1 - step(r3[2], _cd[2])) * (step(r3[0], _cd[2])) * abs(sign(thick));" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*(1 - step(r2[1], _cd[2]));" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
//                                          "_mvar = vec3(clamp(mixwell_onborder + mixwell_before + mixwell_aftere, 0.0,1.0), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0]);" SHNL
                         "_mvar = vec2(clamp(mixwell_onborder + mixwell_before + mixwell_aftere, 0.0,1.0), atan(float(inormed.x), float(inormed.y))*_cd[0]);" SHNL
                                          TRACE_MIX_MVAR_WITH_RESULT
                         );
}

void FshOVCoordsConstructor::simplemix_circle_cc(const char* radius, float fillcoeff)
{
  m_offset += msprintf(&m_to[m_offset], 
                                         "float sg = inormed.x*inormed.x + inormed.y*inormed.y;" SHNL
                                         "float fd = (sg - %1*%1)/max(%1/16.0, 2.0);" SHNL
                                         "mixwell = 1.0 - clamp(fd*fd/(16*%1), 0.0, 1.0);" SHNL
                                         "mixwell = max(mixwell, step(sg, %1*%1)*%F);" SHNL 
                       , radius, fillcoeff);
}

void FshOVCoordsConstructor::simplemix_cross_cc(const char* side, float fillcoeff)
{
//  if (fillcoeff > 0.9f)
//    fillcoeff = 0.9f;
  if (fillcoeff < 0.5f)
    fillcoeff = 0.5f;
  m_offset += msprintf(&m_to[m_offset], "float sc = abs(inormed.x) - abs(inormed.y);" SHNL
                                        "mixwell = clamp(1.0 - sc*sc*%F, 0.0, 1.0)*step(float(abs(inormed.x)-%s), 0.0)*step(float(abs(inormed.y)-%s), 0.0);" SHNL
                       , 1.0f - fillcoeff, side, side);
}

//                                              crosslimit                                           offset                                                 
#define TRACE_MAIN_CONDITION(idx)  "_mvar[0] = %s * (1.0+thick - clamp(abs(inormed["#idx"] - floor(float(%s)" _PLUS_049 ")), 0.0, 1.0+thick))/(1.0+thick);"

#define TRACE_MVAR_INSIDER(a)        isize == nullptr? "_mvar[1] = "#a" - %s;" SHNL \
                                                        "_mvar[0] = _mvar[0] * step(0.0, _mvar[1]);" SHNL \
                                          : \
                                                       "_mvar[1] = "#a" - %s;" SHNL \
                                                       "_mvar[0] = _mvar[0] * step(0.0, _mvar[1]) * step(_mvar[1], %s);" SHNL


void FshOVCoordsConstructor::trace_2linehorz_c(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(1), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(abs(inormed[0])), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
                      
}

void FshOVCoordsConstructor::trace_2linevert_c(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(0), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(abs(inormed[1])), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_linehorz_l(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(1), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(inormed[0]), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_linehorz_r(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(1), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(-inormed[0]), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_linevert_t(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(0), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(-inormed[1]), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_linevert_b(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], TRACE_MAIN_CONDITION(0), icrosslimit == nullptr? "1.0" : icrosslimit, ioffset == nullptr? "0.0" : ioffset);
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(inormed[1]), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_lines_x(const char *isize, const char *igap, const char *icrosslimit)
{
//  m_offset += msprintf(&m_to[m_offset],
//                      "_mvar[0] = atan(float(abs(inormed.x)), float(abs(inormed.y)));" SHNL
//                      "_mvar[1] = length(vec2(inormed))*cos(0.785398 - _mvar[0]);" SHNL
//                      "_mvar[2] = _mvar[1]*0.707;" SHNL
//                      "_mvar.xy = vec2(_mvar[2], _mvar[2]);" SHNL
//                      "_fvar = %s * mix((1.0+thick - clamp(distance(abs(inormed), _mvar.xy), 0.0, 1.0+thick))/(1.0+thick), 1.0, step(length(vec2(inormed)), 0.0));" SHNL
//                      "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(int(_mvar[2]), ivec2(int(%s), int(%s)));" SHNL
//                      TRACE_MIX_MVAR_WITH_RESULT
//                      ,
//                      icrosslimit == nullptr? "1.0" : icrosslimit,
//                      igap == nullptr? "0" : igap,
//                      isize == nullptr? "5000" : isize);
  m_offset += msprintf(&m_to[m_offset],
                      "_fvar = atan(float(abs(inormed.x)), float(abs(inormed.y)));" SHNL
                      "_fvar = 0.7071*length(vec2(inormed))*cos(0.785398 - _fvar);" SHNL
                      "_mvar[0] = %s*mix((1.0+thick - clamp(distance(abs(inormed), vec2(_fvar, _fvar)), 0.0, 1.0+thick))/(1.0+thick), 1.0, step(length(vec2(inormed)), 0.0));" SHNL,
                      icrosslimit == nullptr? "1.0" : icrosslimit
                       );
  m_offset += msprintf(&m_to[m_offset], TRACE_MVAR_INSIDER(_fvar), igap == nullptr? "0" : igap, isize);
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::trace_line_from_normed_to(const char *inormedendpoint)
{ 
  m_offset += msprintf(&m_to[m_offset], "vec2 _pt = %s;" SHNL, inormedendpoint);
  m_offset += msprintf(&m_to[m_offset], 
                        "vec2 bz = vec2(step(_pt.x, 0.0)*step(0.0, _pt.x), step(_pt.y, 0.0)*step(0.0, _pt.y));"
                        "_fvar = _pt.x/mix(_pt.y, 1.0, bz[1]);"
                        "_fvar = (1-bz[0])*(1-bz[1])*(_fvar*inormed.y - float(inormed.x))/sqrt(1.0 + _fvar*_fvar) + float(inormed.x)*bz[0] + float(inormed.y)*bz[1];" SHNL
                        "_fvar = min(abs(_fvar), 1.0+thick);" SHNL
                      );
  m_offset += msprintf(&m_to[m_offset], 
                        "vec3 _lens = vec3(length(_pt), length(vec2(inormed)), length(_pt) + 1.0 + thick);" SHNL
                        "_fvar = mix( _fvar, min(length(_pt.xy - vec2(inormed)), 1.0+thick), step(_lens[0], _lens[1]));" SHNL
                        "_mvar = vec2( (1.0 - _fvar/(1.0+thick))*step(_lens.y*_lens.y - _fvar*_fvar, (_lens.z)*(_lens.z)) * step(0.0, sign(dot(_pt.xy, inormed))), 0 );" SHNL // _lens[1]
                        TRACE_MIX_MVAR_WITH_RESULT
                       );
}

//void FshOVCoordsConstructor::trace_ray_trough_normed_from(const char* inormedstartpoint)
//{
//  m_offset += msprintf(&m_to[m_offset], "_mvar.xy = %s;" SHNL, inormedstartpoint);
//  m_offset += msprintf(&m_to[m_offset], 
//                        "vec2 bz = vec2(step(_mvar.x, 0.0)*step(0.0, _mvar.x), step(_mvar.y, 0.0)*step(0.0, _mvar.y));"
//                        "_mvar.z = _mvar.x/mix(_mvar.y, 1.0, bz[1]);"
//                        "_fvar = (1-bz[0])*(1-bz[1])*(_mvar.z*inormed.y - float(inormed.x))/sqrt(1.0 + _mvar.z*_mvar.z) + float(inormed.x)*bz[0] + float(inormed.y)*bz[1];" SHNL
//                      );
//  m_offset += msprintf(&m_to[m_offset], 
//                        "vec2 _lens = vec2(length(_mvar.xy), length(vec2(inormed)));" SHNL
//                        "_fvar = min(abs(_fvar), 1.0+thick);" SHNL
////                        "_fvar = mix( _fvar, 1.0+thick, step(_lens[0], _lens[1]));" SHNL
////                        "_fvar = mix( _fvar, 1.0+thick, step(0.0, _lens[1] - _lens[0]));" SHNL
////                        "_fvar = mix( 1.0+thick, _fvar, 1.0 - step(_lens[0], _lens[1]));" SHNL
////                        "_fvar = mix( _fvar, 1.0+thick, step(0.0, dot(_mvar.xy, vec2(inormed))) );" SHNL
                       
//                        "_fvar = mix( _fvar, 1.0+thick, (step(0.0, dot(_mvar.xy, vec2(inormed)))) * step(length(_mvar.xy), length(vec2(inormed))) );" SHNL
                       
//                        "_mvar = vec3( (1.0 - _fvar/(1.0+thick)), 0,1);" SHNL
//                        TRACE_MIX_MVAR_WITH_RESULT
//                       );
//}

void FshOVCoordsConstructor::trace_ray_trough(const char* somepoint, const char* size)
{
  m_offset += msprintf(&m_to[m_offset], "vec2 _pt = %s;" SHNL, somepoint);
  m_offset += msprintf(&m_to[m_offset], 
                        "vec2 bz = vec2(step(_pt.x, 0.0)*step(0.0, _pt.x), step(_pt.y, 0.0)*step(0.0, _pt.y));"
                        "_fvar = _pt.x/mix(_pt.y, 1.0, bz[1]);"
                        "_fvar = (1-bz[0])*(1-bz[1])*(_fvar*inormed.y - float(inormed.x))/sqrt(1.0 + _fvar*_fvar) + float(inormed.x)*bz[0] + float(inormed.y)*bz[1];" SHNL
                        "_fvar = min(abs(_fvar), 1.0+thick);" SHNL
                      );
  
  
  m_offset += msprintf(&m_to[m_offset], 
                        "_fvar = mix( _fvar, 1.0+thick, step(0.0, dot(_pt.xy, vec2(inormed))));" SHNL
                        "_mvar = vec2(1.0 - _fvar/(1.0+thick), length(vec2(inormed)));" SHNL
                       );
  if (size != nullptr)
    m_offset += msprintf(&m_to[m_offset], "_mvar[0] = _mvar[0] * step(_mvar[1], %s);", size);  
    
  m_offset += msprintf(&m_to[m_offset], TRACE_MIX_MVAR_WITH_RESULT);
}

void FshOVCoordsConstructor::tex_pickcolor(int palette_param_idx, const char *pickvalue, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s = texture(opm%D_%D, vec2(%s, 1)).rgb;" SHNL, result, m_overlay, palette_param_idx, pickvalue); }

void FshOVCoordsConstructor::tex_addcolor(int palette_param_idx, const char *pickvalue, const char *weight, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s += %s*texture(opm%D_%D, vec2(%s, 1)).rgb;" SHNL, result, weight, m_overlay, palette_param_idx, pickvalue); }

void FshOVCoordsConstructor::tex_meshcolor(int palette_param_idx, const char *pickvalue, const char *mesh, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s = %s*(1.0-%s) + %s*texture(opm%D_%D, vec2(%s, 0.0)).rgb;" SHNL, result, result, mesh, mesh, m_overlay, palette_param_idx, pickvalue); }

void FshOVCoordsConstructor::goto_func_end(bool traced)
{
  if (traced)
  {
    static const char _overtraced[] = "mixwell = result[0];" SHNL;
    memcpy(&m_to[m_offset], _overtraced, sizeof(_overtraced)-1); m_offset += sizeof(_overtraced) - 1;
  }
//  m_offset += msprintf(&m_to[m_offset], "selfposition = ioffset; return vec4(result.x, result.y, 0.0, clamp(mixwell, 0.0, 1.0)); }" SHNL);
  m_offset += msprintf(&m_to[m_offset], "selfposition = ioffset; return vec4(result, mixwell); }" SHNL);
}





