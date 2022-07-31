/// Overlays:   simple sprites, drawed each on own place
///   OSprites: with QImage as source
/// Created By: Elijah Vlasov
#include "bssprites.h"
#include "../core/sheigen/bsshgentrace.h"
#include "../core/sheigen/bsshgencolor.h"

#include <qmath.h>
#include <memory.h>

OSprites::OSprites(QImage *image, IMAGECONVERT icvt, float sizemultiplier, unsigned int count, COLOR_SPRITE cr, CENTER_BY cb): 
  Ovldraw(true),
  OVLQImage(image, icvt, false), 
  m_sm(sizemultiplier), m_countmax(count), m_cb(cb), m_cr(cr), m_countactive(0)
{
  const unsigned int v4 = 4;
  kpdc_t*   kpdc = new kpdc_t[m_countmax*v4];
  
  m_kpdc.count = m_countmax;
  m_kpdc.data = (float*)kpdc;
  appendUniform(DT_SAMP4, &m_kpdc);
  appendUniform(DT_1I, &m_countactive);
  appendUniform(DT_TEXTURE, &m_dmti);
}

OSprites::OSprites(QImage* image, OVLQImage::IMAGECONVERT icvt, float sizemultiplier, unsigned int count, const IPalette* ipal, bool discrete, CENTER_BY cb):
  Ovldraw(true),
  OVLQImage(image, icvt, false), 
  m_sm(sizemultiplier), m_countmax(count), m_cb(cb), m_cr(-1), m_countactive(0)
{
  const unsigned int v4 = 4;
  kpdc_t*   kpdc = new kpdc_t[m_countmax*v4];
  
  m_kpdc.count = m_countmax;
  m_kpdc.data = (float*)kpdc;
  appendUniform(DT_SAMP4, &m_kpdc);
  appendUniform(DT_1I, &m_countactive);
  appendUniform(DT_TEXTURE, &m_dmti);
  
  m_dm_palette.ppal = ipal;
  m_dm_palette.discrete = discrete;
  appendUniform(DT_SAMP4, &m_dm_palette);
}

OSprites::~OSprites()
{
  delete [](kpdc_t*)m_kpdc.data;
}

int OSprites::fshOVCoords(int overlay, bool switchedab, char *to) const
{ 
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_RANDOM);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("countmax", (int)m_countmax);
    ocg.var_fixed("base_size", (int)(m_dmti.w*m_sm), (int)(m_dmti.h*m_sm));
    int texkpdc = ocg.param_push();
    ocg.param_alias("countactive");
    ocg.push("vec4 kpdc;");
    ocg.push("for (int i=0; i<countactive; i++)"
             "{");
    { 
      ocg.push( "kpdc = texture(");  ocg.param_mem(texkpdc); ocg.push(", vec2(float(i)/float(countmax-1), 0.0)).rgba;");
      ocg.push( "ivec2 rect_size = ivec2(base_size*vec2(kpdc[2], kpdc[2]));" );
      
      {   /// zoom
        if (m_cb == CB_CENTER)
          ocg.push("ivec2 inormed = icoords - ivec2(kpdc[0]*ov_ibounds.x, kpdc[1]*ov_ibounds.y) + rect_size/2;");
        else
          ocg.push("ivec2 inormed = icoords - ivec2(kpdc[0]*ov_ibounds.x, kpdc[1]*ov_ibounds.y);");
      }
      
      ocg.push( "_fvar = step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1.0-step(rect_size.x, float(inormed.x)))*(1.0-step(rect_size.y, float(inormed.y)));"
                "vec2  tcoords = inormed/vec2(rect_size.x-1, rect_size.y-1);");
      ocg.push("vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, tcoords.y));");
      
      
      {   /// color
        if (m_cr == -1)
        {
          ocg.push("result = mix(result, vec3(kpdc[3], 0.0, 0.0), _fvar*pixel.a);"
                   "mixwell = max(mixwell, _fvar*pixel.a);");
        }
        else
        {
          if (m_cr == CR_OPACITY)
            ocg.push("_fvar = _fvar * (1.0 - kpdc[3]);");
          else if (m_cr == CR_DISABLE)
            ocg.push("pixel.rgb = mix(pixel.rgb, vec3(0.2,0.2,0.2), kpdc[3]);");
          
          ocg.push("result = mix(result, pixel.rgb, _fvar*pixel.a);"
                   "mixwell = max(mixwell, _fvar*pixel.a);");
        }
      }
    }
    ocg.push("}");
  }
  ocg.goto_func_end(false);
