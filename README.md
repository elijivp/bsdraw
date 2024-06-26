# BSDRAW
Source code for 4 principal types of graphs, drawed by fragment and vertex shaders.

* Technology: Qt widgets, inherits QOpenGLWidget (Qt>=5) or QGLWidget (Qt<5) class.
* Shaders: generated as source code and compiled after initializeGl stage on first show.
* Compatibility: tested on qt4.8, qt5.5, qt5.12, qt5.14(win/linux), qt6.3(linux). GLSL version 1.30+
* Features: fast, cross-platform, universal.
* Note: main define called BSGLSLVER prepends each shader with string "#version %BSGLSLVER%". 
All shaders in bsdraw are compatible with glsl 130, but by default BSGLSLVER is not set. So
if you have any issues, add DEFINES+=BSGLSLVER=130 in your .pro file.
All examples are taken from the project "example".

## Overview

__DrawIntensity__ class allows you to create rectangular 2D draws. Inherits DrawQWidget

![overview_2d_1.png](/demoimages/overview_2d_1.png)

This example is supplemented with overpattern feature - shader posteffect over drawed data.

Mechanics: _2d data converts into 2d texture, palette converts into 2d texture; Palette texture applies to data texture, shader overpattern applies to result_

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    LINES = 14;
    SAMPLES = 20;
    PORTIONS = 1;
    
    overpattern_t dpms[] = {  
    // row 1
    overpattern_off(),
    overpattern_thrs_minus(OP_CONTOUR, 0.5f, 0.0f, 3),
    overpattern_any(OP_CONTOUR, 0.0f, 3),
    
    // row 2
    overpattern_any(OP_LINELEFTBOTTOM, 0.0f),
    overpattern_any(OP_DOTLEFTBOTTOM, 1.0f, 3),
    overpattern_any(OP_LINELEFTBOTTOM, 0.4f, 2),
    
    // row 3
    overpattern_any(OP_LINEBOTTOM, 0.0f, 7),
    overpattern_any(OP_SQUARES, 0.0f),
    overpattern_any(OP_DOTCONTOUR, 0.0f, 3),
    
    // row 4
    overpattern_any(OP_CIRCLEBORDERED2, 0.0f, -20),
    overpattern_any(OP_CIRCLESMOOTH, 0.0f, 4),
    overpattern_thrs_plus(OP_CIRCLESMOOTH, 0.9f, 0.0f),
    
    // row 5
    overpattern_thrs_plus(OP_FILL, 0.8f, 0.0f),
    overpattern_any(OP_DOT, 0.0f, 4),
    overpattern_thrs_minus(OP_SHTRICHL, 0.3f, color3f(0.5f, 0.5f, 0.5f), 1),
    
    // row 6
    overpattern_any(OP_CONTOUR, color3f(0.0f, 0.0f, 0.0f),1),
    overpattern_any(OP_CONTOUR, color3f(0.3f, 0.3f, 0.3f),1),
    overpattern_any(OP_CONTOUR, color3f(1.0f, 1.0f, 1.0f),1)
    };
    
    const int countROWS = 6, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    for (unsigned int r=0; r<countROWS; r++)
      for (unsigned int c=0; c<countCOLUMNS; c++)
      {
        pdraws[r][c] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
        pdraws[r][c]->setOverpattern(dpms[r*countCOLUMNS + c]);
        pdraws[r][c]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
      }
    // adding text as overlay for first draw
    pdraws[0][0]->ovlPushBack(new OTextColored("Original", CR_XABS_YREL_NOSCALED, 5.0f, 0.9f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [LINES x SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>

__DrawDomain__ class allows you to create rectangular 2D draws with regions of different size. One value on setData() method fills one region. 
Inherits DrawQWidget

![overview_2d_2.png](/demoimages/overview_2d_2.png)

All draws above have different regions but takes similar data.

Mechanics: _full 2d field with region markup converts into 2d texture, region data converts into 1d texture, palette converts into 2d texture; Palette texture applies to region data texture, which applies to field texture_

<details><summary>Code snippet (see end of the page for includes)</summary><p>
 

    LINES = 200;
    SAMPLES = 200;
    PORTIONS = 1;
    
    const int countROWS = 2, countCOLUMNS = 2;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    // 1st row, domain 1: little crosses
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int crossRows = SAMPLES/10;
      const int crossColumns = LINES/10;
      for (int i=0; i<crossRows; i++)
      {
        for (int j=0; j<crossColumns; j++)
        {
          int r = LINES/crossRows/2 + i*LINES/crossRows;
          int c = int(SAMPLES/crossColumns/2 + j*SAMPLES/crossColumns);
          ddm.start();
          ddm.includePixel(r-1, c);
          ddm.includePixel(r, c);
          ddm.includePixel(r+1, c);
          ddm.includePixel(r, c+1);
          ddm.includePixel(r, c-1);
          ddm.finish();
        }
      }
      pdraws[0][0] = dd;
      pdraws[0][0]->setScalingLimitsSynced(2); // 1 point now is 2x2 pixels (minimum)
    }
    // 1st row, domain 2: chessboard
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int cc = 20;
      for (int r2=0; r2<LINES/cc/2; r2++)
      {
        for (int j=0; j<SAMPLES; j+=cc)
        {
          int r0 = r2*2*cc + ((j / cc) % 2)*cc;
          int rs[] = { r0, r0, r0+cc/2, r0+cc/2 };
          int cs[] = { j, j+cc/2, j, j+cc/2 };
          for (int i=0; i<4; i++)
          {
            ddm.start();
            for (int r=rs[i]; r<rs[i]+cc/2; r++)
              for (int c=cs[i]; c<cs[i]+cc/2; c++)
                ddm.includePixel(r, c);
            ddm.finish();
          }
        }
      }
      pdraws[0][1] = dd;
      pdraws[0][1]->setScalingLimitsSynced(2); // 1 point now is 2x2 pixels (minimum)
    }
    // 2nd row, domain 1: spiral
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = SAMPLES*6;
      const unsigned int outsider = 18500;
      const double wc = 0.15/(2.0*M_PI);
      for (int i=0; i<maxspiral; i++)
      {
        int y = qRound(LINES/2.0 + outsider*sin((i+1)*wc)/(i+1));
        int x = qRound(SAMPLES/2.0 + outsider*cos((i+1)*wc)/(i+1));
        if (y >= 0 && y < LINES && x >= 0 && x < SAMPLES)
        {
          ddm.start();
            ddm.includePixelFree(y, x);
          ddm.finish();
        }
      }
      pdraws[1][0] = dd;
      pdraws[1][0]->setScalingLimitsSynced(2); // 1 point now is 2x2 pixels (minimum)
    }
    // 2nd row, domain 2: multispiral
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = 600;
      const unsigned int outsider = 9000;
      const double wc = 5.0/(2.0*M_PI);
      for (int i=0; i<maxspiral; i++)
      {
        int y = qRound(LINES/2.0 + outsider*sin((i+1)*wc)/(i+1)), x = qRound(SAMPLES/2.0 + outsider*cos((i+1)*wc)/(i+1));
        if (y >= 0 && y < LINES && x >= 0 && x < SAMPLES && ddm.isFree(y, x))
        {
          ddm.start();
            ddm.includePixel(y, x);
          ddm.finish();
        }
      }
      pdraws[1][1] = dd;
      pdraws[1][1]->setScalingLimitsSynced(2); // 1 point now is 2x2 pixels (minimum)
    }
    
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [LINES x SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>

Portions its about how many draws/graphs painted in one draw. 2 or more portions in 2D draw means colormeshing (mixing) data in each pixel

![overview_2d_3.png](/demoimages/overview_2d_3.png)

At the bottom of picture __DrawRecorder__ class allows you to create 2D draws from 1D data. 
Each setData() call appends 1D data array to current 2D image as new line. Inherits DrawQWidget

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    LINES = 120;
    SAMPLES = 400;
    PORTIONS = 3;
    
    const int countDraws= 3;
    DrawQWidget* pdraws[countDraws];
    pdraws[0] = new DrawIntensity(SAMPLES, LINES/PORTIONS, PORTIONS, OR_LRBT, SP_COLUMN_TB_COLORSPLIT);
    pdraws[0]->setDataPalette(&paletteRGB);
    pdraws[0]->setDataPaletteDiscretion(true);
    pdraws[1] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
    pdraws[1]->setDataPalette(&paletteRGB);
    pdraws[2] = new DrawRecorder(SAMPLES, LINES, 1000, PORTIONS);
    pdraws[2]->setDataPalette(&paletteRGB);
    
    for (int i=0; i<countDraws; i++)
      this->layout()->addWidget(pdraws[i]);
    
    const float* data = someGeneratedData; // at least [LINES x SAMPLES x PORTIONS] of floats
    for (int i=0; i<countDraws; i++)
      pdraws[i]->setData(data);

</p></details>


__DrawGraph__ class allows you to create 1D graphs and histograms. Inherits DrawQWidget

Histograms with 3 portions displayed below

![overview_1d_histogram_1.png](/demoimages/overview_1d_histogram_1.png)

histograms above differ in the ordering of data draw. Histogram bar separation created by overpattern

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 80;
    PORTIONS = 3;
    
    const int countDraws = 4;
    DrawQWidget* pdraws[countDraws];
    
    pdraws[0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_VALUEAROUND));
    pdraws[0]->setOverpattern(overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)));
    pdraws[0]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
    
    pdraws[1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMin(PR_VALUEAROUND));
    pdraws[1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)));
    pdraws[1]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
    
    pdraws[2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(PR_VALUEAROUND));
    pdraws[2]->setOverpattern(overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)));
    pdraws[2]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
    
    pdraws[3] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramMesh(PR_VALUEAROUND));
    pdraws[3]->setOverpattern(overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)));
    pdraws[3]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
    
    for (int i=0; i<countDraws; i++)
      this->layout()->addWidget(pdraws[i]);
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int i=0; i<countDraws; i++)
      pdraws[i]->setData(data);

