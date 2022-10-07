#ifndef BSQLAYOUT_H
#define BSQLAYOUT_H

#include <QBoxLayout>
#include <QStack>
#include <QString>

/// See example below
enum    _BS_LAYTYPE {   _BS_BOXED_LAYOUT, _BS_NONBOXED_LAYOUT, _BS_NOLAYOUT, _BS_EMPTY };

#define _BS_LOCAL_VARIABLES(bs_enum) \
  _BS_LAYTYPE _bs_boxed   = bs_enum; \
  QScrollArea*            _bs_topScroll(NULL);

#define BS_INIT_FOR(boxLayout) \
  QWidget*                _bs_pWidget = NULL; \
  QStack<QWidget*>        _bs_wStack; \
  QStack<QBoxLayout*>     _bs_lStack; \
  QBoxLayout*             _bs_active = boxLayout; \
  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)

#define BS_INIT_NOTCUTED(boxlayouttype) \
  QWidget*                _bs_pWidget = NULL; \
  QStack<QWidget*>        _bs_wStack; \
  QStack<QBoxLayout*>     _bs_lStack; \
  QBoxLayout*             _bs_active = new boxlayouttype(); \
  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)

#define BS_INIT(boxlayouttype) \
  QWidget*                _bs_pWidget = NULL; \
  QStack<QWidget*>        _bs_wStack; \
  QStack<QBoxLayout*>     _bs_lStack; \
  QBoxLayout*             _bs_active = new boxlayouttype(); _bs_active->setContentsMargins(0,0,0,0); \
  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)

#define BS_INIT_IF(condition, bltypeTrue, bltypeFalse) \
  QWidget*                _bs_pWidget = NULL; \
  QStack<QWidget*>        _bs_wStack; \
  QStack<QBoxLayout*>     _bs_lStack; \
  QBoxLayout*             _bs_active = condition? (QBoxLayout*)new bltypeTrue() : (QBoxLayout*)new bltypeFalse(); _bs_active->setContentsMargins(0,0,0,0); \
  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)

#define BS_INIT_DERIVED(boxlayouttype) \
  QWidget*                _bs_pWidget = this; \
  QStack<QWidget*>        _bs_wStack; \
  QStack<QBoxLayout*>     _bs_lStack; \
  QBoxLayout*             _bs_active = new boxlayouttype(); _bs_active->setContentsMargins(0,0,0,0); \
  this->setLayout(_bs_active); \
  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)



#define _BS_LOCAL_VARIABLES_CLEAR _bs_boxed = _BS_BOXED_LAYOUT; _bs_topScroll = NULL;

#define BS_REINIT_FOR(boxLayout) \
  _bs_pWidget = NULL; _bs_wStack.clear(); _bs_lStack.clear(); _bs_active = boxLayout; _BS_LOCAL_VARIABLES_CLEAR

#define BS_REINIT_NOTCUTED(boxlayouttype) \
  _bs_pWidget = NULL; _bs_wStack.clear(); _bs_lStack.clear(); _bs_active = new boxlayouttype(); _BS_LOCAL_VARIABLES_CLEAR

#define BS_REINIT(boxlayouttype) \
  _bs_pWidget = NULL; _bs_wStack.clear(); _bs_lStack.clear(); _bs_active = new boxlayouttype(); _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES_CLEAR

#define BS_REINIT_DERIVED(boxlayouttype) \
  _bs_pWidget = this; _bs_wStack.clear(); _bs_lStack.clear(); _bs_active = new boxlayouttype(); _bs_active->setContentsMargins(0,0,0,0); this->setLayout(_bs_active); _BS_LOCAL_VARIABLES_CLEAR


#define BSWIDGET            _bs_pWidget
#define BSLAYOUT            _bs_active

#define BS_RESIZE_CRAZY     _bs_active->setSizeConstraint(QLayout::SetNoConstraint);
#define BS_RESIZE_OFF       _bs_active->setSizeConstraint(QLayout::SetFixedSize);

#define _BS_POP(STRETCH, ALIGN)     if (_bs_pWidget == nullptr) (QBoxLayout*&)_bs_active = _bs_lStack.pop(); \
                                    else if (_bs_boxed == _BS_BOXED_LAYOUT) ((QBoxLayout*)_bs_active)->addWidget(_bs_pWidget, STRETCH, ALIGN); \
                                    else if (_bs_boxed == _BS_NONBOXED_LAYOUT || _bs_boxed == _BS_NOLAYOUT) _bs_active->addWidget(_bs_pWidget);  \
                                    _bs_pWidget = _bs_wStack.pop();

#define _BS_POP_STASH               if (_bs_pWidget == nullptr) _bs_lStack.pop(); \
                                    _bs_pWidget = _bs_wStack.pop(); //  Q_ASSERT(_bs_pWidget == nullptr);

#define BS_STOP_STASH               if (_bs_pWidget == nullptr) {  ((QBoxLayout*)_bs_lStack.top())->addLayout((QBoxLayout*)_bs_active); } \
                                    if (_bs_boxed == _BS_BOXED_LAYOUT || _bs_boxed == _BS_NONBOXED_LAYOUT) ((QWidget*)_bs_pWidget)->setLayout((QLayout*)_bs_active); \
                                    if (_bs_topScroll){  _bs_topScroll->setWidget(_bs_pWidget); _bs_pWidget = _bs_topScroll; } \
                                    }  _BS_POP_STASH

#define BS_STOP_EX(STRETCH)         if (_bs_pWidget == nullptr) {  ((QBoxLayout*)_bs_lStack.top())->addLayout((QBoxLayout*)_bs_active, STRETCH); } \
                                    else if (_bs_boxed == _BS_BOXED_LAYOUT || _bs_boxed == _BS_NONBOXED_LAYOUT) ((QWidget*)_bs_pWidget)->setLayout((QLayout*)_bs_active); \
                                    if (_bs_topScroll){ _bs_topScroll->setWidget(_bs_pWidget); _bs_pWidget = _bs_topScroll; } \
                                    }  _BS_POP(STRETCH, Qt::AlignmentFlag(0))

#define BS_STOP_EX_ALIGNED(SH, AN)  if (_bs_pWidget == nullptr) {  ((QBoxLayout*)_bs_lStack.top())->addLayout((QBoxLayout*)_bs_active, SH); } \
                                    else if (_bs_boxed == _BS_BOXED_LAYOUT || _bs_boxed == _BS_NONBOXED_LAYOUT) ((QWidget*)_bs_pWidget)->setLayout((QLayout*)_bs_active); \
                                    if (_bs_topScroll){ _bs_topScroll->setWidget(_bs_pWidget); _bs_pWidget = _bs_topScroll; } \
                                    }  _BS_POP(SH, AN)

#define BS_STOP   BS_STOP_EX(0)




#define BSMARGINS(L,T,R,B)  _bs_active->setContentsMargins(L,T,R,B);

