/// MainWindow for simple example
/// Created By: Elijah Vlasov
#include "MainWindow.h"

#include "../bsdrawgraph.h"
#include "../bsdrawintensity.h"
#include "../palettes/bspalettes_adv.h"
#include <stdlib.h>   // for rand()
#include <qmath.h>    // for qFastSin()
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  QBoxLayout* pMainLayout = new QHBoxLayout();
  
  const unsigned int COUNT_SAMPLES = 180;
  const unsigned int COUNT_PORTIONS = 2;
//  pDraw = new DrawGraph(samples);
//  pDraw = new DrawIntensity(samples/16, 16);
  float arr_random[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_random[i] = rand()/float(RAND_MAX);
  
  float arr_sin[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_sin[i] = (qFastSin((float(i%COUNT_SAMPLES)/COUNT_SAMPLES)*2*M_PI) + 1)/2.0f;
  
  float arr_peaks[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
  {
    arr_peaks[i] = arr_random[i]*arr_random[i]*arr_random[i];
    arr_peaks[i] = arr_peaks[i] > 0.2f && arr_peaks[i] < 0.7f? arr_peaks[i]/5.0 : arr_peaks[i];
  }
  
  float arr_pseudonormal[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_pseudonormal[i] = i%2 == 0? (arr_random[i]/1.25f + 0.1f) : (arr_random[i]/2.0f + 0.25f);
  
  float arr_ramp[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_ramp[i] = float(i)/(COUNT_SAMPLES*COUNT_PORTIONS - 1);

  
  QVBoxLayout* pLayout;
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 1 row 1 column
    unsigned int scaling = 8;
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/scaling, COUNT_PORTIONS, graphopts_t::goInterp(0.2f, DE_LINTERP));
    pDraw->setScalingLimitsHorz(scaling);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  
#define OTHERDRAWS
#ifdef OTHERDRAWS
  {
    /// 2 row 1 column
    unsigned int scaling = 8;
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/scaling, COUNT_PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP, 4, 1.0f));
    pDraw->setScalingLimitsHorz(scaling);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 1 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goDots(2, 0.5f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 1 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/8, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(), coloropts_t::copts(CP_OWNRANGE_SYMMETRIC, 1.0f, 0.2f));
    pDraw->setScalingLimitsHorz(8);
    pDraw->setScalingLimitsVert(8);
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINETOP, 1, 1.0f,1.0f,1.0f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_ALL, DPostmask::PM_CONTOUR, 0, 0x00333333, 0.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
    
//    this->setMinimumWidth(2478);
//    this->setMinimumHeight(1200);
    
  }
  
  ///////////////////////////////////////////////
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 1 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/2, COUNT_PORTIONS, graphopts_t::goInterp(0.0f, DE_CENTER), coloropts_t::copts(CP_SINGLE, 0.9f, 0.1f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots(), coloropts_t::copts(CP_RANGE, 1.0f, 0.5f));
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINERIGHTBOTTOM, 0, 0.6f,0.6f,0.6f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_EMPTY, DPostmask::PM_DOT, 0, 0.6f,0.6f,0.6f));
    pDraw->setScalingLimitsSynced(4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots());
    pDraw->setPostMask(DPostmask(DPostmask::PO_EMPTY, DPostmask::PM_LINEBOTTOM, 0, 0.2f,0.2f,0.2f));
    pDraw->setScalingLimitsHorz(4);
    pDraw->setScalingLimitsVert(4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMin());
    pDraw->setScalingLimitsHorz(4);
    pDraw->setScalingLimitsVert(3);
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINETOP, 1, 1.0f,1.0f,1.0f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_CONTOUR, 0, 0x00333333, 0.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  
  ///////////////////////////////////////////////
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 1 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goInterp(0.0f, DE_NONE));
    pDraw->setDataPalette(&palette_gnu_PuYe);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots(0, 0.0f, DE_CENTER), coloropts_t::copts(CP_RANGE));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 3 column
//    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(0.0f, DE_CENTER), DrawGraph::CP_SINGLE, 0.5f, 1.0f);
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(PR_SUMMARY, DE_NONE), coloropts_t::copts(CP_SINGLE, 0.5f, 1.0f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINELEFTBOTTOM, 0, 0.0f));
    pDraw->setScalingLimitsVert(5);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(0.0f, DE_TRIANGLE));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
#endif
  
  {
    QWidget *central = new QWidget();
    central->setLayout(pMainLayout);
    this->setCentralWidget(central);
  }
}

MainWindow::~MainWindow()
{
}
