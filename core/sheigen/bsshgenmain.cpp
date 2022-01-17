/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
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
          
          nums[0] = (int)value;
          int arrsizes[] = {  1000000, 100000, 10000, 1000, 100, 10, 1, 0  };
          int arridx = *fp == 'f' ? 0 : 3;
          nums[1] = (nums[0] < 0? -1 : 1)*(value - nums[0])*arrsizes[arridx] + 0.49;
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

unsigned int FshMainGenerator::basePendingSize(const impulsedata_t& imp, unsigned int ovlscount)
{
  unsigned int base = 2200 + ovlscount*700;
  if (imp.type == impulsedata_t::IR_OFF)
    ;
  else if (imp.type == impulsedata_t::IR_A_COEFF || imp.type == impulsedata_t::IR_A_COEFF_NOSCALED)
    base += 200 + imp.count*210;
  else if (imp.type == impulsedata_t::IR_A_BORDERS || imp.type == impulsedata_t::IR_A_BORDERS_FIXEDCOUNT)
    base += 700;
  else if (imp.type == impulsedata_t::IR_B_COEFF || imp.type == impulsedata_t::IR_B_COEFF_NOSCALED)
    base += 210 + imp.count*260;
  else if (imp.type == impulsedata_t::IR_B_BORDERS || imp.type == impulsedata_t::IR_B_BORDERS_FIXEDCOUNT)
    base += 850;
  else
    ;
  return base;
}

FshMainGenerator::FshMainGenerator(char *deststring, unsigned int allocatedPortions, SPLITPORTIONS splitPortions, const impulsedata_t& imp, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]): 
  m_writebase(deststring), m_to(deststring), m_allocatedPortions(allocatedPortions), m_offset(0), m_splitPortions(splitPortions), m_impulsegen(imp), m_ovlscount(ovlscount), m_ovls(ovlsinfo)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],   "#version %d" SHNL, BSGLSLVER);
#endif
  static const char fsh_base[] =          "uniform highp sampler2D  texData;" SHNL
                                          "uniform highp int        viewdimm_a;" SHNL
                                          "uniform highp int        viewdimm_b;" SHNL
                                          "uniform highp int        scaling_a;" SHNL
                                          "uniform highp int        scaling_b;" SHNL
                                          "uniform highp int        countPortions;" SHNL
                                          "uniform highp sampler2D  texPalette;" SHNL
                                          "uniform highp vec2       palrange;" SHNL
                                          "in highp vec2            coords;" SHNL
                                          "float getValue1D(in int portion, in float x){  return texture(texData, vec2(x, float(portion)/(float(countPortions)-1.0))).r; }" SHNL
                                          "float getValue2D(in int portion, in vec2  x){  return texture(texData, vec2(x.x, float(x.y + float(portion))/float(countPortions))).r; }" SHNL
                                          "vec3  insider(int i, ivec2 ifromvec) { float scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));\n\treturn vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }" SHNL;

  memcpy(&m_to[m_offset], fsh_base, sizeof(fsh_base) - 1);  m_offset += sizeof(fsh_base) - 1;
  for (unsigned int i=0; i<m_ovlscount; i++)
  {
    m_offset += msprintf(&m_to[m_offset], "uniform highp vec4 ovl_exsettings%D;" SHNL
                                          "vec4 overlayTrace%d(in ivec2 icell, in vec4 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 shapeself);" SHNL
                                          "vec3 overlayColor%d(in vec4 trace_on_pix, in vec3 color);" SHNL,
                        i+1, i+1, i+1);
  }
}



