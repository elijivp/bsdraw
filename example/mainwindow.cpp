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
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QTabBar>
#include <QTextEdit>
#include <QButtonGroup>

#include "palettes/QPaletteBox.h"

#include <QSlider>
#include <QScrollBar>
#include <QScrollArea>
#include <QComboBox>
#include <QColorDialog>
#include <QResizeEvent>

#include <QFile>
#include <QtMath>

#if QT_VERSION >= 0x050000
#include <QApplication>
#include <QDateTime>
#include <QScreen>
#endif


#define REGTEST(TEST) #TEST,
static const char* g_testnames[] =
{
#include "tests.h"
};
#undef REGTEST

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
enum LAYOUT_WEIGHT{ LW_1, LW_01x, LW_1x0, LW_10x, LW_0x1 } 
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
static const char* img_path_sdp = "../example/template.png";  /// 700x800

/// if uncomment: data generation will be more in 5 times. setData called with decimation algo
//#define DECIMATION window_min
//#define DECIMATION window_max

/// if uncomment: changes 'point size' for width and height synchroniously
//#define SYNCSCALING 3

/// if uncomment: DrawScales usage
//#define USESCALES

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
//    part += QString().sprintf(i == clc-1? "0x%08x" : "0x%08x,", pptr[i]);
    part += QString("0x%1").arg(pptr[i], 8, 16, QChar('0')) + (i == clc-1? "" : ",");
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


#define TESTTRASS
#ifdef TESTTRASS
OTrass* ovl;
#endif

MainWindow::MainWindow(tests_t testnumber, QWidget *parent):  QMainWindow(parent), 
  MW_TEST(testnumber), randomer(nullptr), 
  active_ovl(0), ovl_visir(-1), ovl_marks(-1), ovl_figures(-1), ovl_snowflake(-1), ovl_active_mark(9), ovl_is_synced(true), 
  sigtype(ST_PEAK3), sig_k(1), sig_b(0), portion_delta(0.1f)
{  
  int syncscaling=0;
  drawscount = 0;
  unsigned drcount = 1, dccount = 1;
  DrawBars* drawbars[32];
  bool  drawbarsstage=false;
  
#define PRECREATE(dr, dc) drcount = dr; dccount = dc; drawscount = (dr)*(dc); draws = new DrawQWidget*[drawscount];
#define AFTERCREATE_NOSNIPPET
#define AFTERCREATE_DRAW2D \
    drcount = countROWS; dccount = countCOLUMNS;  drawscount = drcount*dccount;  draws = new DrawQWidget*[drawscount]; \
    for (unsigned int r=0; r<countROWS; r++)  for (unsigned int c=0; c<countCOLUMNS; c++)  draws[c*countROWS + r] = pdraws[r][c];
  
#define AFTERCREATE_DRAW1C \
    drcount = countDraws; dccount = 1; drawscount = drcount;  draws = new DrawQWidget*[drawscount]; \
    for (unsigned int r=0; r<countDraws; r++)  draws[r] = pdraws[r];
  
#define AFTERCREATE_DRAWBARS2D \
    drcount = countROWS; dccount = countCOLUMNS;  drawscount = drcount*dccount; draws = new DrawQWidget*[drawscount]; \
    for (unsigned int r=0; r<countROWS; r++)  for (unsigned int c=0; c<countCOLUMNS; c++)  \
    {   draws[c*countROWS + r] = pdraws[r][c];  drawbars[c*countROWS + r] = pdrawbars[r][c]; } \
    drawbarsstage = true;
  
#define AFTERCREATE_DRAWBARS1C \
    drcount = countDraws; dccount = 1; drawscount = drcount;  draws = new DrawQWidget*[drawscount]; \
    for (unsigned int i=0; i<countDraws; i++){  draws[i] = pdraws[i];  drawbars[i] = pdrawbars[i]; } \
    drawbarsstage = true;
  
#define AFTERCREATE_DRAWBARS1R \
    drcount = 1; dccount = countDraws; drawscount = dccount;  draws = new DrawQWidget*[drawscount]; \
    for (unsigned int i=0; i<countDraws; i++){  draws[i] = pdraws[i];  drawbars[i] = pdrawbars[i]; } \
    drawbarsstage = true;
  
#define AFTERCREATE_DRAWBARS1 \
    drcount = dccount = drawscount = 1;  draws = new DrawQWidget*[drawscount]; \
    draws[0] = pdraw;  drawbars[0] = pdrawbars; \
    drawbarsstage = true;
  
  
  if (MW_TEST == LET_IT_SNOW)  /// fullsize recorder LET IT SNOW!
  {
    LINES = 600;
    SAMPLES = 1600;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawRecorder(SAMPLES, LINES, 2000, PORTIONS);
  }
  else if (MW_TEST == OVERVIEW_2D_1)
  {
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
                          overpattern_any(OPF_CROSSPUFF, 0.0f, 0.5f),
                          overpattern_any(OPF_CIRCLE, 0.0f, 4),
                          overpattern_thrs_plus(OPF_CIRCLE, 0.9f, 0.0f),
      
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
    
    AFTERCREATE_DRAW2D
    sigtype = ST_RAND;
    
    /*
     *     LINES = 2;
    SAMPLES = 2;
    PORTIONS = 1;

    overpattern_t dpms[] = {  
      overpattern_any(OP_CONTOUR, 0.0f), overpattern_any(OP_LINELEFT, 0.0f), overpattern_any(OP_LINERIGHT, 0.0f), overpattern_any(OP_LINEBOTTOM, 0.0f), 
      overpattern_any(OP_LINETOP, 0.0f), overpattern_any(OP_LINELEFTRIGHT, 0.0f), overpattern_any(OP_LINEBOTTOMTOP, 0.0f), overpattern_any(OP_LINELEFTBOTTOM, 0.0f),
      overpattern_any(OP_LINERIGHTBOTTOM, 0.0f), overpattern_any(OP_LINELEFTTOP, 0.0f), overpattern_any(OP_LINERIGHTTOP, 0.0f), overpattern_any(OP_GRID, 0.0f), 
      overpattern_any(OP_DOT, 0.0f), overpattern_any(OP_DOTLEFTBOTTOM, 0.0f), overpattern_any(OP_DOTCONTOUR, 0.0f), overpattern_any(OP_CROSS, 0.0f), 
      overpattern_any(OP_SHTRICHL, 0.0f), overpattern_any(OP_SHTRICHR, 0.0f), overpattern_any(OP_FILL, 0.0f), overpattern_any(OP_SQUARES, 0.0f), 
      
      overpattern_any(OPF_CIRCLE, 0.0f), overpattern_any(OPF_CIRCLE_REV, 0.0f), overpattern_any(OPF_CROSSPUFF, 0.0f), overpattern_any(OPF_RHOMB, 0.0f), 
      overpattern_any(OPF_SURIKEN, 0.0f), overpattern_any(OPF_SURIKEN_REV, 0.0f), overpattern_any(OPF_DONUT, 0.0f), overpattern_any(OPF_CROSS, 0.0f), 
      overpattern_any(OPF_HOURGLASS, 0.0f), overpattern_any(OPF_BULL, 0.0f), overpattern_any(OPF_HOURGLASS, 0.0f), overpattern_any(OPF_BULL, 0.0f)
                       };
    
    const int countROWS = 8, countCOLUMNS = 4;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    for (unsigned int r=0; r<countROWS; r++)
      for (unsigned int c=0; c<countCOLUMNS; c++)
      {
        pdraws[r][c] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
        pdraws[r][c]->setOverpattern(dpms[r*countCOLUMNS + c]);
        pdraws[r][c]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
      }
    
    AFTERCREATE_DRAW2D
    sigtype = ST_RAND;   
    sp = SP_ONCE;
*/
  }
  else if (MW_TEST == OVERVIEW_2D_2)
  {
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
    
    AFTERCREATE_DRAW2D
    SAMPLES = SAMPLES*3;
    LINES = 6;
//    SAMPLES = SAMPLES + LINES - 1; /// reinit SAMPLES for future DSAMPLES data calculation
    sp = SP_FAST;
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == OVERVIEW_2D_3)
  {
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
    
    AFTERCREATE_DRAW1C
    sigtype = ST_SIN;
    lw = LW_0x1;
  }
  else if (MW_TEST == OVERVIEW_1D_HISTOGRAM_1)
  {
    LINES = 1;
    SAMPLES = 80;
    PORTIONS = 3;
    
    const int countDraws = 5;
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
    
    pdraws[4] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramLastBack(PR_VALUEAROUND));
    pdraws[4]->setOverpattern(overpattern_any(OP_LINELEFT, 0.0f));
    pdraws[4]->setScalingLimitsSynced(10); // 1 point now is 10x10 pixels (minimum)
    
    AFTERCREATE_DRAW1C
    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == OVERVIEW_1D_HISTOGRAM_2)
  {   
    LINES = 1;
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
    
    AFTERCREATE_DRAW2D
    
    sp = SP_SLOWEST;
    sigtype = ST_GEN_NORM;    
  }
  else if (MW_TEST == OVERVIEW_1D_1)
  {
    LINES = 1;
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
  
    AFTERCREATE_DRAW2D
    sigtype = ST_SINXX;
  }
  else if (MW_TEST == OVERVIEW_1D_2)
  {
    LINES = 1;
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
    
    AFTERCREATE_DRAW1C
    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == OVERVIEW_1D_3)
  {
    LINES = 1;
    SAMPLES = 100;
    PORTIONS = 4;
    
    const int countROWS = 7, countCOLUMNS = 2;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    const float label_xpos = 0.5f;
    
    pdraws[0][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_MONO, 0.0f, 1.0f));
    pdraws[0][0]->ovlPushBack(new OTextColored(otextopts_t("CP_MONO", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[0][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_MONO, 0.0f, 1.0f));
    pdraws[0][1]->ovlPushBack(new OTextColored(otextopts_t("CP_MONO", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[0][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[1][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED, 0.0f, 1.0f));
    pdraws[1][0]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[1][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED, 0.0f, 1.0f));
    pdraws[1][1]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[1][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[2][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED_GROSS, 0.0f, 1.0f));
    pdraws[2][0]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED_GROSS", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[2][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED_GROSS, 0.0f, 1.0f));
    pdraws[2][1]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED_GROSS", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[2][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[3][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PAINTED_SYMMETRIC, 0.0f, 1.0f));
    pdraws[3][0]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED_SYMMETRIC", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[3][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PAINTED_SYMMETRIC, 0.0f, 1.0f));
    pdraws[3][1]->ovlPushBack(new OTextColored(otextopts_t("CP_PAINTED_SYMMETRIC", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[3][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[4][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f));
    pdraws[4][0]->ovlPushBack(new OTextColored(otextopts_t("CP_REPAINTED", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[4][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f));
    pdraws[4][1]->ovlPushBack(new OTextColored(otextopts_t("CP_REPAINTED", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[4][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[5][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PALETTE, 0.0f, 1.0f));
    pdraws[5][0]->ovlPushBack(new OTextColored(otextopts_t("CP_PALETTE", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[5][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PALETTE, 0.0f, 1.0f));
    pdraws[5][1]->ovlPushBack(new OTextColored(otextopts_t("CP_PALETTE", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[5][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    pdraws[6][0] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.3f, DE_QINTERP), coloropts_t::copts(CP_PALETTE_SPLIT, 0.0f, 1.0f));
    pdraws[6][0]->ovlPushBack(new OTextColored(otextopts_t("CP_PALETTE_SPLIT", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[6][1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMax(0.0f), coloropts_t::copts(CP_PALETTE_SPLIT, 0.0f, 1.0f));
    pdraws[6][1]->ovlPushBack(new OTextColored(otextopts_t("CP_PALETTE_SPLIT", 0, 10,2,10,2), CR_RELATIVE, label_xpos, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    pdraws[6][1]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    AFTERCREATE_DRAW2D
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == EXTRA_IMPULSE)
  {
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
    AFTERCREATE_DRAW2D
    sigtype = ST_RAMP;
  }
  else if (MW_TEST == EXTRA_SMOOTH)
  {
    LINES = 1;
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
      {
        pdraws[r][c] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(smoothtest[r][c], DE_LINTERP));
        {
          QString gname = QString("smooth ") + QString::number(smoothtest[r][c]);
          pdraws[r][c]->ovlPushBack(new OTextColored(gname.toUtf8().data(), CR_RELATIVE, 0.55f, 0.05f, 12, 0x00000000, 0x44FFFFFF, 0x00000000));
        }
      }
    AFTERCREATE_DRAW2D
        
    sigtype = ST_HIPERB;
  }
  else if (MW_TEST == EXTRA_ORIENTS)
  {
    AFTERCREATE_NOSNIPPET
    LINES = 1;
    SAMPLES = 400;
    PORTIONS = 1;
    syncscaling = 0;
    PRECREATE(6, 1);
    ORIENTATION orients[] = { OR_LRBT, OR_TBLR, OR_BTRL, OR_BTLR, OR_TBRL, OR_RLTB };
    const char* ornames[] = { "LRBT",  "TBLR",  "BTRL",  "BTLR",  "TBRL",  "RLTB" };
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp2(0.5, DE_NONE), coloropts_t::copts(0x00111111));
      draws[i]->setOrientation(orients[i]);
      draws[i]->ovlPushBack(new OTextColored(ornames[i], CR_RELATIVE, i == 1 || i == 3 ? 0.6f : 0.15f, i == 0? 0.7f : 0.05f,
                                             12, 0x00FFFF00, 0xFFFFFFFF, 0x00FFFF00), OO_AREA_LRBT);
    }

    sigtype = ST_MOVE;
  }
  else if (MW_TEST == EXTRA_PALETTES)
  {
    LINES = 1;
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
  
    AFTERCREATE_DRAW2D
    sigtype = ST_TANHX;
  }
  else if (MW_TEST == EXTRA_GRAPHS_MOVE) /// graphs and graphmoves
  {
    LINES = 1;
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
    
    AFTERCREATE_DRAW2D

    sigtype = ST_GEN_NORM;
    sp = SP_FASTEST;
  }
  else if (MW_TEST == EXTRA_SCALING)
  {
    LINES = 1;
    SAMPLES = 50;
    PORTIONS = 1;
    
    const int countROWS = 4, countCOLUMNS = 3;
    DrawQWidget* pdraws[countROWS][countCOLUMNS];
    const BSDESCALING third_de = DE_CENTER;
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
    
    AFTERCREATE_DRAW2D
    sigtype = ST_SIN;
  }
  else if (MW_TEST == OVERVIEW_BARS_1)
  {
    LINES = 1;
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
    
    AFTERCREATE_DRAWBARS1C
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == OVERVIEW_BARS_2)
  {
    LINES = 1;
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
    
    AFTERCREATE_DRAWBARS1C
    sp = SP_FASTEST;
    sigtype = ST_MANYSIN;
  }
  else if (MW_TEST == OVERVIEW_BARS_3)
  {
    LINES = 1;
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
    
    
    AFTERCREATE_DRAWBARS1
    sp = SP_ONCE;
    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == DEMO_1) /// Demo 1
  {
    SAMPLES = 280;
    LINES = 200;
    PORTIONS = 3;
    PRECREATE(3, 3);
    draws[0] = new DrawIntensity(SAMPLES, LINES, 1);
    draws[0]->ovlPushBack(new OTextColored("Intensity", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000));
    draws[3] = new DrawDomain(SAMPLES, LINES, 1, false, OR_LRBT, true);
    draws[3]->ovlPushBack(new OTextColored("Domain", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000));
    {
      DIDomain& ddm = *((DrawDomain*)draws[3])->domain();
      
      for (int j=0; j<SAMPLES; j++)
      {
        ddm.start();
        for (unsigned int r=0; r<LINES/2; r++)
          ddm.includePixel(int(LINES/2 + sin(j/(2.0*M_PI*8))*LINES/4 - LINES/4 + r), j);
        ddm.finish();
      }
    }
    draws[6] = new DrawRecorder(SAMPLES, LINES);
    draws[6]->ovlPushBack(new OTextColored("Recorder", CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000), OO_AREA_LRBT);
    
    graphopts_t  gopts[] = { graphopts_t::goDots(), 
                             graphopts_t::goInterp(0.6f, DE_NONE), 
                             graphopts_t::goHistogramCrossMin()
                           };
    const char*  gnames[] = { "Graph (dots)", "Graph (linterp)", "Graph (histogram)" };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      draws[3*i + 1]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000));
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t) - 1; i++)
    {
      gopts[i].dotsize = i == 2? 0 : i == 1? 4 : 2;
//      gopts[i].dotsmooth = i == 0? 0.5f : 0.1f;
      gopts[i].dotsmooth = i == 0? 0.1f : 1.0f;
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i], coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f/*, 0x00777777*/));
      draws[3*i + 2]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000));
    }
    
    draws[8] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(0.3f, DE_NONE, 0.15f));
    draws[8]->ovlPushBack(new OTextColored(gnames[2], CR_XABS_YREL_NOSCALED, 10.0f, 0.05f, 12, 0x00000000, 0x77FFFFFF, 0x00000000));
    
    sigtype = ST_SINXX;
    
//    for (int i=0; i<drcount*dccount; i++)
//      draws[i]->ovlPushBack(new OShadow(OBLINE_LEFT | OBLINE_TOP | OBLINE_RIGHT | OBLINE_BOTTOM, 4, 0.65f));
//      draws[i]->ovlPushBack(new OShadow(3, 3, 4, 4, 0.65f));
  }
  else if (MW_TEST == DEMO_2_scaling)  /// Demo 2
  {
    SAMPLES = 70;
    LINES = 50;
    PORTIONS = 2;
    syncscaling = 4;
    PRECREATE(3, 3);
    overpattern_t dpmcontour = overpattern_thrs_minus(OP_LINELEFTBOTTOM, 0.0f, 0);
    draws[0] = new DrawIntensity(SAMPLES, LINES, 1);
    draws[0]->setOverpattern(dpmcontour);
    draws[3] = new DrawDomain(SAMPLES, LINES, 1, false, OR_LRBT, true);
    draws[3]->setOverpattern(dpmcontour);
    {
      DIDomain& ddm = *((DrawDomain*)draws[3])->domain();
      
      for (int j=0; j<SAMPLES; j++)
      {
        ddm.start();
        for (int r=0; r<LINES/2; r++)
          ddm.includePixel(LINES/2 + sin(j/(2.0*M_PI*8))*LINES/4 - LINES/4 + r, j);
        ddm.finish();
      }
    }
    draws[6] = new DrawRecorder(SAMPLES, LINES);
    draws[6]->setOverpattern(dpmcontour);
    
    graphopts_t  gopts[] = { graphopts_t::goDots(), 
                             graphopts_t::goInterp(0.3f, DE_NONE), 
                             graphopts_t::goHistogramCrossMax()
                           };
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      draws[3*i + 1]->setOverpattern(dpmcontour);
    }
    
    for (unsigned int i=0; i<sizeof(gopts)/sizeof(graphopts_t); i++)
    {
      draws[3*i + 2] = new DrawGraph(SAMPLES, PORTIONS, gopts[i]);
      if (i == 0)
        draws[3*i + 2]->setOverpattern(overpattern_any(OPF_CROSSPUFF, color3f(0.0f,0.1f,0.0f)));
      else
        draws[3*i + 2]->setOverpattern(overpattern_any(OP_LINELEFT, color3f(0.0f,0.0f,0.0f), 1));
    }
    
    sigtype = ST_SINXX;
  }  
  else if (MW_TEST == DEMO_3_overlays) /// std
  {
    SAMPLES = 600;
    LINES = 200;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
      draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_NONE));
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, LINES, 1000, PORTIONS);
    }
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEMO_4_portions) /// Different Portions
  {
    SAMPLES = 400;
    LINES = 100;
    PORTIONS = 3;
    PRECREATE(3, 2);
    graphopts_t gopts = graphopts_t::goInterp(0.5f, DE_NONE);
    for (unsigned int i=0; i<dccount; i++)
    {
      draws[i*drcount + 0] = new DrawIntensity(SAMPLES, LINES, i == 1? 1 : PORTIONS);
      draws[i*drcount + 0]->setDataPalette(&paletteRGB);
      if (i == 0)
//        draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_MONO, 0.0f, 0.7f, 0x00111111));
        draws[i*drcount + 1] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_MONO, 0.0f, 1.0f, 0x00111111));
      else
        draws[i*drcount + 1] = new DrawGraph(SAMPLES, 1, gopts, coloropts_t::copts(CP_REPAINTED, 0.0f, 1.0f, 0x00111111));
      draws[i*drcount + 1]->setDataPalette(&paletteRGB);
      draws[i*drcount + 2] = new DrawRecorder(SAMPLES, LINES, 1000, i == 1? 1 : PORTIONS);
      draws[i*drcount + 2]->setDataPalette(&paletteRGB);
    }
    sigtype = ST_SIN;
  }
  else if (MW_TEST == DRAW_BRIGHT)  /// brights for ovls
  {    
    SAMPLES = 100;
    LINES = 100;
    PORTIONS = 1;
    
    PRECREATE(2, 2);
    unsigned int msc = 5;
    draws[0] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
    draws[0]->setOverpattern(overpattern_thrs_minus(OP_LINELEFTBOTTOM, 0.02f, 0x0));
    draws[0]->setScalingLimitsSynced(msc, msc);
    
    draws[1] = new DrawIntensity(SAMPLES/2, 1, PORTIONS);
    draws[1]->setOverpattern(overpattern_thrs_minus(OP_LINELEFT, 0.2f, 0x0));
    draws[1]->setScalingLimitsHorz(msc*2);
    draws[1]->setScalingLimitsVert(msc*2, msc*2);
    
    draws[2] = new DrawIntensity(SAMPLES/2, 1, PORTIONS);
    draws[2]->setOrientation(OR_TBLR);
    draws[2]->setOverpattern(overpattern_thrs_minus(OP_LINELEFT, 0.2f, 0x0));
    draws[2]->setScalingLimitsHorz(msc*2);
    draws[2]->setScalingLimitsVert(msc*2, msc*2);
    
    draws[3] = new DrawIntensity(1, 1, PORTIONS);
    draws[3]->setOverpattern(overpattern_thrs_minus(OPF_CIRCLE, 0.2f, 0x0));
    draws[3]->setScalingLimitsSynced(msc*2, msc*2);
    
    lw = LW_10x;
  }
  else if (MW_TEST == DRAW_DOMAIN)   /// domain
  { 
    SAMPLES = 75;
    LINES = 50;
    PORTIONS = 1;
    PRECREATE(2, 2);
    
    syncscaling = 5;
    {
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      for (int i=0; i<8; i++)
      {
        for (int j=0; j<10; j++)
        {
          int r = LINES / 16 + i * LINES / 8, c = int(SAMPLES/20.0 + j*SAMPLES/10.0);
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
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      for (int j=0; j<SAMPLES + LINES; j++)
      {
        ddm.start();
        for (int r=0; r<LINES; r++)
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
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = 600;
      const unsigned int outsider = 1700;
      const double wc = 1.0/(2.0*M_PI);
      for (int i=0; i<maxspiral; i++)
      {
        int y = qRound(LINES/2.0 + outsider*sin((i+1)*wc)/(i+1)), x = qRound(SAMPLES/2.0 + outsider*cos((i+1)*wc)/(i+1));
        if (y >= 0 && y < LINES && x >= 0 && x < SAMPLES)
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
      DrawDomain* dd = new DrawDomain(SAMPLES, LINES, PORTIONS, false, OR_LRBT, true);
      DIDomain& ddm = *dd->domain();
      const int maxspiral = 600;
      const unsigned int outsider = 6000;
      const double wc = 3.0/(2.0*M_PI);
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
//      qDebug()<<"Example: Total spiral _2_ points: "<<ddm.count();
      draws[3] = dd;
    }
    SAMPLES = SAMPLES + LINES - 1; /// reinit SAMPLES for future DSAMPLES data calculation
    
    sp = SP_FAST;
    sigtype = ST_MANYSIN;
  }
  else if (MW_TEST == DRAW_RECORDER)  /// recorders
  {
    SAMPLES = 300;
    LINES = 400;
    PORTIONS = 1;
    PRECREATE(1, 2);
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawRecorder(SAMPLES, LINES, 2000, PORTIONS);
    
#ifdef TESTTRASS
    {
      ovl = new OTrassSelectable(15, 400, &palette_idl_BuGn, false, 400);
      trasspoint_t tline[15];
      for (int i=0; i<200; i++)
      {
        for (int j=0; j<15; j++)
        {
//          tline[j].intensity = j == 5? float(i % 2) : 1.0f;
//          tline[j].intensity = j == 5? 0 : (j + 2)/17.0f;
//          tline[j].intensity = j == 5? 0 : j == 10? 1.0f : 0.5f;
          tline[j].intensity = j == 5? 0 : 0.9f;
          tline[j].position = ((SAMPLES/30.0f + SAMPLES/15.0f*j) + (rand()/float(RAND_MAX)-0.5f)*4)/float(SAMPLES);
          tline[j].halfstrob = 0.01f;
        }
        ovl->appendTrassline(tline);
      }
      ((OTrassSelectable*)ovl)->select(1);
      for (unsigned int i=0; i<drawscount; i++)
        draws[i]->ovlPushBack(ovl);
    }
    
    sp = SP_FAST;
    sigtype = ST_MANYSIN;
#endif
  }
  else if (MW_TEST == DRAW_POLAR) /// polar draw
  {
    SAMPLES = 128;
    LINES = 225;
//    PORTIONS = 4;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = LINES > 30? 0 : (30-LINES)*6;
    impulsedata_t imp[] = { { impulsedata_t::IR_OFF },
                            { impulsedata_t::IR_A_COEFF, 5, 5/2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
//                            { impulsedata_t::IR_A_COEFF, 3, 3/2, 0, { 0.25f, 0.5f, 0.25f } },
                            { impulsedata_t::IR_A_COEFF_NOSCALED, 5, 5/2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } },
                            { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 4, 6, 1, {} },
                            { impulsedata_t::IR_A_BORDERS, 2, 2, 1, {} },
    };
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i] = new DrawPolar(SAMPLES, LINES, PORTIONS, 0x00000000, SP_ROWS_LR_2);
      draws[i] = new DrawPolar(SAMPLES, LINES, PORTIONS, 0, 0.0f, 0x00000000, SP_NONE);
      draws[i]->setImpulse(imp[1]);
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
    LINES = 1;
    PORTIONS = 2;
    PRECREATE(3, 3);
    BSGRAPHTYPE     gts[] = { GT_HISTOGRAM_CROSSMAX, GT_LINTERP, GT_DOTS };
    BSCOLORPOLICY   dclr[] = { CP_MONO, CP_MONO, CP_REPAINTED };
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
        coloropts_t copts = {   dclr[c], 0.0f, 1.0f, c == 1? 0xFFFFFFFF : 0x00999999 };
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gopts, copts);
      }
  
    sigtype = ST_SINXX;
  }
  else if (MW_TEST == DRAW_HISTOGRAMS)   /// pixelation
  {
    SAMPLES = 80;
    LINES = 20;
    PORTIONS = 3;
    PRECREATE(4, 1);
    syncscaling = 10;
    graphopts_t gts[] = { graphopts_t::goHistogram(), graphopts_t::goHistogramCrossMin(), graphopts_t::goHistogramCrossMax(), graphopts_t::goInterp(0.5f, DE_NONE) };
    overpattern_t fsp[] = {   overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)), 
                              overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)), 
                              overpattern_thrs_plus(OP_LINELEFTTOP, 0.0f, color3f(0.3f,0.3f,0.3f)), 
                              overpattern_any(OPF_CIRCLE, color3f(0.1f,0.1f,0.1f))
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
      draws[i]->setOverpattern(fsp[i]);
      
      draws[i]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED_SCALED, 10.0f, 0.85f, 12, 0x00000000, 0x33FFFFFF, 0x00000000));
    }

    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == DRAW_HISTOGRAMS_2)   /// pixelation2
  {   
    SAMPLES = 30;
    LINES = 20;
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
        draws[c*drcount + i]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, 0x00000000, c == 1? i == 2? 4 : 1 : 0));
      }
    
    sp = SP_SLOWEST;
    sigtype = ST_GEN_NORM;    
  }
  else if (MW_TEST == DRAW_SCALES_1)
  {
    SAMPLES = 180;
    LINES = 50;
    PORTIONS = 1;
    PRECREATE(2, 1);
    for (int i=0; i<drawscount; i++)
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP), coloropts_t::copts(CP_MONO, 1.0f, 1.0f, 0x777777));
//    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DRAW_SCALES_2)
  {
    SAMPLES = 180;
    LINES = 50;
    PORTIONS = 1;
    PRECREATE(4, 1);
//    syncscaling = 4;
    
//    DrawGraph::BSCOLORPOLICY cps[] = { CP_MONO, CP_PAINTED, CP_PAINTED_GROSS, CP_PAINTED_SYMMETRIC, CP_REPAINTED, CP_PALETTE };
//    const char* cpnames[] = { "CP_MONO", "CP_PAINTED", "CP_PAINTED_GROSS", "CP_PAINTED_SYMMETRIC", "CP_REPAINTED", "CP_PALETTE" };
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_QINTERP), coloropts_t::copts(CP_MONO, 0.3f, 1.0f, i == 2? 0x00AAAAAA : 0xFFFFFFFF));
//      draws[i]->setScalingLimitsHorz(7);
//      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.8f, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
//    this->setMinimumHeight(1000);
//    this->setMinimumWidth(1200);
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DRAW_SCALES_3)
  {
    SAMPLES = 180;
    LINES = 50;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goDots(2, 1.0f, DE_NONE), coloropts_t::copts(CP_MONO, 1.0f, 0.3f, 0xFFFFFFFF));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.5f, 0.125f, linestyle_inverse_1(4,1,0)));
      draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.5f, 0.125f, linestyle_inverse_1(4,1,0)));
    }
    sigtype = ST_GEN_NORM;
  }
  else if (MW_TEST == DRAW_SDPICTURE)
  {
    SAMPLES = 500;
    LINES = 700;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawSDPicture(SAMPLES, LINES, img_path_sdp /*"/home/elijah/Projects/schema/schemod.png"*/);
      draws[i]->setRawResizeModeNoScaled(true);
    }
