/// This file contains QObject'based wrapper for bsdraw Overlays
/// It appends slot mechanic for DrawOverlay subclasses
/// Created By: Elijah Vlasov
#include "bsqoverlay.h"


void MQOverlay::_vischecker()
{
  float unfortunchanged_opacity = m_povl->getOpacity();
  if (unfortunchanged_opacity != 1.0f)
    c_opacity = unfortunchanged_opacity;
  else
  {
    if (c_opacity == 1.0f)
      c_opacity = 0.0f;
    m_povl->setOpacity(c_opacity);
  }
}

MQOverlay::MQOverlay(DrawOverlay* ovl, bool owner, QObject* parent): QObject(parent), m_povl(ovl), m_owner(owner)
{
  c_opacity = m_povl->getOpacity();
}

MQOverlay::~MQOverlay()
{
  if (m_owner)
    delete m_povl;
}

void MQOverlay::setVisible(bool v)
{
  if (v)
    _vischecker();
  else
    m_povl->setOpacity(1.0f);
}

void MQOverlay::show()
{
  _vischecker();
}

void MQOverlay::hide()
{
  m_povl->setOpacity(1.0f);
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

//void MQOverlayLined::setLineColor(unsigned int clr)
//{
//}





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




//bool OQClicker::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
//{
//  if (oreact == ORM_LMDOUBLE)
//  {
//    *doStop = true;
//    emit doubleClicked();
//    emit doubleClicked(QPoint(((const float*)dataptr)[0], ((const float*)dataptr)[1]));
//  }
//  return false;
//}
