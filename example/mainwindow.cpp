#include "mainwindow.h"

#include <QTimer>
#include <math.h>
#include "palettes/bspalettes_std.h"
#include "palettes/bspalettes_adv.h"
#include "palettes/bspalettes_rgb.h"

#include <omp.h>

#include "bsdrawintensity.h"
#include "bsdrawdomain.h"
#include "bsdrawgraph.h"
#include "bsdrawrecorder.h"

#include "overlays/bsinteractive.h"
#include "overlays/bsgrid.h"
#include "overlays/bspoints.h"
#include "overlays/special/bsblocker.h"
#include "overlays/bstextstatic.h"
#include "overlays/bsfigures.h"
#include "overlays/bssprites.h"
#include "overlays/special/bsmarks.h"
#include "overlays/bsborder.h"
#include "overlays/bscontour.h"
#include "overlays/bsimage.h"

#include "overlays/special/bstestprecision.h"

#include "bsdecimators.h"

#include "layout/bsqlayout.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QSignalMapper>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QTabBar>
#include <QButtonGroup>

#include "palettes/QPaletteBox.h"

#include <QSlider>
#include <QScrollBar>
#include <QScrollArea>
#include <QComboBox>
#include <QColorDialog>

/// Widget weight for layout
enum LAYOUT_WEIGHT{ LW_1, LW_0111, LW_1000, LW_012, LW_1110 } 
                  lw = LW_1;

/// startup speed
enum SPEED_TIMER { SP_STOP=0, SP_ONCE=1, SP_SLOWEST=1000, SP_SLOW=300, SP_FAST=40, SP_FASTEST=20 }
                  sp = SP_SLOW;

/// images for Overlays: sprite, foreground, background
static const char* img_path_mikey = "../example/mikey.jpg";  /// 1920x816
static const char* img_path_sprite =  "../example/snowflakewhite.png";
static const char* img_path_normal = "../example/image2.jpg";  /// 800x600
//static const char* img_path_normal = "../example/mikey.jpg";  /// 1920x816
//static const char* img_path_normal = "../example/image3.jpg";  /// 200x150

/// if uncomment: data generation will be more in 5 times. setData called with decimation algo
//#define DECIMATION window_min
//#define DECIMATION window_max

/// if uncomment: changes 'point size' for width and height synchroniously
//#define SYNCSCALING 3

const IPalette* const ppalettes_std[] = {  &paletteBkWh, &paletteBkGyGyGyWh, &paletteGnYe, &paletteBlWh, &paletteBkRdWh, &paletteBkBlWh, &paletteBkGrWh, &paletteBkBlGrYeWh };
const IPalette* const ppalettes_rgb[] = {  &paletteRG, &paletteRB, &paletteRGB };

MainWindow::MainWindow(tests_t testnumber, QWidget *parent):  QMainWindow(parent), 
  MW_TEST(testnumber), randomer(nullptr), active_ovl(0), ovl_visir(-1), ovl_marks(-1), ovl_figures(-1), ovl_sprites(-1), ovl_active_mark(9), ovl_is_synced(true), sigtype(ST_PEAK3), sig_k(1), sig_b(0)
{  
  int syncscaling=0;
  drawscount = 0;
  unsigned drcount = 1, dccount = 1;
#define PRECREATE(dr, dc) drcount = dr; dccount = dc; drawscount = (dr)*(dc); draws = new DrawQWidget*[drawscount];
  
  if (MW_TEST == LET_IT_SNOW)  /// fullsize recorder LET IT SNOW!
  {
    SAMPLES = 1600;
    MAXLINES = 600;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawRecorder(SAMPLES, MAXLINES, 2000, PORTIONS);
  }
  else if (MW_TEST == DEMO_1) /// Demo 1
  {
    sigtype = ST_SINXX;
    SAMPLES = 200;
    MAXLINES = 200;
    PORTIONS = 3;
    PRECREATE(3, 3);
    draws[0] = new DrawIntensity(SAMPLES, MAXLINES, 1);
    draws[3] = new DrawDomain(SAMPLES, MAXLINES, 1, false, OR_LRBT, true);
    {
      DIDomain& ddm = *((DrawDomain*)draws[3])->domain();
      
      for (int j=0; j<SAMPLES; j++)
      {
        ddm.start();
        for (int r=0; r<MAXLINES/2; r++)
          ddm.includePixel(MAXLINES/2 + sin(j/(2.0*M_PI*8))*MAXLINES/4 - MAXLINES/4 + r, j);
        ddm.finish();
      }
    }
    draws[6] = new DrawRecorder(SAMPLES, MAXLINES);
    
    graphopts_t  gopts[] = { graphopts_t(graphopts_t::GT_DOTS, 0.0f), 
                             graphopts_t(graphopts_t::GT_LINTERP, 0.0f), 
                             graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX, 0.0f)
                           };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], DrawGraph::DC_OFF);
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      gopts[i].dotsize = 1;
      gopts[i].dotweight = 0.5f;
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], DrawGraph::DC_DOWNBASE);
    }
  }
  else if (MW_TEST == DEMO_2)  /// Demo 2
  {
    sigtype = ST_GEN_NORM;
    SAMPLES = 50;
    MAXLINES = 50;
    PORTIONS = 2;
    syncscaling = 4;
    PRECREATE(3, 3);
    DPostmask dpmcontour(DPostmask::PM_LINELEFTBOTTOM, DPostmask::PO_EMPTY, 0, 0.0f,0.0f,0.0f);
    draws[0] = new DrawIntensity(SAMPLES, MAXLINES, 1);
    draws[0]->setPostMask(dpmcontour);
    draws[3] = new DrawDomain(SAMPLES, MAXLINES, 1, false, OR_LRBT, true);
    draws[3]->setPostMask(dpmcontour);
    {
      DIDomain& ddm = *((DrawDomain*)draws[3])->domain();
      
      for (int j=0; j<SAMPLES; j++)
      {
        ddm.start();
        for (int r=0; r<MAXLINES/2; r++)
          ddm.includePixel(MAXLINES/2 + sin(j/(2.0*M_PI*8))*MAXLINES/4 - MAXLINES/4 + r, j);
        ddm.finish();
      }
    }
    draws[6] = new DrawRecorder(SAMPLES, MAXLINES);
    draws[6]->setPostMask(dpmcontour);
    
    graphopts_t  gopts[] = { graphopts_t(graphopts_t::GT_DOTS, 0.0f), 
                             graphopts_t(graphopts_t::GT_LINTERP, 0.0f), 
                             graphopts_t(graphopts_t::GT_LINDOWN_CROSSMAX, 0.0f)
                           };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], DrawGraph::DC_OFF);
      draws[3*i + 1]->setPostMask(dpmcontour);
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], DrawGraph::DC_OFF);
      if (i == 0)
        draws[3*i + 2]->setPostMask(DPostmask(DPostmask::PM_PSEUDOCIRCLE, DPostmask::PO_ALL, 0, 0.0f,0.1f,0.0f));
      else
        draws[3*i + 2]->setPostMask(DPostmask(DPostmask::PM_LINELEFT, DPostmask::PO_ALL, 1, 0.0f,0.0f,0.0f));
    }    
  }  
  else if (MW_TEST == MAIN_DRAWS_WIDE) /// std
  {
    sigtype = ST_MOVE;
    SAMPLES = 600;
    MAXLINES = 200;
    PORTIONS = 1;   // for 2 or 3 portions use rgb colors on tab Color
    PRECREATE(3, 1);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
      draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t(graphopts_t::GT_LINTERP), DrawGraph::DC_OFF);
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, MAXLINES, 1000, PORTIONS);
    }
  }
  else if (MW_TEST == DIFFERENT_PORTIONS) /// Different Portions
  {
    sigtype = ST_SIN;
    SAMPLES = 400;
    MAXLINES = 100;
    PORTIONS = 3;
    PRECREATE(3, 2);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, MAXLINES, i == 1? 1 : PORTIONS);
//          draws[i*drcount + 0]->setScalingLimitsSynced(2,2);
      draws[i*drcount + 1] = new DrawGraph(SAMPLES, i == 1? 1 : PORTIONS, graphopts_t(graphopts_t::GT_LINTERP, 0.0, 0x00111111), DrawGraph::DC_OFF);
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, MAXLINES, 1000, i == 1? 1 : PORTIONS);
    }
  }
  else if (MW_TEST == DRAW_RECORDER)  /// recorders
  {
    SAMPLES = 300;
    MAXLINES = 400;
    PORTIONS = 1;
    PRECREATE(1, 2);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawRecorder(SAMPLES, MAXLINES, 2000, PORTIONS);
//      draws[i]->setOrientation(OR_LRBT);
    }
  }
  else if (MW_TEST == DRAW_BRIGHT)  /// brights for ovls
  {
    SAMPLES = 100;
    MAXLINES = 100;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = 5;
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
//      draws[i]->setScalingLimitsSynced(4);
      DPostmask fsp(DPostmask::PM_LINELEFTBOTTOM, DPostmask::PO_EMPTY, 0, 0.3,0.3,0.3 );
      draws[i]->setPostMask(fsp);
    }
  }
  else if (MW_TEST == DRAW_GRAPH_PIXELATION)   /// pixelation
  {
//    sigtype = ST_PEAK;
    sigtype = ST_MOVE;
    SAMPLES = 100;
    MAXLINES = 20;
    PORTIONS = 3;
    PRECREATE(4, 1);
    syncscaling = 10;
    graphopts_t gts[] = { {graphopts_t::GT_LINDOWN, 0.5f}, {graphopts_t::GT_LINDOWN_CROSSMIN, 0.5f}, {graphopts_t::GT_LINDOWN_CROSSMAX, 0.5f}, {graphopts_t::GT_LINTERP, 0.8f} };
    DPostmask fsp[] = {   DPostmask(DPostmask::PM_LINELEFT, DPostmask::PO_ALL, 0, 0.3,0.3,0.3), 
                          DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_SIGNAL, 0, 0.3,0.3,0.3), 
                          DPostmask(DPostmask::PM_LINELEFTBOTTOM, DPostmask::PO_ALL, 0, 0.3,0.3,0.3), 
                          DPostmask(DPostmask::PM_PSEUDOCIRCLE, DPostmask::PO_ALL, 0, 0.1,0.1,0.1)
                           };