void FshMainGenerator::_main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask& fsp)
{
  static const char fsh_main[] =  "void main()" SHNL
                                  "{" SHNL
                                    "vec4  loc_f4_sets;" SHNL
                                    "ivec2 loc_i2_pos;" SHNL
                                    "float ovMix = 0.0;" SHNL
                                  
                                    "ivec2 ibounds_noscaled = ivec2(viewdimm_a, viewdimm_b);" SHNL
                                    "ivec2 iscaling = ivec2(scaling_a, scaling_b);" SHNL
                                    "ivec2 ibounds = ibounds_noscaled*iscaling;" SHNL;
  memcpy(&m_to[m_offset], fsh_main, sizeof(fsh_main) - 1);  m_offset += sizeof(fsh_main) - 1;
  
  {
    const char* rotaters[] = {      "",
                                    "relcoords.x = 1.0-relcoords.x;",
                                    "relcoords.y = 1.0-relcoords.y;",
                                    "relcoords.xy = vec2(1.0,1.0)-relcoords.xy;",
                                    "relcoords.xy = vec2(1.0-relcoords.y, relcoords.x);",
                                    "relcoords.xy = relcoords.yx;",
                                    "relcoords.xy = vec2(1.0,1.0)-relcoords.yx;",
                                    "relcoords.xy = vec2(relcoords.y, 1.0-relcoords.x);"
                                  };
    
    m_offset += msprintf(&m_to[m_offset], "vec2  relcoords = vec2(coords.xy*0.5 + vec2(0.5,0.5));" SHNL
                                          "%s"
                         , rotaters[orient]);
  }
  
  {
    int spDirection = (m_splitPortions >> 8)&0xFF;
    int spDivider = m_splitPortions&0xFF;
    if (spDivider == 0)
    {
      static const char fsh_portion_summary[] = "ivec2 icell = ivec2(0,0);" SHNL
                                                "vec4  ocoords = vec4(coords.xy*0.5 + vec2(0.5,0.5), relcoords);" SHNL;
      memcpy(&m_to[m_offset], fsh_portion_summary, sizeof(fsh_portion_summary) - 1);  m_offset += sizeof(fsh_portion_summary) - 1;
    }
    else
    {
      if (spDivider == 1)
      {
        m_offset += msprintf(&m_to[m_offset],   "ivec2 icell = ivec2(relcoords.%1 * %d, %d);" SHNL
                                                "relcoords.%1 = (relcoords.%1*icell[1] - icell[0]);" SHNL
                                                "relcoords = mix(relcoords, vec2(-1.0), step(countPortions, float(icell[0])));" SHNL
                                                "vec4  ocoords = vec4(coords.xy*0.5 + vec2(0.5,0.5), relcoords);" SHNL,
                             spDirection == 0? "y" : "x",
                             m_allocatedPortions, m_allocatedPortions
                             );
      }
      else
      {
        m_offset += msprintf(&m_to[m_offset],   "int spDivider = %d;" SHNL
                                                "ivec2 icell = ivec2(0, %d);" SHNL, spDivider, m_allocatedPortions);
        
        m_offset += msprintf(&m_to[m_offset],   "vec2  spdvs = vec2(1.0/spDivider, 1.0/int(icell[1]/float(spDivider) + 0.99) );" SHNL
                                                "ivec2 ispcell = ivec2(relcoords.x/spdvs[%1], relcoords.y/spdvs[%2]);" SHNL
                                                "icell[0] = ispcell[%1]+ispcell[%2]*spDivider;" SHNL
                                                "relcoords.x = relcoords.x / spdvs[%1] - ispcell.x;" SHNL
                                                "relcoords.y = relcoords.y / spdvs[%2] - ispcell.y;" SHNL
                                                "relcoords = mix(relcoords, vec2(-1.0), step(countPortions, float(icell[0])));" SHNL
                                                "vec4  ocoords = vec4(coords.xy*0.5 + vec2(0.5,0.5), relcoords);" SHNL,
                             spDirection == 0? "0" : "1", 
                             spDirection == 0? "1" : "0");
      }
    }
  }
  
  if (initback == INITBACK_BYZERO || initback == INIT_BYZERO)
    m_offset += msprintf(&m_to[m_offset],   "vec3  backcolor = vec3(0.0, 0.0, 0.0);" SHNL);
  else if (initback == INITBACK_BYVALUE || initback == INIT_BYVALUE)
    this->ccolor("backcolor", backcolor);
  else if (initback == INITBACK_BYPALETTE || initback == INIT_BYPALETTE)
    m_offset += msprintf(&m_to[m_offset],   "vec3  backcolor = texture(texPalette, vec2(0.0, 0.0)).rgb;" SHNL);
  
  if (initback == INIT_BYZERO || initback == INIT_BYVALUE || initback == INIT_BYPALETTE)
    m_offset += msprintf(&m_to[m_offset],   "vec3  result = backcolor;" SHNL);
  else
    m_offset += msprintf(&m_to[m_offset],   "vec3  result = vec3(0.0, 0.0, 0.0);" SHNL);
    
  m_offset += msprintf(&m_to[m_offset],     "vec4  post_mask = vec4(0.0, %F, %F, 1.0);" SHNL, fsp.threshold, (float)fsp.weight );   
                                                              /// ppban, threshold, weight, inarea 
}

