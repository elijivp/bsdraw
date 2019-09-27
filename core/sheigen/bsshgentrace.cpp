#include "bsshgentrace.h"

#include <memory.h>
#include <stdio.h>

static const char* glsl_types[] = { "float", "vec2", "vec3", "vec4", 
                                    "float", "vec2", "vec3", "vec4", 
                                    "int",  "ivec2", "ivec3", "ivec4", 
//                                    "vec2", "vec3", "vec4", 
                                    "sampler2D", "int", "ivec2", "ivec3", "ivec4",
                                    "sampler2D", "sampler2D", "sampler2D", 
                                    
                                    /// Special types
                                    "_HC_typecounter", "sampler2D", 
                                    
                                     };

#define SHNL "\n"

extern int msprintf(char* to, const char* format, ...);

FshTraceGenerator::FshTraceGenerator(const AbstractOverlay::uniforms_t &ufms, int overlay, bool rotated, char *deststring, int ocg_include_bits): 
  m_overlay(overlay), m_rotated(rotated),
  m_writebase(deststring), m_to(deststring), m_offset(0), 
  m_pixingsctr(0), m_relingsctr(0), m_maths(0), m_paramsctr(0), m_prmmemoryiter(0)
{
  loc_uniformsCount = ufms.count;
  loc_uniforms = ufms.arr;
  
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  static const char _overstart[] = "uniform highp int  datadimm_a;"  SHNL
                                   "uniform highp int  datadimm_b;"  SHNL
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
  
//  static const char _overinc_bounds[] =   "uniform highp vec2       bounds;"  SHNL
//                                          "uniform highp vec2       contrast;" SHNL;
//  if (ocg_include_bits & OINC_BOUNDS){      memcpy(&m_to[m_offset], _overinc_bounds, sizeof(_overinc_bounds)-1);  m_offset += sizeof(_overinc_bounds) - 1;   }
  
  
  /// PARAMS
  for (unsigned int i=0; i<loc_uniformsCount; i++)
  {
    if (loc_uniforms[i].type >= DT__HC_SPECIAL_TYPES)
    {
      m_paramsctr++;
      continue;
    }
    
    int glsl_types_idx = (int)loc_uniforms[i].type;
    if (glsl_types_idx < (int)sizeof(glsl_types))
    {
      if (loc_uniforms[i].type == DT_ARR || loc_uniforms[i].type == DT_ARR2 || loc_uniforms[i].type == DT_ARR3 || loc_uniforms[i].type == DT_ARR4 || 
          loc_uniforms[i].type == DT_ARRI || loc_uniforms[i].type == DT_ARRI2 || loc_uniforms[i].type == DT_ARRI3 || loc_uniforms[i].type == DT_ARRI4)
      {
        const dmtype_arr_t* parr = (const dmtype_arr_t*)loc_uniforms[i].dataptr;
        m_offset += msprintf(&m_to[m_offset], "uniform highp %s opm%D_%D[%d];\n", glsl_types[glsl_types_idx], m_overlay, i, parr->count);
      }
      else
        m_offset += msprintf(&m_to[m_offset], "uniform highp %s opm%D_%D;\n", glsl_types[glsl_types_idx], m_overlay, i);
    }
    else
      m_offset += msprintf(&m_to[m_offset], "ERROR on param %d;\n", m_overlay);
  }
}

void FshTraceGenerator::_gtb(OVL_ORIENTATION orient)
{ 
  m_offset += msprintf(&m_to[m_offset], "vec4 overlayTrace%d(in vec4 coords, in float density, in ivec2 mastercoords, out ivec2 selfposition){" SHNL, 
                       m_overlay);
  
  bool rtn = orient == OO_LRBT || orient == OO_RLBT || orient == OO_LRTB || orient == OO_RLTB;
  bool rty = orient == OO_BTLR || orient == OO_BTRL || orient == OO_TBLR || orient == OO_TBRL;
  
  if (orient == OO_INHERITED || (m_rotated == false && rtn) || (m_rotated == true && rty))
    m_offset += msprintf(&m_to[m_offset], "ivec2 iscaling = ivec2(scaling_a, scaling_b);" SHNL
                                          "ivec2 ibounds = ivec2(datadimm_a, datadimm_b) * iscaling;" SHNL );
  else
    m_offset += msprintf(&m_to[m_offset], "ivec2 iscaling = ivec2(scaling_b, scaling_a);" SHNL
                                          "ivec2 ibounds = ivec2(datadimm_b, datadimm_a) * iscaling;" SHNL );
  
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
                                          "vec3 _insvar;   vec3 _insban = vec3(1.0,1.0,1.0);   float _densvar;" SHNL;
  memcpy(&m_to[m_offset], _vars, sizeof(_vars)-1); m_offset += sizeof(_vars) - 1;
}

