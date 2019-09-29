## BSDRAW QUICKSTART

This quickstart example you can find on __simpleexample__ project in corresponding folder

![quickstart](/demoimages/quickstart.png)

First, you must include _bsdrawgraph.h_ for DrawGraph (inherits DrawQWidget) class and _palettes/bspalettes_adv.h_ for palettes

```
  const unsigned int COUNT_SAMPLES = 256;
  const unsigned int COUNT_PORTIONS = 1;
  float arr_random[COUNT_SAMPLES*COUNT_PORTIONS]; 	// init somewhere
  float arr_sin[COUNT_SAMPLES*COUNT_PORTIONS];		// init somewhere
  float arr_peaks[COUNT_SAMPLES*COUNT_PORTIONS];	// init somewhere
  float arr_pseudonormal[COUNT_SAMPLES*COUNT_PORTIONS];	// init somewhere
    
  {
    /// 1 row 1 column
    unsigned int scaling = 8;
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/scaling, COUNT_PORTIONS, graphopts_t::goInterp(0.2f, DE_LINTERP), DrawGraph::CP_SINGLE);
    pDraw->setScalingLimitsH(scaling);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2 row 1 column
    unsigned int scaling = 8;
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/scaling, COUNT_PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP, 0xFFFFFFFF, 4, 1.0f), DrawGraph::CP_SINGLE);
    pDraw->setScalingLimitsH(scaling);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 1 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goDots(2, 0.5f, 0xFFFFFFFF), DrawGraph::CP_SINGLE, 1.0f, 0.5f);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 1 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/8, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(), DrawGraph::CP_OWNRANGE_SYMMETRIC, 1.0f, 0.2f);
    pDraw->setScalingLimitsH(8);
    pDraw->setScalingLimitsV(8);
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINETOP, 1, 1.0f,1.0f,1.0f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_ALL, DPostmask::PM_CONTOUR, 0, 0x00333333, 0.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  ///////////////////////////////////////////////
  pLayout = new QVBoxLayout();
  pMainLayout->addLayout(pLayout);
  {
    /// 1 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/2, COUNT_PORTIONS, graphopts_t::goInterp(0.0f, DE_CENTER), DrawGraph::CP_SINGLE, 0.9f, 0.1f);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots(), DrawGraph::CP_RANGE);
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINERIGHTBOTTOM, 0, 0.6f,0.6f,0.6f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_EMPTY, DPostmask::PM_DOT, 0, 0.6f,0.6f,0.6f));
    pDraw->setScalingLimitsSynced(4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots(), DrawGraph::CP_SINGLE);
    pDraw->setPostMask(DPostmask(DPostmask::PO_EMPTY, DPostmask::PM_LINEBOTTOM, 0, 0.2f,0.2f,0.2f));
    pDraw->setScalingLimitsH(4);
    pDraw->setScalingLimitsV(4);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 2 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMin(), DrawGraph::CP_SINGLE, 1.0f, 0.5f);
    pDraw->setScalingLimitsH(4);
    pDraw->setScalingLimitsV(3);
//    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINETOP, 1, 1.0f,1.0f,1.0f));
    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_CONTOUR, 0, 0x00333333, 0.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  ///////////////////////////////////////////////
  {
    /// 1 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES, COUNT_PORTIONS, graphopts_t::goInterp(0.0f, DE_NONE), DrawGraph::CP_RANGE);
    pDraw->setDataPalette(&palette_gnu_PuYe);
    pDraw->setData(arr_pseudonormal);
    pLayout->addWidget(pDraw);
  }
  {
    /// 2 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goDots(0, 0.0f, 0xFFFFFFFF, DE_CENTER), DrawGraph::CP_RANGE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_random);
    pLayout->addWidget(pDraw);
  }
  {
    /// 3 row 3 column
//    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(0.0f, DE_CENTER), DrawGraph::CP_SINGLE, 0.5f, 1.0f);
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(PR_SUMMARY, DE_NONE), DrawGraph::CP_SINGLE, 0.5f, 1.0f);
    pDraw->setPostMask(DPostmask(DPostmask::PO_SIGNAL, DPostmask::PM_LINELEFTBOTTOM, 0, 0.0f));
    pDraw->setScalingLimitsV(5);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 4 row 3 column
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t::goHistogramCrossMax(0.0f, DE_TRIANGLE), DrawGraph::CP_SINGLE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
```

### Minimal required includes:

```
SOURCES += 
    ../bsdrawgraph.cpp \
    ../bsdrawintensity.cpp \
    ../core/bsqdraw.cpp \
    ../core/sheigen/bsshgenmain.cpp

HEADERS  +=
    ../bsdrawgraph.h \
    ../core/bsdraw.h \
    ../core/bsidrawcore.h \
    ../core/bsqdraw.h \
    ../core/sheigen/bsshgenmain.h
```