</p></details>

More histograms with same data and different overpatterns

![overview_1d_histogram_2.png](/demoimages/overview_1d_histogram_2.png)

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 24;
    PORTIONS = 1;
    
    const int countROWS = 3, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    
    // 1st row
    pdraws[0][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_STANDARD) );
    pdraws[0][0]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, // hex color is explicit color (black)
                                                       0));
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_STANDARD) );
    pdraws[0][1]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, 2));
    
    pdraws[0][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_STANDARD) );
    pdraws[0][2]->setOverpattern(overpattern_thrs_plus(OP_LINETOP, 0.0f, // float color is palette depending color
                                                       0.0f, 4));
    
    // 2nd row
    pdraws[1][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(0.0f, DE_TRIANGLE, 0.0f, PR_VALUEAROUND) );
    pdraws[1][0]->setOverpattern(overpattern_any(OP_CONTOUR, 0.0f));
    
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_VALUEAROUND) );
    pdraws[1][1]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, 2));
    
    pdraws[1][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_NONE) );
    pdraws[1][2]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, 1));
    
    // 3rd row
    pdraws[2][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_VALUEONLY) );
    pdraws[2][0]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, 1));
    
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_SUMMARY) );
    pdraws[2][1]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, 2));
    
    pdraws[2][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(PR_VALUEONLY) );
    pdraws[2][2]->setOverpattern(overpattern_any(OP_LINELEFTTOP, 0x00333333, 1));
    
    for (int r=0; r<countROWS; r++)
      for (int c=0; c<countCOLUMNS; c++)
      {
        pdraws[r][c]->setScalingLimitsHorz(12); 
        pdraws[r][c]->setScalingLimitsVert(8);    // 1 point now is 12x8 pixels (minimum)
      }
    
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>


__DrawGraph__ class supports 3 principal types of graph: histogram, dots and interpolated graph. Interpolation is done on shader

1st column is for 1 portion of data, 2nd is for 2 portions

![overview_1d_1.png](/demoimages/overview_1d_1.png)

Up-to-down: dots (dotsize 0), linear interpolation (dotsize 0), dots (dotsize > 0), linear interpolation (dotsize > 0), histogram

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 300;
    PORTIONS = 2;
    
    const int countROWS = 5, countCOLUMNS = 2;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    pdraws[0][0] = new DrawGraph(SAMPLES, 1, graphopts_t::goDots(DE_NONE, 0, 0.5f));
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE, 0, 0.5f));
    
    pdraws[1][0] = new DrawGraph(SAMPLES, 1, graphopts_t::goInterp(DE_NONE, 0, 0.5f));
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_NONE, 0, 0.5f));
    
    pdraws[2][0] = new DrawGraph(SAMPLES, 1, graphopts_t::goDots(DE_NONE, 3, 0.5f));
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE, 3, 0.5f));
    
    pdraws[3][0] = new DrawGraph(SAMPLES, 1, graphopts_t::goInterp(DE_NONE, 4, 0.2f));
    pdraws[3][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_NONE, 4, 0.2f));
    
    pdraws[4][0] = new DrawGraph(SAMPLES, 1, graphopts_t::goHistogramMesh(DE_NONE));
    pdraws[4][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramMesh(DE_NONE));
    
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>


In addition to linear interpolation there is an interpolation on 3 points and 4 points. 
Different types of interpolation and a composition of drawing graph and scaling graph are shown in the picture below

