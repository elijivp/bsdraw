/// This file is a part of shader-code-generation subsystem
/// You dont need to use classes from this file directly
/// Created By: Elijah Vlasov
#include "bsshgenmain.h"

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

#include <QDebug>

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

unsigned int FshDrawConstructor::basePendingSize(const impulsedata_t& imp, unsigned int ovlscount)
{
  unsigned int base = 2300 + ovlscount*1000;
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

FshDrawConstructor::FshDrawConstructor(char *deststring, unsigned int allocatedPortions, SPLITPORTIONS splitPortions, const impulsedata_t& imp, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[]): 
  m_writebase(deststring), m_to(deststring), m_allocatedPortions(allocatedPortions), m_offset(0), m_splitPortions(splitPortions), m_impulsegen(imp), m_ovlscount(ovlscount), m_ovls(ovlsinfo)
{
#ifdef BSGLSLVER
  m_offset += msprintf(&m_to[m_offset],   "#version %d" SHNL, BSGLSLVER);
#endif
  static const char fsh_base[] =          "uniform highp sampler2D  texdata;" SHNL
                                          "uniform highp int        texdatadimm_a;" SHNL
                                          "uniform highp int        texdatadimm_b;" SHNL
                                          "uniform highp int        scaler_a;" SHNL
                                          "uniform highp int        scaler_b;" SHNL
                                          "uniform highp int        portions;" SHNL
                                          "uniform highp sampler2D  texpalette;" SHNL
                                          "uniform highp vec2       palrange;" SHNL
                                          "in highp vec2            coords;" SHNL
#if defined BSGLSLVER && BSGLSLVER >= 420
                                          "out vec4                 outcolor;" SHNL
#endif
                                          "float getValue1D(in int portion, in float x){  return texture(texdata, vec2(x, float(portion)/(float(portions)-1.0))).r; }" SHNL
                                          "float getValue2D(in int portion, in vec2  x){  return texture(texdata, vec2(x.x, float(x.y + float(portion))/float(portions))).r; }" SHNL
                                          "vec3  insider(int i, ivec2 ifromvec) { float scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));\n\treturn vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }" SHNL;

  memcpy(&m_to[m_offset], fsh_base, sizeof(fsh_base) - 1);  m_offset += sizeof(fsh_base) - 1;
  for (unsigned int i=0; i<m_ovlscount; i++)
  {
    m_offset += msprintf(&m_to[m_offset], "uniform highp vec4 ovl_otss_%D;" SHNL
                                          "vec4 overlayOVCoords%d(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 shapeself);" SHNL
                                          "vec3 overlayColor%d(in vec4 in_variant, in vec3 color);" SHNL,
                        i+1, i+1, i+1);
  }
}



void FshDrawConstructor::_main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const overpattern_t& fsp)
{
  m_orient = orient;
  static const char fsh_main[] =  "void main()" SHNL
                                  "{" SHNL
                                    "float mixwell = 1.0;" SHNL
                                    "float dvalue = 0.0;" SHNL
                                    "vec4  ovl_cur_otss;" SHNL
                                    "ivec2 ovl_transfer_pos;" SHNL
                                    
                                    "ivec2 ab_indimms = ivec2(texdatadimm_a, texdatadimm_b);" SHNL
                                    "ivec2 ab_iscaler = ivec2(scaler_a, scaler_b);" SHNL
                                    "ivec2 ab_ibounds = ab_indimms*ab_iscaler;" SHNL;
  memcpy(&m_to[m_offset], fsh_main, sizeof(fsh_main) - 1);  m_offset += sizeof(fsh_main) - 1;
  
  
  
  {
    const char* rotaters[] = {              "",
                                            "ab_coords.x = 1.0-ab_coords.x;",
                                            "ab_coords.y = 1.0-ab_coords.y;",
                                            "ab_coords.xy = vec2(1.0,1.0)-ab_coords.xy;",
                                            "ab_coords.xy = vec2(1.0-ab_coords.y, ab_coords.x);",
                                            "ab_coords.xy = ab_coords.yx;",
                                            "ab_coords.xy = vec2(1.0,1.0)-ab_coords.yx;",
                                            "ab_coords.xy = vec2(ab_coords.y, 1.0-ab_coords.x);"
                                  };
    
    m_offset += msprintf(&m_to[m_offset], 
                                    "vec2  xy_coords = vec2(coords.xy*0.5 + vec2(0.5,0.5));" SHNL     ///   non-oriented non-cell coords
                                    "vec2  ab_coords = xy_coords;         %s    " SHNL                ///   oriented non-cell coords
                                    "vec2  abc_coords = ab_coords;" SHNL                              ///   oriented cell coords
                                , rotaters[orient]);
  }
  
  if (m_splitPortions == 0)
  {
    m_offset += msprintf(&m_to[m_offset],   
                                    "ivec2 ispcell = ivec2(0,0);" SHNL
                                    "ivec2 icells = ivec2(1,1);" SHNL
                         );
  }
  else
  {
    int spRotator =   (m_splitPortions >> 16)&0xFF;   // 0 - BT/LR, 1 - TB/RL
    int spDirection = (m_splitPortions >> 8)&0xFF;    // 0 - Columns, 1 - Rows
    int spDivider =    m_splitPortions&0xFF;
    
    if (spDivider == 1)
    {
      m_offset += msprintf(&m_to[m_offset],   "int explicitPortion = int(abc_coords.%1*%d);" SHNL
                                              "abc_coords.%1 = abc_coords.%1 * %d - explicitPortion;" SHNL,
                                              spDirection == 0? "y" : "x",
                                              m_allocatedPortions, m_allocatedPortions
                          );
      if (spRotator)
        m_offset += msprintf(&m_to[m_offset], "explicitPortion = %d - 1 - explicitPortion;" SHNL, m_allocatedPortions);
      
      if (spDirection == 0)
        m_offset += msprintf(&m_to[m_offset], 
                                    "ivec2 ispcell = ivec2(explicitPortion, 0);" SHNL
                                    "ivec2 icells = ivec2(%d, 1);" SHNL
                             , m_allocatedPortions
                             );
      else
        m_offset += msprintf(&m_to[m_offset], 
                                    "ivec2 ispcell = ivec2(0, explicitPortion);" SHNL
                                    "ivec2 icells = ivec2(1, %d);" SHNL
                             , m_allocatedPortions
                             );
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],   "int    spDivider = %d;" SHNL, spDivider );        
      m_offset += msprintf(&m_to[m_offset],   "ivec2  icells = ivec2(spDivider, %d/float(spDivider) + 0.99);" SHNL
                                              "ivec2  ispcell = ivec2(abc_coords.x * icells[%1], abc_coords.y * icells[%2]);" SHNL
                                              "int explicitPortion = ispcell[%1] + ispcell[%2]*spDivider;" SHNL
                                              "abc_coords.x = abc_coords.x * icells[%1] - ispcell.x;" SHNL
                                              "abc_coords.y = abc_coords.y * icells[%2] - ispcell.y;" SHNL,
                                              m_allocatedPortions,
                                              spDirection == 0? "0" : "1", 
                                              spDirection == 0? "1" : "0"
                          );
    }
    m_offset += msprintf(&m_to[m_offset],     "abc_coords = mix(abc_coords, vec2(-1.0), step(portions, float(explicitPortion)));" SHNL);
  }
  
  
  if (initback == INITBACK_BYZERO || initback == INIT_BYZERO)
    m_offset += msprintf(&m_to[m_offset],   "vec3  backcolor = vec3(0.0, 0.0, 0.0);" SHNL);
  else if (initback == INITBACK_BYVALUE || initback == INIT_BYVALUE)
    this->ccolor("backcolor", backcolor);
  else if (initback == INITBACK_BYPALETTE || initback == INIT_BYPALETTE)
    m_offset += msprintf(&m_to[m_offset],   "vec3  backcolor = texture(texpalette, vec2(0.0, 0.0)).rgb;" SHNL);
  
  if (initback == INIT_BYZERO || initback == INIT_BYVALUE || initback == INIT_BYPALETTE)
    m_offset += msprintf(&m_to[m_offset],   "vec3  result = backcolor;" SHNL);
  else
    m_offset += msprintf(&m_to[m_offset],   "vec3  result = vec3(0.0, 0.0, 0.0);" SHNL);
    
  m_offset += msprintf(&m_to[m_offset],     "vec4  post_mask = vec4(0.0, %F, %F, %F);" SHNL, fsp.threshold, fsp.weight, 1.0f - fsp.smooth );   
                                                              /// ppban, threshold, weight, smooth
}