void FshMainGenerator::main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask& fsp)
{
  _main_begin(initback, backcolor, orient, fsp);
  
//  m_offset += msprintf(&m_to[m_offset],     "ivec2  immod = ivec2( int(mod(fcoords.x, float(iscaling.x))), int(mod(fcoords.y, float(iscaling.y))));" SHNL
//                                            "ivec4  imrect  = ivec4(immod.x, immod.y, iscaling.x-1, iscaling.y-1);" SHNL
//                       );
  
  m_offset += msprintf(&m_to[m_offset],     "ivec2  immod = ivec2( int(mod(relcoords.x*ibounds.x, float(iscaling.x))), int(mod(relcoords.y*ibounds.y, float(iscaling.y))));" SHNL
                                            "ivec4  imrect  = ivec4(immod.x, immod.y, iscaling.x-1, iscaling.y-1);" SHNL
                       );
  m_datamapped = DM_OFF;
}

void FshMainGenerator::main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const DPostmask& fsp, unsigned int dboundsA, unsigned int dboundsB)
{
  _main_begin(initback, backcolor, orient, fsp);
  m_offset += msprintf(&m_to[m_offset],     "ivec2  dbounds_noscaled = ivec2(%d, %d);" SHNL
                                            "vec2   dbounds = vec2(dbounds_noscaled);" SHNL
                                            "ivec2  immod = ivec2(0,0);" SHNL
                                            "ivec4  imrect = ivec4(0);" SHNL,
                       dboundsA, dboundsB);
  m_datamapped = DM_ON;
}

