/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgenmain.h"

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include <QDebug>
#include "bsshgenparams.h"


/// Realisation for msprintf
/// excluded validation
/// %D === %D
/// %F === %F
int msprintf(char* to, const char* format, ...)
{
  char* t=to;
  const char* fp=format;
  const char* pps[9] = { nullptr };

  va_list vl;
  va_start( vl, format );
  while (*fp != '\0')
  {
    if (*fp == '%')
    {
      switch (*++fp)
      {
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
      {
        int idx = *fp - '1';
        if (pps[idx] == nullptr)
          pps[idx] = va_arg(vl, const char*);
        for (const char* f=pps[idx]; *f != '\0'; f++)
          *t++ = *f;
        break;
      }
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
          
          nums[0] = int(value);
          int arrsizes[] = {  1000000, 100000, 10000, 1000, 100, 10, 1, 0  };
          int arridx = *fp == 'f' ? 0 : 3;
          nums[1] = int((nums[0] < 0? -1 : 1)*(value - nums[0])*arrsizes[arridx] + 0.499);
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
  return int(t - to);
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


#define SHGP "  "
#define SHG2 "    "



FshDrawComposer::FshDrawComposer(char* deststring, SPLITPORTIONS splits, const datasubmesh_t& imp): 
  m_writebase(deststring), m_to(deststring), m_offset(0), m_split(splits), m_datasubmesh(imp),
  m_datamapped(false)
{
}

void FshDrawComposer::push(const char* text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
#ifdef AUTOSHNL
  text = SHNL;
  while (*text != '\0')
    m_to[m_offset++] = *text++;
#endif
}

void FshDrawComposer::pushin(const char* text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
}

void FshDrawComposer::ccolor(const char *name, unsigned int value)
{
  float clr[3];
  for (int i=0; i<3; i++)
    clr[i] = ((value >> 8*i) & 0xFF) / 256.0;
  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%F, %F, %F);" SHNL, name, clr[0], clr[1], clr[2]);
}


void FshDrawComposer::cfloatvar(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %f;" SHNL, name, value);  }
void FshDrawComposer::cfloatvar(const char *name, float value1, float value2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%f, %f);" SHNL, name, value1, value2);  }
void FshDrawComposer::cintvar(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;" SHNL, name, value);  }
void FshDrawComposer::cintvar(const char *name, int value1, int value2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, value1, value2);  }

void FshDrawComposer::value2D(const char* varname, const char* coordsname, const char* portionname)
{
  if (m_datasubmesh.type == datasubmesh_t::IR_OFF)
  {
    m_offset += msprintf(&m_to[m_offset], "%s = texture(datasampler, vec2(%s.x, float(%s.y + float(%s))/float(dataportions))).r;" SHNL,
                                          varname,                    coordsname, coordsname,   portionname);
  }
  
                    /// A
  
  else if (m_datasubmesh.type == datasubmesh_t::IR_A_COEFF || m_datasubmesh.type == datasubmesh_t::IR_A_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec3 loc_vv = vec3(0.0, 0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == false ? 
                           m_datasubmesh.type == datasubmesh_t::IR_A_COEFF? "ab_ibounds" : "ab_indimms" :
                           m_datasubmesh.type == datasubmesh_t::IR_A_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 submeshgrad = vec4(0, %s.x*%s.x, 1.0/%s.x, float(%s.y + float(%s))/float(dataportions));" SHNL,
                                                          coordsname,  scnosc,   scnosc,     coordsname,  portionname
                        );
    
    const char* lvclamp = "loc_vv[2]";
    if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP && m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
      lvclamp = "clamp(loc_vv[2], 0.0, 1.0)";
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP)
      lvclamp = "min(loc_vv[2], 1.0)";
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
      lvclamp = "max(loc_vv[2], 0.0)";
    
    for (int i=0; i<m_datasubmesh.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = submeshgrad[1] - %d;" SHNL, -(i - m_datasubmesh.central));
      if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.x + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(datasampler, vec2(loc_vv[1]*submeshgrad[2], submeshgrad[3])).r;" SHNL);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * %s;" SHNL,
                                                                              m_datasubmesh.coeff[i], lvclamp);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(datasampler, vec2(submeshgrad[1]*submeshgrad[2], submeshgrad[3])).r;" SHNL,
                                                                              m_datasubmesh.coeff[m_datasubmesh.central]);
    
    for (int i=m_datasubmesh.central+1; i<m_datasubmesh.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = submeshgrad[1] + %d;" SHNL, i-m_datasubmesh.central);
      if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.x, 1.0 - step(loc_vv[1], %s.x));" SHNL, scnosc, scnosc );
      
      m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(datasampler, vec2(loc_vv[1]*submeshgrad[2], submeshgrad[3])).r;" SHNL);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * %s;" SHNL,
                                                                              m_datasubmesh.coeff[i], lvclamp);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS_FIXEDCOUNT || m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS_SMART)
  {    
    const char* scnosc = m_datamapped == false? "ab_indimms.x" : "dbounds_noscaled.x";
    m_offset += msprintf(&m_to[m_offset],  "vec4 submeshgrad = vec4(0, (%s.x*(%s)), 1.0/(%s), float(%s.y + float(%s))/float(dataportions));" SHNL,
                                                                   coordsname,  scnosc,             scnosc,               coordsname,  portionname );
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(submeshgrad[1] - 1, submeshgrad[1] + 1);" SHNL );
    if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "submeshgrad[0] = texture(datasampler, vec2(%s.x, submeshgrad[3])).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(datasampler, vec2(loc_vv[0]*submeshgrad[2], submeshgrad[3])).r," SHNL
                                                           "texture(datasampler, vec2(loc_vv[1]*submeshgrad[2], submeshgrad[3])).r);" SHNL);
    
    {
      if (m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS_SMART)
        m_offset += msprintf(&m_to[m_offset], "float submeshmix = (0.5 + float(imrect[0]))/float(imrect[2]+1);" SHNL);  // +1 !@!!!
      else
      {
        double halfcent = m_datasubmesh.central/2.0;
        m_offset += msprintf(&m_to[m_offset], "float submeshmix = (0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[0])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[2]-imrect[0])/%F, 0.0, 1.0) );" SHNL,
                              halfcent, halfcent, halfcent);
      }
    }
    
    float fc = m_datasubmesh.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 1.0f) fc = 1.0f;
    
    const char* igclamp = "submeshgrad[0]";
    const char* lv0clamp = "loc_vv[0]", *lv1clamp = "loc_vv[1]";
    if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP && m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
    {
      igclamp = "clamp(submeshgrad[0], 0.0, 1.0)"; lv0clamp = "clamp(loc_vv[0], 0.0, 1.0)"; lv1clamp = "clamp(loc_vv[1], 0.0, 1.0)";
    }
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP)
    { igclamp = "min(submeshgrad[0], 1.0)"; lv0clamp = "min(loc_vv[0], 1.0)"; lv1clamp = "min(loc_vv[1], 1.0)"; }
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
    { igclamp = "max(submeshgrad[0], 0.0)"; lv0clamp = "max(loc_vv[0], 0.0)"; lv1clamp = "max(loc_vv[1], 0.0)"; }
    
    
    if (m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_A_BORDERS_FIXEDCOUNT)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(" SHNL
                           "(0.5 - submeshmix + (submeshmix-1)*submeshmix*(1-2*submeshmix)*%F)*step(submeshmix, 0.5)*(%s - %s)," SHNL
                           "(submeshmix - (submeshmix-1)*submeshmix*(1-2*submeshmix)*%F - 0.5)*step(0.5, submeshmix)*(%s - %s) " SHNL
                                            ");" SHNL, fc*2, igclamp, lv0clamp, fc*2, igclamp, lv1clamp);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset], "vec2 loc_c = vec2("
                              "%F*abs(submeshgrad[0]-loc_vv[0])/max(submeshgrad[0],loc_vv[0]),"
                              "%F*abs(submeshgrad[0]-loc_vv[1])/max(submeshgrad[0],loc_vv[1])"
                              ");" SHNL, 8*fc, 8*fc
                           );
      
      m_offset += msprintf(&m_to[m_offset], "loc_c = vec2("
                              "(0.5 - submeshmix + (submeshmix-1)*submeshmix*loc_c[0]),"
                              "(submeshmix + (submeshmix-1)*submeshmix*loc_c[1] - 0.5)"
                              ");" SHNL
                           );
      
      m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(" SHNL
                           "clamp(loc_c[0], 0.0, 1.0)*step(submeshmix, 0.5)*(%s - %s)," SHNL
                           "clamp(loc_c[1], 0.0, 1.0)*step(0.5, submeshmix)*(%s - %s) " SHNL
                                            ");" SHNL, igclamp, lv0clamp, igclamp, lv1clamp);
    }
    
    m_offset += msprintf(&m_to[m_offset], "%s = submeshgrad[0] - step(%F, float(imrect[2]+1))*(loc_vv.x + loc_vv.y);" SHNL, 
                                                                  varname, double(m_datasubmesh.count));

  }
  
                          /// B
  
  else if (m_datasubmesh.type == datasubmesh_t::IR_B_COEFF || m_datasubmesh.type == datasubmesh_t::IR_B_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec3 loc_vv = vec3(0.0, 0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == false ? 
                           m_datasubmesh.type == datasubmesh_t::IR_B_COEFF? "ab_ibounds" : "ab_indimms" :
                           m_datasubmesh.type == datasubmesh_t::IR_B_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 submeshgrad = vec4(%s.x, %s.y*%s.y, 1.0/%s.y, %s);" SHNL, // float(%s.y + float(%s))/float(dataportions)
                                                          coordsname,  coordsname,  scnosc,   scnosc,     portionname
                        );
    
    const char* lvclamp = "loc_vv[2]";
    if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP && m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
      lvclamp = "clamp(loc_vv[2], 0.0, 1.0)";
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP)
      lvclamp = "min(loc_vv[2], 1.0)";
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
      lvclamp = "max(loc_vv[2], 0.0)";
    
    for (int i=0; i<m_datasubmesh.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = submeshgrad[1] - %d;" SHNL, -(i - m_datasubmesh.central));
      if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.y + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(datasampler, vec2(submeshgrad[0], (loc_vv[1]*submeshgrad[2] + submeshgrad[3])/float(dataportions))).r;" SHNL);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * %s;" SHNL, m_datasubmesh.coeff[i], lvclamp);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(datasampler, vec2(submeshgrad[0], (submeshgrad[1]*submeshgrad[2] + submeshgrad[3])/float(dataportions))).r;" SHNL,
                                                                              m_datasubmesh.coeff[m_datasubmesh.central]);
    
    for (int i=m_datasubmesh.central+1; i<m_datasubmesh.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = submeshgrad[1] + %d;" SHNL, i-m_datasubmesh.central);
      if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.y, 1.0 - step(loc_vv[1], %s.y));" SHNL, scnosc, scnosc );
      
      m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(datasampler, vec2(submeshgrad[0], (loc_vv[1]*submeshgrad[2] + submeshgrad[3])/float(dataportions))).r;" SHNL);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * %s;" SHNL, m_datasubmesh.coeff[i], lvclamp);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS_FIXEDCOUNT || m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS_SMART)
  {
    
    const char* scnosc = m_datamapped == false? "ab_indimms.y" : "dbounds_noscaled.y";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 submeshgrad = vec4(%s.x, (%s.y*(%s)), 1.0/(%s), float(%s));" SHNL,
                                                           coordsname,  coordsname,  scnosc,  scnosc,  portionname );
    
    
    m_offset += msprintf(&m_to[m_offset],  "vec3 loc_vv = vec3(submeshgrad[1] - 1, submeshgrad[1] + 1, 0.0);" SHNL );
    if (m_datasubmesh.flags & datasubmesh_t::F_CYCLED)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(datasampler, vec2(submeshgrad[0], (%s.y + submeshgrad[3])/float(dataportions))).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(datasampler, vec2(submeshgrad[0], (loc_vv[0]*submeshgrad[2] + submeshgrad[3])/float(dataportions))).r," SHNL
                                                           "texture(datasampler, vec2(submeshgrad[0], (loc_vv[1]*submeshgrad[2] + submeshgrad[3])/float(dataportions))).r);" SHNL);

    {
      double halfcent = m_datasubmesh.central/2.0;
      if (m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS_SMART)
        m_offset += msprintf(&m_to[m_offset], "float submeshmix = (0.5 + float(imrect[1]))/float(imrect[3]+1);" SHNL);  // +1 !@!!!
      else
      {
        m_offset += msprintf(&m_to[m_offset], "float submeshmix = (0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[1])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[3]-imrect[1])/%F, 0.0, 1.0) );" SHNL,
                              halfcent, halfcent, halfcent
                           );
      }
    }
    
    float fc = m_datasubmesh.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 1.0f) fc = 1.0f;
    
    const char* igclamp = "loc_vv[2]";
    const char* lv0clamp = "loc_vv[0]", *lv1clamp = "loc_vv[1]";
    if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP && m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
    {
      igclamp = "clamp(loc_vv[2], 0.0, 1.0)"; lv0clamp = "clamp(loc_vv[0], 0.0, 1.0)"; lv1clamp = "clamp(loc_vv[1], 0.0, 1.0)";
    }
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPTOP)
    { igclamp = "min(loc_vv[2], 1.0)"; lv0clamp = "min(loc_vv[0], 1.0)"; lv1clamp = "min(loc_vv[1], 1.0)"; }
    else if (m_datasubmesh.flags & datasubmesh_t::F_CLAMPBOT)
    { igclamp = "max(loc_vv[2], 0.0)"; lv0clamp = "max(loc_vv[0], 0.0)"; lv1clamp = "max(loc_vv[1], 0.0)"; }
    
