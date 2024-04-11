#include "bsdrawdialogs.h"

#include "visual/layout/bsqlayout.h"
#include <QWidget>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>
#include <QTimer>

// bsqlayout includes
#include <QSplitter>
#include <QFormLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QTabBar>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMouseEvent>


#include "bsdrawintensity.h"
#include "../palettes/bspalettes_std.h"
#include "../palettes/bspalettes_adv.h"
#include "../overlays/bsgrid.h"


class BSUOD_GRID: public QObjectUserData
{
public:
  virtual void  apply(int)=0;
};

class BSUOD_GRID_FORM: public BSUOD_GRID
{
  unsigned int* pdata;
public:
  virtual void  apply(int d){ *pdata = d; }
  BSUOD_GRID_FORM(unsigned int* ptr): pdata(ptr){}
};

class BSUOD_GRID_COLOR: public BSUOD_GRID
{
  float* pdata;
public:
  virtual void  apply(int d){ *pdata = d/255.0f; }
  BSUOD_GRID_COLOR(float* ptr): pdata(ptr){}
};

class BSUOD_GRID_FTI: public BSUOD_GRID
{
  float* pdata;
public:
  virtual void  apply(int d){ *pdata = float(d); }
  BSUOD_GRID_FTI(float* ptr): pdata(ptr){}
};

class BSUOD_GRID_OS: public BSUOD_GRID
{
  float* pdata;
public:
  virtual void  apply(int d){ *pdata = d/100.0f; }
  BSUOD_GRID_OS(float* ptr): pdata(ptr){}
};

/************/

class BSUOD_GRID_PLACE: public QObjectUserData
{
  float* pdata;
public:
  void  apply(QString t){ bool ok; float fvalue = t.toFloat(&ok); if (ok) *pdata = fvalue; }
  BSUOD_GRID_PLACE(float* ptr): pdata(ptr){}
};



class BSGridDialogPrivate
{
public:
  QWidget*        wdgColor;
  DrawIntensity*  draw;
public:
  overlayconfig_t     ocfg;
public:
  BSGridDialogPrivate(){}
  virtual ~BSGridDialogPrivate();
  Ovldraw* exemplar() const
  {
    OGridRegular* ogr = new OGridRegular(//overt? OGridRegular::REGULAR_VERT : OGridRegular::REGULAR_HORZ,
                                         OGridRegular::REGULAR_VERT,
                                         ocfg.cn, ocfg.center, ocfg.step, ocfg.kls);
    ogr->setOTS(ocfg.basics);
    return ogr;
    
//    OGridRegular* ogr = new OGridRegular(//overt? OGridRegular::REGULAR_VERT : OGridRegular::REGULAR_HORZ,
//                                         OGridRegular::RISK_VERT,
//                                         ocfg.cn, ocfg.center, ocfg.step, 
//                                         0.5f, false, 0.1f,
//                                         ocfg.kls);
    
//    OGridDecart* ogr = new OGridDecart(//overt? OGridRegular::REGULAR_VERT : OGridRegular::REGULAR_HORZ,
//                                         ocfg.cn, ocfg.center, 0.5f, ocfg.step, 0.1f, 2,
//                                       -1, 0.4f, 0.4f, -1,
////                                       -1,-1,-1,-1,
//                                         ocfg.kls);
    ogr->setOTS(ocfg.basics);
    return ogr;
  }
  void  recreate()
  {
    draw->ovlClearAll();
    draw->ovlPushBack(exemplar());
  }
};

BSGridDialogPrivate::~BSGridDialogPrivate(){}

inline overlayconfig_t exemplar0(bool vertical)
{
  overlayconfig_t result = { vertical, { 0.0f, 0.0f, 1.0f }, CR_RELATIVE, 0.5f, 0.25f, { 4,1,0, 1.0f,1.0f,1.0f, 3, OLS_FLAT} };
  return result;
}

BSGridDialog::BSGridDialog(bool vertical, QWidget* parent, Qt::WindowFlags f): BSGridDialog(EXM_GREY, vertical, exemplar0(vertical), parent, f)
{ 
}