![overview_1d_2.png](/demoimages/overview_1d_2.png)

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 54;
    PORTIONS = 2;
    
    const int countDraws = 7;
    DrawQWidget* pdraws[countDraws];
    const float smoothcoeff = 0.2f;
    
    pdraws[0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(4, 0.8f));
    pdraws[0]->ovlPushBack(new OTextColored("DE_NONE, ORIGINAL DATA DOTS", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    pdraws[1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothcoeff, DE_LINTERP_SCALINGLEFT, 4, 0.8f));
    pdraws[1]->ovlPushBack(new OTextColored("DE_LINETERP_SCALINGLEFT", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    pdraws[2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothcoeff, DE_LINTERP_SCALINGCENTER, 4, 0.8f));
    pdraws[2]->ovlPushBack(new OTextColored("DE_LINETERP_SCALINGCENTER", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    pdraws[3] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp2(smoothcoeff, DE_LINTERP, 4, 0.8f));
    pdraws[3]->ovlPushBack(new OTextColored("DE_LINETERP #2 (glsl' smoothstep)", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[4] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothcoeff, DE_SINTERP, 4, 0.8f));
    pdraws[4]->ovlPushBack(new OTextColored("DE_SINETERP", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[5] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothcoeff, DE_QINTERP, 4, 0.8f));
    pdraws[5]->ovlPushBack(new OTextColored("DE_QINETERP", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[6] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(-1.0f, DE_LINTERP, 4, 0.8f));
    pdraws[6]->ovlPushBack(new OTextColored("\"8bit\"-interp", CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    for (int i=0; i<countDraws; i++)
      this->layout()->addWidget(pdraws[i]);
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int i=0; i<countDraws; i++)
      pdraws[i]->setData(data);

</p></details>

Default palette can be changed by setDataPalette() method

And finally, graphopts_t struct adjust type of graph while coloropts_t struct adjust graph coloring. 

![overview_1d_3.png](/demoimages/overview_1d_3.png)

Mechanics: _data for multiple portions converts into 2d texture, palette converts into 2d texture; Palette texture applies to data texture in accordance with the color policy_

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 100;
    PORTIONS = 4;
    
    const int countROWS = 7, countCOLUMNS = 2;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    const float label_xpos = 0.5f;
    
    pdraws[0][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_MONO, 0.0f, 1.0f));
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_MONO, 0.0f, 1.0f));
    pdraws[0][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[1][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED, 0.0f, 1.0f));
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED, 0.0f, 1.0f));
    pdraws[1][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[2][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED_GROSS, 0.0f, 1.0f));
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED_GROSS, 0.0f, 1.0f));
    pdraws[2][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[3][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED_SYMMETRIC, 0.0f, 1.0f));
    pdraws[3][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED_SYMMETRIC, 0.0f, 1.0f));
    pdraws[3][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[4][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f));
    pdraws[4][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f));
    pdraws[4][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[5][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PALETTE, 0.0f, 1.0f));
    pdraws[5][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PALETTE, 0.0f, 1.0f));
    pdraws[5][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[6][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PALETTE_SPLIT, 0.0f, 1.0f));
    pdraws[6][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PALETTE_SPLIT, 0.0f, 1.0f));
    pdraws[6][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>


## Additional Features

#### IMPULSE
Impulse feature for 2d draws - special shader code which smoothes border between scaled data

![overview_2d_impulse.png](/demoimages/overview_2d_impulse.png)

In example above each 2d draw has size 1 'row' and 5 'columns' what means 1x5 pixels for minimum size. 
Vertical scaling is manually set to 50+ and window enlarged in both sides, so real size is more than 300x50 pixels.
Impulse applies for data who appears, when you resize your 2d draw


<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    LINES = 1;
    SAMPLES = 5;
    PORTIONS = 1;
    
    const int countROWS = 5, countCOLUMNS = 2;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    impulsedata_t imp[] = { { impulsedata_t::IR_OFF },
                            { impulsedata_t::IR_A_COEFF, 5, 5/2, 0, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_A_COEFF_NOSCALED, 5, 5/2, 0, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 30, 4, 0, {} },
                            { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 30, 8, 0, {} },
                            { impulsedata_t::IR_A_BORDERS, 30, 4, 0, {0.0f} },
                            { impulsedata_t::IR_A_BORDERS, 30, 4, 0, {0.2f} },
                            { impulsedata_t::IR_A_BORDERS, 30, 8, 0, {0.4f} },
                            { impulsedata_t::IR_A_BORDERS, 30, 4, 0, {0.8f} },
                            { impulsedata_t::IR_A_BORDERS, 30, 4, 0, {1.0f} },
    };
    const char* cpnames[] = { "ORIGINAL", "COEFF", "COEFF_NOSCALED", "BORDERS_FIXED", "BORDERS_FIXED2",
                              "BORDERS c0.0f", "BORDERS c0.2f", "BORDERS c0.4f", "BORDERS c0.8f", "BORDERS c1.0f"
                            };
    
    for (unsigned int r=0; r<countROWS; r++)
      for (unsigned int c=0; c<countCOLUMNS; c++)
      {
        pdraws[r][c] = new DrawIntensity(SAMPLES, LINES, 1);
        pdraws[r][c]->setImpulse(imp[r*countCOLUMNS + c]);
        pdraws[r][c]->setScalingLimitsB(50); // our single 2d-row now takes at least 50 pixels
        pdraws[r][c]->ovlPushBack(new OTextColored(otextopts_t(cpnames[r*countCOLUMNS + c], 0, 10,2,10,2), CR_RELATIVE, 0.05f, 0.05f, 8, 0x00000000, 0x11FFFFFF, 0x00000000));
      }
    for (int c=0; c<countCOLUMN; c++)
    {
      QVBoxLayout* layV = new QVBoxLayout;
      for (int r=0; r<countROW; r++)
        layV->addWidget(pdraws[r][c]);
      currentHBoxLayout->addLayout(layV);
    }
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int c=0; c<countCOLUMN; c++)
      for (int r=0; r<countROW; r++)
        pdraws[r][c]->setData(data);

</p></details>

#### SMOOTHING

Different smoothing for 1d linterp graph

![overview_1d_smooth.png](/demoimages/overview_1d_smooth.png)

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 280;
    PORTIONS = 3;
    
    const int countROWS = 3, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    float smoothtest[countROWS][countCOLUMNS] = 
          { 
            { -1.0f, -0.3f, 0.0f },
            { 0.2f,  0.3f,  0.4f },
            { 0.6f,  0.8f,  1.0f }
          };
    
    for (unsigned int r=0; r<countROWS; r++)
      for (unsigned int c=0; c<countCOLUMNS; c++)
        pdraws[r][c] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothtest[r][c], DE_LINTERP));
      
    ... and same deploying and data applying like in other examples

</p></details>

#### ORIENTATION

Orientation for any type of draw can be set via setOrientation() method

![extra_orients.png](/demoimages/extra_orients.png)

#### PALETTES

Palettes for any type of draw can be set via setDataPalette() method

![extra_palettes.png](/demoimages/extra_palettes.png)

Simple palettes are collected in one array _ppalettes_std[]_ i.e. paletteBkWh (black-white), paletteRdWh, paletteBkBlWh, etc; 
full list of simple palettes available in "bspalettes_std.h"

There exists trivial palettes with 1 color i.e. paletteBk, paletteWh etc

Advanced palettes are collected in one array _ppalettes_adv[]_; full list of advanced palettes available in "bspalettes_adv.h"

Special palettes available in "bspalettes_spec.h"


<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 50;
    PORTIONS = 2;
    
    const int countROWS = 3, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    pdraws[0][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE, 3, 0.5f));
    pdraws[0][0]->setDataPalette(&paletteBkWh);
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE, 3, 0.5f));
    pdraws[0][1]->setDataPalette(ppalettes_adv[3]);
    pdraws[0][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE, 3, 0.5f));
    pdraws[0][2]->setDataPalette(ppalettes_adv_inv[3]);
    pdraws[0][2]->ovlPushBack(new OTextColored("Inverted palette", CR_RELATIVE, 0.25f, 0.05f,
                                           12, 0x00000000, 0xFFFFFFFF, 0x00000000), OO_AREA_LRBT);
    
    pdraws[1][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP));
    pdraws[1][0]->setDataPalette(ppalettes_adv[18]);
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP));
    pdraws[1][1]->setDataPalette(ppalettes_adv[2]);
    pdraws[1][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP));
    pdraws[1][2]->setDataPalette(ppalettes_adv_inv[2]);
    pdraws[1][2]->ovlPushBack(new OTextColored("Inverted palette", CR_RELATIVE, 0.25f, 0.05f,
                                           12, 0x00000000, 0xFFFFFFFF, 0x00000000), OO_AREA_LRBT);
    
    pdraws[2][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramMesh(DE_NONE));
    pdraws[2][0]->setDataPalette(ppalettes_adv[52]);
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramMesh(DE_NONE));
    pdraws[2][1]->setDataPalette(ppalettes_adv[59]);
    pdraws[2][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramMesh(DE_NONE));
    pdraws[2][2]->setDataPalette(ppalettes_adv[75]);
    
    ... and same deploying and data applying like in other examples
    
</p></details>

__DrawMoveEx__ inherits DrawGraph. This type of graph saves previous data and setData() method appends data to graph

![extra_graphs_move.png](/demoimages/extra_graphs_move.png)

There is one more additional opportunity: background color of draw can be installed independently of palette

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 300;
    PORTIONS = 2;
    
    const int countROWS = 3, countCOLUMNS = 3;
    graphopts_t  gopts[countROWS][countCOLUMNS] =
    {
      {
        graphopts_t::goHistogram(0.4f, DE_NONE, 0.5f),
        graphopts_t::goHistogram(0.3f, DE_NONE, 0.5f),
        graphopts_t::goHistogram(0.4f, DE_NONE, 0.5f),
      },
      
      {
        graphopts_t::goInterp(0.4f, DE_NONE, 0, 0.0f),
        graphopts_t::goInterp(0.4f, DE_NONE, 0, 0.0f),
        graphopts_t::goInterp(0.4f, DE_NONE, 0, 0.0f),
      },
      
      {
        graphopts_t::goDots(2, 0.5f, DE_NONE),
        graphopts_t::goDots(2, 0.5f, DE_NONE),
        graphopts_t::goDots(2, 0.5f, DE_NONE)
      }
    };
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    const unsigned int HISTORY_LENGTH = SAMPLES*4;
    
    for (unsigned int r=0; r<countROWS; r++)
      for (unsigned int c=0; c<countCOLUMNS; c++)
      {
        coloropts_t copts = {   c == 2? CP_REPAINTED : CP_MONO, 0.0f, 1.0f, c == 1? 0xFFFFFFFF : 0x00999999 };
        pdraws[r][c] = new DrawGraphMoveEx(SAMPLES, 5, HISTORY_LENGTH, PORTIONS, 
                                                   gopts[r][c], 
                                                   copts);
      }
    
    ... and same deploying and data applying like in other examples

