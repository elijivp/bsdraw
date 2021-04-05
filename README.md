## BSDRAW
Source code for 4 principal types of graphs, drawed by fragment and vertex shaders.

* Technology: Qt widgets, inherits QOpenGLWidget class.
* Shaders: generated and compiled after initializeGl stage.
* Compatibility: tested on qt4.8, qt5.5, qt5.12 (win/linux). GLSL version 1.30+
* Features: fast, const size, cross-platform, universal.
* Note: main define called BSGLSLVER prepends each shader with string "#version %BSGLSLVER%". 
All shaders in bsdraw are compatible with glsl 130, but by default BSGLSLVER is not set. So
if you have any issues, add DEFINES+=BSGLSLVER=130 in your .pro file.

### Getting started with simple_example_1D_with_scales:
```
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
```
Result:
![simple_example_1D_with_scales.png](/demoimages/simple_example_1D_with_scales.png)

### Getting started with simple_example_2D:
```
#include <QApplication>

#include "bsdrawintensity.h"      // INCLUDEPATH includes bsdraw dir in .pro file
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
  DrawQWidget* pDraw = new DrawIntensity(COUNT_SAMPLES, COUNT_ROWS, 1, OR_LRBT);  // LRBT - left-to-right, bottom-to-top
  pDraw->setDataPalette(&paletteBkBlWh);  // dont forget to set palette. default palette is null.
  pDraw->setScalingLimitsSynced(4); // each sample is now 4 pixels
  pDraw->setData(arr_random);     // default draw bounds are 0..1. default data is 0.
  pDraw->show();
  
  return a.exec();
}
```
Result:
![simple_example_2D.png](/demoimages/simple_example_2D.png)

### Getting started with simple_example_domain:
```
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
```
Result:
![simple_example_domain.png](/demoimages/simple_example_domain.png)

### One more example:
```
DrawQWidget* pDraw = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_LINTEPR, 0x00333333, 1, 0.5f), DrawGraph::CP_OWNRANGE);
pDraw->setDataPalette(&palette_gnu_latte);
pDraw->setData((const float*)data);
setCentralWidget(pDraw);
```
will be created simple graph with fixed count of SAMPLES. Total data waited on setData method = SAMPLES*PORTIONS,
disposed SAMPLES(1st portion), SAMPLES(2nd portion)...

![simple](/demoimages/simple.png)

### Usage
It is difficult to describe all the characteristics, so:
1. Build&Run example from __example__ folder
2. Choose test, remember _testname_
3. Draw creation: find section called _testname_ in MainWindow constructor
4. Obvious methods in MainWindow class: generateData, changePalette
5. Overlays creation: find section called _buttonname_ in MainWindow method createOverlaySTD

![demo1](/demoimages/demo1.png)

Left to right, up to down: 
* DrawIntensity, DrawDomain, DrawRecorder, 
* DrawGraph(GT_DOTS), DrawGraph(GT_LINTERP), DrawGraph(GT_HISTOGRAM_CROSSMAX),
* DrawGraph(GT_DOTS, dotsize=1, weight=0.5), DrawGraph(GT_LINTERP, dotsize=1, weight=0.5), DrawGraph(GT_HISTOGRAM_CROSSMAX, dotsize=1, weight=0.5)

### Scaling
That technology is designed to solve 2 problems: resize and posteffects

![demo2](/demoimages/demo2.png)

Left to right, up to down (SCALING=4, Postmask(PM_LINELEFTBOTTOM)): 
* DrawIntensity, DrawDomain, DrawRecorder, 
* DrawGraph(GT_DOTS), DrawGraph(GT_LINTERP), DrawGraph(GT_HISTOGRAM_CROSSMAX),
* DrawGraph(GT_DOTS, Postmask(PM_PSEUDOCIRCLE)), DrawGraph(GT_LINTERP, Postmask(PM_LINELEFT)), DrawGraph(GT_HISTOGRAM_CROSSMAX, Postmask(PM_LINELEFT))

### Overlays
Fragment shaders compiled like additional functions and mixed with drawed data.
```
for (unsigned int i=0; i<drawscount; i++)
{
  QImage  img(img_path_normal);
  IOverlay* ovl = new OImageStretched(&img, IOverlaySimpleImage::IC_AUTO, false);
  ovl->setSlice(i != 1? 0.25f : 0.0f);
  draws[i]->ovlPushBack(ovl);
  draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
  draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
  draws[i]->ovlPushBack(new OGridDecart(CR_RELATIVE, 0.05, 0.5, 0.1, 0.1, 3));
}
```

![overlays](/demoimages/overlays.png)

### Vertical draws released
```
ORIENTATION orients[] = { OR_LRBT, OR_TBLR, OR_BTRL, OR_BTLR, OR_TBRL, OR_RLTB };
for (unsigned int i=0; i<drawscount; i++)
{
  draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp2(DE_NONE, 0x00111111), DrawGraph::CP_SINGLE, 1.0, -0.5);
  draws[i]->setOrientation(orients[i]);
}
```

![vertical](/demoimages/vertical.png)

#### INCLUDES (without overlays):
1. Required core files:
bsdraw/core/bsqdraw.cpp;
bsdraw/core/bsqdraw.h;
bsdraw/core/sheigen/bsshgenmain.cpp;
bsdraw/core/sheigen/bsshgenmain.h;
bsdraw/core/bsdraw.h;
bsdraw/core/bsidrawcore.h

2. Draw type you need:
bsdraw/bsdrawgraph.cpp;
bsdraw/bsdrawgraph.h;
bsdraw/bsdrawintensity.cpp;
bsdraw/bsdrawintensity.h;
bsdraw/bsdrawrecorder.cpp;
bsdraw/bsdrawrecorder.h;
bsdraw/bsdrawdomain.cpp;
bsdraw/bsdrawdomain.h

3. For palettes (headers only):
bsdraw/palettes/bspalettes_adv.h;
bsdraw/palettes/bspalettes_rgb.h;
bsdraw/palettes/bspalettes_std.h

#### INCLUDES for overlays:
bsdraw/core/bsoverlay.cpp;
bsdraw/core/sheigen/bsshgencolor.cpp;
bsdraw/core/sheigen/bsshgenmain.cpp;
bsdraw/core/sheigen/bsshgentrace.cpp;
bsdraw/overlays/bsborder.cpp & .h;
bsdraw/overlays/bsfigures.cpp & .h;
bsdraw/overlays/bsgrid.cpp & .h;
bsdraw/overlays/bspoints.cpp & .h;
bsdraw/overlays/bssprites.cpp & .h;
bsdraw/overlays/bstextstatic.cpp & .h;
bsdraw/overlays/special/bsmarks.cpp & .h;
bsdraw/overlays/special/bsblocker.cpp & .h;
bsdraw/overlays/bsinteractive.cpp & .h;
bsdraw/overlays/bscontour.cpp & .h;
bsdraw/overlays/bsimage.cpp & .h
    
#### Future:
* texture atlas for text
* axes with text marking
