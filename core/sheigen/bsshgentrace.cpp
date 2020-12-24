/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgentrace.h"

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

#include "bsshgenparams.h"


FshTraceGenerator::FshTraceGenerator(const AbstractDrawOverlay::uniforms_t &ufms, int overlay, bool rotated, char *deststring, int ocg_include_bits): 
  m_overlay(overlay), m_rotated(rotated),
  m_writebase(deststring), m_to(deststring), m_offset(0), 
  m_pixingsctr(0), m_relingsctr(0), m_maths(0), m_paramsctr(0), m_prmmemoryiter(0)
{
  loc_uniformsCount = ufms.count;
  loc_uniforms = ufms.arr;
  
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  static const char _overstart[] = "uniform highp int  viewdimm_a;"  SHNL
                                   "uniform highp int  viewdimm_b;"  SHNL
                                   "uniform highp int  scaling_a;" SHNL
                                   "uniform highp int  scaling_b;" SHNL
                                   "vec3 insider(int i, ivec2 ifromvec);" SHNL
                                   ;
  
  /// HEAD
  memcpy(&m_to[m_offset], _overstart, sizeof(_overstart)-1);  m_offset += sizeof(_overstart) - 1;
  
  static const char _overinc_getvalue[] = "uniform highp int countPortions;"  SHNL
                                          "float getValue1D(in int portion, in float x);" SHNL
                                          "float getValue2D(in int portion, in vec2 x);" SHNL;
  if (ocg_include_bits & OINC_GETVALUE){    memcpy(&m_to[m_offset], _overinc_getvalue, sizeof(_overinc_getvalue)-1);  m_offset += sizeof(_overinc_getvalue) - 1;   }
  
  static const char _overinc_random[] =   "float randfloat(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }" SHNL
                                          "vec2  randvec2(vec2 co){ return vec2(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453), fract(sin(dot(co.yx ,vec2(12.9898,78.233))) * 43758.5453)); }" SHNL;
  if (ocg_include_bits & OINC_RANDOM){      memcpy(&m_to[m_offset], _overinc_random, sizeof(_overinc_random)-1);  m_offset += sizeof(_overinc_random) - 1;   }
  
  static const char _overinc_bounds[] =   "uniform highp vec2   databounds;"  SHNL;
  if (ocg_include_bits & OINC_DATABOUNDS){  memcpy(&m_to[m_offset], _overinc_bounds, sizeof(_overinc_bounds)-1);  m_offset += sizeof(_overinc_bounds) - 1;   }
  
  m_offset += msexpandParams(&m_to[m_offset], m_overlay, loc_uniformsCount, loc_uniforms);
}

void FshTraceGenerator::_gtb(OVL_ORIENTATION orient)
{ 
  m_offset += msprintf(&m_to[m_offset], "vec4 overlayTrace%d(in ivec2 icell, in vec4 coords, in float thick, in ivec2 mastercoords, out ivec2 selfposition){" SHNL, 
                       m_overlay);
  
  bool rtn = orient == OO_LRBT || orient == OO_RLBT || orient == OO_LRTB || orient == OO_RLTB;
  bool rty = orient == OO_BTLR || orient == OO_BTRL || orient == OO_TBLR || orient == OO_TBRL;
  
  if (orient == OO_INHERITED || (m_rotated == false && rtn) || (m_rotated == true && rty))
    m_offset += msprintf(&m_to[m_offset], "ivec2 iscaling = ivec2(scaling_a, scaling_b);" SHNL
                                          "ivec2 ibounds = ivec2(viewdimm_a, viewdimm_b) * iscaling;" SHNL );
  else
    m_offset += msprintf(&m_to[m_offset], "ivec2 iscaling = ivec2(scaling_b, scaling_a);" SHNL
                                          "ivec2 ibounds = ivec2(viewdimm_b, viewdimm_a) * iscaling;" SHNL );
  
  {
    const char* coords_orient[] = {
    //  OO_INHERITED
        "coords.pq",
    //  OO_LRBT, OO_RLBT, OO_LRTB, OO_RLTB, 
        "coords.st", "vec2(1.0 - coords.s, coords.t)", "vec2(coords.s, 1.0 - coords.t)", "vec2(1.0 - coords.s, 1.0 - coords.t)",
    //  OO_TBLR, OO_BTLR, OO_TBRL, OO_BTRL,
        "vec2(1.0 - coords.t, coords.s)", "coords.ts", "vec2(1.0 - coords.t, 1.0 - coords.s)", "vec2(coords.t, 1.0 - coords.s)",
    // OO_IHBT, OO_IHTB, OO_LRIH, OO_RLIH
        "coords.pt", "vec2(coords.p, 1.0 - coords.t)", "coords.sq", "vec2(1.0 - coords.s, coords.q)"
    };
    m_offset += msprintf(&m_to[m_offset], //"ivec2 icoords = ivec2(floor(coords.pq*ibounds + vec2(0.49,0.49)));" SHNL /// ??1
                                          "ivec2 icoords = ivec2(floor(%s*ibounds));" SHNL
                                          , coords_orient[int(orient)]);
  }

  static const char _vars[] =             "vec3 result=vec3(0.0,0.0,0.0);" SHNL
                                          "float mixwell = 0.0;" SHNL
                                          "vec3 _mvar;  float _fvar;" SHNL
                                          "vec3 _insban = vec3(1.0,1.0,1.0);" SHNL;
  memcpy(&m_to[m_offset], _vars, sizeof(_vars)-1); m_offset += sizeof(_vars) - 1;
}