//    DPostmask fsp[] = {   DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_ALL, 0, 0.3,0.3,0.3), 
//                          DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_ALL, 0, 0.3,0.3,0.3), 
//                          DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_ALL, 0, 0.3,0.3,0.3), 
//                          DPostmask(DPostmask::PM_PSEUDOCIRCLE, DPostmask::PO_ALL, 0, 0.1,0.1,0.1)
//                           };
    for (unsigned int i=0; i<drawscount; i++)
    {
//      gts[i].descaling = graphopts_t::DE_CENTER;
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, gts[i], DrawGraph::DC_OFF, 1.0, -0.5);
      draws[i]->setPostMask(fsp[i]);
    }
  }
  else if (MW_TEST == DRAW_DOMAIN)   /// domain
  {
    SAMPLES = 75;
    MAXLINES = 50;
    PORTIONS = 1;
    PRECREATE(2, 2);
    syncscaling = 5;
    for (unsigned int c=0; c<dccount; c++)
    {
      DrawDomain* dfirst = nullptr;
      for (unsigned int i=0; i<drcount; i++)
      {
        if (dfirst == nullptr)
        {
          dfirst = new DrawDomain(SAMPLES, MAXLINES, PORTIONS, false, OR_LRBT, true);
          DIDomain& ddm = *dfirst->domain();
          
          if (c == 0)
          {
            for (int i=0; i<4; i++)
            {
              for (int j=0; j<10; j++)
              {
                int r = MAXLINES / 8 + i * MAXLINES / 4, c = SAMPLES/20 + j*SAMPLES/10;
                ddm.start();
                ddm.includePixel(r-1, c);
                ddm.includePixel(r, c);
                ddm.includePixel(r+1, c);
                ddm.includePixel(r, c+1);
                ddm.includePixel(r, c-1);
                ddm.finish();
              }
            }
          }
          else
          {
            for (int j=0; j<SAMPLES + MAXLINES; j++)
            {
              ddm.start();
              for (int r=0; r<MAXLINES; r++)
              {
                if (j - r < 0)
                  break;
                if (j - r < SAMPLES)
                  ddm.includePixel(r, j - r);
              }
              ddm.finish();
            }
          }
          draws[c*drcount + i] = dfirst;
        }
        else
          draws[c*drcount + i] = new DrawDomain(*dfirst->domain(), PORTIONS);
      }
    }
  }
  else if (MW_TEST == DRAW_GRAPHS || MW_TEST == DRAW_GRAPHS_MOVE) /// graphs and graphmoves
  {
    SAMPLES = 300;
    MAXLINES = 1;
    PORTIONS = 2;
    PRECREATE(3, 3);
    graphopts_t::GRAPHTYPE gts[] = { graphopts_t::GT_LINDOWN_CROSSMAX, graphopts_t::GT_LINTERP, graphopts_t::GT_DOTS };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        graphopts_t               gopts(gts[i], i == 0? c == dccount-1? 0.4f : 0.3f : 0.0f, c == 1? 0xFFFFFFFF : 0x00999999, i != 2? 0 : 2, 0.5f);
        DrawGraph::DOWNCOLORIZE   dclr = c == 0 ? DrawGraph::DC_OFF : c == 1? DrawGraph::DC_OFF : DrawGraph::DC_DOWNBASE;
        if (MW_TEST == DRAW_GRAPHS)
          draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gopts, dclr);
        else
          draws[c*drcount + i] = new DrawGraphMoveEx(SAMPLES, 5, SAMPLES*2, PORTIONS, gopts, dclr);
      }
  }
  else if (MW_TEST == DRAW_BRIGHT_CLUSTER) /// bright cluster
  {
    SAMPLES = 30;
    MAXLINES = 30;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = 14;
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensePoints(SAMPLES, MAXLINES, PORTIONS);
    }
  }
  else if (MW_TEST == ADV_PALETTES)    /// advanced palettes show
  {
    SAMPLES = 400;
    MAXLINES = 20;
    PORTIONS = 1;
    syncscaling = 1;
    PRECREATE(sizeof(ppalettes_adv) / sizeof(const IPalette*), 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, MAXLINES);
      draws[i]->setDataPalette(ppalettes_adv[i]);
    }
    sigtype = ST_RAMP;
  }
  else if (MW_TEST == PROGRESS_BAR)   /// progress
  {
    sigtype = ST_10;
    SAMPLES = 20;
    MAXLINES = 1;
    PORTIONS = 1;
    PRECREATE(5, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      if (i<drawscount - 1)
      {
        draws[i] = new DrawIntensity(SAMPLES, 1, PORTIONS);
        draws[i]->setScalingLimitsV(10,10);
        draws[i]->setScalingLimitsH(10,10);
        if (i == 1)
          draws[i]->setPostMask(DPostmask(DPostmask::PM_CONTOUR, DPostmask::PO_EMPTY, 0, 0.3f,0.3f,0.3f));
        else if (i == 2)
          draws[i]->setPostMask(DPostmask(DPostmask::PM_LINELEFT, DPostmask::PO_EMPTY, 0, 0.7f,0.7f,0.7f));
        else if (i == 3)
          draws[i]->setPostMask(DPostmask(DPostmask::PM_PSEUDOCIRCLE, DPostmask::PO_EMPTY, 0, 0.0f,0.0f,0.0f));
      }
      else
        draws[i] = new DrawGraph(SAMPLES, 1, graphopts_t(graphopts_t::GT_DOTS));
    }
  }
  
  
  
#ifdef SYNCSCALING
  syncscaling = SYNCSCALING;
#endif
  
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      if (MW_TEST != ADV_PALETTES)
        draws[i]->setDataPalette(MW_TEST == DIFFERENT_PORTIONS? (const IPalette*)&paletteRGB : (const IPalette*)&palette_gnu_latte);
      if (syncscaling > 0)
      {
        draws[i]->setScalingLimitsSynced(syncscaling);
//        draws[i]->setScalingLimitsH(syncscaling);
//        draws[i]->setScalingLimitsV(syncscaling);
      }
      
      draws[i]->setClearColor(0x00333333);