</p></details>

### SCALING (visual explanation)

Graph shader combines 2 mechanisms:

1. type of graph (histogram, dots or interpolated graph), who describes how data values connects with each other

2. type of descaling, who describes how stretched (when you resize draw) data value disintegrates into subvalues and connects with each other

![extra_scaling.png](/demoimages/extra_scaling.png)

first row:

draw00 has interpolation between data points and interpolation in horizontal scaling

draw01 has interpolation between data points and nothing in horizontal scaling - data values just repeats when you stretch draw width

draw02 has interpolation between data points and only central points in horz scaling


second row is the same like first, difference is vertical scaling. By default vertical scaling is set to 1 for draw, but you can change it


third row:

draw20 has nothing between data points and interpolation in horizontal scaling

draw21 has nothing between data points and nothing in horizontal scaling - data values just repeats when you stretch draw width

draw22 has nothing between data points and only central points in horz scaling


fourth row is the same like third, difference is vertical scaling

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 50;
    PORTIONS = 1;
    
    const int countROWS = 4, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    const BSDESCALING third_de = DE_TRIANGLE2;
    pdraws[0][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP));
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_NONE));
    pdraws[0][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(third_de));
    
    pdraws[1][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP));
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_NONE));
    pdraws[1][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(third_de));
    for (int c=0; c<countCOLUMNS; c++)
      pdraws[1][c]->setScalingLimitsB(8);
    
    pdraws[2][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_LINTERP));
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE));
    pdraws[2][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(third_de));
    
    pdraws[3][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_LINTERP));
    pdraws[3][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(DE_NONE));
    pdraws[3][2] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(third_de));
    for (int c=0; c<countCOLUMNS; c++)
      pdraws[3][c]->setScalingLimitsB(8);
    
    ... and same deploying and data applying like in other examples

</p></details>


## Draw Bars

__DrawBars__ class constructs with pointer to DrawQWidget object. 
DrawBars incapusulates DrawQWidget functionality and adds 4 (left, top, right, bottom) bars (areas) for drawing axes, lines or labels. 
Drawing is done with QPainter and QStaticText.

![overview_bars_1.png](/demoimages/overview_bars_1.png)

DrawBars takes colors from parent QPalette or DrawQWidget palette

DrawBars counts bar sizes and controls DrawQWidget resize

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 180;
    PORTIONS = 1;
    
    const int countDraws = 4;
    DrawQWidget* pdraws[countDraws];
    for (unsigned int i=0; i<countDraws; i++)
      pdraws[i] = new DrawGraph(SAMPLES, PORTIONS, 
                                graphopts_t::goInterp(0.65f, DE_QINTERP), 
                                coloropts_t::copts(CP_MONO, 0.0f, 1.0f, i == 2? 0x00AAAAAA : 0xFFFFFFFF));
    
    DrawBars* pdrawbars[countDraws];
    pdrawbars[0] = new DrawBars(pdraws[0], DrawBars::CP_DEFAULT);
    pdrawbars[0]->addEScalePixstepDrawbounds(AT_TOP, DBF_ENUMERATE_FROMZERO | DBF_ENUMERATE_SHOWLAST, 20);
    pdrawbars[0]->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 21, 32);
    pdrawbars[0]->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, "colors: default", Qt::AlignCenter, Qt::Vertical);
    pdrawbars[0]->addLabel(AT_TOP, 0, "Enumerator scale", Qt::AlignCenter, Qt::Horizontal);
    
    pdrawbars[1] = new DrawBars(pdraws[1], DrawBars::CP_FROM_DRAWBACK);
    {
      pdrawbars[1]->addContour(AT_LEFT, 0);
      pdrawbars[1]->addContour(AT_TOP, 0);
      pdrawbars[1]->addContour(AT_BOTTOM, 0);
      pdrawbars[1]->addContour(AT_RIGHT, 0);
    }
    pdrawbars[1]->addScalePixstepOwnbounds(AT_TOP, 0, 0.0, SAMPLES-1, SAMPLES);
    pdrawbars[1]->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 21, 32);
    pdrawbars[1]->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, "colors: drawback", Qt::AlignCenter, Qt::Vertical);
    pdrawbars[1]->addLabel(AT_TOP, 0, "Fixed scale, floating marks", Qt::AlignCenter, Qt::Horizontal);
    
    pdrawbars[2] = new DrawBars(pdraws[2], DrawBars::CP_FROM_DRAWBACK);
    {
      pdrawbars[2]->addContour(AT_LEFT, 0);
      pdrawbars[2]->addContour(AT_TOP, 0);
      pdrawbars[2]->addContour(AT_BOTTOM, 0);
      pdrawbars[2]->addContour(AT_RIGHT, 0);
    }
    pdrawbars[2]->addScaleRollingTapNM(AT_TOP, 0, standard_tap_symbolate<-1>, 4, nullptr, SAMPLES, 20);
    pdrawbars[2]->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 21, 32);
    pdrawbars[2]->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, "colors: drawback", Qt::AlignCenter, Qt::Vertical);
    pdrawbars[2]->addLabel(AT_TOP, 0, "Tap by alphabet", Qt::AlignCenter, Qt::Horizontal);
    
    pdrawbars[3] = new DrawBars(pdraws[3], DrawBars::CP_FROM_DRAWPALETTE_INV);
    pdrawbars[3]->addScaleSymmetricOwnbounds(AT_TOP, DBF_NOTE_BORDERS_ONLY | DBF_NOTESINSIDE, 0.0, 1.0, SAMPLES, 10);
    pdrawbars[3]->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 21, 32);
    pdrawbars[3]->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, "colors: inversed", Qt::AlignCenter, Qt::Vertical);
    pdrawbars[3]->addLabel(AT_TOP, 0, "Fixed minimal scale", Qt::AlignCenter, Qt::Horizontal);
    
    for (int i=0; i<countDraws; i++)
    {
      MEWPointer* mpH = pdrawbars[i]->addEPointer01Auto(AT_BOTTOM, DBF_NOTESINSIDE | DBF_ENUMERATE_FROMZERO, 0.0f);
      int oapH = pdrawbars[i]->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpH->createReactor()));
      pdrawbars[i]->getDraw()->ovlPushBack(new OFLine(OFLine::LT_VERT_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapH);
      
      MEWPointer* mpV = pdrawbars[i]->addPointerRelativeDrawbounds(AT_RIGHT, DBF_NOTESINSIDE, 0.0f);
      int oapV = pdrawbars[i]->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpV->createReactor()));
      pdrawbars[i]->getDraw()->ovlPushBack(new OFLine(OFLine::LT_HORZ_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapV);
    }
    
    for (int i=0; i<countDraws; i++)
      this->layout()->addWidget(pdrawbars[i]);
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int i=0; i<countDraws; i++)
      pdraws[i]->setData(data);

</p></details>


DrawBars class supports 3 principal types of scale on axis: 

1 - smart 'native' scale. Marks placement depends of step and data bounds

2 - symmetric scale. Marks are placed so that the resizing of DrawBars adds new marks precisely between existing marks

3 - pixstep scale. Marks are placed so that the largest number of marks can fit on the scale

![overview_bars_2.png](/demoimages/overview_bars_2.png)

Unit of measurement can be placed for last mark' note (1st draw) or for each mark' note (2nd draw) 

