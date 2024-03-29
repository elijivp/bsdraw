/// Overlays:
///   Blocker. View: grey screen who disappears after one left mouse click
/// Created By: Elijah Vlasov
#include "bsblocker.h"
#include "../../core/sheigen/bsshgentrace.h"

OBlocker::OBlocker(unsigned int color): m_blockstate(1)
{
  appendUniform(DT_1I, &m_blockstate);
  bsintTocolor3f(color, m_clr);
}

OBlocker::OBlocker(float r, float g, float b): m_blockstate(1)
{
  appendUniform(DT_1I, &m_blockstate);
  m_clr[0] = r; m_clr[1] = g; m_clr[2] = b;
}

void    OBlocker::setLocked(bool locked)
{
  m_blockstate = locked? 1 : 0;
  updateParameter(false, true);
}

void    OBlocker::setUnlocked(bool unlocked)
{
  m_blockstate = unlocked? 0 : 1;
  updateParameter(false, true);
}

int OBlocker::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.param_alias("blockstate");
    ocg.var_inline("result = vec3(%F, %F, %F);", m_clr[0], m_clr[1], m_clr[2] );
//    ocg.push( "result = vec3(0.8, 0.8, 0.8);" );
    ocg.push( "mixwell = 0.9*blockstate;" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

bool OBlocker::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t*, bool* doStop)
{
  if (m_blockstate != 0)
  {
    *doStop = true;
    if (oreact == ORM_LMRELEASE)
    {
      m_blockstate = 0;
      return true;
    }
  }
  return false;
}


/*******************************************/


OBlockerDots::OBlockerDots(unsigned int modX, unsigned int modY, unsigned int dotcolor, float dotmix, unsigned int backcolor, float backmix)
{
  m_mod[0] = modX;  m_mod[1] = modY;
  m_mixdot = dotmix;  m_mixback = backmix;
  bsintTocolor3f(dotcolor, m_clrdot);
  bsintTocolor3f(backcolor, m_clrback);
}

void    OBlockerDots::setLocked(bool locked)
{
  if (this->opaque())
  {
    if (locked)
      this->setOpacity(0.0f);
  }
  else
  {
    if (!locked)
      this->setOpacity(1.0f);
  }
}

void    OBlockerDots::setUnlocked(bool unlocked)
{
  if (this->opaque())
  {
    if (!unlocked)
      this->setOpacity(0.0f);
  }
  else
  {
    if (unlocked)
      this->setOpacity(1.0f);
  }
}

int OBlockerDots::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_const_fixed("clrdot", m_clrdot[0], m_clrdot[1], m_clrdot[2] );
    ocg.var_const_fixed("clrback", m_clrback[0], m_clrback[1], m_clrback[2] );
    ocg.var_const_fixed("modstep", float(m_mod[0]), float(m_mod[1]));
    ocg.var_const_fixed("mixdot", m_mixdot);  ocg.var_const_fixed("mixback", m_mixback);
    ocg.push(" float isdot = 1.0 - step(1.0, mod(float(inormed.x), modstep.x) + mod(float(inormed.y), modstep.y));");
    ocg.push( "result = mix(clrback, clrdot, isdot);" );
    ocg.push( "mixwell = mix(mixback, mixdot, isdot);" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

bool OBlockerDots::overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t*, bool*)
{
//  if (m_blockstate != 0)
//  {
//    *doStop = true;
//    if (oreact == ORM_LMRELEASE)
//    {
//      m_blockstate = 0;
//      return true;
//    }
//  }
  return false;
}

/*******************************************/


OBlockerShirm::OBlockerShirm(float start, int setup, unsigned int colorLine, float mixLine, unsigned int colorBack, float mixBack):
  Ovldraw_ColorForegoing(), OVLCoordsDynamic(CR_RELATIVE, start, start), OVLDimmsOff(),
  m_setup(setup), m_mixline(mixLine), m_mixback(mixBack)
{
  bsintTocolor3f(colorLine, m_clrline);
  bsintTocolor3f(colorBack, m_clrback);
}

int OBlockerShirm::fshOVCoords(int overlay, bool, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.var_const_fixed("clrline", m_clrline[0], m_clrline[1], m_clrline[2] );
    ocg.var_const_fixed("clrback", m_clrback[0], m_clrback[1], m_clrback[2] );
    ocg.var_const_fixed("mixline", m_mixline);  ocg.var_const_fixed("mixback", m_mixback);
    
    ocg.push("float onLine=0, onBack=0;");
    if (m_setup == OBS_HORZ2LEFT)
      ocg.push("onLine = step(0.0, float(inormed.x))*step(float(inormed.x), 0.0);"
               "onBack = step(float(inormed.x), 0.0);"
               );
    else if (m_setup == OBS_HORZ2RIGHT)
      ocg.push("onLine = step(0.0, float(inormed.x))*step(float(inormed.x), 0.0);"
               "onBack = step(1.0, float(inormed.x));"
               );
    else if (m_setup == OBS_VERT2TOP)
      ocg.push("onLine = step(0.0, float(inormed.y))*step(float(inormed.y), 0.0);"
               "onBack = step(float(inormed.y), 0.0);"
               );
    else if (m_setup == OBS_VERT2BOT)
      ocg.push("onLine = step(0.0, float(inormed.y))*step(float(inormed.y), 0.0);"
               "onBack = step(1.0, float(inormed.y));"
               );
    
    ocg.push( "result = mix(result, mix(clrback, clrline, onLine), onLine + onBack);" );
    ocg.push( "mixwell = mix(mixwell, mix(mixback, mixline, onLine), onLine + onBack);" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}