#define BS_STRETCH                          _bs_active->addStretch(1);
#define BS_WSTRETCH(W)                      _bs_active->addStretch(W);
#define BS_SPACING(count)                   _bs_active->addSpacing(count);


#define BSADD(...)                          _bs_active->addWidget(__VA_ARGS__);



#define BS_START_LAYOUT(boxlayouttype)  _bs_wStack.push(_bs_pWidget); if (_bs_pWidget == nullptr || _bs_boxed == _BS_BOXED_LAYOUT){ _bs_pWidget = nullptr; _bs_lStack.push((QBoxLayout*)_bs_active); } else _bs_pWidget = new QWidget; {   QBoxLayout* _bs_active = new boxlayouttype; _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_LAYOUT_HMAX_VMIN(boxlayouttype)   BS_START_LAYOUT(boxlayouttype)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_HMIN_VMIN(boxlayouttype)   BS_START_LAYOUT(boxlayouttype)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_HMIN_VMAX(boxlayouttype)   BS_START_LAYOUT(boxlayouttype)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_LAYOUT_HMAX_VMAX(boxlayouttype)   BS_START_LAYOUT(boxlayouttype)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_LAYOUT_IF(condition, bltypeTrue, bltypeFalse)  _bs_wStack.push(_bs_pWidget);  if (_bs_pWidget == nullptr || _bs_boxed == _BS_BOXED_LAYOUT){ _bs_pWidget = nullptr; _bs_lStack.push((QBoxLayout*)_bs_active); } else _bs_pWidget = new QWidget; { QBoxLayout*  _bs_active = condition? (QBoxLayout*)new bltypeTrue() : (QBoxLayout*)new bltypeFalse(); _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_LAYOUT_IF_HMAX_VMIN(condition, bltypeTrue, bltypeFalse)      BS_START_LAYOUT_IF(condition, bltypeTrue, bltypeFalse)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_IF_HMIN_VMIN(condition, bltypeTrue, bltypeFalse)      BS_START_LAYOUT_IF(condition, bltypeTrue, bltypeFalse)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_IF_HMIN_VMAX(condition, bltypeTrue, bltypeFalse)      BS_START_LAYOUT_IF(condition, bltypeTrue, bltypeFalse)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_LAYOUT_IF_HMAX_VMAX(condition, bltypeTrue, bltypeFalse)      BS_START_LAYOUT_IF(condition, bltypeTrue, bltypeFalse)  if (_bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_INSERT_LAYOUT_IF(condition, boxlayouttype)  _bs_wStack.push(_bs_pWidget);  if (condition){ if (_bs_pWidget == nullptr || _bs_boxed == _BS_BOXED_LAYOUT){ _bs_pWidget = nullptr; _bs_lStack.push((QBoxLayout*)_bs_active); } else _bs_pWidget = new QWidget; } else _bs_boxed = _BS_EMPTY; { QBoxLayout* _bs_actold = _bs_active; QBoxLayout* _bs_active = _bs_actold; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT) if (condition){  _bs_active = new boxlayouttype(); _bs_active->setContentsMargins(0,0,0,0); }
#define BS_INSERT_LAYOUT_IF_HMAX_VMIN(condition, boxlayouttype)      BS_INSERT_LAYOUT_IF(condition, boxlayouttype)  if (condition && _bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_INSERT_LAYOUT_IF_HMIN_VMIN(condition, boxlayouttype)      BS_INSERT_LAYOUT_IF(condition, boxlayouttype)  if (condition && _bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_INSERT_LAYOUT_IF_HMIN_VMAX(condition, boxlayouttype)      BS_INSERT_LAYOUT_IF(condition, boxlayouttype)  if (condition && _bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_INSERT_LAYOUT_IF_HMAX_VMAX(condition, boxlayouttype)      BS_INSERT_LAYOUT_IF(condition, boxlayouttype)  if (condition && _bs_pWidget) _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);


#define BS_START_WIDGET(boxlayouttype)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; { QBoxLayout*   _bs_active = new boxlayouttype; _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_WIDGET_HMAX_VMIN(boxlayouttype)   BS_START_WIDGET(boxlayouttype)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_WIDGET_HMIN_VMIN(boxlayouttype)   BS_START_WIDGET(boxlayouttype)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_WIDGET_HMIN_VMAX(boxlayouttype)   BS_START_WIDGET(boxlayouttype)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_WIDGET_HMAX_VMAX(boxlayouttype)   BS_START_WIDGET(boxlayouttype)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_WIDGET_IF(condition, bltypeTrue, bltypeFalse)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; { QBoxLayout*  _bs_active = condition? (QBoxLayout*)new bltypeTrue() : (QBoxLayout*)new bltypeFalse(); _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_WIDGET_IF_HMAX_VMIN(condition, bltypeTrue, bltypeFalse)      BS_START_WIDGET_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_WIDGET_IF_HMIN_VMIN(condition, bltypeTrue, bltypeFalse)      BS_START_WIDGET_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_WIDGET_IF_HMIN_VMAX(condition, bltypeTrue, bltypeFalse)      BS_START_WIDGET_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_WIDGET_IF_HMAX_VMAX(condition, bltypeTrue, bltypeFalse)      BS_START_WIDGET_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_WIDGET_H_TAU(ssid)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; if (ssid>0) _bs_pWidget->setProperty("T", ssid); { QBoxLayout*   _bs_active = new QHBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_WIDGET_H_TAU_HMAX_VMIN(ssid)  BS_START_WIDGET_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_WIDGET_H_TAU_HMIN_VMIN(ssid)  BS_START_WIDGET_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_WIDGET_H_TAU_HMIN_VMAX(ssid)  BS_START_WIDGET_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_WIDGET_H_TAU_HMAX_VMAX(ssid)  BS_START_WIDGET_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_WIDGET_V_TAU(ssid)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; if (ssid>0) _bs_pWidget->setProperty("T", ssid); { QBoxLayout*   _bs_active = new QVBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_WIDGET_V_TAU_HMAX_VMIN(ssid)  BS_START_WIDGET_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_WIDGET_V_TAU_HMIN_VMIN(ssid)  BS_START_WIDGET_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_WIDGET_V_TAU_HMIN_VMAX(ssid)  BS_START_WIDGET_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_WIDGET_V_TAU_HMAX_VMAX(ssid)  BS_START_WIDGET_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);


#define BS_FRAME_NONE      QFrame::NoFrame
#define BS_FRAME_BOX       QFrame::Box | QFrame::Plain
#define BS_FRAME_STYLED    QFrame::StyledPanel | QFrame::Plain
#define BS_FRAME_PANEL     QFrame::Panel | QFrame::Raised
#define BS_FRAME_SUNKEN    QFrame::Panel | QFrame::Sunken