Explicit precision can be setted up for notes (3rd draw)

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 50;
    PORTIONS = 1;
    
    const int countDraws = 4;
    DrawQWidget* pdraws[countDraws];
    for (unsigned int i=0; i<countDraws; i++)
      pdraws[i] = new DrawGraph(SAMPLES, PORTIONS, 
                                graphopts_t::goInterp(0.45f, DE_LINTERP, 3, 0.85f), 
                                coloropts_t::copts(CP_MONO, 0.0f, 1.0f, i == 3? 0xFFFFFFFF : 0x00777777));
    
    int dbflags = 0;
    DrawBars* pdrawbars[countDraws];
    pdrawbars[0] = new DrawBars(pdraws[0], DrawBars::CP_DEFAULT);
    pdrawbars[0]->addScaleNativeOwnbounds(AT_TOP, dbflags | DBF_POSTFIX_ONLYLAST, 0.0f, 720.0f, 15.0f, 0.0f, 32, 40, 9, 0.0f, "°");
    pdrawbars[0]->addLabel(AT_TOP, 0, "'Native' scale, mod off", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[0]->addScaleNativeOwnbounds(AT_LEFT, dbflags, 0.0f, 1.0f, 0.25f, 0.0f, 32, 40);
    pdrawbars[0]->addScaleNativeOwnbounds(AT_BOTTOM, dbflags | DBF_POSTFIX_ONLYLAST, 0.0f, 720.0f, 15.0f, 0.0f, 32, 40, 9, 360.0f, "°");
    pdrawbars[0]->addLabel(AT_BOTTOM, 0, "'Native' scale, mod 360.0f", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[0]->addSpace(AT_BOTTOM, 16);
    {
      pdrawbars[0]->getDraw()->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, 
                                                            CR_RELATIVE, 0.0f, 60.0f/360.0f, 
                                                            linestyle_black(0,1,1)));
    }
    
    pdrawbars[1] = new DrawBars(pdraws[1], DrawBars::CP_DEFAULT);
    pdrawbars[1]->addScaleSymmetricOwnbounds(AT_TOP, dbflags, 0.0f, 720.0f, 32, 50, 9, 0.0f, "°");
    pdrawbars[1]->addLabel(AT_TOP, 0, "Symmetric scale, mod off", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[1]->addScaleSymmetricOwnbounds(AT_LEFT, dbflags, 0.0f, 1.0f, 16, 32);
    pdrawbars[1]->addScaleSymmetricOwnbounds(AT_BOTTOM, dbflags, 0.0f, 720.0f, 32, 50, 9, 360.0f, "°");
    pdrawbars[1]->addLabel(AT_BOTTOM, 0, "Symmetric scale, mod 360.0f", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[1]->addSpace(AT_BOTTOM, 16);
    {
      pdrawbars[1]->getDraw()->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, 
                                                            CR_RELATIVE, 0.0f, 1.0f/8, 
                                                            linestyle_black(0,1,1), -1));
    }
    
    pdrawbars[2] = new DrawBars(pdraws[2], DrawBars::CP_DEFAULT);
    pdrawbars[2]->addScalePixstepOwnbounds(AT_TOP, dbflags | DBF_PRECISION_EXACT_2, 0.0f, 720.0f, 32, 50, 9, 0.0f);
    pdrawbars[2]->addLabel(AT_TOP, 0, "Pixstep scale, mod off", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[2]->addScalePixstepOwnbounds(AT_LEFT, dbflags, 0.0f, 1.0f, 16, 32);
    pdrawbars[2]->addScalePixstepOwnbounds(AT_BOTTOM, dbflags | DBF_PRECISION_EXACT_2, 0.0f, 720.0f, 32, 50, 9, 360.0f);
    pdrawbars[2]->addLabel(AT_BOTTOM, 0, "Pixstep scale, mod 360.0f", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[2]->addSpace(AT_BOTTOM, 16);
    
    
    pdrawbars[3] = new DrawBars(pdraws[3], DrawBars::CP_DEFAULT);
    pdrawbars[3]->addEScalePixstepOwnbounds(AT_TOP, dbflags, 32, 40, 1);
    pdrawbars[3]->addLabel(AT_TOP, 0, "Enumerator scale", Qt::AlignCenter, Qt::Horizontal);
    pdrawbars[3]->addScaleNativeOwnbounds(AT_LEFT, dbflags, 0.0f, 1.0f, 0.25f, 0.0f, 16, 32);
    pdrawbars[3]->addEScaleRollingOwnbounds(AT_BOTTOM, dbflags, 32, 40, 1);
    pdrawbars[3]->addLabel(AT_BOTTOM, 0, "Rolling Enumerator scale", Qt::AlignCenter, Qt::Horizontal);
    {
      pdrawbars[3]->getDraw()->ovlPushBack(new OGridCells(4, 12, linestyle_yellow(0,1,1)));
    }
    
    for (int i=0; i<countDraws; i++)
      this->layout()->addWidget(pdrawbars[i]);
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    for (int i=0; i<countDraws; i++)
      pdraws[i]->setData(data);

</p></details>

DrawBars class supports pointers: 

![overview_bars_3.png](/demoimages/overview_bars_3.png)

Pointer is a more complex element. He can be set up as nonmovable on his place but normally we attach pointer to user actions (clics or moves on draw).
The following scheme is used there: we create special overlay object, who follow user action. We attach to him visual overlay (cross, dot, etc).
And we attach to him our pointer. Example below

<details><summary>Code snippet (see end of the page for includes)</summary><p>
  

    SAMPLES = 20;
    PORTIONS = 2;
    
    DrawQWidget* pdraw = new DrawGraph(SAMPLES, PORTIONS, 
                                        graphopts_t::goInterp(0.45f, DE_LINTERP), 
                                        coloropts_t::copts(CP_MONO, 0.0f, 0.75f));
    // 2 overlays just for effects
    {
      pdraw->ovlPushBack(new OGridCells(24, 16, linestyle_white(0,1,1)));
      pdraw->ovlGet(1)->setOpacity(0.9f);
      pdraw->ovlPushBack(new OShadow(10,10,10,10, 0.75f, color3f_white()));
    }
    
    DrawBars* pdrawbars = new DrawBars(pdraw, DrawBars::CP_DEFAULT);
    pdrawbars->addScaleSymmetricEmpty(AT_RIGHT, 0, 32, 20, 4);
    
    // 4 pointers for all 4 sides, attached to one overlay
    {
      MEWPointer* mpHL = pdrawbars->addPointerAbsoluteDrawbounds(AT_LEFT, DBF_NOTESINSIDE, 0.5f, 6, 0.0f, "°");
      MEWPointer* mpHR = pdrawbars->addPointerAbsoluteDrawbounds(AT_RIGHT,  DBF_NOTESINSIDE, 0.5f, 0, 0.0f, "°");
      MEWPointer* mpVT = pdrawbars->addPointerAbsoluteDrawbounds(AT_TOP, DBF_NOTESINSIDE, 0.5f, 6, 0.0f, "s");
      MEWPointer* mpVB = pdrawbars->addPointerAbsoluteDrawbounds(AT_BOTTOM, DBF_NOTESINSIDE, 0.5f, 4, 0.0f, "s");
      OActiveCursorCarrier4* oac = new OActiveCursorCarrier4(mpHL->createReactor(), mpHR->createReactor(), mpVB->createReactor(), mpVT->createReactor());
      int oap = pdrawbars->getDraw()->ovlPushBack(oac);
      pdrawbars->getDraw()->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_red(1,0,0)), oap);
    }
    
    pdrawbars->addSpace(AT_BOTTOM, 8);
    pdrawbars->addContour(AT_BOTTOM);
    pdrawbars->addScaleNativeOwnbounds(AT_BOTTOM, 0, 0.0f, 1.0f, 0.1f, 0.0f, 32, 40, 9);
    
    
    this->layout()->addWidget(pdrawbars);
    
    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    pdraw->setData(data);

</p></details>


## TFTs

__TFT__ is a method to add titles on draw. Deploy text record on special implicit texture called holding, so you can use text multiple times. 
Add static or dynamic titles, linked with text record. You can move and swap titles later. You also can rotate static and dynamic titles.

![overview_tfts_1.png](/demoimages/overview_tfts_1.png)

![overview_tfts_2.png](/demoimages/overview_tfts_2.png)