//    draws[2]->setOverpattern(overpattern_any(OP_DOTCONTOUR, 0, 0.0f, 0.0f));
    sigtype = ST_MANYSIN;
    sp = SP_FAST;
//    sigtype = ST_ZOO;
//    sp = SP_ONCE;
//    defaultPalette = &paletteSemaphoreWRYG;
  }
  else if (MW_TEST == FEATURE_PORTIONS)
  {
    SAMPLES = 300;
    LINES = 10;
    int PORTIONS_MIN = 3;
    PORTIONS = 29;
    PRECREATE(5, 1);
    
    graphopts_t  gopts = { GT_LINTERP, DE_LINTERP, 0.0f, 0, 0.0f, 0.6f, PR_STANDARD };
//    graphopts_t  gopts = { GT_DOTS, DE_NONE, 0.0f, 0, 0.0f, 0.2f, PR_STANDARD };
    draws[0] = new DrawGraph(SAMPLES, PORTIONS_MIN, gopts, coloropts_t::copts(CP_MONO, 0.332f, 1.0f));
    draws[0]->setScalingLimitsVert(1);
    
    draws[1] = new DrawRecorder(SAMPLES, 100, 100, PORTIONS_MIN);
    draws[1]->setDataTextureInterpolation(true);
    
    draws[2] = new DrawIntensity(SAMPLES, LINES, PORTIONS_MIN);
    draws[2]->setDataTextureInterpolation(true);
    draws[2]->setScalingLimitsVert(10,10);

    draws[3] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_MONO, 0.332f, 1.0f), SP_COLUMN_TB);
//    draws[3]->ovlPushBack(new OFLine(OFLine::LT_VERT_BYBOTTOM, CR_RELATIVE, 0.5f, 0.0f, CR_ABSOLUTE, 0));
    
    draws[4] = new DrawIntensity(SAMPLES, LINES, PORTIONS, OR_LRBT, SP_COLUMNS_TB_2);
    draws[4]->ovlPushBack(new OBorder(linestyle_solid(1.0f, 1.0f, 1.0f)));
//    draws[4] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(CP_MONO, 0.332f, 1.0f), SP_COLUMN_BT);
//    setMinimumWidth(1200); ??? 

//    sigtype = ST_RAMP;
    for (int i=0; i<drawscount; i++)
      draws[i]->setDataPalette(i<3? (const IPalette*)&paletteRGB: (const IPalette*)&palette_mat_hot);
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == FEATURE_GRAPH_SMOOTH)
  {
    SAMPLES = 280;
    LINES = 200;
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
          draws[c*drcount + i]->ovlPushBack(new OTextColored(gname.toUtf8().data(), CR_RELATIVE, 0.55f, 0.05f, 12, 0x00000000, 0x44FFFFFF, 0x00000000));
        }
      }
    
    sigtype = ST_HIPERB;
    
//    sigtype = ST_SINXX;
//    sp = SP_SLOWEST;
//    setMinimumSize(2400, 1000);
  }
  else if (MW_TEST == FEATURE_INTERPOLATION)
  {
    SAMPLES = 60;
    LINES = 200;
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
      draws[i]->ovlPushBack(new OTextColored(gnames[i], CR_XABS_YREL_NOSCALED, 10.0f, 0.65f, 12, 0x00000000, 0x44FFFFFF, 0x00000000));
      draws[i]->ovlPushBack(new OTextColored("Resize me", CR_XABS_YREL_NOSCALED, 10.0f, 0.15f, 12, 0x00000000, 0x44FFFFFF, 0x00000000));
    }
    
    this->setMinimumWidth(1200);
    sigtype = ST_GEN_NORM;
    
