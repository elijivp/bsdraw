#include <QApplication>

#include "bsdrawintensity.h"      // INCLUDEPATH includes bsdraw dir in .pro file
#include "palettes/bspalettes_std.h"

static const char* g_numbers[] = {
#include "numbers.inc"
};



int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  tftgeterbook_t*   g_tftnumbers;
  {
    QFont fntValues(QApplication::font());
    fntValues.setPointSize(12);
    fntValues.setBold(true);
    fntValues.setItalic(false);
    g_tftnumbers = tftgeterbook_alloc(fntValues, 6, 16, Qt::white);
  }
//  const unsigned int NUMCOUNT = sizeof(g_numbers)/sizeof(const char*);
  const unsigned int NUMCOUNT = sizeof(g_numbers)/sizeof(const char*);
  tftgeterbook_addtexts(g_tftnumbers, NUMCOUNT, g_numbers);
  tftgetermetrics_t  METRICS = tftgeterbook_metrics(g_tftnumbers);
  
  const unsigned int NUMHORZ = 20;
  const unsigned int NUMVERT = 20;
  
  /// Data generation
  const unsigned int PIXCOLS = NUMHORZ*METRICS.design_width;
  const unsigned int PIXROWS = NUMVERT*METRICS.design_height;
  float* arr_random = new float[PIXROWS*PIXCOLS];
  memset(arr_random, 0, PIXROWS*PIXCOLS*sizeof(float));
  const int CPR = 3;
  for (unsigned int r=1; r<PIXROWS; r += 3)
  {
    for (unsigned int i=0; i<CPR; i++)
    {
      int c = rand()%(PIXCOLS/CPR);
      arr_random[r*PIXCOLS + i*CPR + c] = 1.0f;
    }
  }
  
  /// Drawing
  DrawQWidget* pDraw = new DrawIntensity(PIXROWS, PIXCOLS, 1, OR_TBLR);
  pDraw->tftEnableDynamicClosestMode();
  Q_ASSERT(pDraw->tftHoldingRegister(g_tftnumbers, true) != -1);
  pDraw->setDataPalette(&paletteBkBlWh);
  pDraw->setData(arr_random);
  delete []arr_random;
  
  {
    const int BASIC = NUMCOUNT/2-1;
//    const int BASIC = 1900;
    for (int i=0; i<NUMHORZ; i++)
    {
      for (int j=0; j<NUMVERT; j++)
      {
//        pDraw->tftPushDynamicFA(BASIC + i*NUMVERT + j, CR_ABSOLUTE_NOSCALED, i*METRICS.design_width + METRICS.design_width/2, j*METRICS.design_height + METRICS.design_height/2);
        pDraw->tftPushDynamicRT(BASIC + i*NUMVERT + j, i*METRICS.design_width + METRICS.design_width/2, j*METRICS.design_height + METRICS.design_height/2);
      }
    }
  }
  
  pDraw->show();
  
  return a.exec();
}