//      draws[i]->setClearByPalette();
      if (MW_TEST == LET_IT_SNOW)
      {
        IOverlaySimpleImage* oimg = new OImageStretched(new QImage(img_path_mikey), IOverlaySimpleImage::IC_BLOCKALPHA, false);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.1);
        draws[i]->ovlPushBack(oimg);
        ovl_sprites = draws[i]->ovlPushBack(new OSprites(new QImage(img_path_sprite), OSprites::IC_AUTO, SAMPLES/4/*500*/, 0.2f));
        draws[i]->ovlPushBack(new OTextStatic("Press Me", CR_RELATIVE, 0.5f, 0.1f, 12, true));
      }
    }
    if (MW_TEST == MAIN_DRAWS_WIDE)
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
  }

  {
    QFont     fntSTD = QFont(this->font());
    
    QLabel*       lab;
    
    const unsigned int btnMinWidth = 80, btnMaxWidth = 120;
    const unsigned int edMinWidth = 40, edMaxWidth = 40;
    const unsigned int sbUniWidth = 40;
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    
    {
      int tabshow = 0;
      QSignalMapper*  featsMapper = new QSignalMapper(this);
      
      BSLAYOUT_DECL(mainLayout)
          
      BS_START_LAYOUT(QHBoxLayout)
//        BS_START_LAYOUT(QVBoxLayout)
        BS_START_FRAME_V_HMIN_VMIN(BS_FRAME_BOX, 0)
        {
          QWidget* totalHideShow = BSWIDGET;
        
          QTabBar*  ptb = new QTabBar;
          BSADD(ptb);
          BS_START_STACK_HMAX_VMIN
          QWidget* stackHideShow = BSWIDGET;
          QObject::connect(ptb, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
          QObject::connect(ptb, SIGNAL(currentChanged(int)), stackHideShow, SLOT(show()));
          {
            tabshow = ptb->addTab(tr("Data"));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                int bfsmask = BFS_CHECKABLE | (MW_TEST == DRAW_BRIGHT_CLUSTER? BFS_DISABLED : 0);
                BSFieldSetup sigs[] = { 
                  BSFieldSetup(tr("Random"),  &fntSTD, ST_RAND, bfsmask,     btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Normal"), &fntSTD, ST_GEN_NORM, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Peaks"),   &fntSTD, ST_PEAK,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Peaks2"),   &fntSTD, ST_PEAK2,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Peaks3"),   &fntSTD, ST_PEAK3,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Move"),    &fntSTD, ST_MOVE, bfsmask,   btnMinWidth, btnMaxWidth),
                  
                  BSFieldSetup(tr("SinX"),   &fntSTD, ST_SIN,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("MSins"), &fntSTD, ST_MANYSIN,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Hiperb"),   &fntSTD, ST_HIPERB, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("SinX/X"),   &fntSTD, ST_SINXX, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("X*X"),   &fntSTD, ST_XX, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("tanhX"),   &fntSTD, ST_TANHX, bfsmask,   btnMinWidth, btnMaxWidth),
                  
                  BSFieldSetup("--0--",   &fntSTD, ST_ZERO, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("--1--",   &fntSTD, ST_ONE,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("1-0-1",   &fntSTD, ST_ZOZ,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("1-0--",   &fntSTD, ST_ZOO,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("--0-1",   &fntSTD, ST_OOZ,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("101010", &fntSTD, ST_OZOZO, bfsmask,   btnMinWidth, btnMaxWidth),
              
                  BSFieldSetup(tr("Ramp"),   &fntSTD, ST_RAMP, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Step"), &fntSTD, ST_STEP, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("10",   &fntSTD, ST_10, bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("100",   &fntSTD, ST_100,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("1000",   &fntSTD, ST_1000,  bfsmask,   btnMinWidth, btnMaxWidth),
                  BSFieldSetup("10000",   &fntSTD, ST_10000,  bfsmask,   btnMinWidth, btnMaxWidth),
                };
                for (unsigned int i=0; i<sizeof(sigs)/sizeof(BSFieldSetup); i++)
                  if (sigs[i].mappedvalue == sigtype)
                    sigs[i].flags |= BFS_CHECKED;
                QButtonGroup* qbg = new QButtonGroup(this);
                qbg->setExclusive(true);
                
                const unsigned int vsigs = 6; //sizeof(sigs)/sizeof(BSFieldSetup) / 3;
                BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                  for (unsigned int i=0; i<vsigs; i++)
                    BSAUTO_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
                BS_STOP
                BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                  for (unsigned int i=vsigs; i<vsigs*2; i++)
                    BSAUTO_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
                BS_STOP
                BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                  for (unsigned int i=vsigs*2; i<vsigs*3; i++)
                    BSAUTO_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
                BS_STOP
                BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                  for (unsigned int i=vsigs*3; i<sizeof(sigs)/sizeof(BSFieldSetup); i++)
                    BSAUTO_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
                BS_STOP
                QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeSigtype(int)));
              BS_STOP
            
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSAUTO_TEXT_ADD(tr("Data posttransform:"), 0, Qt::AlignHCenter);
                BS_STRETCH
                BSAUTO_TEXT_ADD(tr("k*x + b"), 0, Qt::AlignHCenter);
                BS_STRETCH
                BSFieldSetup sigeds[] = { 
                  BSFieldSetup(QString::number(sig_k, 'f', 1), &fntSTD, ED_SIGK, 0, edMinWidth, edMaxWidth),
                  BSFieldSetup(QString::number(sig_b, 'f', 1), &fntSTD, ED_SIGB, 0, edMinWidth, edMaxWidth),
                };
                QSignalMapper*  edMapper = new QSignalMapper(this);
                for (unsigned int i=0; i<sizeof(sigeds) / sizeof(BSFieldSetup); i++)
                {
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    BSAUTO_TEXT_ADD(i == 0 ? tr("k ="): tr("b ="));
                    BSAUTO_EDIT_ADDMAPPED(sigeds[i], edMapper, 0, Qt::AlignRight);
                  BS_STOP
                }
                QObject::connect(edMapper, SIGNAL(mapped(int)), this, SLOT(changeFloats(int)));
              BS_STOP
              BS_STRETCH
            BS_STOP
            
            ptb->addTab(tr("Color"));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSAUTO_TEXT_ADD(tr("bspalettes_std.h:"), 0, Qt::AlignLeft);
                QSignalMapper*  palMapper = new QSignalMapper(this);
                const unsigned int btnsinrow = 4;
                unsigned int palscount = sizeof(ppalettes_std)/sizeof(const IPalette*);
                unsigned int rows= palscount / btnsinrow + (palscount % btnsinrow ? 1 : 0);
                unsigned int palctr=0;
                
                const QString   stdPalNames[] = {  tr("Bk-Wh"),  tr("Gray"), tr("Gn-Ye"),   tr("Bu-Wh"),  tr("Bk-Rd-Wh"), tr("Bk-Bu-Wh"), tr("Bk-Gn-Wh"), tr("Bk-Gn-Ye-Wh"), tr("SomeBlue"), tr("Test2") };
                
                for (unsigned int i=0; i < rows && palctr < palscount; i++)
                {
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                      
                  BS_CHEAT_VMIN
                    for (unsigned int j=0; j<btnsinrow && palctr < palscount; j++)
                    {
                      BSFieldSetup bfs(stdPalNames[palctr], &fntSTD, palctr, 0, btnMinWidth);
                      BSAUTO_BTN_ADDMAPPED(bfs, palMapper);
                      palctr++;
                    }
                  BS_STOP
                }
                QObject::connect(palMapper, SIGNAL(mapped(int)), this, SLOT(changePaletteSTD(int)));
                
                BSAUTO_TEXT_ADD(tr("bspalettes_adv.h:"), 0, Qt::AlignLeft);
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_SUNKEN, 1)
                {
                  BS_STRETCH
                  QPaletteBox*  qcb = new QPaletteBox;
                  QObject::connect(qcb, SIGNAL(currentIndexChanged(int)), this, SLOT(changePaletteADV(int)));
                  BSADD(qcb);
                  QCheckBox*  inv = new QCheckBox(tr("Inverted"));
                  QObject::connect(inv, SIGNAL(clicked(bool)), qcb, SLOT(setInverted(bool)));
                  BSADD(inv);
                  BS_STRETCH
                }
                BS_STOP
                
                BSAUTO_TEXT_ADD(tr("bspalettes_rgb.h:"), 0, Qt::AlignLeft);
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_SUNKEN, 1)
                {
                  QSignalMapper*  palMapperRGB = new QSignalMapper(this);
                  bool  on2=true, on3=true;
#if PORTIONS == 2
                  on3 = false;
#elif PORTIONS == 3
                  on2 = false;
#endif
                  BS_STRETCH
                  BSFieldSetup bfs[] = { BSFieldSetup("2 clr: ReGn", &fntSTD, 0, on2 == false? BFS_DISABLED : 0, btnMinWidth), 
                                         BSFieldSetup("2 clr: ReBu", &fntSTD, 1, on2 == false? BFS_DISABLED : 0, btnMinWidth), 
                                         BSFieldSetup("3 clr: RGB", &fntSTD, 2, on3 == false? BFS_DISABLED : 0, btnMinWidth), 
                                       };
                  for (unsigned int j=0; j<sizeof(bfs)/sizeof(BSFieldSetup); j++)
                    BSAUTO_BTN_ADDMAPPED(bfs[j], palMapperRGB);
                  BS_STRETCH
                  QObject::connect(palMapperRGB, SIGNAL(mapped(int)), this, SLOT(changePaletteRGB(int)));
                }
                BS_STOP
              BS_STOP
              
              {
                BSFieldSetup fseds[] = { 
                  BSFieldSetup("1.0", &fntSTD, ED_HIGH, 0, edMinWidth, edMaxWidth),
                  BSFieldSetup("0.0", &fntSTD, ED_LOW, 0, edMinWidth, edMaxWidth),
                  BSFieldSetup("1.0", &fntSTD, ED_CONTRAST, 0, edMinWidth, edMaxWidth),
                  BSFieldSetup("0.0", &fntSTD, ED_OFFSET, 0, edMinWidth, edMaxWidth),
                };
                QSignalMapper*  edMapper = new QSignalMapper(this);
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                  BS_STRETCH
                  
                  lab = new QLabel(tr("setBounds:"));
                  BSADD(lab);
                  BS_START_LAYOUT(QHBoxLayout)
                    for (int i=0; i<2; i++)
                      BSAUTO_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                  BS_STOP
                  BS_STRETCH    
                  
                  lab = new QLabel(tr("setContrast:"));
                  BSADD(lab);
                  BS_START_LAYOUT(QHBoxLayout)
                    for (int i=2; i<4; i++)
                      BSAUTO_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                  BS_STOP
                  BS_STRETCH
                BS_STOP
                QObject::connect(edMapper, SIGNAL(mapped(int)), this, SLOT(changeFloats(int)));
              } 
              BS_STRETCH
            BS_STOP

            ptb->addTab(tr("Sizes"));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSFieldSetup featbtns[] = { 
                  BSFieldSetup(tr("Mirror horz"), &fntSTD, BTF_INVHORZ, 0, btnMinWidth),
                  BSFieldSetup(tr("Mirror vert"), &fntSTD, BTF_INVVERT, 0, btnMinWidth),
                };
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_CHEAT_VMIN
                  for (unsigned int i=0; i<sizeof(featbtns)/sizeof(BSFieldSetup); i++)
                    BSAUTO_BTN_ADDMAPPED(featbtns[i], featsMapper);
                BS_STOP
              BS_STOP
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
              {
                BSAUTO_TEXT_ADD(tr("setScalingLimits:"), 0, Qt::AlignLeft);
                
#define NEW_SPIN_ADDMAPPED(var, slot, ...)  { \
                                      QSpinBox*   _sb = new QSpinBox(); \
                                      _sb->setRange(0, 10); \
                                      _sb->setUserData(0, new BSUOD_0(var)); \
                                      BSADD(_sb, ##__VA_ARGS__); \
                                      QObject::connect(_sb, SIGNAL(valueChanged(int)), this, slot); \
                                    }
//                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_SUNKEN, 1)
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_STRETCH
                  BSAUTO_TEXT_ADD(tr("H.min:"));
                  NEW_SPIN_ADDMAPPED(SC_MIN_H, SLOT(changeScaling(int)), 0, Qt::AlignRight);
                  BSAUTO_TEXT_ADD(tr("H.max:"));
                  NEW_SPIN_ADDMAPPED(SC_MAX_H, SLOT(changeScaling(int)), 0, Qt::AlignRight);
                  BSAUTO_TEXT_ADD(tr("V.min:"));
                  NEW_SPIN_ADDMAPPED(SC_MIN_V, SLOT(changeScaling(int)), 0, Qt::AlignRight);
                  BSAUTO_TEXT_ADD(tr("V.max:"));
                  NEW_SPIN_ADDMAPPED(SC_MAX_V, SLOT(changeScaling(int)), 0, Qt::AlignRight);
                  BS_STRETCH
                BS_STOP
                BS_STRETCH
              }
              BS_STOP
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
              {
                BS_STRETCH
                BSAUTO_TEXT_ADD(tr("setContentMargins:"), 0, Qt::AlignLeft);
                QSpinBox*   _sb = new QSpinBox();
                BSADD(_sb, 0, Qt::AlignRight);
                QObject::connect(_sb, SIGNAL(valueChanged(int)), this, SLOT(changeMargins(int)));
                BS_STRETCH
              }
              BS_STOP
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSFieldSetup extra[] = { 
                  BSFieldSetup(tr("Hide tab panel"), &fntSTD, 0, 0, btnMinWidth),
                  BSFieldSetup(tr("Hide all panel"), &fntSTD, 0, 0, btnMinWidth),
                };
                QWidget* pHiding[] = { stackHideShow, totalHideShow };
                for (unsigned int i=0; i< sizeof(extra) / sizeof(BSFieldSetup); i++)
                {
                  BSAUTO_BTN(QPushButton, _btn, extra[i]);
                  BSADD(_btn, 0, Qt::AlignCenter);
                  QObject::connect(_btn, SIGNAL(pressed()), pHiding[i], SLOT(hide()));
                }
              BS_STOP                
              BS_STRETCH
            BS_STOP
            
            ptb->addTab(tr("Overlays"));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              QTabBar*  ptb_ovl = new QTabBar;
              BSADD(ptb_ovl);
              BS_START_STACK_HMAX_VMIN
                QObject::connect(ptb_ovl, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
                
                ptb_ovl->addTab(tr("Standard"));
                BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                  QButtonGroup* ocrGroup = new QButtonGroup(this);
                  ocrGroup->setExclusive(true);
                  BSFieldSetup fseds[] = { 
                    BSFieldSetup(tr("OFF"), &fntSTD, COS_OFF, BFS_CHECKED, btnMaxWidth, btnMaxWidth),
                    BSFieldSetup(tr("Dekart"), &fntSTD, COS_DEKART, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Grids"), &fntSTD, COS_GRIDS, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Grids+Axes"), &fntSTD, COS_GRIDSAXES, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Circular"), &fntSTD, COS_CIRCULAR, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Selector"), &fntSTD, COS_SELECTOR, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Drop Lines"), &fntSTD, COS_DROPLINES, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Cluster"), &fntSTD, COS_CLUSTER, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Followers"), &fntSTD, COS_FOLLOWERS, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Inside"), &fntSTD, COS_INSIDE, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("CoverL"), &fntSTD, COS_COVERL, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("CoverH"), &fntSTD, COS_COVERH, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Contour"), &fntSTD, COS_CONTOUR, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Sprite\nalpha opaque"), &fntSTD, COS_SPRITEALPHA, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Foreground\nstretchable"), &fntSTD, COS_FOREGROUND, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    BSFieldSetup(tr("Background\nstatic"), &fntSTD, COS_BACKGROUND, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  };
                  
                  BS_FORFOR_P(3, l, lim, sizeof(fseds)/sizeof(BSFieldSetup)-1)
                  {
                    BS_START_LAYOUT(QHBoxLayout)
                      BS_CHEAT_VMIN
                      for (int i=0; i < lim; i++)
                      {
//                        if (lim < 3 && i == 1)
//                          BS_STRETCH
                        BSAUTO_BTN_ADDGROUPED(fseds[1+l+i], ocrGroup);
                      }
                    BS_STOP   
                  }
                  BSAUTO_BTN_ADDGROUPED(fseds[0], ocrGroup, 0, Qt::AlignHCenter);
                  QObject::connect(ocrGroup, SIGNAL(buttonClicked(int)), this, SLOT(createOverlaySTD(int)));
                  
                  if (drawscount > 1)
                  {
                    BSFieldSetup chbbs(tr("Sync interactive elements"), &fntSTD, 0, BFS_CHECKED);
                    BSAUTO_BTN(QCheckBox, _chb, chbbs);
                    _chb->setUserData(0, new BSUOD_0(chbbs.mappedvalue));
                    QObject::connect(_chb, SIGNAL(clicked(bool)), this, SLOT(setOverlaySync(bool)));
                    BSADD(_chb);
                  }
                  
                  BS_STRETCH
                BS_STOP
              
                ptb_ovl->addTab(tr("Additional"));
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 2)
                  QButtonGroup* grForm = new QButtonGroup(this);
                  BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                    grForm->setExclusive(true);
                    BSFieldSetup fseds[] = { 
                      BSFieldSetup(tr("Circles"), &fntSTD, 0, BFS_CHECKED, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("Triangles"), &fntSTD, 1, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("Squares"), &fntSTD, 2, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("Xses"), &fntSTD, 3, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("Points"), &fntSTD, 4, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      
                    };
                    for (unsigned int i=0; i<sizeof(fseds) / sizeof(BSFieldSetup); i++)
                      BSAUTO_BTN_ADDGROUPED(fseds[i], grForm);
                  BS_STOP
                  
                  QButtonGroup* grCount = new QButtonGroup(this);
                  BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                    grCount->setExclusive(true);
                    BSFieldSetup fseds[] = { 
                      BSFieldSetup(tr("x3"), &fntSTD, 0, BFS_CHECKED, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("1"), &fntSTD, 1, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("5"), &fntSTD, 2, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                      BSFieldSetup(tr("10"), &fntSTD, 3, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                    };
                    for (unsigned int i=0; i<sizeof(fseds) / sizeof(BSFieldSetup); i++)
                      BSAUTO_BTN_ADDGROUPED(fseds[i], grCount);
                  BS_STOP
                      
                  BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                    BSFieldSetup add(tr("ADD"),  &fntSTD, 0, 0, btnMinWidth, btnMaxWidth);
                    BSAUTO_BTN(QPushButton, _btn, add);
                    _btn->setUserData(0, new BSUOD_2(grForm));
                    _btn->setUserData(1, new BSUOD_2(grCount));
                    BSADD(_btn, 0, Qt::AlignCenter);
                    QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(createOverlayADD()));
                  BS_STOP
                BS_STOP
              
              BS_STOP
              BS_STRETCH
            BS_STOP
                  
            ptb->addTab(tr("OvlSets"));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                QSignalMapper*  ovlMapper = new QSignalMapper(this);
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                  QButtonGroup* qbg = new QButtonGroup(this);
                  qbg->setExclusive(true);
                  for (unsigned int i=0; i<10; i++)
                    BSAUTO_BTN_ADDGROUPED(BSFieldSetup(QString::number(i+1).toUtf8(), &fntSTD, i+1, i == 0? BFS_CHECKED : BFS_CHECKABLE, 30, 30), qbg, 0, Qt::AlignCenter);
                  active_ovl = 1;
                  QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeOVL(int)));
                BS_STOP
                
                QTabBar*  ptb_ovl = new QTabBar;
                BSADD(ptb_ovl);
                BS_START_STACK_HMAX_VMIN
                  QObject::connect(ptb_ovl, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
                
                  ptb_ovl->addTab(tr("Form"));
                  BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                    BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                      {
                        BSAUTO_TEXT_ADD(tr("Weight:"));
                        QSpinBox*   psb = new QSpinBox();
                        psb->setRange(0, 10);
                        BSADD(psb)
                        QObject::connect(psb, SIGNAL(valueChanged(int)), this, SLOT(changeOVLWeight(int)));
                      }

                      BS_STRETCH
                      
                      BSFieldSetup spbs[] = {      BSFieldSetup(tr("Stroke:"), &fntSTD, 0, 0, sbUniWidth, sbUniWidth),
                                                   BSFieldSetup(tr("Space:"), &fntSTD, 1, 0, sbUniWidth, sbUniWidth),
                                                   BSFieldSetup(tr("Dot:"), &fntSTD, 2, 0, sbUniWidth, sbUniWidth),
                                            };
                      for (unsigned int i=0; i<sizeof(spbs)/sizeof(BSFieldSetup); i++)
                      {
                        BSAUTO_TEXT_ADD(spbs[i].defaultText, 0, Qt::AlignLeft);
                        QSpinBox*   psb = new QSpinBox();
                        BSAPPLY_FEATS(psb, spbs[i]);
                        psb->setRange(0, 100);
                        psb->setUserData(0, new BSUOD_0(spbs[i].mappedvalue));
                        psb->setAccelerated(true);
                        BSADD(psb, 0, Qt::AlignLeft)
                        QObject::connect(psb, SIGNAL(valueChanged(int)), this, SLOT(changeOVLForm(int)));
                      }
                    BS_STOP
                    
                    BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                      BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                        BSAUTO_LBL_ADD(BSFieldSetup(tr("Opacity:")));
                        QSlider* slider = new QSlider(Qt::Horizontal);
                        slider->setRange(0,100);
                        BSADD(slider);
                        QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOVLOpacity(int)));
                      BS_STOP
                      BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                        BSAUTO_LBL_ADD(BSFieldSetup(tr("Slice:")));
                        QSlider* slider = new QSlider(Qt::Horizontal);
                        slider->setRange(0,100);
                        slider->setValue(100);
                        BSADD(slider);
                        QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOVLSlice(int)));
                      BS_STOP
                    BS_STOP
                  BS_STOP
                        
                  ptb_ovl->addTab(tr("Color"));
                  BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                    BSAUTO_BTN_ADDMAPPED(BSFieldSetup(tr("Change Color"), &fntSTD, BTO_COLOR, 0, btnMinWidth, btnMaxWidth), ovlMapper, 0, Qt::AlignHCenter);
                  
                    BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                      QButtonGroup* qbg = new QButtonGroup(this);
                      qbg->setExclusive(true);
                      BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                        BS_CHEAT_VMIN
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Noinvert"), &fntSTD, BTO_NOINV, BFS_CHECKED, btnMinWidth, btnMaxWidth), qbg);
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Invert1"), &fntSTD, BTO_INV1, BFS_CHECKABLE, btnMinWidth/2, btnMaxWidth), qbg);
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Invert2"), &fntSTD, BTO_INV2, BFS_CHECKABLE, btnMinWidth/2, btnMaxWidth), qbg);
                      BS_STOP
                      BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                        BS_CHEAT_VMIN
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Invert3"), &fntSTD, BTO_INV3, BFS_CHECKABLE, btnMinWidth/2, btnMaxWidth), qbg);
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Invert4"), &fntSTD, BTO_INV4, BFS_CHECKABLE, btnMinWidth/2, btnMaxWidth), qbg);
                        BSAUTO_BTN_ADDGROUPED(BSFieldSetup(tr("Invert5"), &fntSTD, BTO_INV5, BFS_CHECKABLE, btnMinWidth/2, btnMaxWidth), qbg);
                      BS_STOP
                      QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeOVLFeatures(int)));
                    BS_STOP