//    sigtype = ST_MANYSIN;
//    sp = SP_ONCE;
  }
  else if (MW_TEST == FEATURE_COLORS_DOTS || MW_TEST == FEATURE_COLORS_INTERP || MW_TEST == FEATURE_COLORS_HISTORGRAM)
  {
    SAMPLES = 180;
    LINES = 70;
    PORTIONS = 4;
    PRECREATE(7, 1);
//    syncscaling = 4;
    
    BSCOLORPOLICY cps[] = { CP_MONO, CP_PAINTED, CP_PAINTED_GROSS, CP_PAINTED_SYMMETRIC, CP_REPAINTED, CP_PALETTE, CP_PALETTE_SPLIT };
    const char* cpnames[] = { "CP_MONO", "CP_PAINTED", "CP_PAINTED_GROSS", "CP_PAINTED_SYMMETRIC", "CP_REPAINTED", "CP_PALETTE", "CP_PALETTE_SPLIT" };
    
    graphopts_t gopts = MW_TEST == FEATURE_COLORS_DOTS?   graphopts_t::goDots(3, 0.5f) : 
                        MW_TEST == FEATURE_COLORS_INTERP? graphopts_t::goInterp(0.3f, DE_QINTERP) : 
                                                          graphopts_t::goHistogramCrossMax(0.0f);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraph(SAMPLES, PORTIONS, gopts, coloropts_t::copts(cps[i], 0.0f, 1.0f));
      if (MW_TEST == FEATURE_COLORS_HISTORGRAM)
        draws[i]->setOverpattern(overpattern_thrs_plus(OP_LINELEFT, 0.0f, color3f(0.3f,0.3f,0.3f)));
      draws[i]->setScalingLimitsHorz(7);
      
      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.8f, 0.7f, 12, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
//    this->setMinimumHeight(1000);
//    this->setMinimumWidth(1200);
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == VERTICAL)
  {    
    SAMPLES = 600;
    LINES = 200;
    PORTIONS = 2;
    PRECREATE(4, 1);
    draws[0] = new DrawGraph(SAMPLES/8, PORTIONS, graphopts_t::goHistogram());
    draws[0]->setScalingLimitsB(8,8);
    draws[0]->setOverpattern(overpattern_thrs_plus(OP_CONTOUR, 0.0f, color3f(0.3f,0.3f,0.3f)));
    
    draws[1] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.5f, DE_NONE));
    
    draws[2] = new DrawRecorder(SAMPLES, LINES, 1000, PORTIONS);
    
    draws[3] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogramCrossMin());
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i]->setMinimumWidth(LINES);
      draws[i]->setOrientation(i != 3? OR_TBLR : OR_TBRL);
    }
    
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == IMPULSE_HORZ)
  {
    SAMPLES = 5;
    LINES = 1;
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
        draws[c*drcount + i] = new DrawIntensity(SAMPLES, LINES, 1);
        draws[c*drcount + i]->setImpulse(imp[c*drcount + i]);
        draws[c*drcount + i]->setScalingLimitsA(50);
        draws[c*drcount + i]->setScalingLimitsB(50);
        draws[c*drcount + i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[c*drcount + i], 0, 10,2,10,2), CR_RELATIVE, 0.05f, 0.05f, 8, 0x00000000, 0x11FFFFFF, 0x00000000));
      }
    sigtype = ST_RAMP;
//    sigtype = ST_SIN;
  }
  else if (MW_TEST == IMPULSE_VERT)
  {
    SAMPLES = 1;
    LINES = 5;
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
      draws[i] = new DrawIntensity(SAMPLES, LINES, 1);
      draws[i]->setImpulse(imp[i]);
      draws[i]->setScalingLimitsA(50);
      draws[i]->setScalingLimitsB(50);
      draws[i]->ovlPushBack(new OTextColored(otextopts_t(cpnames[i], 0, 10,2,10,2), CR_RELATIVE, 0.05f, 0.05f, 8, 0x00000000, 0x11FFFFFF, 0x00000000));
    }
    sigtype = ST_PEAK;
  }
  else if (MW_TEST == DRAW_BRIGHT_CLUSTER) /// bright cluster
  {
    SAMPLES = 30;
    LINES = 30;
    PORTIONS = 1;
    PRECREATE(1, 1);
    syncscaling = 14;
    for (unsigned int i=0; i<drawscount; i++)
      draws[i] = new DrawIntensePoints(SAMPLES, LINES, PORTIONS);
  }
  else if (MW_TEST == HINTS)
  {
    SAMPLES = 180;
    LINES = 50;
    PORTIONS = 3;
    
    int TD = PORTIONS+1;
    PRECREATE(TD, PORTIONS);
    for (int i=0; i<PORTIONS; i++)
    {
      draws[TD*i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(0.2f, DE_QINTERP), coloropts_t::copts(CP_MONO, 0.0f, 1.0f, 0x777777));
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
  }
  else if (MW_TEST == ADV_PALETTES)    /// advanced palettes show
  {
    SAMPLES = 400;
    LINES = 20;
    PORTIONS = 1;
    syncscaling = 1;
    
    PRECREATE(sizeof(ppalettes_adv) / sizeof(const IPalette*), 1);
//    PRECREATE(4, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, LINES);
      draws[i]->setDataPalette(ppalettes_adv[i]);
      draws[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    }
    sigtype = ST_RAMP;
  }
  else if (MW_TEST == DEBUG_LOADING)
  {
    SAMPLES = 400;
    LINES = 160;
    PORTIONS = 1;
    syncscaling = 0;
    PRECREATE(12, 4);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawGraphMove(SAMPLES, 1, PORTIONS, graphopts_t::goInterp2(DE_NONE), coloropts_t::copts(CP_MONO, 1.0, 0.5, 0x00111111));
    }
    
    sigtype = ST_10;
  }
  else if (MW_TEST == DEBUG_TABS)
  {
    SAMPLES = 80;
    LINES = 120;
    PORTIONS = 1;
    PRECREATE(3, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawRecorder(SAMPLES, LINES, 1000, PORTIONS);
      draws[i]->setScalingLimitsSynced(2 + i, 2 + i);
      
    }
    draws[2]->setOverpattern(overpattern_thrs_minus(OP_LINERIGHTTOP, 0.2f, 0x00333333));

    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEBUG_VOCAB)
  {
    SAMPLES = 300;
    LINES = 450;
    PORTIONS = 1;
    PRECREATE(1, 2);
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
      draws[i] = new DrawEmpty(SAMPLES, LINES, 0x00333333);
    }
//    draws[2]->setOverpattern(overpattern_any(OP_DOTCONTOUR, 0, 0.0f, 0.0f));
    sigtype = ST_10;
  }
  else if (MW_TEST == DEBUG_PALETTE)
  {
    const int FIXEDCOUNT=300;
    SAMPLES = FIXEDCOUNT;
    LINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
      draws[i]->setScalingLimitsA(4);
    }
    
    const unsigned int colors_creature[] = { 
      0x00010100, 0x00030300, 0x00050500, 0x00070700, 0x00090900, 0x000b0a00, 0x000d0c00, 0x000f0e00,
      0x00110f00, 0x00131100, 0x00151200, 0x00171300, 0x00191400, 0x001b1500, 0x001d1600, 0x001f1700, 
      0x00211800, 0x00231900, 0x00251a00, 0x00271a00, 0x00291b00, 0x002b1c00, 0x002d1c00, 0x002f1c00,
      0x00311d00, 0x00331d00, 0x00351d00, 0x00371d00, 0x00391d00, 0x003b1d00, 0x003d1d00, 0x003f1c00,
      0x00411c00, 0x00431b00, 0x00451b00, 0x00471a00, 0x00491a00, 0x004b1900, 0x004d1800, 0x004f1700,
      0x00511600, 0x00531500, 0x00551400, 0x00571300, 0x00591200, 0x005b1000, 0x005d0f00, 0x005f0d00, 
      0x00610c00, 0x00630a00, 0x00650900, 0x00670700, 0x00690500, 0x006b0300, 0x006d0100, 0x006f0001, 
      0x00710003, 0x00730006, 0x00750008, 0x0077000a, 0x0079000d, 0x007b0010, 0x007d0012, 0x007f0015, 
      0x00800018, 0x0082001b, 0x0084001e, 0x00860021, 0x00880024, 0x008a0027, 0x008c002a, 0x008e002d, 
      0x00900031, 0x00920034, 0x00940038, 0x0096003c, 0x0098003f, 0x009a0043, 0x009c0047, 0x009e004b, 
      0x009d0050, 0x009b0155, 0x0099025a, 0x00970360, 0x00950465, 0x0093056a, 0x00910670, 0x008f0775, 
      0x008e087a, 0x008c097f, 0x008a0a85, 0x00880b8a, 0x00860c8f, 0x00840d95, 0x00820e9a, 0x00800f9f, 
      0x007e10a5, 0x007d11aa, 0x007b12af, 0x007913b4, 0x007714ba, 0x007515bf, 0x007316c4, 0x007117ca, 
      0x006f18cf, 0x006e19d4, 0x006c1ad9, 0x006a1bdf, 0x00681ce4, 0x00661de9, 0x00641eef, 0x00621ff4, 
      0x005e21ff, 0x005a23ff, 0x005725ff, 0x005427ff, 0x005129ff, 0x004e2bff, 0x004c2dff, 0x00492fff, 
      0x004631ff, 0x004433ff, 0x004135ff, 0x003f37ff, 0x003d39ff, 0x003b3bff, 0x003d3fff, 0x003f44ff,
      0x004149ff, 0x00434eff, 0x004552ff, 0x004757ff, 0x00495bff, 0x004b5fff, 0x004d64ff, 0x004f68ff,
      0x00516cff, 0x005371ff, 0x005575ff, 0x005779ff, 0x00597dff, 0x005b81ff, 0x005d84ff, 0x005f88ff, 
      0x00618cff, 0x006390ff, 0x006593ff, 0x006797ff, 0x00699bff, 0x006b9eff, 0x006da1ff, 0x006fa5ff, 
      0x0071a8ff, 0x0073abff, 0x0075afff, 0x0077b2ff, 0x0079b5ff, 0x007bb8ff, 0x007dbbff, 0x007fbeff, 
      0x0080c1ff, 0x0082c3ff, 0x0084c6ff, 0x0086c9ff, 0x0088cbff, 0x008aceff, 0x008cd0ff, 0x008ed3ff, 
      0x0090d5ff, 0x0092d8ff, 0x0094daff, 0x0096dcff, 0x0098deff, 0x009ae0ff, 0x009ce2ff, 0x009ee4ff, 
      0x00a0e6ff, 0x00a2e8ff, 0x00a4eaff, 0x00a6ecff, 0x00a8edff, 0x00aaefff, 0x00acf1ff, 0x00aef2ff, 
      0x00b0f4ff, 0x00b2f5ff, 0x00b4f6ff, 0x00b6f8ff, 0x00b8f9ff, 0x00bafaff, 0x00bcfbff, 0x00befcff, 
      0x00c0fdff, 0x00c2feff, 0x00c4ffff, 0x00c6fffe, 0x00c8fffd, 0x00cafffd, 0x00ccfffc, 0x00cefffc, 
      0x00d0fffb, 0x00d2fffa, 0x00d4fffa, 0x00d6fffa, 0x00d8fff9, 0x00dafff9, 0x00dcfff9, 0x00defff9,
      0x00e0fff9, 0x00e2fff9, 0x00e4fff9, 0x00e6fff9, 0x00e8fff9, 0x00eafff9, 0x00ecfffa, 0x00eefffa, 
      0x00f0fffa, 0x00f2fffb, 0x00f4fffb, 0x00f6fffc, 0x00f8fffc, 0x00fafffd, 0x00fcfffe, 0x00feffff
    };
    
    
    const int cc = 32;
    static unsigned int part[cc];
    int p0[] = { 0x9e, 0x00, 0x4b };
    int p1[] = { 0x5e, 0x21, 0xff };
    for (int i=0; i<cc; i++)
    {
      int clr = 0;
      for (int c=0; c<3; c++)
        clr |= (p0[c] + int((p1[c]-p0[c])/float(cc+2)*(i+1))) << ((3-1-c)*8);
      part[i] = clr;
    }
    qDebug()<<palette2string(QString("%1").arg("part"), part, cc).toStdString().c_str();
    
////    static unsigned int part[4*8];
////    int nko = 0;
////    for (int i=0; i<4*8; i++)
////    {
////      part[32-1-i] = ((0xFF - nko)<<16) | 0x0000FFFF;
////      nko += i%3? 8:7;
////    }
    
//    static unsigned int part[256];
//    for (int i=0; i<256; i++)
//      part[256-1-i] = colors_creature[i];
//    qDebug()<<palette2string(QString("%1").arg("creature"), part, 256).toStdString().c_str();
      
    
    
    static const PaletteConstFWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature(colors_creature);
    static const PaletteConstBWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature_inv(colors_creature);
    
    typedef PaletteConstFWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature_t;
    for (unsigned int i=0; i<drawscount; i++)
      draws[i]->setDataPalette(&palette_creature);
    
    sigtype = ST_RAMP;
    
    float data[FIXEDCOUNT];
//    const int LIMIT = 40;
//    for (int i=0; i<LIMIT; i++)
//    {
//      data[i] = i/float(LIMIT);
//    }
//    for (int i=LIMIT; i<FIXEDCOUNT; i++)
//    {
////      data[i] = (i % 3)*pimp.multiplier();
////      qDebug()<<data[i];
//      data[i] = pimp(i % 7);
//    }
//    for (int i=0; i<FIXEDCOUNT; i++)
//      data[i] = pimp(i % 4);
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i]->setData(data);
    }
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
    LINES = 100;
    PRECREATE(1, 2);
    for (int i=0; i < drawscount; i++)
    {
      draws[i] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
//      draws[i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goHistogram(0.0f, DE_NONE), coloropts_t::copts(CP_PALETTE, 1.0f, 1.0f, 0x00000000));
      draws[i]->setScalingLimitsSynced(6);
      draws[i]->setClearByPalette();
      draws[i]->setDataPalette(pptr[i]);
      draws[i]->setDataPaletteDiscretion(true);
    }
//    sigtype = ST_ONE;
    sp = SP_ONCE;
  }
  else if (MW_TEST == DEBUG_ROTATIONSOVLS) 
  {
    SAMPLES = 23;
    LINES = 51;
    PORTIONS = 1;
    
    PRECREATE(4, 2);
    OActiveCursor* oac = new OActiveCursor(CR_RELATIVE, -1,-1);
    
    
    for (unsigned int i=0; i<drcount; i++)
      for (unsigned int j=0; j<dccount; j++)
      {
        draws[i*dccount + j] = new DrawIntensity(SAMPLES, LINES, 1, ORIENTATION(i*dccount + j));
        draws[i*dccount + j]->setScalingLimitsA(1,1);
        draws[i*dccount + j]->setScalingLimitsB(1,1);
        
        int id = draws[i*dccount + j]->ovlPushBack(oac);
        OFLine* oaf = new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_RELATIVE, 0.0f, -1, linestyle_red(1,0,0));
        draws[i*dccount + j]->ovlPushBack(oaf, id);
      }
    sigtype = ST_ZERO;
  }
  else if (MW_TEST == DEBUG_HISTO3)
  {   
    SAMPLES = 192;
    LINES = 1;
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
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, gts[c*drcount + i], coloropts_t::copts(CP_MONO, 1.0f, 0.49f) );
        draws[c*drcount + i]->setScalingLimitsHorz(1);
        draws[c*drcount + i]->setScalingLimitsVert(1);
        draws[c*drcount + i]->setDataPalette(&paletteRG);
//        draws[c*drcount + i]->setOverpattern(DPostmask::postmask(PO_SIGNAL, OP_CONTOUR, 0));
      }
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
    draws[0]->setMinimumWidth(1000);
//    defaultPalette = &paletteBY;
  }
  else if (MW_TEST == DEBUG_OVERLAYAREA)
  {
    SAMPLES = 1;
    LINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
        draws[c*drcount + i] = new DrawIntensity(SAMPLES, LINES, PORTIONS);
    
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)    //linestyle_green(4, 2, 0)
      {
        OCluster* pMKFigures = new OCluster(false, 32, CR_RELATIVE, 0.05f, &paletteGnWh, false, 0.15f);
        {
          for (int r=0; r<8; r++)
            for (int c=0; c<3; c++)
              pMKFigures->updateItem(r*3 + c, 0.1f + 0.2f * c, 0.1f + 0.1f * r, 0.5f, (OCluster::FFORM)(r+1), 1.0f);
          pMKFigures->updateFinished();
        }
        draws[c*drcount + i]->ovlPushBack(pMKFigures);
      }
    sp = SP_SLOWEST;
    sigtype = ST_ZERO;
  }
  else if (MW_TEST == DEBUG_MEVSCALES)
  {   
    SAMPLES = 192;
    LINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP), coloropts_t::copts(CP_MONO, 1.0f, 0.49f));
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEBUG_MAINSCALE)
  {   
    SAMPLES = 512;
    LINES = 1;
    PORTIONS = 1;
    PRECREATE(3, 1);
//    PRECREATE(1, 1);
    for (unsigned int c=0; c<dccount; c++)
      for (unsigned int i=0; i<drcount; i++)
        draws[c*drcount + i] = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp(DE_LINTERP), coloropts_t::copts(CP_MONO, 1.0f, 0.49f));
    
    sp = SP_SLOWEST;
    sigtype = ST_MOVE;
  }
  else if (MW_TEST == DEBUG_GRAPHEX)
  {   
    SAMPLES = 512;
    LINES = 1;
    PORTIONS = 1;
    PRECREATE(1, 1);
    draws[0] = new DrawGraphDyport(1,1600, PORTIONS, graphopts_t::goInterp(DE_LINTERP), coloropts_t::copts(CP_MONO, 1.0f, 0.49f));
    sp = SP_SLOWEST;
    sigtype = ST_RAMP;
  }
  
  
#ifdef SYNCSCALING
  syncscaling = SYNCSCALING;
