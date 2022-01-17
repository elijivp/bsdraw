/// MainWindow for main example
/// Created By: Elijah Vlasov
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
#include "bsdrawscales.h"
#include "specdraws/bsdrawempty.h"
#include "specdraws/bsdrawsdpicture.h"
#include "specdraws/bsdrawpolar.h"
#include "specdraws/bsdrawhint.h"

#include "overlays/bsinteractive.h"
#include "overlays/bsgrid.h"
#include "overlays/bspoints.h"
#include "overlays/special/bsblocker.h"
#include "overlays/bstextstatic.h"
#include "overlays/bsfigures.h"
#include "overlays/bssprites.h"
#include "overlays/special/bsmarks.h"
#include "overlays/special/bssnowflake.h"
#include "overlays/bsborder.h"
#include "overlays/bscontour.h"
#include "overlays/bsimage.h"
#include "overlays/special/bsbounded.h"

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

class MarginDebug: public MarginElement
{
  QString   caption;
  int       space;
public:
  MarginDebug(const QString& cap, int _space=20): caption(cap), space(_space) {}
protected:
  virtual bool  updateArea(const uarea_t& /*uarea*/, int /*UPDATEFOR*/){ return false; }
  virtual void  draw(QPainter&){}
  virtual void  mouseEvent(MOUSEEVENT mev, int pos_segm, int pos_atto, int dimm_segm, int dimm_atto, bool* /*doUpdate*/, MEQWrapper* /*selfwrap*/)
  {
    static const char* mevnames[] = { "LPRE", "LREL", "LMOV", "RPRE", "RREL", "RMOV" };
    Q_ASSERT(int(mev) < sizeof(mevnames)/sizeof(const char*));
    qDebug()<<caption<<"   "<<mevnames[mev]<<":  ->"<<pos_atto<<"("<<dimm_atto<<")"<<"    | "<<pos_segm<<dimm_segm;
  }
  virtual void  sizeHint(ATTACHED_TO /*atto*/, int* atto_size, int* mindly, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = space;
    *minsegm_pre = *minsegm_post = *mindly = 0;
  }
  virtual void  relatedInit(const DrawQWidget*){  }
  virtual void  changeColor(const QColor&){ }
};

/// Widget weight for layout
enum LAYOUT_WEIGHT{ LW_1, LW_0111, LW_1000, LW_012, LW_1110 } 
                  lw = LW_1;

/// startup speed
enum SPEED_TIMER { SP_STOP=0, SP_ONCE=1, SP_SLOWEST=1000, SP_SLOW=300, SP_FAST=40, SP_FASTEST=20 }
                  sp = SP_SLOW;

const IPalette*   defaultPalette = nullptr;


/// images for Overlays: sprite, foreground, background
static const char* img_path_mikey = "../example/mikey.jpg";  /// 1920x816
static const char* img_path_sprite =  "../example/snowflakewhite.png";
static const char* img_path_normal = "../example/image2.jpg";  /// 800x600
//static const char* img_path_normal = "../example/mikey.jpg";  /// 1920x816
//static const char* img_path_normal = "../example/image3.jpg";  /// 200x150
static const char* img_path_sdp = "../example/template.png";  /// 700x800

/// if uncomment: data generation will be more in 5 times. setData called with decimation algo
//#define DECIMATION window_min
//#define DECIMATION window_max

/// if uncomment: changes 'point size' for width and height synchroniously
//#define SYNCSCALING 3

/// if uncomment: DrawScales usage
//#define USESCALES

class BSUOD_DPM: public QObjectUserData
{
public:
  unsigned int  id;
  DPostmask*    dpm;
  BSUOD_DPM(unsigned int _id, DPostmask* _dpm): id(_id), dpm(_dpm) {} 
  ~BSUOD_DPM(){ if (id == 0) delete dpm; }
};

struct test_rgb_p
{
  struct  compo
  {
    int  start, stop, clamp;
    
    int   get(float step)
    {
      int result = start + qRound(step*(stop - start));
      if (clamp && result > clamp) result = clamp;
      return result;
    }
  } r, g, b;
};

enum  { C_R, C_G, C_B };
struct myrgb_t
{
  uchar t[3];
};
//inline unsigned int cast(const myrgb_t& clr){  return clr.t[C_R] << 16 | clr.t[C_G] << 8 | clr.t[C_B]; }
inline myrgb_t cast(unsigned int clr){  myrgb_t result = { uchar(clr & 0xFF), uchar((clr >> 8) & 0xFF), uchar((clr >> 16) & 0xFF) }; return result; }



const IPalette* const ppalettes_loc_std[] = {  &paletteBkWh, &paletteBkGyGyGyWh, &paletteGnYe, &paletteBlWh, &paletteBkRdWh, &paletteBkBlWh, &paletteBkGrWh, &paletteBkBlGrYeWh };
const IPalette* const ppalettes_rgb[] = {  &paletteRG, &paletteRB, &paletteRGB, &paletteBGR };


template <typename T>
QString palette2string(const QString& name, const T& pptr, unsigned int clc)
{
  QString nc = QString("colors_rgb_%1").arg(name);
  QString pc = QString("palette%1").arg(name), pci = QString("palette%1_inv").arg(name);
  QString result = QString("const unsigned int %1[] = { ").arg(nc) + "\n";
  QString part="  ";
  for (int i=0; i<clc; i++)
  {
    part += QString().sprintf(i == clc-1? "0x%08x" : "0x%08x,", pptr[i]);
    if ((i+1) % (clc/4) == 0)
    {
      result += part + "\n";
      part = "  ";
    }
  }
  result += part;
  result += "};\n";
  result += QString("const PaletteConstFWD<sizeof(%1) / sizeof(unsigned int)>   %2(%1);").arg(nc).arg(pc) + "\n";
  result += QString("const PaletteConstBWD<sizeof(%1) / sizeof(unsigned int)>   %2(%1);").arg(nc).arg(pci) + "\n";
  return result;
}


MainWindow::MainWindow(tests_t testnumber, QWidget *parent):  QMainWindow(parent), 
  MW_TEST(testnumber), randomer(nullptr), active_ovl(0), ovl_visir(-1), ovl_marks(-1), ovl_figures(-1), ovl_snowflake(-1), ovl_active_mark(9), ovl_is_synced(true), sigtype(ST_PEAK3), sig_k(1), sig_b(0)
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
    SAMPLES = 280;
    MAXLINES = 200;
    PORTIONS = 3;
    PRECREATE(3, 3);
    draws[0] = new DrawIntensity(SAMPLES, MAXLINES, 1);
    draws[0]->ovlPushBack(new OTextColored("Intensity", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x77FFFFFF, 0x00000000));
    draws[3] = new DrawDomain(SAMPLES, MAXLINES, 1, false, OR_LRBT, true);
    draws[3]->ovlPushBack(new OTextColored("Domain", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x77FFFFFF, 0x00000000));
    {
      DIDomain& ddm = *((DrawDomain*)draws[3])->domain();
      
      for (int j=0; j<SAMPLES; j++)
      {
        ddm.start();
        for (int r=0; r<MAXLINES/2; r++)
          ddm.includePixel(int(MAXLINES/2 + sin(j/(2.0*M_PI*8))*MAXLINES/4 - MAXLINES/4 + r), j);
        ddm.finish();
      }
    }
    draws[6] = new DrawRecorder(SAMPLES, MAXLINES);
    draws[6]->ovlPushBack(new OTextColored("Recorder", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_LRBT, 0x00000000, 0x77FFFFFF, 0x00000000));
    
    graphopts_t  gopts[] = { graphopts_t::goDots(), 
                             graphopts_t::goInterp(0.6f, DE_NONE), 
                             graphopts_t::goHistogramCrossMin()
                           };
    const char*  gnames[] = { "Graph (dots)", "Graph (linterp)", "Graph (histogram)" };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      draws[3*i + 1]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x77FFFFFF, 0x00000000));
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t) - 1; i++)
    {
      gopts[i].dotsize = i == 0? 3 : 2;
      gopts[i].dotsmooth = i == 0? 0.5f : 0.1f;
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], coloropts_t::copts(CP_RANGE, 1.0f, 0.5f, 0x00777777));
      draws[3*i + 2]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x77FFFFFF, 0x00000000));
    }
    
    draws[8] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(0.3f, DE_NONE, 0.15f));
    draws[8]->ovlPushBack(new OTextColored(gnames[2], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x77FFFFFF, 0x00000000));
    
    sigtype = ST_SINXX;
  }
  else if (MW_TEST == DEMO_2_scaling)  /// Demo 2
  {
    SAMPLES = 50;
    MAXLINES = 50;
    PORTIONS = 2;
    syncscaling = 4;
    PRECREATE(3, 3);
    DPostmask dpmcontour = DPostmask::postmask(PO_EMPTY, PM_LINELEFTBOTTOM, 0, 0.0f,0.0f,0.0f);
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
    
    graphopts_t  gopts[] = { graphopts_t::goDots(), 
                             graphopts_t::goInterp(0.3f, DE_NONE), 
                             graphopts_t::goHistogramCrossMax()
                           };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      draws[3*i + 1]->setPostMask(dpmcontour);
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      if (i == 0)
        draws[3*i + 2]->setPostMask(DPostmask::postmask(PO_ALL, PM_CIRCLEBORDERED, 0, 0.0f,0.1f,0.0f));
      else
        draws[3*i + 2]->setPostMask(DPostmask::postmask(PO_ALL, PM_LINELEFT, 1, 0.0f,0.0f,0.0f));
    }
    
    sigtype = ST_GEN_NORM;
  }  
  else if (MW_TEST == DEMO_3_overlays) /// std
  {
    SAMPLES = 600;
    MAXLINES = 200;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
      draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_NONE));
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, MAXLINES, 1000, PORTIONS);
    }
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEMO_4_portions) /// Different Portions
  {
    SAMPLES = 400;
    MAXLINES = 100;
    PORTIONS = 3;
    PRECREATE(3, 2);
    graphopts_t gopts = graphopts_t::goInterp(0.5f, DE_NONE);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, MAXLINES, i == 1? 1 : PORTIONS);
      if (i == 0)
        draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_SINGLE, 0.332f, 1.0f, 0x00111111));
      else
        draws[i*drcount + 1] = new DrawGraph(SAMPLES, 1, gopts, coloropts_t::copts(CP_RANGE, 0.0f, 1.0f, 0x00111111));
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, MAXLINES, 1000, i == 1? 1 : PORTIONS);
    }
    
    defaultPalette = (const IPalette*)&paletteRGB;
    sigtype = ST_SIN;
  }
  else if (MW_TEST == DRAW_BRIGHT)  /// brights for ovls
  {    
    SAMPLES = 100;
    MAXLINES = 100;
    PORTIONS = 1;
    
    PRECREATE(2, 2);
    unsigned int msc = 5;
    draws[0] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
    draws[0]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINELEFTBOTTOM, 0, 0.02f));
    draws[0]->setScalingLimitsSynced(msc, msc);
    
    draws[1] = new DrawIntensity(SAMPLES/2, 1, PORTIONS);
    draws[1]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINELEFT, 0, 0.2f));
    draws[1]->setScalingLimitsHorz(msc*2);
    draws[1]->setScalingLimitsVert(msc*2, msc*2);
    
    draws[2] = new DrawIntensity(SAMPLES/2, 1, PORTIONS);
    draws[2]->setOrientation(OR_TBLR);
    draws[2]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINELEFT, 0, 0.2f));
    draws[2]->setScalingLimitsHorz(msc*2);
    draws[2]->setScalingLimitsVert(msc*2, msc*2);
    
    draws[3] = new DrawIntensity(1, 1, PORTIONS);
    draws[3]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_CIRCLESMOOTH, 0, 0.2f));
    draws[3]->setScalingLimitsSynced(msc*2, msc*2);
    
    lw = LW_1000;    
  }
  else if (MW_TEST == DRAW_DOMAIN)   /// domain
  { 
    SAMPLES = 75;
    MAXLINES = 50;
    PORTIONS = 1;
    PRECREATE(2, 2);
    
    syncscaling = 5;
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, MAXLINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      for (int i=0; i<8; i++)
      {
        for (int j=0; j<10; j++)
        {
          int r = MAXLINES / 16 + i * MAXLINES / 8, c = int(SAMPLES/20.0 + j*SAMPLES/10.0);
          ddm.start();
          ddm.includePixel(r-1, c);
          ddm.includePixel(r, c);
          ddm.includePixel(r+1, c);
          ddm.includePixel(r, c+1);
          ddm.includePixel(r, c-1);
          ddm.finish();
        }
      }
      draws[0] = dd;
    }
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, MAXLINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
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
      draws[1] = dd;
    }
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, MAXLINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = 600;
      const unsigned int outsider = 1700;
      const double wc = 1.0/(2.0*M_PI);
      for (int i=0; i<maxspiral; i++)
      {
        int y = qRound(MAXLINES/2.0 + outsider*sin((i+1)*wc)/(i+1)), x = qRound(SAMPLES/2.0 + outsider*cos((i+1)*wc)/(i+1));
        if (y >= 0 && y < MAXLINES && x >= 0 && x < SAMPLES)
        {
          ddm.start();
            ddm.includePixelFree(y, x);
          ddm.finish();
        }
      }
