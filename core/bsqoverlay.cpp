/// This file contains QObject'based wrapper for bsdraw Overlays
/// It appends slot mechanic for DrawOverlay subclasses
/// Created By: Elijah Vlasov
#include "bsqoverlay.h"


MQOverlay::MQOverlay(DrawOverlay* ovl, bool owner, QObject* parent): QObject(parent), m_povl(ovl), m_owner(owner)
{
}

MQOverlay::~MQOverlay()
{
  if (m_owner)
    delete m_povl;
}

void MQOverlay::setOpacity(float opacity)
{
  m_povl->setOpacity(opacity);
}

void MQOverlay::setVisible(bool v)
{
  m_povl->setVisible(v);
}

void MQOverlay::show()
{
  setVisible(true);
}

void MQOverlay::hide()
{
  setVisible(false);
}

DrawOverlay* MQOverlay::replace(DrawOverlay* ovl)
{
  DrawOverlay* tmp = m_povl;
  m_povl->eject(ovl, false);
  m_povl = ovl;
  return tmp;
}

void MQOverlay::remove()
{
  m_povl->eject();
  if (m_owner)
    delete m_povl;
  m_povl = nullptr;
}


MQOverlayLined::MQOverlayLined(DrawOverlay* ovl, bool owner, QObject* parent): MQOverlay(ovl, owner, parent)
{
}



/**********************************************************************/



//MQOverlaySwap::MQOverlaySwap(DrawOverlay* ovlPrimary, DrawOverlay* ovlSecondary, bool owner, QObject* parent): 
//  QObject(parent), m_povlPrimary(ovlPrimary), m_povlSecondary(ovlSecondary), m_owner(owner)
//{
//  c_opacity = m_povl->getOpacity();
//}

//MQOverlaySwap::~MQOverlaySwap()
//{
//  if (m_owner)
//    delete m_povl;
//}




//bool OQClicker::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
//{
//  if (oreact == ORM_LMDOUBLE)
//  {
//    *doStop = true;
//    emit doubleClicked();
//    emit doubleClicked(QPoint(((const float*)dataptr)[0], ((const float*)dataptr)[1]));
//  }
//  return false;
//}

bool  OQRemitPress_Release::overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/)
{
  float v = m_horz? ct->fx_rel : ct->fy_rel;
  if (orm == ORM_LMPRESS)
    emit mouseAction(true, v);
  else if (orm == ORM_LMRELEASE)
    emit mouseAction(false, v);
  return false;
}

bool  OQRemitPressMove_Release::overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/)
{
  float v = m_horz? ct->fx_rel : ct->fy_rel;
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
    emit mouseAction(true, v);
  else if (orm == ORM_LMRELEASE)
    emit mouseAction(false, v);
  return false;
}