void FshDrawConstructor::main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const overpattern_t& fsp)
{
  _main_begin(initback, backcolor, orient, fsp);
  
//  m_offset += msprintf(&m_to[m_offset],     "ivec2  immod = ivec2( int(mod(fcoords.x, float(ab_iscaler.x))), int(mod(fcoords.y, float(ab_iscaler.y))));" SHNL
//                                            "ivec4  imrect  = ivec4(immod.x, immod.y, ab_iscaler.x-1, ab_iscaler.y-1);" SHNL
//                       );
  
  m_offset += msprintf(&m_to[m_offset],     "ivec2  immod = ivec2( int(mod(abc_coords.x*ab_ibounds.x, float(ab_iscaler.x))), int(mod(abc_coords.y*ab_ibounds.y, float(ab_iscaler.y))));" SHNL
                                            "ivec4  imrect  = ivec4(immod.x, immod.y, ab_iscaler.x-1, ab_iscaler.y-1);" SHNL
                       );
  m_datamapped = DM_OFF;
}

void FshDrawConstructor::main_begin(int initback, unsigned int backcolor, ORIENTATION orient, const overpattern_t& fsp, unsigned int dboundsA, unsigned int dboundsB)
{
  _main_begin(initback, backcolor, orient, fsp);
  m_offset += msprintf(&m_to[m_offset],     "ivec2  dbounds_noscaled = ivec2(%d, %d);" SHNL
                                            "vec2   dbounds = vec2(dbounds_noscaled);" SHNL
                                            "ivec2  immod = ivec2(0,0);" SHNL
                                            "ivec4  imrect = ivec4(0);" SHNL,
                       dboundsA, dboundsB);
  m_datamapped = DM_ON;
}