//      qDebug()<<"Example: Total spiral _1_ points: "<<ddm.count();
      draws[2] = dd;
    }
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, MAXLINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = 600;
      const unsigned int outsider = 6000;
      const double wc = 3.0/(2.0*M_PI);
      for (int i=0; i<maxspiral; i++)
      {
        int y = qRound(MAXLINES/2.0 + outsider*sin((i+1)*wc)/(i+1)), x = qRound(SAMPLES/2.0 + outsider*cos((i+1)*wc)/(i+1));
        if (y >= 0 && y < MAXLINES && x >= 0 && x < SAMPLES && ddm.isFree(y, x))
        {
          ddm.start();
            ddm.includePixel(y, x);
          ddm.finish();
        }
      }
//      qDebug()<<"Example: Total spiral _2_ points: "<<ddm.count();
      draws[3] = dd;
    }
    SAMPLES = SAMPLES + MAXLINES - 1; /// reinit SAMPLES for future DSAMPLES data calculation
    
    sp = SP_FAST;
    sigtype = ST_MANYSIN;
  }
  else if (MW_TEST == DRAW_RECORDER)  /// recorders
  {
    SAMPLES = 300;
    MAXLINES = 400;
    PORTIONS = 1;
    PRECREATE(1, 2);
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawRecorder(SAMPLES, MAXLINES, 2000, PORTIONS);
  }
  else if (MW_TEST == DRAW_POLAR) /// polar draw
  {
    SAMPLES = 128;
    MAXLINES = 225;
//    PORTIONS = 4;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = MAXLINES > 30? 0 : (30-MAXLINES)*6;
    impulsedata_t imp[] = { { impulsedata_t::IR_OFF },
                            { impulsedata_t::IR_A_COEFF, 5, 5/2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
//                            { impulsedata_t::IR_A_COEFF, 3, 3/2, 0, { 0.25f, 0.5f, 0.25f } },
                            { impulsedata_t::IR_A_COEFF_NOSCALED, 5, 5/2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 4, 6, 1, {} },
                            { impulsedata_t::IR_A_BORDERS, 2, 2, 1, {} },
    };
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i] = new DrawPolar(SAMPLES, MAXLINES, PORTIONS, 0x00000000, SL_HORZ2);
      draws[i] = new DrawPolar(SAMPLES, MAXLINES, PORTIONS, 0, 0.0f, 0x00000000, SL_NONE);
      draws[i]->setImpulse(imp[1]);
//      draws[i]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_LINELEFT, 0, 0x00333333, 0.35f));
//      draws[i]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_LINELEFTBOTTOM, 0, 0x00333333, 0.2f));
//      ((DrawPolar*)draws[i])->turn(0f);
    }
//    sigtype = ST_GEN_NORM;
    sigtype = ST_MOVE;
//    sigtype = ST_ONE;
//    sigtype = ST_ZOO;
//    sp = SP_ONCE;
  }
  else if (MW_TEST == DRAW_GRAPHS) /// graphs and graphmoves
  {
    SAMPLES = 300;
    MAXLINES = 1;
    PORTIONS = 2;
    PRECREATE(3, 3);
    BSGRAPHTYPE     gts[] = { GT_HISTOGRAM_CROSSMAX, GT_LINTERP, GT_DOTS };
    BSCOLORPOLICY   dclr[] = { CP_SINGLE, CP_SINGLE, CP_RANGE };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        graphopts_t  gopts = {      gts[i],  DE_NONE,
                                    i == 0? c == dccount-1? 0.4f : 0.3f : 0.0f,
                                    i != 2? 0 : 2, 
                                    0.5f,
                                    i < 2? 0.4f : 0.2f,
                                    PR_STANDARD
                           };
//        gopts.smooth = 0.0f;
        coloropts_t copts = {   dclr[c], 1.0f, 0.5f, c == 1? 0xFFFFFFFF : 0x00999999 };
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gopts, copts);
      }
  
    sigtype = ST_SINXX;
  }
  else if (MW_TEST == DRAW_GRAPHS_MOVE) /// graphs and graphmoves
  {
    SAMPLES = 300;
    MAXLINES = 1;
    PORTIONS = 2;
    PRECREATE(3, 3);
    BSGRAPHTYPE gts[] = { GT_HISTOGRAM, GT_LINTERP, GT_DOTS };
    BSCOLORPOLICY   dclr[] = { CP_SINGLE, CP_SINGLE, CP_RANGE };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        graphopts_t  gopts = {      gts[i],  DE_NONE,
                                    i == 0? c == dccount-1? 0.4f : 0.3f : 0.0f,
                                    i != 2? 0 : 2,
                                    0.5f,
                                    0.5f,
                                    PR_STANDARD
                              };
        coloropts_t copts = {   dclr[c], 1.0f, 0.5f, c == 1? 0xFFFFFFFF : 0x00999999 };
        draws[c*drcount + i] = new DrawGraphMoveEx(SAMPLES, 5, SAMPLES*2, PORTIONS, gopts, copts);
      }

    sigtype = ST_GEN_NORM;
    sp = SP_FASTEST;
  }
  else if (MW_TEST == DRAW_HISTOGRAMS)   /// pixelation
  {
    SAMPLES = 80;
    MAXLINES = 20;
    PORTIONS = 3;
    PRECREATE(4, 1);
    syncscaling = 10;
    graphopts_t gts[] = { graphopts_t::goHistogram(), graphopts_t::goHistogramCrossMin(), graphopts_t::goHistogramCrossMax(), graphopts_t::goInterp(0.5f, DE_NONE) };
    DPostmask fsp[] = {   DPostmask::postmask(PO_SIGNAL, PM_LINELEFTTOP, 0, 0.3f,0.3f,0.3f), 
                          DPostmask::postmask(PO_SIGNAL, PM_LINELEFTTOP, 0, 0.3f,0.3f,0.3f), 
                          DPostmask::postmask(PO_SIGNAL, PM_LINELEFTTOP, 0, 0.3f,0.3f,0.3f), 
                          DPostmask::postmask(PO_ALL, PM_CIRCLESMOOTH, 0, 0.1f,0.1f,0.1f)
                           };
    
    const char* gnames[] = { "Histogram (cross over)", "Histogram (cross min)", "Histogram (cross max)", "Linterp + pseudocircle" };
    
    
    for (unsigned int i=0; i<drawscount; i++)
    {
//      gts[i].descaling = DE_LINTERP;
      if (i < 3)
        gts[i].postrect = PR_VALUEAROUND;
      else
      {
        gts[i].opacity = 0.8f;
        gts[i].smooth = -1.0f;
      }
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, gts[i]);
      draws[i]->setPostMask(fsp[i]);
      
      draws[i]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED_SCALED, 10.0f, 0.85f, 12, OO_INHERITED, 0x00000000, 0x33FFFFFF, 0x00000000));
    }

    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == DRAW_HISTOGRAMS_2)   /// pixelation2
  {   
    SAMPLES = 30;
    MAXLINES = 20;
    PORTIONS = 1;
    PRECREATE(4, 3);
    graphopts_t gts[] = { graphopts_t::goHistogramCrossMax(), graphopts_t::goHistogramCrossMax(), graphopts_t::goInterp(0.5f, DE_NONE) };
    BSPOSTRECT prs[] = { PR_STANDARD, PR_VALUEONLY, PR_VALUEAROUND, PR_SUMMARY };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
//        gts[c].postrect = prs[i];
//        gts[c].smooth = 0.05f;
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gts[c] + prs[i] );
        draws[c*drcount + i]->setScalingLimitsHorz(c == 2? 5 : 12);
        draws[c*drcount + i]->setScalingLimitsVert(c == 0 && i == 1? 12 : 5);
        draws[c*drcount + i]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_CONTOUR, c == 1? i == 2? 4 : 1 : 0));
      }
    
    sp = SP_SLOWEST;
    sigtype = ST_GEN_NORM;    
  }
  else if (MW_TEST == DRAW_SCALES_1)
  {
    SAMPLES = 180;
    MAXLINES = 50;
    PORTIONS = 1;
    PRECREATE(2, 1);
    for (int i=0; i<drawscount; i++)
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP), coloropts_t::copts(CP_SINGLE, 1.0f, 1.0f, 0x777777));
//    sigtype = ST_MOVE;
//    defaultPalette = ppalettes_adv[11];
  }
  else if (MW_TEST == DRAW_SCALES_2)
  {
    SAMPLES = 180;
    MAXLINES = 50;
    PORTIONS = 1;
    PRECREATE(4, 1);
//    syncscaling = 4;
    
//    DrawGraph::BSCOLORPOLICY cps[] = { CP_SINGLE, CP_OWNRANGE, CP_OWNRANGE_GROSS, CP_OWNRANGE_SYMMETRIC, CP_RANGE, CP_SUBPAINTED };
//    const char* cpnames[] = { "CP_SINGLE", "CP_OWNRANGE", "CP_OWNRANGE_GROSS", "CP_OWNRANGE_SYMMETRIC", "CP_RANGE", "CP_SUBPAINTED" };
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_QINTERP), coloropts_t::copts(CP_SINGLE, 1.0f, 0.3f, i == 2? 0x00AAAAAA : 0xFFFFFFFF));
//      draws[i]->setScalingLimitsHorz(7);
//      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.8f, 0.7f, 12, OO_INHERITED, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
//    this->setMinimumHeight(1000);
//    this->setMinimumWidth(1200);
    sigtype = ST_MOVE;
    defaultPalette = ppalettes_adv[12];
  }
  else if (MW_TEST == DRAW_SCALES_3)
  {
    SAMPLES = 180;
    MAXLINES = 50;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(2, 1.0f, DE_NONE), coloropts_t::copts(CP_SINGLE, 1.0f, 0.3f, 0xFFFFFFFF));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.5f, 0.125f, linestyle_inverse_1(4,1,0)));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.5f, 0.125f, linestyle_inverse_1(4,1,0)));
    }
    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == DRAW_SDPICTURE)
  {
    SAMPLES = 500;
    MAXLINES = 700;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawSDPicture(SAMPLES, MAXLINES, img_path_sdp /*"/home/elijah/Projects/schema/schemod.png"*/);
      draws[i]->setRawResizeModeNoScaled(true);
    }
//    draws[2]->setPostMask(DPostmask::postmask(PO_ALL, PM_DOTCONTOUR, 0, 0.0f, 0.0f));
    sigtype = ST_MANYSIN;
    sp = SP_FAST;
//    sigtype = ST_ZOO;
//    sp = SP_ONCE;
//    defaultPalette = &paletteSemaphoreWRYG;
    defaultPalette = &palette_idl_rainbow;
  }
  else if (MW_TEST == FEATURE_PORTIONS)
  {
    SAMPLES = 300;
    MAXLINES = 10;
    int PORTIONS_MIN = 3;
    PORTIONS = 29;
    PRECREATE(5, 1);
    
    graphopts_t  gopts = { GT_LINTERP, DE_LINTERP, 0.0f, 0, 0.0f, 0.6f, PR_STANDARD };
//    graphopts_t  gopts = { GT_DOTS, DE_NONE, 0.0f, 0, 0.0f, 0.2f, PR_STANDARD };
    draws[0] = new DrawGraph(SAMPLES, PORTIONS_MIN, gopts, coloropts_t::copts(CP_SINGLE, 0.332f, 1.0f));
    draws[0]->setScalingLimitsVert(1);
    
    draws[1] = new DrawRecorder(SAMPLES, 100, 100, PORTIONS_MIN);
    draws[1]->setDataTextureInterpolation(true);
    
    draws[2] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS_MIN);
    draws[2]->setDataTextureInterpolation(true);
    draws[2]->setScalingLimitsVert(10,10);

    draws[3] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_SINGLE, 0.332f, 1.0f), SL_VERT);
//    draws[3]->ovlPushBack(new OFLine(OFLine::LT_VERT_BYBOTTOM, CR_RELATIVE, 0.5f, 0.0f, CR_ABSOLUTE, 0));
    
    draws[4] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS, OR_LRBT, SL_VERT2);
    draws[4]->ovlPushBack(new OBorder(linestyle_solid(1.0f, 1.0f, 1.0f)));
//    setMinimumWidth(1200); ??? 

//    sigtype = ST_RAMP;
    for (int i=0; i<drawscount; i++)
      draws[i]->setDataPalette(i<3? (const IPalette*)&paletteRGB: (const IPalette*)&palette_mat_hot);
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == FEATURE_GRAPH_SMOOTH)
  {
    SAMPLES = 280;
    MAXLINES = 200;
    PORTIONS = 3;
    PRECREATE(3, 3);
    float smoothtest[] = { -1.0f, -0.3f, 0.0f,
                           0.2f,  0.3f,  0.4f,
                           0.6f,  0.8f,  1.0f
                         };
    graphopts_t  gopts = { GT_LINTERP, DE_NONE, 0.0f, 0, 0.0f, 0.5f, PR_STANDARD };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        gopts.smooth = smoothtest[i*dccount + c];
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gopts);
        
        {
          QString gname = QString("smooth ") + QString::number(gopts.smooth);
          draws[c*drcount + i]->ovlPushBack(new OTextColored(gname.toUtf8().data(), CR_RELATIVE, 0.55f, 0.05f, 12, OO_INHERITED, 0x00000000, 0x44FFFFFF, 0x00000000));
        }
      }
    
    sigtype = ST_HIPERB;
    defaultPalette = (const IPalette*)ppalettes_adv[47];
    
