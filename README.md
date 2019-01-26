## BSDRAW
Source code for 4 principal types of graphs, drawed by fragment and vertex shaders.

* Technology: Qt widgets, inherits QOpenGLWidget class.
* Shaders: generated and compiled after initializeGl stage.
* Compatibility: tested on qt4.8, qt5.5, qt5.10 (win/linux). GLSL version 1.30+
* Features: fast, const size, cross-platform, universal.
* Note: main define called BSGLSLVER prepends each shader with string "#version %BSGLSLVER%". 
All shaders in bsdraw are compatible with glsl 130, but by default BSGLSLVER is not set. So
if you have any issues, add DEFINES+=BSGLSLVER=130 in your .pro file.

### Simple example:
```
DrawQWidget* pDraw = new DrawGraph(SAMPLES, PORTIONS, graphopts_t(graphopts_t::GT_LINTERP, 0.0f, 0x77777777, 1, 0.5f), DrawGraph::DC_DOWNBASE);
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
* DrawGraph(GT_DOTS), DrawGraph(GT_LINTERP), DrawGraph(GT_LINDOWN_CROSSMAX),
* DrawGraph(GT_DOTS, dotsize=1, weight=0.5), DrawGraph(GT_LINTERP, dotsize=1, weight=0.5), DrawGraph(GT_LINDOWN_CROSSMAX, dotsize=1, weight=0.5)

### Scaling
That technology is designed to solve 2 problems: resize and posteffects

![demo2](/demoimages/demo2.png)

Left to right, up to down (SCALING=4, Postmask(PM_LINELEFTBOTTOM)): 
* DrawIntensity, DrawDomain, DrawRecorder, 
* DrawGraph(GT_DOTS), DrawGraph(GT_LINTERP), DrawGraph(GT_LINDOWN_CROSSMAX),
* DrawGraph(GT_DOTS, Postmask(PM_PSEUDOCIRCLE)), DrawGraph(GT_LINTERP, Postmask(PM_LINELEFT)), DrawGraph(GT_LINDOWN_CROSSMAX, Postmask(PM_LINELEFT))

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
  draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t(graphopts_t::GT_LINTERPSMOOTH, 0.0, 0x00111111), DrawGraph::DC_OFF, 1.0, -0.5);
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
* vertical draws: graph and recorder
* texture atlas for text
* axes with text marking
