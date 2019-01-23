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
  
  const unsigned int COUNT_SAMPLES = 256;
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

  
  QVBoxLayout* pLayout;
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 1
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINTERP), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINTERP, 0.2f, 0xFFFFFFFF, 1, 0.5f), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte_inv);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_DOTS, 0.0f, 0xFFFFFFFF, 1, 0.5f), DrawGraph::DC_OFF);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/8, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMIN), DrawGraph::DC_OFF, 0.5f, -1.0f);
    pDraw->setPostMask(DPostmask(DPostmask::PM_LINETOP, DPostmask::PO_SIGNAL, 1, 1.0f,1.0f,1.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  
  ///////////////////////////////////////////////
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 5
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINTERP, 0.0f, 0xFFFFFFFF, 0, 0.0f, graphopts_t::DE_TRIANGLE2), DrawGraph::DC_OFF, 0.5f, -1.0f);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 6
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_DOTS), DrawGraph::DC_DOWNBASE);
    pDraw->setPostMask(DPostmask(DPostmask::PM_LINERIGHTBOTTOM, DPostmask::PO_SIGNAL, 0, 0.6f,0.6f,0.6f));
    pDraw->setScalingLimitsSynced(4,4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 7
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_DOTS), DrawGraph::DC_OFF);
    pDraw->setPostMask(DPostmask(DPostmask::PM_LINEBOTTOM, DPostmask::PO_EMPTY, 0, 0.2f,0.2f,0.2f));
    pDraw->setScalingLimitsSynced(4,4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 8
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/16, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMIN), DrawGraph::DC_DOWNBASE);
    pDraw->setPostMask(DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_SIGNAL, 0, 1.0f,1.0f,1.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  
  ///////////////////////////////////////////////
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 9
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINTERP, 0.0f, 0xFFFFFFFF, 0, 0.0f, graphopts_t::DE_TRIANGLE), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 10
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_DOTS, graphopts_t::DE_CENTER), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 11
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMIN, graphopts_t::DE_TRIANGLE), DrawGraph::DC_OFF);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 12
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMIN, graphopts_t::DE_CENTER), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  
  {
    QWidget *central = new QWidget();
    central->setLayout(pMainLayout);
    this->setCentralWidget(central);
  }
}

MainWindow::~MainWindow()
{
}
