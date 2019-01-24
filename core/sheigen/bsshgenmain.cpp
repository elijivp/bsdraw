#include "bsshgenmain.h"

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>


#ifndef SHNL
#define SHNL "\n"
#endif

/// Realisation for msprintf
/// excluded validation
/// %D === %D
/// %F === %F
int msprintf(char* to, const char* format, ...)
{
  char* t=to;
  const char* fp=format;

  va_list vl;
  va_start( vl, format );
  while (*fp != '\0')
  {
    if (*fp == '%')
    {
      switch (*++fp)
      {
      case 's':
      {
        for (const char* f=va_arg(vl, const char*); *f != '\0'; f++)
          *t++ = *f;
        break;
      }
      case 'c':
      {
        *t++ = (char)va_arg(vl, int);
        break;
      }
      case 'd': case 'f': case 'D': case 'F':
      {
        int numscount=0;
        int nums[3];
                
        if (*fp == 'd' || *fp == 'D')
        {
           nums[0] = va_arg(vl, int);
           if (nums[0] < 0)
           {
             *t++ = '-';
             nums[0] = -nums[0];
           }
           
           if (*fp == 'D')
           {
             int arrsizes[] = {  100, 10, 0 };
             for (nums[2]=0; nums[2]<int(sizeof(arrsizes)/sizeof(int)); nums[2]++)
               if (arrsizes[nums[2]] <= nums[0])
                 break;
           }
           numscount = 1;
        }
        else if (*fp == 'f' || *fp == 'F')
        {
          double value = va_arg(vl, double);
          if (value < 0)
          {
            *t++ = '-';
            value = -value;
          }
          
          nums[0] = (int)value;
          int arrsizes[] = {  100000, 10000, 1000, 100, 10, 1, 0  };
          int arridx = *fp == 'f' ? 0 : 2;
          nums[1] = (nums[0] < 0? -1 : 1)*(value - nums[0])*arrsizes[arridx] + 0.499;
          if (nums[1] == 0)
            nums[2] = 0;
          else
            for (nums[2]=0; nums[2]<int(sizeof(arrsizes)/sizeof(int))-arridx; nums[2]++)
              if (arrsizes[arridx+nums[2]+1] <= nums[1])
                break;
          numscount = 2;
        }
        
        if (*fp == 'D')
          for (int z=0; z<nums[2]; z++)
            *t++ = '0';
        
        for (int n=0; n<numscount; n++)
        {
          if (n == 1)
          {
            *t++ = '.';
            for (int z=0; z<nums[2]; z++)
              *t++ = '0';
          }
          int tmpcount = 0;
          int tmpnums[16];
          while (true)
          {
            tmpnums[tmpcount++] = nums[n] % 10;
            nums[n] /= 10;
            if (nums[n] == 0)
              break;
          }
          
          static char chset[] = {'0','1','2','3','4','5','6','7','8','9'};
          for (int i=tmpcount-1; i>=0; i--)
            *t++ = chset[tmpnums[i]];
        }
        break;
      }
      }
      fp++;
    }
    else
      *t++ = *fp++;
  }
  va_end(vl);
  *t = '\0';
  return t - to;
}


unsigned int VshMainGenerator1D::pendingSize(){  return 130; }
unsigned int VshMainGenerator1D::operator()(char *to)
{
  const char vertex_body[] =        "attribute highp vec4 vertices;"
                                    "varying highp vec2 coords;"
                                    "void main() {"
                                    "    gl_Position = vertices;"
                                    "    coords = vertices.xy;"
                                    "}";
  memcpy(to, vertex_body, sizeof(vertex_body));
  return sizeof(vertex_body);
}


unsigned int VshMainGenerator2D::pendingSize(){  return 130; }
unsigned int VshMainGenerator2D::operator()(char *to)
{
  const char vertex_body[] =        "attribute highp vec4 vertices;"
                                    "varying highp vec2 coords;"
                                    "void main() {"
                                    "    gl_Position = vertices;"
                                    "    coords = vertices.xy;"
                                    "}";
  memcpy(to, vertex_body, sizeof(vertex_body));
  return sizeof(vertex_body);
}


/***********************************************************/

unsigned int FshMainGenerator::basePendingSize(unsigned int ovlscount)
{
  return 1750 + ovlscount*540;
}

FshMainGenerator::FshMainGenerator(char *deststring, bool rotated, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]): 
  m_writebase(deststring), m_to(deststring), m_offset(0), m_rotated(rotated), m_ovlscount(ovlscount), m_ovls(ovlsinfo)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],  "#version %d" SHNL, BSGLSLVER);