#endif
  
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
//      draws[i]->setRawResizeModeNoScaled(true);
      if (syncscaling > 0)
      {
        draws[i]->setScalingLimitsSynced(syncscaling);
      }
      if (MW_TEST == LET_IT_SNOW)
      {
        Ovldraw* oimg = new OImageStretched(new QImage(img_path_mikey), OVLQImage::IC_BLOCKALPHA);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.1);
        draws[i]->ovlPushBack(oimg, OO_AREA_LRBT);
        ovl_snowflake = draws[i]->ovlPushBack(new OSnowflake(new QImage(img_path_sprite), OSnowflake::IC_AUTO, SAMPLES/4/*500*/, 0.2f), OO_AREA_LRTB);
        draws[i]->ovlPushBack(new OTextTraced("Press Me", CR_RELATIVE, 0.5f, 0.1f, 12, true), OO_AREA_LRBT);
      }
      else if (MW_TEST == DRAW_BRIGHT && i == 0)
      {
        Ovldraw* oimg = new OImageStretched(new QImage(img_path_normal), OVLQImage::IC_BLOCKALPHA);
        oimg->setSlice(0.35);
        oimg->setOpacity(0.15);
        draws[i]->ovlPushBack(oimg);
      }
      else if (MW_TEST == DEBUG_VOCAB)
      {
        Ovldraw* oimg = new OImageStretched(new QImage(img_path_normal), OVLQImage::IC_ASIS);
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
        Ovldraw* ovl = new OImageStretched(&img, OVLQImage::IC_AUTO);
        ovl->setSlice(i != 1? 0.25f : 0.0f);
        draws[i]->ovlPushBack(ovl, OO_AREA_LRBT);
        draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_HORZ, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
        draws[i]->ovlPushBack(new OGridRegular(OGridRegular::REGULAR_VERT, CR_RELATIVE, 0.05, 0.05, linestyle_greydark(5,1,0),-1));
        draws[i]->ovlPushBack(new OGridDecart(CR_RELATIVE, 0.05, 0.5, 0.1, 0.1, 3));
      }
  }

  {
//    const unsigned int btnMicroWidth = 44;
//    const unsigned int btnMinWidth = 64, btnMaxWidth = 100;
//    const unsigned int edMinWidth = 48, edMaxWidth = 48;
    const unsigned int sbUniWidth = 48;
    
    QBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    
    QSignalMapper*  featsMapper = new QSignalMapper(this);
    
    BS_INIT_FOR(mainLayout)
    BS_START_SCROLL_V_HMAX_VMIN
    {
#define NEW_SPIN_ADDMAPPED(var, slot, ...)  { \
                              QSpinBox*   _sb = new QSpinBox(); \
                              _sb->setRange(0, 99); \
                              _sb->setProperty("itarget", var); \
                              _sb->setMaximumWidth(80); \
                              BSADD(_sb, ##__VA_ARGS__); \
                              QObject::connect(_sb, SIGNAL(valueChanged(int)), this, slot); \
                            }
#define NEW_DOUBLESPIN_ADDMAPPED(var, slot, ...)  { \
                              QDoubleSpinBox*   _sb = new QDoubleSpinBox(); \
                              _sb->setRange(0, 10); \
                              _sb->setSingleStep(0.1); \
                              _sb->setProperty("itarget", var); \
                              _sb->setMaximumWidth(80); \
                              BSADD(_sb, ##__VA_ARGS__); \
                              QObject::connect(_sb, SIGNAL(valueChanged(double)), this, SLOT(slot(double))); \
                            }
      menucount = 0;
      const int maincaption = 16;
      _bs_topScroll->setMinimumWidth(536);
      
#define COLLAPSOR_BEGIN(caption) \
      BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout) \
        menuBtns[menucount] = new QCheckBox(); \
        menuBtns[menucount]->setChecked(menucount == 1); \
        BSADD(menuBtns[menucount]) \
        { \
          QPushButton*  btn = new QPushButton(tr(caption)); \
          btn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred); \
          btn->setFlat(true); \
          QObject::connect(btn, SIGNAL(clicked()), menuBtns[menucount], SLOT(click())); \
          BSADD(btn,1); \
        } \
      BS_STOP \
      BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_STYLED, 1) \
        BSWIDGET->setContentsMargins(16, 8, 8, 12); \
        BSWIDGET->setMinimumWidth(500); \
        menuWdgs[menucount] = BSWIDGET;
      
#define COLLAPSOR_END \
        BSWIDGET->setVisible(menucount == 1); \
        QObject::connect(menuBtns[menucount], SIGNAL(clicked(bool)), menuWdgs[menucount], SLOT(setVisible(bool))); \
        menucount++; \
      BS_STOP

      {
        COLLAPSOR_BEGIN("Code snipped")
          QString result;
          QFile f(":/mainwindow.cpp");
          if (f.open(QFile::ReadOnly))
          {
            char buffer[1024];
            qint64 len; 
            int onmyway = 0;
            int ctr=0;
            while ((len = f.readLine(buffer, 1024)) != -1)
            {
              QString line(buffer);
              if (onmyway == 0)
              {
                if (line.contains(g_testnames[MW_TEST]))
                  onmyway = 1;
              }
              else if (onmyway == 1)
              {
                if (line.contains("{"))
                  onmyway = 2;
                else
                  onmyway = 5;
              }
              else if (onmyway == 2)
              {
                if (line.contains("AFTERCREATE") == false)
                {
                  if (LINES == 1 && line.contains("LINES ="))
                    continue;
                  result += line;
                  if (ctr++ > 500) 
                    onmyway = 4;
                }
                else
                {
                  if (line.contains("NOSNIPPET"))
                  {
                    onmyway = 5;
                    continue;
                  }                  
                  onmyway = 3;
                  if (result.endsWith("\n") == false)
                    result += "\n";
                  if (line.contains("DRAW1C"))
                    result += "    ""for (int i=0; i<countDraws; i++)\n"
                              "    ""  this->layout()->addWidget(pdraws[i]);\n";
                  else if (line.contains("DRAW2D"))
                    result += "    ""for (int c=0; c<countCOLUMN; c++)\n"
                              "    ""{\n"
                              "    ""  QVBoxLayout* layV = new QVBoxLayout;\n"
                              "    ""  for (int r=0; r<countROW; r++)\n"
                              "    ""    layV->addWidget(pdraws[r][c]);\n"
                              "    ""  currentHBoxLayout->addLayout(layV);\n"
                              "    ""}\n";
                  else if (line.contains("DRAWBARS1C"))
                    result += "    ""for (int i=0; i<countDraws; i++)\n"
                              "    ""  this->layout()->addWidget(pdrawbars[i]);\n";
                  else if (line.contains("DRAWBARS1R"))
                    result += "    ""for (int i=0; i<countDraws; i++)\n"
                              "    ""  currentHBoxLayout->addWidget(pdrawbars[i]);\n";
                  else if (line.contains("DRAWBARS1"))
                    result += "    ""this->layout()->addWidget(pdrawbars);\n";
                  
                  result += "\n";
                  
                  result += "    ""const float* data = someGeneratedData; // at least ";
                  if (LINES > 1)
                    result += "[LINES x SAMPLES x PORTIONS] of floats\n";
                  else
                    result += "[SAMPLES x PORTIONS] of floats\n";
                  if (drcount == 1 && dccount == 1)
                  {
                    result += "    ""pdraw->setData(data);\n";
                  }
                  else if (drcount == 1 || dccount == 1)
                  {
                    result += "    ""for (int i=0; i<countDraws; i++)\n"
                              "    ""  pdraws[i]->setData(data);\n";
                  }
                  else
                  {
                    result += "    ""for (int c=0; c<countCOLUMN; c++)\n"
                              "    ""  for (int r=0; r<countROW; r++)\n"
                              "    ""    pdraws[r][c]->setData(data);\n";
                  }
                  
                  result += "\n";
                }
              }
              else if (onmyway >= 3)
                break;
            }
            f.close();
            if (onmyway != 3)
              result = "Unable to get code snippet.. Im sorry :(";
          }
          else
            result = "Unable to open resource file with code :(";
          QTextEdit*  qte = new QTextEdit();
          qte->setMinimumHeight(500);
          qte->setReadOnly(true);
          qte->setLineWrapMode(QTextEdit::NoWrap);
          qte->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
          qte->setText(result);
          BSADD(qte);
        COLLAPSOR_END
        
        
        COLLAPSOR_BEGIN("Data Generation")
          TAU_TEXT_ADD(0, "setData()", 0, Qt::AlignLeft);
          BS_SPACING(maincaption)
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            int bfsmask = BFS_CHECKABLE | (MW_TEST == DRAW_BRIGHT_CLUSTER || MW_TEST == DEBUG_PALETTE2D? BFS_DISABLED : 0);
            TauSetup sigs[] = { 
              TauSetup("Random",  0, ST_RAND, bfsmask),
              TauSetup("Normal", 0, ST_GEN_NORM, bfsmask),
              TauSetup("Peaks",   0, ST_PEAK,  bfsmask),
              TauSetup("Peaks2",   0, ST_PEAK2,  bfsmask),
              TauSetup("Peaks3",   0, ST_PEAK3,  bfsmask),
              TauSetup("Move",    0, ST_MOVE, bfsmask),
              
              TauSetup("SinX",   0, ST_SIN,  bfsmask),
              TauSetup("MSins", 0, ST_MANYSIN,  bfsmask),
              TauSetup("Hiperb",   0, ST_HIPERB, bfsmask),
              TauSetup("SinX/X",   0, ST_SINXX, bfsmask),
              TauSetup("X*X",   0, ST_XX, bfsmask),
              TauSetup("tanhX",   0, ST_TANHX, bfsmask),
              
              TauSetup("--0--",   0, ST_ZERO, bfsmask),
              TauSetup("--1--",   0, ST_ONE,  bfsmask),
              TauSetup("1-0-1",   0, ST_ZOZ,  bfsmask),
              TauSetup("1-0--",   0, ST_ZOO,  bfsmask),
              TauSetup("--0-1",   0, ST_OOZ,  bfsmask),
              TauSetup("101010", 0, ST_OZOZO, bfsmask),
          
              TauSetup("Ramp",   0, ST_RAMP, bfsmask),
              TauSetup("Step", 0, ST_STEP, bfsmask),
              TauSetup("10",   0, ST_10, bfsmask),
              TauSetup("100",   0, ST_100,  bfsmask),
              TauSetup("1000",   0, ST_1000,  bfsmask),
              TauSetup("10000",   0, ST_10000,  bfsmask),
            };
            for (unsigned int i=0; i<sizeof(sigs)/sizeof(TauSetup); i++)
              if (sigs[i].mappedvalue == sigtype)
                sigs[i].flags |= BFS_CHECKED;
            QButtonGroup* qbg = new QButtonGroup(this);
            qbg->setExclusive(true);
            
            const unsigned int vsigs = 6;
            BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
              for (unsigned int i=0; i<vsigs; i++)
                TAU_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
            BS_STOP
            BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
              for (unsigned int i=vsigs; i<vsigs*2; i++)
                TAU_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
            BS_STOP
            BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
              for (unsigned int i=vsigs*2; i<vsigs*3; i++)
                TAU_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
            BS_STOP
            BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
              for (unsigned int i=vsigs*3; i<sizeof(sigs)/sizeof(TauSetup); i++)
                TAU_BTN_ADDGROUPED(sigs[i], qbg, 0, Qt::AlignCenter);
            BS_STOP
#if QT_VERSION >= 0x060000
            QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeSigtype);
#else
            QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeSigtype(int)));
#endif
          BS_STOP
        
          BS_SPACING(8)
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TAU_TEXT_ADD(0, "scaling data by [k*x+b]:", 0, Qt::AlignHCenter);
            QSignalMapper*  edMapper = new QSignalMapper(this);
            for (unsigned int i=0; i<2; i++)
            {
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                TAU_TEXT_ADD(0, i == 0 ? "k =": "b =");
                QLineEdit* _ed = new QLineEdit(QString::number(i == 0? sig_k:sig_b, 'f', 1));
                _ed->setMaximumWidth(_BS_FIELD_WIDTH_MAX_SYMBOL);
                _ed->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                edMapper->setMapping(_ed, i == 0? ED_SIGK : ED_SIGB);
                connect(_ed, SIGNAL(editingFinished()), edMapper, SLOT(map()));
                BSADD(_ed, 0, Qt::AlignRight);
              BS_STOP
            }
#if QT_VERSION >= 0x060000
            QObject::connect(edMapper, &QSignalMapper::mappedInt, this, &MainWindow::changeFloats);
#else
            QObject::connect(edMapper, SIGNAL(mapped(int)), this, SLOT(changeFloats(int)));
#endif
            BS_STRETCH
          BS_STOP
          
          BS_SPACING(8)
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            TAU_TEXT_ADD(0, "clearData()", 0, Qt::AlignLeft);
            BS_STRETCH
            TAU_BTN_ADDMAPPED(TauSetup("Clear Data", 0, BTF_CLEAR, 0), featsMapper, 0, Qt::AlignCenter);
            BS_STRETCH
          BS_STOP
          BS_SPACING(8) 
            
//            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 0)

          TAU_TEXT_ADD(0, "Generation speed", 0, Qt::AlignLeft);
          
          TauSetup rtsbtns[] = { 
            TauSetup("1", 0, SP_ONCE, 0 | BFS_WIDTHMAX_SYMBOL),
            TauSetup(">",    0, SP_SLOWEST, BFS_CHECKABLE | BFS_WIDTHMAX_SYMBOL),
            TauSetup(">>",   0, SP_SLOW, BFS_CHECKABLE | BFS_WIDTHMAX_SYMBOL),
            TauSetup(">>>",  0, SP_FAST, BFS_CHECKABLE | BFS_WIDTHMAX_SYMBOL), 
            TauSetup(">>>>", 0, SP_FASTEST, BFS_CHECKABLE | BFS_WIDTHMAX_SYMBOL), 
            TauSetup("STOP", 0, SP_STOP, BFS_CHECKABLE | BFS_WIDTHMAX_SYMBOL),
          };
          const int rtsbtnscount = sizeof(rtsbtns) / sizeof(TauSetup);
          QButtonGroup* qbg = new QButtonGroup(this);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_SPACING(16)
            for (int i=0; i<rtsbtnscount; i++)
            {
              if (rtsbtns[i].mappedvalue == sp)
                rtsbtns[i].flags |= BFS_CHECKED;
              BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                TAU_BTN_ADDGROUPED(rtsbtns[i], qbg, 0, Qt::AlignCenter);
                int v = rtsbtns[i].mappedvalue;
                TAU_QSTR_ADD(0, i == 0 || i == rtsbtnscount - 1? QString("-") : v > 1000? QString("%1 sec").arg(v/1000) : QString("%1ms").arg(v), 0, Qt::AlignCenter);
              BS_STOP
              if (i == 0)
                BS_SPACING(12)
              if (i == rtsbtnscount - 2)
                BS_STRETCH
            }
            BS_SPACING(16)
          BS_STOP
                

#if QT_VERSION >= 0x060000
          QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeSpeedData);
#else
          QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeSpeedData(int)));
#endif
        COLLAPSOR_END
              
        COLLAPSOR_BEGIN("General methods")
          {
            TauSetup fseds[] = {
              TauSetup("1.0", 0, ED_HIGH, BFS_WIDTHMAX_SYMBOL),
              TauSetup("0.0", 0, ED_LOW, BFS_WIDTHMAX_SYMBOL),
              TauSetup("1.0", 0, ED_CONTRAST, BFS_WIDTHMAX_SYMBOL),
              TauSetup("0.0", 0, ED_OFFSET, BFS_WIDTHMAX_SYMBOL),
            };
            QSignalMapper*  edMapper = new QSignalMapper(this);
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                TAU_TEXT_ADD(0, "setBounds()  \t[linked with setContrast]", 0, Qt::AlignLeft);
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_STRETCH
                  for (int i=0; i<2; i++)
                    TAU_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                  BS_STRETCH    
                BS_STOP
                TAU_TEXT_ADD(0, "setContrast()  \t[linked with setBounds]", 0, Qt::AlignLeft);
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  BS_STRETCH
                  for (int i=2; i<4; i++)
                    TAU_EDIT_ADDMAPPED(fseds[i], edMapper, 0, Qt::AlignRight);
                  BS_STRETCH
                BS_STOP
              BS_STOP
              TAU_BTN_ADDMAPPED(TauSetup("reset", 0, ED_RESET, 0), edMapper, 0, Qt::AlignCenter);
            BS_STOP
#if QT_VERSION >= 0x060000
            QObject::connect(edMapper, &QSignalMapper::mappedInt, this, &MainWindow::changeFloats);
#else
            QObject::connect(edMapper, SIGNAL(mapped(int)), this, SLOT(changeFloats(int)));
#endif
          }
          TAU_TEXT_ADD(0, "setOrientation()", 0, Qt::AlignLeft);
          QButtonGroup* qbgOrient = new QButtonGroup(this);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TauSetup or0[] = { 
              TauSetup("OR_LRBT", 0, 0, 0),
              TauSetup("OR_RLBT", 0, 1, 0),
              TauSetup("OR_LRTB", 0, 2, 0),
              TauSetup("OR_RLTB", 0, 3, 0),
            };
            for (unsigned int i=0; i<sizeof(or0)/sizeof(TauSetup); i++)
              TAU_BTN_ADDGROUPED(or0[i], qbgOrient);
            BS_STRETCH
          BS_STOP
          
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TauSetup or1[] = {
              TauSetup("OR_TBLR", 0, 4, 0),
              TauSetup("OR_BTLR", 0, 5, 0),
              TauSetup("OR_TBRL", 0, 6, 0),
              TauSetup("OR_BTRL", 0, 7, 0),
            };
            for (unsigned int i=0; i<sizeof(or1)/sizeof(TauSetup); i++)
              TAU_BTN_ADDGROUPED(or1[i], qbgOrient);
            BS_STRETCH
          BS_STOP
#if QT_VERSION >= 0x060000
          QObject::connect(qbgOrient, &QButtonGroup::idPressed, this, &MainWindow::changeOrientation);
#else
          QObject::connect(qbgOrient, SIGNAL(buttonPressed(int)), this, SLOT(changeOrientation(int)));