//    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(" SHNL
//                                            "-submeshmix*step(submeshmix, 0.0)*(%s - %s)/(1.0 - %f*(1.0+submeshmix))," SHNL
//                                            " submeshmix*step(0.0, submeshmix)*(%s - %s)/(1.0 - %f*(1.0-submeshmix)) " SHNL
//                                          ");" SHNL, igclamp, lv0clamp, fc, igclamp, lv1clamp, fc);
    
    if (m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS || m_datasubmesh.type == datasubmesh_t::IR_B_BORDERS_FIXEDCOUNT)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(" SHNL
                           "(0.5 - submeshmix + (submeshmix-1)*submeshmix*(1-2*submeshmix)*%F)*step(submeshmix, 0.5)*(%s - %s)," SHNL
                           "(submeshmix - (submeshmix-1)*submeshmix*(1-2*submeshmix)*%F - 0.5)*step(0.5, submeshmix)*(%s - %s) " SHNL
                                            ");" SHNL, fc*2, igclamp, lv0clamp, fc*2, igclamp, lv1clamp);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset], "vec2 loc_c = vec2("
                              "%F*abs(submeshgrad[0]-loc_vv[0])/max(submeshgrad[0],loc_vv[0]),"
                              "%F*abs(submeshgrad[0]-loc_vv[1])/max(submeshgrad[0],loc_vv[1])"
                              ");" SHNL, 8*fc, 8*fc
                           );
      
      m_offset += msprintf(&m_to[m_offset], "loc_c = vec2("
                              "(0.5 - submeshmix + (submeshmix-1)*submeshmix*loc_c[0]),"
                              "(submeshmix + (submeshmix-1)*submeshmix*loc_c[1] - 0.5)"
                              ");" SHNL
                           );
      
      m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(" SHNL
                           "clamp(loc_c[0], 0.0, 1.0)*step(submeshmix, 0.5)*(%s - %s)," SHNL
                           "clamp(loc_c[1], 0.0, 1.0)*step(0.5, submeshmix)*(%s - %s) " SHNL
                                            ");" SHNL, igclamp, lv0clamp, igclamp, lv1clamp);
    }
    
    m_offset += msprintf(&m_to[m_offset], "%s = loc_vv[2] - step(%F, float(imrect[3]))*(loc_vv.x + loc_vv.y);", varname, m_datasubmesh.count);
  }
}


