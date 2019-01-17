#ifndef BSQLAYOUT_H
#define BSQLAYOUT_H

#include <QBoxLayout>
#include <QStack>
#include <QString>

/// See example below

enum    _BS_LAYTYPE {   _BS_BOXED_LAYOUT, _BS_NONBOXED_LAYOUT, _BS_NOLAYOUT };

#define _BS_LOCAL_(bs_enum) _BS_LAYTYPE _bs_boxed = bs_enum; \
  Qt::Alignment           _bs_align_h; \
  Qt::Alignment           _bs_align_v; \
  if (bs_enum == _BS_BOXED_LAYOUT){ _bs_align_h = 0; _bs_align_v = 0; } \
  QScrollArea*            _bs_topScroll = NULL;

#define BSLAYOUT_DECL(boxLayout) \
  QWidget*                _bs_pWidget = NULL; \
  QStack<QWidget*>        _bs_wStack; \
  QBoxLayout*             _bs_active = boxLayout; \
  _BS_LOCAL_(_BS_BOXED_LAYOUT)

#define BSWIDGET            _bs_pWidget
#define BSLAYOUT            _bs_active

#define BS_ALIGN_LEFT       _bs_align_h = Qt::AlignLeft;
#define BS_ALIGN_RIGHT      _bs_align_h = Qt::AlignRight;
#define BS_ALIGN_HCENTER    _bs_align_h = Qt::AlignHCenter;
#define BS_ALIGN_NONE       _bs_align_h = 0;

#define BS_FRAME_BOX       QFrame::Box | QFrame::Plain
#define BS_FRAME_PANEL     QFrame::Panel | QFrame::Raised
#define BS_FRAME_SUNKEN    QFrame::Panel | QFrame::Sunken

#define _BS_POP_            if (_bs_boxed == _BS_BOXED_LAYOUT) ((QBoxLayout*)_bs_active)->addWidget(_bs_pWidget, 0, _bs_align_h | _bs_align_v);   else   _bs_active->addWidget(_bs_pWidget);  _bs_pWidget = _bs_wStack.pop();
#define BS_STOP             if (_bs_boxed == _BS_BOXED_LAYOUT || _bs_boxed == _BS_NONBOXED_LAYOUT) ((QWidget*)_bs_pWidget)->setLayout((QLayout*)_bs_active);    if (_bs_topScroll){  _bs_topScroll->setWidget(_bs_pWidget); _bs_pWidget = _bs_topScroll; } }  _BS_POP_

#define BS_START_LAYOUT(boxlayouttype)     _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; {   QBoxLayout*  _bs_active = new boxlayouttype();     _BS_LOCAL_(_BS_BOXED_LAYOUT)
#define BS_START_LAYOUT_HMAX_VMIN(boxlayouttype)          BS_START_LAYOUT(boxlayouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_HMIN_VMIN(boxlayouttype)          BS_START_LAYOUT(boxlayouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_LAYOUT_HMIN_VMAX(boxlayouttype)          BS_START_LAYOUT(boxlayouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_LAYOUT_HMAX_VMAX(boxlayouttype)          BS_START_LAYOUT(boxlayouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_STRETCH                          _bs_active->addStretch(1);
#if QT_VERSION >= 0x050b00
#define BS_CHEAT_VMIN
#else
#define BS_CHEAT_VMIN                       _bs_pWidget->setContentsMargins(0, -10, 0, -10);
#endif

