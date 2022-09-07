/// Overlays:
///   OSnowflake. View: snowflake lol
/// Created By: Elijah Vlasov
#include "bssnowflake.h"
#include "../../core/sheigen/bsshgentrace.h"

#include <qmath.h>
#include <memory.h>

#if QT_VERSION >= 0x051000
#include <QRandomGenerator>
#endif

OSnowflake::OSnowflake(QImage *image, IMAGECONVERT icvt, unsigned int count, float sizemultiplier): OVLQImage(image, icvt, false), m_count(count), m_sm(sizemultiplier)
{
  const unsigned int v4 = 4;
  float* rnd = new float[m_count*v4];
#if QT_VERSION < 0x051000
  for (unsigned int i=0; i<m_count*v4; i++)
    rnd[i] = qrand()/float(RAND_MAX);
#else
  QRandomGenerator rg;
  for (unsigned int i=0; i<m_count*v4; i++)
    rnd[i] = rg.generateDouble();
#endif
  
  m_randomer.count = m_count;
  m_randomer.data = rnd;
  appendUniform(DT_SAMP4, &m_randomer);
  
  
  m_counter = 0;
  appendUniform(DT_1I, &m_counter);
  memset(m_click, 0, sizeof(m_click));
  appendUniform(DT_4F, m_click);
  appendUniform(DT_TEXTURE, &m_dmti);
}

OSnowflake::~OSnowflake()
{
  delete [](float*)m_randomer.data;
}

void OSnowflake::update()
{
  m_counter++;
  updateParameter(false, true);
}

int OSnowflake::fshOVCoords(int overlay, bool switchedab, char *to) const
{ 
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_RANDOM);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("spritescount", (int)m_count);
    ocg.var_fixed("base_size", (int)(m_dmti.w*m_sm), (int)(m_dmti.h*m_sm));
//    ocg.param_pass();
    int texrandomer = ocg.param_push();
    ocg.param_alias("counter");
    ocg.param_alias("moveto");
    ocg.push("vec4 randomer;");
    
    ocg.push("for (int i=0; i<spritescount; i++)"
             "{");
    { 
      /// randomer[0] = size => velocity => ampliture
      /// randomer[1] & randomer[2] = just start x,y position
      ocg.push( "randomer = texture(");  ocg.param_mem(texrandomer); ocg.push(", vec2(float(i)/float(spritescount-1), 0.0)).rgba;");
      ocg.push( "ivec2 rect_size = ivec2(base_size*(mix(0.3 + randomer[0]*0.4, 1.0, step(0.99, randomer[0]))));"
                "_mvar.xy = vec2(cos(counter*(0.3 + 1.0/(0.3+randomer[0]))/(2.0*3.1415))*5*((randomer[0] - 1.0)*0.5 + 0.6)*(2.0*mod(i, 2) - 1.0), counter*(randomer[0]*2.0 + 1.2));"
                "_mvar.x = randomer[1]*ov_ibounds.x + _mvar.x;"
                "_mvar.y = mod((1.0-randomer[2])*ov_ibounds.y - _mvar.y, ov_ibounds.y + rect_size.y);"
                
                "_mvar.x = _mvar.x + (1.0-step(moveto[3], 0.0))*(counter - moveto[2])*(distance(vec2(moveto.x, 1.0 - moveto.y), vec2(_mvar.xy/ov_ibounds)))*(1.0 - 2.0*step(moveto.x, _mvar.x/ov_ibounds.x))*(randomer[0]*1.5+1.0);"
                
                "ivec2 inormed = icoords - ivec2(_mvar.x, ov_ibounds.y - _mvar.y);"
                "_fvar = step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1.0-step(rect_size.x, float(inormed.x)))*(1.0-step(rect_size.y, float(inormed.y)));"
                "vec2  tcoords = inormed/vec2(rect_size.x-1, rect_size.y-1);");
      
      ocg.push("vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, 1.0 - tcoords.y));");
      ocg.push("result = mix(result, pixel.rgb, _fvar*pixel.a);"
               "mixwell = max(mixwell, _fvar*pixel.a);");
    }
    ocg.push("}");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

bool OSnowflake::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool*)
{
  if (oreact == ORM_LMPRESS)
  {
    m_click[0] = ct->fx_ovl;
    m_click[1] = ct->fy_ovl;
    m_click[2] = float(m_counter);
    m_click[3] = 1.0f;
    return true;
  }
  if (oreact == ORM_LMRELEASE)
  {
    m_click[3] = 0.0f;
    return true;
  }
  return false;
}