//                    BS_STRETCH
                  BS_STOP
                          
                  ptb_ovl->addTab(tr("Position"));
                  BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                    QSignalMapper*  ovlposMapper = new QSignalMapper(this);
                    BSAUTO_BTN_ADDMAPPED(BSFieldSetup("+1", &fntSTD, BTOP_UP, 0, btnMinWidth/2, btnMaxWidth), ovlposMapper, 0, Qt::AlignHCenter);
                    BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                      BSAUTO_BTN_ADDMAPPED(BSFieldSetup("-1", &fntSTD, BTOP_LEFT, 0, btnMinWidth/2, btnMaxWidth), ovlposMapper);
                      BSAUTO_BTN_ADDMAPPED(BSFieldSetup("+1", &fntSTD, BTOP_RIGHT, 0, btnMinWidth/2, btnMaxWidth), ovlposMapper);
                    BS_STOP
                    BSAUTO_BTN_ADDMAPPED(BSFieldSetup("-1", &fntSTD, BTOP_DOWN, 0, btnMinWidth/2, btnMaxWidth), ovlposMapper, 0, Qt::AlignHCenter);
                    QObject::connect(ovlposMapper, SIGNAL(mapped(int)), this, SLOT(changeOVLPos(int)));
                    BS_STRETCH
                  BS_STOP
                  
                  QString ftabs[] = { tr("Replace"), tr("Create") };
                  for (unsigned int t=0; t<sizeof(ftabs)/sizeof(const char*); t++)
                  {
                    ptb_ovl->addTab(ftabs[t]);
                    BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 2)
                      BSFieldSetup forms1[] = { 
                        BSFieldSetup(tr("Circle"),  &fntSTD, BTV_CIRCLE,  0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Square"),    &fntSTD, BTV_SQUARE,  0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Line horz"), &fntSTD, BTV_LINEHORZ,   0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Line vert"),  &fntSTD, BTV_LINEVERT,    0,   btnMinWidth, btnMaxWidth),
                      };
                      BSFieldSetup forms2[] = { 
                        BSFieldSetup(tr("Visir"), &fntSTD, BTV_FACTOR,    0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Cross"), &fntSTD, BTV_CROSS,    0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Text"), &fntSTD, BTV_TEXT,    0,   btnMinWidth, btnMaxWidth),
                        BSFieldSetup(tr("Border"), &fntSTD, BTV_BORDER,    0,   btnMinWidth, btnMaxWidth),
                      };
                      BSFieldSetup forms3[] = { 
                        BSFieldSetup(tr("Remove"), &fntSTD, BTV_REMOVE,    0,   btnMinWidth, btnMaxWidth),
                      };
                      QButtonGroup* qbg = new QButtonGroup(this);
                      qbg->setExclusive(true);
                      BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
                        for (unsigned int i=0; i<sizeof(forms1)/sizeof(BSFieldSetup); i++)
                          BSAUTO_BTN_ADDGROUPED(forms1[i], qbg, 0, Qt::AlignCenter);
                      BS_STOP
                      BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
                        for (unsigned int i=0; i<sizeof(forms2)/sizeof(BSFieldSetup); i++)
                          BSAUTO_BTN_ADDGROUPED(forms2[i], qbg, 0, Qt::AlignCenter);
                      BS_STOP
                          
                      if (t == 0)
                      {
                        BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
  //                        for (unsigned int i=0; i<sizeof(forms3)/sizeof(BSFieldSetup); i++)
                          QPushButton* rmbtn = new QPushButton();
                          BSDEPLOY_BTN(rmbtn, forms3[0]);
                          rmbtn->setMinimumHeight(btnMinWidth);
                          qbg->addButton(rmbtn, forms3[0].mappedvalue);
                          BSADD(rmbtn, 0, Qt::AlignCenter);
                        BS_STOP
                      }
                      
                      if (t == 0)
                        QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(metaOVLReplace(int)));
                      else if (t == 1)
                        QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(metaOVLCreate(int)));
                      
                    BS_STOP
                  }
                BS_STOP
                
                BSAUTO_TEXT_ADD(tr("(settings are not saved between overlays here)"), 0, Qt::AlignHCenter);

              BS_STRETCH
              QObject::connect(ovlMapper, SIGNAL(mapped(int)), this, SLOT(changeOVLFeatures(int)));
            BS_STOP

            
            ptb->addTab(tr("Addit."));
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSAUTO_TEXT_ADD(tr("Interpolation (future):"), 0, Qt::AlignLeft);
                BSFieldSetup interp[] = { 
                  BSFieldSetup(tr("Off"),  &fntSTD, 0,  BFS_CHECKED,     btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Static"),&fntSTD, 1,  BFS_CHECKABLE,   btnMinWidth, btnMaxWidth),
//                  BSFieldSetup(tr("Linear"), &fntSTD, IT_DATALINE,  BFS_CHECKABLE,   btnMinWidth, btnMaxWidth),
//                  BSFieldSetup(tr("Nearest"), &fntSTD, IT_NEAREST,  BFS_CHECKABLE,   btnMinWidth, btnMaxWidth),
                };
                QButtonGroup* qbg = new QButtonGroup(this);
                qbg->setExclusive(true);
                
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_CHEAT_VMIN
                  for (unsigned int i=0; i<sizeof(interp)/sizeof(BSFieldSetup); i++)
                    BSAUTO_BTN_ADDGROUPED(interp[i], qbg, 0, Qt::AlignCenter);
                BS_STOP
                QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeInterpolation(int)));
              BS_STOP
              
              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                {
                  BSAUTO_TEXT_ADD(tr("Pick Cluster in Overlay tab"));
                  BSFieldSetup test(tr("Change Palette"),  &fntSTD, 0, 0, btnMinWidth, btnMaxWidth);
                  BSAUTO_BTN(QPushButton, _btn, test);
                  BSADD(_btn, 0, Qt::AlignCenter);
                  QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(changeClusterPalette()));
                }
//                {
//                  BSFieldSetup test2("TEST_TIMED",  &fntSTD, 0, BFS_CHECKABLE, btnMinWidth, btnMaxWidth);
//                  BSAUTO_BTN(QPushButton, _btn, test2);
//                  BSADD(_btn, 0, Qt::AlignCenter);
//                }
              BS_STOP
              
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                BSAUTO_BTN_ADDMAPPED(BSFieldSetup(tr("Remove last graph"), &fntSTD, BTF_DESTROYGRAPH, 0, btnMinWidth), featsMapper, 0, Qt::AlignCenter);
              BS_STOP
                  
//              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
//                BSAUTO_TEXT_ADD(tr("Overlay' images:"), 0, Qt::AlignLeft);
//                QSignalMapper*  pathMapper = new QSignalMapper(this);
//                for (int i=0; i<sizeof(img_paths)/sizeof(img_paths[0]); i++)
//                {
//                  BSFieldSetup bfs(img_paths[i], &fntSTD, i, 0, btnMinWidth);
//                  QLineEdit ed;
//                  BSAUTO_EDIT_ADDMAPPED(bfs, pathMapper, 0, Qt::AlignCenter);
//                };
//                QObject::connect(pathMapper, SIGNAL(mapped(int)), this, SLOT(changePaths(int)));
//              BS_STOP
                  
              BS_STRETCH
            BS_STOP

            if (ovl_marks != -1)
            {
              ptb->addTab(tr("Marks"));
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BSAUTO_LBL_ADD(BSFieldSetup(tr("State:")));
                  QSlider* slider = new QSlider(Qt::Horizontal);
                  slider->setRange(0,100);
                  slider->setUserData(0, new BSUOD_0(MF_TCOLOR));
                  BSADD(slider);
                  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeMarkData(int)));
                BS_STOP
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BSAUTO_LBL_ADD(BSFieldSetup(tr("Position:")));
                  QSlider* slider = new QSlider(Qt::Horizontal);
                  slider->setRange(0,100);
                  slider->setUserData(0, new BSUOD_0(MF_POS));
                  BSADD(slider);
                  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeMarkData(int)));
                BS_STOP
              BS_STOP
            }
          }
          BS_STOP

          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
            BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_BOX, 1)
              BSAUTO_TEXT_ADD(tr("Ban update from:"), 0, Qt::AlignLeft);
              BS_STRETCH
              BSFieldSetup banbtns[] = { 
                BSFieldSetup(tr("data"),      &fntSTD, BTF_BANUPDATEDATA, 0, btnMinWidth, btnMaxWidth),
                BSFieldSetup(tr("settings"),   &fntSTD, BTF_BANUPDATESETS, 0, btnMinWidth, btnMaxWidth),
                BSFieldSetup(tr("overlays"),   &fntSTD, BTF_BANUPDATEOVERLAYS, 0, btnMinWidth, btnMaxWidth),
              };
              for (unsigned int i=0; i<sizeof(banbtns) / sizeof(BSFieldSetup); i++)
              {
                BSAUTO_BTN(QCheckBox, _chb, banbtns[i]);
                _chb->setUserData(0, new BSUOD_0(banbtns[i].mappedvalue));
                BSADD(_chb);
                QObject::connect(_chb, SIGNAL(clicked(bool)), this, SLOT(changeBans(bool)));
              }
            BS_STOP
                
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
            
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                BS_CHEAT_VMIN
                BSAUTO_TEXT_ADD(tr("Refresh widget:"), 0, Qt::AlignLeft);
                BS_STRETCH
                BSFieldSetup bfs(tr("Once"),    &fntSTD, SP_ONCE, 0, btnMinWidth, btnMinWidth);
                BSAUTO_BTN(QPushButton, _btn, bfs);
                BSADD(_btn)
                QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(changeSpeedUpdate_Once()));
              BS_STOP
            
              QButtonGroup* qbg = new QButtonGroup(this);
              BSFieldSetup upbtns[] = { 
                BSFieldSetup(tr(">"),    &fntSTD, SP_SLOWEST, BFS_CHECKABLE, btnMinWidth, btnMinWidth),
                BSFieldSetup(tr(">>"),    &fntSTD, SP_SLOW, BFS_CHECKABLE, btnMinWidth, btnMinWidth),
                BSFieldSetup(tr(">>>"),  &fntSTD, SP_FAST, BFS_CHECKABLE, btnMinWidth, btnMinWidth), 
                BSFieldSetup(tr(">>>>"),    &fntSTD, SP_FASTEST, BFS_CHECKABLE, btnMinWidth, btnMinWidth), 
                BSFieldSetup(tr("Stop"),    &fntSTD, SP_STOP, BFS_CHECKED, btnMinWidth, btnMinWidth),
              };
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                BS_CHEAT_VMIN
                for (unsigned int i=0; i<sizeof(upbtns) / sizeof(BSFieldSetup); i++)
                  BSAUTO_BTN_ADDGROUPED(upbtns[i], qbg);
              BS_STOP
              QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeSpeedUpdate(int)));
            BS_STOP
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_CHEAT_VMIN
                  BSAUTO_TEXT_ADD(tr("Generate data:"), 0, Qt::AlignLeft);
                  BS_STRETCH
                  BSFieldSetup bfs(tr("Once"),    &fntSTD, SP_ONCE, 0, btnMinWidth, btnMinWidth);
                  BSAUTO_BTN(QPushButton, _btn, bfs);
                  BSADD(_btn)
                  QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(changeSpeedData_Once()));
                BS_STOP
  
                BSFieldSetup rtsbtns[] = { 
                  BSFieldSetup(tr(">"),    &fntSTD, SP_SLOWEST, BFS_CHECKABLE, btnMinWidth, btnMinWidth),
                  BSFieldSetup(tr(">>"),    &fntSTD, SP_SLOW, BFS_CHECKABLE, btnMinWidth, btnMinWidth),
                  BSFieldSetup(tr(">>>"),  &fntSTD, SP_FAST, BFS_CHECKABLE, btnMinWidth, btnMinWidth), 
                  BSFieldSetup(tr(">>>>"),    &fntSTD, SP_FASTEST, BFS_CHECKABLE, btnMinWidth, btnMinWidth), 
                  BSFieldSetup(tr("Stop"),    &fntSTD, SP_STOP, BFS_CHECKABLE, btnMinWidth, btnMinWidth),
                };
                for (unsigned int i=0; i<sizeof(rtsbtns) / sizeof(BSFieldSetup); i++)
                  if (rtsbtns[i].mappedvalue == sp)
                    rtsbtns[i].flags |= BFS_CHECKED;
                
                QButtonGroup* qbg = new QButtonGroup(this);
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_CHEAT_VMIN
                  for (unsigned int i=0; i<sizeof(rtsbtns) / sizeof(BSFieldSetup); i++)
                    BSAUTO_BTN_ADDGROUPED(rtsbtns[i], qbg);
                BS_STOP
                QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeSpeedData(int)));
                
                BSAUTO_BTN_ADDMAPPED(BSFieldSetup(tr("Clear Data"), &fntSTD, BTF_CLEAR, btnMinWidth, btnMaxWidth), featsMapper);
              BS_STOP
          BS_STOP
                
          ptb->setCurrentIndex(tabshow);
