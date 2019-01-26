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
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/8, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX), DrawGraph::DC_OFF, 0.5f, -1.0f);
    pDraw->setPostMask(DPostmask(DPostmask::PM_LINETOP, DPostmask::PO_SIGNAL, 1, 1.0f,1.0f,1.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
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
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/16, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX), DrawGraph::DC_DOWNBASE);
    pDraw->setPostMask(DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_SIGNAL, 0, 1.0f,1.0f,1.0f));
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
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
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX, graphopts_t::DE_TRIANGLE), DrawGraph::DC_OFF);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_peaks);
    pLayout->addWidget(pDraw);
  }
  {
    /// 12
    DrawQWidget* pDraw = new DrawGraph(COUNT_SAMPLES/4, COUNT_PORTIONS, graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX, graphopts_t::DE_CENTER), DrawGraph::DC_DOWNBASE);
    pDraw->setDataPalette(&palette_gnu_latte);
    pDraw->setData(arr_pseudonormal);
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