#define BS_START_SPLITTER_V                 _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QSplitter;  { QSplitter*   _bs_active = (QSplitter*)_bs_pWidget; _bs_active->setOrientation(Qt::Vertical); _bs_active->setChildrenCollapsible(false);     _BS_LOCAL_(_BS_NOLAYOUT)
#define BS_START_SPLITTER_V_HMAX_VMIN       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_V_HMIN_VMIN       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_V_HMIN_VMAX       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SPLITTER_V_HMAX_VMAX       BS_START_SPLITTER_V  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_SPLITTER_H                 _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QSplitter;  { QSplitter*   _bs_active = (QSplitter*)_bs_pWidget; _bs_active->setOrientation(Qt::Horizontal); _bs_active->setChildrenCollapsible(false);     _BS_LOCAL_(_BS_NOLAYOUT)
#define BS_START_SPLITTER_H_HMAX_VMIN       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_H_HMIN_VMIN       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SPLITTER_H_HMIN_VMAX       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SPLITTER_H_HMAX_VMAX       BS_START_SPLITTER_H  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_H(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { QHBoxLayout*   _bs_active = new QHBoxLayout;    ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle); ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);     _BS_LOCAL_(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_H_HMAX_VMIN(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_HMIN_VMIN(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_H_HMIN_VMAX(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_H_HMAX_VMAX(fstyle, fwidth)  BS_START_FRAME_H(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_V(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { QVBoxLayout*   _bs_active = new QVBoxLayout;    ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle); ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);     _BS_LOCAL_(_BS_BOXED_LAYOUT)
#define BS_START_FRAME_V_HMAX_VMIN(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_HMIN_VMIN(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_FRAME_V_HMIN_VMAX(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_FRAME_V_HMAX_VMAX(fstyle, fwidth)  BS_START_FRAME_V(fstyle, fwidth)    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_FRAME_E(fstyle, fwidth)   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QFrame;  { ((QFrame*)_bs_pWidget)->setFrameStyle(fstyle);  ((QFrame*)_bs_pWidget)->setLineWidth(fwidth);  QBoxLayout* _bs_active = NULL;    _BS_LOCAL_(_BS_NOLAYOUT)

#define BS_START_SCROLL_V                   _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget;  { \
  QScrollArea*    scrollArea = new QScrollArea;   \
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  \
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); \
  scrollArea->setWidgetResizable(true); \
  QVBoxLayout* _bs_active = new QVBoxLayout;   _BS_LOCAL_(_BS_BOXED_LAYOUT) \
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
  QHBoxLayout* _bs_active = new QHBoxLayout;   _BS_LOCAL_(_BS_BOXED_LAYOUT) \
  _bs_topScroll = scrollArea;
#define BS_START_SCROLL_H_HMAX_VMIN         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_SCROLL_H_HMIN_VMIN         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_SCROLL_H_HMIN_VMAX         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_SCROLL_H_HMAX_VMAX         BS_START_SCROLL_H    _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_STACK                     _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QWidget; {   QStackedLayout*  _bs_active = new QStackedLayout();    _BS_LOCAL_(_BS_NONBOXED_LAYOUT)
#define BS_START_STACK_HMAX_VMIN            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_STACK_HMIN_VMIN            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_STACK_HMIN_VMAX            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_STACK_HMAX_VMAX            BS_START_STACK  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BS_START_GROUP(title, layouttype)  _bs_wStack.push(_bs_pWidget);  _bs_pWidget = new QGroupBox(title);  { layouttype*   _bs_active = new layouttype();   _BS_LOCAL_(_BS_BOXED_LAYOUT)
#define BS_START_GROUP_HMAX_VMIN(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
#define BS_START_GROUP_HMIN_VMIN(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
#define BS_START_GROUP_HMIN_VMAX(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
#define BS_START_GROUP_HMAX_VMAX(title, layouttype)            BS_START_GROUP(title, layouttype)  _bs_pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

#define BSADD(...)                          _bs_active->addWidget(__VA_ARGS__);

/***************************************************************************************************************************************************************************/

enum {  BFS_NONE, BFS_CHECKABLE=1, BFS_CHECKED=3, BFS_DISABLED=4, BFS_INVISIBLE=8 };

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
  BSFieldSetup(const QString& text, const BSFieldSetup& cpy, int newmapped, int newflags=0): defaultText(text), pFont(cpy.pFont), mappedvalue(newmapped), flags(cpy.flags | newflags), widthMin(cpy.widthMin), widthMax(cpy.widthMax), 
    stretch(cpy.stretch), alignment(cpy.alignment){}
};
#define BSAPPLY_FEATS(_name, fieldsetup)                  if (fieldsetup.pFont)           _name->setFont(*fieldsetup.pFont); \
                                                          if (fieldsetup.widthMin != -1)  _name->setMinimumWidth(fieldsetup.widthMin); \
                                                          if (fieldsetup.widthMax != -1)  _name->setMaximumWidth(fieldsetup.widthMax); \
                                                          if (fieldsetup.flags & BFS_DISABLED) _name->setEnabled(false); \
                                                          if (fieldsetup.flags & BFS_INVISIBLE) _name->setVisible(false);

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