<details><summary>Code snippet</summary><p>
    SAMPLES = 20;
    PORTIONS = 4;
    
    DrawQWidget* pdraw = new DrawGraph(SAMPLES, PORTIONS, 
                                        graphopts_t::goInterp(0.45f, DE_LINTERP), 
                                        coloropts_t::copts(CP_MONO, 0.0f, 0.75f, 0x00666666));
    { /// Static titles
      QFont fnt(this->font());
      fnt.setPointSize(12);
      fnt.setItalic(true);
      pdraw->tftHoldingRegister(fnt, 32, 1);  // 1. Register holding
      pdraw->tftAddRecord("Hello world!");    // 2. Add record
      pdraw->tftAddRecord("Click me");
      pdraw->tftPushStatic(0, CR_RELATIVE, 0.48f, 0.52f);   // 3. Deploy title
      pdraw->tftPushStatic(1, CR_RELATIVE, 0.47f, 0.47f);
    }
    
    { /// Dynamic records
      QFont fnt(this->font());
      fnt.setPointSize(10);
      fnt.setBold(true);
      pdraw->tftHoldingRegister(fnt, 6, 8);   // auto switching on new holding
      
      ovl_visir = pdraw->ovlPushBack(new OActiveCursor(CR_RELATIVE, 0.5f, 0.5f));
      char buffer[32];
      for (int i=0; i<360; i++)
      {
        sprintf(buffer, "%d°", i);
        pdraw->tftAddRecord(buffer);
      }
      int TOT = 12*4;
      for (int i=0; i<TOT; i++)
      {
        float pr = i/float(TOT)*M_PI*2.0f;
        pdraw->tftPushDynamicFA(i*360/TOT, CR_RELATIVE, 0.3f*sin(pr), 0.3f*cos(pr), ovl_visir);
      }
      
      QTimer* tm = new QTimer();
      tm->setSingleShot(false);
      tm->setInterval(30);
      QObject::connect(tm, &QTimer::timeout, [=]()
      { 
        for (int j=0; j<pdraw->tftDynamicsCount(); j++)
          pdraw->tftSwitchTo(j, (pdraw->tftRecordIndex(j) + 1) % pdraw->tftRecordsCount());
      } );
      QTimer::singleShot(2000, tm, SLOT(start()));
    }
        
    DrawBars* pdrawbars = new DrawBars(pdraw, DrawBars::CP_DEFAULT);
    ATTACHED_TO atto[] = { AT_LEFT, AT_RIGHT, AT_TOP, AT_BOTTOM };
    for (unsigned int i=0; i<sizeof(atto)/sizeof(ATTACHED_TO); i++)
    {
      pdrawbars->addScaleSymmetricEmpty(atto[i], 0, 32, 20, 4);
      pdrawbars->addSpace(atto[i], 16);
    }
    
    this->layout()->addWidget(pdrawbars);

    const float* data = someGeneratedData; // at least [SAMPLES x PORTIONS] of floats
    pdraw->setData(data);

</p></details>





## Examples Usage

1. Build&Run example from __example__ folder
2. Choose test and press Accept button
3. Additional tests available in folders: __simple_example__, __simple_example_1D__, __simple_example_1D_with_scales__,
__simple_example_2D__, __simple_example_2D_with_scales__, __simple_example_domain__

#### Generated shader example

<details><summary>Generated shader snippet for 2D draw (DrawIntensity + overpattern)</summary><p>
  

    #version 130
    uniform highp sampler2D  texData;
    uniform highp int        viewdimm_a;
    uniform highp int        viewdimm_b;
    uniform highp int        scaler_a;
    uniform highp int        scaler_b;
    uniform highp int        countPortions;
    uniform highp sampler2D  texPalette;
    uniform highp vec2       palrange;
    in highp vec2            coords;
    float getValue1D(in int portion, in float x){  return texture(texData, vec2(x, float(portion)/(float(countPortions)-1.0))).r; }
    float getValue2D(in int portion, in vec2  x){  return texture(texData, vec2(x.x, float(x.y + float(portion))/float(countPortions))).r; }
    vec3  insider(int i, ivec2 ifromvec) { float scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));
      return vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }
    void main()
    {
      float mixwell = 1.0;
      float dvalue = 0.0;
      vec4  ovl_cur_otss;
      ivec2 ovl_transfer_pos;
      vec2  ovl_cur_coords;
      ivec2 ab_indimms = ivec2(viewdimm_a, viewdimm_b);
      ivec2 ab_iscaler = ivec2(scaler_a, scaler_b);
      ivec2 ab_ibounds = ab_indimms*ab_iscaler;
      vec2  xy_coords = vec2(coords.xy*0.5 + vec2(0.5,0.5));
      vec2  ab_coords = xy_coords;             
      vec2  abc_coords = ab_coords;
      ivec2 ispcell = ivec2(0,0);
      ivec2 icells = ivec2(1,1);
      vec3  backcolor = texture(texPalette, vec2(0.0, 0.0)).rgb;
      vec3  result = vec3(0.0, 0.0, 0.0);
      vec4  post_mask = vec4(0.0, 0.0, 1.0, 1.0);
      ivec2  immod = ivec2( int(mod(abc_coords.x*ab_ibounds.x, float(ab_iscaler.x))), int(mod(abc_coords.y*ab_ibounds.y, float(ab_iscaler.y))));
      ivec4  imrect  = ivec4(immod.x, immod.y, ab_iscaler.x-1, ab_iscaler.y-1);
      const int allocatedPortions = 1;
      for (int i=0; i<countPortions; i++)
      {
        float value = texture(texData, vec2(abc_coords.x, float(abc_coords.y + float(i))/float(countPortions))).r;
        dvalue = max(dvalue, value);
        value = palrange[0] + (palrange[1] - palrange[0])*value;
        result = result + texture(texPalette, vec2(value, float(i)/(allocatedPortions-1) )).rgb;
        post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));
      }
      float ppb_in = sign(step(imrect.x, post_mask[2]) + step(imrect.y, post_mask[2]) + step(imrect[2] - imrect.x, post_mask[2]) + step(imrect[3] - imrect.y, post_mask[2]));
      vec3   ppb_color = vec3(1.0,1.0,1.0);
      result = mix(result, ppb_color, ppb_in * 1.0 );
      vec4 ovTrace;
      gl_FragColor = vec4(result, 0.0);
    }
    

</p></details>