#endif
          
          TAU_TEXT_ADD(0, "setScalingLimitsHorz()", 0, Qt::AlignLeft);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TAU_TEXT_ADD(0, "H.min:");
            NEW_SPIN_ADDMAPPED(SC_MIN_H, SLOT(changeScaling(int)), 0, Qt::AlignRight);
            TAU_TEXT_ADD(0, "H.max:");
            NEW_SPIN_ADDMAPPED(SC_MAX_H, SLOT(changeScaling(int)), 0, Qt::AlignRight);
            BS_STRETCH
          BS_STOP
                
          TAU_TEXT_ADD(0, "setScalingLimitsVert()", 0, Qt::AlignLeft);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TAU_TEXT_ADD(0, "V.min:");
            NEW_SPIN_ADDMAPPED(SC_MIN_V, SLOT(changeScaling(int)), 0, Qt::AlignRight);
            TAU_TEXT_ADD(0, "V.max:");
            NEW_SPIN_ADDMAPPED(SC_MAX_V, SLOT(changeScaling(int)), 0, Qt::AlignRight);
            BS_STRETCH
          BS_STOP
              
          
          TAU_TEXT_ADD(0, "setContentMargins()");
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
          {
            BS_STRETCH
            QSpinBox*   _sb = new QSpinBox();
            _sb->setMaximumWidth(80);
            BSADD(_sb, 0, Qt::AlignRight);
            QObject::connect(_sb, SIGNAL(valueChanged(int)), this, SLOT(changeMargins(int)));
            BS_STRETCH
          }
          BS_STOP
        COLLAPSOR_END
        
        COLLAPSOR_BEGIN("Palette")
          TAU_TEXT_ADD(0, "setDataPalette()", 0, Qt::AlignLeft);
          BS_SPACING(maincaption)
        
          TAU_TEXT_ADD(0, "bspalettes_std.h:", 0, Qt::AlignCenter);
          QSignalMapper*  palMapper = new QSignalMapper(this);
          const unsigned int btnsinrow = 4;
          unsigned int palscount = sizeof(ppalettes_loc_std)/sizeof(const IPalette*);
          unsigned int rows= palscount / btnsinrow + (palscount % btnsinrow ? 1 : 0);
          unsigned int palctr=0;
          
          const char*  stdPalNames[] = { "Bk-Wh",  "Gray", "Gn-Ye",   "Bu-Wh",  "Bk-Rd-Wh", "Bk-Bu-Wh", "Bk-Gn-Wh", "Bk-Gn-Ye-Wh", "SomeBlue", "Test2" };
          
          for (unsigned int i=0; i < rows && palctr < palscount; i++)
          {
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              for (unsigned int j=0; j<btnsinrow && palctr < palscount; j++)
              {
                TAU_BTN_ADDMAPPED(TauSetup(stdPalNames[palctr], 0, palctr, 0), palMapper);
                palctr++;
              }
            BS_STOP
          }
#if QT_VERSION >= 0x060000
          QObject::connect(palMapper, &QSignalMapper::mappedInt, this, &MainWindow::changePaletteSTD);
#else
          QObject::connect(palMapper, SIGNAL(mapped(int)), this, SLOT(changePaletteSTD(int)));
#endif
            
          BS_SPACING(8)
          TAU_TEXT_ADD(0, "bspalettes_adv.h:", 0, Qt::AlignCenter);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
          {
            BS_STRETCH
            QPaletteBox*  qcb = new QPaletteBox;
            QObject::connect(qcb, SIGNAL(currentIndexChanged(int)), this, SLOT(changePaletteADV(int)));
            BSADD(qcb);
            QCheckBox*  inv = new QCheckBox("Inverted");
            QObject::connect(inv, SIGNAL(clicked(bool)), qcb, SLOT(setInverted(bool)));
            BSADD(inv);
            BS_STRETCH
          }
          BS_STOP
          
          BS_SPACING(8)
          TAU_TEXT_ADD(0, "bspalettes_rgb.h:", 0, Qt::AlignCenter);
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
          {
            QSignalMapper*  palMapperRGB = new QSignalMapper(this);
            bool  on2=true, on3=true;
#if PORTIONS == 2
            on3 = false;
#elif PORTIONS == 3
            on2 = false;
#endif
            BS_STRETCH
            TauSetup bfs[] = { TauSetup("2 clr: ReGn", 0, 0, on2 == false? BFS_DISABLED : 0), 
                                   TauSetup("2 clr: ReBu", 0, 1, on2 == false? BFS_DISABLED : 0), 
                                   TauSetup("3 clr: RGB", 0, 2, on3 == false? BFS_DISABLED : 0), 
                                   TauSetup("3 clr: BGR", 0, 3, on3 == false? BFS_DISABLED : 0), 
                                 };
            for (unsigned int j=0; j<sizeof(bfs)/sizeof(TauSetup); j++)
              TAU_BTN_ADDMAPPED(bfs[j], palMapperRGB);
            BS_STRETCH
#if QT_VERSION >= 0x060000
            QObject::connect(palMapperRGB, &QSignalMapper::mappedInt, this, &MainWindow::changePaletteRGB);
#else
            QObject::connect(palMapperRGB, SIGNAL(mapped(int)), this, SLOT(changePaletteRGB(int)));
#endif
          }
          BS_STOP
              
          BS_SPACING(16)
          BS_START_LAYOUT_HMIN_VMIN(QHBoxLayout)
            BS_STRETCH
            TAU_CHECK(dsc, TauSetup("setDataPaletteDiscretion()", 0, 0, PORTIONS < 2? BFS_DISABLED : 0));
            BSADD(dsc)
            QObject::connect(dsc, SIGNAL(toggled(bool)), this, SLOT(changePaletteDiscretion(bool)));
            TAU_CHECK(itp, TauSetup("Data interpolation", 0, 0, 0));
            BSADD(itp)
            QObject::connect(itp, SIGNAL(toggled(bool)), this, SLOT(changeDataTextureInterpolation(bool)));
            BS_STRETCH
          BS_STOP
          BS_SPACING(8)
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            TAU_TEXT_ADD(0, "setDataPaletteRangeStart()  ");
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(0,100);
            slider->setValue(0);
            BSADD(slider, 1);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changePalrangeStart(int)));
          BS_STOP
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            TAU_TEXT_ADD(0, "setDataPaletteRangeStop()   ");
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(0,100);
            slider->setValue(100);
            BSADD(slider, 1);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changePalrangeStop(int)));
          BS_STOP
        COLLAPSOR_END

        COLLAPSOR_BEGIN("QWidget methods")
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
            TAU_TEXT_ADD(0, "Another type of draw refreshing:", 0, Qt::AlignLeft);
            TAU_TEXT_ADD(0, "Ban implicit update() calls", 0, Qt::AlignLeft);
            TAU_TEXT_ADD(0, "And update manually (explicit update() method).", 0, Qt::AlignLeft);
            TAU_TEXT_ADD(0, "Useful when You update whole screen by timeout", 0, Qt::AlignLeft);
            BS_SPACING(maincaption)
                
            BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
              BSMARGINS(16,0,0,0)
              TauSetup banbtns[] = { 
                TauSetup("banAutoUpdate(DrawCore::RD_BYDATA)",      0, BTF_BANUPDATEDATA, 0),
                TauSetup("banAutoUpdate(DrawCore::RD_BYSETTINGS)",   0, BTF_BANUPDATESETS, 0),
                TauSetup("banAutoUpdate(DrawCore::RD_BYOVL_ACTION)",  0, BTF_BANUPDATEOVERLAYS, 0),
              };
              const char* dopcaps[] = {     "implicit when Draw::setData or Draw::clearData calls", 
                                            "implicit when any Draw::set... method calls", 
                                            "implicit when overlay parameter updates"
                                      }
                                            ;

              for (unsigned int i=0; i<sizeof(banbtns) / sizeof(TauSetup); i++)
              {
                TAU_CHECK(_chb, banbtns[i]);
                _chb->setProperty("itarget", banbtns[i].mappedvalue);
                BSADD(_chb, 0, Qt::AlignLeft);
                TAU_TEXT_ADD(0, dopcaps[i], 0, Qt::AlignRight);
                QObject::connect(_chb, SIGNAL(clicked(bool)), this, SLOT(changeBans(bool)));
              }
            BS_STOP
            
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              TAU_TEXT_ADD(0, "Refresh widget:", 0, Qt::AlignLeft);
              BS_STRETCH
              TAU_BTN(_btn, TauSetup("Once",    0, SP_ONCE, 0));
              BSADD(_btn)
              QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(changeSpeedUpdate_Once()));
            BS_STOP
          
            QButtonGroup* qbg = new QButtonGroup(this);
            TauSetup upbtns[] = { 
              TauSetup(">",    0, SP_SLOWEST, BFS_CHECKABLE),
              TauSetup(">>",    0, SP_SLOW, BFS_CHECKABLE),
              TauSetup(">>>",  0, SP_FAST, BFS_CHECKABLE), 
              TauSetup(">>>>",    0, SP_FASTEST, BFS_CHECKABLE), 
              TauSetup("STOP",    0, SP_STOP, BFS_CHECKED),
            };
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              const unsigned int upbtnscount = sizeof(upbtns) / sizeof(TauSetup);
              for (unsigned int i=0; i<upbtnscount; i++)
              {
                BS_START_LAYOUT_HMAX_VMIN(QVBoxLayout)
                  TAU_BTN_ADDGROUPED(upbtns[i], qbg, 0, Qt::AlignCenter);
                  int v = upbtns[i].mappedvalue;
                  TAU_QSTR_ADD(0, i == upbtnscount - 1? QString("-") : v > 1000? QString("%1 sec").arg(v/1000) : QString("%1ms").arg(v), 0, Qt::AlignCenter);
                BS_STOP
              }
            BS_STOP
#if QT_VERSION >= 0x060000
            QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeSpeedUpdate);
#else
            QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeSpeedUpdate(int)));
#endif
          BS_STOP            
        COLLAPSOR_END
              
        COLLAPSOR_BEGIN("Overpattern [scaling+]")
          TAU_TEXT_ADD(0, "setOverpattern()   when Scaling > 1", 0, Qt::AlignLeft)
          BS_SPACING(maincaption)
              
          overpattern_t* dpm = new overpattern_t(overpattern_off());
          TAU_TEXT_ADD(0, "Over: ");
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TauSetup upbtns[] = { 
              TauSetup("Off",    0, 0, BFS_CHECKED),
              TauSetup(">  Thrs",    0, 1, BFS_CHECKABLE),
              TauSetup("<= Thrs",  0, 2, BFS_CHECKABLE), 
              TauSetup("Any",    0, 3, BFS_CHECKABLE), 
            };
            QButtonGroup* qbg = new QButtonGroup(this);
            for (int i=0; i<sizeof(upbtns)/sizeof(TauSetup); i++)
            {
              TAU_BTN_ADDGROUPED(upbtns[i], qbg)
            }
            qbg->setProperty("itarget", 0);
#if QT_VERSION >= 0x060000
            QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeOverpattern);
#else
            QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeOverpattern(int)));
#endif
            BS_STRETCH
          BS_STOP
                
          TAU_TEXT_ADD(0, "Threshold: ");
          NEW_DOUBLESPIN_ADDMAPPED(/*new BSUOD_DPM(1, dpm)*/1, changeOverpatternThreshold, 0, Qt::AlignCenter)
          TAU_TEXT_ADD(0, "Type: ");
          {
            QStringList dpmMain; dpmMain<<QString::fromUtf8("Contour")
                                     <<QString::fromUtf8("Line left")<<QString::fromUtf8("Line right")<<QString::fromUtf8("Line bottom")<<QString::fromUtf8("Line top")
                                     <<QString::fromUtf8("Line left-rigth")<<QString::fromUtf8("Line bottom-top")<<QString::fromUtf8("Lines left-bot")<<QString::fromUtf8("Lines right-bot")
                                     <<QString::fromUtf8("Lines left-top")<<QString::fromUtf8("Lines right-top")<<QString::fromUtf8("Grid")
                                     <<QString::fromUtf8("Dot")<<QString::fromUtf8("Dot left-bot")<<QString::fromUtf8("Dot contour")<<QString::fromUtf8("/")<<QString::fromUtf8("\\")
                                     <<QString::fromUtf8("Cross")<<QString::fromUtf8("Fill")<<QString::fromUtf8("Squares")
                                     <<QString::fromUtf8("Circle")<<QString::fromUtf8("Circle rev")<<QString::fromUtf8("Crosspuf")
                                     <<QString::fromUtf8("Rhomb")<<QString::fromUtf8("Suriken")<<QString::fromUtf8("Suriken rev")
                                     <<QString::fromUtf8("Donut")<<QString::fromUtf8("Cross2")<<QString::fromUtf8("Umbrella")
                                     <<QString::fromUtf8("Hourglass")<<QString::fromUtf8("Star")<<QString::fromUtf8("Bull")<<QString::fromUtf8("Bulr")
                                       ;
            QComboBox* qcb = new QComboBox;
            qcb->addItems(dpmMain);
//            qcb->setUserData(1, new BSUOD_DPM(1, dpm));
            qcb->setProperty("itarget", 1);
            QObject::connect(qcb, SIGNAL(currentIndexChanged(int)), this, SLOT(changeOverpattern(int)));
            BSADD(qcb, 0, Qt::AlignHCenter);
          }
            
          TAU_TEXT_ADD(0, "Integer settings: ")
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            TAU_TEXT_ADD(0, "Weight: ")
            QSpinBox* qcb3 = new QSpinBox;
            qcb3->setRange(-20, 20);
            qcb3->setMaximumWidth(80);
//            qcb3->setUserData(1, new BSUOD_DPM(2, dpm));
            qcb3->setProperty("itarget", 2);
            QObject::connect(qcb3, SIGNAL(valueChanged(int)), this, SLOT(changeOverpatternIntWeight(int)));
            BSADD(qcb3);
            BS_SPACING(32)
          BS_STOP
          TAU_TEXT_ADD(0, "Float settings: ")
          TAU_TEXT_ADD(0, "    Weight: ")
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_SPACING(32)
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(0,1000);
            slider->setValue(500);
            BSADD(slider, 1);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOverpatternFloatWeight(int)));
          BS_STOP
          TAU_TEXT_ADD(0, "    Smooth: ")
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_SPACING(32)
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(0,1000);
            slider->setValue(500);
            BSADD(slider, 1);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOverpatternFloatSmooth(int)));
          BS_STOP
              
          TAU_TEXT_ADD(0, "Palette color idx (0..20 -> 0.0..1.0): ");
          BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
            BS_STRETCH
            QSpinBox* qcb4 = new QSpinBox;
            qcb4->setRange(-20, 20);
            qcb4->setMaximumWidth(80);
//            qcb4->setUserData(1, new BSUOD_DPM(3, dpm));
            qcb4->setProperty("itarget", 3);
            QObject::connect(qcb4, SIGNAL(valueChanged(int)), this, SLOT(changeOverpattern(int)));
            BSADD(qcb4);
            BS_SPACING(32)
          BS_STOP
        COLLAPSOR_END
                
        COLLAPSOR_BEGIN("Impulse [scaling+]")
          TAU_TEXT_ADD(0, "setImpulse()   when Scaling > 1", 0, Qt::AlignLeft)
          BS_SPACING(maincaption)
                
            TauSetup upbtnoff("Off", 0, 0, BFS_CHECKED);
            TauSetup upbtns[] = { 
              TauSetup("coeff 1",    0, 0, BFS_CHECKABLE),
              TauSetup("coeff 2",  0, 0, BFS_CHECKABLE), 
              TauSetup("coeff 3",    0, 0, BFS_CHECKABLE),
              TauSetup("coeff 4",    0, 0, BFS_CHECKABLE),
              
              TauSetup("bordF 1",    0, 0, BFS_CHECKABLE),
              TauSetup("bordF 2",  0, 0, BFS_CHECKABLE), 
              TauSetup("bordF 3",    0, 0, BFS_CHECKABLE), 
              TauSetup("bordF 4",    0, 0, BFS_CHECKABLE), 
              
              TauSetup("bord 5",    0, 0, BFS_CHECKABLE),
              TauSetup("bord 6",  0, 0, BFS_CHECKABLE), 
              TauSetup("bord 7",    0, 0, BFS_CHECKABLE), 
              TauSetup("bord 8",    0, 0, BFS_CHECKABLE), 
            };
          
          QButtonGroup* qbg = new QButtonGroup(this);
          const char* caps[] = { "Direction A", "Direction B"};
          TAU_BTN_ADDGROUPED(upbtnoff, qbg, 0, Qt::AlignHCenter);
          for (int ab=0; ab<2; ab++)
          {
            TAU_TEXT_ADD(caps[ab], 0, Qt::AlignLeft)
            for (int r=0; r<3; r++)
            {
              BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                for (int c=0; c<4; c++)
                {
                  upbtns[r*4 + c].mappedvalue = 1 + ab*12 + r*4 + c;
                  TAU_BTN_ADDGROUPED(upbtns[r*4 + c], qbg);
                }
              BS_STOP
            }
          }
#if QT_VERSION >= 0x060000
          QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeImpulse);
#else
          QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeImpulse(int)));