BSGridDialog::BSGridDialog(EXAMPLE exm, bool vertical, const overlayconfig_t& bc, QWidget* parent, Qt::WindowFlags f): QDialog(parent, f), 
  d_ptr(new BSGridDialogPrivate)
{
  Q_D(BSGridDialog);
  d->ocfg = bc;
  
  this->setWindowTitle(QString::fromUtf8("Мастер создания сетки"));
  BS_INIT_V_NOTCUTED
  BS_START_LAY_HV_IF(vertical)
    BS_START_LAY_VH_IF(vertical)
    {
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Образец"))
        QButtonGroup* qbg = new QButtonGroup(this);
        const char* cc[] = { "Серый", "Черный", "Белый", "Цветной" };
        for (int i=0; i<sizeof(cc)/sizeof(const char*); i++)
//          BSAUTO_RADIO_ADDGROUPED(BSFieldSetup(cc[i], nullptr, i, 0), qbg);
          TAU_RADIO_ADDGROUPED(TauSetup(cc[i], -1, i, 0), qbg);
        qbg->button((int)exm)->setChecked(true);
        QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(mocdocp(int)));
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)

      {
        d->draw = new DrawIntensity(300, 1, 1, vertical? OR_BTLR : OR_LRBT );
        if (vertical)
        {
          d->draw->setMinimumWidth(120);
          d->draw->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        }
        else
        {
          d->draw->setMinimumHeight(120);
    //      d->draw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        }
        d->draw->setDataPalette(&paletteBkWh);
        {
          float data[300];
          for (int d=0; d<300; d++)
            data[d] = d / 299.0f;
          d->draw->setData(data);
        }
        BSADD(d->draw, 0, Qt::AlignHCenter)
      }
    }
    BS_STOP
    
    BS_START_LAY_VH_IF(vertical)
    {
      BS_STRETCH      
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Наложение"))
        const char* strnames[] = { "Толщина", "Прозрачность", "Отсечение" };
        float* values[] = { &d->ocfg.basics.thickness, &d->ocfg.basics.opacity, &d->ocfg.basics.slice_hl };
        for (int i=0; i<3; i++)
        {
          BS_START_LAY_H_HMAX_VMIN
            TAU_TEXT_ADD(-1, strnames[i])
            BS_STRETCH
            QSpinBox* qsb = new QSpinBox;
            qsb->setButtonSymbols(QSpinBox::UpDownArrows);
            qsb->setRange(0, 100);
            qsb->setValue(i == 0? *values[i] : *values[i]*100.0f);
            qsb->setMaximumWidth(48);
            if (i == 0)
              qsb->setUserData(1, new BSUOD_GRID_FTI(values[i]));
            else
              qsb->setUserData(1, new BSUOD_GRID_OS(values[i]));
            QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(mocdoci(int)));
            BSADD(qsb);
          BS_STOP
        }
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Место"))
        const char* strnames[] = { "Начало", "Шаг" };
        float* values[] = { &d->ocfg.center, &d->ocfg.step };
        for (int i=0; i<2; i++)
        {
          BS_START_LAY_H_HMAX_VMIN
//            TAU_TEXT_ADD(-1, strnames[i]))
            TAU_TEXT_ADD(-1, strnames[i])
            BS_STRETCH
            QLineEdit* qle = new QLineEdit(QString::number(*values[i]));
//            qle->setValidator(new QDoubleValidator());
//            qle->setInputMask("0.00000");
//            qle->set
            qle->setMaximumWidth(48);
            qle->setUserData(1, new BSUOD_GRID_PLACE(values[i]));
            QObject::connect(qle, SIGNAL(textEdited(const QString&)), this, SLOT(mocdocs(const QString&)));
            BSADD(qle);
          BS_STOP
        }
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)
      BS_STRETCH
    }
    BS_STOP

    BS_START_LAY_VH_IF(vertical)
      BS_STRETCH
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Форма"))
        const char* strnames[] = { "Отрезок", "Пропуск", "Точка" };
        unsigned int* values[] = { &d->ocfg.kls.lenstroke, &d->ocfg.kls.lenspace, &d->ocfg.kls.countdot };
        for (int i=0; i<3; i++)
        {
          BS_START_LAY_H_HMAX_VMIN
            TAU_TEXT_ADD(-1, strnames[i])
            BS_STRETCH
            QSpinBox* qsb = new QSpinBox;
            qsb->setButtonSymbols(QSpinBox::UpDownArrows);
            qsb->setRange(0, 30);
            qsb->setValue(*values[i]);
            qsb->setMaximumWidth(56);
            qsb->setUserData(1, new BSUOD_GRID_FORM(values[i]));
            QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(mocdoci(int)));
            BSADD(qsb);
          BS_STOP
        }
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Тип цвета"))
        const char* strnames[] = { "Инверсия 1", "Инверсия 2", "Инверсия 3" };
        Q_ASSERT( d->ocfg.kls.inversive >= 0 && d->ocfg.kls.inversive < sizeof(strnames)/sizeof(const char*) + 1);
      
        QButtonGroup* qbt = new QButtonGroup(this);
        for (int i=0; i<sizeof(strnames)/sizeof(const char*); i++)
        {
//          BSAUTO_RADIO_ADDGROUPED(BSFieldSetup(strnames[i], nullptr, i+1, 0), qbt);
          TAU_RADIO_ADDGROUPED(TauSetup(strnames[i], -1, i+1, 0), qbt);
        }
