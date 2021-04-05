#include <QApplication>

#include "bsdrawdomain.h"      // INCLUDEPATH includes bsdraw dir in .pro file
#include "palettes/bspalettes_std.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  /// Data generation
  const unsigned int COUNT_SAMPLES = 80;
  const unsigned int COUNT_ROWS = 40;
  float arr_random[COUNT_SAMPLES*COUNT_ROWS];
  for (unsigned int i=0; i<COUNT_SAMPLES*COUNT_ROWS; i++)
    arr_random[i] = rand()/float(RAND_MAX);  /// default data diapason 0..1
  
  /// Drawing
  DrawDomain* pDraw = new DrawDomain(COUNT_SAMPLES, COUNT_ROWS, 1, OR_LRBT);  // LRBT - left-to-right, bottom-to-top
  DIDomain* dd = pDraw->domain();
  for (int i=0; i<COUNT_ROWS/2; i++)
  {
    dd->start();
    dd->includeRect(i, i, COUNT_SAMPLES - 2*i, COUNT_ROWS - 2*i);   /// rectangles inside
    dd->finish();
  }
  
  pDraw->setDataPalette(&paletteBkWh);  // dont forget to set palette. default palette is null.
  pDraw->setScalingLimitsSynced(4); // each sample is now 4 pixels
  pDraw->setData(arr_random);     // default draw bounds are 0..1. default data is 0.
  pDraw->show();
  
  return a.exec();
}