void FshMainGenerator::main_end(const DPostmask &fsp)
{  
  if (fsp.over != PO_OFF)
  {
    const char* dmasks[] = {  
                                  "float ppb_in = sign(step(imrect.x, post_mask[2]) + step(imrect.y, post_mask[2])"
                                          " + step(imrect[2] - imrect.x, post_mask[2]) + step(imrect[3] - imrect.y, post_mask[2]));", // PM_CONTOUR
                                  "float ppb_in = step(imrect.x, post_mask[2]);", // PM_LINELEFT
                                  "float ppb_in = step(imrect[2] - imrect.x, post_mask[2]);", // PM_LINERIGHT
                                  "float ppb_in = step(imrect.y, post_mask[2]);", // PM_LINEBOTTOM
                                  "float ppb_in = step(imrect[3] - imrect.y, post_mask[2]);", // PM_LINETOP
                                  "float ppb_in = sign(step(imrect.x, post_mask[2])+step(imrect.y, post_mask[2]));", //  PM_LINELEFTBOTTOM
                                  "float ppb_in = sign(step(imrect[2] - imrect.x, post_mask[2])+step(imrect.y, post_mask[2]));", //  PM_LINERIGHTBOTTOM
                                  "float ppb_in = sign(step(imrect.x, post_mask[2])+step(imrect[3] - imrect.y, post_mask[2]));", //  PM_LINELEFTTOP
                                  "float ppb_in = sign(step(imrect[2] - imrect.x, post_mask[2])+step(imrect[3] - imrect.y, post_mask[2]));", // PM_LINERIGHTTOP 
                                  "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
                                    "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
//"float ppb_in = smoothstep((post_mask[2]*0.1 + 0.2)*(post_mask[2]*0.1 + 0.2), 0.7*0.7, _ppb_d2);"
                                    "float ppb_in = smoothstep(0.25*0.25, (0.66 - post_mask[2]*0.05)*(0.66 - post_mask[2]*0.05), _ppb_d2);", // PM_CIRCLESMOOTH
                                  "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
                                    "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
//                                    "float ppb_in = smoothstep(0.5*0.5, (0.9 - post_mask[2]*0.02)*(0.9 - post_mask[2]*0.02), _ppb_d2);", // PM_CIRCLEBORDERED
//                                    "float ppb_in = smoothstep(0.3*0.3, (0.6 - post_mask[2]*0.02)*(0.6 - post_mask[2]*0.02), _ppb_d2);", // PM_CIRCLEBORDERED
                                    "float ppb_in = smoothstep(0.4*0.4, (0.6 - post_mask[2]*0.02)*(0.6 - post_mask[2]*0.02), _ppb_d2);", // PM_CIRCLEBORDERED
                                  "float ppb_in = step(abs(imrect.x-imrect[2]/2), post_mask[2])*step(abs(imrect.y-imrect[3]/2), post_mask[2]);", // PM_DOT
                                  "float ppb_in = step(imrect.x, post_mask[2])*step(imrect.y, post_mask[2]);", // PM_DOTLEFTBOTTOM
                                  "float ppb_in = sign( "
                                                        "step(imrect.x, post_mask[2])*step(imrect.y, post_mask[2]) + "
                                                        "step(imrect[2]-imrect.x, post_mask[2])*step(imrect.y, post_mask[2]) + "
                                                        "step(imrect.x, post_mask[2])*step(imrect[3]-imrect.y, post_mask[2]) + "
                                                        "step(imrect[2]-imrect.x, post_mask[2])*step(imrect[3]-imrect.y, post_mask[2]));", // PM_DOTCONTOUR
                                  "float ppb_in = step(mod(abs(imrect.x - imrect.y), post_mask[2] + sign(post_mask[2])*2.0), 0.0);", // PM_SHTRICHL
                                  "float ppb_in = step(mod(abs(imrect.x - imrect[3] + imrect.y), post_mask[2] + sign(post_mask[2])*2.0), 0.0);", // PM_SHTRICHR
                                  "float ppb_in = step(mod(float(imrect.x), 4.0),0.0)*step(mod(float(imrect.y), 4.0), 0.0) + mod(float(imrect.x),2.0)*mod(float(imrect.y),2.0);", // PM_CROSS
                                  "float ppb_in = step(mod(abs(imrect.x - imrect.y), (post_mask[2] + 1.0)*2.0), 0.0);", // PM_GRID
                                  "float ppb_in = step(mod(abs(imrect.x - imrect.y), 2.0 + post_mask[2]) + mod(abs(imrect.x - imrect[3] - 1 + imrect.y), 2.0 + post_mask[2]), 0.0);", // PM_FILL
                                  "float ppb_in = step(mod(abs(imrect.x - imrect.y) + abs(imrect.x - imrect[3] + 1 + imrect.y), 3.0 + post_mask[2]), 0.0);" // PM_SQUARES
      
    };
    
    if (fsp.mask < sizeof(dmasks) / sizeof(const char*))
    {
      m_offset += msprintf(&m_to[m_offset],   "%s" SHNL, dmasks[fsp.mask]);
      
      
      if (fsp.colorManual == -1)
      {
        if (fsp.colorPalette == 0.0f)
          m_offset += msprintf(&m_to[m_offset], "vec3   ppb_color = backcolor;" SHNL);
        else
          m_offset += msprintf(&m_to[m_offset], "vec3   ppb_color = texture(texPalette, vec2(%F, 0.0)).rgb;" SHNL, fsp.colorPalette);
      }
      else
      {
        float rgb[3];
        bsintTocolor3f(fsp.colorManual, rgb);
        m_offset += msprintf(&m_to[m_offset],   "vec3   ppb_color = vec3(%F,%F,%F);" SHNL, rgb[2], rgb[1], rgb[0]);
      }
      m_offset += msprintf(&m_to[m_offset],     "result = mix(result, ppb_color, ppb_in * %s );" SHNL,
                           fsp.over == PO_SIGNAL?  "post_mask[0]" : 
                           fsp.over == PO_EMPTY?   "(1.0 - post_mask[0])" : 
                           fsp.over == PO_ALL?     "1.0" : "0.0"
                                          );
    }
  } // if mask
  else
  {
    m_offset += msprintf(&m_to[m_offset], "float ppb_in = 0.0;" SHNL);
  }
  
  static const char fsh_decltrace[] = "vec4 ovTrace;" SHNL;
  memcpy(&m_to[m_offset], fsh_decltrace, sizeof(fsh_decltrace) - 1);  m_offset += sizeof(fsh_decltrace) - 1;
  for (unsigned int i=0; i<m_ovlscount; i++)
    if (m_ovls[i].link >= 0)
    {
      m_offset += msprintf(&m_to[m_offset],   "loc_f4_sets = ovl_exsettings%D;" SHNL
                                              "loc_i2_pos = ivec2(0,0);" SHNL
                                              "if (step(1.0, loc_f4_sets[0]) != 1)" SHNL
                                              "{" SHNL
                                                "ovTrace = overlayTrace%d(icell, ocoords, loc_f4_sets[1], ovl_position%d, vec3(post_mask[0], post_mask[3], ppb_in), loc_i2_pos);" SHNL
                                                "if (sign(ovTrace[3]) != 0.0 && step(ovMix, loc_f4_sets[2]) == 1.0)" SHNL
                                                  "result = mix(result, overlayColor%d(ovTrace, result), 1.0 - loc_f4_sets[0]);" SHNL
                                              "}" SHNL
                                              "ivec2 ovl_position%d = loc_i2_pos;" SHNL,
                                              i+1, i+1, m_ovls[i].link + 1, i+1, i+1);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],   "loc_f4_sets = ovl_exsettings%D;" SHNL
                                              "loc_i2_pos = ivec2(0,0);" SHNL
                                              "if (step(1.0, loc_f4_sets[0]) != 1)" SHNL
                                              "{" SHNL
                                                 "ovTrace = overlayTrace%d(icell, ocoords, loc_f4_sets[1], ivec2(0,0), vec3(post_mask[0], post_mask[3], ppb_in), loc_i2_pos);" SHNL
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

void FshMainGenerator::ccolor(const char *name, unsigned int value)
{
  float clr[3];
  for (int i=0; i<3; i++)
    clr[i] = ((value >> 8*i) & 0xFF) / 256.0;
  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%F, %F, %F);" SHNL, name, clr[0], clr[1], clr[2]);
}