void FshDrawConstructor::main_end(const overpattern_t& fsp)
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
      m_offset += msprintf(&m_to[m_offset], "%s" SHNL, dmasks[fsp.mask >= _OP_TOTAL ? OP_CROSS : fsp.mask]);
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
      m_offset += msprintf(&m_to[m_offset], "%s" SHNL, dmasks_sigm[fsp.mask >= _OPF_TOTAL ? OPF_CROSS : fsp.mask]);
      m_offset += msprintf(&m_to[m_offset], "ppb_in = clamp(ppb_in*(1.0+12.0*post_mask[3])/(1.0 + abs(ppb_in)*(1.0+12.0*post_mask[3]))*1.5 + 0.5, 0.0, 1.0);" SHNL);
    }
    else // ANGLEFIGURES
    {
//      m_offset += msprintf(&m_to[m_offset], "vec4 ppb_a4 = vec4("
//                                            "distance(vec2(imrect.x, imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect[2] - 1 - imrect.x, imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect.x, imrect[3] - 1 - imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0)),"
//                                            "distance(vec2(imrect[2] - 1 - imrect.x, imrect[3] - 1 - imrect.y), vec2(imrect[2]/2.0, imrect[3]/2.0))"
//                                            ");" SHNL);
      
      m_offset += msprintf(&m_to[m_offset], "vec2 ppb_a2 = vec2("
                                              "mix(float(imrect.x), float(imrect[2] - imrect.x), step(imrect[2]/2.0, float(imrect.x))),"
                                              "mix(float(imrect.y), float(imrect[3] - imrect.y), step(imrect[3]/2.0, float(imrect.y)))"
                                            ");"  SHNL
                                            "ppb_a2 = ppb_a2 / vec2(imrect[2]/2.0, imrect[3]/2.0);" SHNL
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
      m_offset += msprintf(&m_to[m_offset], "%s" SHNL, dmasks_anglefigures[fsp.mask >= _OPA_TOTAL ? OPA_PUFFCIRCLE : fsp.mask]);
      
      //return 0.5f + 0.5f*(x-xpos)*speed/(1.0f + fabs(x-xpos)*speed);
      m_offset += msprintf(&m_to[m_offset], "ppb_in = 0.5 + 0.5*(ppb_in*50*post_mask[3])/(1.0 + abs(ppb_in)*50*post_mask[3]);" SHNL);
//      m_offset += msprintf(&m_to[m_offset], "ppb_in = clamp(ppb_in*(1.0+12.0*post_mask[3])/(1.0 + abs(ppb_in)*(1.0+12.0*post_mask[3]))*1.5 + 0.5, 0.0, 1.0);" SHNL);
    }
      
    /*
     * 
     *  const char* dmasks[] = {  
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
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), (post_mask[2] + 1.0)*2.0), 0.0);", 
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
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), post_mask[2] + sign(post_mask[2])*2.0), 0.0);", 
        // OP_SHTRICHR
            "float ppb_in = step(mod(abs(imrect.x - imrect[3] + imrect.y), post_mask[2] + sign(post_mask[2])*2.0), 0.0);",
        // OP_CROSS
            "float ppb_in = step(mod(float(imrect.x), 4.0),0.0)*step(mod(float(imrect.y), 4.0), 0.0) + mod(float(imrect.x),2.0)*mod(float(imrect.y),2.0);", 
        // OP_FILL
            "float ppb_in = step(mod(abs(imrect.x - imrect.y), 2.0 + post_mask[2]) + mod(abs(imrect.x - imrect[3] - 1 + imrect.y), 2.0 + post_mask[2]), 0.0);", 
        // OP_SQUARES
            "float ppb_in = step(mod(abs(imrect.x - imrect.y) + abs(imrect.x - imrect[3] + 1 + imrect.y), 3.0 + post_mask[2]), 0.0);"
      };
      
      
     * // OPF_CIRCLE
          "float ppb_in = distance(0.25*0.25, (0.66 - post_mask[2]*0.05)*(0.66 - post_mask[2]*0.05), _ppb_d2);",
          "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
              "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
              "float ppb_in = smoothstep(0.25*0.25, (0.66 - post_mask[2]*0.05)*(0.66 - post_mask[2]*0.05), _ppb_d2);",
        // OPF_CROSSPUFF
            "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
              "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
              "float ppb_in = smoothstep(0.4*0.4, (0.6 - post_mask[2]*0.02)*(0.6 - post_mask[2]*0.02), _ppb_d2);",
//        // OPF_CIRCLE2
//            "vec2 _ppb_pos = vec2(float(imrect[2])/imrect[3], float(imrect[3])/imrect[2]);"
//            "_ppb_pos = vec2(max(_ppb_pos.x, 1.0), max(_ppb_pos.y, 1.0));"
//            "_ppb_pos = _ppb_pos*vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
//              "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
//              "float ppb_in = smoothstep(0.25*0.25, (0.66 - post_mask[2]*0.05)*(0.66 - post_mask[2]*0.05), _ppb_d2);",
                
//        // OPF_CROSSPUFF2
//            "vec2 _ppb_pos = vec2(float(imrect[2])/imrect[3], float(imrect[3])/imrect[2]);"
//              "_ppb_pos = vec2(max(_ppb_pos.x, 1.0), max(_ppb_pos.y, 1.0));"
//              "_ppb_pos = _ppb_pos*vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
//              "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
//              "float ppb_in = smoothstep(0.4*0.4, (0.6 - post_mask[2]*0.02)*(0.6 - post_mask[2]*0.02), _ppb_d2);",
        
        // OPF_RHOMB
            "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));" 
              "float ppb_in = step(0.25 + post_mask[2]*0.05, _ppb_pos.x + _ppb_pos.y);", 
        // OPF_SURIKEN
            "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));" 
  //            "float ppb_in = step(0.25 + post_mask[2]*0.05, _ppb_pos.x + _ppb_pos.y + 2*_ppb_pos.x*_ppb_pos.y);", 
              "float ppb_in = clamp(_ppb_pos.x + _ppb_pos.y + (2+post_mask[2])*_ppb_pos.x*_ppb_pos.y, 0.0, 1.0) - 0.5;"
              "ppb_in = 0.5 + 0.5*ppb_in*26/(1.0 + abs(ppb_in)*26);",  
        // OPF_DONUT
  //      inline float  bsf_step(float x, float xpos, float speed=200.0f){ return 0.5f + 0.5f*(x-xpos)*speed/(1.0f + fabs(x-xpos)*speed); }
            "vec2 _ppb_pos = vec2(abs(0.5 - float(imrect.x)/imrect[2]), abs(0.5 - float(imrect.y)/imrect[3]));"
              "float _ppb_d2 = dot(_ppb_pos, _ppb_pos);"
              "float ppb_in = smoothstep(0.25*0.25, (0.15 + post_mask[2]*0.05)*(0.15 + post_mask[2]*0.05), _ppb_d2);",
        // OPF_BULL
  //          "vec2 _ppb_pos = vec2(abs(float(imrect.x)/imrect[2] - float(imrect.y)/imrect[3]), abs(0.5 - float(imrect.x)/imrect[2]));"
            "float _ppb_pos = abs(float(imrect.x)/imrect[2] - float(imrect.y)/imrect[3])* abs(1.0 - 0.5 + float(imrect.x)/imrect[2]);"
              "float ppb_in = _ppb_pos;", 
        // OPF_BULR
            "float _ppb_pos = abs(float(imrect.x)/imrect[2] - float(imrect[3] - imrect.y)/imrect[3])* abs(1.0 - 0.5 + float(imrect.x)/imrect[2]);"
              "float ppb_in = _ppb_pos;", 
      
        };
*/
    if (fsp.colorByPalette)
    {
      if (fsp.color.r == 0.0f)
        m_offset += msprintf(&m_to[m_offset], "vec3   ppb_color = backcolor;" SHNL);
      else
        m_offset += msprintf(&m_to[m_offset], "vec3   ppb_color = texture(texpalette, vec2(%F, 0.0)).rgb;" SHNL, fsp.color.r);
    }
    else
    {
      m_offset += msprintf(&m_to[m_offset],   "vec3   ppb_color = vec3(%F,%F,%F);" SHNL, fsp.color.r, fsp.color.g, fsp.color.b);
    }
    m_offset += msprintf(&m_to[m_offset],     "result = mix(result, ppb_color, ppb_in * %s );" SHNL,
                             fsp.algo == overpattern_t::OALG_THRS_PLUS?   "post_mask[0]" : 
                             fsp.algo == overpattern_t::OALG_THRS_MINUS?  "(1.0 - post_mask[0])" : 
                             fsp.algo == overpattern_t::OALG_ANY?         "1.0" : 
                                                                          "0.0"
                         );
  } // if mask

  
  static const char fsh_decltrace[] = "vec4 ovTrace;" SHNL;
  memcpy(&m_to[m_offset], fsh_decltrace, sizeof(fsh_decltrace) - 1);  m_offset += sizeof(fsh_decltrace) - 1;
 
  bool transposed = orientationTransposed(m_orient);
  
  for (unsigned int i=0; i<m_ovlscount; i++)
  {
//    bool ovl_transposed = m_ovls[i].orient == OO_AREA_TBLR || m_ovls[i].orient == OO_AREA_BTLR || m_ovls[i].orient == OO_AREA_TBRL || m_ovls[i].orient == OO_AREA_BTRL;
//    bool area = m_ovls[i].orient >= OO_AREA_LRBT;
                        
    m_offset += msprintf(&m_to[m_offset],   "ovl_cur_otss = ovl_otss_%D;" SHNL    // opacity, thickness, slice
                                            "ovl_transfer_pos = ivec2(0,0);" SHNL, i+1);
    
    
    if (m_ovls[i].link >= 0)
      m_offset += msprintf(&m_to[m_offset], "bool ovl_visible_%d = ovl_visible_%d && step(1.0, ovl_cur_otss[0]) != 1;" SHNL, i+1, m_ovls[i].link + 1 );
    else
      m_offset += msprintf(&m_to[m_offset], "bool ovl_visible_%d = step(1.0, ovl_cur_otss[0]) != 1;" SHNL, i+1 );
    
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
    const char* ocall_ovcoords = ovl_coords_oriented[m_ovls[i].orient];
    if (m_ovls[i].orient < OO_AREA_LRBT)
    {
      ocall_indimms = "ab_indimms.xy";
      ocall_iscaler = "ab_iscaler.xy";
      ocall_ibounds = "ab_ibounds.xy"; 
    }
    else
    {
      if (m_ovls[i].orient == OO_AREA_LRBT || m_ovls[i].orient == OO_AREA_RLBT || m_ovls[i].orient == OO_AREA_LRTB || m_ovls[i].orient == OO_AREA_RLTB)
      {
        ocall_indimms = transposed ? "ab_indimms.yx" : "ab_indimms.xy";
        ocall_iscaler = transposed ? "ab_iscaler.yx" : "ab_iscaler.xy";
        ocall_ibounds = transposed ? "ab_indimms.yx*ab_iscaler.yx*icells.xy" : "ab_indimms.xy*ab_iscaler.xy*icells.yx";
      }
      else if (m_ovls[i].orient == OO_AREA_TBLR || m_ovls[i].orient == OO_AREA_BTLR || m_ovls[i].orient == OO_AREA_TBRL || m_ovls[i].orient == OO_AREA_BTRL)
      {
        ocall_indimms = transposed ? "ab_indimms.xy" : "ab_indimms.yx";
        ocall_iscaler = transposed ? "ab_iscaler.xy" : "ab_iscaler.yx";
        ocall_ibounds = transposed ? "ab_indimms.xy*ab_iscaler.xy*icells.yx" : "ab_indimms.yx*ab_iscaler.yx*icells.xy";
      }
      else if (m_ovls[i].orient == OO_AREAOR || m_ovls[i].orient == OO_AREAOR_MIRROR_HORZ || m_ovls[i].orient == OO_AREAOR_MIRROR_VERT || m_ovls[i].orient == OO_AREAOR_MIRROR_BOTH)
      {
        ocall_indimms = "ab_indimms.xy";
        ocall_iscaler = "ab_iscaler.xy";
        ocall_ibounds = transposed ? "ab_indimms.yx*ab_iscaler.yx*icells.xy" : "ab_indimms.xy*ab_iscaler.xy*icells.yx";
      }
      else
        ;
    }
    
    
    m_offset += msprintf(&m_to[m_offset],   "if  (ovl_visible_%d)" SHNL
                                            "{" SHNL
                                               "ovTrace = overlayOVCoords%d(ispcell, %s, %s, %s, %s, ovl_cur_otss[1], "
                                            , 
                                            i+1, i+1, ocall_indimms, ocall_iscaler, ocall_ibounds, ocall_ovcoords
                                            );
    
    if (m_ovls[i].link >= 0)
      m_offset += msprintf(&m_to[m_offset],       "ovl_pos_%d, vec3(post_mask[0], post_mask[3], ppb_in), ovl_transfer_pos);" SHNL
                                            , m_ovls[i].link + 1);
    else
      m_offset += msprintf(&m_to[m_offset],       "ivec2(0,0), vec3(post_mask[0], post_mask[3], ppb_in), ovl_transfer_pos);" SHNL );
    
    
    m_offset += msprintf(&m_to[m_offset],     "if (sign(ovTrace[3]) != 0.0 && (step(mixwell, 0.0) == 1 || (step(dvalue, ovl_cur_otss[2]) == 0 && step(ovl_cur_otss[3], dvalue) == 0)) )" SHNL
                                                "result = mix(result, overlayColor%d(ovTrace, result), 1.0 - ovl_cur_otss[0]);" SHNL
                                            "}" SHNL
                                            "ivec2 ovl_pos_%d = ovl_transfer_pos;" SHNL
                                            , 
                                            i+1, i+1);
  }