void FshTraceGenerator::_gtb_coords(const _bs_unzip_t &bsu)
{
  if (bsu.type)
  {
    int coordspixing = add_movecs_pixing(bsu.cr);
    m_offset += msprintf(&m_to[m_offset], "ivec2 ioffset = ivec2(floor((");
    if (bsu.type == 1)
      m_offset += msprintf(&m_to[m_offset], "vec2(%f, %f)", bsu.ffs[0], bsu.ffs[1]);
    else if (bsu.type >= 2)
      m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
//    m_offset += msprintf(&m_to[m_offset],  " * movecs_pixing%d) + vec2(0.49,0.49)));" SHNL, coordspixing);    /// ??2
    m_offset += msprintf(&m_to[m_offset],  " * movecs_pixing%d)));" SHNL, coordspixing);
    m_offset += msprintf(&m_to[m_offset],  "ioffset = ioffset + mastercoords;" SHNL);
  }
  else
    m_offset += msprintf(&m_to[m_offset],  "ivec2 ioffset = ivec2(0,0);\n");
  
}

void FshTraceGenerator::_gtb_dimms(const _bs_unzip_t &bsu)
{
  if (bsu.type)
  {
    int dimmpixing = add_movecs_pixing(bsu.cr);
    
    if (bsu.type == 1 || bsu.type == 2)
    {
      if (bsu.cr <= CR_XREL_YABS)               /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,   (SCALED variants)
      {
        m_offset += msprintf(&m_to[m_offset], "_fvar = mix(movecs_pixing%d.x, movecs_pixing%d.y, step(float(movecs_pixing%d.y), float(movecs_pixing%d.x)));" SHNL, dimmpixing,dimmpixing,dimmpixing,dimmpixing);
      }
      else if (bsu.cr >= CR_XABS_YABS_NOSCALED_SCALED)   /// all NOSCALED_SCALED or SCALED_NOSCALED variants
      {
        const char endings[] = {  'y', 'x', 
                                  'y', 'x',
                                  'y', 'x', 
                                  'y', 'x'
                               };
        m_offset += msprintf(&m_to[m_offset], "_fvar = movecs_pixing%d.%c;" SHNL, dimmpixing, endings[bsu.cr - CR_XABS_YABS_NOSCALED_SCALED]);
      }
      else                                      /// last NOSCALED variants
        m_offset += msprintf(&m_to[m_offset], "_fvar = 1;" SHNL);
      
      m_offset += msprintf(&m_to[m_offset],  "int idimms1 = int(floor(");
      if (bsu.type == 1)
        m_offset += msprintf(&m_to[m_offset], "%f", bsu.ffs[0]);
      else if (bsu.type == 2)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * _fvar + 0.49));" SHNL, dimmpixing);
    }
    else if (bsu.type == 3 || bsu.type == 4)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec2 idimms2 = ivec2(floor(");
      if (bsu.type == 3)
        m_offset += msprintf(&m_to[m_offset], "vec2(%f, %f)", bsu.ffs[0], bsu.ffs[1]);
      else if (bsu.type == 4)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * vec2(movecs_pixing%d.x, movecs_pixing%d.y) + vec2(0.49,0.49)));" SHNL, dimmpixing,dimmpixing);
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
      m_offset += msprintf(&m_to[m_offset],  " * vec4(movecs_pixing%d.x, movecs_pixing%d.y, movecs_pixing%d.x, movecs_pixing%d.y)) + vec4(0.49,0.49,0.49,0.49)));" SHNL, dimmpixing,dimmpixing,dimmpixing,dimmpixing);
    }
    else if (bsu.type == 7)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec2 idimms2 = ivec2(floor(opm%D_%D * vec2(movecs_pixing%d.x, movecs_pixing%d.y) + vec2(0.49,0.49)));" SHNL, m_overlay, m_paramsctr++, dimmpixing, dimmpixing);
    }
  }
}