/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

unsigned int FshDrawMain::basePendingSize(const datasubmesh_t& imp, unsigned int ovlscount)
{
  unsigned int base = 2300 + ovlscount*1000;
  switch (imp.type)
  {
  case datasubmesh_t::IR_A_COEFF: case datasubmesh_t::IR_A_COEFF_NOSCALED: case datasubmesh_t::IR_B_COEFF: case datasubmesh_t::IR_B_COEFF_NOSCALED:
    base += 500 + imp.count*220;
    break;
  case datasubmesh_t::IR_A_BORDERS: case datasubmesh_t::IR_A_BORDERS_FIXEDCOUNT: case datasubmesh_t::IR_A_BORDERS_SMART: 
  case datasubmesh_t::IR_B_BORDERS: case datasubmesh_t::IR_B_BORDERS_FIXEDCOUNT: case datasubmesh_t::IR_B_BORDERS_SMART: 
    base += 1500;
    break;
  default: break;
  }
  return base;
}

FshDrawMain::FshDrawMain(char* deststring, SPLITPORTIONS splits, const datasubmesh_t& imp): FshDrawComposer(deststring, splits, imp)  
{
}

void FshDrawMain::generic_decls_begin()
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],   "#version %d" SHNL, BSGLSLVER);
#endif
  static const char fsh_base[] =          "uniform highp sampler2D  datasampler;" SHNL
                                          "uniform highp int        datadimm_a;" SHNL
                                          "uniform highp int        datadimm_b;" SHNL
                                          "uniform highp int        dataportions;" SHNL
                                          "uniform highp int        dataportionsize;" SHNL
                                          "uniform highp int        scaler_a;" SHNL
                                          "uniform highp int        scaler_b;" SHNL
                                          "uniform highp sampler2D  paletsampler;" SHNL
                                          "in highp vec2            coords;" SHNL
#if defined BSGLSLVER && BSGLSLVER >= 420
                                          "out vec4                 outcolor;" SHNL
#endif
                                          "float getValue1D(in int portion, in float x){  return texture(datasampler, vec2(x, float(portion)/(float(dataportions)-1.0))).r; }" SHNL
                                          "float getValue2D(in int portion, in vec2  x){  return texture(datasampler, vec2(x.x, float(x.y + float(portion))/float(dataportions))).r; }" SHNL
                                          "vec3  insider(int i, ivec2 ifromvec) { float scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));\n\treturn vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }" SHNL SHNL;
  memcpy(&m_to[m_offset], fsh_base, sizeof(fsh_base) - 1);  m_offset += sizeof(fsh_base) - 1;
}

void FshDrawMain::generic_decls_add(DTYPE type, const char* name)
{
  Q_ASSERT(type >= 0 && type <= sizeof(glsl_types)/sizeof(glsl_types[0]));
  m_offset += msprintf(&m_to[m_offset], "uniform highp %s \t%s;" SHNL, glsl_types[type], name);
//  strcpy(m_c_locbacks[m_c_locbackscount].varname, name);
//  m_c_locbacks[m_c_locbackscount].istexture = type >= _DT_TEXTURES_BEGIN;
//  m_c_locbackscount += 1;
  
//  m_c_locbackscount = 0;
//  for (unsigned int i=0; i<globscount; i++)
//  {
//    Q_ASSERT(globsinfo[i].type >= 0 && globsinfo[i].type <= sizeof(glsl_types)/sizeof(glsl_types[0]));
//    m_offset += msprintf(&m_to[m_offset], "uniform highp %s    %s;" SHNL, glsl_types[globsinfo[i].type], globsinfo[i].name);
//    strcpy(m_c_locbacks[m_c_locbackscount].varname, globsinfo[i].name);
//    m_c_locbacks[m_c_locbackscount].istexture = globsinfo[i].type >= _DT_TEXTURES_BEGIN;
//    m_c_locbackscount += 1;
  //  }
}

void FshDrawMain::generic_decls_add(DTYPE type, const char* name, int count)
{
  Q_ASSERT(type >= 0 && type <= sizeof(glsl_types)/sizeof(glsl_types[0]));
  m_offset += msprintf(&m_to[m_offset], "uniform highp %s \t%s[%d];" SHNL, glsl_types[type], name, count);
}

void FshDrawMain::generic_decls_add_tft_area(int texid, char* result)
{
  msprintf(result, "tftholding_%d", texid);
#ifndef BSGLSLOLD
  m_offset += msprintf(&m_to[m_offset], "uniform highp sampler2DArray    %s;" SHNL, result);
#else
  m_offset += msprintf(&m_to[m_offset], "uniform highp sampler2D         %s;" SHNL, result);
#endif
}

void FshDrawMain::generic_decls_add_tft_dslots(int texid, int count, char* result_i, char* result_c)
{
  msprintf(result_i, "tft_i_%d", texid);
  generic_decls_add(DT_1I, result_i, count);
  msprintf(result_c, "tft_c_%d", texid);
  generic_decls_add(DT_4F, result_c, count);
}

void FshDrawMain::generic_decls_add_ovl_input(int ovlid, char* result)
{
  msprintf(result, "ovlprm%d_in", ovlid+1);
  m_offset += msprintf(&m_to[m_offset], SHNL
                                        "uniform highp vec4 \t%s;" SHNL
                                        "vec4 overlayOVCoords%d(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, "
                                                               "in vec2 coords, in float dvalue, in float thick, "
                                                               "in vec2 root_offset_px, in vec3 post_in, out vec2 shapeself);" SHNL
                                        "vec3 overlayColor%d(in vec3 color_drawed, in vec3 color_ovled, in vec4 in_variant);" SHNL,
                       result, ovlid+1, ovlid+1);
}

void FshDrawMain::generic_decls_add_ovl_nameonly(int ovlid, int ovlparamid, DTYPE type, char* result)
{
  msprintf(result, "ovlprm%d_%d", ovlid+1, ovlparamid);
//  generic_decls_add(type, result);
}