#define BS_START_FRAME_H(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { QBoxLayout*   _bs_active = new QHBoxLayout;    ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle); ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);     _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_H_HMAX_VMIN(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_HMIN_VMIN(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_HMIN_VMAX(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_H_HMAX_VMAX(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_V(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { QBoxLayout*   _bs_active = new QVBoxLayout;    ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle); ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);     _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_V_HMAX_VMIN(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_HMIN_VMIN(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_HMIN_VMAX(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_V_HMAX_VMAX(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_IF(condition, bltypeTrue, bltypeFalse, fstyle, fwidth)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame; { QBoxLayout*  _bs_active = condition? (QBoxLayout*)new bltypeTrue() : (QBoxLayout*)new bltypeFalse(); ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle); ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);  _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_IF_HMAX_VMIN(condition, bltypeTrue, bltypeFalse, fstyle, fwidth)      BS_START_FRAME_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_IF_HMIN_VMIN(condition, bltypeTrue, bltypeFalse, fstyle, fwidth)      BS_START_FRAME_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_IF_HMIN_VMAX(condition, bltypeTrue, bltypeFalse, fstyle, fwidth)      BS_START_FRAME_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_IF_HMAX_VMAX(condition, bltypeTrue, bltypeFalse, fstyle, fwidth)      BS_START_FRAME_IF(condition, bltypeTrue, bltypeFalse)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_E(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle);  ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);  QBoxLayout* _bs_active = NULL;    _BS_LOCAL_VARIABLES(_BS_NOLAYOUT)
#define BS_START_FRAME_CONTINUE(fstyle, fwidth, layout)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle);  ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);  QBoxLayout* _bs_active = layout;    _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)

#define BS_START_FRAME_H_TAU(ssid)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame; if (ssid>0) _bs_pWidget->setProperty("T", ssid); { QBoxLayout*   _bs_active = new QHBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_H_TAU_HMAX_VMIN(ssid)  BS_START_FRAME_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_TAU_HMIN_VMIN(ssid)  BS_START_FRAME_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_TAU_HMIN_VMAX(ssid)  BS_START_FRAME_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_H_TAU_HMAX_VMAX(ssid)  BS_START_FRAME_H_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_V_TAU(ssid)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame; if (ssid>0) _bs_pWidget->setProperty("T", ssid); { QBoxLayout*   _bs_active = new QVBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_V_TAU_HMAX_VMIN(ssid)  BS_START_FRAME_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_TAU_HMIN_VMIN(ssid)  BS_START_FRAME_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_TAU_HMIN_VMAX(ssid)  BS_START_FRAME_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_V_TAU_HMAX_VMAX(ssid)  BS_START_FRAME_V_TAU(ssid)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//////////////////////


#define BS_START_STACK                     _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; {   QStackedLayout*  _bs_active = new QStackedLayout(); _bs_active->setContentsMargins(0,0,0,0); _BS_LOCAL_VARIABLES(_BS_NONBOXED_LAYOUT)
#define BS_START_STACK_HMAX_VMIN            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_STACK_HMIN_VMIN            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_STACK_HMIN_VMAX            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_STACK_HMAX_VMAX            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
#define BS_STACK_SWITCH(idx)  ((QStackedLayout*)_bs_active)->setCurrentIndex(idx);
#define BS_STACK_CONNECT_TABBAR(pTabBar)           QObject::connect(pTabBar, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
#define BS_STACK_CONNECT_MAPPER(pMapper)           QObject::connect(pMapper, SIGNAL(mapped(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
#define BS_STACK_CONNECT_BGROUP(pBgroup)           QObject::connect(pBgroup, SIGNAL(buttonClicked(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
#define BS_STACK_CONNECT_COMBOBOX(pCombo)           QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));

#define BS_START_GROUP(title, layouttype)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QGroupBox(title);  { layouttype*   _bs_active = new layouttype();   _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_GROUP_HMAX_VMIN(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_GROUP_HMIN_VMIN(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_GROUP_HMIN_VMAX(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_GROUP_HMAX_VMAX(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_SPLITTER_H                 _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QSplitter;  { QSplitter*   _bs_active = (QSplitter*)_bs_pWidget; _bs_active->setContentsMargins(0,0,0,0); _bs_active->setOrientation(Qt::Horizontal); _bs_active->setChildrenCollapsible(false);     _BS_LOCAL_VARIABLES(_BS_NOLAYOUT)
#define BS_START_SPLITTER_H_HMAX_VMIN       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_H_HMIN_VMIN       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_H_HMIN_VMAX       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SPLITTER_H_HMAX_VMAX       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_SPLITTER_V                 _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QSplitter;  { QSplitter*   _bs_active = (QSplitter*)_bs_pWidget; _bs_active->setContentsMargins(0,0,0,0); _bs_active->setOrientation(Qt::Vertical); _bs_active->setChildrenCollapsible(false);     _BS_LOCAL_VARIABLES(_BS_NOLAYOUT)
#define BS_START_SPLITTER_V_HMAX_VMIN       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_V_HMIN_VMIN       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_V_HMIN_VMAX       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SPLITTER_V_HMAX_VMAX       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_SPLITTER_STRETCH2(stretchA,stretchB)                    _bs_active->setStretchFactor(0, stretchA);  _bs_active->setStretchFactor(1, stretchB);
#define BS_SPLITTER_STRETCH3(stretchA,stretchB,stretchC)           _bs_active->setStretchFactor(0, stretchA);  _bs_active->setStretchFactor(1, stretchB);  _bs_active->setStretchFactor(2, stretchC);
#define BS_SPLITTER_STRETCH4(stretchA,stretchB,stretchC,stretchD)  _bs_active->setStretchFactor(0, stretchA);  _bs_active->setStretchFactor(1, stretchB);  _bs_active->setStretchFactor(2, stretchC);  _bs_active->setStretchFactor(3, stretchD);

//////////////////////


#define BS_START_PAGE_H  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; _bs_boxed = _BS_EMPTY; { QBoxLayout*   _bs_active = new QHBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_PAGE_H_HMAX_VMIN   BS_START_PAGE_H   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_PAGE_H_HMIN_VMIN   BS_START_PAGE_H   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_PAGE_H_HMIN_VMAX   BS_START_PAGE_H   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_PAGE_H_HMAX_VMAX   BS_START_PAGE_H   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_PAGE_V  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; _bs_boxed = _BS_EMPTY; { QBoxLayout*   _bs_active = new QVBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_PAGE_V_HMAX_VMIN   BS_START_PAGE_V   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_PAGE_V_HMIN_VMIN   BS_START_PAGE_V   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_PAGE_V_HMIN_VMAX   BS_START_PAGE_V   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_PAGE_V_HMAX_VMAX   BS_START_PAGE_V   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_DERIVED_H(ClassName)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new ClassName; { QBoxLayout*   _bs_active = new QHBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_DERIVED_H_HMAX_VMIN(ClassName)   BS_START_DERIVED_H(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_DERIVED_H_HMIN_VMIN(ClassName)   BS_START_DERIVED_H(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_DERIVED_H_HMIN_VMAX(ClassName)   BS_START_DERIVED_H(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_DERIVED_H_HMAX_VMAX(ClassName)   BS_START_DERIVED_H(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_DERIVED_V(ClassName)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new ClassName; { QBoxLayout*   _bs_active = new QVBoxLayout; _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT)
#define BS_START_DERIVED_V_HMAX_VMIN(ClassName)   BS_START_DERIVED_V(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_DERIVED_V_HMIN_VMIN(ClassName)   BS_START_DERIVED_V(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_DERIVED_V_HMIN_VMAX(ClassName)   BS_START_DERIVED_V(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_DERIVED_V_HMAX_VMAX(ClassName)   BS_START_DERIVED_V(ClassName)   _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);