void FshTraceGenerator::math_pi()
{
  static const char _overmath[] = "const float PI = 3.1415926;" SHNL;
  memcpy(&m_to[m_offset], _overmath, sizeof(_overmath)-1); m_offset += sizeof(_overmath) - 1;
  m_maths |= (1 << (int)MM_PI);
}

int FshTraceGenerator::add_movecs_pixing(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                                         "ivec2 movecs_pixing%d = iscaling;" SHNL,
                                         "ivec2 movecs_pixing%d = ibounds;" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(iscaling.x, ibounds.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(ibounds.x, iscaling.y);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "ivec2 movecs_pixing%d = ivec2(1, 1);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(viewdimm_a, viewdimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(1, viewdimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(viewdimm_a, 1);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "ivec2 movecs_pixing%d = ivec2(1, iscaling.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(iscaling.x, 1);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(viewdimm_a, ibounds.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(ibounds.x, viewdimm_b);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "ivec2 movecs_pixing%d = ivec2(1, ibounds.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(iscaling.x, viewdimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(ibounds.x, 1);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(viewdimm_a, iscaling.y);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_pixingsctr);
  return m_pixingsctr++;
}

int FshTraceGenerator::add_movecs_rel(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
//                                         "vec2 movecs_rel_%d = vec2(viewdimm_a-1, viewdimm_b-1);" SHNL,   // ntf!
                                         "vec2 movecs_rel_%d = vec2(viewdimm_a, viewdimm_b);" SHNL,   // ntf!
                                         "vec2 movecs_rel_%d = vec2(1.0, 1.0);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(viewdimm_a, 1);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, viewdimm_b);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "vec2 movecs_rel_%d = vec2(ibounds);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, ibounds.y);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "vec2 movecs_rel_%d = vec2(viewdimm_a, ibounds.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, viewdimm_b);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, 1);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, 1);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(viewdimm_a, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, ibounds.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, viewdimm_b);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_relingsctr);
  return m_relingsctr++;
}

void FshTraceGenerator::param_alias(const char *name, int memslot)
{
  if (memslot == -1)
  {
    m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D;" SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name, m_overlay, m_paramsctr);
    m_paramsctr++;
  }
  else
    m_offset += msprintf(&m_to[m_offset], "%s %s = opm%D_%D;" SHNL, glsl_types[loc_uniforms[m_prmmemory[memslot]].type], name, m_overlay, m_prmmemory[memslot]);
}

int FshTraceGenerator::param_push()
{
  int memslot = m_prmmemoryiter;
  m_prmmemory[m_prmmemoryiter++] = m_paramsctr++;
  return memslot;
}

void FshTraceGenerator::param_get()
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr);
  m_paramsctr++;
}

void FshTraceGenerator::param_mem(int memslot)
{
  m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_prmmemory[memslot]);
}

void FshTraceGenerator::param_pass()
{
  m_paramsctr++;
}

void FshTraceGenerator::param_for_arr_begin(const char *name, const char *arrlengthname, const char* additname)
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

void FshTraceGenerator::param_for_rarr_begin(const char *name)  // DT_ARR, DT_1I, DT_1I
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

void FshTraceGenerator::param_for_oarr_begin(const char *name_cur, const char *name_next, const char *arrlengthname)  // DT_ARR, DT_1I
{
  m_offset += msprintf(&m_to[m_offset],  "%s %s = opm%D_%D[0]; %s %s; " SHNL, glsl_types[loc_uniforms[m_paramsctr].type], name_cur, m_overlay, m_paramsctr, glsl_types[loc_uniforms[m_paramsctr].type], name_next);
  m_offset += msprintf(&m_to[m_offset],  "int %s = opm%D_%D;" SHNL
                      "for (int i=1; i<%s; i++){" SHNL
                      "%s = opm%D_%D[i];" SHNL,  arrlengthname, m_overlay, m_paramsctr + 1,
                      arrlengthname, 
                      name_next, m_overlay, m_paramsctr);
  m_paramsctr += 2;
}

void FshTraceGenerator::param_for_end() {  m_offset += msprintf(&m_to[m_offset], "}");  }

void FshTraceGenerator::goto_normed()
{
  m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ioffset;" SHNL);
}