void FshMainGenerator::cfloatvar(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %f;" SHNL, name, value);  }
void FshMainGenerator::cfloatvar(const char *name, float value1, float value2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%f, %f);" SHNL, name, value1, value2);  }
void FshMainGenerator::cintvar(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;" SHNL, name, value);  }
void FshMainGenerator::cintvar(const char *name, int value1, int value2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, value1, value2);  }

//#include <QDebug>



void FshMainGenerator::value2D(const char* varname, const char* coordsname, const char* portionname)
{
  if (m_impulsegen.type == impulsedata_t::IR_OFF)
  {
    m_offset += msprintf(&m_to[m_offset], "%s = texture(texData, vec2(%s.x, float(%s.y + float(%s))/float(countPortions))).r;" SHNL, 
                                          varname,                    coordsname, coordsname,   portionname);
  }
  
                    /// A
  
  else if (m_impulsegen.type == impulsedata_t::IR_A_COEFF || m_impulsegen.type == impulsedata_t::IR_A_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == DM_OFF ? 
                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "ibounds" : "ibounds_noscaled" :
                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(0, %s.x*%s.x, 1.0/%s.x, float(%s.y + float(%s))/float(countPortions));" SHNL,
                                                          coordsname,  scnosc,   scnosc,     coordsname,  portionname
                        );
    
    for (int i=0; i<m_impulsegen.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = loc_f4_sets[1] - %d;" SHNL, -(i - m_impulsegen.central));
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.x + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_vv[1]*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_f4_sets[1]*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
                                                                              m_impulsegen.coeff[m_impulsegen.central]);
    
    for (int i=m_impulsegen.central+1; i<m_impulsegen.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = loc_f4_sets[1] + %d;" SHNL, i-m_impulsegen.central);
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.x, 1.0 - step(loc_vv[1], %s.x));" SHNL, scnosc, scnosc );
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_vv[1]*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS || m_impulsegen.type == impulsedata_t::IR_A_BORDERS_FIXEDCOUNT)
  {    
    const char* scnosc = m_datamapped == DM_OFF? "ibounds_noscaled.x" : "dbounds_noscaled.x";
    m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(0, (%s.x*(%s)), 1.0/(%s), float(%s.y + float(%s))/float(countPortions));" SHNL,
                                                                   coordsname,  scnosc,             scnosc,               coordsname,  portionname );
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(loc_f4_sets[1] - 1, loc_f4_sets[1] + 1);" SHNL );
    if (m_impulsegen.cycled == 1)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = texture(texData, vec2(%s.x, loc_f4_sets[3])).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(texData, vec2(loc_vv[0]*loc_f4_sets[2], loc_f4_sets[3])).r," SHNL
                                                            "texture(texData, vec2(loc_vv[1]*loc_f4_sets[2], loc_f4_sets[3])).r);" SHNL);
    
    {
      double halfcent = m_impulsegen.central/2.0;
      if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS)
      {
//        m_offset += msprintf(&m_to[m_offset], "ovMix = float(1.0 + imrect[0])/int((imrect[2] + 1)/2);");
//        m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[2]))*(ovMix - 1.0)/2.0;" SHNL,
//                              double(m_impulsegen.count)
//                           );
        
//        m_offset += msprintf(&m_to[m_offset], "ovMix = max((imrect[2] - %f)/2.0, 0.0);" SHNL, halfcent);
        m_offset += msprintf(&m_to[m_offset], "ovMix = float(1.0 + imrect[0])/int((imrect[2] + 1)/2);" SHNL);
        m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[2]))*(ovMix - 1.0)/2.0;" SHNL,
                              double(m_impulsegen.count)
                           );
      }
      else
      {
        m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[2]))*(0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[0])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[2]-imrect[0])/%F, 0.0, 1.0) );" SHNL,
                              double(m_impulsegen.count), halfcent, halfcent, halfcent
                           );
      }
    }
    
    float fc = m_impulsegen.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 0.99f) fc = 0.99f;
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2("
                                            "-ovMix*step(ovMix, 0.0)*(loc_f4_sets[0] - loc_vv[0])/(1.0 - %f*(1.0+ovMix)),"
                                            " ovMix*step(0.0, ovMix)*(loc_f4_sets[0] - loc_vv[1])/(1.0 - %f*(1.0-ovMix)) "
                                          ");" SHNL, fc, fc);
    
    m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = loc_f4_sets[0] - loc_vv.x - loc_vv.y;");
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_f4_sets[0];" SHNL, varname);

  }
  
                          /// B
  
  else if (m_impulsegen.type == impulsedata_t::IR_B_COEFF || m_impulsegen.type == impulsedata_t::IR_B_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == DM_OFF ? 
                           m_impulsegen.type == impulsedata_t::IR_B_COEFF? "ibounds" : "ibounds_noscaled" :
                           m_impulsegen.type == impulsedata_t::IR_B_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(%s.x, %s.y*%s.y, 1.0/%s.y, %s);" SHNL, // float(%s.y + float(%s))/float(countPortions)
                                                          coordsname,  coordsname,  scnosc,   scnosc,     portionname
                        );
    
    for (int i=0; i<m_impulsegen.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = loc_f4_sets[1] - %d;" SHNL, -(i - m_impulsegen.central));
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.y + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_f4_sets[0], (loc_vv[1]*loc_f4_sets[2] + loc_f4_sets[3])/float(countPortions))).r;" SHNL, 
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_f4_sets[0], (loc_f4_sets[1]*loc_f4_sets[2] + loc_f4_sets[3])/float(countPortions))).r;" SHNL, 
                                                                              m_impulsegen.coeff[m_impulsegen.central]);
    
    for (int i=m_impulsegen.central+1; i<m_impulsegen.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = loc_f4_sets[1] + %d;" SHNL, i-m_impulsegen.central);
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.y, 1.0 - step(loc_vv[1], %s.y));" SHNL, scnosc, scnosc );
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texData, vec2(loc_f4_sets[0], (loc_vv[1]*loc_f4_sets[2] + loc_f4_sets[3])/float(countPortions))).r;" SHNL, 
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_impulsegen.type == impulsedata_t::IR_B_BORDERS || m_impulsegen.type == impulsedata_t::IR_B_BORDERS_FIXEDCOUNT)
  {
    
    const char* scnosc = m_datamapped == DM_OFF? "ibounds_noscaled.y" : "dbounds_noscaled.y";
    
    m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(%s.x, (%s.y*(%s)), 1.0/(%s), float(%s));" SHNL,
                                                           coordsname,  coordsname,  scnosc,  scnosc,  portionname );
    
    
    m_offset += msprintf(&m_to[m_offset],  "vec3 loc_vv = vec3(loc_f4_sets[1] - 1, loc_f4_sets[1] + 1, 0.0);" SHNL );
    if (m_impulsegen.cycled == 1)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(texData, vec2(loc_f4_sets[0], (%s.y + loc_f4_sets[3])/float(countPortions))).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(texData, vec2(loc_f4_sets[0], (loc_vv[0]*loc_f4_sets[2] + loc_f4_sets[3])/float(countPortions))).r," SHNL
                                                           "texture(texData, vec2(loc_f4_sets[0], (loc_vv[1]*loc_f4_sets[2] + loc_f4_sets[3])/float(countPortions))).r);" SHNL);
    
    {
      double halfcent = m_impulsegen.central/2.0;
      if (m_impulsegen.type == impulsedata_t::IR_B_BORDERS)
      {
  //      m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[3]))*(float(1.0 + imrect[1])/float(imrect[3] + 1)*2.0 - 1.0)/2.0;" SHNL,
  //                            double(m_impulsegen.count)
  //                         );
        m_offset += msprintf(&m_to[m_offset], "ovMix = float(1.0 + imrect[1])/int((imrect[3] + 1)/2);");
        m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[3]))*(ovMix - 1.0)/2.0;" SHNL,
                              double(m_impulsegen.count)
                           );
      }
      else
      {
        m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[3]))*(0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[1])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[3]-imrect[1])/%F, 0.0, 1.0) );" SHNL,
                              double(m_impulsegen.count), halfcent, halfcent, halfcent
                           );
      }
    }
    
    float fc = m_impulsegen.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 0.99f) fc = 0.99f;
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2("
                                            "-ovMix*step(ovMix, 0.0)*(loc_vv[2] - loc_vv[0])/(1.0 - %f*(1.0+ovMix)),"
                                            " ovMix*step(0.0, ovMix)*(loc_vv[2] - loc_vv[1])/(1.0 - %f*(1.0-ovMix)) "
                                          ");" SHNL, fc, fc);
    m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = loc_vv[2] - loc_vv.x - loc_vv.y;");
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[2];" SHNL, varname);

  }
}