#define BS_START_SCROLL_V                   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget;  { \
  QScrollArea*    scrollArea = new QScrollArea;   \
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  \
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); \
  scrollArea->setWidgetResizable(true); \
  QBoxLayout* _bs_active = new QVBoxLayout;   _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT) \
  _bs_topScroll = scrollArea;
#define BS_START_SCROLL_V_HMAX_VMIN         BS_START_SCROLL_V    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SCROLL_V_HMIN_VMIN         BS_START_SCROLL_V    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SCROLL_V_HMIN_VMAX         BS_START_SCROLL_V    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SCROLL_V_HMAX_VMAX         BS_START_SCROLL_V    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_SCROLL_H                   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget;  { \
  QScrollArea*    scrollArea = new QScrollArea;   \
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  \
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); \
  scrollArea->setWidgetResizable(true); \
  QBoxLayout* _bs_active = new QHBoxLayout;   _BS_LOCAL_VARIABLES(_BS_BOXED_LAYOUT) \
  _bs_topScroll = scrollArea;
#define BS_START_SCROLL_H_HMAX_VMIN         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SCROLL_H_HMIN_VMIN         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SCROLL_H_HMIN_VMAX         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SCROLL_H_HMAX_VMAX         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);


class BSFakeEba{  QWidget**  pwdg;  QWidget*  native; public:   
  BSFakeEba(QWidget** result){ pwdg = result; }
  void  push(QWidget* wdg){ native = wdg; } 
  QWidget*  pop(){  *pwdg = native; return nullptr; } 
};
#define BS_PREWIDGET_BEGIN(name)   QWidget* name; {   BSFakeEba _bs_wStack(&name); QWidget* _bs_pWidget = NULL;
#define BS_PREWIDGET_END()         }

inline void clearLayout(QLayout* layout)
{
  QLayoutItem* item;
  while ((item = layout->takeAt(0)))
  {
    if (item->layout()){      clearLayout(item->layout());  delete item->layout(); }
    else if (item->widget())  delete item->widget();
    delete item;
  }
}

#define BS_CLEAR_LAYOUT(layname)            clearLayout(layname);
#define BS_CLEAR                            BS_CLEAR_LAYOUT(_bs_active)

#define BS_FOCUS_BAN                        if (_bs_pWidget)  _bs_pWidget->setFocusPolicy(Qt::NoFocus);
#define BS_FOCUS_ADD_STEALER                { QPushButton* _stealer = new QPushButton(); _stealer->setFixedSize(1, 1); _stealer->setAutoDefault(true); _stealer->setFlat(true); BSADD(_stealer);  }

#define BS_FORFOR_1(perline, iter, sublimit, max)     for (int iter=0, _forfor0=(max)/(perline) + ((max)%(perline)?1:0), _forfor1=(max)/(perline), _forfor2=(max)%(perline), sublimit=perline; iter < _forfor0 && (sublimit=iter<_forfor1? perline:_forfor2) != 0; iter++)
#define BS_FORFOR_P(perline, iter, sublimit, max)     for (int iter=0, _forfor0=max, _forfor1=(max)-(max)%(perline), _forfor2=(max)%(perline), sublimit=perline; iter < _forfor0 && (sublimit=iter<_forfor1? perline:_forfor2) != 0; iter+=perline)

#ifndef BSSTRLIST
#define BSSTRLIST(name, ...)  const char* name[] = { __VA_ARGS__ };   const int name##count = sizeof(name)/sizeof(const char*);
#endif


/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/

enum {  
  BFS_NONE, 
  BFS_CHECKABLE   =0x1,
  BFS_CHECKED     =0x3,
  BFS_DISABLED    =0x4,
  BFS_INVISIBLE   =0x8,
  BFS_NOFOCUS     =0x10,
  
  
  BFS_WIDTHMIN_ICON   = 0x0100,
  BFS_WIDTHMAX_ICON   = 0x1000,     BFS_WIDTH_ICON = BFS_WIDTHMIN_ICON | BFS_WIDTHMAX_ICON,
  
  BFS_WIDTHMIN_SYMBOL = 0x0200,
  BFS_WIDTHMAX_SYMBOL = 0x2000,     BFS_WIDTH_SYMBOL = BFS_WIDTHMIN_SYMBOL | BFS_WIDTHMAX_SYMBOL,
  
  BFS_WIDTHMIN_SHORT  = 0x0300,
  BFS_WIDTHMAX_SHORT  = 0x3000,     BFS_WIDTH_SHORT = BFS_WIDTHMIN_SHORT | BFS_WIDTHMAX_SHORT,
  
  BFS_WIDTHMIN_1WORD  = 0x0400,
  BFS_WIDTHMAX_1WORD  = 0x4000,     BFS_WIDTH_1WORD = BFS_WIDTHMIN_1WORD | BFS_WIDTHMAX_1WORD,
  
  BFS_WIDTHMIN_2WORD  = 0x0500,
  BFS_WIDTHMAX_2WORD  = 0x5000,     BFS_WIDTH_2WORD = BFS_WIDTHMIN_2WORD | BFS_WIDTHMAX_2WORD,
  
  BFS_WIDTHMIN_WIDE   = 0x0600,
  BFS_WIDTHMAX_WIDE   = 0x6000,     BFS_WIDTH_WIDE = BFS_WIDTHMIN_WIDE | BFS_WIDTHMAX_WIDE,
};

#ifndef _BS_FIELD_WIDTH_MIN_ICON
#define _BS_FIELD_WIDTH_MIN_ICON 24
#endif
#ifndef _BS_FIELD_WIDTH_MAX_ICON
#define _BS_FIELD_WIDTH_MAX_ICON 24
#endif
#ifndef _BS_FIELD_WIDTH_MIN_SYMBOL
#define _BS_FIELD_WIDTH_MIN_SYMBOL 50
#endif
#ifndef _BS_FIELD_WIDTH_MAX_SYMBOL
#define _BS_FIELD_WIDTH_MAX_SYMBOL 50
#endif
#ifndef _BS_FIELD_WIDTH_MIN_SHORT
#define _BS_FIELD_WIDTH_MIN_SHORT 90
#endif
#ifndef _BS_FIELD_WIDTH_MAX_SHORT
#define _BS_FIELD_WIDTH_MAX_SHORT 90
#endif
#ifndef _BS_FIELD_WIDTH_MIN_1WORD
#define _BS_FIELD_WIDTH_MIN_1WORD 120
#endif
#ifndef _BS_FIELD_WIDTH_MAX_1WORD
#define _BS_FIELD_WIDTH_MAX_1WORD 120
#endif
#ifndef _BS_FIELD_WIDTH_MIN_2WORD
#define _BS_FIELD_WIDTH_MIN_2WORD 200
#endif
#ifndef _BS_FIELD_WIDTH_MAX_2WORD
#define _BS_FIELD_WIDTH_MAX_2WORD 200
#endif
#ifndef _BS_FIELD_WIDTH_MIN_WIDE
#define _BS_FIELD_WIDTH_MIN_WIDE 300
#endif
#ifndef _BS_FIELD_WIDTH_MAX_WIDE
#define _BS_FIELD_WIDTH_MAX_WIDE 300
#endif