//    sigtype = ST_SINXX;
//    sp = SP_SLOWEST;
//    setMinimumSize(2400, 1000);
  }
  else if (MW_TEST == FEATURE_ORIENTS)
  {
    SAMPLES = 400;
    MAXLINES = 200;
    PORTIONS = 1;
    syncscaling = 0;
    PRECREATE(6, 1);
    ORIENTATION orients[] = { OR_LRBT, OR_TBLR, OR_BTRL, OR_BTLR, OR_TBRL, OR_RLTB };
    const char* ornames[] = { "LRBT",  "TBLR",  "BTRL",  "BTLR",  "TBRL",  "RLTB" };
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp2(0.5, DE_NONE), coloropts_t::copts(0x00111111));
      draws[i]->setOrientation(orients[i]);
      draws[i]->ovlPushBack(new OTextColored(ornames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f,
                                             12, i == 0 || i == drawscount-1? OO_BTRL : OO_LRBT, 0x00000000, 0x00FFFFFF, 0x00000000));
    }

    sigtype = ST_MOVE;
  }
  else if (MW_TEST == FEATURE_INTERPOLATION)
  {
    SAMPLES = 60;
    MAXLINES = 200;
    PORTIONS = 2;
    PRECREATE(8, 1);
    
    graphopts_t  gopts[] = { 
       graphopts_t::goInterp(0.2f, DE_LINTERP_SCALINGLEFT), 
       graphopts_t::goInterp(-1.0f, DE_LINTERP_SCALINGLEFT), 
       graphopts_t::goInterp(0.2f, DE_LINTERP_SCALINGCENTER),
       graphopts_t::goInterp(-1.0f, DE_LINTERP_SCALINGCENTER), 
      
       graphopts_t::goDots(0, 0.0f, DE_NONE), 
       graphopts_t::goInterp2(0.2f, DE_LINTERP), 
       graphopts_t::goInterp(0.2f, DE_SINTERP), 
       graphopts_t::goInterp(0.2f, DE_QINTERP), 
      
    };
//    const char*  gnames[] = { "Original Data", "Linear interpolation", "Smooth by 3 points", "Smooth by 4 points" };
    const char*  gnames[] = { "Linear interp. Leftscaled", "Linear interp. Leftscaled, 8bit", "Linear interp. Centercaled", "Linear interp. Centerscaled, 8bit", 
                              "Original Data", "Linear interp.v2", "Smooth by 3 points", "Smooth by 4 points" };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      gopts[i].dotsize = 4;
//      gopts[i].dotsmooth = 2.0;
      gopts[i].dotsmooth = 0.8f;
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      draws[i]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.85f, 12, OO_INHERITED, 0x00000000, 0x44FFFFFF, 0x00000000));
      draws[i]->ovlPushBack(new OTextColored("Resize me", CR_RELATIVE, 0.15f, 0.97, 12, OO_TBLR, 0x00000000, 0x44FFFFFF, 0x00000000));
    }
    
    this->setMinimumWidth(1200);
    sigtype = ST_GEN_NORM;
    
//    sigtype = ST_MANYSIN;
//    sp = SP_ONCE;
  }
  else if (MW_TEST == FEATURE_POSTMASK)
  {
    SAMPLES = 20;
    MAXLINES = 14;
    PORTIONS = 1;
    syncscaling = 10;
    PRECREATE(6, 3);
    
//    SAMPLES = 10;
//    MAXLINES = 7;
//    PORTIONS = 1;
//    syncscaling = 10;
//    PRECREATE(3, 3);
    
    DPostmask dpms[] = {  
                          // 1st row
                          DPostmask::postmask(PO_OFF, PM_DOT, 0),
                          DPostmask::postmask(PO_EMPTY, PM_CONTOUR, 3, 0.0f, 0.5f),
                          DPostmask::postmask(PO_ALL, PM_CONTOUR, 3, 0.0f),
      
                          // 2nd row
                          DPostmask::postmask(PO_ALL, PM_LINELEFTBOTTOM, 0, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_DOTLEFTBOTTOM, 2, 1.0f, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_LINELEFTBOTTOM, 2, 0.0f, 0.4f, 0.9f),
      
                          // 3rd row
                          DPostmask::postmask(PO_ALL, PM_DOTCONTOUR, 0, 0.0f, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_SQUARES, 0, 0.0f, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_DOTCONTOUR, 3, 0.0f, 0.0f),
    
                          // 4th row
                          DPostmask::postmask(PO_ALL, PM_CIRCLESMOOTH, 0, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_CIRCLESMOOTH, 4, 0.0f),
                          DPostmask::postmask(PO_SIGNAL, PM_CIRCLESMOOTH, 0, 0.0f, 0.9f),
      
                          DPostmask::postmask(PO_SIGNAL, PM_FILL, 0, 0.0f, 0.8f),
                          DPostmask::postmask(PO_ALL, PM_DOT, 1, 0.0f, 0.0f),
                          DPostmask::postmask(PO_EMPTY, PM_SHTRICHL, 1, 0.5f, 0.5f, 0.5f, 0.3f),
                          
                          DPostmask::postmask(PO_ALL, PM_CONTOUR, 1, 0.0f, 0.0f, 0.0f),
                          DPostmask::postmask(PO_ALL, PM_CONTOUR, 1, 0.3f, 0.3f, 0.3f),
                          DPostmask::postmask(PO_ALL, PM_CONTOUR, 1, 1.0f, 1.0f, 1.0f),
                          
                       };
    
//    for (int i=0; i<sizeof(dpms)/sizeof(DPostmask); i++)
//      dpms[i] = DPostmask::postmask(PO_EMPTY, PM_CROSS, 0, 0.5f, 0.5f, 0.5f, i / 18.0f);
//    sp = SP_ONCE;
    
    
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        draws[c*drcount + i] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
        draws[c*drcount + i]->setPostMask(dpms[i*dccount + c]);
//        draws[c*drcount + i]->setScalingLimitsHorz(16);
//        draws[c*drcount + i]->setScalingLimitsVert(10,10);
      }
    draws[0]->ovlPushBack(new OTextColored("Original", CR_XABS_YREL_NOSCALED, 5.0f, 0.9f, 12, OO_INHERITED, 0x00000000, 0x11FFFFFF, 0x00000000));
    
    sigtype = ST_RAND;
    defaultPalette = (const IPalette*)ppalettes_adv[69];
  }
  else if (MW_TEST == FEATURE_COLOR_POLICY)
  {
    SAMPLES = 180;
    MAXLINES = 70;
    PORTIONS = 4;
    PRECREATE(7, 1);
//    syncscaling = 4;
    
    BSCOLORPOLICY cps[] = { CP_SINGLE, CP_OWNRANGE, CP_OWNRANGE_GROSS, CP_OWNRANGE_SYMMETRIC, CP_RANGE, CP_SUBPAINTED, CP_RANGESUBPAINTED };
    const char* cpnames[] = { "CP_SINGLE", "CP_OWNRANGE", "CP_OWNRANGE_GROSS", "CP_OWNRANGE_SYMMETRIC", "CP_RANGE", "CP_SUBPAINTED", "CP_RANGESUBPAINTED" };
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(cps[i], 1.0f, 0.3f));
//      draws[i]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_LINELEFTTOP, 0, 0.3f,0.3f,0.3f));
      draws[i]->setScalingLimitsHorz(7);
      
      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.8f, 0.7f, 12, OO_INHERITED, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
//    this->setMinimumHeight(1000);
//    this->setMinimumWidth(1200);
    sigtype = ST_MOVE;
    defaultPalette = ppalettes_adv[58];
  }
  else if (MW_TEST == VERTICAL)
  {    
    SAMPLES = 600;
    MAXLINES = 200;
    PORTIONS = 2;
    PRECREATE(4, 1);
    draws[0] = new DrawGraph(SAMPLES/8, PORTIONS, graphopts_t::goHistogram());
    draws[0]->setScalingLimitsB(8,8);
    draws[0]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_CONTOUR, 0, 0.3f,0.3f,0.3f));
    
    draws[1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_NONE));
    
    draws[2] = new DrawRecorder(SAMPLES, MAXLINES, 1000, PORTIONS);
    
    draws[3] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMin());
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i]->setMinimumWidth(MAXLINES);
      draws[i]->setOrientation(i != 3? OR_TBLR : OR_TBRL);
    }
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == IMPULSE_HORZ)
  {
    SAMPLES = 5;
    MAXLINES = 1;
    PORTIONS = 1;
    PRECREATE(5, 2);
    
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
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        draws[c*drcount + i] = new DrawIntensity(SAMPLES, MAXLINES, 1);
        draws[c*drcount + i]->setImpulse(imp[c*drcount + i]);
        draws[c*drcount + i]->setScalingLimitsA(50);
        draws[c*drcount + i]->setScalingLimitsB(50);
        draws[c*drcount + i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[c*drcount + i], 0, 10,2,10,2), CR_RELATIVE, 0.05f, 0.05f, 8, OO_INHERITED, 0x00000000, 0x11FFFFFF, 0x00000000));
      }
    sigtype = ST_RAMP;
//    sigtype = ST_SIN;
//    defaultPalette = ppalettes_adv[12];
  }
  else if (MW_TEST == IMPULSE_VERT)
  {
    SAMPLES = 1;
    MAXLINES = 5;
    PORTIONS = 1;
    PRECREATE(1, 5);
    
    impulsedata_t imp[] = { { impulsedata_t::IR_OFF },
                            { impulsedata_t::IR_B_COEFF, 5, 5/2, 0, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_B_COEFF_NOSCALED, 5, 5/2, 0, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_B_BORDERS_FIXEDCOUNT, 30, 4, 0, {} },
                            { impulsedata_t::IR_B_BORDERS, 30, 4, 0, {} },
    };
    const char* cpnames[] = { "ORIGINAL", "COEFF", "COEFF_NOSCALED", "BORDERS_FIXED", "BORDERS", "" };
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, MAXLINES, 1);
      draws[i]->setImpulse(imp[i]);
      draws[i]->setScalingLimitsA(50);
      draws[i]->setScalingLimitsB(50);
      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.05f, 0.05f, 8, OO_INHERITED, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
    sigtype = ST_PEAK;
//    defaultPalette = ppalettes_adv[12];
  }
  else if (MW_TEST == DRAW_BRIGHT_CLUSTER) /// bright cluster
  {
    SAMPLES = 30;
    MAXLINES = 30;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = 14;
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawIntensePoints(SAMPLES, MAXLINES, PORTIONS);
  }
  else if (MW_TEST == HINTS)
  {
    SAMPLES = 180;
    MAXLINES = 50;
    PORTIONS = 3;
//    PRECREATE(1 + PORTIONS, 1);
//    draws[0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP), coloropts_t::copts(CP_SINGLE, 0.5f, 1.0f, 0x777777));
//    for (int i=0; i<PORTIONS; i++)
//      draws[1 + i] = new DrawHint((DrawGraph*)draws[0], i, DH_LINE, OR_LRTB);
//    sigtype = ST_MOVE;
    int TD = PORTIONS+1;
    PRECREATE(TD, PORTIONS);
    for (int i=0; i<PORTIONS; i++)
    {
      draws[TD*i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP), coloropts_t::copts(CP_SINGLE, 0.5f, 1.0f, 0x777777));
      draws[TD*i]->setFixedHeight(600);
    }
    int flags[] = {
      DH_LINE, DH_FILL, DH_DIAGONAL,
      DH_SAW | 4, DH_TRIANGLE | 4, DH_MEANDER | 4,
      DH_SAW | 12, DH_TRIANGLE | 12, DH_MEANDER | 12
    };
    for (int i=0; i<PORTIONS; i++)
    {
      for (int j=0; j<PORTIONS; j++)
      {
        draws[1 + TD*i + j] = new DrawHint((DrawGraph*)draws[TD*i], i, flags[i*PORTIONS + j] | DH_AUTOMARGIN_8, OR_LRTB);
//        draws[1 + TD*i + j] = new DrawHint(0.25*i);
//        draws[1 + TD*i + j]->setScalingLimitsSynced(10,10);
      }
    }
    sigtype = ST_MOVE;
//    defaultPalette = ppalettes_adv[11];
  }
  else if (MW_TEST == ADV_PALETTES)    /// advanced palettes show
  {
    SAMPLES = 400;
    MAXLINES = 20;
    PORTIONS = 1;
    syncscaling = 1;
    
    PRECREATE(sizeof(ppalettes_adv) / sizeof(const IPalette*), 1);
//    PRECREATE(4, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, MAXLINES);
      draws[i]->setDataPalette(ppalettes_adv[i]);
      draws[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    }
    sigtype = ST_RAMP;
  }
  else if (MW_TEST == DEBUG_LOADING)
  {
    SAMPLES = 400;
    MAXLINES = 160;
    PORTIONS = 1;
    syncscaling = 0;
    PRECREATE(12, 4);
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t(GT_LINTERPSMOOTH, 0.0, 0x00111111), CP_SINGLE, 1.0, -0.5);
      draws[i] = new DrawGraphMove(SAMPLES, 1, PORTIONS, graphopts_t::goInterp2(DE_NONE), coloropts_t::copts(CP_SINGLE, 1.0, 0.5, 0x00111111));
    }
    
    sigtype = ST_10;
  }
  else if (MW_TEST == DEBUG_TABS)
  {
    SAMPLES = 80;
    MAXLINES = 120;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawRecorder(SAMPLES, MAXLINES, 1000, PORTIONS);
      draws[i]->setScalingLimitsSynced(2 + i, 2 + i);
      
    }