void FshDrawMain::generic_main_begin(int allocatedPortions, ORIENTATION orient, unsigned int emptycolor, const overpattern_t& fsp)
{
  static const char fsh_main[] =  SHNL SHNL 
                                  "void main()" SHNL
                                  "{" SHNL
                                    SHGP "float mixwell = 1.0;" SHNL
                                    SHGP "float dvalue = 0.0;" SHNL
                                    
                                    SHGP "ivec2 ab_indimms = ivec2(datadimm_a, datadimm_b);" SHNL
                                    SHGP "ivec2 ab_iscaler = ivec2(scaler_a, scaler_b);" SHNL
                                    SHGP "ivec2 ab_ibounds = ab_indimms*ab_iscaler;" SHNL;
  memcpy(&m_to[m_offset], fsh_main, sizeof(fsh_main) - 1);  m_offset += sizeof(fsh_main) - 1;
  
  {
    const char* rotaters[] = {  "ab_ibounds.xy",  "ab_ibounds.xy",  "ab_ibounds.xy",  "ab_ibounds.xy",
                                "ab_ibounds.yx",  "ab_ibounds.yx",  "ab_ibounds.yx",  "ab_ibounds.yx" 
                             };
    
    m_offset += msprintf(&m_to[m_offset], SHGP "ivec2 xy_ibounds = %s;" SHNL, rotaters[orient]);
  }
  
  static const char fsh_xy[] = SHGP "vec2  xy_coords = vec2(coords.x*0.5 + 0.5, coords.y*0.5 + 0.5);" SHNL; ///   non-oriented non-cell coords
  memcpy(&m_to[m_offset], fsh_xy, sizeof(fsh_xy) - 1);  m_offset += sizeof(fsh_xy) - 1;
  
  {
    const char* rotaters[] = {  "xy_coords.xy",
                                "vec2(1.0 - xy_coords.x, xy_coords.y)",
                                "vec2(xy_coords.x, 1.0 - xy_coords.y)",
                                "vec2(1.0 - xy_coords.x, 1.0 - xy_coords.y)",
                                "vec2(1.0 - xy_coords.y, xy_coords.x)",
                                "xy_coords.yx",
                                "vec2(1.0 - xy_coords.y, 1.0 - xy_coords.x)",
                                "vec2(xy_coords.y, 1.0 - xy_coords.x)"
                             };
    
    m_offset += msprintf(&m_to[m_offset], 
                                    SHGP "vec2  ab_coords = %s;" SHNL                ///   oriented non-cell coords
                                    SHGP "vec2  abc_coords = ab_coords;" SHNL        ///   oriented cell coords
                                , rotaters[orient]);
    
//    const char* rotaters[] = {              "",
//                                            "ab_coords.x = 1.0-ab_coords.x;",
//                                            "ab_coords.y = 1.0-ab_coords.y;",
//                                            "ab_coords.xy = vec2(1.0,1.0)-ab_coords.xy;",
//                                            "ab_coords.xy = vec2(1.0-ab_coords.y, ab_coords.x);",
//                                            "ab_coords.xy = ab_coords.yx;",
//                                            "ab_coords.xy = vec2(1.0,1.0)-ab_coords.yx;",
//                                            "ab_coords.xy = vec2(ab_coords.y, 1.0-ab_coords.x);"
//                                  };
    
//    m_offset += msprintf(&m_to[m_offset], 
                                    
//                                    SHGP "vec2  ab_coords = xy_coords;         %s    " SHNL                ///   oriented non-cell coords
//                                    SHGP "vec2  abc_coords = ab_coords;" SHNL                              ///   oriented cell coords
//                                , rotaters[orient]);
  }
  if (m_split == 0)
  {
    m_offset += msprintf(&m_to[m_offset],   SHGP "ivec2 ispcell = ivec2(0,0);" SHNL
                                            SHGP "ivec2 icells = ivec2(1,1);" SHNL );
  }
  else
  {
    int spRotator =   (m_split >> 16)&0xFF;   // 0 - BT/LR, 1 - TB/RL
    int spDirection = (m_split >> 8)&0xFF;    // 0 - Columns, 1 - Rows
    int spDivider =    m_split & 0xFF;
    
    if (spDivider == 1)
    {
      m_offset += msprintf(&m_to[m_offset],  SHGP "int explicitPortion = int(abc_coords.%1*%d);" SHNL
                                             SHGP "abc_coords.%1 = abc_coords.%1 * %d - explicitPortion;" SHNL,
                                              spDirection == 0? "y" : "x",
                                              allocatedPortions, allocatedPortions
                          );
      if (spRotator)
        m_offset += msprintf(&m_to[m_offset], SHGP "explicitPortion = %d - 1 - explicitPortion;" SHNL, allocatedPortions);
      
      if (spDirection == 0)
        m_offset += msprintf(&m_to[m_offset], 
                                    SHGP "ivec2 ispcell = ivec2(explicitPortion, 0);" SHNL
                                    SHGP "ivec2 icells = ivec2(%d, 1);" SHNL
                             , allocatedPortions
                             );
      else
        m_offset += msprintf(&m_to[m_offset], 
                                    SHGP "ivec2 ispcell = ivec2(0, explicitPortion);" SHNL
                                    SHGP "ivec2 icells = ivec2(1, %d);" SHNL
                             , allocatedPortions
                             );
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],   SHGP "int    spDivider = %d;" SHNL, spDivider );        
      m_offset += msprintf(&m_to[m_offset],   SHGP "ivec2  icells = ivec2(spDivider, %d/float(spDivider) + 0.99);" SHNL
                                              SHGP "ivec2  ispcell = ivec2(abc_coords.x * icells[%1], abc_coords.y * icells[%2]);" SHNL
                                              SHGP "int explicitPortion = ispcell[%1] + ispcell[%2]*spDivider;" SHNL
                                              SHGP "abc_coords.x = abc_coords.x * icells[%1] - ispcell.x;" SHNL
                                              SHGP "abc_coords.y = abc_coords.y * icells[%2] - ispcell.y;" SHNL,
                                              allocatedPortions,
                                              spDirection == 0? "0" : "1", 
                                              spDirection == 0? "1" : "0"
                          );
    }
    m_offset += msprintf(&m_to[m_offset],     SHGP "abc_coords = mix(abc_coords, vec2(-1.0), step(dataportions, float(explicitPortion)));" SHNL);
  }
  
  m_offset += msprintf(&m_to[m_offset], SHGP "int allocatedPortions = %d;" SHNL, (int)allocatedPortions);
  
  
  if (emptycolor != 0xFFFFFFFF)
    this->ccolor("backcolor", emptycolor);
  else
    m_offset += msprintf(&m_to[m_offset],   SHGP "vec3  backcolor = texture(paletsampler, vec2(0.0, 0.0)).rgb;" SHNL);
  
  
  
  m_offset += msprintf(&m_to[m_offset],     SHGP "vec3  result = backcolor;" SHNL);
  
  
  m_offset += msprintf(&m_to[m_offset],     SHGP "vec4  post_mask = vec4(0.0, %F, %F, %F);" SHNL, fsp.threshold, fsp.weight, 1.0f - fsp.smooth );   
                                                              /// ppban, threshold, weight, smooth 

  m_offset += msprintf(&m_to[m_offset],     SHGP "ivec2  immod = ivec2( int(mod(abc_coords.x*ab_ibounds.x, float(ab_iscaler.x))), int(mod(abc_coords.y*ab_ibounds.y, float(ab_iscaler.y))));" SHNL
                                            SHGP "ivec4  imrect  = ivec4(immod.x, immod.y, ab_iscaler.x-1, ab_iscaler.y-1);" SHNL
  );
}

void FshDrawMain::generic_main_prepare_tft()
{
}

const char* cr_to_px_str(COORDINATION con)
{
//        ocall_ibounds = transposed ? "ab_indimms.yx*ab_iscaler.yx*icells.xy" : "ab_indimms.xy*ab_iscaler.xy*icells.yx";
  Q_ASSERT(con != CR_SAME);
  static const char* coordination[] = {  /// CR_ABSOLUTE, CR_RELATIVE, CR_XABS_YREL, CR_XREL_YABS,
                                         "ab_iscaler",
                                         "(ab_ibounds - ivec2(1))",
                                         "ivec2(ab_iscaler.x, ab_ibounds.y-1)",
                                         "ivec2(ab_ibounds.x-1, ab_iscaler.y)",
                                         
                                         ///  CR_ABSOLUTE_NOSCALED, CR_RELATIVE_NOSCALED, CR_XABS_YREL_NOSCALED, CR_XREL_YABS_NOSCALED
                                         "ivec2(1, 1)",
                                         "ivec2(ab_indimms.x-1, ab_indimms.y-1)",
                                         "ivec2(1, ab_indimms.y-1)",
                                         "ivec2(ab_indimms.x-1, 1)",
                                         
                                         ///  CR_XABS_YABS_NOSCALED_SCALED, CR_XABS_YABS_SCALED_NOSCALED, CR_XREL_YREL_NOSCALED_SCALED, CR_XREL_YREL_SCALED_NOSCALED, 
                                         "ivec2(1, ab_iscaler.y)",
                                         "ivec2(ab_iscaler.x, 1)",
                                         "ivec2(ab_indimms.x-1, ab_ibounds.y-1)",
                                         "ivec2(ab_ibounds.x-1, ab_indimms.y-1)",
                                         
                                         /// CR_XABS_YREL_NOSCALED_SCALED, CR_XABS_YREL_SCALED_NOSCALED, CR_XREL_YABS_NOSCALED_SCALED, CR_XREL_YABS_SCALED_NOSCALED,
                                         "ivec2(1, ab_ibounds.y-1)",
                                         "ivec2(ab_iscaler.x, ab_indimms.y-1)",
                                         "ivec2(ab_ibounds.x-1, 1)",
                                         "ivec2(ab_indimms.x-1, ab_iscaler.y)"
                                      } ;
  return coordination[con];
}