static const int _bs_fw_min[] = { _BS_FIELD_WIDTH_MIN_ICON, _BS_FIELD_WIDTH_MIN_SYMBOL, _BS_FIELD_WIDTH_MIN_SHORT, _BS_FIELD_WIDTH_MIN_1WORD, _BS_FIELD_WIDTH_MIN_2WORD, _BS_FIELD_WIDTH_MIN_WIDE  };
static const int _bs_fw_max[] = { _BS_FIELD_WIDTH_MAX_ICON, _BS_FIELD_WIDTH_MAX_SYMBOL, _BS_FIELD_WIDTH_MAX_SHORT, _BS_FIELD_WIDTH_MAX_1WORD, _BS_FIELD_WIDTH_MAX_2WORD, _BS_FIELD_WIDTH_MAX_WIDE  };

/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/*                                          TAU                                                                                                                            */
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/

struct  TauSetup
{
  const char*   pcsztext;
  int           mappedvalue;
  int           ssid;
  int           flags;
  // standard version
  TauSetup(const char* text, int styleid, int mappedval, int BFSFLAGS): pcsztext(text), mappedvalue(mappedval), ssid(styleid), flags(BFSFLAGS){}
  // version for simple autoscaled elements
  TauSetup(const char* text, int styleid): pcsztext(text), mappedvalue(0), ssid(styleid), flags(0){}
};

#ifdef BS_TAU_PROPERTY_OFF
#define TAU_PROPERTY(_wdg, ssid)                {}
#else
#ifdef BS_TAU_PROPERTY_NAME
#define TAU_PROPERTY(_wdg, ssid)                { if (ssid>0) _wdg->setProperty(BS_TAU_PROPERTYNAME, ssid); }
#else
#define TAU_PROPERTY(_wdg, ssid)                { if (ssid>0) _wdg->setProperty("T", ssid); }
#endif
#endif // ifdef BS_NO_PROPERTY





#ifndef TAU_CAST_TEXT
#define TAU_CAST_TEXT(x) QString::fromUtf8(x)
#endif

#define TAU_APPLY_WIDGET(_name)                 TAU_PROPERTY(_name, _ts.ssid) \
                                                if (_ts.flags & BFS_DISABLED) _name->setEnabled(false); \
                                                if (_ts.flags & BFS_INVISIBLE) _name->setVisible(false); \
                                                if (_ts.flags & 0x0700) _name->setMinimumWidth(_bs_fw_min[(_ts.flags >> 8 & 0x7)-1]); \
                                                if (_ts.flags & 0x7000) _name->setMaximumWidth(_bs_fw_max[(_ts.flags >> 12 & 0x7)-1]); \
                                                

#define _TDEPL_LBL_OPEN(_lbl, tausetup)         { \
                                                  TauSetup  _ts(tausetup); \
                                                  _lbl->setText(TAU_CAST_TEXT(_ts.pcsztext)); \
                                                  TAU_APPLY_WIDGET(_lbl)

#define _TDEPL_BTN_OPEN(_btn, tausetup)         { \
                                                  TauSetup  _ts(tausetup); \
                                                  _btn->setText(TAU_CAST_TEXT(_ts.pcsztext)); \
                                                  TAU_APPLY_WIDGET(_btn) \
                                                  if (_ts.flags & BFS_NOFOCUS) _btn->setFocusPolicy(Qt::NoFocus); \
                                                  if (_ts.flags & BFS_CHECKABLE) _btn->setCheckable(true); \
                                                  if ((_ts.flags & BFS_CHECKED) == BFS_CHECKED) _btn->setChecked(true);
                                                  
#define _TDEPL_ICON_OPEN(_btn, tausetup)        { \
                                                  TauSetup  _ts(tausetup); \
                                                  _btn->setIcon(QIcon(_ts.pcsztext)); \
                                                  TAU_APPLY_WIDGET(_btn) \
                                                  if (_ts.flags & BFS_NOFOCUS) _btn->setFocusPolicy(Qt::NoFocus); \
                                                  if (_ts.flags & BFS_CHECKABLE) _btn->setCheckable(true); \
                                                  if ((_ts.flags & BFS_CHECKED) == BFS_CHECKED) _btn->setChecked(true);

#define _TDEPL_EDIT_OPEN(_ed, tausetup)         { \
                                                  TauSetup  _ts(tausetup); \
                                                  _ed->setText(TAU_CAST_TEXT(_ts.pcsztext)); \
                                                  TAU_APPLY_WIDGET(_ed); \
                                                  if (_ts.flags & BFS_NOFOCUS) _ed->setFocusPolicy(Qt::NoFocus);

///

// TT_SELF1..TT_SELF9
#define TAU_SETUP_CLEAN(stylesheetstring)           _bs_pWidget->setStyleSheet(stylesheetstring);
#define TAU_SETUP(stylesheetstring, TT_SELF_IDX)    {   _bs_pWidget->setStyleSheet(stylesheetstring);  TAU_PROPERTY(_bs_pWidget, TT_SELF_IDX);  }
#define TAU_SETUP_CLEAN_FOR(_wdg, stylesheetstring) _wdg->setStyleSheet(stylesheetstring);
#define TAU_SETUP_FOR(_wdg, stylesheetstring, TT_SELF_IDX)       {   _wdg->setStyleSheet(stylesheetstring);  TAU_PROPERTY(_wdg, TT_SELF_IDX);  }

///


#define TDEPL_LBL(_lbl, tausetup)                   _TDEPL_LBL_OPEN(_lbl, tausetup)   }