//    draws[2]->setPostMask(DPostmask::postmask(PO_ALL, PM_DOTCONTOUR, 0, 0.0f, 0.0f));
    draws[2]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINERIGHTTOP, 0, 0x00333333, 0.2f));

    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEBUG_VOCAB)
  {
    SAMPLES = 300;
    MAXLINES = 450;
    PORTIONS = 1;
    PRECREATE(1, 2);
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
      draws[i] = new DrawEmpty(SAMPLES, MAXLINES, 0x00333333);
    }
//    draws[2]->setPostMask(DPostmask::postmask(PO_ALL, PM_DOTCONTOUR, 0, 0.0f, 0.0f));
    sigtype = ST_10;
  }
  else if (MW_TEST == DEBUG_PALETTE2D)
  {
#if 0
    static const int clc = 172;
    static unsigned int colors[2][clc];
    int dm = clc/2;
    
    const char* schemaname[2] = { "R", "G" };
//    const char* postfix = "";
    const char* postfix = "_dark";
//    int rp1[][2] = { {25, 0x550000}, {59, 0x550000}, {25, 0x550000} };
//    int rgb_p1[][2] = { { 0,0xFF }, {0,0}, {0,0} };
//    int rgb_p2[][2] = { { 0,0 }, {0,0}, {0,0xFF} };
//    int rgb_p1[][2] = { { 0,0xA0 }, {0,0xED}, {0,0xA0} };
//    int rgb_p2[][2] = { { 0,0xED }, {0,0x99}, {0,0xA0} };
    test_rgb_p  p1 = { { 0,0xFF }, {0,0}, {0x0,0x0} };
    test_rgb_p  p2 = { { 0,0 }, {0,0xFF}, {0x0,0x0} };
    
    test_rgb_p* cc[][2] = {  { &p1, &p2 },
                             { &p2, &p1 }
                          };
    
    for (int c=0; c<2; c++)
    {
      for (int h=0; h<2; h++)
      {
        int cr=cc[c][h]->r.start;
        int cg=cc[c][h]->g.start;
        int cb=cc[c][h]->b.start;
        for (int i=0; i<dm; i++)
        {
          float step = float(i)/(dm-1);
          //mathf = [ x - (c*decay*(0.25 - (x-0.5)**2)) for c in coeffs ]
          float cs = step - ((0.25f - (step - 0.5f)*(step - 0.5f)));
          cr = cc[c][h]->r.get(cs);
          cg = cc[c][h]->g.get(cs);
          cb = cc[c][h]->b.get(cs);
          colors[c][dm*h + i] = (cb&0xFF) << 16 | (cg&0xFF) << 8 | (cr&0xFF);
        }
      }
    }
    qDebug()<<palette2string(QString("%1%2%3").arg(schemaname[0]).arg(schemaname[1]).arg(postfix), colors[0], clc).toStdString().c_str();
    qDebug()<<palette2string(QString("%1%2%3").arg(schemaname[1]).arg(schemaname[0]).arg(postfix), colors[1], clc).toStdString().c_str();
    
    static PalettePArray palettes[2] = { PalettePArray(colors[0], clc, false), PalettePArray(colors[1], clc, false) };
    const IPalette* pptr[2] = { &palettes[0], &palettes[1] };
    PORTIONS = 2;
#else
    static const int clc = 172;
    static PaletteBORDS<clc> brd0(0xFFFFFF);
    static PaletteBORDS<clc> brd1(0xFFFFFF);
    PaletteBORDS<clc>*  pbrd[] = { &brd0, &brd1 }; 
    float starter[] = { 0.2f, 0.3f };
    int   finaler = 0x9F;
    for (int b=0; b<2; b++)
    {
      int start = int(clc*starter[b]);
      for (int i=start; i<clc; i++)
      {
        float step = float(i - start)/(clc - start);
        float cs = step - ((0.25f - (step - 0.5f)*(step - 0.5f)));
        unsigned int cc = 0xFF * (1.0f - cs);
        unsigned int fc = 0xFF - (0xFF - finaler)*cs;
//        (*pbrd[b])[i] = ((*pbrd[b])[i] & 0x0000FF) | (cc << 16) | (cc << 8);
        (*pbrd[b])[i] = (fc) | (cc << 16) | (cc << 8);
//        (*pbrd[b])[i] = (fc << 8) | (cc << 16) | (cc);
//        (*pbrd[b])[i] = (fc << 8) | (fc) | (cc << 16);
      }
    }
    
    QString names[2];
    for (int b=0; b<2; b++)
      names[b] = QString("WR_0%1").arg(int(starter[b]*10));
    
    qDebug()<<palette2string(names[0], brd0, clc).toStdString().c_str();
    qDebug()<<palette2string(names[1], brd1, clc).toStdString().c_str();
    
    for (int b=0; b<2; b++)
    {
      QImage img(clc, 16, QImage::Format_RGB888);
      for (int j=0; j<16; j++)
      {
        myrgb_t*  bits = (myrgb_t*)img.scanLine(j);
        for (int i=0; i<clc; i++)
        {
          bits[i] = cast((*pbrd[b])[i]);
        }
      }
      img = img.scaled(clc*2, 16);
      img.save(QString("spec_") + names[b] + QString(".png"), nullptr);
    }
    
    const IPalette* pptr[2] = { &brd0, &brd1 };
    PORTIONS = 1;
#endif
    
    
    SAMPLES = 100;
    MAXLINES = 100;
    PRECREATE(1, 2);
    for (int i=0; i < drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
//      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(0.0f, DE_NONE), coloropts_t::copts(CP_SUBPAINTED, 1.0f, 1.0f, 0x00000000));
      draws[i]->setScalingLimitsSynced(6);
      draws[i]->setClearByPalette();
      draws[i]->setDataPalette(pptr[i]);
      draws[i]->setDataPaletteDiscretion(true);
    }
//    sigtype = ST_ONE;
    sp = SP_ONCE;
    
//    static PaletteBORDS<100> pptr(0x000000ff, 0.15f, 0x0000ff00, 0.85f, 0x000000ff);
//    defaultPalette = &pptr;
    
//    SAMPLES = 3;
//    MAXLINES = 100;
//    PORTIONS = 1;
//    PRECREATE(1, 1);
//    for (int i=0; i < 1; i++)
//    {
//      draws[i] = new DrawIntensity(SAMPLES, MAXLINES, PORTIONS);
//      draws[i]->setDataPaletteDiscretion(true);
//    }
//    sigtype = ST_RAMP;
//    sp = SP_ONCE;
    
//    static PaletteBORDS<100> pptr(0x000000ff, 0.3333f, 0x0000ff00, 0.6666f, 0x000000ff);
//    defaultPalette = &pptr;
  }
  else if (MW_TEST == DEBUG_ROTATIONSOVLS) 
  {
    SAMPLES = 23;
    MAXLINES = 51;
    PORTIONS = 1;
    
    PRECREATE(4, 2);
    OActiveCursor* oac = new OActiveCursor(CR_RELATIVE, -1,-1);
    
    
    for (unsigned int i=0; i<drcount; i++)
      for (unsigned int j=0; j<dccount; j++)
      {
        draws[i*dccount + j] = new DrawIntensity(SAMPLES, MAXLINES, 1, ORIENTATION(i*dccount + j));
        draws[i*dccount + j]->setScalingLimitsA(1,1);
        draws[i*dccount + j]->setScalingLimitsB(1,1);
        
        int id = draws[i*dccount + j]->ovlPushBack(oac);
        OFLine* oaf = new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_RELATIVE, 0.0f, -1, linestyle_white(1,0,0));
        draws[i*dccount + j]->ovlPushBack(oaf, id);
      }
    sigtype = ST_ZERO;
  }
  else if (MW_TEST == DEBUG_HISTO3)
  {   
    SAMPLES = 192;
    MAXLINES = 1;
    PORTIONS = 2;
    PRECREATE(3, 1);
    graphopts_t gts[] = { 
      graphopts_t::goHistogram(0.0f, DE_LINTERP, 0.6f), 
      graphopts_t::goHistogramMesh(0.0f, DE_LINTERP, 0.6f), 
      graphopts_t::goHistogramSum(0.0f, DE_LINTERP, 0.6f) 
    };
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
      {
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gts[c*drcount + i], coloropts_t::copts(CP_SINGLE, 1.0f, 0.49f) );
        draws[c*drcount + i]->setScalingLimitsHorz(1);
        draws[c*drcount + i]->setScalingLimitsVert(1);
//        draws[c*drcount + i]->setPostMask(DPostmask::postmask(PO_SIGNAL, PM_CONTOUR, 0));
      }
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
    draws[0]->setMinimumWidth(1000);
//    defaultPalette = &paletteBY;
    defaultPalette = &paletteRG;
  }
  else if (MW_TEST == DEBUG_MEVSCALES)
  {   
    SAMPLES = 192;
    MAXLINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP), coloropts_t::copts(CP_SINGLE, 1.0f, 0.49f));
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEBUG_MAINSCALE)
  {   
    SAMPLES = 512;
    MAXLINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP), coloropts_t::copts(CP_SINGLE, 1.0f, 0.49f));
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
  }
  
//  else if (MW_TEST == DEBUG_PALETTE2DX)
//  {
//#if false
//    static const int clc = 33;
//    static unsigned int colors[clc];
//    int dm = clc/3;
//    for (int i=0; i<dm; i++)
//    {
//      unsigned int up = (unsigned int)((i/float(dm-1))*255);
//      colors[dm*0 + i] = up << 8;
//      colors[dm*1 + i] = up << 8 | up << 0;
//      colors[dm*2 + i] = up << 0;
//    }
//#elif false
//    static const int clc = 24;
//    static unsigned int colors[clc];
//    int dm = clc/4;
//    for (int i=0; i<dm; i++)
//    {
//      colors[dm*0 + i] = 0xFFFFFF;
//      colors[dm*1 + i] = 0xFF << 0;
//      colors[dm*2 + i] = 0xFF << 8 | 0xFF << 0;
//      colors[dm*3 + i] = 0xFF << 8;
//    }
//#else
//  }
//  else if (MW_TEST == PROGRESS_BAR)   /// progress
//  {
//    sigtype = ST_10;
//    SAMPLES = 20;
//    MAXLINES = 1;
//    PORTIONS = 1;
//    PRECREATE(5, 1);
//    for (unsigned int i=0; i<drawscount; i++)
//    {
//      if (i<drawscount - 1)
//      {
//        draws[i] = new DrawIntensity(SAMPLES, 1, PORTIONS);
//        draws[i]->setScalingLimitsVert(10,10);
//        draws[i]->setScalingLimitsHorz(10,10);
//        if (i == 1)
//          draws[i]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_CONTOUR, 0, 0.3f,0.3f,0.3f));
//        else if (i == 2)
//          draws[i]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINELEFT, 0, 0.7f,0.7f,0.7f));
//        else if (i == 3)
//          draws[i]->setPostMask(DPostmask::postmask(PO_EMPTY, PM_CIRCLESMOOTH, 0, 0.0f,0.0f,0.0f));
//      }
//      else
//        draws[i] = new DrawGraph(SAMPLES, 1, graphopts_t(GT_DOTS));
//    }
//  }
  
  
  
  
#ifdef SYNCSCALING
  syncscaling = SYNCSCALING;
#endif
  
  {
    if (defaultPalette == nullptr)
      defaultPalette = (const IPalette*)&palette_gnu_latte;
    
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i]->setRawResizeModeNoScaled(true);
      
      if (MW_TEST != FEATURE_PORTIONS && MW_TEST != ADV_PALETTES && MW_TEST != DEBUG_PALETTE2D)
        draws[i]->setDataPalette(defaultPalette);
      
      if (syncscaling > 0)
      {
        draws[i]->setScalingLimitsSynced(syncscaling);
//        draws[i]->setScalingLimitsHorz(syncscaling);
//        draws[i]->setScalingLimitsVert(syncscaling);
      }
      
//      draws[i]->setClearColor(0x00FFFFFF);
//      draws[i]->setClearByPalette();
      if (MW_TEST == LET_IT_SNOW)
      {
        DrawOverlay* oimg = new OImageStretched(new QImage(img_path_mikey), OVLQImage::IC_BLOCKALPHA, false);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.1);
        draws[i]->ovlPushBack(oimg);
        ovl_snowflake = draws[i]->ovlPushBack(new OSnowflake(new QImage(img_path_sprite), OSnowflake::IC_AUTO, SAMPLES/4/*500*/, 0.2f));
        draws[i]->ovlPushBack(new OTextTraced("Press Me", CR_RELATIVE, 0.5f, 0.1f, 12, OO_INHERITED, true));
      }
      else if (MW_TEST == DRAW_BRIGHT && i == 0)
      {
        DrawOverlay* oimg = new OImageStretched(new QImage(img_path_normal), OVLQImage::IC_BLOCKALPHA, false);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.15);
        draws[i]->ovlPushBack(oimg);
      }
      else if (MW_TEST == DEBUG_VOCAB)
      {
        DrawOverlay* oimg = new OImageStretched(new QImage(img_path_normal), OVLQImage::IC_ASIS, false);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.15);
        draws[i]->ovlPushBack(oimg);
        