//void FshMainGenerator::value2D(const char* varname, const char* coordsname, const char* portionname)
//{
//  if (m_impulsegen.type == impulsedata_t::IR_OFF)
//  {
//    m_offset += msprintf(&m_to[m_offset], "%s = texture(texData, vec2(%s.x, float(%s.y + float(%s))/float(countPortions))).r;" SHNL, 
//                                          varname,                    coordsname, coordsname,   portionname);
//  }
//  else if (m_impulsegen.type == impulsedata_t::IR_A_COEFF || m_impulsegen.type == impulsedata_t::IR_A_COEFF_NOSCALED)
//  {
//    const char* scnosc = m_datamapped == DM_OFF ? 
//                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "ibounds" : "ibounds_noscaled" :
//                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "dbounds" : "dbounds_noscaled";
    
////    const char* scnosc = m_impulsegen.type == impulsedata_t::IR_A_COEFF? "ibounds" : "ibounds_noscaled";
    
//    m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(0, %s.x*%s.x, 1.0/%s.x, float(%s.y + float(%s))/float(countPortions));" SHNL,
//                                                          coordsname,  scnosc,   scnosc,     coordsname,  portionname
//                        );
    
//    //fmg.push( "vec3 fx = vec3(float(max(relcoords.x*ibounds_noscaled.x, 1) - 1)/ibounds_noscaled.x, "
//    //"relcoords.x, "
//    //"float(min(relcoords.x*ibounds_noscaled.x, ibounds_noscaled.x-1)  + 1)/ibounds_noscaled.x);" SHNL
    