//          BS_STRETCH
        }
        BS_STOP
  
        if (MW_TEST == ADV_PALETTES)
        {
          BS_START_SCROLL_V_HMAX_VMAX
            for (unsigned int i=0; i<drawscount; i++)
            {
              BSAUTO_TEXT_ADD(ppalettes_names[i]);
              BSADD(draws[i]);
            }
          BS_STOP;
        }
        else if (MW_TEST == PROGRESS_BAR)
        {
          BS_START_FRAME_V_HMIN_VMIN(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<drawscount; i++)
            {
              if (i == drawscount - 1)
                draws[i]->setFixedHeight(40);
              BSADD(draws[i])
            }
          BS_STOP;
        }
        else
        {
          QScrollBar* qsb = MW_TEST == DRAW_GRAPHS_MOVE? new QScrollBar(Qt::Vertical) : nullptr;
          BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<dccount; i++)
            {
              if (MW_TEST == DRAW_RECORDER)
                qsb = new QScrollBar();
              BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
              for (unsigned int j=0; j<drcount; j++)
              {
                if (i*drcount + j >= drawscount)
                  break;
                
                int lwresult = 0;
                if (lw == LW_1) lwresult = 1;
                else if (lw == LW_1000) lwresult = j == 0? 1 : 0;
                else if (lw == LW_0111) lwresult = j == 0? 0 : 1;
                else if (lw == LW_012)  lwresult = j;
                else if (lw == LW_1110) lwresult = j < 3? 1 : 0;
    
                if (qsb)
                  draws[i*drcount + j]->connectScrollBar(qsb, false);
                BSADD(draws[i*drcount + j], lwresult);
              }
              BS_STOP
              if (i == dccount - 1 && qsb)
                BSADD(qsb);
            }
          BS_STOP
        }
      BS_STOP
      
      QObject::connect(featsMapper, SIGNAL(mapped(int)), this, SLOT(changeFeatures(int)));
    }
    
    {
      QWidget* emptyForm = new QWidget(this);
      emptyForm->setLayout(mainLayout);
      this->setCentralWidget(emptyForm);
    }
  }
  
  
  speedUpdateTimer = new QTimer(this);
  connect(speedUpdateTimer, SIGNAL(timeout()), this, SLOT(updateAllDraws()));

  speedDataTimer = new QTimer(this);
  connect(speedDataTimer, SIGNAL(timeout()), this, SLOT(generateData()));
  
  
  {
#ifdef DECIMATION
    DSAMPLES = SAMPLES*5;
#else
    DSAMPLES = SAMPLES;
#endif
    testbuf2D = new float[PORTIONS*MAXLINES*DSAMPLES];
    testbuf1D = new float[PORTIONS*DSAMPLES];
    
    RND = MAXLINES > 1000? 100: 1000;
    randomer = new float[RND*DSAMPLES];
    float* rr = (float*)randomer;
    #pragma omp parallel for
    for (int i=0; i<DSAMPLES*RND; i++)
      rr[i] = rand()/float(RAND_MAX);
  }
  
  if (sp > 1)  
    speedDataTimer->start((int)sp);
}

MainWindow::~MainWindow()
{
  for (unsigned int i=0; i<drawscount; i++)
    if (draws[i])
      delete draws[i];
  delete []draws;
  if (randomer)
    delete []randomer;
  
  delete []testbuf2D;
  delete []testbuf1D;
}

void MainWindow::changeMargins(int value)
{
  for (unsigned int j=0; j<drawscount; j++)
    draws[j]->setContentsMargins(value, value, value, value);
}

#include <qmath.h>

static float my_hiperb(float x, float){ return 1.0f / x; }
static float my_sinxx(float x, float mov){ return qFastSin(x*(1+mov))/(x); }
static float my_xx(float x, float mov){ return x*x/(100 + 1000*mov); }
static float my_tanhx(float x, float mov){ return tanh(x/(20*(mov+0.05f)))/5.0; }

inline void generateGaussian(float mu, float sigma, float* rnd1, float* rnd2)
{
  float tmp = *rnd1;
  if (tmp < 0.0001f)  tmp = 0.0001f;
  else if (tmp >= 0.9999f)  tmp = 0.9999f;
  tmp = qSqrt(-2.0*qLn(tmp));
  float z0 = tmp*qCos(M_PI*2*(*rnd2));
  float z1 = tmp*qSin(M_PI*2*(*rnd2));
  *rnd1 = z0 * sigma + mu;
  *rnd2 = z1 * sigma + mu;
}

unsigned int g_movX, g_movX2=0;

