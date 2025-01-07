#include <QApplication>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>

#include "bsdrawgraph.h"    // INCLUDEPATH includes bsdraw dir in .pro file
#include "palettes/bspalettes_std.h"

#include "overlays/bsinteractive.h"
#include "overlays/bsfigures.h"

class OverlayClicker: public QObject, public IOverlayReactor
{
  Q_OBJECT
protected:
  virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* cvs, bool* /*doStop*/);
signals:
  void  sig_clicked(int, int, float, float);
};

bool OverlayClicker::overlayReactionMouse(OVL_REACTION_MOUSE orm, const coordstriumv_t* cvs, bool*)
{
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
    emit sig_clicked(cvs->fx_pix, cvs->fy_pix, cvs->fx_01, cvs->fy_01);
  return true; 
}


#include "main.moc"



int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  const int DOSPLITHORZ = 2;
  const int DOSPLITVERT = 3;
  
  /// Data generation
  const unsigned int COUNT_SAMPLES = 32;
  const unsigned int COUNT_PORTIONS = 1*DOSPLITHORZ*DOSPLITVERT;     /// graphs on draw
  float arr_random[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_random[i] = rand()/float(RAND_MAX);  /// default data diapason 0..1
  
  QVBoxLayout* layV = new QVBoxLayout;
  QButtonGroup* qbg = new QButtonGroup(layV);
  
  {
    /// Drawing
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goHistogram(0.3f, DE_LINTERP, 0.0f), coloropts_t::copts(),
                              DOSPLITHORZ*DOSPLITVERT == 1? SP_NONE :
                                        SP_COLUMNS_TB_2
                                        );
    pDraw->setContentsMargins(16, 12, 16, 24);
    pDraw->setDataPalette(&paletteBkBlWh);  // dont forget to set palette. Default palette is null
    pDraw->setViewAlign(0.5f, 0.5f);    // centered
    pDraw->setScalingLimitsHorz(4);   // 'space' occupied by one sample. Default is 1
    pDraw->setMinimumHeight(200);
    pDraw->setOutsideAreaModeLM(DrawQWidget::OROM_OUTSIDER);
    pDraw->setData(arr_random);     // default draw bounds are 0..1
    
    layV->addWidget(pDraw, 1);
    layV->addSpacing(32);
    
    enum { E_XPIX, E_YPIX, E_X01, E_Y01, _E_COUNT };
    enum { T_CLICKING, T_TRACKING, T_OVERLAY, _T_COUNT };
    
    /// 1. New buttons to layout
    {
      QHBoxLayout* layH = new QHBoxLayout;
      qbg->addButton(new QRadioButton("Клик"), 0);
      qbg->addButton(new QRadioButton("Трэкинг"), 1);
      qbg->addButton(new QRadioButton("Оверлей"), 2);
      layH->addStretch();
      layH->addSpacing(32);
      for (int i=0; i<_T_COUNT; i++)
      {
        layH->addWidget(qbg->button(i));
        layH->addSpacing(32);
      }
      layH->addStretch();
      layV->addLayout(layH);
    }
    
    QLabel* qlb[_T_COUNT][_E_COUNT];
    for (int i=0; i<_T_COUNT; i++)
      for (int j=0; j<_E_COUNT; j++)
      {
        qlb[i][j] = new QLabel();
        qlb[i][j]->setMinimumWidth(68);
        qlb[i][j]->setAlignment(Qt::AlignRight);
        qlb[i][j]->setFrameShape(QLabel::Panel);
      }
    
    /// 2. New labels to layout
    {
      QHBoxLayout* layH = new QHBoxLayout;
      layH->addStretch();
      for (int i=0; i<_T_COUNT; i++)
      {
        QFormLayout* qsl = new QFormLayout();
        const char* caps[] = { "Xpix", "Ypix", "X01", "Y01" };
        for (int j=0; j<_E_COUNT; j++)
        {
          qsl->addRow(new QLabel(caps[j]), qlb[i][j]);
        }
        layH->addLayout(qsl);
        layH->addStretch();
      }
      layV->addLayout(layH);
    }
    
    /// 3. Buttons event
    {
      QObject::connect(qbg, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), 
        [=](int id)
        {
          DrawEventReactor* previousReactor = nullptr;
          if (id == 0)    /// 3.1. click event trough setProactive
          {
            pDraw->ovlClearAll();
            BSQMousePoint* reactor = new BSQMousePoint(BSQMousePoint::MSP_LEFTBUTTON);
            QObject::connect(reactor, static_cast<void (BSQMousePoint::*)(QPoint)>(&BSQMousePoint::active), [=](QPoint pt)
            {
              qlb[T_CLICKING][E_XPIX]->setNum(pt.x());
              qlb[T_CLICKING][E_YPIX]->setNum(pt.y());
            });
            QObject::connect(reactor, static_cast<void (BSQMousePoint::*)(QPointF)>(&BSQMousePoint::active), [=](QPointF pt)
            {
              qlb[T_CLICKING][E_X01]->setText(QString::number(pt.x(), 'f', 3));
              qlb[T_CLICKING][E_Y01]->setText(QString::number(pt.y(), 'f', 3));
            });
            pDraw->setMouseTracking(false);
            previousReactor = pDraw->setProactive(reactor);
          }
          else if (id == 1)    /// 3.2. click event trough setProactive with Tracking enabled
          {
            pDraw->ovlClearAll();
            BSQTrackerXY* reactor = new BSQTrackerXY();
            QObject::connect(reactor, &BSQTrackerXY::tracked, [=](float x01, float y01, float xpix, float ypix)
            {
              qlb[T_TRACKING][E_XPIX]->setNum(xpix);
              qlb[T_TRACKING][E_YPIX]->setNum(ypix);
              qlb[T_TRACKING][E_X01]->setText(QString::number(x01, 'f', 3));
              qlb[T_TRACKING][E_Y01]->setText(QString::number(y01, 'f', 3));
            });
            pDraw->setMouseTracking(true);
            previousReactor = pDraw->setProactive(reactor);
          }
          else if (id == 2)    /// 3.1. click event trough overlay
          {
            previousReactor = pDraw->setProactive(nullptr);
            OverlayClicker* ocl = new OverlayClicker;
            QObject::connect(ocl, &OverlayClicker::sig_clicked, [=](int xpix, int ypix, float x01, float y01)
            {
              qlb[T_OVERLAY][E_XPIX]->setNum(xpix);
              qlb[T_OVERLAY][E_YPIX]->setNum(ypix);
              qlb[T_OVERLAY][E_X01]->setText(QString::number(x01, 'f', 3));
              qlb[T_OVERLAY][E_Y01]->setText(QString::number(y01, 'f', 3));
            });
            Ovldraw* ocarrier = new OActiveCursorCarrierL(ocl);
            int oap = pDraw->ovlPushBack(ocarrier);
            Ovldraw* visir = new OFVisir(CR_RELATIVE, 0.0f, 0.0f, CR_ABSOLUTE_NOSCALED, 4, 12, linestyle_yellow(0,1,1));
            pDraw->ovlPushBack(visir, oap);
            pDraw->setMouseTracking(false);
          }
          if (previousReactor)
            delete previousReactor;
        }
      );
    }
  }
  
  QWidget* mainWidget = new QWidget();
  mainWidget->setLayout(layV);
  mainWidget->show();
  qbg->button(0)->click();  /// default choose
  
  return a.exec();
}