//  Q_ASSERT(false);
  return ocg.written();
}

int OSprites::fshColor(int overlay, char* to) const
{
  FshOVColorConstructor ocg(overlay, to, 3);
  ocg.goto_func_begin();
  ocg.push("result = in_variant.rgb;"
           "mixwell = in_variant.a;");
  ocg.goto_func_end();
//  if (m_cr == -1)
//  {
//    ocg.goto_func_begin(FshOVColorConstructor::CGV_TEXTURED);
////    ocg.push("mixwell = trace[0];");
//  }
//  else
//    ocg.goto_func_begin(FshOVColorConstructor::CGV_COLORED);
//  ocg.goto_func_end();
  return ocg.written();
}

bool OSprites::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
//  if (oreact == ORM_LMPRESS)
//  {
//    m_click[0] = ((const float*)dataptr)[0];
//    m_click[1] = ((const float*)dataptr)[1];
//    m_click[2] = float(m_counter);
//    m_click[3] = 1.0f;
//    return true;
//  }
//  if (oreact == ORM_LMRELEASE)
//  {
//    m_click[3] = 0.0f;
//    return true;
//  }
  return false;
}

void OSprites::setPalette(const IPalette* ipal, bool discrete)
{
  m_dm_palette.ppal = ipal;
  m_dm_palette.discrete = discrete;
  _Ovldraw::updateParameter(false, true); 
}

void  OSprites::setKPDC(unsigned int idx, float x, float y){ ((kpdc_t*)m_kpdc.data)[idx].x = x; ((kpdc_t*)m_kpdc.data)[idx].y = y; }
void  OSprites::setKPDCZoom(unsigned int idx, float zoom){ ((kpdc_t*)m_kpdc.data)[idx].zoom = zoom; }
void  OSprites::setKPDCColor(unsigned int idx, float color){ ((kpdc_t*)m_kpdc.data)[idx].color = color; }
void OSprites::setKPDCZoomColor(unsigned int idx, float zoom, float color){ ((kpdc_t*)m_kpdc.data)[idx].zoom = zoom; ((kpdc_t*)m_kpdc.data)[idx].color = color; }
void  OSprites::setKPDC(unsigned int idx, float x, float y, float color){ ((kpdc_t*)m_kpdc.data)[idx].x = x; ((kpdc_t*)m_kpdc.data)[idx].y = y; ((kpdc_t*)m_kpdc.data)[idx].color = color; }
void  OSprites::setKPDC(unsigned int idx, float x, float y, float zoom, float color)
{
  ((kpdc_t*)m_kpdc.data)[idx].x = x;
  ((kpdc_t*)m_kpdc.data)[idx].y = y;
  ((kpdc_t*)m_kpdc.data)[idx].zoom = zoom;
  ((kpdc_t*)m_kpdc.data)[idx].color = color;
}


void OSprites::updateKPDC()
{
  _Ovldraw::updateParameter(false, true);
}

const kpdc_t& OSprites::at(int idx) const
{
  return ((const kpdc_t*)m_kpdc.data)[idx];
}

void OSprites::setActiveCount(unsigned int count)
{
  if (count <= m_countmax)
  {
    if (count != m_countactive)
    {
      m_countactive = count;
//      _Ovldraw::updateParameter(false, true);
    }
  }
}