void MainWindow::generateData()
{ 
  static unsigned int randomer_ctr = 0;
  int idx_primary = randomer_ctr % RND;
  const float* randbuf = &randomer[ idx_primary*DSAMPLES ];
  
  if (MW_TEST == DRAW_BRIGHT_CLUSTER)
  {
//  {
//    std::normal_distribution<float> ND(0.5, 0.1);
//    std::generate(pointsX.begin(), pointsX.end(), [&ND, &gen](){ return ND(gen); } );
//    std::generate(pointsY.begin(), pointsY.end(), [&ND, &gen](){ return ND(gen); } );
//    std::replace_if(pointsX.begin(), pointsX.end(), [](float x){ return x < 0.0f; }, 0.0f);
//    std::replace_if(pointsX.begin(), pointsX.end(), [](float x){ return x > 1.0f; }, 1.0f);
//    std::replace_if(pointsY.begin(), pointsY.end(), [](float x){ return x < 0.0f; }, 0.0f);
//    std::replace_if(pointsY.begin(), pointsY.end(), [](float x){ return x > 1.0f; }, 1.0f);
    
//    std::uniform_real_distribution<float> RD(0.5, 0.8);
//    std::generate(testbuf1D.begin(), testbuf1D.end(), [&RD, &gen](){ return RD(gen); } );
//    for (arrsamples_t::iterator iter = testbuf1D.begin(); iter != testbuf1D.end(); ++iter)
//      *iter = *iter*sig_k + sig_b;
//  }
//  std::array<unsigned int, PORTIONS> dsizes;
//  std::fill_n(dsizes.begin(), PORTIONS, DSAMPLES);
  
    const int count = DSAMPLES*MAXLINES/3;
    unsigned int countByPortions[PORTIONS];
    float Xs[count*PORTIONS], Ys[count*PORTIONS];
    float values[count*PORTIONS];
    for (int p=0; p<PORTIONS; p++)
    {
      countByPortions[p] = count;
      int idx_rnd = (int)(randomer[(idx_primary*DSAMPLES + p) % (RND*DSAMPLES)]*(RND-1));
      randbuf = &randomer[ idx_rnd ];
      
      #pragma omp parallel for
      for (int i=0; i<(int)countByPortions[p]; i++)
      {
        Xs[count*p + i] = randbuf[i % RND];
        Ys[count*p + i] = randbuf[(i+count) % RND];
        generateGaussian(0.5, 0.2, &Xs[count*p + i], &Ys[count*p + i]);
        if (Xs[count*p + i] < 0)  Xs[count*p + i] = 0;
        if (Xs[count*p + i] > 1)  Xs[count*p + i] = 1;
        if (Ys[count*p + i] < 0)  Ys[count*p + i] = 0;
        if (Ys[count*p + i] > 1)  Ys[count*p + i] = 1;
        values[count*p + i] = randbuf[i%RND]*sig_k + sig_b;
  //      values[count*p + i] = randbuf[i % RND] > 0.75f? 0.96 : randbuf[i % RND] > 0.4? (0.64f) : 0.0f;
  //      values[count*p + i] = rand()/float(RAND_MAX);
  //      values[count*p + i] = values[count*p + i] > 0.75f? 0.96 : values[count*p + i] > 0.4? (0.64f) : 0.0f;
      }
    }
    for (unsigned int i=0; i<drawscount; i++)
    {
      ((DrawIntensePoints*)draws[i])->setData( countByPortions, Xs, Ys, values);
    }
  }
  else
  {
    float (*manual_fn)(float, float) = nullptr;
    
    #pragma omp parallel
    for (int pm=0; pm<PORTIONS*MAXLINES; pm++)
    {
      int portion = pm / MAXLINES;
      float fmov01samples = ((g_movX + portion*portion*10)%DSAMPLES)/float(DSAMPLES);
      float fmov01sin = qFastSin((g_movX + portion*portion*10)/(M_PI*2*2))/2.0f + 0.5f;
      
      float* testbuf = &testbuf2D[pm*DSAMPLES];
      
      int idx_secondary = (int)(randomer[(idx_primary*DSAMPLES + pm) % (RND*DSAMPLES)]*(RND-1));
      if (pm != 0)
        randbuf = &randomer[ idx_secondary*DSAMPLES ];
      
      switch (sigtype)
      {
      case ST_SIN:
      {
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = (qFastSin((float(i)/DSAMPLES + fmov01samples)*2*M_PI) + 1)/2.0f;
        break;
      }
      case ST_MANYSIN:
      {
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = (qFastSin((float(i*10)/DSAMPLES + fmov01samples)*2*M_PI) + 1)/2.0f;
        break;
      }
      case ST_RAND:
      {
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = randbuf[i];
        break;
      }
      case ST_RAMP:
      {
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = i/float(DSAMPLES-1);
        break;
      }
      case ST_MOVE:
      {
        static const float mover[] = { 1.0, 0.9, 0.7, 0.5, 0.3 };
        int base = qRound(fmov01samples*DSAMPLES);
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
        {
          testbuf[i] = randbuf[i]*0.3f;
          unsigned int offs = qAbs(i - base);
          if (offs < sizeof(mover)/sizeof(float))
            testbuf[i] = mover[offs];
        }
        break;
      }
      case ST_ZERO: case ST_ZOZ: case ST_ZOO: case ST_OOZ:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
          testbuf[i] = 0;
        
        if (sigtype == ST_ZOZ)
          testbuf[0] = testbuf[DSAMPLES-1] = 1;
        else if (sigtype == ST_ZOO)
          testbuf[0] = 1;
        else if (sigtype == ST_OOZ)
          testbuf[DSAMPLES-1] = 1;
        break;
      }
      case ST_OZOZO:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
          testbuf[i] = i%2 == 0? 1.0f : 0.0f;
        break;
      }
      case ST_ONE:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
          testbuf[i] = 1;
        break;
      }
      case ST_PEAK:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
        {
          testbuf[i] = randbuf[i];
          if (testbuf[i] < 0.8f)
            testbuf[i] *= 0.3f;
        }
  //        testbuf[i] = randbuf[i] > 0.8f? 0.0f : randbuf[i];
        break;
      }
      case ST_PEAK2:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
        {
          testbuf[i] = randbuf[i];
          if (testbuf[i] < 0.9f)
            testbuf[i] *= 0.1f;
          else if (testbuf[i] < 0.98f)
            testbuf[i] = 0;
        }
        break;
      }
      case ST_PEAK3:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
        {
          testbuf[i] = randbuf[i];
          if (testbuf[i] < 0.95f)
            testbuf[i] *= 0.05f;
          else if (testbuf[i] < 0.99f)
            testbuf[i] = 0;
        }
        break;
      }
      case ST_STEP:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
          testbuf[i] = i*2/DSAMPLES;
        break;
      }
      case ST_GEN_NORM:
      {
        #pragma omp for
        for (int i = 0; i < DSAMPLES/2; i++)
        {
          testbuf[i] = randbuf[i];
          testbuf[DSAMPLES/2 + i] = randbuf[DSAMPLES/2 + i];
          generateGaussian(0.5, 0.2, &testbuf[i], &testbuf[DSAMPLES/2 + i]);
        }
        break;
      }
      case ST_HIPERB: case ST_SINXX: case ST_XX: case ST_TANHX:
      {
  //      float fX = float(mm)/MAXLINES - 0.5f;
        if (sigtype == ST_HIPERB)
          manual_fn = my_hiperb;
        else if (sigtype == ST_SINXX)
          manual_fn = my_sinxx;
        else if (sigtype == ST_XX)
          manual_fn = my_xx;
        else if (sigtype == ST_TANHX)
          manual_fn = my_tanhx;
  //      fX = manual_fn(fX, fmov01sin);
        #pragma omp for
  //      for (int i = 0; i < DSAMPLES; i++)
  //      {
  //        float value = manual_fn((i - DSAMPLES/2)/10.0f, fmov01sin);
  //        if (int((value+0.5f)*MAXLINES) >= mm)
  //          testbuf[i] = 1;
  //        else
  //          testbuf[i] = 0;
  //      }
        for (int i = 0; i < DSAMPLES; i++)
        {
          float value = manual_fn((i - DSAMPLES/2)/10.0f, fmov01sin) + 0.5f;
          testbuf[i] = value;
        }
        break;
      }
      case ST_10: case ST_100: case ST_1000: case ST_10000:
      {
        static const int spi[] = { 10, 100, 1000, 10000 };
        int sp = spi[sigtype - (int)ST_10];
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = ((g_movX2 + portion*5 + i)%sp) / float(sp - 1);
  //        testbuf[i] = (int(i + fmov01samples*DSAMPLES)%sp) / float(sp - 1);
        break;
      }
      default: break;
      }
        
      #pragma omp for
      for (int i=0; i<DSAMPLES; i++)
        testbuf[i] = testbuf[i]*sig_k + sig_b;
      
      if (pm % MAXLINES == 0)
      {
        if (manual_fn)
        {
          #pragma omp for
          for (int i=0; i<DSAMPLES; i++)
          {
            float X = (i - DSAMPLES/2)/10.0f;
            testbuf1D[portion*DSAMPLES + i] = manual_fn(X, fmov01sin)*sig_k + 0.5f + sig_b;
          }
        }
        else
        {
          #pragma omp for
          for (int i=0; i<DSAMPLES; i++)
            testbuf1D[portion*DSAMPLES + i] = testbuf[i];
        }
      }
    } /// PORTIONS*MAXLINES
    
  #ifndef DECIMATION
    for (unsigned int i=0; i<drawscount; i++)
      draws[i]->setData(draws[i]->directions()==2? (const float*)testbuf2D : (const float*)testbuf1D);
  #else
    for (unsigned int i=0; i<drawscount; i++)
    {
      DataDecimatorFunc ddf(DECIMATION, draws[i]->directions() == 1? DSAMPLES : DSAMPLES*MAXLINES);
      draws[i]->setData(draws[i]->directions()==2? (const float*)testbuf2D : (const float*)testbuf1D, &ddf);
    }
  #endif
  }
  
  g_movX++; g_movX2++;
  randomer_ctr++;
  
  
  if (ovl_marks != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OMarkDashs* pMKStrokes = (OMarkDashs*)draws[i]->ovlGet(ovl_marks);
      if (pMKStrokes)
      {
        for (unsigned int i=0; i<pMKStrokes->countMarks(); i++)
        {
          pMKStrokes->updateMark(i, i / 10.0f, randbuf[i%DSAMPLES]);
        }
        pMKStrokes->updateFinished();
      }
    }
  }
  
  if (ovl_figures != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OMarkFigures* pMKFigures = (OMarkFigures*)draws[i]->ovlGet(ovl_figures);
      if (pMKFigures)
      {
        int cf = pMKFigures->countFigures();
        #pragma omp parallel for
        for (int j=0; j<cf; j++)
        {
          float x = randbuf[j%DSAMPLES], y = randbuf[(cf - j - 1)%DSAMPLES];
          generateGaussian(0.5, 0.2, &x, &y);
          pMKFigures->updateFigure(j, x, y);
        }
        float nzm = randbuf[0];
        pMKFigures->updateFigureZoom(rand()%pMKFigures->countFigures(), 1 + nzm);
        pMKFigures->updateFinished();
      }
    }
  }
  
  if (ovl_sprites != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OSprites* pSprites = (OSprites*)draws[i]->ovlGet(ovl_sprites);
      if (pSprites)
        pSprites->update();
    }
  }
}


#if 0

#if __cplusplus >= 201101L
#include <array>
#include <iterator>
#include <algorithm>
#include <random>
#include <time.h>
//#endif

#include <qmath.h>

void MainWindow::generateData()
{
  
#if __cplusplus >= 201101L 
  std::mt19937  gen(time(0) + g_movX);
#endif
  
#if MW_TEST == DRAW_BRIGHT_CLUSTER
#if __cplusplus >= 201101L 
  static std::array<float, PORTIONS*DSAMPLES> pointsX, pointsY;
  typedef std::array<float, PORTIONS*DSAMPLES> arrsamples_t;
  arrsamples_t testbuf1D;
  {
    std::normal_distribution<float> ND(0.5, 0.1);
    std::generate(pointsX.begin(), pointsX.end(), [&ND, &gen](){ return ND(gen); } );
    std::generate(pointsY.begin(), pointsY.end(), [&ND, &gen](){ return ND(gen); } );
    std::replace_if(pointsX.begin(), pointsX.end(), [](float x){ return x < 0.0f; }, 0.0f);
    std::replace_if(pointsX.begin(), pointsX.end(), [](float x){ return x > 1.0f; }, 1.0f);
    std::replace_if(pointsY.begin(), pointsY.end(), [](float x){ return x < 0.0f; }, 0.0f);
    std::replace_if(pointsY.begin(), pointsY.end(), [](float x){ return x > 1.0f; }, 1.0f);
    
    std::uniform_real_distribution<float> RD(0.5, 0.8);
    std::generate(testbuf1D.begin(), testbuf1D.end(), [&RD, &gen](){ return RD(gen); } );
    for (arrsamples_t::iterator iter = testbuf1D.begin(); iter != testbuf1D.end(); ++iter)
      *iter = *iter*sig_k + sig_b;
  }
  std::array<unsigned int, PORTIONS> dsizes;
  std::fill_n(dsizes.begin(), PORTIONS, DSAMPLES);
  
  for (unsigned int i=0; i<drawscount; i++)
  {
//    unsigned int dsize = DSAMPLES;
//    ((DrawIntensePoints*)draws[i])->setData( &dsize, pointsX.begin(), pointsY.begin());
    ((DrawIntensePoints*)draws[i])->setData( dsizes.begin(), pointsX.begin(), pointsY.begin(), testbuf1D.begin());
  }
#endif
  
#else
  // -- Заполнение тестового буфера -- 
  static std::array<float,PORTIONS*MAXLINES*DSAMPLES> testbuf2D;
  static std::array<float,PORTIONS*DSAMPLES> testbuf1DP;
  std::uniform_real_distribution<> URD1(0.0f, 1.0f);
//#endif
  
  #pragma omp parallel
  {
    #pragma omp for
    for (int pm=0; pm<PORTIONS*MAXLINES; pm++)
    {
      int d = pm / MAXLINES;
      typedef std::array<float, DSAMPLES> arrsamples_t;
      arrsamples_t    testbuf1D;
      
      switch (sigtype)
      {
      case ST_SIN:
      {
        for (unsigned int i=0; i<testbuf1D.size(); i++)
          testbuf1D[i] = (qFastSin((float(i) / DSAMPLES) * 2 * M_PI + ((g_movX + d*10)*2*M_PI)/DSAMPLES) + 1)/2.0f;
        break;
      }
      case ST_MANYSIN:
      {
        for (unsigned int i=0; i<testbuf1D.size(); i++)
          testbuf1D[i] = (qFastSin((float(i*10) / DSAMPLES) * 2 * M_PI + ((g_movX + d*100)*2*M_PI)/DSAMPLES) + 1)/2.0f;
        break;
      }
      case ST_RAND: std::generate(testbuf1D.begin(), testbuf1D.end(), [&URD1, &gen](){ return URD1(gen); } ); break;
      case ST_RAMP:
      {
        float i=0.0f;
        std::generate(testbuf1D.begin(), testbuf1D.end(), [&i](){ return i++/DSAMPLES; } );
        break;
      }
      case ST_MOVE:
      {
        std::uniform_real_distribution<> URD(0.0f, 0.3f);
        std::generate(testbuf1D.begin(), testbuf1D.end(), [&URD, &gen](){ return URD(gen); } );
        static const std::array<float,5> multy{ 1.0, 0.9, 0.7, 0.5, 0.3 };
        for (int i = 0; i < (int)multy.size(); ++i)
        {
          int base = int(d*DSAMPLES/6.0f + g_movX);
          if (base + i < (int)testbuf1D.size())                     testbuf1D[base + i] = multy[i];
          if (base - i >= 0 && base - i < (int)testbuf1D.size())    testbuf1D[base - i] = multy[i];
        }
        break;
      }
      case ST_ZERO: case ST_BORDERS: std::fill(testbuf1D.begin(), testbuf1D.end(), 0.0f); break;
      case ST_ONE:  std::fill(testbuf1D.begin(), testbuf1D.end(), 1.0f); break;
      case ST_ZOZ:  std::fill(testbuf1D.begin(), testbuf1D.end(), 0.0f); testbuf1D.front() = 1.0f; testbuf1D.back() = 1.0f; break;
      case ST_ZOO:  std::fill(testbuf1D.begin(), testbuf1D.end(), 0.0f); testbuf1D.front() = 1.0f; break;
      case ST_OOZ:  std::fill(testbuf1D.begin(), testbuf1D.end(), 0.0f); testbuf1D.back() = 1.0f; break;
      case ST_PEAK:
      {
        std::generate(testbuf1D.begin(), testbuf1D.end(), [&URD1, &gen](){ return URD1(gen); } );
        std::replace_if(testbuf1D.begin(), testbuf1D.end(), [](float f){ return f > 0.8f; }, 0.0 );
        break;
      }
      case ST_PEAK2:
      {
        std::uniform_real_distribution<> URD(0.0f, 0.3f);
        std::generate(testbuf1D.begin(), testbuf1D.end(), [&URD, &gen](){ return URD(gen); } );
        std::replace_if(testbuf1D.begin(), testbuf1D.end(), [](float f){ return f > 0.28f; }, 1.0 );
        break;
      }
      case ST_STEP:
      {
        std::fill_n(testbuf1D.begin(), testbuf1D.size()/2, 0.0f);
        std::fill(testbuf1D.begin() + testbuf1D.size()/2, testbuf1D.end(), 1.0f);
        break;
      }
      case ST_GEN_NORM:
      {
        std::normal_distribution<float> ND(0.5f, 0.4f);
        std::generate(testbuf1D.begin(), testbuf1D.end(), [&ND, &gen](){ return ND(gen); } );
        break;
      }
      case ST_10: case ST_100: case ST_1000: case ST_10000: case ST_1000000:
      {
        int spi[] = { 10, 100, 1000, 10000, 1000000 };
        int sp = spi[sigtype - (int)ST_10];
        for (unsigned int i=0; i<testbuf1D.size(); i++)
          testbuf1D[i] = ((g_movX2 + i)%sp) / float(sp);
//          std::generate(testbuf1D.begin(), testbuf1D.end(), [&testbuf1D](){ return ; } );          
        break;
      }
      default: break;
      }
      
      for (arrsamples_t::iterator iter = testbuf1D.begin(); iter != testbuf1D.end(); ++iter)
        *iter = *iter*sig_k + sig_b;
      
      if (pm % MAXLINES == 0)
        std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf1DP.begin() + testbuf1D.size()*d);
      
      std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf2D.begin() + testbuf1D.size()*pm);
//      for (int y=1; y<MAXLINES; y++)
//        std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf2D.begin() + d*DSAMPLES*MAXLINES + testbuf1D.size()*y);
    } // for PORTIONS
  
    if (sigtype == ST_BORDERS)
    {
      #pragma omp for
      for (int d=0; d<PORTIONS; d++)
      {
        for(int y = 0; y < MAXLINES; ++y)
          for(int x = 0; x < DSAMPLES; ++x)
          {
            testbuf2D[d*MAXLINES*DSAMPLES + y*DSAMPLES + x] = x == 0 || y == 0 || x == DSAMPLES-1 || y == MAXLINES - 1 ? 1 : 0;
            if (y == 0) testbuf1DP[d*DSAMPLES + x] = x % 2;
          }
      }
    }
  } // parallel
  
  g_movX2++; 
    
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setData(draws[i]->directions() == 1? testbuf1DP.begin() : testbuf2D.begin());
  