#define TFT_OPTIMISE
#define TFT_OPTIMISE2

#if 1
void FshDrawMain::generic_main_process_tft(const tftfraginfo_t& tft)
{
  m_offset += msprintf(&m_to[m_offset],     "{" SHNL);
  {
    int limit = tft.limitrows * tft.limitcols;
    bool gorotate = tft.isunrotateable == false;
    if (tft.isstatic)
    {
      m_offset += msprintf(&m_to[m_offset],     
                                                SHGP "int  tft_rec = %d;" SHNL
                                                SHGP "vec4 tft_slot = vec4(%F, %F, %F, %F);" SHNL, 
                                                      tft.recordid/* / tft.limitrows*/,
                                                      tft.slotdata.fx, tft.slotdata.fy, tft.slotdata.scale, tft.slotdata.rotate);
      if (tft.slotdata.rotate > -0.0001f && tft.slotdata.rotate < 0.0001f)
        gorotate = false;
    }
    else
      m_offset += msprintf(&m_to[m_offset],     SHGP "int  tft_rec = tft_i_%d[%d];" SHNL
                                                SHGP "vec4 tft_slot = tft_c_%d[%d];" SHNL, tft.texid, tft.varid, tft.texid, tft.varid);
    
    if (tft.driven_id == -1)
      m_offset += msprintf(&m_to[m_offset],       SHGP "tft_slot.xy = tft_slot.xy*%s;" SHNL, cr_to_px_str(tft.slotdata.cr));
    else
      m_offset += msprintf(&m_to[m_offset],       SHGP "tft_slot.xy = ovl_offset_px_%d.xy + tft_slot.xy*%s;" SHNL, tft.driven_id, cr_to_px_str(tft.slotdata.cr));
    
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec2  rc = xy_coords*xy_ibounds - tft_slot.xy + vec2(0.499);" SHNL);
    
#ifdef TFT_OPTIMISE
    const float lenmax = tft.isstatic ? sqrt(tft.textwidth*tft.textwidth/4.0f + tft.recordheight*tft.recordheight/4.0f) :
                                        sqrt(tft.recordwidth*tft.recordwidth/4.0f + tft.recordheight*tft.recordheight/4.0f);
//    m_offset += msprintf(&m_to[m_offset],       SHGP "if (step(distance(rc, vec2(%f, %f)), 0.0) == 1.0)" SHNL
    m_offset += msprintf(&m_to[m_offset],       SHGP "if (length(rc) < %f)" SHNL
                                                SHGP "{" SHNL, lenmax);
#endif
    
    if (gorotate)
    {
      m_offset += msprintf(&m_to[m_offset],     SHGP "vec2  aa = vec2(cos(tft_slot[3]), sin(tft_slot[3]));" SHNL);
      m_offset += msprintf(&m_to[m_offset],     SHGP "rc = rc*mat2(aa.x, -aa.y, aa.y, aa.x);" SHNL);
    }
    
    float oneroww = 1.0f / tft.limitcols;
    float onerowh = 1.0f / tft.limitrows;
    m_offset += msprintf(&m_to[m_offset],       SHGP "float pc = rc.y/%f + 0.5;" SHNL,  float(tft.recordheight));
#ifndef BSGLSLOLD
    if (tft.limitcols == 1)
    {
      m_offset += msprintf(&m_to[m_offset],       SHGP "vec3  tcoords = vec3( rc.x/%f + 0.5, "
                                                                             "1.0 - (mod(mod(tft_rec,%d),%d) + pc)*%f, "
                                                                             "float(tft_rec/%d)/%f);" SHNL,
                                                    float(tft.recordwidth),
                                                    limit, tft.limitrows, onerowh, 
                                                    limit, float(tft.texcount == 1? 1 : tft.texcount-1));
    }
    else
    {
//      m_offset += msprintf(&m_to[m_offset],       SHGP "vec3  tcoords = vec3((int(mod(tft_rec,%d))/%d + (rc.x/%f + 0.5))*%f, "
//                                                                             "1.0 - (mod(mod(tft_rec,%d),%d) + pc)*%f, "
//                                                                             "float(tft_rec/%d)/%f);" SHNL,
//                                                    limit, tft.limitrows, float(tft.recordwidth), oneroww,
//                                                    limit, tft.limitrows, onerowh, 
//                                                    limit, float(tft.texcount == 1? 1 : tft.texcount-1));
      m_offset += msprintf(&m_to[m_offset],       SHGP "vec3  tcoords = vec3((int(mod(tft_rec,%d))/%d + (rc.x/%f + 0.5))*%f, "
                                                                             "1.0 - (mod(mod(tft_rec,%d),%d) + pc)*%f, "
                                                                             "float(tft_rec/%d));" SHNL, // MEOW!!
                                                    limit, tft.limitrows, float(tft.recordwidth), oneroww,
                                                    limit, tft.limitrows, onerowh, 
                                                    limit);
    }
#else
    if (tft.limitcols == 1)
    {
      m_offset += msprintf(&m_to[m_offset],       SHGP "vec2  tcoords = vec2( rc.x/%f + 0.5, "
                                                                             "1.0 - (tft_rec + pc)*%f);" SHNL, onerowh);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],       SHGP "vec2  tcoords = vec2((int(mod(tft_rec,%d))/%d + (rc.x/%f + 0.5))*%f, "
                                                                              "1.0 - (tft_rec + pc)*%f);" SHNL,
                                                    limit, tft.limitrows, float(tft.recordwidth), oneroww, onerowh);
    }
#endif
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec4  ttc = texture(tftholding_%d, tcoords).rgba;" SHNL
                                                SHGP "result = mix(result, ttc.rgb, ttc.a*step(0.0, pc)*step(pc, 1.0));" SHNL,
                                                  tft.texid);
#ifdef TFT_OPTIMISE
    m_offset += msprintf(&m_to[m_offset],       SHGP "}" SHNL);
#endif
  }
  m_offset += msprintf(&m_to[m_offset],     "}" SHNL);
}
#else
void FshDrawMain::generic_main_process_tft(const tftfraginfo_t& tft)
{
  m_offset += msprintf(&m_to[m_offset],     "{" SHNL);
  {
    int limit = tft.limitrows * tft.limitcols;
    bool gorotate = true;
    if (tft.isstatic)
    {
      m_offset += msprintf(&m_to[m_offset],     
                                                SHGP "int  tft_rec = %d;" SHNL
                                                SHGP "vec4 tft_slot = vec4(%F, %F, %F, %F);" SHNL, 
                                                      tft.recordid/* / tft.limitrows*/,
                                                      tft.slotdata.fx, tft.slotdata.fy, tft.slotdata.scale, tft.slotdata.rotate);
      if (tft.slotdata.rotate > -0.0001f && tft.slotdata.rotate < 0.0001f)
        gorotate = false;
    }
    else
      m_offset += msprintf(&m_to[m_offset],     SHGP "int  tft_rec = tft_i_%d[%d];" SHNL
                                                SHGP "vec4 tft_slot = tft_c_%d[%d];" SHNL, tft.texid, tft.varid, tft.texid, tft.varid);
    m_offset += msprintf(&m_to[m_offset],       SHGP "tft_slot.xy = tft_slot.xy*%s;" SHNL, cr_to_px_str(tft.slotdata.cr));
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec2  rc = ab_coords*ab_ibounds - tft_slot.xy + vec2(0.499);" SHNL);
    
#ifdef TFT_OPTIMISE
//    m_offset += msprintf(&m_to[m_offset],       SHGP "if (step(distance(rc, vec2(%f, %f)), 0.0) == 1.0)" SHNL
    m_offset += msprintf(&m_to[m_offset],       SHGP "if (length(rc) < length(vec2(%f, %f)))" SHNL
                                                SHGP "{" SHNL, float(tft.textwidth)/2, float(tft.recordheight)/2);
#endif
    
    if (gorotate)
    {
      m_offset += msprintf(&m_to[m_offset],     SHGP "vec2  aa = vec2(cos(tft_slot[3]), sin(tft_slot[3]));" SHNL);
      m_offset += msprintf(&m_to[m_offset],     SHGP "rc = rc*mat2(aa.x, -aa.y, aa.y, aa.x);" SHNL);
    }
    
    float oneroww = 1.0f / tft.limitcols;
    float onerowh = 1.0f / tft.limitrows;
    m_offset += msprintf(&m_to[m_offset],       SHGP "float pc = rc.y/%f + 0.5;" SHNL,  float(tft.recordheight));
#ifndef BSGLSLOLD
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec3  tcoords = vec3((int(mod(tft_rec,%d))/%d + (rc.x + 0.5*%f)/%f)*%f, "
                                                                           "1.0 - (mod(mod(tft_rec,%d),%d) + pc)*%f, "
                                                                           "float(tft_rec/%d)/%f);" SHNL,
                                                  limit, tft.limitrows, float(tft.textwidth), float(tft.recordwidth), oneroww,
                                                  limit, tft.limitrows, onerowh, 
                                                  limit, float(tft.texcount == 1? 1 : tft.texcount-1));
#else
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec2  tcoords = vec2((int(mod(tft_rec,%d))/%d + (rc.x + 0.5*%f)/%f)*%f, "
                                                                            "1.0 - (tft_rec + pc)*%f);" SHNL,
                                                  limit, tft.limitrows, float(tft.textwidth), float(tft.recordwidth), onerowh);
#endif
    m_offset += msprintf(&m_to[m_offset],       SHGP "vec4  ttc = texture(tftholding_%d, tcoords).rgba;" SHNL
                                                SHGP "result = mix(result, ttc.rgb, ttc.a*step(0.0, pc)*step(pc, 1.0));" SHNL,
                                                  tft.texid);
#ifdef TFT_OPTIMISE
    m_offset += msprintf(&m_to[m_offset],       SHGP "}" SHNL);
#endif
  }
  m_offset += msprintf(&m_to[m_offset],     "}" SHNL);
}
#endif

