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
  updateParameter(false);
}

void    OBlocker::setUnlocked(bool unlocked)
{
  m_blockstate = unlocked? 0 : 1;
  updateParameter(false);
}

int OBlocker::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
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

bool OBlocker::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void*, bool* doStop)
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