#define BSDEPLOY_BTN(_btn, fieldsetup)                    { \
                                                            BSFieldSetup  loc_fs##_name(fieldsetup); \
                                                            _btn->setText(loc_fs##_name.defaultText); \
                                                            if (loc_fs##_name.pFont)           _btn->setFont(*loc_fs##_name.pFont); \
                                                            if (loc_fs##_name.widthMin != -1)  _btn->setMinimumWidth(loc_fs##_name.widthMin); \
                                                            if (loc_fs##_name.widthMax != -1)  _btn->setMaximumWidth(loc_fs##_name.widthMax); \
                                                            if (loc_fs##_name.flags & BFS_CHECKABLE) _btn->setCheckable(true); \
                                                            if ((loc_fs##_name.flags & BFS_CHECKED) == BFS_CHECKED) _btn->setChecked(true); \
                                                            if (loc_fs##_name.flags & BFS_DISABLED) _btn->setEnabled(false); \
                                                            if (loc_fs##_name.flags & BFS_INVISIBLE) _btn->setVisible(false); \
                                                          }

#define BSAUTO_BTN(QBtnType, _btn, fieldsetup)            QBtnType* _btn = new QBtnType(); \
                                                          BSDEPLOY_BTN(_btn, fieldsetup);

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

#define BSAUTO_RADIO_ADDGROUPED(fieldsetup, group, ...)   { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_BTN(QRadioButton, _btn, _fscpy) \
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

#define BSAUTO_EDIT(_ed, fieldsetup)                      QLineEdit* _ed = new QLineEdit();   _ed->setAlignment(Qt::AlignRight); \
                                                          BSDEPLOY_EDIT(_ed, fieldsetup);

#define BSAUTO_EDIT_ADDMAPPED(fieldsetup, mapper, ...)    { \
                                                            BSFieldSetup _fscpy(fieldsetup); \
                                                            BSAUTO_EDIT(_ed, _fscpy) \
                                                            mapper->setMapping(_ed, _fscpy.mappedvalue); \
                                                            connect(_ed, SIGNAL(editingFinished()), mapper, SLOT(map())); \
                                                            BSADD(_ed, ##__VA_ARGS__); \
                                                          }


struct  BSUOD_0: public QObjectUserData
{
  unsigned int  id;
  BSUOD_0(unsigned int identifier): id(identifier) {}
};

struct  BSUOD_1: public QObjectUserData
{
  char  textes[2][64];
  BSUOD_1(const char* text1, const char* text2){  strncpy(textes[0], text1, 64); strncpy(textes[1], text2, 64); }
};

struct  BSUOD_2: public QObjectUserData
{
  void* rawlink;
  BSUOD_2(void* directLink): rawlink(directLink) {}
};

struct  BSUOD_3: public QObjectUserData
{
  QString str;
  BSUOD_3(const QString& value): str(value) {}
};


#define BS_FORFOR_1(perline, iter, sublimit, max)     for (int iter=0, _forfor0=(max)/(perline) + ((max)%(perline)?1:0), _forfor1=(max)/(perline), _forfor2=(max)%(perline), sublimit=perline; iter < _forfor0 && (sublimit=iter<_forfor1? perline:_forfor2) != 0; iter++)
#define BS_FORFOR_P(perline, iter, sublimit, max)     for (int iter=0, _forfor0=max, _forfor1=(max)-(max)%(perline), _forfor2=(max)%(perline), sublimit=perline; iter < _forfor0 && (sublimit=iter<_forfor1? perline:_forfor2) != 0; iter+=perline)

/*
#define REGISTER_UOD(widget, signame, uodid, ...) \
  { \
    BSUOD_##uodid *widget_uod_##uodid = new BSUOD_##uodid(__VA_ARGS__); \
    widget->setUserData(uodid, widget_uod_##uodid); \
    connect(widget, SIGNAL(signame), this, SLOT(view_UOD##uodid)); \
  }
*/

/*    EXAMPLE usage BSLayouts in MainWindow' constructor
 * 
 * 
    QVBoxLayout *mainLayout = new QVBoxLayout();  // Some layout type
    QFont     fntSTD = QFont(this->font());
    const unsigned int btnMinWidth = 80, btnMaxWidth = 120;
    
    BSLAYOUT_DECL(mainLayout);
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