#endif
        COLLAPSOR_END
        
        COLLAPSOR_BEGIN("Overlays")
          QTabBar*  ptb_ovl = new QTabBar;
          BSADD(ptb_ovl);
          BS_START_STACK_HMAX_VMIN
            QObject::connect(ptb_ovl, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
            
            ptb_ovl->addTab("Standard");
            BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
              QButtonGroup* ocrGroup = new QButtonGroup(this);
              ocrGroup->setExclusive(true);
              TauSetup fseds[] = { 
                TauSetup("OFF", 0, COS_OFF, BFS_CHECKED),
                TauSetup("Dekart", 0, COS_DEKART, BFS_CHECKABLE),
                TauSetup("Grids", 0, COS_GRIDS, BFS_CHECKABLE),
                TauSetup("Grids+Axes", 0, COS_GRIDSAXES, BFS_CHECKABLE),
                TauSetup("Circular", 0, COS_CIRCULAR, BFS_CHECKABLE),
                TauSetup("Drop Lines", 0, COS_DROPLINES, BFS_CHECKABLE),
                TauSetup("Brush", 0, COS_BRUSH, BFS_CHECKABLE),
                TauSetup("Cluster", 0, COS_CLUSTER, BFS_CHECKABLE),
                TauSetup("Followers", 0, COS_FOLLOWERS, BFS_CHECKABLE),
                TauSetup("Inside", 0, COS_INSIDE, BFS_CHECKABLE),
                TauSetup("CoverL", 0, COS_COVERL, BFS_CHECKABLE),
                TauSetup("CoverH", 0, COS_COVERH, BFS_CHECKABLE),
                TauSetup("Contour", 0, COS_CONTOUR, BFS_CHECKABLE),
                TauSetup("Selector", 0, COS_SELECTOR, BFS_CHECKABLE),
                TauSetup("Selector o", 0, COS_SELECTOR2, BFS_CHECKABLE),
                TauSetup("Objectif", 0, COS_OBJECTIF, BFS_CHECKABLE),
                TauSetup("Sprite\nalpha opaque", 0, COS_SPRITEALPHA, BFS_CHECKABLE),
                TauSetup("Foreground\nstretchable", 0, COS_FOREGROUND, BFS_CHECKABLE),
                TauSetup("Background\nstatic", 0, COS_BACKGROUND, BFS_CHECKABLE),
                TauSetup("Shadow rb", 0, COS_SHADOW1, BFS_CHECKABLE),
                TauSetup("Shadow full", 0, COS_SHADOW2, BFS_CHECKABLE),
                TauSetup("Shadow light", 0, COS_SHADOW3, BFS_CHECKABLE),
              };
              
              BSLAYOUT->setSpacing(2);
              BS_FORFOR_P(3, l, lim, sizeof(fseds)/sizeof(TauSetup)-1)
              {
                BS_START_LAYOUT_HMAX_VMAX(QHBoxLayout)
                  BSLAYOUT->setSpacing(2);
                  
                  for (int i=0; i < lim; i++)
                  {
//                        if (lim < 3 && i == 1)
//                          BS_STRETCH
                    TAU_BTN_ADDGROUPED(fseds[1+l+i], ocrGroup);
                  }
                BS_STOP   
              }
              TAU_BTN_ADDGROUPED(fseds[0], ocrGroup, 0, Qt::AlignHCenter);
#if QT_VERSION >= 0x060000
              QObject::connect(ocrGroup, &QButtonGroup::idPressed, this, &MainWindow::createOverlaySTD);
#else
              QObject::connect(ocrGroup, SIGNAL(buttonPressed(int)), this, SLOT(createOverlaySTD(int)));
#endif
              
              if (drawscount > 1)
              {
                TAU_CHECK(_chb, TauSetup("Sync interactive elements", 0, 0, BFS_CHECKED));
//                _chb->setUserData(0, new BSUOD_0(chbbs.mappedvalue));
                _chb->setProperty("itarget", 0);
                QObject::connect(_chb, SIGNAL(clicked(bool)), this, SLOT(setOverlaySync(bool)));
                BSADD(_chb);
              }
              
              BS_STRETCH
            BS_STOP
          
            ptb_ovl->addTab("Additional");
            BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 2)
              grForm = new QButtonGroup(this);
              BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                grForm->setExclusive(true);
                TauSetup fseds[] = { 
                  TauSetup("Circles", 0, 0, BFS_CHECKED),
                  TauSetup("Triangles", 0, 1, BFS_CHECKABLE),
                  TauSetup("Squares", 0, 2, BFS_CHECKABLE),
                  TauSetup("Xses", 0, 3, BFS_CHECKABLE),
                  TauSetup("Points", 0, 4, BFS_CHECKABLE),
                  
                };
                for (unsigned int i=0; i<sizeof(fseds) / sizeof(TauSetup); i++)
                  TAU_BTN_ADDGROUPED(fseds[i], grForm);
              BS_STOP
              
              grCount = new QButtonGroup(this);
              BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                grCount->setExclusive(true);
                TauSetup fseds[] = { 
                  TauSetup("x3", 0, 0, BFS_CHECKED),
                  TauSetup("1", 0, 1, BFS_CHECKABLE),
                  TauSetup("5", 0, 2, BFS_CHECKABLE),
                  TauSetup("10", 0, 3, BFS_CHECKABLE),
                };
                for (unsigned int i=0; i<sizeof(fseds) / sizeof(TauSetup); i++)
                  TAU_BTN_ADDGROUPED(fseds[i], grCount);
              BS_STOP
                  
              BS_START_FRAME_V_HMIN_VMAX(BS_FRAME_SUNKEN, 1)
                TAU_BTN(_btn, TauSetup("ADD",  0, 0, 0));
//                _btn->setUserData(0, new BSUOD_2(grForm));
//                _btn->setUserData(1, new BSUOD_2(grCount));
                BSADD(_btn, 0, Qt::AlignCenter);
                QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(createOverlayADD()));
              BS_STOP
            BS_STOP
          
          BS_STOP
          BS_STRETCH
        COLLAPSOR_END
              
        COLLAPSOR_BEGIN("Overlay Settings")
            QSignalMapper*  ovlMapper = new QSignalMapper(this);
            BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
              QButtonGroup* qbg = new QButtonGroup(this);
              qbg->setExclusive(true);
              static const char* numi[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
              for (unsigned int i=0; i<10; i++)
              {
                TAU_BTN_ADDGROUPED(TauSetup(numi[i+1], 0, i+1, (i == 0? BFS_CHECKED : BFS_CHECKABLE) | BFS_WIDTHMAX_ICON), qbg, 0, Qt::AlignCenter);
              }
              active_ovl = 1;
#if QT_VERSION >= 0x060000
              QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeOVL);
#else
              QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeOVL(int)));
#endif
            BS_STOP
            
            QTabBar*  ptb_ovl = new QTabBar;
            BSADD(ptb_ovl);
            BS_START_STACK_HMAX_VMIN
              QObject::connect(ptb_ovl, SIGNAL(currentChanged(int)), (QStackedLayout*)_bs_active, SLOT(setCurrentIndex(int)));
            
              ptb_ovl->addTab("Form");
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
                  {
                    TAU_TEXT_ADD(0, "Thick:");
                    QSpinBox*   psb = new QSpinBox();
                    psb->setRange(0, 10);
                    psb->setMaximumWidth(80);
                    BSADD(psb)
                    QObject::connect(psb, SIGNAL(valueChanged(int)), this, SLOT(changeOVLWeight(int)));
                  }

                  BS_STRETCH
                  
                  TauSetup spbs[] = { TauSetup("Stroke:", 0, 0, 0),
                                      TauSetup("Space:", 0, 1, 0),
                                      TauSetup("Dot:", 0, 2, 0),
                                        };
                  for (unsigned int i=0; i<sizeof(spbs)/sizeof(TauSetup); i++)
                  {
                    TAU_TEXT_ADD(0, spbs[i].pcsztext, 0, Qt::AlignRight);
                    QSpinBox*   psb = new QSpinBox();
                    psb->setMaximumWidth(100);
                    psb->setRange(0, 100);
                    psb->setProperty("itarget", spbs[i].mappedvalue);
                    psb->setAccelerated(true);
                    BSADD(psb, 0, Qt::AlignLeft)
                    QObject::connect(psb, SIGNAL(valueChanged(int)), this, SLOT(changeOVLForm(int)));
                    BS_SPACING(8)
                  }
                BS_STOP
                
                BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    TAU_TEXT_ADD(0, "Opacity:");
                    QSlider* slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0,100);
                    BSADD(slider);
                    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOVLOpacity(int)));
                  BS_STOP
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    TAU_TEXT_ADD(0, "Slice LL:");
                    QSlider* slider = new QSlider(Qt::Horizontal);
                    slider->setRange(-10, 100);
                    slider->setValue(0);
                    BSADD(slider);
                    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOVLSliceLL(int)));
                  BS_STOP
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    TAU_TEXT_ADD(0, "Slice HL:");
                    QSlider* slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, 110);
                    slider->setValue(100);
                    BSADD(slider);
                    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeOVLSliceHL(int)));
                  BS_STOP
                BS_STOP
              BS_STOP
                    
              ptb_ovl->addTab(tr("Color"));
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                TAU_BTN_ADDMAPPED(TauSetup("Change Color", 0, BTO_COLOR, 0), ovlMapper, 0, Qt::AlignHCenter);
              
                BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
                  QButtonGroup* qbg = new QButtonGroup(this);
                  qbg->setExclusive(true);
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    
                    TAU_BTN_ADDGROUPED(TauSetup("Noinvert", 0, BTO_NOINV, BFS_CHECKED), qbg);
                    TAU_BTN_ADDGROUPED(TauSetup("Invert1", 0, BTO_INV1, BFS_CHECKABLE/2), qbg);
                    TAU_BTN_ADDGROUPED(TauSetup("Invert2", 0, BTO_INV2, BFS_CHECKABLE/2), qbg);
                  BS_STOP
                  BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                    
                    TAU_BTN_ADDGROUPED(TauSetup("Invert3", 0, BTO_INV3, BFS_CHECKABLE/2), qbg);
                    TAU_BTN_ADDGROUPED(TauSetup("Invert4", 0, BTO_INV4, BFS_CHECKABLE/2), qbg);
                    TAU_BTN_ADDGROUPED(TauSetup("Invert5", 0, BTO_INV5, BFS_CHECKABLE/2), qbg);
                  BS_STOP
#if QT_VERSION >= 0x060000
                  QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeFeatures);
#else
                  QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeOVLFeatures(int)));
#endif
                BS_STOP
//                    BS_STRETCH
              BS_STOP
                      
              ptb_ovl->addTab(tr("Position"));
              BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 2)
                QSignalMapper*  ovlposMapper = new QSignalMapper(this);
                TAU_BTN_ADDMAPPED(TauSetup("+1", 0, BTOP_UP, BFS_WIDTHMAX_SHORT), ovlposMapper, 0, Qt::AlignHCenter);
                BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
                  TAU_BTN_ADDMAPPED(TauSetup("-1", 0, BTOP_LEFT, BFS_WIDTHMAX_SHORT), ovlposMapper);
                  TAU_BTN_ADDMAPPED(TauSetup("+1", 0, BTOP_RIGHT, BFS_WIDTHMAX_SHORT), ovlposMapper);
                BS_STOP
                TAU_BTN_ADDMAPPED(TauSetup("-1", 0, BTOP_DOWN, BFS_WIDTHMAX_SHORT), ovlposMapper, 0, Qt::AlignHCenter);
#if QT_VERSION >= 0x060000
                QObject::connect(ovlposMapper, &QSignalMapper::mappedInt, this, &MainWindow::changeOVLPos);
#else
                QObject::connect(ovlposMapper, SIGNAL(mapped(int)), this, SLOT(changeOVLPos(int)));
#endif
                BS_STRETCH
              BS_STOP
              
              QString ftabs[] = { tr("Replace"), tr("Create") };
              for (unsigned int t=0; t<sizeof(ftabs)/sizeof(QString); t++)
              {
                ptb_ovl->addTab(ftabs[t]);
                BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 2)
                  TauSetup forms1[] = { 
                    TauSetup("Circle",  0, BTV_CIRCLE,  0),
                    TauSetup("Square",    0, BTV_SQUARE,  0),
                    TauSetup("Line horz", 0, BTV_LINEHORZ,   0),
                    TauSetup("Line vert",  0, BTV_LINEVERT,    0),
                  };
                  TauSetup forms2[] = { 
                    TauSetup("Visir", 0, BTV_FACTOR,    0),
                    TauSetup("Cross", 0, BTV_CROSS,    0),
                    TauSetup("Text", 0, BTV_TEXT,    0),
                    TauSetup("Border", 0, BTV_BORDER,    0),
                  };
                  TauSetup forms3[] = { 
                    TauSetup("Remove", 0, BTV_REMOVE,    0),
                  };
                  QButtonGroup* qbg = new QButtonGroup(this);
                  qbg->setExclusive(true);
                  BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
                    for (unsigned int i=0; i<sizeof(forms1)/sizeof(TauSetup); i++)
                      TAU_BTN_ADDGROUPED(forms1[i], qbg, 0, Qt::AlignCenter);
                  BS_STOP
                  BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
                    for (unsigned int i=0; i<sizeof(forms2)/sizeof(TauSetup); i++)
                      TAU_BTN_ADDGROUPED(forms2[i], qbg, 0, Qt::AlignCenter);
                  BS_STOP
                      
                  if (t == 0)
                  {
                    BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_BOX, 1)
//                        for (unsigned int i=0; i<sizeof(forms3)/sizeof(TauSetup); i++)
                      TAU_BTN_ADDGROUPED(forms3[0], qbg, 0, Qt::AlignHCenter);
                    BS_STOP
                  }
                  
                  if (t == 0)
                  {
#if QT_VERSION >= 0x060000
                    QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::metaOVLReplace);
#else
                    QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(metaOVLReplace(int)));
#endif
                  }
                  else if (t == 1)
                  {
#if QT_VERSION >= 0x060000
                    QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::metaOVLCreate);
#else
                    QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(metaOVLCreate(int)));
#endif
                  }
                  
                BS_STOP
              }
            BS_STOP
            
            TAU_TEXT_ADD(0, "(settings are not saved between overlays there)", 0, Qt::AlignHCenter);

          BS_STRETCH
#if QT_VERSION >= 0x060000
          QObject::connect(ovlMapper, &QSignalMapper::mappedInt, this, &MainWindow::changeOVLFeatures);
#else
          QObject::connect(ovlMapper, SIGNAL(mapped(int)), this, SLOT(changeOVLFeatures(int)));
#endif
        COLLAPSOR_END

        
        COLLAPSOR_BEGIN("Addit.")                
                
          BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
            TAU_TEXT_ADD(0, "Interpolation (future):", 0, Qt::AlignLeft);
            TauSetup interp[] = { 
              TauSetup("Off",  0, 0,  BFS_CHECKED),
              TauSetup("Static",0, 1,  BFS_CHECKABLE),
//                  TauSetup("Linear", 0, IT_DATALINE,  BFS_CHECKABLE),
//                  TauSetup("Nearest", 0, IT_NEAREST,  BFS_CHECKABLE),
            };
            QButtonGroup* qbg = new QButtonGroup(this);
            qbg->setExclusive(true);
            
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              
              for (unsigned int i=0; i<sizeof(interp)/sizeof(TauSetup); i++)
                TAU_BTN_ADDGROUPED(interp[i], qbg, 0, Qt::AlignCenter);
            BS_STOP
#if QT_VERSION >= 0x060000
            QObject::connect(qbg, &QButtonGroup::idPressed, this, &MainWindow::changeInterpolation);
#else
            QObject::connect(qbg, SIGNAL(buttonPressed(int)), this, SLOT(changeInterpolation(int)));
#endif
          BS_STOP
          
          BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
            {
              TAU_TEXT_ADD(0, "Pick Cluster in Overlay tab");
              TAU_BTN(_btn, TauSetup("Change Palette",  0, 0, 0));
              BSADD(_btn, 0, Qt::AlignCenter);
              QObject::connect(_btn, SIGNAL(clicked(bool)), this, SLOT(changeClusterPalette()));
            }
          BS_STOP
          
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
            TAU_BTN_ADDMAPPED(TauSetup("Remove last graph", 0, BTF_DESTROYGRAPH, 0), featsMapper, 0, Qt::AlignCenter);
          BS_STOP
          BS_START_FRAME_V_HMAX_VMIN(BS_FRAME_PANEL, 1)
            TAU_BTN_ADDMAPPED(TauSetup("Special Debug Button", 0, BTF_DEBUG, 0), featsMapper, 0, Qt::AlignCenter);
            TAU_BTN_ADDMAPPED(TauSetup("updateGeometry()", 0, BTF_UPDATEGEOMETRY, 0), featsMapper, 0, Qt::AlignLeft);
          BS_STOP              
          BS_STRETCH
        COLLAPSOR_END

        if (ovl_marks != -1)
        {
          COLLAPSOR_BEGIN("Marks")
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              TAU_TEXT_ADD(0, "State:");
              QSlider* slider = new QSlider(Qt::Horizontal);
              slider->setRange(0,100);
//              slider->setUserData(0, new BSUOD_0(MF_TCOLOR));
              slider->setProperty("itarget", MF_TCOLOR);
              BSADD(slider);
              QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeMarkData(int)));
            BS_STOP
            BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
              TAU_TEXT_ADD(0, "Position:");
              QSlider* slider = new QSlider(Qt::Horizontal);
              slider->setRange(0,100);
//              slider->setUserData(0, new BSUOD_0(MF_POS));
              slider->setProperty("itarget", MF_POS);
              BSADD(slider);
              QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeMarkData(int)));
            BS_STOP
          COLLAPSOR_END
        }
      }
            
      BS_STRETCH
    }
#if QT_VERSION >= 0x060000
    QObject::connect(featsMapper, &QSignalMapper::mappedInt, this, &MainWindow::changeFeatures);
#else
    QObject::connect(featsMapper, SIGNAL(mapped(int)), this, SLOT(changeFeatures(int)));