//        OSprites* sprites = new OSprites(new QImage(img_path_sprite), OSprites::IC_AUTO, 1.0f, 20, OSprites::CR_OPACITY, OSprites::CB_CENTER);
        OSprites* sprites = new OSprites(new QImage(img_path_sprite), OSprites::IC_AUTO, 1.0f, 20, ppalettes_adv[3], false);
        sprites->setActiveCount(5);
        for (unsigned int i=0; i<sprites->count(); i++)
          sprites->setKPDC(i, 0.1 + i*0.2f, 0.5f, i == 1? 0.5f : 0.2f, /*i == 1? 0.0f : 0.2f*/0.1*i);
        draws[i]->ovlPushBack(sprites);
      }
    }
    if (MW_TEST == DEMO_3_overlays)
      for (unsigned int i=0; i<drawscount; i++)
      {
        QImage  img(img_path_normal);
        DrawOverlay* ovl = new OImageStretched(&img, OVLQImage::IC_AUTO, false);
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
    const unsigned int edMinWidth = 48, edMaxWidth = 48;
    const unsigned int sbUniWidth = 48;
    
    QBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    
    {
      int tabshow = 0;
      QSignalMapper*  featsMapper = new QSignalMapper(this);
      
      BS_INIT_FOR(mainLayout)
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
              int bfsmask = BFS_CHECKABLE | (MW_TEST == DRAW_BRIGHT_CLUSTER || MW_TEST == DEBUG_PALETTE2D? BFS_DISABLED : 0);
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
              BS_SPACING(30)
              BSAUTO_TEXT_ADD(tr("Modify data:"), 0, Qt::AlignHCenter);
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
              BS_SPACING(30)
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
                
                lab = new QLabel(tr("Bounds:"));
                BSADD(lab);
                BS_START_LAYOUT(QHBoxLayout)
                  for (int i=0; i<2; i++)
                    BSAUTO_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                BS_STOP
                BS_STRETCH    
                
                lab = new QLabel(tr("Contrast:"));
                BSADD(lab);
                BS_START_LAYOUT(QHBoxLayout)
                  for (int i=2; i<4; i++)
                    BSAUTO_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                BS_STOP
                BS_STRETCH
                    
                BSAUTO_BTN_ADDMAPPED(BSFieldSetup("reset", nullptr, ED_RESET, 0, 44,44), edMapper);
              BS_STOP
              QObject::connect(edMapper, SIGNAL(mapped(int)), this, SLOT(changeFloats(int)));
            } 
            BS_STRETCH
          BS_STOP
          
          ptb->addTab(tr("Color"));
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
              BSAUTO_TEXT_ADD(tr("bspalettes_std.h:"), 0, Qt::AlignLeft);
              QSignalMapper*  palMapper = new QSignalMapper(this);
              const unsigned int btnsinrow = 4;
              unsigned int palscount = sizeof(ppalettes_loc_std)/sizeof(const IPalette*);
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
                                       BSFieldSetup("3 clr: BGR", &fntSTD, 3, on3 == false? BFS_DISABLED : 0, btnMinWidth), 
                                     };
                for (unsigned int j=0; j<sizeof(bfs)/sizeof(BSFieldSetup); j++)
                  BSAUTO_BTN_ADDMAPPED(bfs[j], palMapperRGB);
                BS_STRETCH
                QObject::connect(palMapperRGB, SIGNAL(mapped(int)), this, SLOT(changePaletteRGB(int)));
              }
              BS_STOP
            BS_STOP
            BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
              BS_START_LAYOUT_HMIN_VMIN(QVBoxLayout)
                BS_STRETCH
                BSAUTO_BTN(QCheckBox, dsc, BSFieldSetup("Palette Discrete", &fntSTD, 0, PORTIONS < 2? BFS_DISABLED : 0, btnMinWidth));
                BSADD(dsc)
                QObject::connect(dsc, SIGNAL(toggled(bool)), this, SLOT(changePaletteDiscretion(bool)));
                BSAUTO_BTN(QCheckBox, itp, BSFieldSetup("Data interpolation", &fntSTD, 0, 0, btnMinWidth));
                BSADD(itp)
                QObject::connect(itp, SIGNAL(toggled(bool)), this, SLOT(changeDataTextureInterpolation(bool)));
                BS_STRETCH
              BS_STOP
              BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BSAUTO_LBL_ADD(BSFieldSetup(tr("Range Start:")));
                  QSlider* slider = new QSlider(Qt::Horizontal);
                  slider->setRange(0,100);
                  slider->setValue(0);
                  BSADD(slider);
                  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changePalrangeStart(int)));
                BS_STOP
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BSAUTO_LBL_ADD(BSFieldSetup(tr("Range Stop: ")));
                  QSlider* slider = new QSlider(Qt::Horizontal);
                  slider->setRange(0,100);
                  slider->setValue(100);
                  BSADD(slider);
                  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changePalrangeStop(int)));
                BS_STOP
              BS_STOP
            BS_STOP
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
                                    _sb->setMaximumWidth(80); \
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
              _sb->setMaximumWidth(80);
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
                  BSFieldSetup(tr("Drop Lines"), &fntSTD, COS_DROPLINES, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Brush"), &fntSTD, COS_BRUSH, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Cluster"), &fntSTD, COS_CLUSTER, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Followers"), &fntSTD, COS_FOLLOWERS, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Inside"), &fntSTD, COS_INSIDE, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("CoverL"), &fntSTD, COS_COVERL, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("CoverH"), &fntSTD, COS_COVERH, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Contour"), &fntSTD, COS_CONTOUR, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Selector"), &fntSTD, COS_SELECTOR, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Selector o"), &fntSTD, COS_SELECTOR2, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Objectif"), &fntSTD, COS_OBJECTIF, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Sprite\nalpha opaque"), &fntSTD, COS_SPRITEALPHA, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Foreground\nstretchable"), &fntSTD, COS_FOREGROUND, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                  BSFieldSetup(tr("Background\nstatic"), &fntSTD, COS_BACKGROUND, BFS_CHECKABLE, btnMinWidth, btnMaxWidth),
                };
                
                BSLAYOUT->setSpacing(2);
                BS_FORFOR_P(3, l, lim, sizeof(fseds)/sizeof(BSFieldSetup)-1)
                {
                  BS_START_LAYOUT_HMAX_VMAX(QHBoxLayout)
                    BSLAYOUT->setSpacing(2);
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
                      BSAUTO_TEXT_ADD(tr("Thick:"));
                      QSpinBox*   psb = new QSpinBox();
                      psb->setRange(0, 10);
                      psb->setMaximumWidth(80);
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
              
              BSAUTO_TEXT_ADD(tr("(settings are not saved between overlays there)"), 0, Qt::AlignHCenter);

            BS_STRETCH
            QObject::connect(ovlMapper, SIGNAL(mapped(int)), this, SLOT(changeOVLFeatures(int)));
          BS_STOP

          
          ptb->addTab(tr("Addit."));
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
              DPostmask* dpm = new DPostmask(DPostmask::empty());
              BSAUTO_TEXT_ADD(tr("Postmask: "), Qt::AlignLeft);
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                BSAUTO_TEXT_ADD(QString::fromUtf8("Over: "))
                QStringList dpmOver; dpmOver<<QString::fromUtf8("Off")<<QString::fromUtf8("Signal")<<QString::fromUtf8("Empty")<<QString::fromUtf8("All");
                QComboBox* qcb2 = new QComboBox;
                qcb2->addItems(dpmOver);
                qcb2->setCurrentIndex(0);
                qcb2->setUserData(1, new BSUOD_DPM(1, dpm));
                QObject::connect(qcb2, SIGNAL(currentIndexChanged(int)), this, SLOT(changePostmask(int)));
                BSADD(qcb2);
                BS_STRETCH
                
                BSAUTO_TEXT_ADD(QString::fromUtf8("Type: "))
                QStringList dpmMain; dpmMain<<QString::fromUtf8("Contour")<<QString::fromUtf8("Line left")
                                           <<QString::fromUtf8("Line right")<<QString::fromUtf8("Line bottom")
                                              <<QString::fromUtf8("Line top")<<QString::fromUtf8("Lines left-bot")
                                                <<QString::fromUtf8("Lines right-bot")
                                                  <<QString::fromUtf8("Lines left-top")<<QString::fromUtf8("Lines right-top")
                                                    <<QString::fromUtf8("Pseudocircle")<<QString::fromUtf8("Dot")
                                                      <<QString::fromUtf8("Dot left-bot")<<QString::fromUtf8("Dot contour")
                                                      <<QString::fromUtf8("/")<<QString::fromUtf8("\\")
                                                      <<QString::fromUtf8("Cross")<<QString::fromUtf8("Grid")
                                                        <<QString::fromUtf8("Fill")<<QString::fromUtf8("Squares");
                QComboBox* qcb = new QComboBox;
                qcb->addItems(dpmMain);
                qcb->setUserData(1, new BSUOD_DPM(0, dpm));
                QObject::connect(qcb, SIGNAL(currentIndexChanged(int)), this, SLOT(changePostmask(int)));
                BSADD(qcb);
//              BS_STOP
//              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                BS_STRETCH
                    
                BSAUTO_TEXT_ADD(QString::fromUtf8("Weight: "))
                QSpinBox* qcb3 = new QSpinBox;
                qcb3->setRange(0, 100);
                qcb3->setMaximumWidth(80);
                qcb3->setUserData(1, new BSUOD_DPM(2, dpm));
                QObject::connect(qcb3, SIGNAL(valueChanged(int)), this, SLOT(changePostmask(int)));
                BSADD(qcb3);
              BS_STOP
              
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                
//              BS_STOP
//              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                BSAUTO_TEXT_ADD(QString::fromUtf8("Palette color idx (0..20 -> 0.0..1.0): "));
                QSpinBox* qcb4 = new QSpinBox;
                qcb4->setRange(-20, 20);
                qcb4->setMaximumWidth(80);
                qcb4->setUserData(1, new BSUOD_DPM(3, dpm));
                QObject::connect(qcb4, SIGNAL(valueChanged(int)), this, SLOT(changePostmask(int)));
                BSADD(qcb4);
                  
                BS_STRETCH
                
                BSAUTO_TEXT_ADD(QString::fromUtf8("Thrs.: "))
                QSpinBox* qcb5 = new QSpinBox;
                qcb5->setRange(0, 10);
                qcb5->setMaximumWidth(80);
                qcb5->setUserData(1, new BSUOD_DPM(4, dpm));
                QObject::connect(qcb5, SIGNAL(valueChanged(int)), this, SLOT(changePostmask(int)));
                BSADD(qcb5);
//                BSAUTO_TEXT_ADD(QString::fromUtf8("Manual: "));
//                for (unsigned int i=0; i<3; i++)
//                {
//                  QSpinBox* qcb5 = new QSpinBox;
//                  qcb5->setRange(0, 255);
//                  qcb5->setUserData(1, new BSUOD_DPM(4+i, dpm));
//                  QObject::connect(qcb5, SIGNAL(valueChanged(int)), this, SLOT(changePostmask(int)));
//                  BSADD(qcb5);
//                }
              BS_STOP
//              QObject::connect(qbg, SIGNAL(buttonClicked(int)), this, SLOT(changeInterpolation(int)));
            BS_STOP
                  
                  
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
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
              BSAUTO_BTN_ADDMAPPED(BSFieldSetup(tr("Special Debug Button"), &fntSTD, BTF_DEBUG, 0, btnMinWidth), featsMapper, 0, Qt::AlignCenter);
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
      QObject::connect(featsMapper, SIGNAL(mapped(int)), this, SLOT(changeFeatures(int)));
      BS_STOP

      BS_START_LAYOUT_HMAX_VMAX(QVBoxLayout)
      BS_ALIGN_HCENTER
      BS_START_FRAME_H_HMIN_VMIN(BS_FRAME_PANEL, 1)
      {
#define REGTEST(A) #A,
  const char* testnames[] = {
  #include "tests.h"
  };
#undef REGTEST
        BSAUTO_TEXT_ADD(QString("%1. %2").arg(MW_TEST).arg(testnames[MW_TEST]));
      }
      BS_STOP
      BS_ALIGN_NONE
      {
        if (MW_TEST == ADV_PALETTES)
        {
          BS_START_SCROLL_V_HMAX_VMAX
            for (unsigned int i=0; i<drawscount; i++)
            {
              BSAUTO_TEXT_ADD(QString().sprintf("ppalettes_adv[%d]:\t\t%s", i, ppalettenames_adv[i]));
              BSADD(draws[i]);
            }
          BS_STRETCH
          BS_STOP;
        }
  //      else if (MW_TEST == PROGRESS_BAR)
  //      {
  //        BS_START_FRAME_V_HMIN_VMIN(BS_FRAME_PANEL, 2)
  //          for (unsigned int i=0; i<drawscount; i++)
  //          {
  //            if (i == drawscount - 1)
  //              draws[i]->setFixedHeight(40);
  //            BSADD(draws[i])
  //          }
  //        BS_STOP;
  //      }
        else if (MW_TEST == FEATURE_PORTIONS)
        {
          QSpinBox* psb = new QSpinBox;
          psb->setRange(0, PORTIONS);
          psb->setValue(PORTIONS);
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            BS_START_LAYOUT_HMAX_VMAX(QHBoxLayout)
              BS_START_LAYOUT_HMAX_VMAX(QVBoxLayout)
                for (int i=0; i<3; i++)
                  BSADD(draws[i], i==0? 1 : 0);
              BS_STOP
              BSADD(draws[3]);
              BSADD(draws[4]);
            BS_STOP
            BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_SUNKEN, 2)
              BSAUTO_TEXT_ADD("Change portions count:");
              BSADD(psb);
              for (int i=0; i<drawscount; i++)
                QObject::connect(psb, SIGNAL(valueChanged(int)), draws[i], SLOT(slot_setPortionsCount(int)));
              BSAUTO_TEXT_ADD(" ; Note, your upper limit is fixed (cause of already allocated data)");
              BS_STRETCH
            BS_STOP
//            BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
//              for (int i=0; i<3; i++)
//                BSADD(draws[3 + i], i==0? 1 : 0)
//              BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_SUNKEN, 2)
//                BSAUTO_TEXT_ADD("Change portions count:");
//                QSpinBox* psb = new QSpinBox;
//                psb->setRange(0, PORTIONS);
//                psb->setValue(PORTIONS);
//                BSADD(psb);
//                for (int i=0; i<drawscount; i++)
//                  QObject::connect(psb, SIGNAL(valueChanged(int)), draws[3 + i], SLOT(slot_setPortionsCount(int)));
//                BSAUTO_TEXT_ADD(" ; Note, your upper limit is fixed,\n(cause of already allocated data)");
//                BS_STRETCH
//              BS_STOP
//            BS_STOP
          BS_STOP
        }
        else if (MW_TEST == FEATURE_ORIENTS)
        {
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            BSADD(draws[0])
            BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
              for (unsigned int i=1; i<drawscount-1; i++)
                BSADD(draws[i])
            BS_STOP
            BSADD(draws[drawscount-1])
          BS_STOP;
        }
        else if (MW_TEST == VERTICAL)
        {
          BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<drawscount; i++)
              BSADD(draws[i])
          BS_STOP;
        }
        else if (MW_TEST == DRAW_SCALES_1)
        {
          BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<drawscount; i++)
            {
//              draws[i]->setOrientation(OR_LRTB);
//              draws[i]->setOrientation(OR_RLBT);
              DrawBars* pDB = new DrawBars(draws[i]);
              
              pDB->setContentsMargins(60,60,60,60);
              
              
              if (i == 0)
              {
                bool contourMaxZone = false;
                
                int dbmode = DBMODE_STRETCHED;
    //            int dbmode = DBMODE_STATIC;
                
                int otherFlags = /*DBF_ENUMERATE_FROMZERO | */DBF_ENUMERATE_SHOWLAST;
    //            int otherFlags = DBF_ONLY2NOTES;
    //            int otherFlags = DBF_NOTESINSIDE;
                
                int dockerFlags = DBF_DOCKTO_PREVMARK; 
                for (int j=0; j<4; j++)
                {
                  ATTACHED_TO at = ATTACHED_TO(j);
                  pDB->addContour(at, 0, contourMaxZone);
                  pDB->addScaleEnumerator(at, dbmode | otherFlags | dockerFlags, 64 + 1, 30);
                  pDB->addContour(at, 0, contourMaxZone);
                  pDB->addScaleFixed(at, dbmode | otherFlags | dockerFlags | DBF_NOTESINSIDE, 0.0f, 1.0f, SAMPLES);
    //              pDB->addContour(at, 0, contourMaxZone);
                  
    //              pDB->addScaleDrawUniSide(at, dbmode | otherFlags | dockerFlags, 30);
                  
                  if (j == AT_LEFT)
                    pDB->addScaleDrawGraphB(AT_LEFT, 0, 3, 32);
                }
              }
              else if (i == 1)
              {
                ATTACHED_TO at1 = AT_LEFT, at2 = AT_RIGHT;
                pDB->addSpace(at1, 60);
                pDB->addPointerDrawGraphB(at1, DBF_SHARED, 0.0f, 10);
                pDB->addPointerDrawGraphB(at1, DBF_SHARED, 0.5f, 10);
                pDB->addPointerDrawGraphB(at1, DBF_SHARED, 1.0f, 10);
                pDB->addSpace(at2, 60);
                pDB->addPointerFixed(at2, DBF_SHARED, 0.0f, 0.0f, 1.0f, 20);
                pDB->addPointerFixed(at2, DBF_SHARED, 0.5f, 0.0f, 1.0f, 20);
                pDB->addPointerFixed(at2, DBF_SHARED, 1.0f, 0.0f, 1.0f, 20);
                
                pDB->addSpace(AT_TOP, 20);
                pDB->addSpace(AT_TOP, 60);
                pDB->addLabel(AT_TOP, DBF_SHARED | DBF_LABELAREA_FULLBAR, "Dynamic", Qt::AlignLeft);
                pDB->addLabel(AT_TOP, DBF_SHARED | DBF_LABELAREA_FULLBAR, "Static", Qt::AlignRight);
              }
              
              
  //            if (i == 0)
  //              pDB->addScaleDrawUniSide(AT_TOP, DBF_ENUMERATE_FROMZERO | DBF_ENUMERATE_SHOWLAST, 20);
  //            else if (i == 1)
  //              pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED, 0.0, SAMPLES-1, SAMPLES);
  //            else if (i == 2)
  //              pDB->addScaleTapNM(AT_TOP, DBMODE_STATIC, standard_tap_symbolate<-1>, 4, nullptr, SAMPLES, 20);
  //            else
  //              pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED_POW2 | DBF_ONLY2NOTES | DBF_NOTESINSIDE, 0.0, 1.0, SAMPLES, 10);
              
  //            MEWPointer* mpH = pDB->addPointerDrawUniSide(AT_BOTTOM, DBF_NOTESINSIDE);
  //            MEWPointer* mpV = pDB->addPointerDrawGraphB(AT_RIGHT, DBF_NOTESINSIDE);
  //            int oapH = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpH->createProactive()));
  //            pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_VERT_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapH);
  //            int oapV = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpV->createProactive()));
  //            pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_HORZ_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapV);
  //            {
  //              pDB->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, labels_clr[i][0], Qt::AlignCenter, Qt::Vertical);
  //              pDB->addLabel(AT_TOP, 0, labels_clr[i][1], Qt::AlignCenter, Qt::Horizontal);
  //            }
              
  //            draws[i]->setFixedHeight(10);
              
              BSADD(pDB)
              QObject::connect(this, SIGNAL(remitBounds(float)), pDB, SLOT(slot_setBoundHigh(float)));
            }
            QScrollBar* qsb = new QScrollBar(Qt::Vertical);
            qsb->setRange(1, 200);
            qsb->setValue(100);
            QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(changePointer(int)));
            BSADD(qsb);
          BS_STOP
        }
        else if (MW_TEST == DRAW_SCALES_2)
        {
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            DrawBars::COLORS cps[] = {DrawBars::CP_DEFAULT, DrawBars::CP_FROM_DRAWBACK, 
                                      DrawBars::CP_FROM_DRAWBACK, DrawBars::CP_FROM_DRAWPALETTE_INV };
            const char* labels_clr[][2] = {  {"colors: default", "Universe scale"},
                                             {"colors: drawback", "Fixed scale, floating marks"},
                                             {"colors: drawback", "Tap by alphabet"},
                                             {"colors: inversed", "Fixed minimal scale"},
                                          };
            for (unsigned int i=0; i<drawscount; i++)
            {
              DrawBars* pDB = new DrawBars(draws[i], cps[i]);
              if (cps[i] == DrawBars::CP_FROM_DRAWBACK)
              {
                pDB->addContour(AT_LEFT, 0);
                pDB->addContour(AT_TOP, 0);
                pDB->addContour(AT_BOTTOM, 0);
                pDB->addContour(AT_RIGHT, 0);
              }
                
              
              if (i == 0)
                pDB->addScaleDrawUniSide(AT_TOP, DBF_ENUMERATE_FROMZERO | DBF_ENUMERATE_SHOWLAST, 20);
              else if (i == 1)
                pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED, 0.0, SAMPLES-1, SAMPLES);
              else if (i == 2)
                pDB->addScaleTapNM(AT_TOP, DBMODE_STATIC, standard_tap_symbolate<-1>, 4, nullptr, SAMPLES, 20);
              else
                pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED_POW2 | DBF_ONLY2NOTES | DBF_NOTESINSIDE, 0.0, 1.0, SAMPLES, 10);
              