#if !defined BSGLSLVER || BSGLSLVER < 420
  static const char fsh_end[] =   "gl_FragColor = vec4(result, 0.0);" SHNL "}" SHNL;
#else
  static const char fsh_end[] =   "outcolor = vec4(result, 0.0);" SHNL "}" SHNL;
#endif
//  static const char fsh_end[] =   "fragColor = vec4(result, 0.0);" SHNL "}" SHNL;
  memcpy(&m_to[m_offset], fsh_end, sizeof(fsh_end) - 1); m_offset += sizeof(fsh_end) - 1;
  m_to[m_offset++] = '\0';
}

void FshDrawConstructor::push(const char* text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
#ifdef AUTOSHNL
  text = SHNL;
  while (*text != '\0')
    m_to[m_offset++] = *text++;
#endif
}

void FshDrawConstructor::pushin(const char* text)
{
  while (*text != '\0')
    m_to[m_offset++] = *text++;
}

void FshDrawConstructor::ccolor(const char *name, unsigned int value)
{
  float clr[3];
  for (int i=0; i<3; i++)
    clr[i] = ((value >> 8*i) & 0xFF) / 256.0;
  m_offset += msprintf(&m_to[m_offset], "const vec3 %s = vec3(%F, %F, %F);" SHNL, name, clr[0], clr[1], clr[2]);
}