#endif
    BS_STOP

    BS_START_WIDGET_HMAX_VMAX(QVBoxLayout)
    {
      wdgArea = BSWIDGET;
      BS_START_FRAME_H_HMAX_VMIN(BS_FRAME_PANEL, 1)
      {
        BS_STRETCH
        TAU_QSTR_ADD(0, QString("%1. %2").arg(MW_TEST).arg(g_testnames[MW_TEST]));
        
        BS_STRETCH
#if QT_VERSION >= 0x050000
        TAU_BTN(btn, TauSetup("Screenshot",  0, 0, 0));
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(screenshot()));
        BSADD(btn)
#endif
      }
      BS_STOP
          
      if (MW_TEST == ADV_PALETTES)
      {
        BS_START_SCROLL_V_HMAX_VMAX
          for (unsigned int i=0; i<drawscount; i++)
          {
            TAU_QSTR_ADD(0, QString("ppalettes_adv[%1]:\t\t%2").arg(i).arg(ppalettenames_adv[i]));
            BSADD(draws[i]);
          }
        BS_STRETCH
        BS_STOP;
      }
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
            TAU_TEXT_ADD(0, "Change portions count:");
            BSADD(psb);
            for (int i=0; i<drawscount; i++)
              QObject::connect(psb, SIGNAL(valueChanged(int)), draws[i], SLOT(slot_setPortionsCount(int)));
            TAU_TEXT_ADD(0, " ; Note, your upper limit is fixed (cause of already allocated data)");
            BS_STRETCH
          BS_STOP
        BS_STOP
      }
      else if (MW_TEST == EXTRA_ORIENTS)
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
            DrawBars* pDB = new DrawBars(draws[i]);
            
            pDB->setContentsMargins(60,60,60,60);
            
            
            if (i == 0)
            {
              bool contourMaxZone = false;
              
              int otherFlags = /*DBF_ENUMERATE_FROMZERO | */DBF_ENUMERATE_SHOWLAST;
              
              int dockerFlags = DBF_DOCKTO_PREVMARK; 
              for (int j=0; j<4; j++)
              {
                ATTACHED_TO at = ATTACHED_TO(j);
                pDB->addContour(at, 0, contourMaxZone);
                pDB->addEScaleRollingOwnbounds(at, otherFlags | dockerFlags, 64 + 1, 30);
                pDB->addContour(at, 0, contourMaxZone);
                pDB->addScaleRollingOwnbounds(at, otherFlags | dockerFlags | DBF_NOTESINSIDE, 0.0f, 1.0f, SAMPLES);
  //              pDB->addContour(at, 0, contourMaxZone);
                
  //              pDB->addScaleDrawUniSide(at, dbmode | otherFlags | dockerFlags, 30);
                
                if (j == AT_LEFT)
                  pDB->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 0.0f, 3, 32);
              }
            }
            else if (i == 1)
            {
              ATTACHED_TO at1 = AT_LEFT, at2 = AT_RIGHT;
              pDB->addSpace(at1, 60);
              pDB->addPointerRelativeDrawbounds(at1, DBF_SHARED, 0.0f, 10);
              pDB->addPointerRelativeDrawbounds(at1, DBF_SHARED, 0.5f, 10);
              pDB->addPointerRelativeDrawbounds(at1, DBF_SHARED, 1.0f, 10);
              pDB->addSpace(at2, 60);
              pDB->addPointerRelativeOwnbounds(at2, DBF_SHARED, 0.0f, 0.0f, 1.0f, 20);
              pDB->addPointerRelativeOwnbounds(at2, DBF_SHARED, 0.5f, 0.0f, 1.0f, 20);
              pDB->addPointerRelativeOwnbounds(at2, DBF_SHARED, 1.0f, 0.0f, 1.0f, 20);
              
              pDB->addSpace(AT_TOP, 20);
              pDB->addSpace(AT_TOP, 60);
              pDB->addLabel(AT_TOP, DBF_SHARED | DBF_LABELAREA_FULLBAR, "Dynamic", Qt::AlignLeft);
              pDB->addLabel(AT_TOP, DBF_SHARED | DBF_LABELAREA_FULLBAR, "Static", Qt::AlignRight);
            }
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
              pDB->addEScalePixstepDrawbounds(AT_TOP, DBF_ENUMERATE_FROMZERO | DBF_ENUMERATE_SHOWLAST, 20);
            else if (i == 1)
              pDB->addScalePixstepOwnbounds(AT_TOP, 0, 0.0, SAMPLES-1, SAMPLES);
            else if (i == 2)
              pDB->addScaleRollingTapNM(AT_TOP, 0, standard_tap_symbolate<-1>, 4, nullptr, SAMPLES, 20);
            else
              pDB->addScaleSymmetricOwnbounds(AT_TOP, DBF_NOTE_BORDERS_ONLY | DBF_NOTESINSIDE, 0.0, 1.0, SAMPLES, 10);
            MEWPointer* mpH = pDB->addEPointer01Auto(AT_BOTTOM, DBF_NOTESINSIDE | DBF_ENUMERATE_FROMZERO, 0.0f);
            MEWPointer* mpV = pDB->addPointerRelativeDrawbounds(AT_RIGHT, DBF_NOTESINSIDE, 0.0f);
            int oapH = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpH->createReactor()));
            pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_VERT_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapH);
            int oapV = pDB->getDraw()->ovlPushBack(new OActiveCursorCarrier(mpV->createReactor()));
            pDB->getDraw()->ovlPushBack(new OFLine(OFLine::LT_HORZ_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_stroks(1.0f,0.0f,0.0f)), oapV);
            if (i == drawscount - 1)
            {
              pDB->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 3, 32);
            }
            else
              pDB->addScaleNativeDrawbounds(AT_LEFT, 0, 0.1f, 1.0f, 21, 32);
            {
              pDB->addLabel(AT_LEFT, 0 | DBF_LABELAREA_FULLBAR, labels_clr[i][0], Qt::AlignCenter, Qt::Vertical);
              pDB->addLabel(AT_TOP, 0, labels_clr[i][1], Qt::AlignCenter, Qt::Horizontal);
            }
            
            BSADD(pDB)
          }
        BS_STOP
      }
      else if (MW_TEST == DRAW_SCALES_3)
      {
        BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
          int rounding[] = { DBF_MARKS_ROUNDING_ON, DBF_MARKS_ROUNDING_OFF, DBF_MARKS_ROUNDING_OFF_INC };
          for (unsigned int i=0; i<drawscount; i++)
          {
            DrawBars* pDB = new DrawBars(draws[i]);
            pDB->setContentsMargins(10, 10, 10, 10);
            pDB->addScalePixstepOwnbounds(AT_TOP, 0, 0.0, SAMPLES-1, SAMPLES);
            pDB->addScalePixstepDrawbounds(AT_LEFT, rounding[i], 11, 18);
            BSADD(pDB)
          }
        BS_STOP
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
      }
      else if (MW_TEST == DEBUG_MAINSCALE)
      {
        MEWScale* scs[32];
        DrawBars* pdbs[32];
        int scscount=0;
        int flags[] = { 0, 
                        DBF_PRECISION_INCREASE,
                        DBF_PRECISION_EXACT_0
                      };
        int eflags[] = {  0,
                          DBF_DOCKTO_NEXTMARK, 
                          DBF_DOCKTO_PREVMARK, 
                      };
        
        BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
          for (unsigned int i=0; i<drawscount; i++)
          {
            pdbs[i] = new DrawBars(draws[i]);
            int count = 33;
            int pixstep = 40;
            float ownboundLL = 0.0f;
            float ownboundHL = 14.0f;
            
            ATTACHED_TO ats[] = { AT_LEFT, AT_TOP };
            for (int a=0; a<sizeof(ats)/sizeof(ATTACHED_TO); a++)
            {
              scs[scscount++] = pdbs[i]->addScaleNativeOwnbounds(ats[a], flags[i], ownboundLL, ownboundHL, 0.025f, 0, count, pixstep, 4);
              scs[scscount++] = pdbs[i]->addScalePixstepOwnbounds(ats[a], flags[i], ownboundLL, ownboundHL, count, pixstep, 4);
              scs[scscount++] = pdbs[i]->addScaleSymmetricOwnbounds(ats[a], flags[i], ownboundLL, ownboundHL, count, pixstep, 4);
            }
            
            MEWPointer* mpH = pdbs[i]->addPointerRelativeOwnbounds(AT_TOP, DBF_PRECISION_MAXIMIZE, -1.0f, ownboundLL, ownboundHL, 5);
            OActiveCursorCarrier* ocarrier = new OActiveCursorCarrier(mpH->createReactor(), CR_RELATIVE, -1,-1);
            OFLine* pOvl = new OFLine(OFLine::LT_VERT_SYMMETRIC, CR_RELATIVE, 0,0, CR_RELATIVE, 0, -1, linestyle_white(5, 1, 0));
            int oap = pdbs[i]->getDraw()->ovlPushBack(ocarrier);
            pdbs[i]->getDraw()->ovlPushBack(pOvl, oap);
            
            pdbs[i]->addPointerRelativeDrawbounds(AT_BOTTOM, DBF_PRECISION_INCREASE, 0.25f, 4, true);
            
            BSADD(pdbs[i])
          }
        BS_STOP
        BS_START_LAYOUT_HMAX_VMIN(QHBoxLayout)
          BS_STRETCH
          QDoubleSpinBox* qdsblow = new QDoubleSpinBox();
          qdsblow->setPrefix("Low  ");
          qdsblow->setRange(-10000, 10000);
          qdsblow->setValue(0);
          BSADD(qdsblow)
          QDoubleSpinBox* qdsbhigh = new QDoubleSpinBox();
          qdsbhigh->setPrefix("High  ");
          qdsbhigh->setRange(-10000, 10000);
          qdsbhigh->setValue(17);
          BSADD(qdsbhigh)
#if 1
          for (int s=0; s<scscount; s++)
          {
            QObject::connect(qdsblow, SIGNAL(valueChanged(double)), scs[s], SLOT(setBoundLow(double)));
            QObject::connect(qdsbhigh, SIGNAL(valueChanged(double)), scs[s], SLOT(setBoundHigh(double)));
          }
#else
          for (unsigned int i=0; i<drawscount; i++)
          {
            QObject::connect(qdsblow, SIGNAL(valueChanged(double)), pdbs[i], SLOT(slot_setBoundLow_dbl(double)));
            QObject::connect(qdsbhigh, SIGNAL(valueChanged(double)), pdbs[i], SLOT(slot_setBoundHigh_dbl(double)));
          }
#endif
          BS_STRETCH
        BS_STOP
      }
      else
      {
        QScrollBar* qsb = MW_TEST == EXTRA_GRAPHS_MOVE? new QScrollBar(Qt::Horizontal) : nullptr;
        BS_START_FRAME_H_HMAX_VMAX(BS_FRAME_PANEL, 2)
          for (unsigned int i=0; i<dccount; i++)
          {
            if (MW_TEST == DRAW_RECORDER)
              qsb = new QScrollBar();
            
//            BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
            BS_START_LAYOUT_HMAX_VMAX(QVBoxLayout)
                
              if (MW_TEST == DEMO_4_portions)
                TAU_TEXT_ADD(0, i == 0? "Three portions, meshing colors" : "One portion", 0, Qt::AlignHCenter)
              else if (MW_TEST == DRAW_HISTOGRAMS)
                TAU_QSTR_ADD(0, QString::number(PORTIONS) + 
                                QString(" portions with different colors. See tab Addit. section Postmask for more settings"), 
                                0, Qt::AlignHCenter)
              
              for (unsigned int j=0; j<drcount; j++)
              {
                if (i*drcount + j >= drawscount)
                  break;
                
                int lwresult = 0;
                if (lw == LW_1) lwresult = 1;
                else if (lw == LW_01x)  lwresult = j == 0? 0 : 1;
                else if (lw == LW_1x0)  lwresult = j == drcount - 1? 0 : 1;
                else if (lw == LW_10x)  lwresult = j == 0? 1 : 0;
                else if (lw == LW_0x1)  lwresult = j == drcount - 1? 1 : 0;
    
                if (qsb)
                  draws[i*drcount + j]->connectScrollBar(qsb, false);
                
                if (drawbarsstage)
                {
                  BSADD(drawbars[i*drcount + j], lwresult);
                }
                else
                {
                  BSADD(draws[i*drcount + j], lwresult);
                }
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
        if (MW_TEST == EXTRA_GRAPHS_MOVE)
        {
          BSADD(qsb);
          TAU_TEXT_ADD(0, "\tGraph width: 300pts. New data: 5pts. History: 4*300pts.");
        }
      }
    }
    BS_STOP_EX(1)
    
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
  
  this->resize(1460, 720);
  
  {
#ifdef DECIMATION
    DSAMPLES = SAMPLES*5;
#else
    DSAMPLES = int(SAMPLES);
#endif
    testbuf2D = new float[PORTIONS*LINES*DSAMPLES];
    testbuf1D = new float[PORTIONS*DSAMPLES];
    
    RND = LINES > 1000? 100: 1000;
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

void MainWindow::resizeEvent(QResizeEvent* ev)
{
  qDebug()<<"Resizing to: "<<ev->size();
  QMainWindow::resizeEvent(ev);
}

void MainWindow::changeMenu()
{
  int curidx=-1, nextidx=-1;
  for (int i=0; i<menucount; i++)
  {
    if (menuWdgs[i]->isVisible())
      curidx = i;
    else if (menuBtns[i] == sender())
      nextidx = i;
  }
  
  if (curidx == nextidx)
    return;
  
  if (curidx >= 0)
  {
    menuWdgs[curidx]->setVisible(false);
  }
  if (nextidx >= 0)
  {
    menuWdgs[nextidx]->setVisible(true);
  }
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
static float my_tanhx(float x, float mov){ return 0.5f*tanh(x/(24*mov+2.0f)); }

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
  
    const int count = DSAMPLES*int(LINES)/3;
    
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
  else if (MW_TEST == DEBUG_GRAPHEX)
  {
    const int count = 4096;
    float data[count];
    for (unsigned int i=0; i<drawscount; i++)
    {
      int portionSize = draws[i]->portionSize();
      for (int s=0; s<portionSize; s++)
        data[s] = s/float(portionSize);
      draws[i]->setData(data);
    }
  }
  else if (MW_TEST == DEBUG_PALETTE2D)
  {
//    for (int p=0; p<PORTIONS; p++)
//      for (int j=0; j<LINES; j++)
//        for (int i=0; i<DSAMPLES; i++)
//        {
//          testbuf2D[p*DSAMPLES*LINES + j*DSAMPLES + i] = i/float(DSAMPLES-1) + j/float(LINES-1);
//        }
    
    for (int j=0; j<LINES; j++)
      for (int i=0; i<DSAMPLES; i++)
      {
        testbuf2D[0*DSAMPLES*LINES + j*DSAMPLES + i] = i/float(DSAMPLES-1);
      }
    for (int j=0; j<LINES; j++)
      for (int i=0; i<DSAMPLES; i++)
      {
        testbuf2D[1*DSAMPLES*LINES + j*DSAMPLES + i] = j/float(LINES-1);
      }
    
    for (unsigned int i=0; i<drawscount; i++)
    {
      draws[i]->setData(testbuf2D);
    }
  }
  else if (sigtype == ST_CUSTOM)
    ;
  else
  {
    
    float (*manual_fn)(float, float) = nullptr;
    
    #pragma omp parallel
    for (int pm=0; pm<PORTIONS*LINES; pm++)
    {
      int portion = pm / LINES;
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
        
        float pcoeff = 1.0f - float(portion)/(PORTIONS + 10);
        for (int i = 0; i < mover_size; ++i)
        {
          int iip = base + i; if (iip >= DSAMPLES) iip = iip - DSAMPLES;
          int iim = base - i; if (iim < 0) iim = DSAMPLES + iim;
          testbuf[iip] = pcoeff*mover[i];
          testbuf[iim] = pcoeff*mover[i];
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
        if (sigtype == ST_HIPERB)
          manual_fn = my_hiperb;
        else if (sigtype == ST_SINXX)
          manual_fn = my_sinxx;
        else if (sigtype == ST_XX)
          manual_fn = my_xx;
        else if (sigtype == ST_TANHX)
          manual_fn = my_tanhx;
        #pragma omp for
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
      
      if (pm % LINES == 0)
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
    } /// PORTIONS*LINES
    
  #ifndef DECIMATION
    for (unsigned int i=0; i<drawscount; i++)
      draws[i]->setData(draws[i]->directions()==2? (const float*)testbuf2D : (const float*)testbuf1D);
  #else
    for (unsigned int i=0; i<drawscount; i++)
    {
      DataDecimatorFunc ddf(DECIMATION, draws[i]->directions() == 1? DSAMPLES : DSAMPLES*LINES);
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
      OCluster* pMKFigures = (OCluster*)draws[i]->ovlGet(ovl_figures);
      if (pMKFigures)
      {
        int cf = pMKFigures->count();
        #pragma omp parallel for
        for (int j=0; j<cf; j++)
        {
          float x = randbuf[j%DSAMPLES], y = randbuf[(cf - j - 1)%DSAMPLES];
          generateGaussian(0.5, 0.2, &x, &y);
          pMKFigures->updateItem(j, x, y);
        }
        float nzm = randbuf[0];
        pMKFigures->updateItemZoom(rand()%cf, 1 + nzm);
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
  static std::array<float,PORTIONS*LINES*DSAMPLES> testbuf2D;
  static std::array<float,PORTIONS*DSAMPLES> testbuf1DP;
  std::uniform_real_distribution<> URD1(0.0f, 1.0f);
//#endif
  
  #pragma omp parallel
  {
    #pragma omp for
    for (int pm=0; pm<PORTIONS*LINES; pm++)
    {
      int d = pm / LINES;
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
      
      if (pm % LINES == 0)
        std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf1DP.begin() + testbuf1D.size()*d);
      
      std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf2D.begin() + testbuf1D.size()*pm);
//      for (int y=1; y<LINES; y++)
//        std::copy(testbuf1D.begin(), testbuf1D.end(), testbuf2D.begin() + d*DSAMPLES*LINES + testbuf1D.size()*y);
    } // for PORTIONS
  
    if (sigtype == ST_BORDERS)
    {
      #pragma omp for
      for (int d=0; d<PORTIONS; d++)
      {
        for(int y = 0; y < LINES; ++y)
          for(int x = 0; x < DSAMPLES; ++x)
          {
            testbuf2D[d*LINES*DSAMPLES + y*DSAMPLES + x] = x == 0 || y == 0 || x == DSAMPLES-1 || y == LINES - 1 ? 1 : 0;
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
      OCluster* pMKFigures = (OCluster*)draws[i]->ovlGet(ovl_figures);
      if (pMKFigures)
      {
        std::normal_distribution<float> ND(0.5, 0.1);
        for (unsigned int i=0; i<pMKFigures->countFigures(); i++)
        {
          pMKFigures->updateItem(i, ND(gen), ND(gen));
        }
        float nzm = ND(gen)*2;
        pMKFigures->updateItemZoom(rand()%pMKFigures->countFigures(), nzm*nzm*nzm);
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
  const unsigned int size2D = PORTIONS*LINES*DSAMPLES;
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
  }
}

void MainWindow::changeOrientation(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setOrientation(ORIENTATION(value));
}

void  MainWindow::changeSpeedData(int id)
{
  if (id == SP_ONCE)
    emit this->generateData();
  else
  {
    speedDataTimer->stop();
    if (id == SP_STOP || id == SP_ONCE) ;
    else speedDataTimer->start(id);
  }
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
          Ovldraw* povl = draws[i]->ovlGet(j + 1);
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
    else if (id == BTF_UPDATEGEOMETRY)
    {
      draws[i]->updateGeometry();
    }
  }
  if (id == BTF_DESTROYGRAPH)
  {
    if (drawscount > 0)
      delete draws[--drawscount];
  }
}

void MainWindow::changeOverpattern(int sigid)
{  
  int id = sender()->property("itarget").toInt();
  for (unsigned int i=0; i<drawscount; i++)
  {
    overpattern_t ovp = draws[i]->overpattern();
    switch (id)
    {
    case 0: ovp.algo = sigid; break;
    case 1:
    {
      if (sigid < _OP_TOTAL)
      {
        ovp.masktype = true;
        ovp.mask = sigid;
      }
      else
      {
        ovp.masktype = false;
        ovp.mask = sigid-_OP_TOTAL;
      }
      break;
    }
    case 3:
    {
      if (sigid < 0)
      {
        ovp.colorByPalette = false;
        int mygrey = -sigid*12;
        ovp.color = color3f(mygrey, mygrey, mygrey);
      }
      else
      {
        ovp.colorByPalette = true;
        ovp.color = color3f(sigid/20.0f,sigid/20.0f,sigid/20.0f);
      }
      break;
    }
    default: break;
    }
    draws[i]->setOverpattern(ovp);
  }
}

void MainWindow::changeOverpatternThreshold(double val)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    overpattern_t ovp = draws[i]->overpattern();
    ovp.threshold = val;
    draws[i]->setOverpattern(ovp);
  }
}

void MainWindow::changeOverpatternIntWeight(int val)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    overpattern_t ovp = draws[i]->overpattern();
    if (ovp.masktype)
    {
      ovp.weight = val;
      draws[i]->setOverpattern(ovp);
    }
  }
}

void MainWindow::changeOverpatternFloatWeight(int val)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    overpattern_t ovp = draws[i]->overpattern();
    if (ovp.masktype == false)
    {
      ovp.weight = val/1000.0f;
      draws[i]->setOverpattern(ovp);
    }
  }
}

void MainWindow::changeOverpatternFloatSmooth(int val)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    overpattern_t ovp = draws[i]->overpattern();
    if (ovp.masktype == false)
    {
      ovp.smooth = val/1000.0f;
      draws[i]->setOverpattern(ovp);
    }
  }
}

