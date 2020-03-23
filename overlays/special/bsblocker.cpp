#include "bsblocker.h"
#include "../../core/sheigen/bsshgentrace.h"

OBlocker::OBlocker(): m_blockstate(1)
{
  appendUniform(DT_1I, &m_blockstate);
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
    ocg.push( "result = vec3(0.8, 0.8, 0.8);" );
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