void FshTraceGenerator::_gtb_coords(const _bs_unzip_t &bsu)
{
  if (bsu.type)
  {
    int coordspixing = add_movecs_pixing(bsu.cr);
    m_offset += msprintf(&m_to[m_offset], "ivec2 ioffset = ivec2(floor((");
    if (bsu.type == 1)
      m_offset += msprintf(&m_to[m_offset], "vec2(%F, %F)", bsu.ffs[0], bsu.ffs[1]);
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
        m_offset += msprintf(&m_to[m_offset], "_densvar = mix(movecs_pixing%d.x, movecs_pixing%d.y, step(float(movecs_pixing%d.y), float(movecs_pixing%d.x)));" SHNL, dimmpixing,dimmpixing,dimmpixing,dimmpixing);
      }
      else if (bsu.cr >= CR_XABS_YABS_NOSCALED_SCALED)   /// all NOSCALED_SCALED or SCALED_NOSCALED variants
      {
        const char endings[] = {  'y', 'x', 
                                  'y', 'x',
                                  'y', 'x', 
                                  'y', 'x'
                               };
        m_offset += msprintf(&m_to[m_offset], "_densvar = movecs_pixing%d.%c;" SHNL, dimmpixing, endings[bsu.cr - CR_XABS_YABS_NOSCALED_SCALED]);
      }
      else                                      /// last NOSCALED variants
        m_offset += msprintf(&m_to[m_offset], "_densvar = 1;" SHNL);
      
      m_offset += msprintf(&m_to[m_offset],  "int idimms1 = int(floor(");
      if (bsu.type == 1)
        m_offset += msprintf(&m_to[m_offset], "%F", bsu.ffs[0]);
      else if (bsu.type == 2)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * _densvar + 0.49));" SHNL, dimmpixing);
    }
    else if (bsu.type == 3 || bsu.type == 4)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec2 idimms2 = ivec2(floor(");
      if (bsu.type == 3)
        m_offset += msprintf(&m_to[m_offset], "vec2(%F, %F)", bsu.ffs[0], bsu.ffs[1]);
      else if (bsu.type == 4)
        m_offset += msprintf(&m_to[m_offset], "opm%D_%D", m_overlay, m_paramsctr++);
      m_offset += msprintf(&m_to[m_offset],  " * vec2(movecs_pixing%d.x, movecs_pixing%d.y) + vec2(0.49,0.49)));" SHNL, dimmpixing,dimmpixing);
    }
    else if (bsu.type == 5 || bsu.type == 6)
    {
      m_offset += msprintf(&m_to[m_offset],  "ivec4 idimms4 = ivec4(floor((");
      if (bsu.type == 5)
      {
        m_offset += msprintf(&m_to[m_offset], "vec4(%F, %F, %F, %F)", bsu.ffs[0], bsu.ffs[1], bsu.ffs[2], bsu.ffs[3]);
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
                                         "ivec2 movecs_pixing%d = ivec2(datadimm_a, datadimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(1, datadimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(datadimm_a, 1);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "ivec2 movecs_pixing%d = ivec2(1, iscaling.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(iscaling.x, 1);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(datadimm_a, ibounds.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(ibounds.x, datadimm_b);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "ivec2 movecs_pixing%d = ivec2(1, ibounds.y);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(iscaling.x, datadimm_b);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(ibounds.x, 1);" SHNL,
                                         "ivec2 movecs_pixing%d = ivec2(datadimm_a, iscaling.y);" SHNL
                                      } ;
  m_offset += msprintf(&m_to[m_offset], coordination[(int)con], m_pixingsctr);
  return m_pixingsctr++;
}

int FshTraceGenerator::add_movecs_rel(COORDINATION con)
{
  if (con == CR_SAME) return -1;
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
//                                         "vec2 movecs_rel_%d = vec2(datadimm_a-1, datadimm_b-1);" SHNL,   // ntf!
                                         "vec2 movecs_rel_%d = vec2(datadimm_a, datadimm_b);" SHNL,   // ntf!
                                         "vec2 movecs_rel_%d = vec2(1.0, 1.0);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(datadimm_a, 1);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, datadimm_b);" SHNL,
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "vec2 movecs_rel_%d = vec2(ibounds);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, ibounds.y);" SHNL,
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "vec2 movecs_rel_%d = vec2(datadimm_a, ibounds.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, datadimm_b);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, 1);" SHNL,
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "vec2 movecs_rel_%d = vec2(ibounds.x, 1);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(datadimm_a, iscaling.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(1, ibounds.y);" SHNL,
                                         "vec2 movecs_rel_%d = vec2(iscaling.x, datadimm_b);" SHNL
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

void FshTraceGenerator::var_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "float %s = %F;", name, value); }

void FshTraceGenerator::var_const_fixed(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %F;", name, value); }

void FshTraceGenerator::var_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "int %s = %d;", name, value); }

void FshTraceGenerator::var_const_fixed(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;", name, value); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "vec2 %s = vec2(%F, %F);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%F, %F);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_const_fixed(const char *name, int v1, int v2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, v1, v2); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "vec3 %s = vec3(%F, %F, %F);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2, float v3){  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%F, %F, %F);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_const_fixed(const char *name, int v1, int v2, int v3){  m_offset += msprintf(&m_to[m_offset], "const ivec3 %s = ivec3(%d, %d, %d);" SHNL, name, v1, v2, v3); }

void FshTraceGenerator::var_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "vec4 %s = vec4(%F, %F, %F, %F);" SHNL, name, v1, v2, v3, v4); }