//              pDB->getDraw()->setPostMask(DPostmask::postmask(PO_EMPTY, PM_LINERIGHT, 0, 0.3f, 0.3f, 0.3f));
//              MEPointer mpH = pDB->addPointerFixed(AT_BOTTOM, 0, 180);
              MEWPointer* mpH = pDB->addPointerDrawUniSide(AT_BOTTOM, DBF_NOTESINSIDE, 0.0f);
//              MEPointer mpV = pDB->addPointerDrawUniSide(AT_RIGHT);
              MEWPointer* mpV = pDB->addPointerDrawGraphB(AT_RIGHT, DBF_NOTESINSIDE, 0.0f);
//              mpH.setPosition(0.7f);
              int oapH = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpH->createProactive()));
              pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_VERT_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapH);
              
//              mpV.setPosition(0.7f);
              int oapV = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpV->createProactive()));
              pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_HORZ_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapV);
//              pDB->getDraw()->ovlPushBack(new OFDouble(true, CR_RELATIVE, 0, CR_ABSOLUTE, 6, linestyle_stroks(1.0f,1.0f,0.0f)), oap);
//              pDB->getDraw()->ovlPushBack(new OFDouble(false, CR_RELATIVE, 0, CR_ABSOLUTE, 6, linestyle_stroks(1.0f,1.0f,0.0f)), oap);
              

              if (i == drawscount - 1)
              {
//                pDB->addContour(AT_LEFT, 40);
                pDB->addScaleDrawGraphB(AT_LEFT, 0 /*| DBF_NOTESINSIDE/* | DBF_INTERVENT*/, 3, 32);
//                pDB->addSpace(AT_LEFT, 10);
              }
              else
                pDB->addScaleDrawGraphB(AT_LEFT, 0 /*| DBF_NOTESINSIDE*/, 21, 32);
              
//              if (i < 3)
              {
                pDB->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, labels_clr[i][0], Qt::AlignCenter, Qt::Vertical);
                pDB->addLabel(AT_TOP, 0, labels_clr[i][1], Qt::AlignCenter, Qt::Horizontal);
              }
              
              BSADD(pDB)
            }
          BS_STOP