#endif
  
  if (ovl_marks != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OMarkDashs* pMKStrokes = (OMarkDashs*)draws[i]->ovlGet(ovl_marks);
      if (pMKStrokes)
      {
        for (unsigned int i=0; i<pMKStrokes->countMarks(); i++)
        {
          pMKStrokes->updateMark(i, i / 10.0f, rand()/float(RAND_MAX));
        }
        pMKStrokes->updateFinished();
      }
    }
  }
  
  if (ovl_figures != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OMarkFigures* pMKFigures = (OMarkFigures*)draws[i]->ovlGet(ovl_figures);
      if (pMKFigures)
      {
        std::normal_distribution<float> ND(0.5, 0.1);
        for (unsigned int i=0; i<pMKFigures->countFigures(); i++)
        {
          pMKFigures->updateFigure(i, ND(gen), ND(gen));
        }
        float nzm = ND(gen)*2;
        pMKFigures->updateFigureZoom(rand()%pMKFigures->countFigures(), nzm*nzm*nzm);
        pMKFigures->updateFinished();
      }
    }
  }
  if (++g_movX >= DSAMPLES)
    g_movX = 0;
}

#else
void MainWindow::generateData()
{
  const unsigned int size2D = PORTIONS*MAXLINES*DSAMPLES;
  const unsigned int size1D = PORTIONS*DSAMPLES;
  static float testbuf2D[size2D];
  static float testbuf1DP[size1D];
  for (unsigned int i=0; i<size2D; i++)
  {
    testbuf2D[i] = rand()/float(RAND_MAX);
    if (i <= size1D)
      testbuf1DP[i] = testbuf2D[i];
  }
  
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setData(draws[i]->directions() == 1? testbuf1DP : testbuf2D);
}
#endif


#endif

void  MainWindow::updateAllDraws()
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->update();
}

void  MainWindow::changePaletteSTD(int id)
{
  if ((unsigned int)id >= sizeof(ppalettes_std)/sizeof(const IPalette*))
    return;
  
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPalette(ppalettes_std[id]);
}

void  MainWindow::changePaletteADV(int id)
{
  QPaletteBox* qpb = (QPaletteBox*)sender();
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPalette(!qpb->isInverted() ? ppalettes_adv[id] : ppalettes_adv_inv[id]);
}

void MainWindow::changePaletteRGB(int id)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPalette(ppalettes_rgb[id]);
}

void  MainWindow::changeFloats(int edid)
{
  float value;
  {
    QSignalMapper* pMap = (QSignalMapper*)sender();
    QLineEdit* pEd = (QLineEdit*)pMap->mapping(edid);
    QString text = pEd->text();
    bool ok;
    value = text.toFloat(&ok);
    if (!ok)
    {
      qDebug()<<"Conversion to float failed!";
      return;
    }
  }
  if (edid == ED_SIGB || edid == ED_SIGK)
  {
    if (edid == ED_SIGB)       sig_b = value;
    else if (edid == ED_SIGK)  sig_k = value;
    return;
  }
  
  for (unsigned int i=0; i<drawscount; i++)
  {
    if (edid == ED_LOW || edid == ED_HIGH)
    {
      if (edid == ED_LOW) draws[i]->setBoundLow(value);
      else if (edid == ED_HIGH) draws[i]->setBoundHigh(value);
    }
    else if (edid == ED_CONTRAST || edid == ED_OFFSET)
    {
      contrast_t cur = draws[i]->contrast();
      if (edid == ED_CONTRAST) cur.contrast = value;
      else if (edid == ED_OFFSET) cur.offset = value;
      draws[i]->setContrast(cur);
    }
  }
}

void  MainWindow::changeSpeedData(int id)
{
  speedDataTimer->stop();
  if (id == SP_STOP || id == SP_ONCE) ;
  else speedDataTimer->start(id);
}

void  MainWindow::changeSpeedData_Once()
{
//  speedDataTimer->stop();
  emit this->generateData();
}

void  MainWindow::changeSpeedUpdate(int id)
{
  speedUpdateTimer->stop();
  if (id == SP_STOP || id == SP_ONCE)      ;
  else  speedUpdateTimer->start(id);
}

void MainWindow::changeSpeedUpdate_Once()
{
//  speedUpdateTimer->stop();
  emit this->updateAllDraws();
}

void  MainWindow::changeFeatures(int id)
{
  ORIENTATION invHorz[] = {  OR_RLBT, OR_LRBT, OR_RLTB, OR_LRTB  };
  ORIENTATION invVert[] = {  OR_LRTB, OR_RLTB, OR_LRBT, OR_RLBT};
  for (unsigned int i=0; i<drawscount; i++)
  {
    if (id == BTF_CLEAR)          draws[i]->clearData();
    else if (id == BTF_INVHORZ)   draws[i]->setOrientation(invHorz[(int)draws[i]->orientation()]);
    else if (id == BTF_INVVERT)   draws[i]->setOrientation(invVert[(int)draws[i]->orientation()]);
    else if (id == BTF_HIDEOTHERS)
    {
      for (int j=0; j<DrawCore::OVLLIMIT; j++)
        if (j + 1 != ovl_visir)
        {
          IOverlay* povl = draws[i]->ovlGet(j + 1);
          if (povl) povl->setOpacity(1.0f);
        }
    }
  }
  if (id == BTF_DESTROYGRAPH)
  {
    if (drawscount > 0)
      delete draws[--drawscount];
  }
}

void MainWindow::changeInterpolation(int sigid)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setPaletteInterpolation(sigid == 1);
}

void MainWindow::changeBans(bool banned)
{
  int id = ((BSUOD_0*)sender()->userData(0))->id;
  for (unsigned int i=0; i<drawscount; i++)
  {
    if (id == BTF_BANUPDATEDATA)
    {
      draws[i]->banAutoUpdate(DrawCore::RD_BYDATA, banned);
    }
    else if (id == BTF_BANUPDATESETS)
    {
      draws[i]->banAutoUpdate(DrawCore::RD_BYSETTINGS, banned);
    }
    else if (id == BTF_BANUPDATEOVERLAYS)
    {
      draws[i]->banAutoUpdate(DrawCore::RD_BYOVL_ACTIONS, banned);
    }
  }
}

void MainWindow::createOverlaySTD(int id)
{
  if (drawscount == 0)
    return;
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->ovlClearAll();
  ovl_visir = -1;
  ovl_active_mark = -1;
  ovl_marks = -1;
  ovl_figures = -1;
  int ovl_tmp = -1;
  
  if (id == COS_OFF)
    return;

  for (unsigned int i=0; i<drawscount; i++)
  {
    switch (id)
    {
    case COS_DEKART:
    {
      draws[i]->ovlPushBack(new OBorder(1, linestyle_red(1,0,0)));
      draws[i]->ovlPushBack(new OGridDecart(CR_RELATIVE, 0.5, 0.5, 0.05, 0.05, 3));
      
      if (!ovl_is_synced || i == 0)
        ovl_visir = draws[i]->ovlPushBack(new OActiveCursor());
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_visir));
      
      draws[i]->ovlPushBack(new OFFactor(CR_ABSOLUTE, 0, 0, CR_ABSOLUTE_NOSCALED, 5, 30, linestyle_yellow(2,1,0)), ovl_visir);
      break;
    }
    case COS_GRIDS:
    {
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.5, 0.125, linestyle_greydark(5,1,0),-1));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.5, 0.125, linestyle_greydark(5,1,0),-1));
      
      if (!ovl_is_synced || i == 0)
      {
        ovl_visir = draws[i]->ovlPushBack(new OActiveCursor());
      }
      else
      {
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_visir));
      }
//      draws[i]->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_XREL_YREL_SCALED_NOSCALED, 0.01, 0.1, linestyle_green(0,0,1)), ovl_visir);
      draws[i]->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_XREL_YREL_NOSCALED_SCALED, 0.01, 0.1, linestyle_green(0,0,1)), ovl_visir);
      //draws[i]->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_RELATIVE_NOSCALED, 0.01, 0.01, linestyle_green(0,0,1)), ovl_visir);
      break;
    }
    case COS_GRIDSAXES:
    {
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
      draws[i]->ovlPushBack(new OGridDecart(CR_RELATIVE, 0.05, 0.05, 0.1, 0.1, 3));
      break;
    }
    case COS_CIRCULAR:
    {
      draws[i]->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.5, 0.5, CR_ABSOLUTE, 0, 10, linestyle_grey(1,0,0)));
      draws[i]->ovlPushBack(new OGridCircular(CR_RELATIVE, 0.5, 0.5, CR_RELATIVE, 0.1, linestyle_grey(1,0,0)));
      draws[i]->ovlPushBack(new OToons(CR_RELATIVE, 0.95, 1, linestyle_greydark(1,0,0)));
      break;
    }
    case COS_SELECTOR:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new OSelector(linestyle_bluelight(13,2,2), 0.2f));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextStatic("Press left mouse button and move", CR_RELATIVE, 0.05, 0.05, 12, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_DROPLINES:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new ODropLine(25, true, linestyle_white(5,2,0)));