void FshDrawConstructor::cfloatvar(const char *name, float value){  m_offset += msprintf(&m_to[m_offset], "const float %s = %f;" SHNL, name, value);  }
void FshDrawConstructor::cfloatvar(const char *name, float value1, float value2){  m_offset += msprintf(&m_to[m_offset], "const vec2 %s = vec2(%f, %f);" SHNL, name, value1, value2);  }
void FshDrawConstructor::cintvar(const char *name, int value){  m_offset += msprintf(&m_to[m_offset], "const int %s = %d;" SHNL, name, value);  }
void FshDrawConstructor::cintvar(const char *name, int value1, int value2){  m_offset += msprintf(&m_to[m_offset], "const ivec2 %s = ivec2(%d, %d);" SHNL, name, value1, value2);  }

//#include <QDebug>



void FshDrawConstructor::value2D(const char* varname, const char* coordsname, const char* portionname)
{
  if (m_impulsegen.type == impulsedata_t::IR_OFF)
  {
    m_offset += msprintf(&m_to[m_offset], "%s = texture(texdata, vec2(%s.x, float(%s.y + float(%s))/float(portions))).r;" SHNL,
                                          varname,                    coordsname, coordsname,   portionname);
  }
  
                    /// A
  
  else if (m_impulsegen.type == impulsedata_t::IR_A_COEFF || m_impulsegen.type == impulsedata_t::IR_A_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == DM_OFF ? 
                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "ab_ibounds" : "ab_indimms" :
                           m_impulsegen.type == impulsedata_t::IR_A_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 impulsegrad = vec4(0, %s.x*%s.x, 1.0/%s.x, float(%s.y + float(%s))/float(portions));" SHNL,
                                                          coordsname,  scnosc,   scnosc,     coordsname,  portionname
                        );
    
    for (int i=0; i<m_impulsegen.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = impulsegrad[1] - %d;" SHNL, -(i - m_impulsegen.central));
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.x + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(loc_vv[1]*impulsegrad[2], impulsegrad[3])).r;" SHNL,
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(impulsegrad[1]*impulsegrad[2], impulsegrad[3])).r;" SHNL,
                                                                              m_impulsegen.coeff[m_impulsegen.central]);
    
    for (int i=m_impulsegen.central+1; i<m_impulsegen.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = impulsegrad[1] + %d;" SHNL, i-m_impulsegen.central);
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.x, 1.0 - step(loc_vv[1], %s.x));" SHNL, scnosc, scnosc );
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(loc_vv[1]*impulsegrad[2], impulsegrad[3])).r;" SHNL,
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS || m_impulsegen.type == impulsedata_t::IR_A_BORDERS_FIXEDCOUNT)
  {    
    const char* scnosc = m_datamapped == DM_OFF? "ab_indimms.x" : "dbounds_noscaled.x";
    m_offset += msprintf(&m_to[m_offset],  "vec4 impulsegrad = vec4(0, (%s.x*(%s)), 1.0/(%s), float(%s.y + float(%s))/float(portions));" SHNL,
                                                                   coordsname,  scnosc,             scnosc,               coordsname,  portionname );
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(impulsegrad[1] - 1, impulsegrad[1] + 1);" SHNL );
    if (m_impulsegen.cycled == 1)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "impulsegrad[0] = texture(texdata, vec2(%s.x, impulsegrad[3])).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(texdata, vec2(loc_vv[0]*impulsegrad[2], impulsegrad[3])).r," SHNL
                                                            "texture(texdata, vec2(loc_vv[1]*impulsegrad[2], impulsegrad[3])).r);" SHNL);
    
    {
      double halfcent = m_impulsegen.central/2.0;
      if (m_impulsegen.type == impulsedata_t::IR_A_BORDERS)
      {
        m_offset += msprintf(&m_to[m_offset], "float impulsemix = float(1.0 + imrect[0])/int((imrect[2] + 1)/2);" SHNL);
        m_offset += msprintf(&m_to[m_offset], "impulsemix = step(%F, float(imrect[2]))*(impulsemix - 1.0)/2.0;" SHNL,
                              double(m_impulsegen.count)
                           );
//        qDebug()<<m_to;
      }
      else
      {
        m_offset += msprintf(&m_to[m_offset], "float impulsemix = step(%F, float(imrect[2]))*(0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[0])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[2]-imrect[0])/%F, 0.0, 1.0) );" SHNL,
                              double(m_impulsegen.count), halfcent, halfcent, halfcent
                           );
      }
    }
    
    float fc = m_impulsegen.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 0.99f) fc = 0.99f;
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2("
                                            "-impulsemix*step(impulsemix, 0.0)*(impulsegrad[0] - loc_vv[0])/(1.0 - %f*(1.0+impulsemix)),"
                                            " impulsemix*step(0.0, impulsemix)*(impulsegrad[0] - loc_vv[1])/(1.0 - %f*(1.0-impulsemix)) "
                                          ");" SHNL, fc, fc);
    
    m_offset += msprintf(&m_to[m_offset], "impulsegrad[0] = impulsegrad[0] - loc_vv.x - loc_vv.y;");
    
    m_offset += msprintf(&m_to[m_offset],  "%s = impulsegrad[0];" SHNL, varname);

  }
  
                          /// B
  
  else if (m_impulsegen.type == impulsedata_t::IR_B_COEFF || m_impulsegen.type == impulsedata_t::IR_B_COEFF_NOSCALED)
  {
    m_offset += msprintf(&m_to[m_offset],  "vec2 loc_vv = vec2(0.0, 0.0);" SHNL );
                         
    const char* scnosc = m_datamapped == DM_OFF ? 
                           m_impulsegen.type == impulsedata_t::IR_B_COEFF? "ab_ibounds" : "ab_indimms" :
                           m_impulsegen.type == impulsedata_t::IR_B_COEFF? "dbounds" : "dbounds_noscaled";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 impulsegrad = vec4(%s.x, %s.y*%s.y, 1.0/%s.y, %s);" SHNL, // float(%s.y + float(%s))/float(portions)
                                                          coordsname,  coordsname,  scnosc,   scnosc,     portionname
                        );
    
    for (int i=0; i<m_impulsegen.central; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = impulsegrad[1] - %d;" SHNL, -(i - m_impulsegen.central));
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], %s.y + loc_vv[1], 1.0 - step(0.0, loc_vv[1]));" SHNL, scnosc);
        
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(impulsegrad[0], (loc_vv[1]*impulsegrad[2] + impulsegrad[3])/float(portions))).r;" SHNL,
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(impulsegrad[0], (impulsegrad[1]*impulsegrad[2] + impulsegrad[3])/float(portions))).r;" SHNL,
                                                                              m_impulsegen.coeff[m_impulsegen.central]);
    
    for (int i=m_impulsegen.central+1; i<m_impulsegen.count; i++)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = impulsegrad[1] + %d;" SHNL, i-m_impulsegen.central);
      if (m_impulsegen.cycled == 1)
        m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s.y, 1.0 - step(loc_vv[1], %s.y));" SHNL, scnosc, scnosc );
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = loc_vv[0] + %f * texture(texdata, vec2(impulsegrad[0], (loc_vv[1]*impulsegrad[2] + impulsegrad[3])/float(portions))).r;" SHNL,
                                                                              m_impulsegen.coeff[i]);
    }
    
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[0];" SHNL, varname);
  }
  else if (m_impulsegen.type == impulsedata_t::IR_B_BORDERS || m_impulsegen.type == impulsedata_t::IR_B_BORDERS_FIXEDCOUNT)
  {
    
    const char* scnosc = m_datamapped == DM_OFF? "ab_indimms.y" : "dbounds_noscaled.y";
    
    m_offset += msprintf(&m_to[m_offset],  "vec4 impulsegrad = vec4(%s.x, (%s.y*(%s)), 1.0/(%s), float(%s));" SHNL,
                                                           coordsname,  coordsname,  scnosc,  scnosc,  portionname );
    
    
    m_offset += msprintf(&m_to[m_offset],  "vec3 loc_vv = vec3(impulsegrad[1] - 1, impulsegrad[1] + 1, 0.0);" SHNL );
    if (m_impulsegen.cycled == 1)
    {
      m_offset += msprintf(&m_to[m_offset], "loc_vv[0] = mix(loc_vv[0], %s + loc_vv[0], 1.0 - step(0.0, loc_vv[0]));" SHNL, scnosc);
      m_offset += msprintf(&m_to[m_offset], "loc_vv[1] = mix(loc_vv[1], loc_vv[1] - %s, 1.0 - step(loc_vv[1], %s));" SHNL, scnosc, scnosc);
    }
    
    m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = texture(texdata, vec2(impulsegrad[0], (%s.y + impulsegrad[3])/float(portions))).r;" SHNL, coordsname);
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2(texture(texdata, vec2(impulsegrad[0], (loc_vv[0]*impulsegrad[2] + impulsegrad[3])/float(portions))).r," SHNL
                                                           "texture(texdata, vec2(impulsegrad[0], (loc_vv[1]*impulsegrad[2] + impulsegrad[3])/float(portions))).r);" SHNL);
    
    {
      double halfcent = m_impulsegen.central/2.0;
      if (m_impulsegen.type == impulsedata_t::IR_B_BORDERS)
      {
        m_offset += msprintf(&m_to[m_offset], "float impulsemix = float(1.0 + imrect[1])/int((imrect[3] + 1)/2);");
        m_offset += msprintf(&m_to[m_offset], "impulsemix = step(%F, float(imrect[3]))*(impulsemix - 1.0)/2.0;" SHNL,
                              double(m_impulsegen.count)
                           );
      }
      else
      {
        m_offset += msprintf(&m_to[m_offset], "float impulsemix = step(%F, float(imrect[3]))*(0.5 - 0.25/%F)*(-clamp(1.0 - (imrect[1])/%F, 0.0, 1.0) + clamp(1.0 - float(imrect[3]-imrect[1])/%F, 0.0, 1.0) );" SHNL,
                              double(m_impulsegen.count), halfcent, halfcent, halfcent
                           );
      }
    }
    
    float fc = m_impulsegen.coeff[0];
    if (fc < 0.0f)  fc = 0.0f;  else if (fc > 0.99f) fc = 0.99f;
    m_offset += msprintf(&m_to[m_offset], "loc_vv.xy = vec2("
                                            "-impulsemix*step(impulsemix, 0.0)*(loc_vv[2] - loc_vv[0])/(1.0 - %f*(1.0+impulsemix)),"
                                            " impulsemix*step(0.0, impulsemix)*(loc_vv[2] - loc_vv[1])/(1.0 - %f*(1.0-impulsemix)) "
                                          ");" SHNL, fc, fc);
    m_offset += msprintf(&m_to[m_offset], "loc_vv[2] = loc_vv[2] - loc_vv.x - loc_vv.y;");
    m_offset += msprintf(&m_to[m_offset],  "%s = loc_vv[2];" SHNL, varname);

  }
}