void FshTraceGenerator::goto_normed(const char *someparam, int pixing, bool saveasoffset)
{
  if (saveasoffset)
  {
    m_offset += msprintf(&m_to[m_offset], "ioffset = ivec2(floor((%s * movecs_pixing%d) + vec2(0.49,0.49)));" SHNL, someparam, pixing);
    m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ioffset;" SHNL);
  }
  else
    m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = icoords - ivec2(floor((%s * movecs_pixing%d) + vec2(0.49,0.49)));" SHNL, someparam, pixing);
}

void FshTraceGenerator::goto_normed_empty()
{
  m_offset += msprintf(&m_to[m_offset], "ivec2 inormed = ivec2(0,0);");
}

void FshTraceGenerator::goto_normed_rotated(const char *angleRadName)
{
  m_offset += msprintf(&m_to[m_offset], "mat2 rotationMatrix = mat2 (cos(%s), -sin(%s), sin(%s), cos(%s));  inormed = inormed*rotationMatrix;", angleRadName, angleRadName, angleRadName, angleRadName);
}

void FshTraceGenerator::var_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "float %s = %f;", name, value); }

void FshTraceGenerator::var_const_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %f;", name, value); }

void FshTraceGenerator::var_inline(const char* name, float v){  m_offset += msprintf(&m_to[m_offset], name, v); }

void FshTraceGenerator::var_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "int %s = %d;", name, value); }

void FshTraceGenerator::var_const_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;", name, value); }

void FshTraceGenerator::var_inline(const char* name, int v){  m_offset += msprintf(&m_to[m_offset], name, v); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "vec2 %s = vec2(%f, %f);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%f, %f);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_inline(const char* name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], name, v1, v2); }

void FshTraceGenerator::var_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_const_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_inline(const char* name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], name, v1, v2); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "vec3 %s = vec3(%f, %f, %f);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%f, %f, %f);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_inline(const char* name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], name, v1, v2, v3); }

void FshTraceGenerator::var_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_const_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "const ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_inline(const char* name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], name, v1, v2, v3); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "vec4 %s = vec4(%f, %f, %f, %f);" SHNL, name, v1, v2, v3, v4); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "const vec4 %s = vec4(%f, %f, %f, %f);" SHNL, name, v1, v2, v3, v4); }

void FshTraceGenerator::var_static(DTYPE type, const char *name_eq_value){  m_offset += msprintf(&m_to[m_offset], "%s %s;" SHNL, glsl_types[(int)type], name_eq_value); }

void FshTraceGenerator::var_static(const char *name, const char *value){  m_offset += msprintf(&m_to[m_offset], "%s = %s;" SHNL, name, value); }

void FshTraceGenerator::var_const_static(DTYPE type, const char *name_eq_value){  m_offset += msprintf(&m_to[m_offset], "const %s %s;" SHNL, glsl_types[(int)type], name_eq_value); }