void FshDrawMain::generic_main_process_fsp(const overpattern_t &fsp, float fspopacity)
{
  m_offset += msprintf(&m_to[m_offset], "" SHNL);
  if (fsp.algo == overpattern_t::OALG_OFF || fsp.mask >= _OP_TOTAL)
    m_offset += msprintf(&m_to[m_offset], "float ppb_in = 0.0;" SHNL);
  else
  {
    if (fsp.masktype == overpattern_t::OMASK_INT)
    {
      const char* dmasks[] = {  
        // OP_CONTOUR
            "float ppb_in = sign(step(imrect.x, post_mask[2]) + step(imrect.y, post_mask[2])"
                    " + step(imrect[2] - imrect.x, post_mask[2]) + step(imrect[3] - imrect.y, post_mask[2]));",
        // OP_LINELEFT
            "float ppb_in = step(imrect.x, post_mask[2]);",
        // OP_LINERIGHT
            "float ppb_in = step(imrect[2] - imrect.x, post_mask[2]);",
        // OP_LINEBOTTOM
            "float ppb_in = step(imrect.y, post_mask[2]);",
        // OP_LINETOP
            "float ppb_in = step(imrect[3] - imrect.y, post_mask[2]);",
        // OP_LINELEFTRIGHT
            "float ppb_in = sign(step(imrect.x, post_mask[2]) + step(imrect[2] - imrect.x, post_mask[2]));",
        // OP_LINEBOTTOMTOP
            "float ppb_in = sign(step(imrect.y, post_mask[2]) + step(imrect[3] - imrect.y, post_mask[2]));",
        //  OP_LINELEFTBOTTOM
            "float ppb_in = sign(step(imrect.x, post_mask[2])+step(imrect.y, post_mask[2]));",
        //  OP_LINERIGHTBOTTOM
            "float ppb_in = sign(step(imrect[2] - imrect.x, post_mask[2])+step(imrect.y, post_mask[2]));",
        //  OP_LINELEFTTOP
            "float ppb_in = sign(step(imrect.x, post_mask[2])+step(imrect[3] - imrect.y, post_mask[2]));",
        // OP_LINERIGHTTOP 
            "float ppb_in = sign(step(imrect[2] - imrect.x, post_mask[2])+step(imrect[3] - imrect.y, post_mask[2]));",
        
        // OP_GRID
            "float ppb_in = sign(step(mod(1+imrect.x, 2 + post_mask[2]), 0.0) + step(mod(1+imrect.y, 2 + post_mask[2]), 0.0));", 
        // OP_DOT
            "float ppb_in = step(abs(imrect.x-imrect[2]/2), post_mask[2])*step(abs(imrect.y-imrect[3]/2), post_mask[2]);",
        // OP_DOTLEFTBOTTOM
            "float ppb_in = step(imrect.x, post_mask[2])*step(imrect.y, post_mask[2]);",
        // OP_DOTCONTOUR
            "float ppb_in = sign( "
                                  "step(imrect.x, post_mask[2])*step(imrect.y, post_mask[2]) + "
                                  "step(imrect[2]-imrect.x, post_mask[2])*step(imrect.y, post_mask[2]) + "
                                  "step(imrect.x, post_mask[2])*step(imrect[3]-imrect.y, post_mask[2]) + "
                                  "step(imrect[2]-imrect.x, post_mask[2])*step(imrect[3]-imrect.y, post_mask[2]));",
        // OP_SHTRICHL
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), 3.0 + post_mask[2]), 0.0);", 
        // OP_SHTRICHR
            "float ppb_in = step(mod(abs(imrect.x - imrect[3] + imrect.y), 3.0 + post_mask[2]), 0.0);",
        // OP_CROSS
//            "float ppb_in = step(mod(float(imrect.x), 4.0),0.0)*step(mod(float(imrect.y), 4.0), 0.0) + mod(float(imrect.x),2.0)*mod(float(imrect.y),2.0);", 
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), 3.0 + post_mask[2]), 0.0) + step(mod(abs(imrect.x - imrect[3] + imrect.y), 3.0 + post_mask[2]), 0.0);", 
        // OP_FILL
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), 2.0 + post_mask[2]) + mod(abs(imrect.x - imrect[3] - 1 + imrect.y), 2.0 + post_mask[2]), 0.0);", 
        // OP_SQUARES
            "float ppb_in = step(mod(abs(imrect.x - imrect.y) + abs(imrect.x - imrect[3] + 1 + imrect.y), 3.0 + 2.0*post_mask[2]), 0.0);"
      };
      m_offset += msprintf(&m_to[m_offset], SHGP "%s" SHNL, dmasks[fsp.mask >= _OP_TOTAL ? OP_CROSS : fsp.mask]);
    }
    else if (fsp.masktype == overpattern_t::OMASK_FLOAT)    // FLOATS
    {
      m_offset += msprintf(&m_to[m_offset], "vec2 ppb_dc = vec2(1.0 - 2.0*float(imrect.x)/imrect[2], 1.0 - 2.0*float(imrect.y)/imrect[3]);" SHNL);
      
      const char* dmasks_sigm[] = {  
        // OPF_CIRCLE
            "float ppb_in = dot(ppb_dc, ppb_dc) - post_mask[2];",
        // OPF_CIRCLE_REV
            "float ppb_in = (1.0 - post_mask[2])*(1.0 - post_mask[2]) - dot(ppb_dc, ppb_dc);",
        // OPF_CROSSPUFF
            "float ppb_in = dot(ppb_dc, ppb_dc)/2.0 + abs(abs(ppb_dc.y) - abs(ppb_dc.x))/(1.0 + dot(ppb_dc, ppb_dc)) - post_mask[2];",
        // OPF_RHOMB
            "float ppb_in = (abs(ppb_dc.x) + abs(ppb_dc.y) - post_mask[2]*2.0)/2.0;", //step(0.25 + post_mask[2]*0.05, ppb_dc.x + ppb_dc.y);", 
        // OPF_SURIKEN
            "float ppb_in = (abs(ppb_dc.x) + abs(ppb_dc.y) - post_mask[2]*2.0 + 2*abs(ppb_dc.x*ppb_dc.y))/2.0;",
        // OPF_SURIKEN_REV
            "float ppb_in = distance(vec2(0.0,0.0), vec2(1.0, 1.0) - abs(ppb_dc))/1.0 - 0.5 - post_mask[2];",
        // OPF_DONUT 
            "float ppb_in = abs( (1.0 - post_mask[2]/2.0)*(1.0 - post_mask[2]/2.0) - dot(ppb_dc, ppb_dc)) - 0.25;",
        // OPF_CROSS
            "float ppb_in = abs(abs(ppb_dc.x)-abs(ppb_dc.y)) - post_mask[2];",
        // OPF_UMBRELLA
            "float ppb_in = dot(ppb_dc, ppb_dc)/2.0 + abs((abs(ppb_dc.y) - abs(ppb_dc.x))*ppb_dc.x*ppb_dc.y) - post_mask[2];",
        // OPF_HOURGLASS
            "float ppb_in = (abs(ppb_dc.x)-abs(ppb_dc.y))/(1.0 + dot(ppb_dc, ppb_dc)) + (0.5-post_mask[2]);",
        // OPF_STAR
            "float ppb_in = abs((abs(ppb_dc.y) - abs(ppb_dc.x))*ppb_dc.x*ppb_dc.y)*8.0 - post_mask[2];",
        // OPF_BULL
            "float ppb_in = dot(ppb_dc, ppb_dc)/2.0 + abs(ppb_dc.y - ppb_dc.x)/(1.0 + dot(ppb_dc, ppb_dc)) - post_mask[2];",
        // OPF_BULR
            "float ppb_in = dot(ppb_dc, ppb_dc)/2.0 + abs(-ppb_dc.y - ppb_dc.x)/(1.0 + dot(ppb_dc, ppb_dc)) - post_mask[2];",
        // OPF_CROSSHAIR
            "float ppb_in = 10*abs(ppb_dc.x*ppb_dc.y) - abs(ppb_dc.x) - abs(ppb_dc.y) + 0.5 - post_mask[2];",
        };
      m_offset += msprintf(&m_to[m_offset], SHGP "%s" SHNL, dmasks_sigm[fsp.mask >= _OPF_TOTAL ? OPF_CROSS : fsp.mask]);
      m_offset += msprintf(&m_to[m_offset], SHGP "ppb_in = clamp(ppb_in*(1.0+12.0*post_mask[3])/(1.0 + abs(ppb_in)*(1.0+12.0*post_mask[3]))*1.5 + 0.5, 0.0, 1.0);" SHNL);
    }
    else // ANGLEFIGURES
    {
//      m_offset += msprintf(&m_to[m_offset], "vec4 ppb_a4 = vec4("
//                                            "distance(vec2(imrect.x, imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect[2] - 1 - imrect.x, imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect.x, imrect[3] - 1 - imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect[2] - 1 - imrect.x, imrect[3] - 1 - imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0))"
//                                            ");" SHNL);
      
      m_offset += msprintf(&m_to[m_offset], SHGP "vec2 ppb_a2 = vec2("
                                              SHGP "mix(float(imrect.x), float(imrect[2] - imrect.x), step(imrect[2]/2.0, float(imrect.x))),"
                                              SHGP "mix(float(imrect.y), float(imrect[3] - imrect.y), step(imrect[3]/2.0, float(imrect.y)))"
                                            SHGP ");"  SHNL
                                            SHGP "ppb_a2 = ppb_a2 / vec2(imrect[2]/2.0, imrect[3]/2.0);" SHNL
                           );
      
      const char* dmasks_anglefigures[] = {  
        // OPA_PUFFHYPERB
//            "vec2 ppb_cc = vec2(post_mask[2]*1.0/ppb_a2.x, post_mask[2]*1.0/ppb_a2.y);" //    ppb_cc = vec2(max(ppb_cc.x, ppb_a2.x), max(ppb_cc.y, ppb_a2.y));
//            "float ppb_in = length(vec2(1.0) - ppb_cc)/length(vec2(1.0) - ppb_a2);",
            "float ppb_in = 1.0 - (ppb_a2.x*ppb_a2.y - 1.0*post_mask[2])/length(vec2(1.0) - ppb_a2);",
        // OPA_PUFFCIRCLE
            "vec2 ppb_cc = vec2(max(post_mask[2], ppb_a2.x), max(post_mask[2], ppb_a2.y));"
            "float ppb_in = (length(ppb_a2 - ppb_cc) - post_mask[2]) / post_mask[2];",
        // OPA_PUFFCIRCLESPACED125
            "vec2 ppb_cc = vec2(max(post_mask[2], ppb_a2.x), max(post_mask[2], ppb_a2.y));"
            "float ppb_in = (length(ppb_a2 - ppb_cc)*1.25 - post_mask[2]) / post_mask[2];",
        // OPA_PUFFCIRCLESPACED15
            "vec2 ppb_cc = vec2(max(post_mask[2], ppb_a2.x), max(post_mask[2], ppb_a2.y));"
            "float ppb_in = (length(ppb_a2 - ppb_cc)*1.5 - post_mask[2]) / post_mask[2];",
        // OPA_PUFFCIRCLESPACED2
            "vec2 ppb_cc = vec2(max(post_mask[2], ppb_a2.x), max(post_mask[2], ppb_a2.y));"
            "float ppb_in = (length(ppb_a2 - ppb_cc)*2.0 - post_mask[2]) / post_mask[2];",
        // OPA_PUFFCIRCLESPACED25
            "vec2 ppb_cc = vec2(max(post_mask[2], ppb_a2.x), max(post_mask[2], ppb_a2.y));"
            "float ppb_in = (length(ppb_a2 - ppb_cc)*2.5 - post_mask[2]) / post_mask[2];"
        };
      m_offset += msprintf(&m_to[m_offset], SHGP "%s" SHNL, dmasks_anglefigures[fsp.mask >= _OPA_TOTAL ? OPA_PUFFCIRCLE : fsp.mask]);
      
      //return 0.5f + 0.5f*(x-xpos)*speed/(1.0f + fabs(x-xpos)*speed);
      m_offset += msprintf(&m_to[m_offset], SHGP "ppb_in = 0.5 + 0.5*(ppb_in*50*post_mask[3])/(1.0 + abs(ppb_in)*50*post_mask[3]);" SHNL);
//      m_offset += msprintf(&m_to[m_offset], "ppb_in = clamp(ppb_in*(1.0+12.0*post_mask[3])/(1.0 + abs(ppb_in)*(1.0+12.0*post_mask[3]))*1.5 + 0.5, 0.0, 1.0);" SHNL);
    }
      
    if (fsp.colorByPalette)
    {
      if (fsp.color.r == 0.0f)
        m_offset += msprintf(&m_to[m_offset], SHGP "vec3   ppb_color = backcolor;" SHNL);
      else
        m_offset += msprintf(&m_to[m_offset], SHGP "vec3   ppb_color = texture(paletsampler, vec2(%F, 0.0)).rgb;" SHNL, fsp.color.r);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],   SHGP "vec3   ppb_color = vec3(%F,%F,%F);" SHNL, fsp.color.r, fsp.color.g, fsp.color.b);
    }
    m_offset += msprintf(&m_to[m_offset],     SHGP "result = mix(result, ppb_color, ppb_in * %s * %F );" SHNL,
                             fsp.algo == overpattern_t::OALG_THRS_PLUS?   "post_mask[0]" : 
                             fsp.algo == overpattern_t::OALG_THRS_MINUS?  "(1.0 - post_mask[0])" : 
                             fsp.algo == overpattern_t::OALG_ANY?         "1.0" : 
                                                                          "0.0",
                             1.0f - fspopacity
                         );
  } // if mask
  m_offset += msprintf(&m_to[m_offset],     SHGP "vec3 result_drawed = result;" SHNL);
}

