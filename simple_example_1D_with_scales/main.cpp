#include <QApplication>

#include "bsdrawgraph.h"    // INCLUDEPATH includes bsdraw dir in .pro file
#include "palettes/bspalettes_std.h"
#include "bsdrawscales.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  /// Data generation
  const unsigned int COUNT_SAMPLES = 128;
  const unsigned int COUNT_PORTIONS = 1;     /// graphs on draw
  float arr_random[COUNT_SAMPLES*COUNT_PORTIONS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_PORTIONS; i++)
    arr_random[i] = rand()/float(RAND_MAX);  /// default data diapason 0..1
  
  /// Drawing
  DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goInterp(0.5f, DE_LINTERP), coloropts_t::copts());
  pDraw->setDataPalette(&paletteBkBlWh);  // dont forget to set palette. Default palette is null
  pDraw->setScalingLimitsHorz(4);   // 'space' occupied by one sample. Default is 1
  pDraw->setMinimumHeight(100);
  pDraw->setData(arr_random);     // default draw bounds are 0..1
  
  DrawBars* pDB = new DrawBars(pDraw, DrawBars::CP_FROM_DRAWPALETTE);
  pDB->addContour(AT_TOP, 0);   // horizontal line, serving as a delimiter, if you need it
  pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED_POW2, 0, 1000.0f, 20, 36);
  pDB->addScaleDrawGraphB(AT_LEFT, DBMODE_STRETCHED_POW2, 20, 20);
  pDB->show();  // resize me :)
  
  return a.exec();
}
