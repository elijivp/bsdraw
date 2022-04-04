#ifndef BSQOVERLAY_H
#define BSQOVERLAY_H

/// This file contains QObject'based wrapper for bsdraw Overlays
/// It appends slot mechanic for DrawOverlay subclasses
/// Created By: Elijah Vlasov

#include <QObject>
#include <QPoint>
#include "bsoverlay.h"

class MQOverlay: public QObject
{
  Q_OBJECT
protected:  
  DrawOverlay*     m_povl;
  bool          m_owner;
public:
  MQOverlay(DrawOverlay* ovl, bool owner=false, QObject* parent=nullptr);
  ~MQOverlay();
  DrawOverlay* overlay() { return m_povl; }
  const DrawOverlay* overlay() const { return m_povl; }
  DrawOverlay* operator*() { return m_povl; }
  const DrawOverlay* operator*() const { return m_povl; }
public slots:
  void      setOpacity(float opacity);
  void      setThickness(float thickness){  m_povl->setThickness(thickness); }
  void      setSlice(float slice){  m_povl->setSlice(slice); }
  
  void      setOTS(const ovlbasics_t& ob){ m_povl->setOTS(ob); }
  void      setOTS(float opacity, float thickness, float slice){ m_povl->setOTS(opacity, thickness, slice); }
  
  void      setVisible(bool v);
  
  void      show();
  void      hide();
  
  DrawOverlay* replace(DrawOverlay* ovl);
  void      remove();
public:
  float     getOpacity() const { return m_povl->getOpacity(); }
  bool      opaque() const {  return m_povl->opaque(); }
  float     getThickness() const { return m_povl->getThickness(); }
  float     getSlice() const { return m_povl->getSlice(); }
  bool      isVisible() const { return m_povl->isVisible(); }
};

class MQOverlayLined: public MQOverlay
{
  Q_OBJECT
public:
  MQOverlayLined(DrawOverlay* ovl, bool owner=false, QObject* parent=nullptr);
//public slots:
//  void      setLineColor(unsigned int clr);
};


//class MQOverlaySwap: public QObject
//{
//  Q_OBJECT
//protected:  
//  DrawOverlay*  m_povlPrimary, *m_povlSecondary;
//  bool          m_owner;
//public:
//  MQOverlaySwap(DrawOverlay* ovlPrimary, DrawOverlay* ovlSecondary, bool owner=false, QObject* parent=nullptr);
//  ~MQOverlaySwap();
//  DrawOverlay* overlayPrimary() const { return m_povlPrimary; }
//  DrawOverlay* overlaySecondary() const { return m_povlSecondary; }
//  DrawOverlay* overlay(bool primary) const { return primary? m_povlPrimary : m_povlSecondary; }
//public slots:
//  void show(bool);
//  void hide(bool);
//};

/****/

class OQRemitPress_Release: public QObject, public IOverlayReactor
{
  Q_OBJECT
  bool    m_horz;
public:
  OQRemitPress_Release(bool isHorz): m_horz(isHorz){}
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* doStop);
signals:
  void  mouseAction(bool activePhase, float center);
};

class OQRemitPressMove_Release: public QObject, public IOverlayReactor
{
  Q_OBJECT
  bool    m_horz;
public:
  OQRemitPressMove_Release(bool isHorz): m_horz(isHorz){}
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* doStop);
signals:
  void  mouseAction(bool activePhase, float center);
};

/****/

class OQRemitCellPress: public QObject, public IOverlayReactor    // coordstriumv_ex_t  !!
{
  Q_OBJECT
public:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* doStop);
signals:
  void  mouseAction(int row, int column, int count_rows, int count_columns);
};



//class OQClicker: public QObject, public DrawOverlayProactive
//{
//  Q_OBJECT
//public:
//  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE, const coordstriumv_t* ct, bool* /*doStop*/);
//signals:
//  void  doubleClicked();
//  void  doubleClicked(QPoint);
//};

#endif // BSQOVERLAY_H
