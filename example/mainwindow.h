#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#define REGTEST(A) A,
enum  tests_t {
#include "tests.h"
};
#undef REGTEST

class DrawQWidget;
class QTimer;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(tests_t testnumber, QWidget *parent = 0);
  ~MainWindow();
private:
  DrawQWidget**  draws;
  const int             MW_TEST;
  int                   SAMPLES, DSAMPLES;
  int                   MAXLINES;
  int                   PORTIONS;
  int                   RND;
  
  float*                testbuf2D;
  float*                testbuf1D;
  
  unsigned int          drawscount;
  float *               randomer;
  unsigned int          active_ovl;
  int                   ovl_visir, ovl_marks, ovl_figures, ovl_sprites;
  int                   ovl_active_mark;
  bool                  ovl_is_synced;
  enum    ED_DRAW     {  ED_UNKNOWN, ED_LOW, ED_HIGH, ED_CONTRAST, ED_OFFSET, ED_SIGB, ED_SIGK };

  QTimer*               speedDataTimer, *speedUpdateTimer;
  
  enum    BTN_FEATS     { BTF_CLEAR, BTF_INVHORZ, BTF_INVVERT, BTF_HIDEOTHERS, 
                          BTF_BANUPDATEDATA, BTF_BANUPDATESETS, BTF_BANUPDATEOVERLAYS, BTF_DESTROYGRAPH  };
  enum    BTN_OVLFEATS  { BTO_VISIBLE, BTO_COLOR, BTO_NOINV, 
                          BTO_INV1, BTO_INV2, BTO_INV3, BTO_INV4, BTO_INV5 };
  
  enum    BTN_VISIR     { BTV_CIRCLE, BTV_SQUARE, BTV_LINEHORZ, BTV_LINEVERT, 
                          BTV_FACTOR, BTV_CROSS, BTV_TEXT, BTV_BORDER, 
                          BTV_REMOVE };
  
  enum    MARK_FIELDS   { MF_TCOLOR, MF_POS, _MF_COUNT };
  
  enum    SCALING       { SC_MIN_H, SC_MIN_V, SC_MAX_H, SC_MAX_V};
  
  enum    BTN_OVLPOS    { BTOP_UP, BTOP_LEFT, BTOP_RIGHT, BTOP_DOWN };
private:
  enum    BTN_SIGTYPE   { ST_RAND, ST_SIN, ST_MANYSIN, ST_SINXX, ST_XX, ST_TANHX, 
                          ST_RAMP, ST_MOVE, ST_ZERO, ST_ONE, ST_ZOZ, ST_ZOO, ST_OOZ, 
                          ST_PEAK, ST_PEAK2, ST_PEAK3, ST_STEP, ST_OZOZO, ST_GEN_NORM,
                          ST_HIPERB, ST_10, ST_100, ST_1000, ST_10000 };
  int     sigtype;
  float   sig_k, sig_b;
private:
  enum    OSS           { COS_OFF, COS_DEKART, COS_GRIDS, COS_GRIDSAXES, COS_CIRCULAR, 
                          COS_SELECTOR, COS_DROPLINES, COS_CLUSTER, COS_FOLLOWERS, 
                          COS_INSIDE, COS_COVERL, COS_COVERH, COS_CONTOUR, 
                          COS_SPRITEALPHA, COS_FOREGROUND, COS_BACKGROUND };
private slots:
  void    changeMargins(int value);
  void    changePaletteSTD(int palid);
  void    changePaletteADV(int palid);
  void    changePaletteRGB(int palid);
  void    changeFloats(int edid);
  void    changeSpeedData(int sigid);
  void    changeSpeedData_Once();
  void    changeSpeedUpdate(int sigid);
  void    changeSpeedUpdate_Once();
  void    changeFeatures(int sigid);
  void    changeInterpolation(int sigid);
  void    changeBans(bool banned);
private slots:
  void    createOverlaySTD(int);
  void    createOverlayADD();
  void    setOverlaySync(bool);
private slots:  
  void    changeOVL(int sigid);
  void    changeOVLOpacity(int op);
  void    changeOVLSlice(int op);
  void    changeOVLWeight(int op);
  void    changeOVLForm(int op);
  void    changeOVLFeatures(int sigid);
  void    changeOVLPos(int id);
  
  void    metaOVLReplace(int);
  void    metaOVLCreate(int);
  void    changeSigtype(int);
  void    changeScaling(int value);
  
  void    changeMarkData(int);
  void    changeClusterPalette();
public slots:
  void    generateData();
  void    updateAllDraws();
};

#endif // MAINWINDOW_H