//    for (int i=0; i<m_impulsegen.central; i++)
//      m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = loc_f4_sets[0] + %f * texture(texData, vec2((loc_f4_sets[1] - %d)*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
//                                                                              m_impulsegen.coeff[i],                  -(i - m_impulsegen.central));
    
//    m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = loc_f4_sets[0] + %f * texture(texData, vec2((loc_f4_sets[1] + 0)*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
//                                                                              m_impulsegen.coeff[m_impulsegen.central]);
    
//    for (int i=m_impulsegen.central+1; i<m_impulsegen.count; i++)
//      m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = loc_f4_sets[0] + %f * texture(texData, vec2((loc_f4_sets[1] + %d)*loc_f4_sets[2], loc_f4_sets[3])).r;" SHNL, 
//                                                                              m_impulsegen.coeff[i],                  i-m_impulsegen.central);
    
//    m_offset += msprintf(&m_to[m_offset],  "%s = loc_f4_sets[0];" SHNL, varname);
//  }
//  else if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS || m_impulsegen.type == impulsedata_t::IR_A_BORDERS_FIXEDCOUNT)
//  {
//    double halfcent = m_impulsegen.central/2.0;
//    if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS)
//    {
//      m_offset += msprintf(&m_to[m_offset], "ovMix = (%F+floor((imrect[2] - %F)/1.0));" SHNL,  halfcent, double(m_impulsegen.count));
//      m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[2]))*(0.5 - 0.25/ovMix)*(-clamp(1.0 - (imrect[0])/ovMix, 0.0, 1.0) + clamp(1.0 - float(imrect[2]-imrect[0])/ovMix, 0.0, 1.0) );" SHNL,//+ 1.0 - 
//                            double(m_impulsegen.count)
//                         );
//    }
//    else
//    {
//      m_offset += msprintf(&m_to[m_offset], "ovMix = step(%F, float(imrect[2]))*(0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[0])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[2]-imrect[0])/%F, 0.0, 1.0) );" SHNL,
//                            double(m_impulsegen.count), halfcent, halfcent, halfcent
//                         );
//    }
    
//    if (m_datamapped == DM_OFF)
//      m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(0, (%s.x*(ibounds_noscaled.x)), 1.0/(ibounds_noscaled.x), float(%s.y + float(%s))/float(countPortions));" SHNL,
//                                                                     coordsname,                    coordsname,  portionname );
//    else
//      m_offset += msprintf(&m_to[m_offset],  "loc_f4_sets = vec4(0, (%s.x*(dbounds_noscaled.x)), 1.0/(dbounds_noscaled.x), float(%s.y + float(%s))/float(countPortions));" SHNL,
//                                                                     coordsname,                    coordsname,  portionname );
    
    
//    m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = texture(texData, vec2(%s.x, loc_f4_sets[3])).r;" SHNL, coordsname);
//    m_offset += msprintf(&m_to[m_offset], "loc_f4_sets[0] = (loc_f4_sets[0]"
//                                                              " - (loc_f4_sets[0] - texture(texData, vec2((loc_f4_sets[1] - 1)*loc_f4_sets[2], loc_f4_sets[3])).r)*clamp(-ovMix, 0.0, 1.0)" SHNL
//                                                              " - (loc_f4_sets[0] - texture(texData, vec2((loc_f4_sets[1] + 1)*loc_f4_sets[2], loc_f4_sets[3])).r)*clamp(ovMix, 0.0, 1.0)" SHNL
//                                                                 ");");
//    m_offset += msprintf(&m_to[m_offset],  "%s = loc_f4_sets[0];" SHNL, varname);