#endif
  static const char fsh_base[] =  "uniform highp sampler2D  texData;" SHNL
                                  "uniform highp int        datadimm_a;" SHNL
                                  "uniform highp int        datadimm_b;" SHNL
                                  "uniform highp int        scaling_a;" SHNL
                                  "uniform highp int        scaling_b;" SHNL
                                  "uniform highp int        countPortions;" SHNL
                                  "uniform highp sampler2D  texPalette;" SHNL
                                  "in highp vec2            coords;" SHNL
//                                  "varying  vec4            fragColor;" SHNL
                                  "vec2 rotate(vec2 coords);" SHNL
                                  "float getValue1D(in int portion, in float x){  return texture(texData, vec2(x, float(portion)/(float(countPortions)-1.0))).r; }" SHNL
                                  "float getValue2D(in int portion, in vec2  x){  return texture(texData, vec2(x.x, float(x.y + float(portion))/float(countPortions))).r; }" SHNL
                                  "vec3 insider(int i, ivec2 ifromvec) {\n\tfloat scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));\n\treturn vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }" SHNL;

  memcpy(&m_to[m_offset], fsh_base, sizeof(fsh_base) - 1);  m_offset += sizeof(fsh_base) - 1;

  for (unsigned int i=0; i<m_ovlscount; i++)
  {
    m_offset += msprintf(&m_to[m_offset],  "uniform highp vec4 ovl_exsettings%D;" SHNL
                                          "vec4 overlayTrace%d(in vec2 coords, in float density, in ivec2 mastercoords, out ivec2 shapeself);" SHNL
                                          "vec3 overlayColor%d(in vec4 trace, in vec3 color);" SHNL,
                        i+1, i+1, i+1);
  }
  
  
}

void FshMainGenerator::goto_func_begin(const DPostmask& fsp)
{
  static const char fsh_main[] =  "void main()" SHNL
                                  "{" SHNL
                                    "vec4  loc_f4_sets;" SHNL
                                    "ivec2 loc_i2_pos;" SHNL
                                    "float ovMix = 0.0;" SHNL
                                    "ivec2 iscaling = ivec2(scaling_a, scaling_b);" SHNL
                                    "ivec2 ibounds_noscaled = ivec2(datadimm_a, datadimm_b);" SHNL
                                    "ivec2 ibounds = ibounds_noscaled * iscaling;" SHNL
                                    "vec2  normCoord = coords.xy*0.5 + 0.5;" SHNL
                                    "vec2  fcoords = rotate(normCoord.xy);" SHNL
                                    "ivec2 icoords = ivec2(fcoords * vec2(ibounds_noscaled));" SHNL
                                    "vec3  result = vec3(0.0,0.0,0.0);" SHNL;
  memcpy(&m_to[m_offset], fsh_main, sizeof(fsh_main) - 1);  m_offset += sizeof(fsh_main) - 1;
  
  m_offset += msprintf(&m_to[m_offset], "vec3   ppb_color = vec3(%F,%F,%F);" SHNL
                                        "vec4   ppb_sfp   = vec4(0.0, %F, %F, %F);" SHNL    /// ppban, ppoutsignal, 
                                        "ivec4  ppb_rect  = ivec4(int(mod(floor(fcoords.x*float(ibounds.x) + 0.49), float(iscaling.x))),\n\tint(mod(floor(fcoords.y*float(ibounds.y) + 0.49), float(iscaling.y))),iscaling.x-1, iscaling.y-1);" SHNL, 
                                        fsp.r, fsp.g, fsp.b,
                                        fsp.over & 1? 1.0f : 0.0f, fsp.over & 2? 1.0f : 0.0f, (float)fsp.weight );

  
}