<details><summary>Generated shader snippet for 1D draw (DrawGraph + 2 overlays)</summary><p>
  

    #version 130
    uniform highp sampler2D  texData;
    uniform highp int        viewdimm_a;
    uniform highp int        viewdimm_b;
    uniform highp int        scaler_a;
    uniform highp int        scaler_b;
    uniform highp int        countPortions;
    uniform highp sampler2D  texPalette;
    uniform highp vec2       palrange;
    in highp vec2            coords;
    float getValue1D(in int portion, in float x){  return texture(texData, vec2(x, float(portion)/(float(countPortions)-1.0))).r; }
    float getValue2D(in int portion, in vec2  x){  return texture(texData, vec2(x.x, float(x.y + float(portion))/float(countPortions))).r; }
    vec3  insider(int i, ivec2 ifromvec) { float scaled01 = float(i - ifromvec[0])/float(ifromvec[1] - sign(float(ifromvec[1])));
      return vec3( step(0.0, scaled01)*(1.0-step(1.001, scaled01)), scaled01, sign(ifromvec[1])*ifromvec[1]); }
    uniform highp vec4 ovl_otss_001;
    vec4 overlayOVCoords1(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 shapeself);
    vec3 overlayColor1(in vec4 in_variant, in vec3 color);
    uniform highp vec4 ovl_otss_002;
    vec4 overlayOVCoords2(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 shapeself);
    vec3 overlayColor2(in vec4 in_variant, in vec3 color);
    void main()
    {
      float mixwell = 1.0;
      float dvalue = 0.0;
      vec4  ovl_cur_otss;
      ivec2 ovl_transfer_pos;
      vec2  ovl_cur_coords;
      ivec2 ab_indimms = ivec2(viewdimm_a, viewdimm_b);
      ivec2 ab_iscaler = ivec2(scaler_a, scaler_b);
      ivec2 ab_ibounds = ab_indimms*ab_iscaler;
      vec2  xy_coords = vec2(coords.xy*0.5 + vec2(0.5,0.5));
      vec2  ab_coords = xy_coords;             
      vec2  abc_coords = ab_coords;
      ivec2 ispcell = ivec2(0,0);
      ivec2 icells = ivec2(1,1);
      vec3  backcolor = texture(texPalette, vec2(0.0, 0.0)).rgb;
      vec3  result = backcolor;
      vec4  post_mask = vec4(0.0, 0.0, 0.0, 1.0);
      ivec2  immod = ivec2( int(mod(abc_coords.x*ab_ibounds.x, float(ab_iscaler.x))), int(mod(abc_coords.y*ab_ibounds.y, float(ab_iscaler.y))));
      ivec4  imrect  = ivec4(immod.x, immod.y, ab_iscaler.x-1, ab_iscaler.y-1);
      mixwell = 0.0;const float specopc = 1.0;
      vec2  ab_fndimms = vec2(viewdimm_a, viewdimm_b);
      vec2  ab_fbounds = vec2(ab_ibounds);
      float b_coord = abc_coords.y*ab_fbounds.y;
      float b_coord_ns = floor(abc_coords.y*ab_fndimms.y);
      const float specsmooth = 0.450000;
      vec3 fx = vec3(float(max(abc_coords.x*ab_indimms.x, 1) - 1)/ab_indimms.x, abc_coords.x, float(min(abc_coords.x*ab_indimms.x, ab_indimms.x-1)  + 1)/ab_indimms.x);
      for (int i=0; i<countPortions; i++)
      {
        vec3  fy = vec3(getValue1D(i, fx[0]), getValue1D(i, fx[1]), getValue1D(i, fx[2]));
        vec3  fy_ns = floor(fy*ab_indimms.y);
        vec3  fy_view = fy*ab_ibounds.y;
        ivec3 iy_view = ivec3(floor(fy_view));
        vec3 fds = vec3(immod.x/float(ab_iscaler.x)) + vec3(-1.0/ab_iscaler.x, 0.0, 1.0/ab_iscaler.x);
        fy_view = vec3(
            mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.x, iy_view[1] + (iy_view[2] - iy_view[1])*fds.x, step(0.0, fds.x)),
            mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.y, iy_view[1] + (iy_view[2] - iy_view[1])*fds.y, step(0.0, fds.y)),
            mix(iy_view[1] + (iy_view[1] - iy_view[0])*fds.z, iy_view[1] + (iy_view[2] - iy_view[1])*fds.z, step(0.0, fds.z))
            );
        iy_view = ivec3(floor(fy_view));
        fy_ns = floor(fy_view/(ab_iscaler.y));
        float fmix_self = abs(b_coord_ns - fy_ns[1]);
        fmix_self = (1.0-fmix_self + fmix_self*(0.25+specsmooth*0.375))*step(fmix_self, 2.0);
        float fsig_prev = sign(fy_ns[0] - fy_ns[1]);
        float fsig_next = sign(fy_ns[2] - fy_ns[1]);
        float fmix_prev = fy_ns[0] + fsig_prev;
        float fmix_next = fy_ns[2] + fsig_next;
        fmix_prev = (fmix_prev - b_coord_ns)/(fmix_prev-fy_ns[1]);
        fmix_next = (fmix_next - b_coord_ns)/(fmix_next-fy_ns[1]);
        fmix_prev = fmix_prev*step(0.0, fmix_prev)*(1.0 - step(1.0, fmix_prev));
        fmix_next = fmix_next*step(0.0, fmix_next)*(1.0 - step(1.0, fmix_next));
        float fmix_rej = step(1.0, fsig_prev*fsig_next)*0.25;
        fmix_prev = fmix_prev + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_prev-0.5)*(fmix_prev-0.5));
        fmix_next = fmix_next + (specsmooth-fmix_rej)*2.0*(0.25-(fmix_next-0.5)*(fmix_next-0.5));
        fmix_prev = mix(fmix_prev, 0.2, (1.0-step(fmix_prev, 0.0))*step(fmix_prev, 0.2));
        fmix_next = mix(fmix_next, 0.2, (1.0-step(fmix_next, 0.0))*step(fmix_next, 0.2));
        fmix_prev = mix(fmix_prev, 1.0, step(1.0, fmix_prev));
        fmix_next = mix(fmix_next, 1.0, step(1.0, fmix_next));
        vec3 fhit = vec3(0.0, step(1.0, fmix_self), 0.0);
        float mixwellp =  max(fhit.y, specopc*max(max(fmix_prev, fmix_next), fmix_self ));
        fhit.x = 1.0 - step(mixwellp, 0.0);
        float VALCLR = b_coord_ns / ab_fndimms.y;
        post_mask[0] = post_mask[0]*(1.0 - (fhit.x + fhit.z)) + (fhit.x + fhit.z);
        const int allocatedPortions = 2;
        float porc = palrange[1] - (palrange[1] - palrange[0])/float(allocatedPortions)*(allocatedPortions - 1 - i);
        vec3  colorGraph = texture(texPalette, vec2(porc, 0.0)).rgb;
        result = mix(result, colorGraph, mixwellp);
        mixwell = max(mixwell, mixwellp);
        dvalue = mix(fy[1], dvalue, step(abs(b_coord - dvalue), abs(b_coord - fy[1])) );
      }
      
      float ppb_in = 0.0;
      vec4 ovTrace;
      ovl_cur_otss = ovl_otss_001;
      ovl_transfer_pos = ivec2(0,0);
      ovl_cur_coords = abc_coords;
      bool ovl_visible_1 = step(1.0, ovl_cur_otss[0]) != 1;
      if  (ovl_visible_1)
      {
        ovTrace = overlayOVCoords1(ispcell, ab_indimms.xy, ab_iscaler.xy, ab_ibounds, ovl_cur_coords, ovl_cur_otss[1], ivec2(0,0), vec3(post_mask[0], post_mask[3], ppb_in), ovl_transfer_pos);
        if (sign(ovTrace[3]) != 0.0 && (step(mixwell, 0.0) == 1 || (step(dvalue, ovl_cur_otss[2]) == 0 && step(ovl_cur_otss[3], dvalue) == 0)) )
        result = mix(result, overlayColor1(ovTrace, result), 1.0 - ovl_cur_otss[0]);
      }
      ivec2 ovl_pos_1 = ovl_transfer_pos;
      ovl_cur_otss = ovl_otss_002;
      ovl_transfer_pos = ivec2(0,0);
      ovl_cur_coords = abc_coords;
      bool ovl_visible_2 = step(1.0, ovl_cur_otss[0]) != 1;
      if  (ovl_visible_2)
      {
        ovTrace = overlayOVCoords2(ispcell, ab_indimms.xy, ab_iscaler.xy, ab_ibounds, ovl_cur_coords, ovl_cur_otss[1], ivec2(0,0), vec3(post_mask[0], post_mask[3], ppb_in), ovl_transfer_pos);
        if (sign(ovTrace[3]) != 0.0 && (step(mixwell, 0.0) == 1 || (step(dvalue, ovl_cur_otss[2]) == 0 && step(ovl_cur_otss[3], dvalue) == 0)) )
        result = mix(result, overlayColor2(ovTrace, result), 1.0 - ovl_cur_otss[0]);
      }
      ivec2 ovl_pos_2 = ovl_transfer_pos;
      gl_FragColor = vec4(result, 0.0);
    }
        
    #version 130
    vec3 insider(int i, ivec2 ifromvec);
    vec4 overlayOVCoords1(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 selfposition)
    {
      ivec2 icoords = ivec2(coords*ov_ibounds);
      vec3 result = vec3(0.0);
      float mixwell = 0.0;
      vec2 _mvar;  float _fvar;
      vec2 _tracepass = vec2(1.0,1.0);
      ivec2 ioffset = ivec2(0,0);
      ioffset = ioffset + mastercoords;
      ivec2 inormed = icoords - ioffset;
      ivec2 cr = ivec2(16, 24);
      vec2   cellsizepix = vec2(float(ov_ibounds.x-1) / cr.x, float(ov_ibounds.y-1) / cr.y);ivec2  optiid = ivec2(inormed.x / cellsizepix.x + 0.49, inormed.y / cellsizepix.y + 0.49);ioffset = ivec2(cellsizepix.x*optiid.x, 0);inormed = icoords - ioffset;_mvar[0] = 1.0 * (1.0+thick - clamp(abs(inormed[0] - floor(float(0.0))), 0.0, 1.0+thick))/(1.0+thick);_mvar[1] = inormed[1] - 0;
      _mvar[0] = _mvar[0] * step(0.0, _mvar[1]);
      result = mix(result, vec3(_mvar*_tracepass, 0.0), 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );
      ioffset = ivec2(0, cellsizepix.y*optiid.y);inormed = icoords - ioffset;_mvar[0] = 1.0 * (1.0+thick - clamp(abs(inormed[1] - floor(float(0.0))), 0.0, 1.0+thick))/(1.0+thick);_mvar[1] = inormed[0] - 0;
      _mvar[0] = _mvar[0] * step(0.0, _mvar[1]);
      result = mix(result, vec3(_mvar*_tracepass, 0.0), 1.0 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );
      mixwell = result[0];
      selfposition = ioffset; 
      return vec4(result, mixwell);
    }
    
    #version 130
    vec3 overlayColor1(in vec4 in_variant, in vec3 undercolor) 
    {
      vec3 result;
      float mixwell = 0.0;
      mixwell = in_variant[0];
      const int lenspace = 1;
      const int countdot = 1;
      int pos = int(mod(in_variant[1], float(2*countdot - 1 + lenspace)));
      int mixdot = int((1.0 - step(2.0*countdot, float(pos)))*step(1.0, mod(pos - (2.0*countdot-1.0), 2.0)));
      mixwell = mixwell*mixdot;
      result = vec3(1.0,1.0,1.0);
      return mix(undercolor, result, mixwell);
    }
    
    #version 130
    vec3 insider(int i, ivec2 ifromvec);
    vec4 overlayOVCoords2(in ivec2 ispcell, in ivec2 ov_indimms, in ivec2 ov_iscaler, in ivec2 ov_ibounds, in vec2 coords, in float thick, in ivec2 mastercoords, in vec3 post_in, out ivec2 selfposition)
    {
      ivec2 icoords = ivec2(coords*ov_ibounds);
      vec3 result = vec3(0.0);
      float mixwell = 0.0;
      vec2 _mvar;  float _fvar;
      vec2 _tracepass = vec2(1.0,1.0);
      ivec2 ioffset = ivec2(0,0);
      ioffset = ioffset + mastercoords;
      const float weight = 0.750000;
      ivec2 inormed = icoords - ioffset;
      ivec2 dd = ivec2(9999, 0);dd = (ivec2)mix(dd, ivec2(inormed.x, 9), step(float(inormed.x), float(dd[0])));dd = (ivec2)mix(dd, ivec2(ov_ibounds.y - 1 - inormed.y, 9), step(float(ov_ibounds.y - 1 - inormed.y), float(dd[0])));dd = (ivec2)mix(dd, ivec2(ov_ibounds.x - 1 - inormed.x, 9), step(float(ov_ibounds.x - 1 - inormed.x), float(dd[0])));dd = (ivec2)mix(dd, ivec2(inormed.y, 9), step(float(inormed.y), float(dd[0])));vec2 tms = vec2(abs(dd[0]), 0.0);
      tms[1] = clamp((1+dd[1] - tms[0])/float(1+dd[1]), 0.0, 1.0);
      tms[0] = tms[1]*(1.0 / (1 + 6.2*abs(weight)*(1.0 - tms[1])) );
      tms[1] = mix(2.0*tms[1] - tms[0], tms[0], step(0.0, weight));
      tms[0] = clamp(tms[1], 0.0, 1.0);
      mixwell = tms[1];
      selfposition = ioffset;
      return vec4(result, mixwell);
    }
    
    #version 130
    vec3 overlayColor2(in vec4 in_variant, in vec3 undercolor) 
    {
      vec3 result;
      float mixwell = 0.0;
      mixwell = in_variant[3];
      result = vec3(1.0,1.0,1.0);
      return mix(undercolor, result, mixwell);
    }

