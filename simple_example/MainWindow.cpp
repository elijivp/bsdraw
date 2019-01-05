#include "MainWindow.h"

#include "../bsdrawgraph.h"
#include "../bsdrawintensity.h"
#include "../palettes/bspalettes_adv.h"
#include <stdlib.h>   // for rand() function

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  const unsigned int samples = 256;
//  pDraw = new DrawGraph(samples);
  pDraw = new DrawGraph(samples, 1, graphopts_t(graphopts_t::GT_LINTERP, 0.0f, 0x77777777, 1, 0.5f), DrawGraph::DC_DOWNBASE);
//  pDraw = new DrawIntensity(samples/16, 16);
  
  pDraw->setDataPalette(&palette_gnu_latte);
  
  {
    float data[samples];
    for (unsigned int i=0; i<samples; i++)
      data[i] = rand()/float(RAND_MAX);
    pDraw->setData(data);
  }
  this->setCentralWidget(pDraw);
}

MainWindow::~MainWindow()
{
  delete pDraw;
}