void FshTraceGenerator::var_const_fixed(const char *name, float v1, float v2, float v3, float v4){  m_offset += msprintf(&m_to[m_offset], "const vec4 %s = vec4(%F, %F, %F, %F);" SHNL, name, v1, v2, v3, v4); }

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

void FshTraceGenerator::push(const char *text)
{
//#ifdef SHNL
//  while (*text != '\0')
//    if (*text == ';')
//    {
//      m_to[m_offset++] = *text++;
//      m_to[m_offset++] = '\n';
//    }
//    else
//      m_to[m_offset++] = *text++;
//#else
  while (*text != '\0')
    m_to[m_offset++] = *text++;
//#endif
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


#define TRACE_MIX_INSVAR_WITH_RESULT "result = mix(result, _insvar, 1.0 - step(abs(_insvar[0]) - abs(result[0]), 0.0) );" SHNL

void FshTraceGenerator::trace_triangle_cc_begin(const char *) {}

void FshTraceGenerator::trace_triangle_cc_end(const char *side, int direction, float fillcoeff)
{
  char rd[32];    msprintf(rd, "(%s)", side);
  char rd_plus[32];     msprintf(rd_plus, "(0.707*%s)", side);
  char rd_minus[32];    msprintf(rd_minus, "(-0.707*%s)", side);
  
  m_offset += msprintf(&m_to[m_offset],  "_insvar[0] = float(inormed.y) + %s + 0.49 - 1.0;" SHNL
                                        "_insvar[1] = %s;" SHNL, 
                      direction == 0 || direction == 2? rd_plus :
                      direction == 1 || direction == 3? rd_minus : "",
                      direction == 0 || direction == 2? "1.0 - step(_insvar[0], 0.0)" :
                      direction == 1 || direction == 3? "1.0 - step(0.0, _insvar[0])" :
                                      "0.0"
                                      );
  
  m_offset += msprintf(&m_to[m_offset],
                          "_insvar[0] = 1.0 - (abs(_insvar[0]))/(%s*0.866*2.0);"
                          "_insvar[2] = _insvar[1]*_insvar[0]*%s - 0.49;" SHNL    /// otrezok
                          "_densvar = (1.0+density - clamp(float(abs(inormed.x) - int(_insvar[2])), 0.0, 1.0+density))/(1.0+density) * (1.0-step(_insvar[2], 0.1));" SHNL
                          "_insvar[2] = step(float(abs(inormed.x)), _insvar[2]-1.0);"
                          "_densvar = mix(_densvar, %F, _insvar[2]);" SHNL
                          "_insvar = vec3(_insvar[1]*_densvar, _insvar[0]*(1-_insvar[2]), %s*2.31);" SHNL /// 2.31 - trace coeff
                          TRACE_MIX_INSVAR_WITH_RESULT
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
//                        "_insvar = vec3(step(0.0, inormed[0])*step(inormed[0], %s)*step(0.0, inormed[1])*step(inormed[1], %s), 0.0, 1.0)*_insban;" SHNL
                        "_insvar = vec3( "
                        "step(float(abs(inormed[0])), float(abs(%s)))*(1.0-step(float(sign(inormed[0])*sign(%s)),0.0))*"
                        "step(float(abs(inormed[1])), float(abs(%s)))*(1.0-step(float(sign(inormed[1])*sign(%s)),0.0)),"
                        "0.0, 1.0)*_insban;" SHNL
                        "result = mix(result, vec3(_insvar.r * %F, _insvar.gb), _insvar[0] * (1.0-step(1.0, result[0])) );" SHNL,
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
                        "_insvar = vec3(step(float(abs(inormed[0])), abs(%s)) * step(float(abs(inormed[1])), float(abs(%s))), 0.0, 1.0)*_insban;" SHNL
                        "result = mix(result, vec3(_insvar.r * %F, _insvar.gb), _insvar[0] * (1.0 - step(1.0, result[0])) );" SHNL,
                        r11, r01, fillcoeff);
  }
}