//        BSAUTO_RADIO_ADDGROUPED(BSFieldSetup("Моноцвет", nullptr, 0, 0), qbt);
        TAU_RADIO_ADDGROUPED(TauSetup("Моноцвет", -1, 0, 0), qbt);
        qbt->button(d->ocfg.kls.inversive)->setChecked(true);
        QObject::connect(qbt, SIGNAL(buttonClicked(int)), this, SLOT(mocdoct(int)));
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)
      BS_START_GROUP_V_HMIN_VMIN(QString::fromUtf8("Цвет"))
        const char* strnames[] = { "Красный", "Зелёный", "Синий" };
        float* values[] = { &d->ocfg.kls.r, &d->ocfg.kls.g, &d->ocfg.kls.b };
        for (int i=0; i<3; i++)
        {
          BS_START_LAY_H_HMAX_VMIN
            TAU_TEXT_ADD(-1, strnames[i])
            BS_STRETCH
            QSpinBox* qsb = new QSpinBox;
            qsb->setButtonSymbols(QSpinBox::UpDownArrows);
            qsb->setRange(0, 255);
            qsb->setValue(int(*values[i]*255));
            qsb->setUserData(1, new BSUOD_GRID_COLOR(values[i]));
            QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(mocdoci(int)));
            qsb->setMaximumWidth(56);
            BSADD(qsb);
          BS_STOP
        }
        d->wdgColor = BSWIDGET;
      BS_STOP_EX_ALIGNED(0, Qt::AlignHCenter)
      BS_STRETCH
    BS_STOP
  BS_STOP
  
  BS_START_LAY_H_HMAX_VMIN
    BS_STRETCH
    TAU_BTN(btnAccept, TauSetup("Принять", -1, 0, 0));
    QObject::connect(btnAccept, SIGNAL(clicked()), this, SLOT(accept()));
    BSADD(btnAccept)
    btnAccept->setAutoDefault(false);
    BS_SPACING(32)
    TAU_BTN(btnReject, TauSetup("Отменить", -1, 0, 0));
    QObject::connect(btnReject, SIGNAL(clicked()), this, SLOT(reject()));
    BSADD(btnReject)
    btnReject->setAutoDefault(false);
    BS_STRETCH
  BS_STOP
  d->recreate();
    
  BS_RESIZE_OFF
      
  this->setLayout(BSLAYOUT);
}

BSGridDialog::~BSGridDialog()
{
  delete d_ptr;
}

overlayconfig_t BSGridDialog::result() const
{
  const Q_D(BSGridDialog);
  return d->ocfg;
}

Ovldraw* BSGridDialog::createOverlay() const
{
  const Q_D(BSGridDialog);
  return d->exemplar();
}

void BSGridDialog::mocdoci(int value)
{
  ((BSUOD_GRID*)sender()->userData(1))->apply(value);
  Q_D(BSGridDialog);
  d->recreate();
}

void BSGridDialog::mocdocs(const QString& text)
{
  ((BSUOD_GRID_PLACE*)sender()->userData(1))->apply(text);
  Q_D(BSGridDialog);
  d->recreate();
}

void BSGridDialog::mocdocp(int v)
{
  Q_D(BSGridDialog);
  const IPalette* pals[] = {  &paletteBkWh, &paletteBk, &paletteWh, (const IPalette*)&palette_gnu_rainbow };
  d->draw->setDataPalette(pals[v]);
}

void BSGridDialog::mocdoct(int v)
{
  Q_D(BSGridDialog);
  d->ocfg.kls.inversive = v;
  d->wdgColor->setEnabled(v == 0);
  d->recreate();
}