//          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
//            BSAUTO_TEXT_ADD("\tResize Me");
//          BS_STOP
        }
        else if (MW_TEST == DRAW_SCALES_3)
        {
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            int rounding[] = { 0, DBF_MARKS_DONTROUND, DBF_MARKS_DONTROUND1 };
            for (unsigned int i=0; i<drawscount; i++)
            {
//              draws[i]->setOrientation(OR_BTLR);
              DrawBars* pDB = new DrawBars(draws[i]);
//              {
//                pDB->addContour(AT_LEFT, 0);
//                pDB->addContour(AT_TOP, 0);
//                pDB->addContour(AT_BOTTOM, 0);
//                pDB->addContour(AT_RIGHT, 0);
//              }
              pDB->setContentsMargins(10, 10, 10, 10);
              pDB->addScaleFixed(AT_TOP, DBMODE_STRETCHED, 0.0, SAMPLES-1, SAMPLES);
//              MEWPointer* mpV = pDB->addPointerDrawGraphB(AT_LEFT, DBF_NOTESINSIDE | rounding[i], 0.0f);
              pDB->addScaleDrawGraphB(AT_LEFT, rounding[i], 11, 18);
              BSADD(pDB)
            }
          BS_STOP
//          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
//            BSAUTO_TEXT_ADD("\tResize Me");
//          BS_STOP
        }
        else if (MW_TEST == DEBUG_TABS)
        {
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
            QBoxLayout* stackedLayoutIsFullOfShit;
            QTabBar*  ptb_ovl = new QTabBar;
            BSADD(ptb_ovl);
            BS_START_STACK_HMIN_VMIN
                ptb_ovl->addTab(tr("Tab 1"));
                BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
                  stackedLayoutIsFullOfShit = BSLAYOUT;
                  BS_STRETCH
//                  BSADD(draws[0])
                BS_STOP
                ptb_ovl->addTab(tr("Tab 2"));
                BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
                  BS_STRETCH
                  BSADD(draws[1])
                BS_STOP
                ptb_ovl->addTab(tr("Tab 3"));
                BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
                  BSADD(draws[2])
                BS_STOP
                QObject::connect(ptb_ovl, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
            BS_STOP
            stackedLayoutIsFullOfShit->addWidget(draws[0]);
          BS_STOP
          BS_STRETCH
        }
        else if (MW_TEST == DEBUG_MEVSCALES)
        {
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<drawscount; i++)
            {
              DrawBars* pDB = new DrawBars(draws[i]);
//              pDB->setContentsMargins(10, 10, 10, 10);
              pDB->addMarginElement(AT_LEFT, new MarginDebug("LEFT_"), nullptr, false, false);
              pDB->addMarginElement(AT_TOP, new MarginDebug("TOP__"), nullptr, false, false);
              pDB->addMarginElement(AT_RIGHT, new MarginDebug("RIGHT"), nullptr, false, false);
              pDB->addMarginElement(AT_BOTTOM, new MarginDebug("BOT__"), nullptr, false, false);
              BSADD(pDB)
            }
          BS_STOP
//          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
//            BSAUTO_TEXT_ADD("\tResize Me");
//          BS_STOP
        }
        else if (MW_TEST == DEBUG_MAINSCALE)
        {
          MEWScaleNN* scs[32];
          int scscount=0;
          BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<drawscount; i++)
            {
//              draws[i]->setOrientation(OR_RLBT);
              DrawBars* pDB = new DrawBars(draws[i]);
//              pDB->setContentsMargins(10, 10, 10, 10);
//              scs[scscount++] = pDB->addScaleregFixed(AT_TOP, 0, 200.0f, 1200.0f, 200.0f, 0, 11, 50, 4);
              int count = 33;
              int pixstep = 50;
              scs[scscount++] = pDB->addScaleregFixed(AT_LEFT, 0, 0.1f, 1.0f, 0.025f, 0, count, pixstep, 4);
              scs[scscount++] = pDB->addScaleregFixed(AT_TOP, 0, 0.1f, 1.0f, 0.025f, 0, count, pixstep, 4);
              scs[scscount++] = pDB->addScaleregFixed(AT_BOTTOM, 0, 0.1f, 1.0f, 0.25f, 0, count, pixstep, 4);
              scs[scscount++] = pDB->addScaleregFixed(AT_RIGHT, 0, 0.1f, 1.0f, 0.25f, 0, count, pixstep, 4);
//              scs[scscount++] = pDB->addScaleFixed(AT_BOTTOM, DBMODE_STRETCHED, 0.0f, 1.0f, 11, 50, 4);
              
//              pDB->addScaleregFixed(AT_TOP, 0, -1200.0f, 1200.0f, 250.0f, 0, 11, 50, 4);
//              pDB->addScaleregFixed(AT_BOTTOM, 0, 200.0f, 1200.0f, 200.0f, 0, 11, 50, 4);
//              pDB->addScaleregFixed(AT_BOTTOM, 0, -1200.0f, 1200.0f, 250.0f, 0, 11, 50, 4);
              
//              pDB->addScaleregFixed(AT_BOTTOM, 0, 33.0f, 10000.0f, 500.0f, 0, 3, 50, 4);
//              pDB->addScaleregFixed(AT_BOTTOM, 0, 33.0f, 10000.0f, 50.0f, 0, 100, 50, 4);
              
              BSADD(pDB)
            }
          BS_STOP
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            QDoubleSpinBox* qdsblow = new QDoubleSpinBox();
            qdsblow->setPrefix("Low  ");
            qdsblow->setRange(-10000, 10000);
            qdsblow->setValue(0);
            BSADD(qdsblow)
            for (int s=0; s<scscount; s++)
              QObject::connect(qdsblow, SIGNAL(valueChanged(double)), scs[s], SLOT(setBoundLow(double)));
            QDoubleSpinBox* qdsbhigh = new QDoubleSpinBox();
            qdsbhigh->setPrefix("High  ");
            qdsbhigh->setRange(-10000, 10000);
            qdsbhigh->setValue(17);
            BSADD(qdsbhigh)
            for (int s=0; s<scscount; s++)
              QObject::connect(qdsbhigh, SIGNAL(valueChanged(double)), scs[s], SLOT(setBoundHigh(double)));
            BS_STRETCH
          BS_STOP
//          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
//            BSAUTO_TEXT_ADD("\tResize Me");
//          BS_STOP
        }
        else
        {
          QScrollBar* qsb = MW_TEST == DRAW_GRAPHS_MOVE? new QScrollBar(Qt::Horizontal) : nullptr;
          BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
  //        BS_START_FRAME_H_HMIN_VMIN(BS_FRAME_PANEL, 2)
            for (unsigned int i=0; i<dccount; i++)
            {
              if (MW_TEST == DRAW_RECORDER)
                qsb = new QScrollBar();
              
              BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
                  
                if (MW_TEST == DEMO_4_portions)
                  BSAUTO_TEXT_ADD(i == 0? "One palette, three portions" : "One palette, one portion", 0, Qt::AlignHCenter)
                else if (MW_TEST == DRAW_HISTOGRAMS)
                  BSAUTO_TEXT_ADD(QString::number(PORTIONS) + 
                                  QString(" portions with different colors. See tab Addit. section Postmask for more settings"), 
                                  0, Qt::AlignHCenter)
                
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
                  
  #ifdef USESCALES
  #define BS_SCALED_ALIAS(draw, drdre)  DrawBars* drdre = new DrawBars(draw, DrawBars::CP_FROM_DRAWBACK); \
                                        drdre->addScaleDrawUniSide(AT_TOP, 0, 21); \
                                        if (draw->directions() == 2)  drdre->addScaleDrawUniSide(AT_LEFT, 0, 21); \
                                        else                          drdre->addScaleDrawGraphB(AT_LEFT, 0 | DBF_NOTESINSIDE, 21, 20);
  #else
  #define BS_SCALED_ALIAS(draw, drdre)  DrawQWidget* drdre = draw;
  #endif
                  
                  BS_SCALED_ALIAS(draws[i*drcount + j], pDraw)
                  BSADD(pDraw, lwresult);
  //                BSADD(draws[i*drcount + j], lwresult);
                }
                if (MW_TEST == DRAW_BRIGHT)
                  BS_STRETCH
                      
              BS_STOP
                      
              if (qsb && i == dccount - 1 && MW_TEST == DRAW_RECORDER)
                BSADD(qsb);
              
            }
          BS_STOP
          if (MW_TEST == DRAW_BRIGHT)
            BS_STRETCH
          if (MW_TEST == DRAW_GRAPHS_MOVE)
          {
            BSADD(qsb);
            BSAUTO_TEXT_ADD("\tGraph width: 300pts. New data: 5pts. History: 600pts.");
          }
        }
      }
      BS_STOP
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
    DSAMPLES = int(SAMPLES);
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
  else if (sp == 1)
    emit generateData();
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

static float my_hiperb(float x, float mov){ return 1.0f/x - 0.25f + mov*0.5f; }
static float my_sinxx(float x, float mov){ return qFastSin(x*(1+mov))/(x); }
static float my_xx(float x, float mov){ return x*x/(100 + 1000*mov) - 0.5f; }
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
  
    const int count = DSAMPLES*int(MAXLINES)/3;
    
    static unsigned int* countByPortions = nullptr;
    static float* Xs, *Ys, *values;
    if (countByPortions == nullptr)
    {
      countByPortions = new unsigned int[PORTIONS];
      Xs = new float[count*PORTIONS];
      Ys = new float [count*PORTIONS];
      values = new float[count*PORTIONS];
    }
    
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
        generateGaussian(0.5f, 0.2f, &Xs[count*p + i], &Ys[count*p + i]);
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
  else if (MW_TEST == DEBUG_PALETTE2D)
  {
//    for (int p=0; p<PORTIONS; p++)
//      for (int j=0; j<MAXLINES; j++)
//        for (int i=0; i<DSAMPLES; i++)
//        {
//          testbuf2D[p*DSAMPLES*MAXLINES + j*DSAMPLES + i] = i/float(DSAMPLES-1) + j/float(MAXLINES-1);
//        }
    
    for (int j=0; j<MAXLINES; j++)
      for (int i=0; i<DSAMPLES; i++)
      {
        testbuf2D[0*DSAMPLES*MAXLINES + j*DSAMPLES + i] = i/float(DSAMPLES-1);
      }
    for (int j=0; j<MAXLINES; j++)
      for (int i=0; i<DSAMPLES; i++)
      {
        testbuf2D[1*DSAMPLES*MAXLINES + j*DSAMPLES + i] = j/float(MAXLINES-1);
      }
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i]->setData(testbuf2D);
    }
  }
  else
  {
    float (*manual_fn)(float, float) = nullptr;
    
    #pragma omp parallel
    for (int pm=0; pm<PORTIONS*MAXLINES; pm++)
    {
      int portion = pm / MAXLINES;
//      float fmov01samples = ((g_movX + portion*portion*10)%DSAMPLES)/float(DSAMPLES);
//      float fmov01sin = qFastSin((g_movX + portion*portion*10)/(M_PI*2*2))/2.0f + 0.5f;
      float fmov01samples = ((g_movX + portion*10)%DSAMPLES)/float(DSAMPLES);
      float fmov01sin = qFastSin((g_movX + portion*10)/(M_PI*2*2))/2.0f + 0.5f;
      
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
          testbuf[i] = (qFastSin((float(i)/DSAMPLES - fmov01samples)*2*M_PI) + 1)/2.0f;
        break;
      }
      case ST_MANYSIN:
      {
        #pragma omp for
        for (int i=0; i<DSAMPLES; i++)
          testbuf[i] = (qFastSin((float(i*10)/DSAMPLES - fmov01samples)*2*M_PI) + 1)/2.0f;
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
        static const int mover_size = sizeof(mover)/sizeof(float);
        int base = qRound(fmov01samples*DSAMPLES);
        #pragma omp for
        for (int i = 0; i < DSAMPLES; ++i)
          testbuf[i] = randbuf[i]*0.3f;
        
        for (int i = 0; i < mover_size; ++i)
        {
          int iip = base + i; if (iip >= DSAMPLES) iip = iip - DSAMPLES;
          int iim = base - i; if (iim < 0) iim = DSAMPLES + iim;
          testbuf[iip] = mover[i];
          testbuf[iim] = mover[i];
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
          float value = manual_fn((i - int(DSAMPLES)/2)/10.0f, fmov01sin) + 0.5f;
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
          testbuf[DSAMPLES - i - 1] = ((g_movX2 + portion*3 + i)%sp) / float(sp - 1);
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
  
  if (ovl_snowflake != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OSnowflake* pSprites = (OSnowflake*)draws[i]->ovlGet(ovl_snowflake);
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
  if ((unsigned int)id >= sizeof(ppalettes_loc_std)/sizeof(const IPalette*))
    return;
  
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPalette(ppalettes_loc_std[id]);
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

void MainWindow::changePaletteDiscretion(bool v)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPaletteDiscretion(v);
}

void MainWindow::changeDataTextureInterpolation(bool v)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataTextureInterpolation(v);
}

void  MainWindow::changeFloats(int edid)
{
  float value;
  QSignalMapper* psm = (QSignalMapper*)sender();
  if (edid != ED_RESET)
  {
    QLineEdit* pEd = (QLineEdit*)psm->mapping(edid);
    QString text = pEd->text();
    bool ok;
    value = text.toFloat(&ok);
    if (!ok)
    {
      qDebug()<<"Example: Conversion to float failed!";
      return;
    }
  }
  if (edid == ED_SIGB || edid == ED_SIGK)
  {
    if (edid == ED_SIGB)       sig_b = value;
    else if (edid == ED_SIGK)  sig_k = value;
    return;
  }
  else if (drawscount != 0)
  {
    QLineEdit*  qle[] = {  
      (QLineEdit*)psm->mapping(ED_HIGH), 
      (QLineEdit*)psm->mapping(ED_LOW), 
      (QLineEdit*)psm->mapping(ED_CONTRAST), 
      (QLineEdit*)psm->mapping(ED_OFFSET)
    };
    if (edid == ED_LOW || edid == ED_HIGH)
    {
      if (edid == ED_HIGH)        for (unsigned int i=0; i<drawscount; i++) draws[i]->setBoundHigh(value);
      else if (edid == ED_LOW)    for (unsigned int i=0; i<drawscount; i++) draws[i]->setBoundLow(value);
      qle[ED_CONTRAST]->setText(QString::number(draws[0]->contrastK(), 'g', 3));
      qle[ED_OFFSET]->setText(QString::number(draws[0]->contrastB(), 'g', 3));
    }
    else if (edid == ED_CONTRAST || edid == ED_OFFSET)
    {
      if (edid == ED_CONTRAST)    for (unsigned int i=0; i<drawscount; i++) draws[i]->setContrastK(value);
      else if (edid == ED_OFFSET) for (unsigned int i=0; i<drawscount; i++) draws[i]->setContrastB(value);
      qle[ED_HIGH]->setText(QString::number(draws[0]->bounds().HL, 'g', 3));
      qle[ED_LOW]->setText(QString::number(draws[0]->bounds().LL, 'g', 3));
    }
    else if (edid == ED_RESET)
    {
      for (unsigned int i=0; i<drawscount; i++) draws[i]->setBounds(0, 1);
      qle[ED_HIGH]->setText("1.0");
      qle[ED_LOW]->setText("0.0");
      qle[ED_CONTRAST]->setText("1.0");
      qle[ED_OFFSET]->setText("0.0");
    }
//    for (unsigned int i=0; i<drawscount; i++)
//    {
//      if (edid == ED_LOW || edid == ED_HIGH)
//      {
//        if (edid == ED_LOW)         draws[i]->setBoundLow(value);
//        else if (edid == ED_HIGH)   draws[i]->setBoundHigh(value);
//      }
//      else if (edid == ED_CONTRAST || edid == ED_OFFSET)
//      {
//        if (edid == ED_CONTRAST)    draws[i]->setContrastK(value);
//        else if (edid == ED_OFFSET) draws[i]->setContrastB(value);
//      }
//    }
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
  ORIENTATION invHorz[] = {  OR_RLBT, OR_LRBT, OR_RLTB, OR_LRTB, OR_TBRL, OR_BTRL, OR_TBLR, OR_BTLR  };
  ORIENTATION invVert[] = {  OR_LRTB, OR_RLTB, OR_LRBT, OR_RLBT, OR_BTLR, OR_TBLR, OR_BTRL, OR_TBRL  };
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
          DrawOverlay* povl = draws[i]->ovlGet(j + 1);
          if (povl) povl->setOpacity(1.0f);
        }
    }
    else if (id == BTF_DEBUG)
    {
      if (MW_TEST == DRAW_SCALES_1)
      {
        draws[i]->setMinimumWidth(800);
      }
    }
  }
  if (id == BTF_DESTROYGRAPH)
  {
    if (drawscount > 0)
      delete draws[--drawscount];
  }
}

