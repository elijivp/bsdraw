#ifndef BSSHGENPARAMS_H
#define BSSHGENPARAMS_H

/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov

#include "../bsidrawcore.h"

static const char* glsl_types[] = { "float", "vec2", "vec3", "vec4", 
                                    "float", "vec2", "vec3", "vec4", 
                                    "int",  "ivec2", "ivec3", "ivec4", 
//                                    "vec2", "vec3", "vec4", 
                                    "sampler2D", "int", "ivec2", "ivec3", "ivec4",
                                    "sampler2D", "sampler2D",
                                    
                                    /// Special types
                                    "_HC_typecounter", "sampler2D", 
                                    
                                     };

#define SHNL "\n"

extern int msprintf(char* to, const char* format, ...);

inline unsigned int  msexpandParams(char* to, int overlay, unsigned int uniformsCount, const dmtype_t* uniforms)
{
  unsigned int offset = 0;
  /// PARAMS
  for (unsigned int i=0; i<uniformsCount; i++)
  {
    int glsl_types_idx = (int)uniforms[i].type;
    if (glsl_types_idx < (int)sizeof(glsl_types))
    {
      if (uniforms[i].type == DT_ARR || uniforms[i].type == DT_ARR2 || uniforms[i].type == DT_ARR3 || uniforms[i].type == DT_ARR4 || 
          uniforms[i].type == DT_ARRI || uniforms[i].type == DT_ARRI2 || uniforms[i].type == DT_ARRI3 || uniforms[i].type == DT_ARRI4)
      {
        const dmtype_arr_t* parr = (const dmtype_arr_t*)uniforms[i].dataptr;
        offset += msprintf(&to[offset], "uniform highp %s opm%D_%D[%d];\n", glsl_types[glsl_types_idx], overlay, i, parr->count);
      }
      else
        offset += msprintf(&to[offset], "uniform highp %s opm%D_%D;\n", glsl_types[glsl_types_idx], overlay, i);
    }
    else
      offset += msprintf(&to[offset], "ERROR on param %d;\n", overlay);
  }
  return offset;
}

#endif // BSSHGENPARAMS_H