#define TAU_LBL(_lbl, tausetup)                     QLabel* _lbl = new QLabel(); _TDEPL_LBL_OPEN(_lbl, tausetup) }
#define TAU_LBL_ADD(tausetup, ...)                  { TAU_LBL(_lbl, tausetup) BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_RLBL(_lbl, tausetup)                    QLabel* _lbl = new QLabel(); _lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter); _TDEPL_LBL_OPEN(_lbl, tausetup) }
#define TAU_RLBL_ADD(tausetup, ...)                 { TAU_LBR(_lbl, tausetup) BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_TEXT_ADD(ssid, text, ...)               { QLabel* _lbl = new QLabel(TAU_CAST_TEXT(text)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_NUMI_ADD(ssid, numint, ...)             { QLabel* _lbl = new QLabel(QString::number(numint)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_NUMF1_ADD(ssid, numflt, ...)             { QLabel* _lbl = new QLabel(QString::number(numflt, 'f', 1)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_NUMF2_ADD(ssid, numflt, ...)             { QLabel* _lbl = new QLabel(QString::number(numflt, 'f', 2)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_NUMF3_ADD(ssid, numflt, ...)             { QLabel* _lbl = new QLabel(QString::number(numflt, 'f', 3)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_NUMF4_ADD(ssid, numflt, ...)             { QLabel* _lbl = new QLabel(QString::number(numflt, 'f', 4)); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }
#define TAU_QSTR_ADD(ssid, text, ...)               { QLabel* _lbl = new QLabel(text); TAU_PROPERTY(_lbl, ssid); BSADD(_lbl, ##__VA_ARGS__); }

#define TAUI_LBL_ADD(m_lbl, tausetup, ...)          m_lbl = new QLabel(); _TDEPL_LBL_OPEN(m_lbl, tausetup) } BSADD(m_lbl, ##__VA_ARGS__);

///

#define TDEPL_BTN(_btn, tausetup)                   _TDEPL_BTN_OPEN(_btn, tausetup)   }
#define TDEPL_ICON(_btn, tausetup)                  _TDEPL_ICON_OPEN(_btn, tausetup)  }
#define TDEPL_EDIT(_ed, tausetup)                   _TDEPL_EDIT_OPEN(_ed, tausetup)  }

///

#define TAU_BTN(_btn, tausetup)                     QPushButton* _btn = new QPushButton();  TDEPL_BTN(_btn, tausetup);
#define TAU_CHECK(_btn, tausetup)                   QCheckBox* _btn = new QCheckBox();  TDEPL_BTN(_btn, tausetup);
#define TAUI_BTN_ADD(m_btn, tausetup, ...)          m_btn = new QPushButton(); _TDEPL_BTN_OPEN(m_btn, tausetup) } BSADD(m_btn, ##__VA_ARGS__);
#define TAUI_ICON_ADD(m_btn, tausetup, ...)         m_btn = new QPushButton(); _TDEPL_ICON_OPEN(m_btn, tausetup) } BSADD(m_btn, ##__VA_ARGS__);
#define TAUI_CHECK_ADD(m_btn, tausetup, ...)        m_btn = new QCheckBox(); _TDEPL_BTN_OPEN(m_btn, tausetup) } BSADD(m_btn, ##__VA_ARGS__);

#ifndef TAU_BUTTON_SIGNAL
//#define TAU_BUTTON_SIGNAL SIGNAL(clicked())
#define TAU_BUTTON_SIGNAL SIGNAL(pressed())
#endif

#define TAU_BTN_MAPPED(_btn, tausetup, mapper)      QPushButton* _btn = new QPushButton(); \
                                                    _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                      mapper->setMapping(_btn, _ts.mappedvalue); \
                                                      connect(_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                    }
#define TAUI_BTN_MAPPED(m_btn, tausetup, mapper)    m_btn = new QPushButton(); \
                                                    _TDEPL_BTN_OPEN(m_btn, tausetup) \
                                                      mapper->setMapping(m_btn, _ts.mappedvalue); \
                                                      connect(m_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                    }

#define TAU_BTN_GROUPED(_btn, tausetup, group)      QPushButton* _btn = new QPushButton(); \
                                                    _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                      group->addButton(_btn, _ts.mappedvalue); \
                                                    }
#define TAUI_BTN_GROUPED(m_btn, tausetup, group)    m_btn = new QPushButton(); \
                                                    _TDEPL_BTN_OPEN(m_btn, tausetup) \
                                                      group->addButton(m_btn, _ts.mappedvalue); \
                                                    }


#define TAU_BTN_ADDMAPPED(tausetup, mapper, ...)    { \
                                                      QPushButton* _btn = new QPushButton(); \
                                                      _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                        mapper->setMapping(_btn, _ts.mappedvalue); \
                                                        connect(_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }

#define TAU_BTN_ADDGROUPED(tausetup, group, ...)    { \
                                                      QPushButton* _btn = new QPushButton(); \
                                                      _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                        group->addButton(_btn, _ts.mappedvalue); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }
///

#define TAU_RADIO_ADDMAPPED(tausetup, mapper, ...)  { \
                                                      QRadioButton* _btn = new QRadioButton; \
                                                      _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                        mapper->setMapping(_btn, _ts.mappedvalue); \
                                                        connect(_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }


#define TAU_RADIO_ADDGROUPED(tausetup, group, ...)  { \
                                                      QRadioButton* _btn = new QRadioButton; \
                                                      _TDEPL_BTN_OPEN(_btn, tausetup) \
                                                        group->addButton(_btn, _ts.mappedvalue); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }
///

#define TAU_ICON(_btn, tausetup)                    QPushButton* _btn = new QPushButton(); \
                                                    TDEPL_ICON(_btn, tausetup);

#define TAU_ICON_MAPPED(_btn, tausetup, mapper)     QPushButton* _btn = new QPushButton(); \
                                                    _TDEPL_ICON_OPEN(_btn, tausetup) \
                                                      mapper->setMapping(_btn, _ts.mappedvalue); \
                                                      connect(_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                    }

#define TAUI_ICON_MAPPED(m_btn, tausetup, mapper)   m_btn = new QPushButton(); \
                                                    _TDEPL_ICON_OPEN(m_btn, tausetup) \
                                                      mapper->setMapping(m_btn, _ts.mappedvalue); \
                                                      connect(m_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                    }

#define TAU_ICON_GROUPED(_btn, tausetup, group)     QPushButton* _btn = new QPushButton(); \
                                                    _TDEPL_ICON_OPEN(_btn, tausetup) \
                                                       group->addButton(_btn, _ts.mappedvalue); \
                                                    }

#define TAUI_ICON_GROUPED(m_btn, tausetup, group)   m_btn = new QPushButton(); \
                                                    _TDEPL_ICON_OPEN(m_btn, tausetup) \
                                                       group->addButton(m_btn, _ts.mappedvalue); \
                                                    }

#define TAU_ICON_ADDMAPPED(tausetup, mapper, ...)   { \
                                                      QPushButton* _btn = new QPushButton(); \
                                                      _TDEPL_ICON_OPEN(_btn, tausetup) \
                                                        mapper->setMapping(_btn, _ts.mappedvalue); \
                                                        connect(_btn, TAU_BUTTON_SIGNAL, mapper, SLOT(map())); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }

#define TAU_ICON_ADDGROUPED(tausetup, group, ...)   { \
                                                      QPushButton* _btn = new QPushButton(); \
                                                      _TDEPL_ICON_OPEN(_btn, tausetup) \
                                                        group->addButton(_btn, _ts.mappedvalue); \
                                                      } \
                                                      BSADD(_btn, ##__VA_ARGS__); \
                                                    }
///