//        ovl_tmp = draws[i]->ovlPushBack(new OBrush(1000, linestyle_white(5,2,0)));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextStatic("Press left mouse button", CR_RELATIVE, 0.05, 0.05, 12, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_CLUSTER:
    {
      if (!ovl_is_synced || i == 0)
      {
        OMarkFigures* pMKFigures = new OMarkFigures(500, CR_ABSOLUTE_NOSCALED, 4, &paletteBkWh, false);
        for (unsigned int j=0; j<pMKFigures->countFigures(); j++)
        {
          pMKFigures->updateFigureForm(j, (OMarkFigures::FFORM)( 1 + j % 5));
  //        pMKFigures->updateFigureColor(i, 1.0 - 0.15f*(rand()%4));
  //        pMKFigures->updateFigureColor(i, (1+rand()%5)*0.15f);
          pMKFigures->updateFigureColor(j, rand()/float(RAND_MAX));
        }
        ovl_figures = draws[i]->ovlPushBack(pMKFigures);
      }
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_figures));
      break;
    }
    case COS_FOLLOWERS:
    {
      draws[i]->ovlPushBack(new OBorder(2, linestyle_yellow(4,1,0)));
      if (!ovl_is_synced || i == 0)
        ovl_visir = draws[i]->ovlPushBack(new OActiveCursor());
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_visir));
      draws[i]->ovlPushBack(new OFCross(CR_RELATIVE, 0, 0, CR_ABSOLUTE, 5, 30, linestyle_yellow(2,1,0)), ovl_visir);
      draws[i]->ovlPushBack(new OTextStatic("LEFT", CR_ABSOLUTE, -80, 50, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextStatic("RIGHT", CR_ABSOLUTE, 50, 50, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextStatic("We all", CR_ABSOLUTE, -25, -60, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextStatic("Follow mouse", CR_ABSOLUTE, -50, -80, 12, false, linestyle_red(1,0,0)), ovl_visir);
      break;
    }
    case COS_INSIDE:
    {
      float opacity = 0.1f;
      for (int j=7; j>=0; j--)
      {
        IOverlay* ovl = new OFSquareCC(opacity, CR_RELATIVE, 0.5f, 0.5f, CR_SAME, 0.04*(j+1), linestyle_redlight(1,0,0));
        draws[i]->ovlPushBack(ovl);
      }
      break;
    }
    case COS_COVERL:
    {
      draws[i]->ovlPushBack(new OCover(0.1f, 0.65f, 0.2f, 0.2f, 0.2f, OCover::COP_SAVEUPPER));
      break;
    }
    case COS_COVERH:
    {
//      draws[i]->ovlPushBack(new OCover(0.7f, 1.0f, 0.2f, 0.2f, 0.2f, OCover::COP_COVER));
      draws[i]->ovlPushBack(new OCover(0.7f, 1.0f, 0, OCover::COP_COVER));
//      draws[i]->ovlPushBack(new OSlice(5.0, 0.0f, 0.4f, 0.0));
//      draws[i]->ovlPushBack(new OSlice(0.0, 1));
      break;
    }
    case COS_CONTOUR:
    {
      draws[i]->ovlPushBack(new OContour(0.95f, 1.0f, linestyle_inverse_1(2,1,0)));
      break;
    }
    case COS_SPRITEALPHA:
    {
      {
        if (!ovl_is_synced || i == 0)
          ovl_visir = draws[i]->ovlPushBack(new OActiveCursor());
        else
          draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_visir));
        QImage img(img_path_sprite);
        draws[i]->ovlPushBack(new OImageOriginal(&img, IOverlaySimpleImage::IC_AUTO, false, CR_PIXEL, -24, 0, 0.2f, 0.2f), ovl_visir);
      }
      break;
    }
    case COS_FOREGROUND:
    {
      QImage  img(img_path_normal);
      draws[i]->ovlPushBack(new OImageStretched(&img, IOverlaySimpleImage::IC_BLOCKALPHA, false));
      break;
    }
    case COS_BACKGROUND:
    {
      QImage  img(img_path_normal);
      IOverlay* ovl = new OImageOriginal(&img, IOverlaySimpleImage::IC_AUTO, false, CR_RELATIVE, 0.0f, 0.0f);
      ovl->setSlice(0.0f);
      draws[i]->ovlPushBack(ovl);
      break;
    }
    default: break;
    }
  }
}

void MainWindow::createOverlayADD()
{
  if (drawscount == 0)
    return;
  
  QButtonGroup* qbgCount = (QButtonGroup*)((BSUOD_2*)sender()->userData(1))->rawlink;
  const unsigned int counts[] = { 3, 1, 5, 10,  1,1,1 };

  unsigned int count = counts[qbgCount->checkedId()];
  
  QButtonGroup* qbgForm = (QButtonGroup*)((BSUOD_2*)sender()->userData(0))->rawlink;
  for (unsigned int i=0; i<count; i++)
  {
    float opacity = 0.2f;
    float centerX = rand() / float(RAND_MAX), centerY = rand()/float(RAND_MAX);
    float abssize = 10;
    linestyle_t kls = linestyle_green(1,0,0);
    
    IOverlay* ovl = nullptr;
    switch (qbgForm->checkedId())
    {
    case 0: ovl = new OFCircle(opacity, CR_RELATIVE, centerX, centerY, CR_ABSOLUTE, abssize, kls); break;
    case 1: ovl = new OFTriangle(OFTriangle::ORIENT_UP, opacity, CR_RELATIVE, centerX, centerY, CR_ABSOLUTE, abssize, kls); break;
    case 2: ovl = new OFSquareCC(opacity, CR_RELATIVE, centerX, centerY, CR_ABSOLUTE, abssize, kls); break;
    case 3: ovl = new OFCross(CR_RELATIVE, centerX, centerY, CR_ABSOLUTE, 0, abssize, kls); break;
    case 4: ovl = new OFPoint(CR_RELATIVE, centerX, centerY, kls); break;
    default: break;
    }
    if (ovl)
      for (unsigned int j=0; j<drawscount; j++)
        draws[j]->ovlPushBack(ovl);
  }
}

void MainWindow::setOverlaySync(bool value)
{
  ovl_is_synced = value;
}

void MainWindow::changeOVL(int id)
{
  active_ovl = id;
}


void MainWindow::changeOVLOpacity(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    IOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setOpacity(op/100.0f);
  }
}

void MainWindow::changeOVLSlice(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    IOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setSlice(op/100.0f);
  }
}

void MainWindow::changeOVLWeight(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    IOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setDensity(op);
  }
}

void MainWindow::changeOVLForm(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    _IOverlayLined* povl = dynamic_cast<_IOverlayLined*>(draws[i]->ovlGet(active_ovl));
    if (povl)
    {
      linestyle_t kls = povl->getLineStyle();
      switch (((BSUOD_0*)sender()->userData(0))->id)
      {
      case 0: kls.lenstroke = value; break;
      case 1: kls.lenspace = value; break;
      case 2: kls.countdot = value; break;
      }
      povl->setLineStyle(kls);
    }
  }
}

void MainWindow::changeOVLPos(int id)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    OVLCoordsDynamic* povl = dynamic_cast<OVLCoordsDynamic*>(draws[i]->ovlGet(active_ovl));
    if (povl)
    {
      float x,y;
      povl->getCoordinates(&x, &y);
      if (povl->getCoordination() == CR_ABSOLUTE)
        povl->setCoordinates(x + (id == BTOP_LEFT? -1 : id == BTOP_RIGHT? 1 : 0), y + (id == BTOP_UP? 1 : id == BTOP_DOWN? -1 : 0));
      else
        povl->setCoordinates(x + (id == BTOP_LEFT? -0.05f : id == BTOP_RIGHT? 0.05f : 0.0f), y + (id == BTOP_UP? 0.05f : id == BTOP_DOWN? -0.05f : 0.0f));
    }
    else
      qDebug()<<"OVL Position: dynamic_cast failure!";
  }
}

void MainWindow::changeOVLFeatures(int id)
{
  if (id == BTO_COLOR)
  {
    QColor clr;
    QColorDialog qdg;
    qdg.show();
    if (qdg.exec() != QColorDialog::Accepted)
      return;
    clr = qdg.currentColor();
    
//    QButtonGroup* snd = (QButtonGroup*)sender();
//    snd->buttons()[0]->setChecked(true);
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      _IOverlayLined* povl = dynamic_cast<_IOverlayLined*>(draws[i]->ovlGet(active_ovl));
      if (povl)
      {
        linestyle_t kls = linestyle_update(povl->getLineStyle(), clr.redF(), clr.greenF(), clr.blueF());
        povl->setLineStyle(kls);
      }
      else
      {
        qDebug()<<"OVL Features: dynamic_cast failure!";
      }
    }
  }
  else if (id == BTO_NOINV || id == BTO_INV1 || id == BTO_INV2 || id == BTO_INV3 || id == BTO_INV4 || id == BTO_INV5)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      _IOverlayLined* povl = dynamic_cast<_IOverlayLined*>(draws[i]->ovlGet(active_ovl));
      if (povl)
      {
        linestyle_t kls = povl->getLineStyle();
        kls.inversive = id == BTO_NOINV? -1 : (id - BTO_INV1);
        povl->setLineStyle(kls);
      }
    }
  }
}

//IOverlay* MainWindow::createNewOverlay(BTN_VISIR btv)
//{
//  IOverlay*  newOverlay = nullptr;
//  switch (vistype)
//  {
//    case BTV_CIRCLE:  newOverlay = new OFCircle(false, ocs, 0.0, 0.0, true, 100, linestyle_green(1,0,0)); break;
//    case BTV_SQUARE:  newOverlay = new OFSquareCC(false, ocs, 0.0, 0.0, true, 100, linestyle_green(1,0,0)); break;
//    case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, ocs, 0.0, 0.0, 10, 100, linestyle_green(1,0,0)); break;
//    case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, ocs, 0.0, 0.0, 10, 100, linestyle_green(1,0,0)); break;
//    case BTV_FACTOR:  newOverlay = new OFFactor(ocs, 0.0, 0.0, 10, 30, linestyle_yellow(5,1,0)); break;
//    case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, ocs, 0.0, 0.0, 10, -1, linestyle_green(1,0,0)); break;
//    case BTV_TEXT: newOverlay = new OTextStatic("CREATED", CR_RELATIVE, 0.5, 0.5, 12, true, linestyle_solid(1,0,0)); break;
//    case BTV_BORDER: newOverlay = new OBorder(4, linestyle_solid(1,0,0)); break;
//    default: break;
//  }
//  return newOverlay
//}

void MainWindow::metaOVLReplace(int vistype)
{
  if (active_ovl != 0)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      if (vistype == BTV_REMOVE)
        draws[i]->ovlRemove(active_ovl);
      else
      {
        IOverlay* ovl = draws[i]->ovlGet(active_ovl);
        OVLCoordsStatic* ocs = dynamic_cast<OVLCoordsStatic*>(ovl);
        if (ocs == nullptr)
        {
          qDebug()<<"OVL Replace: dynamic_cast failure!";
          continue;
        }
        IOverlay*  newOverlay = nullptr;
        switch (vistype)
        {
          case BTV_CIRCLE:  newOverlay = new OFCircle(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_SQUARE:  newOverlay = new OFSquareCC(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_FACTOR:  newOverlay = new OFFactor(ocs, 0.0, 0.0, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
          case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
          case BTV_TEXT: newOverlay = new OTextStatic("CREATED", ocs, 0.0, 0.0, 12, true, linestyle_solid(1,0,0)); break;
          case BTV_BORDER: newOverlay = new OBorder(4, linestyle_solid(1,0,0)); break;
          default: break;
        }
        if (newOverlay)
        {
          draws[i]->ovlReplace(active_ovl, newOverlay, true);
        }
      }
    }
  }
}

void MainWindow::metaOVLCreate(int vistype)
{
//  if (active_ovl != 0)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      IOverlay*  newOverlay = nullptr;
      COORDINATION cr = CR_RELATIVE;
      switch (vistype)
      {
        case BTV_CIRCLE:  newOverlay = new OFCircle(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_SQUARE:  newOverlay = new OFSquareCC(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_FACTOR:  newOverlay = new OFFactor(cr, 0.5, 0.5, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
        case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, cr, 0.5, 0.5, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
        case BTV_TEXT: newOverlay = new OTextStatic("CREATED", CR_RELATIVE, 0.5, 0.5, 12, true, linestyle_solid(1,0,0)); break;
        case BTV_BORDER: newOverlay = new OBorder(4, linestyle_solid(1,0,0)); break;
        default: break;
      }
      if (newOverlay)
      {
        draws[i]->ovlPushBack(newOverlay, true);
      }
    }
  }
}

void MainWindow::changeSigtype(int st)
{
  sigtype = (BTN_SIGTYPE)st;
}

void MainWindow::changeScaling(int value)
{
  SCALING to = (SCALING)((BSUOD_0*)sender()->userData(0))->id;
  for (unsigned int i=0; i<drawscount; i++)
  {
    if (to == SC_MIN_H || to == SC_MAX_H)
    {
      unsigned int scmin, scmax;
      draws[i]->scalingLimitsH(&scmin, &scmax);
      if (to == SC_MIN_H) scmin = value;
      else scmax = value;
      draws[i]->setScalingLimitsH(scmin, scmax);
    }
    else if (to == SC_MIN_V || to == SC_MAX_V)
    {
      unsigned int scmin, scmax;
      draws[i]->scalingLimitsV(&scmin, &scmax);
      if (to == SC_MIN_V) scmin = value;
      else scmax = value;
      draws[i]->setScalingLimitsV(scmin, scmax);
    }
  }
}

void MainWindow::changeMarkData(int ivalue)
{
  int mf = ((BSUOD_0*)sender()->userData(0))->id;
  if (mf >= _MF_COUNT || ovl_marks == -1)
    return;
    
  float value = ivalue/100.0f;
  for (unsigned int i=0; i<drawscount; i++)
  {
    OMarkDashs*  omarks = (OMarkDashs*)draws[i]->ovlGet(ovl_marks);
    if (mf == MF_TCOLOR)
      omarks->updateMarkColor(ovl_active_mark, value);
    else if (mf == MF_POS)
      omarks->updateMarkPos(ovl_active_mark, value);
    omarks->updateFinished();
  }
}

void MainWindow::changeClusterPalette()
{
  if (ovl_figures != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OMarkFigures* pMKFigures = dynamic_cast<OMarkFigures*>(draws[i]->ovlGet(ovl_figures));
      if (pMKFigures)
      {
        pMKFigures->setPalette(ppalettes_adv[rand() % sizeof(ppalettes_adv)/sizeof(ppalettes_adv[0])], false);
      }
    }
  }
}