void MainWindow::changeInterpolation(int sigid)
{
  for (unsigned int i=0; i<drawscount; i++)
    draws[i]->setDataTextureInterpolation(sigid == 1);
}


static const impulsedata_t g_impulse_rec[2][12] = 
{  
  {
    { impulsedata_t::IR_A_COEFF, 3, 1, 1, { 0.1f, 0.8f, 0.1f } } ,
    { impulsedata_t::IR_A_COEFF, 3, 1, 1, { 0.28f, 0.44f, 0.28f } } ,
    { impulsedata_t::IR_A_COEFF, 5, 2, 1, { 0.05f, 0.15f, 0.6f, 0.15f, 0.05f } } ,
    { impulsedata_t::IR_A_COEFF, 5, 2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } } ,
    
    { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 3, 1, 1, {} } ,
    { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 3, 2, 1, {} } ,
    { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 3, 3, 1, {} } ,
    { impulsedata_t::IR_A_BORDERS_FIXEDCOUNT, 3, 4, 1, {} } ,

    { impulsedata_t::IR_A_BORDERS, 3, 1, 1, { 0.0f } } ,
    { impulsedata_t::IR_A_BORDERS, 3, 2, 1, { 0.2f } } ,
    { impulsedata_t::IR_A_BORDERS, 3, 6, 1, { 0.0f } } ,
    { impulsedata_t::IR_A_BORDERS, 3, 2, 1, { 0.4f } }
  },
  {
    { impulsedata_t::IR_B_COEFF, 3, 1, 1, { 0.1f, 0.8f, 0.1f } } ,
    { impulsedata_t::IR_B_COEFF, 3, 1, 1, { 0.28f, 0.44f, 0.28f } } ,
    { impulsedata_t::IR_B_COEFF, 5, 2, 1, { 0.05f, 0.15f, 0.6f, 0.15f, 0.05f } } ,
    { impulsedata_t::IR_B_COEFF, 5, 2, 1, { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f } } ,
    
    { impulsedata_t::IR_B_BORDERS_FIXEDCOUNT, 3, 1, 1, {} } ,
    { impulsedata_t::IR_B_BORDERS_FIXEDCOUNT, 3, 2, 1, {} } ,
    { impulsedata_t::IR_B_BORDERS_FIXEDCOUNT, 3, 3, 1, {} } ,
    { impulsedata_t::IR_B_BORDERS_FIXEDCOUNT, 3, 4, 1, {} } ,

    { impulsedata_t::IR_B_BORDERS, 3, 1, 1, { 0.0f } } ,
    { impulsedata_t::IR_B_BORDERS, 3, 2, 1, { 0.2f } } ,
    { impulsedata_t::IR_B_BORDERS, 3, 6, 1, { 0.0f } } ,
    { impulsedata_t::IR_B_BORDERS, 3, 2, 1, { 0.4f } }
          }
};

void MainWindow::changeImpulse(int v)
{
  if (v == 0)
  {
    const impulsedata_t impoff = { impulsedata_t::IR_OFF };
    for (unsigned int i=0; i<drawscount; i++)
      draws[i]->setImpulse(impoff);
  }
  else
  {
    for (unsigned int i=0; i<drawscount; i++)
      draws[i]->setImpulse(((const impulsedata_t*)g_impulse_rec)[v-1]);
  }
}

void MainWindow::changeBans(bool banned)
{
  int id = sender()->property("itarget").toInt();
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
      draws[i]->ovlPushBack(new OGridDecart(CR_RELATIVE, 0.5f, 0.5f, 0.05f, 0.05f, 3));
      
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
      draws[i]->ovlPushBack(new OFLine(OFLine::LT_CROSS, CR_RELATIVE, 0.0f, 0.0f, CR_XREL_YREL_NOSCALED_SCALED, 0.01, 0.1, linestyle_green(0,0,1)), ovl_visir);
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
      break;
    }
    case COS_DROPLINES:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new ODropLine(25, true, linestyle_white(5,2,0)));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Press left mouse button", CR_RELATIVE, 0.05, 0.05, 12, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_BRUSH:
    {
      if (!ovl_is_synced || i == 0)
        ovl_tmp = draws[i]->ovlPushBack(new OBrush(1000, linestyle_solid(1.0f,0,0)));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Draw with your new brush", CR_RELATIVE, 0.05, 0.05, 12, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_CLUSTER:
    {
      if (!ovl_is_synced || i == 0)
      {
        OCluster* pMKFigures = new OCluster(false, 500, CR_ABSOLUTE_NOSCALED, 4, &paletteBkWh, false);
        for (unsigned int j=0; j<pMKFigures->count(); j++)
        {
          pMKFigures->updateItemForm(j, (OCluster::FFORM)(1 + j % 6));
          pMKFigures->updateItemColor(j, rand()/float(RAND_MAX));
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
      draws[i]->ovlPushBack(new OTextTraced("Follow mouse", CR_ABSOLUTE, -50, -80, 12, false, linestyle_red(1,0,0)), ovl_visir);
      break;
    }
    case COS_INSIDE:
    {
      float opacity = 0.1f;
      for (int j=7; j>=0; j--)
      {
        Ovldraw* ovl = new OFSquareCC(opacity, CR_RELATIVE, 0.5f, 0.5f, CR_SAME, 0.04f*(j+1), linestyle_redlight(1,0,0));
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
      draws[i]->ovlPushBack(new OCover(0.7f, 1.0f, 2, OCover::COP_COVER));
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
                                          (Ovldraw*)new OSelector(linestyle_bluelight(13,2,2), 0.2f) :
                                          (Ovldraw*)new OSelectorCirc(linestyle_bluelight(13,2,2), 0.2f, true));
      else
        draws[i]->ovlPushBack(draws[0]->ovlGet(ovl_tmp));
      
      draws[i]->ovlPushBack(new OTextTraced("Press left mouse button and move", CR_RELATIVE, 0.05f, 0.05f, 11, true, linestyle_white(5,2,0)));
      break;
    }
    case COS_OBJECTIF:
    {
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
        draws[i]->ovlPushBack(new OImageOriginal(&img, OVLQImage::IC_AUTO, CR_PIXEL, -24, 0, 0.2f, 0.2f), ovl_visir);
      }
      break;
    }
    case COS_FOREGROUND:
    {
      QImage  img(img_path_normal);
      draws[i]->ovlPushBack(new OImageStretched(&img, OVLQImage::IC_BLOCKALPHA));
      break;
    }
    case COS_BACKGROUND:
    {
      QImage  img(img_path_normal);
      Ovldraw* ovl = new OImageOriginal(&img, OVLQImage::IC_AUTO, 1);
      ovl->setSlice(0.0f);
      draws[i]->ovlPushBack(ovl);
      break;
    }
    case COS_SHADOW1:
    {
      draws[i]->ovlPushBack(new OShadow(OBLINE_RIGHT | OBLINE_BOTTOM, 3), OO_INHERITED);
      break;
    }
    case COS_SHADOW2:
    {
      draws[i]->ovlPushBack(new OShadow(3, 3, 4, 4), OO_AREA_LRBT);
      break;
    }
    case COS_SHADOW3:
    {
      draws[i]->ovlPushBack(new OShadow(4, 4, 7, 7, 0.25f, color3f_white()), OO_AREA_LRBT);
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
  const unsigned int counts[] = { 3, 1, 5, 10,  1,1,1 };
  unsigned int count = counts[grCount->checkedId()];
  for (unsigned int i=0; i<count; i++)
  {
    float opacity = 0.2f;
    float centerX = rand() / float(RAND_MAX), centerY = rand()/float(RAND_MAX);
    float abssize = 20;
    linestyle_t kls = linestyle_green(1,0,0);
    
    Ovldraw* ovl = nullptr;
    switch (grForm->checkedId())
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
    Ovldraw* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setOpacity(op/100.0f);
  }
}

void MainWindow::changeOVLSliceLL(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    Ovldraw* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setSliceLL(op/100.0f);
  }
}

void MainWindow::changeOVLSliceHL(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    Ovldraw* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setSliceHL(op/100.0f);
  }
}

void MainWindow::changeOVLWeight(int op)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    Ovldraw* povl = draws[i]->ovlGet(active_ovl);
    if (povl)
      povl->setThickness(op);
  }
}

void MainWindow::changeOVLForm(int value)
{
  for (unsigned int i=0; i<drawscount; i++)
  {
    Ovldraw_ColorTraced* povl = dynamic_cast<Ovldraw_ColorTraced*>(draws[i]->ovlGet(active_ovl));
    if (povl)
    {
      linestyle_t kls = povl->getLineStyle();
//      switch (((BSUOD_0*)sender()->userData(0))->id)
      switch (sender()->property("itarget").toInt())
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
    for (unsigned int i=0; i<drawscount; i++)
    {
      Ovldraw_ColorTraced* povl = dynamic_cast<Ovldraw_ColorTraced*>(draws[i]->ovlGet(active_ovl));
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
      Ovldraw_ColorTraced* povl = dynamic_cast<Ovldraw_ColorTraced*>(draws[i]->ovlGet(active_ovl));
      if (povl)
      {
        linestyle_t kls = povl->getLineStyle();
        kls.inversive = id == BTO_NOINV? 0 : (id - BTO_NOINV);
        povl->setLineStyle(kls);
      }
    }
  }
}

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
        Ovldraw* ovl = draws[i]->ovlGet(active_ovl);
        OVLCoordsStatic* ocs = dynamic_cast<OVLCoordsStatic*>(ovl);
        if (ocs == nullptr)
        {
          qDebug()<<"Example: Overlay Replace dynamic_cast failure!";
          continue;
        }
        Ovldraw*  newOverlay = nullptr;
        switch (vistype)
        {
          case BTV_CIRCLE:  newOverlay = new OFCircle(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_SQUARE:  newOverlay = new OFSquareCC(false, ocs, 0.0, 0.0, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
          case BTV_FACTOR:  newOverlay = new OFFactor(ocs, 0.0, 0.0, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
          case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, ocs, 0.0, 0.0, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
          case BTV_TEXT: newOverlay = new OTextTraced("CREATED", ocs, 0.0, 0.0, 12, true, linestyle_solid(1,0,0)); break;
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
      Ovldraw*  newOverlay = nullptr;
      COORDINATION cr = CR_RELATIVE;
      switch (vistype)
      {
        case BTV_CIRCLE:  newOverlay = new OFCircle(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_SQUARE:  newOverlay = new OFSquareCC(false, cr, 0.5, 0.5, CR_ABSOLUTE, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEHORZ:  newOverlay = new OFLine(OFLine::LT_HORZ_BYLEFT, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_LINEVERT:  newOverlay = new OFLine(OFLine::LT_VERT_BYBOTTOM, cr, 0.5, 0.5, CR_ABSOLUTE, 10, 100, linestyle_green(1,0,0)); break;
        case BTV_FACTOR:  newOverlay = new OFFactor(cr, 0.5, 0.5, CR_ABSOLUTE_NOSCALED, 10, 30, linestyle_yellow(5,1,0)); break;
        case BTV_CROSS:  newOverlay = new OFLine(OFLine::LT_CROSS, cr, 0.5, 0.5, CR_ABSOLUTE, 10, -1, linestyle_green(1,0,0)); break;
        case BTV_TEXT: newOverlay = new OTextTraced("CREATED", CR_RELATIVE, 0.5, 0.5, 12, true, linestyle_solid(1,0,0)); break;
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
  SCALING to = (SCALING)(sender()->property("itarget").toInt());
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
//  int mf = ((BSUOD_0*)sender()->userData(0))->id;
  int mf = sender()->property("itarget").toInt();
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
#ifdef TESTTRASS
  trasspoint_t tline[15];
  for (int i=0; i<100; i++)
  {
    for (int j=0; j<15; j++)
    {
//          tline[j].intensity = j == 5? float(i % 2) : 1.0f;
      tline[j].intensity = j == 0? 0.1f : 0;
      tline[j].position = i / 100.0f;
      tline[j].halfstrob = 0.05f;
    }
    ovl->appendTrassline(tline);
  }
  return;
#endif
  
  
  if (ovl_figures != -1)
  {
    for (unsigned int i=0; i<drawscount; i++)
    {
      OCluster* pMKFigures = dynamic_cast<OCluster*>(draws[i]->ovlGet(ovl_figures));
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
      Ovldraw_ColorThroughPalette* povl= dynamic_cast<Ovldraw_ColorThroughPalette*>(draws[i]->ovlGet(1));
      if (povl)
      {
        povl->setPalette(ppalettes_adv[rand() % sizeof(ppalettes_adv)/sizeof(ppalettes_adv[0])], false);
      }
    }
  }
}

void MainWindow::screenshot()
{
#if QT_VERSION >= 0x050000
//  QPixmap ss = QApplication::primaryScreen()->grabWindow(this->winId());
  QPixmap ss = QApplication::primaryScreen()->grabWindow(wdgArea->winId());
  
  QString filename;
  filename += QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + QString("_") + QString(g_testnames[MW_TEST]).toLower() + ".png";
  {
    if (ss.save(filename, "png", 100))
      qDebug()<<"Screenshot saved successfully into "<<filename;
    else
      qDebug()<<"Unable to save screenshot:"<<filename;
  }
#else
  qDebug()<<"Qt < 5.0 not supporting QScreen. Unable to save screenshots";
#endif
}
