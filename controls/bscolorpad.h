#ifndef BSCOLORPAD_H
#define BSCOLORPAD_H

#include <QWidget>
#include <QLineEdit>


enum COLORPATTERN
{
  CPP_MAT,      CPP_MAT_N_GRAY,
  CPP_RAINBOW,  CPP_RAINBOW_N_GRAY,
  
  CPP_GRAY
};

enum
{
//  CPF_POSITIVE    =0x1,     // No '+/-' sign changing operation
//  CPF_NOZERO      =0x2,     // starts from 1
//  CPF_NOZEROINTERVAL =0x4,  // no special 0..1 interval
//  CPF_INT         =0x8,     // emit valueChanged INT signal version instead float
//  CPF_NOINTERVALS =0x10,    // disable intervals, fixed values only
//  CPF_NOEFFECTS   =0x40,    // off draw decentric effects
  CPF_DEWHITELAST = 0x10,
  CPF_OWNFIELD    =0x20,    // LineEdit with current value in colorpad widget
  CPF_RIGHTFINISH =0x40,    // right button click emits signal
  
  CPF_INTERPOFF   =0x80,
  
  CPF_LEVELS_MINIMIZE =0x100,
  CPF_LEVELS_SUPPRESS =0x200,
  
  CPF_INTERPDEFAULT   =0
};

class IPalette;
class BSColorpadPrivate;
class BSColorpad: public QWidget
{
  Q_OBJECT
public:
  BSColorpad(COLORPATTERN cp, int flags, QWidget* parent=0);
  ~BSColorpad();
public:
  void    setScalingLimitsH(unsigned int scmin, unsigned int scmax=0);
  void    presetValue(unsigned int hexvalue, bool doEmit=true);
  void    switchPalette(int idx);
public:
  unsigned int  value() const;
signals:
  void    valueChanged(unsigned int);
  void    originalFinished();
private slots:
  void    slotloc_updatefield(unsigned int);
  void    slotloc_switchpalettes();
private:
  BSColorpadPrivate* pImpl;
};

class BSColorcallPrivate;
class BSColorcall: public QLineEdit
{
  Q_OBJECT
public:
  BSColorcall(unsigned int value, COLORPATTERN cp, int flags=0, QWidget* parent=0);
  ~BSColorcall();
public:
  void    setPopupOptions(COLORPATTERN cp, int flags);
  void    doSavePopup();
  void    setPopupButton(Qt::MouseButton button, bool doubleClick);
  void    unsetPopupButtons();
signals:
  void    valueChanged(unsigned int);
private slots:
  void    slotloc_editing();
  void    slotloc_updatefield(unsigned int);
  void    slotloc_syncfield(const QString&);
  void    activatePopup();
  friend class BSCustomEditInsquare;
protected:
  void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *) Q_DECL_OVERRIDE;
private:
  BSColorcallPrivate*   pImpl;
};

class BSColorcallInsquare: public QWidget
{
  Q_OBJECT
  BSColorcall* pNumcall;
public:
  BSColorcallInsquare(BSColorcall* pnc, bool rejectDefaultButtons=true, QWidget* parent=0);
};

#endif // BSCOLORPAD_H