void FshDrawMain::generic_main_prepare_ovl()
{
  static const char fsh_decltrace[] =     SHNL
                                          SHGP "vec4   ovTrace;" SHNL
                                          SHGP "vec4   ovlprm_in;" SHNL   /// visibility, opacity, slice_ll, slice_hl
                                          SHGP "vec2   ovl_offset_px_transfer;" SHNL
                                          SHNL;
  memcpy(&m_to[m_offset], fsh_decltrace, sizeof(fsh_decltrace) - 1);  m_offset += sizeof(fsh_decltrace) - 1;
}

void FshDrawMain::generic_main_process_ovl(ORIENTATION orient, int i, int link, OVL_ORIENTATION ovlorient)
{
  bool transposed = orientationTransposed(orient);
  
  m_offset += msprintf(&m_to[m_offset],   SHGP "ovlprm_in = ovlprm%d_in;" SHNL    // opacity, thickness, slice
                                          SHGP "ovl_offset_px_transfer = vec2(0,0);" SHNL, i+1);
  
  if (link >= 0)
    m_offset += msprintf(&m_to[m_offset], SHGP "bool ovl_visible_%d = ovl_visible_%d && step(1.0, ovlprm_in[0]) != 1;" SHNL, i+1, link + 1 );
  else
    m_offset += msprintf(&m_to[m_offset], SHGP "bool ovl_visible_%d = step(1.0, ovlprm_in[0]) != 1;" SHNL, i+1 );
  
  const char* ovl_coords_oriented[] =
  {
    //  OO_INHERITED=0, OO_INHERITED_MIRROR_HORZ, OO_INHERITED_MIRROR_VERT, OO_INHERITED_MIRROR_BOTH,
    "abc_coords", "vec2(1.0 - abc_coords.x, abc_coords.y)", "vec2(abc_coords.x, 1.0 - abc_coords.y)", "vec2(1.0 - abc_coords.x, 1.0 - abc_coords.y)", 
    //  OO_AREA_LRBT, OO_AREA_RLBT, OO_AREA_LRTB, OO_AREA_RLTB, OO_AREA_TBLR, OO_AREA_BTLR, OO_AREA_TBRL, OO_AREA_BTRL,
    "xy_coords", "vec2(1.0 - xy_coords.x, xy_coords.y)", "vec2(xy_coords.x, 1.0 - xy_coords.y)", "vec2(1.0 - xy_coords.x, 1.0 - xy_coords.y)", 
    "vec2(1.0 - xy_coords.y, xy_coords.x)", "xy_coords.yx", "vec2(1.0 - xy_coords.y, 1.0 - xy_coords.x)", "vec2(xy_coords.y, 1.0 - xy_coords.x)",
    //  OO_AREAOR, OO_AREAOR_MIRROR_HORZ, OO_AREAOR_MIRROR_VERT, OO_AREAOR_MIRROR_BOTH,
    "ab_coords", "vec2(1.0 - ab_coords.x, ab_coords.y)", "vec2(ab_coords.x, 1.0 - ab_coords.y)", "vec2(1.0 - ab_coords.x, 1.0 - ab_coords.y)", 
    // OO_SAME
    ""
  };
  const char* ocall_indimms = nullptr, *ocall_iscaler = nullptr, *ocall_ibounds = nullptr;
  const char* ocall_ovcoords = ovl_coords_oriented[ovlorient];
  if (ovlorient < OO_AREA_LRBT)
  {
    ocall_indimms = "ab_indimms.xy";
    ocall_iscaler = "ab_iscaler.xy";
    ocall_ibounds = "ab_ibounds.xy"; 
  }
  else
  {
    if (ovlorient == OO_AREA_LRBT || ovlorient == OO_AREA_RLBT || ovlorient == OO_AREA_LRTB || ovlorient == OO_AREA_RLTB)
    {
      ocall_indimms = transposed ? "ab_indimms.yx" : "ab_indimms.xy";
      ocall_iscaler = transposed ? "ab_iscaler.yx" : "ab_iscaler.xy";
      ocall_ibounds = transposed ? "ab_indimms.yx*ab_iscaler.yx*icells.xy" : "ab_indimms.xy*ab_iscaler.xy*icells.yx";
    }
    else if (ovlorient == OO_AREA_TBLR || ovlorient == OO_AREA_BTLR || ovlorient == OO_AREA_TBRL || ovlorient == OO_AREA_BTRL)
    {
      ocall_indimms = transposed ? "ab_indimms.xy" : "ab_indimms.yx";
      ocall_iscaler = transposed ? "ab_iscaler.xy" : "ab_iscaler.yx";
      ocall_ibounds = transposed ? "ab_indimms.xy*ab_iscaler.xy*icells.yx" : "ab_indimms.yx*ab_iscaler.yx*icells.xy";
    }
    else if (ovlorient == OO_AREAOR || ovlorient == OO_AREAOR_MIRROR_HORZ || ovlorient == OO_AREAOR_MIRROR_VERT || ovlorient == OO_AREAOR_MIRROR_BOTH)
    {
      ocall_indimms = "ab_indimms.xy";
      ocall_iscaler = "ab_iscaler.xy";
      ocall_ibounds = transposed ? "ab_indimms.yx*ab_iscaler.yx*icells.xy" : "ab_indimms.xy*ab_iscaler.xy*icells.yx";
    }
    else
      ;
  }
  
  
  m_offset += msprintf(&m_to[m_offset],   SHGP "if  (ovl_visible_%d)" SHNL
                                          SHGP "{" SHNL
                                          SHG2    "ovTrace = overlayOVCoords%d(ispcell, %s, %s, %s, %s, dvalue, ovlprm_in[1], ",
                                          i+1, i+1, ocall_indimms, ocall_iscaler, ocall_ibounds, ocall_ovcoords);
  
  if (link >= 0)
    m_offset += msprintf(&m_to[m_offset],       SHG2 "ovl_offset_px_%d, vec3(post_mask[0], post_mask[3], ppb_in), ovl_offset_px_transfer);" SHNL
                                          , link + 1);
  else
    m_offset += msprintf(&m_to[m_offset],       SHG2 "ivec2(0,0), vec3(post_mask[0], post_mask[3], ppb_in), ovl_offset_px_transfer);" SHNL );
  
  
//  m_offset += msprintf(&m_to[m_offset],     SHG2    "if (sign(ovTrace[3]) != 0.0 && (step(mixwell, 0.0) == 1 || (step(dvalue, ovlprm_in[2]) == 0 && step(ovlprm_in[3], dvalue) == 0)) )" SHNL
//                                              SHG2 SHGP "result = mix(result, asd overlayColor%d(ovTrace, result), 1.0 - ovlprm_in[0]);" SHNL
//                                          SHGP "}" SHNL
//                                          SHGP "vec2 ovl_offset_px_%d = ovl_offset_px_transfer;" SHNL
//                                          , 
//                                          i+1, i+1);
  
//  m_offset += msprintf(&m_to[m_offset],     SHG2    "result = mix(result, asd overlayColor%d(ovTrace, result), (1.0 - ovlprm_in[0])*step(ovlprm_in[2], dvalue)*step(dvalue, ovlprm_in[3]));" SHNL 
  m_offset += msprintf(&m_to[m_offset],     SHG2    "result = mix(result, overlayColor%d(result_drawed, result, ovTrace), (1.0 - ovlprm_in[0])*(1.0 - step(dvalue, ovlprm_in[2])*step(ovlprm_in[3],dvalue)));" SHNL 
                                          SHGP "}" SHNL
                                          SHGP "vec2 ovl_offset_px_%d = ovl_offset_px_transfer;" SHNL
                                          , 
                                          i+1, i+1);
}

void FshDrawMain::generic_main_end()
{
#if !defined BSGLSLVER || BSGLSLVER < 420
  static const char fsh_end[] =   SHGP "gl_FragColor = vec4(result, 0.0);" SHNL "}" SHNL;
#else
  static const char fsh_end[] =   SHGP "outcolor = vec4(result, 0.0);" SHNL "}" SHNL;
#endif
  memcpy(&m_to[m_offset], fsh_end, sizeof(fsh_end) - 1); m_offset += sizeof(fsh_end) - 1;
  m_to[m_offset++] = '\0';
}