#define TAU_EDIT(_ed, tausetup)                     QLineEdit* _ed = new QLineEdit(); \
                                                    _ed->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
                                                    TDEPL_EDIT(_ed, tausetup)

#define TAUI_EDIT_ADD(m_ed, tausetup, ...)          m_ed = new QLineEdit(); \
                                                    m_ed->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
                                                    TDEPL_EDIT(_ed, tausetup) \
                                                    BSADD(m_ed, ##__VA_ARGS__);

#define TAU_EDIT_ADDMAPPED(tausetup, mapper, ...)   { \
                                                      QLineEdit* _ed = new QLineEdit(); \
                                                      _ed->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
                                                      _TDEPL_EDIT_OPEN(_ed, tausetup) \
                                                        mapper->setMapping(_ed, _ts.mappedvalue); \
                                                        connect(_ed, SIGNAL(editingFinished()), mapper, SLOT(map())); \
                                                      } \
                                                      BSADD(_ed, ##__VA_ARGS__); \
                                                    }



/***************************************************************************************************************************************************************************/

#define BS_FL_BEGIN                                 { QFormLayout* _bs_qfl = new QFormLayout();
#define BS_FL_ADD_TW(ssid, text, wdg)                 { TAU_LBL(lbl1, TauSetup(text, ssid));  _bs_qfl->addRow(lbl1, wdg); }
#define BS_FL_ADD_TT(ssid, text, ssid2, text2)        { TAU_LBL(lbl1, TauSetup(text, ssid));  TAU_LBL(lbl2, TauSetup(text2, ssid2));  _bs_qfl->addRow(lbl1, lbl2); }
#define BS_FL_ADD_TI(ssid, text, ssid2, text2, lbl)   { TAU_LBL(lbl1, TauSetup(text, ssid));  TAU_LBL(lbl2, TauSetup(text2, ssid2));  _bs_qfl->addRow(lbl1, lbl2); lbl = lbl2; }
#define BS_FL_ADD_WW(wdg1, wdg2)                      { _bs_qfl->addRow(wdg1, wdg2); }
#define BS_FL_END                                   BSLAYOUT->addLayout(_bs_qfl); }



/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/*                                          BSFieldSetup                                                                                                                   */
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/

#ifdef BSFS_USE

// Another helpers:
class   QFont;
struct  BSFieldSetup
{
  QString     defaultText;
//  const char* toggledText;
  const QFont* pFont;
  int         mappedvalue;
  int         flags;
  int         widthMin, widthMax;
  int         stretch;
  int         alignment;
  
  // standard version
  BSFieldSetup(const QString& text, const QFont* font, int mappedval, int BFSFLAGS, int min_width=-1, int max_width=-1): defaultText(text), pFont(font), mappedvalue(mappedval), 
    flags(BFSFLAGS), widthMin(min_width), widthMax(max_width), stretch(0), alignment(0){}
  
  // version for decorator
  BSFieldSetup(const QFont* font, int BFSFLAGS, int min_width=-1, int max_width=-1): defaultText(), pFont(font), mappedvalue(0), 
    flags(BFSFLAGS), widthMin(min_width), widthMax(max_width), stretch(0), alignment(0){}
  
  // version for simple autoscaled elements
  BSFieldSetup(const QString& text, const QFont* font=NULL): defaultText(text), pFont(font), mappedvalue(0), flags(0), widthMin(-1), widthMax(-1), stretch(0), alignment(0){}
  
  // version for remap
  BSFieldSetup(const QString& text, const BSFieldSetup& cpy, int newmapped=0, int newflags=0): defaultText(text), pFont(cpy.pFont), mappedvalue(newmapped), flags(cpy.flags | newflags), widthMin(cpy.widthMin), widthMax(cpy.widthMax), 
    stretch(cpy.stretch), alignment(cpy.alignment){}
};
#define BSAPPLY_FEATS(_name, fieldsetup)                  if (fieldsetup.pFont)           _name->setFont(*fieldsetup.pFont); \
                                                          if (fieldsetup.widthMin != -1)  _name->setMinimumWidth(fieldsetup.widthMin); \
                                                          if (fieldsetup.widthMax != -1)  _name->setMaximumWidth(fieldsetup.widthMax); \
                                                          if (fieldsetup.flags & BFS_DISABLED) _name->setEnabled(false); \
                                                          if (fieldsetup.flags & BFS_INVISIBLE) _name->setVisible(false); \
                                                          if (fieldsetup.flags & BFS_NOFOCUS) _name->setFocusPolicy(Qt::NoFocus);

#define BSADD_FEATURED(wdg, feats)                        { QWidget* _wdg = wdg; BSAPPLY_FEATS(_wdg, feats) _bs_active->addWidget(_wdg); }

#define BSCOVER(x)                          { x }

#define BSAUTO(QType, _name, fieldsetup)                  BSFieldSetup  loc_fs##_name(fieldsetup); \
                                                          QType* _name = new QType(loc_fs##_name.defaultText); \
                                                          BSAPPLY_FEATS(_name, loc_fs##_name)

///
/// 
/// 

#define BSAUTO_LBL(_lbl, fieldsetup)                      BSAUTO(QLabel, _lbl, fieldsetup)

#define BSAUTO_LBL_ADD(fieldsetup, ...)                   { \
                                                            BSAUTO_LBL(_lbl, fieldsetup) \
                                                            BSADD(_lbl, ##__VA_ARGS__); \
                                                          }

#define BSAUTO_TEXT_ADD(text, ...)                        { \
                                                            QLabel* _lbl = new QLabel(text); \
                                                            BSADD(_lbl, ##__VA_ARGS__); \
                                                          }

///
///
///

#define _BSDEPLOY_BTN_OPEN(_btn, fieldsetup)  { \
                                                BSFieldSetup  loc_fs(fieldsetup); \
                                                _btn->setText(loc_fs.defaultText); \
                                                if (loc_fs.pFont)           _btn->setFont(*loc_fs.pFont); \
                                                if (loc_fs.widthMin != -1)  _btn->setMinimumWidth(loc_fs.widthMin); \
                                                if (loc_fs.widthMax != -1)  _btn->setMaximumWidth(loc_fs.widthMax); \
                                                if (loc_fs.flags & BFS_CHECKABLE) _btn->setCheckable(true); \
                                                if ((loc_fs.flags & BFS_CHECKED) == BFS_CHECKED) _btn->setChecked(true); \
                                                if (loc_fs.flags & BFS_DISABLED) _btn->setEnabled(false); \
                                                if (loc_fs.flags & BFS_INVISIBLE) _btn->setVisible(false); \
                                                if (loc_fs.flags & BFS_NOFOCUS) _btn->setFocusPolicy(Qt::NoFocus);

#define BSDEPLOY_BTN(_btn, fieldsetup)          _BSDEPLOY_BTN_OPEN(_btn, fieldsetup) \
                                              }

#define _BSDEPLOY_ICON_OPEN(_btn, fieldsetup)  { \
                                                BSFieldSetup  loc_fs(fieldsetup); \
                                                _btn->setIcon(QIcon(loc_fs.defaultText)); \
                                                if (loc_fs.widthMin != -1)  _btn->setMinimumSize(loc_fs.widthMin, loc_fs.widthMin); \
                                                if (loc_fs.widthMax != -1)  _btn->setMaximumSize(loc_fs.widthMax, loc_fs.widthMax); \
                                                if (loc_fs.flags & BFS_CHECKABLE) _btn->setCheckable(true); \
                                                if ((loc_fs.flags & BFS_CHECKED) == BFS_CHECKED) _btn->setChecked(true); \
                                                if (loc_fs.flags & BFS_DISABLED) _btn->setEnabled(false); \
                                                if (loc_fs.flags & BFS_INVISIBLE) _btn->setVisible(false); \
                                                if (loc_fs.flags & BFS_NOFOCUS) _btn->setFocusPolicy(Qt::NoFocus);

#define BSDEPLOY_ICON(_btn, fieldsetup)          _BSDEPLOY_ICON_OPEN(_btn, fieldsetup) \
                                              }