void FshMainGenerator::goto_func_end(const DPostmask &fsp)
{
  static const char fsh_decltrace[] = "vec4 ovTrace;" SHNL;
  memcpy(&m_to[m_offset], fsh_decltrace, sizeof(fsh_decltrace) - 1);  m_offset += sizeof(fsh_decltrace) - 1;
  
  if (fsp.postmask != DPostmask::PM_NONE)
  {
    m_offset += msprintf(&m_to[m_offset],  "%s" SHNL
                                          "result = mix(result, ppb_color, ppb_sfp[1] * ppb_sfp[0] * ppb_in + ppb_sfp[2] * (1.0 - ppb_sfp[0])*ppb_in );" SHNL,
                      fsp.postmask == DPostmask::PM_LINELEFT?         "float ppb_in = step(ppb_rect.x, ppb_sfp[3]);" :
                      fsp.postmask == DPostmask::PM_LINERIGHT?        "float ppb_in = step(ppb_rect[2] - ppb_rect.x, ppb_sfp[3]);" :
                      fsp.postmask == DPostmask::PM_LINEBOTTOM?       "float ppb_in = step(ppb_rect.y, ppb_sfp[3]);" :
                      fsp.postmask == DPostmask::PM_LINETOP?          "float ppb_in = step(ppb_rect[3] - ppb_rect.y, ppb_sfp[3]);" :
                      fsp.postmask == DPostmask::PM_LINELEFTBOTTOM?   "float ppb_in = sign(step(ppb_rect.x, ppb_sfp[3])+step(ppb_rect.y, ppb_sfp[3]));" : 
                      fsp.postmask == DPostmask::PM_LINERIGHTBOTTOM?  "float ppb_in = sign(step(ppb_rect[2] - ppb_rect.x, ppb_sfp[3])+step(ppb_rect.y, ppb_sfp[3]));" : 
                      fsp.postmask == DPostmask::PM_CONTOUR?          "float ppb_in = sign(step(ppb_rect.x, ppb_sfp[3]) + step(ppb_rect.y, ppb_sfp[3])"
                                                                      " + step(ppb_rect[2] - ppb_rect.x, ppb_sfp[3]) + step(ppb_rect[3] - ppb_rect.y, ppb_sfp[3]));" :
                      fsp.postmask == DPostmask::PM_PSEUDOCIRCLE?     "vec2 _ppb_pos = vec2(abs(0.5 - float(ppb_rect.x)/ppb_rect[2]), abs(0.5 - float(ppb_rect.y)/ppb_rect[3]));"
                                                                        "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
//                                                                        "float ppb_in = smoothstep((ppb_sfp[3]*0.1 + 0.2)*(ppb_sfp[3]*0.1 + 0.2), 0.7*0.7, _ppb_d2);"
                                                                        "float ppb_in = smoothstep(0.25*0.25, (0.66 - ppb_sfp[3]*0.05)*(0.66 - ppb_sfp[3]*0.05), _ppb_d2);"
                                                                      : "float ppb_in = 0.0;"
                                                                            );
  }
  
  for (unsigned int i=0; i<m_ovlscount; i++)
    if (m_ovls[i].link >= 0)
    {
      m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = ovl_exsettings%D;" SHNL
                                            "loc_i2_pos = ivec2(0,0);" SHNL
                                            "if (step(1.0, loc_f4_sets[0]) != 1){" SHNL
                                              "ovTrace = overlayTrace%d(normCoord, loc_f4_sets[1], ovl_position%d, loc_i2_pos);" SHNL
                                              "if (sign(ovTrace[3]) != 0.0 && step(ovMix, loc_f4_sets[2]) == 1.0) result = mix(result, overlayColor%d(ovTrace, result), 1.0 - loc_f4_sets[0]);" SHNL 
                                            "}" SHNL
                                            "ivec2 ovl_position%d = loc_i2_pos;" SHNL,
                                        i+1, i+1, m_ovls[i].link + 1, i+1, i+1);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = ovl_exsettings%D;" SHNL
                                            "loc_i2_pos = ivec2(0,0);" SHNL
                                            "if (step(1.0, loc_f4_sets[0]) != 1){" SHNL
                                              "ovTrace = overlayTrace%d(normCoord, loc_f4_sets[1], ivec2(0,0), loc_i2_pos);" SHNL
                                              "if (sign(ovTrace[3]) != 0.0 && step(ovMix, loc_f4_sets[2]) == 1.0) result = mix(result, overlayColor%d(ovTrace, result), 1.0 - loc_f4_sets[0]);" SHNL
                                            "}" SHNL
                                            "ivec2 ovl_position%d = loc_i2_pos;" SHNL,
                                        i+1, i+1, i+1, i+1);
    }
  
  
  static const char fsh_end[] =   "gl_FragColor = vec4(result, 0.0);" SHNL "}" SHNL;
//  static const char fsh_end[] =   "fragColor = vec4(result, 0.0);" SHNL "}" SHNL;
  
  memcpy(&m_to[m_offset], fsh_end, sizeof(fsh_end) - 1); m_offset += sizeof(fsh_end) - 1;
  m_to[m_offset++] = '\0';
}

void FshMainGenerator::push(const char *text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
}

void FshMainGenerator::cfloatvar(const char *name, float value)
{
  m_offset += msprintf(&m_to[m_offset], "const float %s = %F;" SHNL, name, value);
}

void FshMainGenerator::ccolor(const char *name, unsigned int value)
{
  float clr[3];
  for (int i=0; i<3; i++)
    clr[i] = ((value >> 8*i) & 0xFF) / 256.0;
  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%F, %F, %F);" SHNL, name, clr[0], clr[1], clr[2]);
}

void FshMainGenerator::cintvar(const char *name, int value)
{
  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;" SHNL, name, value);
}