void FshTraceGenerator::movecs_pix_x(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor((%s*movecs_pixing%d.x) + 0.49);" SHNL, name, name, resc_idx); }

void FshTraceGenerator::movecs_pix_y(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor((%s*movecs_pixing%d.y) + 0.49);" SHNL, name, name, resc_idx); }

void FshTraceGenerator::movecs_pix(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = floor((%s*movecs_pixing%d) + 0.49);" SHNL, name, name, resc_idx); }

void FshTraceGenerator::push_cs_rel_x(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/movecs_rel_%d.x;" SHNL, name, name, resc_idx); }

void FshTraceGenerator::push_cs_rel_y(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/movecs_rel_%d.y;" SHNL, name, name, resc_idx); }

void FshTraceGenerator::push_cs_rel(const char *name, int resc_idx){  m_offset += msprintf(&m_to[m_offset], "%s = %s/movecs_rel_%d;" SHNL, name, name, resc_idx); }

void FshTraceGenerator::pop_cs_rel_x(const char *name){  m_offset += msprintf(&m_to[m_offset], "%s = floor(%s*(ibounds.x) + 0.49);" SHNL, name, name); }

void FshTraceGenerator::pop_cs_rel_y(const char *name){  m_offset += msprintf(&m_to[m_offset], "%s = floor(%s*(ibounds.y) + 0.49);" SHNL, name, name); }

void FshTraceGenerator::pop_cs_rel(const char *name){  m_offset += msprintf(&m_to[m_offset], "%s = floor(%s*ibounds + 0.49);" SHNL, name, name); }

void FshTraceGenerator::push(const char *sztext)
{
  while (*sztext != '\0')
    m_to[m_offset++] = *sztext++;
}

void FshTraceGenerator::push(const char* text, unsigned int len)
{
  if (len > 0)
  {
    memcpy(&m_to[m_offset], text, len - 1);
    m_offset += len - 1;
  }
}

void FshTraceGenerator::inside_begin1(const char *limrad1){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(-%s,float(inormed.x))*step(-%s,float(inormed.y))*(1-step(%s + 1.0, float(inormed.x)))*(1-step(%s + 1.0, float(inormed.y))));   if (inside != 0){" SHNL,
                                                                              limrad1, limrad1, limrad1, limrad1); }

void FshTraceGenerator::inside_begin2(const char *limits2){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1-step(%s[0], float(inormed.x)))*(1-step(%s[1], float(inormed.y))));   if (inside != 0){" SHNL, limits2, limits2); }

void FshTraceGenerator::inside_begin4(const char *limits4){  m_offset += msprintf(&m_to[m_offset], "int inside = int(step(%s[0],float(inormed.x))*step(%s[2],float(inormed.y))*(1-step(%s[1] + 1.0, float(inormed.x)))*(1-step(%s[3] + 1.0, float(inormed.y))));   if (inside != 0){" SHNL, 
                                                                              limits4, limits4, limits4, limits4); }

void FshTraceGenerator::inside_end(){ m_offset += msprintf(&m_to[m_offset], "}"); }

void FshTraceGenerator::ban_trace(bool ban)
{
  if (ban)
    m_offset += msprintf(&m_to[m_offset], "_insban = vec3(1.0, 0.0, 1.0);");
  else
    m_offset += msprintf(&m_to[m_offset], "_insban = vec3(1.0, 1.0, 1.0);");
}

//void FshTraceGenerator::place_rect_lb(const char *rdimms, const char *inside_name)
//{
//  m_offset += msprintf(&m_to[m_offset], "int %s = step(0.0,inormed.x)*step(0.0,inormed.y)*(1-step(%s[0], inormed.x))*(1-step(%s[1], inormed.y));" SHNL, inside_name, rdimms, rdimms);
//}

//void FshTraceGenerator::place_rect_cc(const char *r2dimms, const char *inside_name)
//{
//  m_offset += msprintf(&m_to[m_offset], "int %s = step(-%s[0] + 1,inormed.x)*step(-%s[1] + 1,inormed.y)*(1-step(%s[0] , inormed.x))*(1-step(%s[1] , inormed.y));" SHNL, inside_name, r2dimms, r2dimms, r2dimms, r2dimms);
//}

//void FshTraceGenerator::place_line(const char *ABC, const char *border, const char *inside_name)
//{
//  m_offset += msprintf(&m_to[m_offset], "int %s = 1.0 - step(%s, abs(%s.x*inormed.x + %s.y*inormed.y + %s.z));" SHNL, inside_name, border, ABC, ABC, ABC);
//}


#define TRACE_MIX_MVAR_WITH_RESULT "result = mix(result, _mvar, 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );" SHNL

void FshTraceGenerator::trace_triangle_cc_begin(const char *) {}

void FshTraceGenerator::trace_triangle_cc_end(const char *side, int direction, float fillcoeff)
{
  char rd[32];    msprintf(rd, "(%s)", side);
  char rd_plus[32];     msprintf(rd_plus, "(0.707*%s)", side);
  char rd_minus[32];    msprintf(rd_minus, "(-0.707*%s)", side);
  
  m_offset += msprintf(&m_to[m_offset],  "_mvar[0] = float(inormed.y) + %s + 0.49 - 1.0;" SHNL
                                        "_mvar[1] = %s;" SHNL, 
                      direction == 0 || direction == 2? rd_plus :
                      direction == 1 || direction == 3? rd_minus : "",
                      direction == 0 || direction == 2? "1.0 - step(_mvar[0], 0.0)" :
                      direction == 1 || direction == 3? "1.0 - step(0.0, _mvar[0])" :
                                      "0.0"
                                      );
  
  m_offset += msprintf(&m_to[m_offset],
                          "_mvar[0] = 1.0 - (abs(_mvar[0]))/(%s*0.866*2.0);"
                          "_mvar[2] = _mvar[1]*_mvar[0]*%s - 0.49;" SHNL    /// otrezok
                          "_fvar = (1.0+thick - clamp(float(abs(inormed.x) - int(_mvar[2])), 0.0, 1.0+thick))/(1.0+thick) * (1.0-step(_mvar[2], 0.1));" SHNL
                          "_mvar[2] = step(float(abs(inormed.x)), _mvar[2]-1.0);"
                          "_fvar = mix(_fvar, %F, _mvar[2]);" SHNL
                          "_mvar = vec3(_mvar[1]*_fvar, _mvar[0]*(1-_mvar[2]), %s*2.31);" SHNL /// 2.31 - trace coeff
                          TRACE_MIX_MVAR_WITH_RESULT
                      ,
                      side, side, fillcoeff, side
                      );
  trace_2linehorz_c(rd, nullptr, (direction == 0 || direction == 2? rd_minus:
                                  direction == 1 || direction == 3? rd_plus:
                                                                    ""));
}


void FshTraceGenerator::trace_rect_xywh_begin(const char *) {}

void FshTraceGenerator::trace_rect_xywh_end(const char *wh, float fillcoeff, const char *crosslimit)
{
  char r0[32], r1[32], r01[64], r11[64]; msprintf(r0, "%s[0]", wh); msprintf(r1, "%s[1]", wh); msprintf(r01, "%s[0]-sign(%s[0])", wh,wh); msprintf(r11, "%s[1]-sign(%s[1])", wh,wh); 
  trace_linehorz_l(r0, nullptr, nullptr, crosslimit); trace_linehorz_l(r0, nullptr, r11, crosslimit);
  trace_linevert_b(r1, nullptr, nullptr, crosslimit); trace_linevert_b(r1, nullptr, r01, crosslimit);
  if (fillcoeff > 0.0f)
  {
    m_offset += msprintf(&m_to[m_offset],
//                        "_mvar = vec3(step(0.0, inormed[0])*step(inormed[0], %s)*step(0.0, inormed[1])*step(inormed[1], %s), 0.0, 1.0)*_insban;" SHNL
                        "_mvar = vec3( "
                        "step(float(abs(inormed[0])), float(abs(%s)))*(1.0-step(float(sign(inormed[0])*sign(%s)),0.0))*"
                        "step(float(abs(inormed[1])), float(abs(%s)))*(1.0-step(float(sign(inormed[1])*sign(%s)),0.0)),"
                        "0.0, 1.0)*_insban;" SHNL
                        "result = mix(result, vec3(_mvar.r * %F, _mvar.gb), _mvar[0] * (1.0-step(1.0, result[0])) );" SHNL,
                        r01, r01, r11, r11,
//                         r01, r11, 
                         fillcoeff);
  }
}

void FshTraceGenerator::trace_rect_cc_begin(const char *) {}

void FshTraceGenerator::trace_rect_cc_end(const char *rdimms, float fillcoeff, const char *crosslimit)
{   
  char r0[32], r1[32], r01[64], r11[64]; msprintf(r0, "%s[0]", rdimms); msprintf(r1, "%s[1]", rdimms); msprintf(r01, " (%s[0]-sign(%s[0]))", rdimms,rdimms); msprintf(r11, " (%s[1]-sign(%s[1]))", rdimms,rdimms); 
  trace_2linehorz_c(r0, nullptr, &r11[1], crosslimit); r11[0] = '-'; trace_2linehorz_c(r0, nullptr, r11, crosslimit);
  trace_2linevert_c(r1, nullptr, &r01[1], crosslimit); r01[0] = '-'; trace_2linevert_c(r1, nullptr, r01, crosslimit);
  if (fillcoeff > 0.0f)
  {
    m_offset += msprintf(&m_to[m_offset],
                        "_mvar = vec3(step(float(abs(inormed[0])), abs(%s)) * step(float(abs(inormed[1])), float(abs(%s))), 0.0, 1.0)*_insban;" SHNL
                        "result = mix(result, vec3(_mvar.r * %F, _mvar.gb), _mvar[0] * (1.0 - step(1.0, result[0])) );" SHNL,
                        r11, r01, fillcoeff);
  }
}

void FshTraceGenerator::trace_square_lb_begin(const char *aside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sqdimms = vec2(%s, %s);", aside, aside);  trace_rect_xywh_begin("_sqdimms"); }

void FshTraceGenerator::trace_square_lb_end(float fillcoeff) {  trace_rect_xywh_end("_sqdimms", fillcoeff); }

void FshTraceGenerator::trace_square_cc_begin(const char *halfside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sq2dimms = vec2(%s, %s);", halfside, halfside);  trace_rect_cc_begin("_sq2dimms"); }

void FshTraceGenerator::trace_square_cc_end(float fillcoeff) {  trace_rect_cc_end("_sq2dimms", fillcoeff); }

void FshTraceGenerator::trace_circle_cc_begin(const char *radius, const char *border) { m_offset += msprintf(&m_to[m_offset], "vec4 _cd = vec4(%s-1, %s + thick, inormed.x*inormed.x + inormed.y*inormed.y, %s);", radius, border, border); }

void FshTraceGenerator::trace_circle_cc_end(float fillcoeff/*, bool notraceinside*/)
{
  if ((m_maths & (1 << MM_PI)) == 0)  math_pi();
  if (fillcoeff > 0.0f)
    m_offset += msprintf(&m_to[m_offset],  "vec3 r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3 r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_fill = 1-step(r2[1], _cd[2]);" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*mixwell_fill;" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
//                                          "_mvar = vec3(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0])*_insban;" SHNL
                         "_mvar = vec3(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))/(2*PI)*step(mixwell_fill, 0.0), 2*PI*_cd[0])*_insban;" SHNL
                                          TRACE_MIX_MVAR_WITH_RESULT
                        , fillcoeff
                        );    
  else
    m_offset += msprintf(&m_to[m_offset],  "vec3  r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3  r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_onborder = (1 - step(r3[2], _cd[2])) * (step(r3[0], _cd[2])) * abs(sign(thick));" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*(1 - step(r2[1], _cd[2]));" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
                                          "_mvar = vec3(clamp(mixwell_onborder + mixwell_before + mixwell_aftere, 0.0,1.0), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0])*_insban;" SHNL
                                          TRACE_MIX_MVAR_WITH_RESULT
                        );
}

//                                              crosslimit                                           offset                                                 
#define _TRACE_MAIN_CONDITION(idx)  "_fvar = %s * (1.0+thick - clamp(abs(inormed["#idx"] - floor(%s + 0.49)), 0.0, 1.0+thick))/(1.0+thick);"
//#define _TRACE_MAIN_CONDITION(idx)  "_fvar = %s * (1.0 - clamp((inormed["#idx"]-floor(%s+0.49))/(1.0 + thick), 0.0, 1.0));"


void FshTraceGenerator::trace_2linehorz_c(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(abs(inormed[0]), ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,                        
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_2linevert_c(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(abs(inormed[1]), ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_linehorz_l(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(inormed[0], ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_linehorz_r(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(-inormed[0], ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_linevert_t(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(-inormed[1], ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_linevert_b(const char *isize, const char *igap, const char *ioffset, const char *icrosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(inormed[1], ivec2(%s, %s))*_insban;" TRACE_MIX_MVAR_WITH_RESULT,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      ioffset == nullptr? "0.0" : ioffset,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_lines_x(const char *isize, const char *igap, const char *icrosslimit)
{
  m_offset += msprintf(&m_to[m_offset], 
                      "_mvar[0] = atan(float(abs(inormed.x)), float(abs(inormed.y)));" SHNL
                      "_mvar[1] = length(vec2(inormed))*cos(0.785398 - _mvar[0]);" SHNL
                      "_mvar[2] = _mvar[1]*0.707;" SHNL
                      "_mvar.xy = vec2(_mvar[2], _mvar[2]);" SHNL
                      "_fvar = %s * mix((1.0+thick - clamp(distance(abs(inormed), _mvar.xy), 0.0, 1.0+thick))/(1.0+thick), 1.0, step(length(vec2(inormed)), 0.0));" SHNL
                      "_mvar = vec3(_fvar, sign(_fvar), sign(_fvar)) * insider(int(_mvar[2]), ivec2(int(%s), int(%s)))*_insban;" SHNL
                      TRACE_MIX_MVAR_WITH_RESULT
                      ,
                      icrosslimit == nullptr? "1.0" : icrosslimit,
                      igap == nullptr? "0" : igap,
                      isize == nullptr? "5000" : isize);
}

void FshTraceGenerator::trace_line_from_normed_to(const char *inormedendpoint)
{ 
  m_offset += msprintf(&m_to[m_offset], "_mvar.xy = %s;" SHNL, inormedendpoint);
#if true
  m_offset += msprintf(&m_to[m_offset], 
                        "vec2 bz = vec2(step(_mvar.x, 0.0)*step(0.0, _mvar.x), step(_mvar.y, 0.0)*step(0.0, _mvar.y));"
                        "_mvar.z = _mvar.x/mix(_mvar.y, 1.0, bz[1]);"
                        "_fvar = (1-bz[0])*(1-bz[1])*(_mvar.z*inormed.y - float(inormed.x))/sqrt(1.0 + _mvar.z*_mvar.z) + float(inormed.x)*bz[0] + float(inormed.y)*bz[1];" SHNL
                      );
#else
  m_offset += msprintf(&m_to[m_offset],
                        "if (_mvar.x == 0.0)    _fvar = float(inormed.x);" SHNL
                        "else if (_mvar.y == 0.0)    _fvar = float(inormed.y);" SHNL
                        "else{  _mvar.z = _mvar.x/_mvar.y;  _fvar = (_mvar.z*inormed.y - float(inormed.x))/sqrt(1.0 + _mvar.z*_mvar.z); }" SHNL
                      );
#endif
//  m_offset += msprintf(&m_to[m_offset], 
//                        "_mvar.z = step(0.0, sign(dot(_mvar.xy, inormed)));" SHNL    /// quarter
////                       "_mvar = vec3(step(_fvar, 0.49 + thick)*step(-0.49 - thick, _fvar)*step(_mvar.y*_mvar.y - _fvar*_fvar, _mvar.x*_mvar.x) * _mvar.z, 0,1)*_insban;" SHNL
////                        "_mvar.xy = vec2(length(_mvar.xy) + 1+thick-_fvar, length(vec2(inormed)));" SHNL
                       
////                        "_fvar = max(min(abs(_fvar), 1.0+thick), min(length(_mvar.xy + vec2(inormed)), 1.0+thick) );" SHNL
                       
//                        "_mvar.xy = vec2(length(_mvar.xy), length(vec2(inormed)));" SHNL
////                        "_fvar = min(abs(_fvar), 1.0+thick)*(_mvar.x + 1.0 + thick)/(_mvar.x);" SHNL
////                        "_fvar = min(abs(_fvar), 1.0+thick)*max(1.0, (_mvar.y)/(_mvar.x + 1.0+thick));" SHNL
//                        "_fvar = min(abs(_fvar), 1.0+thick);" SHNL
////                        "_mvar.x = _mvar.x + 1.0 + thick;"
////                        "_mvar = vec3( (1.0 - _fvar/(1.0+thick))*step(_mvar.y*_mvar.y - _fvar*_fvar, (_mvar.x+1.0+thick)*(_mvar.x+1.0+thick)) * _mvar.z *clamp((_mvar.y - _mvar.x)/(1.0 + thick), 0.0, 1.0) , 0,1)*_insban;" SHNL
//                       "_mvar = vec3( (1.0 - _fvar/(1.0+thick))*step(_mvar.y*_mvar.y - _fvar*_fvar, (_mvar.x)*(_mvar.x)) * _mvar.z, 0,1)*_insban;" SHNL
//                       TRACE_MIX_MVAR_WITH_RESULT
                       
////                       "result = mix(result, _mvar, 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );" SHNL
////                       "result = mix(result, _mvar, 1.0 - step(_mvar[0], 0.0) );" SHNL
////                        "result = mix(result, _mvar, 1.0 - step(_mvar[0], 0.0) );" SHNL
//                       );
  
  m_offset += msprintf(&m_to[m_offset], 
                        "_mvar.z = step(0.0, sign(dot(_mvar.xy, inormed)));" SHNL    /// quarter
                        "vec3 _lens = vec3(length(_mvar.xy), length(vec2(inormed)), length(_mvar.xy) + 1.0 + thick);" SHNL
                        "_fvar = min(abs(_fvar), 1.0+thick);" SHNL
                        "_fvar = mix( _fvar, min(length(_mvar.xy - vec2(inormed)), 1.0+thick), step(_lens[0], _lens[1]));" SHNL
                        "_mvar = vec3( (1.0 - _fvar/(1.0+thick))*step(_lens.y*_lens.y - _fvar*_fvar, (_lens.z)*(_lens.z)) * _mvar.z, 0,1)*_insban;" SHNL
                        TRACE_MIX_MVAR_WITH_RESULT
                       );
}

void FshTraceGenerator::tex_pickcolor(int palette_param_idx, const char *pickvalue, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s = texture(opm%D_%D, vec2(%s, 1)).rgb;" SHNL, result, m_overlay, palette_param_idx, pickvalue); }

void FshTraceGenerator::tex_addcolor(int palette_param_idx, const char *pickvalue, const char *weight, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s += %s*texture(opm%D_%D, vec2(%s, 1)).rgb;" SHNL, result, weight, m_overlay, palette_param_idx, pickvalue); }

void FshTraceGenerator::tex_meshcolor(int palette_param_idx, const char *pickvalue, const char *mesh, const char *result){  m_offset += msprintf(&m_to[m_offset], "%s = %s*(1.0-%s) + %s*texture(opm%D_%D, vec2(%s, 0.0)).rgb;" SHNL, result, result, mesh, mesh, m_overlay, palette_param_idx, pickvalue); }

void FshTraceGenerator::goto_func_end(bool traced)
{
  if (traced)
  {
    static const char _overtraced[] = "mixwell = result[0];" SHNL;
    memcpy(&m_to[m_offset], _overtraced, sizeof(_overtraced)-1); m_offset += sizeof(_overtraced) - 1;
  }
  m_offset += msprintf(&m_to[m_offset], "selfposition = ioffset; return vec4(result, clamp(mixwell, 0.0, 1.0)); }" SHNL);
}