///
///
///

#define BSAUTO_BTN(QBtnType, _btn, fieldsetup)            QBtnType* _btn = new QBtnType(); \
                                                          BSDEPLOY_BTN(_btn, fieldsetup);

///
#define BSAUTO_BTN_MAPPED(_btn, fieldsetup, mapper)       QPushButton* _btn = new QPushButton(); \
                                                          _BSDEPLOY_BTN_OPEN(_btn, fieldsetup) \
                                                            mapper->setMapping(_btn, loc_fs.mappedvalue); \
                                                            connect(_btn, SIGNAL(clicked()), mapper, SLOT(map())); \
                                                          }

#define BSAUTO_BTN_GROUPED(_btn, fieldsetup, group)       QPushButton* _btn = new QPushButton(); \
                                                          _BSDEPLOY_BTN_OPEN(_btn, fieldsetup) \
                                                            group->addButton(_btn, loc_fs.mappedvalue); \
                                                          }

///
#define BSAUTO_BTN_ADDMAPPED(fieldsetup, mapper, ...)     { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_BTN(QPushButton, _btn, _fscpy) \
                                                            mapper->setMapping(_btn, _fscpy.mappedvalue); \
                                                            connect(_btn, SIGNAL(clicked()), mapper, SLOT(map())); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }

#define BSAUTO_BTN_ADDGROUPED(fieldsetup, group, ...)     { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_BTN(QPushButton, _btn, _fscpy) \
                                                            group->addButton(_btn, _fscpy.mappedvalue); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }


#define BSAUTO_RADIO_ADDMAPPED(fieldsetup, mapper, ...)   { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_BTN(QRadioButton, _btn, _fscpy) \
                                                            mapper->setMapping(_btn, _fscpy.mappedvalue); \
                                                            connect(_btn, SIGNAL(clicked()), mapper, SLOT(map())); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }


#define BSAUTO_RADIO_ADDGROUPED(fieldsetup, group, ...)   { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_BTN(QRadioButton, _btn, _fscpy) \
                                                            group->addButton(_btn, _fscpy.mappedvalue); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }


////////////

#define BSAUTO_ICON(_btn, fieldsetup)                     QPushButton* _btn = new QPushButton(); \
                                                          BSDEPLOY_ICON(_btn, fieldsetup);

#define BSAUTO_ICON_MAPPED(_btn, fieldsetup, mapper)       QPushButton* _btn = new QPushButton(); \
                                                          _BSDEPLOY_ICON_OPEN(_btn, fieldsetup) \
                                                            mapper->setMapping(_btn, loc_fs.mappedvalue); \
                                                            connect(_btn, SIGNAL(clicked()), mapper, SLOT(map())); \
                                                          }

#define BSAUTO_ICON_GROUPED(_btn, fieldsetup, group)       QPushButton* _btn = new QPushButton(); \
                                                          _BSDEPLOY_ICON_OPEN(_btn, fieldsetup) \
                                                            group->addButton(_btn, loc_fs.mappedvalue); \
                                                          }

///
#define BSAUTO_ICON_ADDMAPPED(fieldsetup, mapper, ...)     { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_ICON(_btn, _fscpy) \
                                                            mapper->setMapping(_btn, _fscpy.mappedvalue); \
                                                            connect(_btn, SIGNAL(clicked()), mapper, SLOT(map())); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }

#define BSAUTO_ICON_ADDGROUPED(fieldsetup, group, ...)     { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_ICON(_btn, _fscpy) \
                                                            group->addButton(_btn, _fscpy.mappedvalue); \
                                                            BSADD(_btn, ##__VA_ARGS__); \
                                                          }

///
///
/// 


#define BSDEPLOY_EDIT(_ed, fieldsetup)                    { \
                                                            BSFieldSetup  loc_fs##_name(fieldsetup); \
                                                            _ed->setText(loc_fs##_name.defaultText); \
                                                            BSAPPLY_FEATS(_ed, loc_fs##_name); \
                                                          }

#define BSAUTO_EDIT(_ed, fieldsetup)                      QLineEdit* _ed = new QLineEdit();   _ed->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
                                                          BSDEPLOY_EDIT(_ed, fieldsetup);

#define BSAUTO_EDIT_ADDMAPPED(fieldsetup, mapper, ...)    { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_EDIT(_ed, _fscpy) \
                                                            mapper->setMapping(_ed, _fscpy.mappedvalue); \
                                                            connect(_ed, SIGNAL(editingFinished()), mapper, SLOT(map())); \
                                                            BSADD(_ed, ##__VA_ARGS__); \
                                                          }

#endif


/*    EXAMPLE usage BSLayouts in MainWindow' constructor
 * 
 * 
    QVBoxLayout *mainLayout = new QVBoxLayout();  // Some layout type
    QFont     fntSTD = QFont(this->font());
    const unsigned int btnMinWidth = 80, btnMaxWidth = 120;
    
    BS_INIT_FOR(mainLayout);
    BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
        BSFieldSetup btns[] = {
          BSFieldSetup(tr("Hello"), &fntSTD, 1, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
          BSFieldSetup(tr("World"), &fntSTD, 0, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
          BSFieldSetup(tr("!"), &fntSTD, 0, BFS_CHECKED, btnMinWidth, btnMaxWidth),
        };
        QButtonGroup* qbg = new QButtonGroup(this);
        qbg->setExclusive(true);
        BS_START_FRAME_H_HMIN_VMIN(BS_FRAME_BOX, 1)
            for (unsigned int i=0; i<sizeof(btns)/sizeof(BSFieldSetup); i++)
              BSAUTO_BTN_ADDGROUPED(btns[i], qbg, 0, Qt::AlignRight);
        BS_STOP
        QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(btnClicked(int)));
    BS_STOP
    
    {
      QWidget* emptyForm = new QWidget(this);
      emptyForm->setLayout(mainLayout);
      this->setCentralWidget(emptyForm);
    }
*/


#endif