</p></details>

## Includes

#### FOR COMPILATION (add to your Qt project):

1. Required core files:
```
    bsdraw/core/bsidrawcore.h           # include for symbols. General structs, flags, enums, defines
    bsdraw/core/bsdraw.h                # include for symbols. base class DrawCore (Non-Qt)
    bsdraw/core/bsqdraw.h               # include for symbols. DrawCore + Qt
    bsdraw/core/bsqdraw.cpp             # translation unit
    bsdraw/core/sheigen/bsshgenmain.h   # include for symbols. shader generation funcs
    bsdraw/core/sheigen/bsshgenmain.cpp # translation unit
```

2. For 2D draws:
```
    bsdraw/bsdrawintensity.h            # class DrawIntensity, declaration
    bsdraw/bsdrawintensity.cpp          # translation unit, definition
    bsdraw/bsdrawrecorder.h             # class DrawRecorder, declaration
    bsdraw/bsdrawrecorder.cpp           # translation unit, definition
    bsdraw/bsdrawdomain.h               # class DrawDomain, declaration
    bsdraw/bsdrawdomain.cpp             # translation unit, definition
```

3. For 1D draws:
```
    bsdraw/bsdrawgraph.h                # class DrawGraph, declaration
    bsdraw/bsdrawgraph.cpp              # translation unit, definition
```    

4. For draw bars:
```
    bsdraw/bsdrawscales.h;              # class DrawBars, declaration
    bsdraw/bsdrawscales.cpp             # translation unit, definition
```    

5. For overlays:
```
    bsdraw/core/bsoverlay.h             # include for symbols. Overlay base classes
    bsdraw/core/bsoverlay.cpp           # translation unit
    bsdraw/core/bsqoverlay.h            # optionally. Qt signals&slots support for basic overlay methods
    bsdraw/core/bsqoverlay.cpp          # optionally. translation unit
    bsdraw/core/sheigen/bsshgencolor.h    # include for symbols. Shader generator for overlay coloring 
    bsdraw/core/sheigen/bsshgencolor.cpp  # translation unit
    bsdraw/core/sheigen/bsshgentrace.h    # include for symbols. Shader generator for overlay coordinates maths
    bsdraw/core/sheigen/bsshgentrace.cpp  # translation unit
    ... and overlays you need, i.e.
    bsdraw/overlays/bsborder.cpp & .h
    bsdraw/overlays/bsfigures.cpp & .h
    bsdraw/overlays/bsgrid.cpp & .h
    bsdraw/overlays/bspoints.cpp & .h
    bsdraw/overlays/bssprites.cpp & .h
    bsdraw/overlays/bstextstatic.cpp & .h
    bsdraw/overlays/special/bsmarks.cpp & .h
    bsdraw/overlays/special/bsblocker.cpp & .h
    bsdraw/overlays/bsinteractive.cpp & .h
    bsdraw/overlays/bscontour.cpp & .h
    bsdraw/overlays/bsimage.cpp & .h
```
  

#### FOR SYMBOLS (add to your header or source file):

1. For 2D draws:
```
    bsdraw/bsdrawintensity.h            // class DrawIntensity
    bsdraw/bsdrawrecorder.h             // class DrawRecorder
    bsdraw/bsdrawdomain.h               // class DrawDomain
```

2. For 1D draws:
```
    bsdraw/bsdrawgraph.h                // class DrawGraph
```    

3. For draw bars:
```
    bsdraw/bsdrawscales.h;              // class DrawBars
```   

4. For palettes:
```
    bsdraw/palettes/bspalettes_std.h    // simple palettes list and includes
    bsdraw/palettes/bspalettes_adv.h    // advanced palettes list and includes
    bsdraw/palettes/bspalettes_rgb.h    // 2d palettes list and includes
```

5. For overlays you need:
```
    bsdraw/overlays/bsborder.h
    bsdraw/overlays/bsfigures.h
    bsdraw/overlays/bsgrid.h
    bsdraw/overlays/bspoints.h
    bsdraw/overlays/bssprites.h
    bsdraw/overlays/bstextstatic.h
    bsdraw/overlays/special/bsmarks.h
    bsdraw/overlays/special/bsblocker.h
    bsdraw/overlays/bsinteractive.h
    bsdraw/overlays/bscontour.h
    bsdraw/overlays/bsimage.h
```

#### Future:
* shader precompilation
* multiportional overpattern