void MainWindow::changePostmask(int sigid)
{
  BSUOD_DPM* dpm = (BSUOD_DPM*)sender()->userData(1);
  if (dpm->id == 0)
    dpm->dpm->mask = (BSPOSTMASKTYPE)sigid;
  else if (dpm->id == 1)
    dpm->dpm->over = (BSPOSTMASKOVER)sigid;
  else if (dpm->id == 2)
    dpm->dpm->weight = sigid;
  else if (dpm->id == 3)
  {
    if (sigid < 0)
    {
      int mygrey = -sigid*12;
      dpm->dpm->colorManual = mygrey + (mygrey << 8) + (mygrey << 16);
    }
    else
    {
      dpm->dpm->colorManual = -1;
      dpm->dpm->colorPalette = sigid / 20.0f;
    }
  }
  else if (dpm->id == 4)
  {
    dpm->dpm->threshold= sigid / 10.0f;
  }
//  else if (dpm->id == 4)
//    dpm->dpm->color = (dpm->dpm->color & ~(0xFF)) + (sigid);
//  else if (dpm->id == 5)
//    dpm->dpm->color = (dpm->dpm->color & ~(0xFF<<8)) + (sigid<<8);
//  else if (dpm->id == 6)
//    dpm->dpm->color = (dpm->dpm->color & ~(0xFF<<16)) + (sigid<<16);
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setPostMask(*dpm->dpm);
}

void MainWindow::changeInterpolation(int sigid)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataTextureInterpolation(sigid == 1);
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

void MainWindow::changePalrangeStart(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPaletteRangeStart(value / 100.0f);
}

void MainWindow::changePalrangeStop(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataPaletteRangeStop(value / 100.0f);
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
      draws[i]->ovlPushBack(new OBorder(linestyle_red(1,0,0)));
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
//      draws[i]->ovlPushBack(new OToons(CR_RELATIVE, 0.95, 1, linestyle_greydark(1,0,0)));
      break;
    }
    case COS_DROPLINES:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new ODropLine(25, true, linestyle_white(5,2,0)));
//        ovl_tmp = draws[i]->ovlPushBack(new OBrush(1000, linestyle_white(5,2,0)));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Press left mouse button", CR_RELATIVE, 0.05, 0.05, 12, OO_INHERITED, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_BRUSH:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new OBrush(1000, linestyle_solid(1.0f,0,0)));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Draw with your new brush", CR_RELATIVE, 0.05, 0.05, 12, OO_INHERITED, true, linestyle_white(5,2,0)));
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
      draws[i]->ovlPushBack(new OBorder(linestyle_yellow(4,1,0)));
      if (!ovl_is_synced || i == 0)
        ovl_visir = draws[i]->ovlPushBack(new OActiveCursor());
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_visir));
      draws[i]->ovlPushBack(new OFCross(CR_RELATIVE, 0, 0, CR_ABSOLUTE, 5, 30, linestyle_yellow(2,1,0)), ovl_visir);
      draws[i]->ovlPushBack(new OTextTraced("LEFT", CR_ABSOLUTE, -80, 50, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextTraced("RIGHT", CR_ABSOLUTE, 50, 50, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextTraced("We all", CR_ABSOLUTE, -25, -60, 12), ovl_visir);
      draws[i]->ovlPushBack(new OTextTraced("Follow mouse", CR_ABSOLUTE, -50, -80, 12, OO_INHERITED, false, linestyle_red(1,0,0)), ovl_visir);
      break;
    }
    case COS_INSIDE:
    {
      float opacity = 0.1f;
      for (int j=7; j>=0; j--)
      {
        DrawOverlay* ovl = new OFSquareCC(opacity, CR_RELATIVE, 0.5f, 0.5f, CR_SAME, 0.04f*(j+1), linestyle_redlight(1,0,0));
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
      draws[i]->ovlPushBack(new OCover(0.7f, 1.0f, 2, OCover::COP_COVER));
//      draws[i]->ovlPushBack(new OSlice(5.0, 0.0f, 0.4f, 0.0));
      break;
    }
    case COS_CONTOUR:
    {
      draws[i]->ovlPushBack(new OContour(0.95f, 1.0f, linestyle_inverse_1(2,1,0)));
      break;
    }
    case COS_SELECTOR: case COS_SELECTOR2:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(id == COS_SELECTOR? 
                                          (DrawOverlay*)new OSelector(linestyle_bluelight(13,2,2), 0.2f) :
                                          (DrawOverlay*)new OSelectorCirc(linestyle_bluelight(13,2,2), 0.2f, true));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Press left mouse button and move", CR_RELATIVE, 0.05f, 0.05f, 11, OO_INHERITED, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_OBJECTIF:
    {
//      draws[i]->ovlPushBack(new OFFactor(CR_RELATIVE, 0.5f, 0.5f, CR_RELATIVE, 0.25f, 0.45f, linestyle_inverse_1(2,1,0)));
//      draws[i]->ovlPushBack(new OFObjectif(CR_RELATIVE, 0.5f, 0.5f, CR_RELATIVE, 0.4f, 0.4f, 0.1f, 0.1f, linestyle_inverse_1(2,1,0)));
      draws[i]->ovlPushBack(new OFObjectif(CR_RELATIVE, 0.5f, 0.5f, CR_RELATIVE, 0.45f, 0.45f, 0.25f, 0.25f, linestyle_solid(0,1,1)));
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
        draws[i]->ovlPushBack(new OImageOriginal(&img, OVLQImage::IC_AUTO, false, CR_PIXEL, -24, 0, 0.2f, 0.2f), ovl_visir);
      }
      break;
    }
    case COS_FOREGROUND:
    {
      QImage  img(img_path_normal);
      draws[i]->ovlPushBack(new OImageStretched(&img, OVLQImage::IC_BLOCKALPHA, false));
      break;
    }
    case COS_BACKGROUND:
    {
      QImage  img(img_path_normal);
      DrawOverlay* ovl = new OImageOriginal(&img, OVLQImage::IC_AUTO, false, CR_RELATIVE, 0.0f, 0.0f);
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
    float abssize = 20;
    linestyle_t kls = linestyle_green(1,0,0);
    
    DrawOverlay* ovl = nullptr;
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
    DrawOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setOpacity(op/100.0f);
  }
}

void MainWindow::changeOVLSlice(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    DrawOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setSlice(op/100.0f);
  }
}

void MainWindow::changeOVLWeight(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    DrawOverlay* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setThickness(op);
  }
}

void MainWindow::changeOVLForm(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    _DrawOverlayLined* povl = dynamic_cast<_DrawOverlayLined*>(draws[i]->ovlGet(active_ovl));
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
      qDebug()<<"Example: Overlay Position dynamic_cast failure!";
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
      _DrawOverlayLined* povl = dynamic_cast<_DrawOverlayLined*>(draws[i]->ovlGet(active_ovl));
      if (povl)
      {
        linestyle_t kls = linestyle_update(povl->getLineStyle(), clr.redF(), clr.greenF(), clr.blueF());
        povl->setLineStyle(kls);
      }
      else
      {
        qDebug()<<"Example: Overlay Features dynamic_cast failure!";
      }
    }
  }
  else if (id == BTO_NOINV || id == BTO_INV1 || id == BTO_INV2 || id == BTO_INV3 || id == BTO_INV4 || id == BTO_INV5)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      _DrawOverlayLined* povl = dynamic_cast<_DrawOverlayLined*>(draws[i]->ovlGet(active_ovl));
      if (povl)
      {
        linestyle_t kls = povl->getLineStyle();
        kls.inversive = id == BTO_NOINV? 0 : (id - BTO_NOINV);
        povl->setLineStyle(kls);
      }
    }
  }
}

//DrawOverlay* MainWindow::createNewOverlay(BTN_VISIR btv)
//{
//  DrawOverlay*  newOverlay = nullptr;
//  switch (vistype)
//  {
//    case BTV_CIRCLE:  newOverlay = new OFCircle(false, ocs, 0.0, 0.0, true, 100, linestyle_green(1,0,0)); break;
//    case BTV_SQUARE:  newOverlay = new OFSquareCC(false, ocs, 0.0, 0.0, true, 100, linestyle_green(1,0,0)); break;
//    case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, ocs, 0.0, 0.0, 10, 100, linestyle_green(1,0,0)); break;
//    case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, ocs, 0.0, 0.0, 10, 100, linestyle_green(1,0,0)); break;
//    case BTV_FACTOR:  newOverlay = new OFFactor(ocs, 0.0, 0.0, 10, 30, linestyle_yellow(5,1,0)); break;
//    case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, ocs, 0.0, 0.0, 10, -1, linestyle_green(1,0,0)); break;
//    case BTV_TEXT: newOverlay = new OTextTraced("CREATED", CR_RELATIVE, 0.5, 0.5, 12, true, linestyle_solid(1,0,0)); break;
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
        DrawOverlay* ovl = draws[i]->ovlGet(active_ovl);
        OVLCoordsStatic* ocs = dynamic_cast<OVLCoordsStatic*>(ovl);
        if (ocs == nullptr)
        {
          qDebug()<<"Example: Overlay Replace dynamic_cast failure!";
          continue;
        }
        DrawOverlay*  newOverlay = nullptr;
        switch (vistype)
        {
          case BTV_CIRCLE:  newOverlay = new OFCircle(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_SQUARE:  newOverlay = new OFSquareCC(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_FACTOR:  newOverlay = new OFFactor(ocs, 0.0, 0.0, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
          case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
          case BTV_TEXT: newOverlay = new OTextTraced("CREATED", ocs, 0.0, 0.0, 12, OO_INHERITED, true, linestyle_solid(1,0,0)); break;
          case BTV_BORDER: newOverlay = new OBorder(linestyle_solid(1,0,0)); break;
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
      DrawOverlay*  newOverlay = nullptr;
      COORDINATION cr = CR_RELATIVE;
      switch (vistype)
      {
        case BTV_CIRCLE:  newOverlay = new OFCircle(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_SQUARE:  newOverlay = new OFSquareCC(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_FACTOR:  newOverlay = new OFFactor(cr, 0.5, 0.5, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
        case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, cr, 0.5, 0.5, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
        case BTV_TEXT: newOverlay = new OTextTraced("CREATED", CR_RELATIVE, 0.5, 0.5, 12, OO_INHERITED, true, linestyle_solid(1,0,0)); break;
        case BTV_BORDER: newOverlay = new OBorder(linestyle_solid(1,0,0)); break;
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
      draws[i]->scalingLimitsHorz(&scmin, &scmax);
      if (to == SC_MIN_H) scmin = value;
      else scmax = value;
      draws[i]->setScalingLimitsHorz(scmin, scmax);
    }
    else if (to == SC_MIN_V || to == SC_MAX_V)
    {
      unsigned int scmin, scmax;
      draws[i]->scalingLimitsVert(&scmin, &scmax);
      if (to == SC_MIN_V) scmin = value;
      else scmax = value;
      draws[i]->setScalingLimitsVert(scmin, scmax);
    }
  }
}

void MainWindow::changePointer(int value)
{
  emit remitBounds((201 - value)/100.0f);
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
  else
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      DrawOverlayHard* povl= dynamic_cast<DrawOverlayHard*>(draws[i]->ovlGet(1));
      if (povl)
      {
        povl->setPalette(ppalettes_adv[rand() % sizeof(ppalettes_adv)/sizeof(ppalettes_adv[0])], false);
      }
    }
  }
}