//  }
////  qDebug()<<m_to;
//}


//  const char* rotaters[] = {      "vec2 rotater(vec2 coords){ return coords; }",
//                                  "vec2 rotater(vec2 coords){ coords.x = 1.0-coords.x; return coords; }",
//                                  "vec2 rotater(vec2 coords){ coords.y = 1.0-coords.y; return coords; }",
//                                  "vec2 rotater(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords; }",
//                                  "vec2 rotater(vec2 coords){ coords.y = 1.0-coords.y; return coords.yx; }",
//                                  "vec2 rotater(vec2 coords){ return coords.yx; }",
//                                  "vec2 rotater(vec2 coords){ coords.xy = vec2(1.0,1.0)-coords.xy; return coords.yx; }",
//                                  "vec2 rotater(vec2 coords){ coords.x = 1.0-coords.x; return coords.yx; }"
//                                };
//  m_offset += msprintf(&m_to[m_offset], "%s" SHNL, rotaters[orient]);
/*
 * //                                    "vec2  relcoords = rotater(vec2(coords.xy*0.5 + vec2(0.5,0.5)));" SHNL
 * 
//                                  "vec2  fcoords = floor((coords.xy+vec2(1,1))*fbounds/vec2(2.0,2.0) + vec2(0.49, 0.49));" SHNL
//                                  "vec2  fcoords_noscaled = floor((coords.xy+vec2(1,1))*fbounds_noscaled/vec2(2.0,2.0) + vec2(0.49, 0.49));" SHNL
//                                    "vec2  fcoords = floor((coords.xy+vec2(1,1))*fbounds/vec2(2.0,2.0));" SHNL
//                                    "vec2  fcoords_noscaled = floor((coords.xy+vec2(1,1))*fbounds_noscaled/vec2(2.0,2.0));" SHNL
                                  
//                                    "vec2  fcoords = floor(relcoords*(fbounds) + vec2(0.49, 0.49));" SHNL
//                                    "vec2  fcoords_noscaled = floor(relcoords*(fbounds_noscaled) + vec2(0.49, 0.49));" SHNL
//                                  "vec2  fcoords = floor(relcoords*(fbounds - vec2(1,0)) + vec2(0.49999, 0.49999));" SHNL
//                                  "vec2  fcoords_noscaled = floor(relcoords*(fbounds_noscaled - vec2(1,0)) + vec2(0.49999, 0.49999));" SHNL
                                  
                                  //                                  "vec2  fcoords = floor(relcoords*(fbounds - vec2(1,1)) + vec2(0.49, 0.49));" SHNL
                                  //                                  "vec2  fcoords_noscaled = floor(relcoords*(fbounds_noscaled - vec2(1,1)) + vec2(0.49, 0.49));" SHNL
                                  
//                                    "vec2  fcoords = floor(relcoords*(fbounds - vec2(1,1)) + vec2(0.49, 0.49));" SHNL
//                                    "vec2  fcoords_noscaled = floor(relcoords*(fbounds_noscaled - vec2(1,1)) + vec2(0.49, 0.49));" SHNL
//                                  "vec2  fcoords = floor(relcoords*(fbounds - vec2(1,1)));" SHNL
//                                  "vec2  fcoords_noscaled = floor(relcoords*(fbounds_noscaled - vec2(1,1)));" SHNL
//                                  "vec2  fcoords = floor(relcoords*fbounds);" SHNL
//                                  "vec2  fcoords_noscaled = floor(relcoords*fbounds_noscaled);" SHNL
                                  
//                                    "ivec2 icoords = ivec2(fcoords);" SHNL
//                                    "ivec2 icoords_noscaled = ivec2(fcoords_noscaled);" SHNL

*/
//  "result = mix(result, ppb_color, post_mask[1]*post_mask[0]*ppb_in + post_mask[2]*(1.0 - post_mask[0])*ppb_in );" SHNL,