void FshTraceGenerator::trace_square_lb_begin(const char *aside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sqdimms = vec2(%s, %s);", aside, aside);  trace_rect_xywh_begin("_sqdimms"); }

void FshTraceGenerator::trace_square_lb_end(float fillcoeff) {  trace_rect_xywh_end("_sqdimms", fillcoeff); }

void FshTraceGenerator::trace_square_cc_begin(const char *halfside) {  m_offset += msprintf(&m_to[m_offset], "vec2 _sq2dimms = vec2(%s, %s);", halfside, halfside);  trace_rect_cc_begin("_sq2dimms"); }

void FshTraceGenerator::trace_square_cc_end(float fillcoeff) {  trace_rect_cc_end("_sq2dimms", fillcoeff); }

void FshTraceGenerator::trace_circle_cc_begin(const char *radius, const char *border) { m_offset += msprintf(&m_to[m_offset], "vec4 _cd = vec4(%s-1, %s + density, inormed.x*inormed.x + inormed.y*inormed.y, %s);", radius, border, border); }

void FshTraceGenerator::trace_circle_cc_end(float fillcoeff)
{
  if ((m_maths & (1 << MM_PI)) == 0)  math_pi();
  if (fillcoeff > 0.0f)
    m_offset += msprintf(&m_to[m_offset],  "vec3  r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3  r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_fill = 1-step(r2[1], _cd[2]);" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*mixwell_fill;" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
                                          "_insvar = vec3(mix(mixwell_before + mixwell_aftere, %F, mixwell_fill*(1 - step(1.0, mixwell_before + mixwell_aftere))), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0])*_insban;" SHNL
                                          TRACE_MIX_INSVAR_WITH_RESULT
                        , fillcoeff
                        );    
  else
    m_offset += msprintf(&m_to[m_offset],  "vec3  r2 = vec3((_cd[0]-_cd[1])*(_cd[0]-_cd[1]), _cd[0]*_cd[0], (_cd[0]+_cd[1])*(_cd[0]+_cd[1]));" SHNL
                                          "vec3  r3 = vec3((_cd[0]-_cd[3])*(_cd[0]-_cd[3]), _cd[0]*_cd[0], (_cd[0]+_cd[3])*(_cd[0]+_cd[3]));" SHNL
                                          "float mixwell_onborder = (1 - step(r3[2], _cd[2])) * (step(r3[0], _cd[2])) * abs(sign(density));" SHNL
                                          "float mixwell_before = smoothstep(r2[0], r2[1], _cd[2])*(1 - step(r2[1], _cd[2]));" SHNL
                                          "float mixwell_aftere = (1 - smoothstep(r2[1], r2[2], _cd[2]))*(step(r2[1], _cd[2]));" SHNL
                                          "_insvar = vec3(clamp(mixwell_onborder + mixwell_before + mixwell_aftere, 0.0,1.0), atan(float(inormed.x), float(inormed.y))/(2*PI), 2*PI*_cd[0])*_insban;" SHNL
                                          TRACE_MIX_INSVAR_WITH_RESULT
                        );
}

#define _TRACE_MAIN_CONDITION(idx)  "_densvar = %s * (1.0+density - clamp(abs(inormed["#idx"] - floor(%s + 0.49)), 0.0, 1.0+density))/(1.0+density);"


void FshTraceGenerator::trace_2linehorz_c(const char *size, const char *gap, const char *offset, const char *crosslimit)
{
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(abs(inormed[0]), ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,                        
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_2linevert_c(const char *size, const char *gap, const char *offset, const char *crosslimit)
{
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(abs(inormed[1]), ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_linehorz_l(const char *size, const char *gap, const char *offset, const char *crosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(inormed[0], ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_linehorz_r(const char *size, const char *gap, const char *offset, const char *crosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(1) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(-inormed[0], ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_linevert_t(const char *size, const char *gap, const char *offset, const char *crosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(-inormed[1], ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_linevert_b(const char *size, const char *gap, const char *offset, const char *crosslimit)
{ 
  m_offset += msprintf(&m_to[m_offset], _TRACE_MAIN_CONDITION(0) "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(inormed[1], ivec2(%s, %s))*_insban;" TRACE_MIX_INSVAR_WITH_RESULT,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      offset == nullptr? "0.0" : offset,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_lines_x(const char *size, const char *gap, const char *crosslimit)
{
  m_offset += msprintf(&m_to[m_offset], 
                      "_insvar[0] = atan(float(abs(inormed.x)), float(abs(inormed.y)));" SHNL
                      "_insvar[1] = length(vec2(inormed))*cos(0.785398 - _insvar[0]);" SHNL
                      "_insvar[2] = _insvar[1]*0.707;" SHNL
                      "_insvar.xy = vec2(_insvar[2], _insvar[2]);" SHNL
                      "_densvar = %s * mix((1.0+density - clamp(distance(abs(inormed), _insvar.xy), 0.0, 1.0+density))/(1.0+density), 1.0, step(length(vec2(inormed)), 0.0));" SHNL
                      "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(int(_insvar[2]), ivec2(int(%s), int(%s)))*_insban;" SHNL
                      TRACE_MIX_INSVAR_WITH_RESULT
                      ,
                      crosslimit == nullptr? "1.0" : crosslimit,
                      gap == nullptr? "0" : gap,
                      size == nullptr? "5000" : size);
}

void FshTraceGenerator::trace_line_from_normed_to(const char *normedendpoint)
{ 
  m_offset += msprintf(&m_to[m_offset],
                        "_insvar.xy = %s;" SHNL
                        "_insvar.z = length(_insvar.xy);" SHNL
                        "if (step(1.6, _insvar.z) != 0){" SHNL         /// ntf finally
                        "_densvar = abs(_insvar.x*inormed[1] - _insvar.y*inormed[0])/_insvar.z;" SHNL
                        "_insvar.x = step(0, dot(inormed, _insvar.xy));" SHNL
                        "_insvar.y = sqrt(1 - _densvar*_densvar/(length(vec2(inormed))*length(vec2(inormed))))*float(length(vec2(inormed)));" SHNL /// cos(alpha), 
                        "_densvar = (1.0+density - clamp(_densvar, 0.0, 1.0+density))/(1.0+density) * _insvar.x;" SHNL
                        "_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * insider(int(_insvar.y), ivec2(0, int(_insvar.z)))*_insban;" SHNL
                        TRACE_MIX_INSVAR_WITH_RESULT
                        "}"
                      , normedendpoint
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





