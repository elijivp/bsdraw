#ifndef BSDRAWSCALES_H
#define BSDRAWSCALES_H

/// This file contains special class who describes Bars around bsdraws
/// Each Bar, or margin, allow you to locate scales, labels, lines, etc.. near bsdraw
/// Each Bar is a mini layout. He eats in one turn two special interfaces:
///   - MarginElement for place and draw
///   - MEQWrapper for control
/// Background and foreground ColorPolicy can be bsdraw-depended. Or You can make is standard by QPalette
/// 
/// Example:
/// #include "bsdrawgraph.h"
/// #include "bsdrawscales.h"
/// #include "palettes/bspalettes_std.h"
/// 
/// DrawGraph* draw = new DrawGraph(SAMPLES, PORTIONS, graphopts_t::goInterp());
/// draw->setDataPalette(&paletteBkGrWh);
/// ...
/// DrawBars* dbar = new DrawBars(draw, DrawBars::CP_FROM_DRAWBACK);
/// dbar->addEScalePixstepDrawbounds(AT_TOP, 0, 21);
/// dbar->addScalePixstepDrawbounds(AT_LEFT, 0 | DBF_NOTESINSIDE, 21, 20);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include <QWidget>


class DrawQWidget;
class QScrollBar;
class DrawBars;
class MEQWrapper;

enum  ATTACHED_TO { AT_LEFT, AT_RIGHT, AT_TOP, AT_BOTTOM };

class MarginElement
{
public:
  virtual ~MarginElement();
protected:
  enum UPDATEFOR  { UF_RESIZE, UF_CONTENT, UF_FORCED };
  enum MOUSEEVENT { ME_LPRESS, ME_LRELEASE, ME_LMOVE, ME_RPRESS, ME_RRELEASE, ME_RMOVE };
  struct uarea_t
  {
    ATTACHED_TO atto;
    int atto_begin, atto_end;
    int segm_pre, segm_main, segm_post, segm_over;
    int segment_full;
    bool mirrored;
    int ex_scaling;
    
    bool operator==(const uarea_t& cmp) const
    {
      return cmp.atto == atto && cmp.atto_begin == atto_begin && cmp.atto_end == atto_end && 
          cmp.segm_pre == segm_pre && cmp.segm_main == segm_main && cmp.segm_post == segm_post && cmp.segm_over == segm_over && 
          cmp.mirrored == mirrored && cmp.ex_scaling == ex_scaling;
    }
    bool operator!=(const uarea_t& cmp) const
    {
      return cmp.atto != atto || cmp.atto_begin != atto_begin || cmp.atto_end != atto_end || 
          cmp.segm_pre != segm_pre || cmp.segm_main != segm_main || cmp.segm_post != segm_post || cmp.segm_over != segm_over || 
          cmp.mirrored != mirrored || cmp.ex_scaling != ex_scaling;
    }
  };
  virtual bool  updateArea(const uarea_t& uarea, int UPDATEFOR)=0;
  virtual void  draw(QPainter&)=0;
  virtual void  mouseEvent(MOUSEEVENT /*mev*/, int /*pos_segm*/, int /*pos_atto*/, int /*dimm_segm*/, int /*dimm_atto*/, bool* /*doUpdate*/, MEQWrapper* /*selfwrap*/=nullptr){}
  virtual void  sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const =0;
  virtual void  relatedInit(const DrawQWidget*) {  }
  virtual void  changeColor(const QColor&)=0;
  friend class DrawBars;
  friend class DrawBars_impl;
  friend class MEQWrapper;
};

class MEQWrapper: public QObject
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MarginElement*  m_pme;
  DrawBars*       m_premote;
  MEQWrapper();
  virtual ~MEQWrapper();
  void remoteUpdate();
  void remoteRebound();
public slots:
  void  remove();
  void  removeAndLeftSpace();
  void  setVisible(bool);
  void  setVisibleWithoutUpdate(bool);
  void  moveToAnotherSide();
  void  changeColor(const QColor& clr);
};




typedef void  (*mtap_qstring_fn)(int mark, int dimmarea, int reloffset, const void*, QString& reservedResult);

template <int recycle>
void  standard_tap_symbolate(int mark, int dimmarea, int reloffset, const void*, QString& reservedResult)
{
  const char* latins = "abcdefghijklmnopqrstuvwxyz";
  const int   latinslen = recycle < 0 || recycle > 26? 26 : recycle;
  reservedResult = latins[mark % int(latinslen + latinslen*(float(reloffset)/dimmarea))];
}

typedef QWidget*  (*mtap_qwidget_fn)(int mark, int dimmarea, int reloffset, void*);



class IToolUpdateReactor
{
public:
  virtual ~IToolUpdateReactor(){}
  virtual void    bndUpdated(const DrawQWidget* bnds)=0;
};


class MEWLabel;
class MEWSpace; class MEWColoredSpace;
class MEWPointer;
class MEWScaleNN;      // NN - 1 note per 1 mark
class MEWScaleNM;      // NM notes between marks
class MEWScaleTAP;  class MEWScaleTAPNN;  class MEWScaleTAPNM;
class MEWScale;

enum   // Draw Bars Flags
{
  DBF_SHARED=           0x1,          // use space from previous MarginElement
  DBF_ORIENT_IGNORE=    0x2,          // ignore Draw orientation
  DBF_ORIENT_INVERT=    0x4,          // invert Draw orientation
  DBF_INTERVENTBANNED=  0x8,          // deny use neighboor bars for MarginElement
  
  DBF_NOTE_BORDERS_ONLY =     0x10,     // scale with first and last labels
  DBF_NOTESINSIDE =           0x20,     // first and last labels dont cross neighboor bars
  DBF_ENUMERATE_FROMZERO =    0x40,     // enumerator scale starts from 0, not from 1
  DBF_ENUMERATE_SHOWLAST =    0x80,     // forever show last enumerator and attach it to last mark
  
  DBF_MINSIZE_BY_MINSPACING = 0x100,    // minimal size of Draw sets by minspacing*marks count
  DBF_MINSIZE_BY_PIXSTEP =    0x100,    // same as previous
  
  DBF_LABELAREA_FULLBAR =     0x200,    // label use neighboor bars
  
  DBF_MARKS_ROUNDING_OFF =        0,    // mark placement rounding algorithm: dont round marks, default
  DBF_MARKS_ROUNDING_ON =     0x400,    // mark placement rounding algorithm: qRound mark position
  DBF_MARKS_ROUNDING_OFF_INC =0x800,    // mark placement rounding algorithm: dont round marks and add 1
  
  DBF_POSTFIX_TO_PREFIX =     0x1000,   // 
  DBF_DOCKTO_PREVMARK =       0x2000,   // for NM attach each label to previous mark
  DBF_DOCKTO_NEXTMARK =       0x4000,   // for NM attach each label to next mark
  DBF_POSTFIX_ONLYLAST =      0x8000,   // for NM attach each label to next mark
  
  
  DBF_PRECISION_INCREASE =    0x10000,  // add 1 more digit to default precision algorithm
  DBF_PRECISION_MAXIMIZE =    0x20000,  // add 2 more digits to default precision algorithm
  DBF_PRECISION_DECREASE =    0x30000,  // remove 1 more digit from default precision algorithm
  DBF_PRECISION_MINIMIZE =    0x40000,  // remove 2 more digits (if possible) from default precision algorithm
  
  _DBF_PRECISION_GROUP =      0xF0000,
  DBF_PRECISION_EXACT_0 =     0x50000,
  DBF_PRECISION_EXACT_1 =     0x60000,
  DBF_PRECISION_EXACT_2 =     0x70000,
  DBF_PRECISION_EXACT_3 =     0x80000,
  DBF_PRECISION_EXACT_4 =     0x90000,
  DBF_PRECISION_EXACT_5 =     0xA0000,
  DBF_PRECISION_EXACT_6 =     0xB0000,
  
  DBF_RETAP_ON_RESIZE =       0x100000,
  DBF_NO_RETAP_ON_SCROLL =    0x200000,
  
  DBF_NATIVE_DIV_10_5_2 =     0x100000, // default
  DBF_NATIVE_DIV_15_5_3 =     0x200000,
  DBF_NATIVE_DIV_10 =         0x300000
};


class DrawBars : public QWidget
{
  Q_OBJECT
  class DrawBars_impl*  pImpl;
  DrawQWidget*          pDraw;
  IToolUpdateReactor*   pTool;
public:
  enum  COLORS {  CP_DEFAULT,       // use default widget palette
                  CP_FROM_DRAWBACK,     // use DrawQWidget colorBack function
                  CP_FROM_DRAWPALETTE,      // use DrawQWidget dataPalette colors
                  CP_FROM_DRAWPALETTE_INV,   // use DrawQWidget dataPalette inversed colors
                  
                  CP_WHITE,
                  CP_BLACK
               };
  
  explicit DrawBars(DrawQWidget* pdraw, COLORS colors=CP_FROM_DRAWPALETTE, /*bool expandNeighborBarsIfNeed=false, */QWidget *parent = nullptr);
  ~DrawBars();
  void                setColorPolicy(COLORS cp);
  void                setColors(const QColor& backgroundColor, const QColor& foregroundColor);
  void                setColors(unsigned int backgroundColor, unsigned int foregroundColor);
  
  void                setOpacity(float opacity);    // 0 - invisible, 1 - noopacity
  
  void                enableDrawBoundsUpdater(bool);
public:
  DrawQWidget*        draw();
  DrawQWidget*        getDraw();
  const DrawQWidget*  draw() const;
  const DrawQWidget*  getDraw() const;
  QRect               getDrawGeometry() const;
  DrawQWidget*        replaceDraw(DrawQWidget*);
public:
  int                 barSize(ATTACHED_TO atto) const;
  int                 barSizeLeft() const { return barSize(AT_LEFT); }
  int                 barSizeRight() const { return barSize(AT_RIGHT); }
  int                 barSizeTop() const { return barSize(AT_TOP); }
  int                 barSizeBottom() const { return barSize(AT_BOTTOM); }
public:
  void                setToolUpdateReactor(IToolUpdateReactor* ptool){  pTool = ptool; }
  void                clearToolUpdateReactor(){  pTool = nullptr; }
public:
  MEQWrapper*         addMarginElement(ATTACHED_TO atto, MarginElement* pme, MEQWrapper* pwp, bool sharedWithPrev, bool interventBanned, int mirrorAlgo=0);   // Miralg = 0 - off, 1 - mirroring ignore, 2 - mirroring invert
public:
  /// 1. Fixed objects
  MEWLabel*           addLabel(ATTACHED_TO atto, int flags, QString text, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal/*, float orientAngleGrad=0.0f*/);
  MEWSpace*           addSpace(ATTACHED_TO atto, int space);
  MEWColoredSpace*    addSpace(ATTACHED_TO atto, int space, QColor color, bool maxzone=false);
//  MEWSpace*           addStretch(ATTACHED_TO atto, int space, int stepSelf=1, int stepDraw=10);
  MEWSpace*           addContour(ATTACHED_TO atto, int space=0, bool maxzone=false);
  MEWSpace*           addContour(ATTACHED_TO atto, int space, QColor color, bool maxzone);
  
  
  /// 2. Pointers
  /// Mark with label. Label is lineary attached to mark position: LL + p*(HL-LL)
  /// initial position sets in constructor and can be changed through setPosition() method
  /// position value can be relative in range 0..1, or absolute in LL..HL range.

  /// Naming:       addPointer_T_S
  /// where    T - Marks placement algorithm
  ///          S - Labels source
  MEWPointer*         addPointerRelativeOwnbounds(ATTACHED_TO atto, int flags, float pos, float LL, float HL, int marklen=0, float MOD=0.0f, const char* postfix=nullptr);
  MEWPointer*         addPointerAbsoluteOwnbounds(ATTACHED_TO atto, int flags, float pos, float LL, float HL, int marklen=0, float MOD=0.0f, const char* postfix=nullptr);
  MEWPointer*         addPointerRelativeDrawbounds(ATTACHED_TO atto, int flags, float pos, int marklen=0, float MOD=0.0f, const char* postfix=nullptr);
  MEWPointer*         addPointerAbsoluteDrawbounds(ATTACHED_TO atto, int flags, float pos, int marklen=0, float MOD=0.0f, const char* postfix=nullptr);
  MEWPointer*         addEPointer01Auto(ATTACHED_TO atto, int flags, float pos, int marklen=0, const char* postfix=nullptr);
  
  /// 3. Scales
  /// Example:      I i i i I i i i I i i i I
  ///     I - maxi mark, i - mini mark
  ///     maxi mark count is limited by marksLimit value. true count depends on algorithm.
  ///     mini mark count is regulated by miniPerMaxi value
  /// Example 2:    I i i i I i i i I i i i I
  ///              0.2     0.4     0.6     0.8
  /// Example 3:    I i i i I i i i I i i i I
  ///                   0       1       2
  ///     labels can be placed under maxi marks (example 2): N labels for N marks, NN
  ///       or between maxi marks (example 3): N labels for M marks, NM
  /// 
  
  /// Naming:      addScale_M_S
  /// where    M - Marks placement algorithm
  ///          S - Labels source
  
                      ///   Unmarked scales
  MEWScale*           addScalePixstepEmpty(ATTACHED_TO atto, int flags, int marksLimit=11, int minSpacing=30, int miniPerMaxi=9);
  MEWScale*           addScaleSymmetricEmpty(ATTACHED_TO atto, int flags, int marksLimit=11, int minSpacing=30, int miniPerMaxi=9);
  MEWScale*           addScaleRollingEmpty(ATTACHED_TO atto, int flags, int marksLimit=11, int pixStep=30, int miniPerMaxi=9);

                      ///   Marked scales
  MEWScaleNN*         addScaleNativeOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, float minSTEP, float minSTEPbase=0, int marksLimit=11, int minSpacing=50, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScaleNN*         addScalePixstepOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit=11, int minSpacing=50, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScaleNN*         addScaleSymmetricOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit=11, int minSpacing=50, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScaleNN*         addScaleRollingOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit=11, int pixStep=50, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScale*           addScaleNativeDrawbounds(ATTACHED_TO atto, int flags, float minSTEP, float minSTEPbase=0, int marksLimit=11, int minSpacing=30, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScale*           addScalePixstepDrawbounds(ATTACHED_TO atto, int flags, int marksLimit=11, int minSpacing=30, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScale*           addScaleSymmetricDrawbounds(ATTACHED_TO atto, int flags, int marksLimit=11, int minSpacing=30, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  MEWScale*           addScaleRollingDrawbounds(ATTACHED_TO atto, int flags, int marksLimit=11, int pixStep=30, int miniPerMaxi=9, float MOD=0.0f, const char* postfix=nullptr);
  
                      ///   Enumerators 
//  MEWScaleNM*         addEScaleNativeOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step=1, const char* postfix=nullptr);
  MEWScaleNM*         addEScalePixstepOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step=1, const char* postfix=nullptr);
  MEWScaleNM*         addEScaleRollingOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step=1, const char* postfix=nullptr);
  MEWScale*           addEScalePixstepDrawbounds(ATTACHED_TO atto, int flags, int minSpacing, unsigned int step=1, const char* postfix=nullptr);
  
                      ///   Tappers
                        ///   Tap = call external function for get text
  MEWScaleTAPNN*      addScalePixstepTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int minSpacing=30, const char* postfix=nullptr);
  MEWScaleTAPNN*      addScaleSymmetricTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int minSpacing=30, const char* postfix=nullptr);
  MEWScaleTAPNN*      addScaleRollingTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int pixStep=30, const char* postfix=nullptr);
  
  MEWScaleTAPNM*      addScalePixstepTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int minSpacing=30, const char* postfix=nullptr);
  MEWScaleTAPNM*      addScaleSymmetricTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int minSpacing=30, const char* postfix=nullptr);
  MEWScaleTAPNM*      addScaleRollingTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksLimit=11, int pixStep=30, const char* postfix=nullptr);

                        ///   Tap = call external function for get QWidget object
  MEWScaleTAPNM*      addWScalePixstepTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param=nullptr, int marksLimit=11, int minSpacing=30);
  MEWScaleTAPNM*      addWScaleSymmetricTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param=nullptr, int marksLimit=11, int minSpacing=30);
  MEWScaleTAPNM*      addWScaleRollingTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param=nullptr, int marksLimit=11, int pixStep=30);
  
  MEWScaleNM*         addWScalePixstepSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int minSpacing=30);
  MEWScaleNM*         addWScaleSymmetricSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int minSpacing=30);
  MEWScaleNM*         addWScaleRollingSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int pixStep=30);
  
public:
  void                retrieveMElement(MEQWrapper*, bool replaceWithEqSpace);
  void                setMElementVisible(MEQWrapper*, bool v, bool autoupdate=true);
  void                switchToAnotherSide(MEQWrapper*);
//  void                changeColor(MEQWrapper*, const QColor& clr);
  void                swapBars(ATTACHED_TO);
  void                removeAllMElements(bool squeeze=false);
  void                mouseEvent(MarginElement::MOUSEEVENT mev, int x, int y);
protected:
  void                rollbackGeometry();
  void                elemSizeHintChanged(MarginElement* me);
  friend class        MEQWrapper;
protected:
  virtual QSize minimumSizeHint() const;
//  virtual QSize sizeHint() const;
  virtual void  resizeEvent(QResizeEvent *event);
  virtual void  paintEvent(QPaintEvent *event);
  virtual bool  event(QEvent*);
  virtual void  mousePressEvent(QMouseEvent* event);
  virtual void  mouseMoveEvent(QMouseEvent* event);
  virtual void  mouseReleaseEvent(QMouseEvent* event);
  virtual void  mouseDoubleClickEvent(QMouseEvent* event);
public:
  void    connectScrollBar(QScrollBar*, bool staticView=false, bool setOrientation=true);
public slots:
  void    slot_setScalingA(int);
  void    slot_setScalingB(int);
  void    slot_setScalingH(int);
  void    slot_setScalingV(int);
  void    slot_setBounds(float low, float high);
  void    slot_setBoundLow(float);
  void    slot_setBoundHigh(float);
  void    slot_setBounds01();
  void    slot_setContrast(float k, float b);
  void    slot_setContrastK(float);
  void    slot_setContrastKinv(float);
  void    slot_setContrastB(float);
  void    slot_setDataTextureInterpolation(bool);
  void    slot_setDataPalette(const class IPalette*);
  void    slot_setDataPaletteDiscretion(bool);
  void    slot_setDataPaletteRangeStart(float);
  void    slot_setDataPaletteRangeStop(float);
  void    slot_setDataPaletteRange(float, float);
  void    slot_setData(const float*);
  void    slot_setData(const QVector<float>&);
  void    slot_fillData(float);
  void    slot_clearData();
  
  void    slot_adjustBounds();
  void    slot_adjustBounds(unsigned int portion);
  void    slot_adjustBounds(unsigned int start, unsigned int stop);
  void    slot_adjustBounds(unsigned int start, unsigned int stop, unsigned int portion);
  void    slot_adjustBoundsWithSpacingAdd(float add2min, float add2max);
  void    slot_adjustBoundsWithSpacingMul(float mul2min, float mul2max);
  
  void    slot_setMirroredHorz();
  void    slot_setMirroredVert();
  void    slot_setPortionsCount(int count);
  
  void    slot_enableAutoUpdate(bool);
  void    slot_disableAutoUpdate(bool);
  void    slot_enableAutoUpdateByData(bool);
  void    slot_disableAutoUpdateByData(bool);
  
  //   additional slots for spinboxes
  void    slot_setBoundLow_dbl(double);
  void    slot_setBoundHigh_dbl(double);
  void    slot_setContrast_dbl(double k, double b);
  void    slot_setContrastK_dbl(double);
  void    slot_setContrastKinv_dbl(double);
  void    slot_setContrastB_dbl(double);
  void    slot_setDataPaletteRangeStart_dbl(double);
  void    slot_setDataPaletteRangeStop_dbl(double);
public slots:
//  void    slot_setBackgroundColor(const QColor& color);
  void    slot_setForegroundColor(const QColor& color);
  
  void    slot_swapBarsLR();
  void    slot_swapBarsTB();
  
  void    slot_updatedDataPalette();
  void    slot_updatedBounds();
  void    slot_updatedOrientation();
protected slots:
  void    scrollDataTo(int);
  void    toolUpdateBoundHigh(double);
  void    toolUpdateBoundLow(double);
};

/**********************************************************************************************************************/
/**********************************************************************************************************************/


class MEQWTexted: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  setFont(const QFont& font);
};

class MEWLabel: public MEQWTexted
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  setText(const QString& text);
};

class MEWSpace: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  setSpace(int space);
};

class MEWColoredSpace: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  setSpace(int space);
};


class IOverlayReactor;
class MEWPointer: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public:
  typedef float (*proconvert_fn)(float x, float y);
  typedef float (*proconvert_bi_fn)(float x, float y, float* ptrbi01);
public:
  IOverlayReactor*    createReactor();
//  void  setPrefix(const char* str);
//  void  setPostfix(const char* str);
public slots:
  void  setPosition(float pos01);
  void  setPositionBifunc(float pos01, float posText);
public slots:
  void  setBounds(float LL, float HL);
  void  setBoundLow(float LL);
  void  setBoundHigh(float HL);
  void  setBounds(double LL, double HL);
  void  setBoundLow(double LL);
  void  setBoundHigh(double HL);
};

class MEWScale: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public:
  void  updateTapParam(const void*);
public:
//  void  setPrefix(const char* str);
//  void  setPostfix(const char* str);
public slots:
  void  setFont(const QFont& font);
  void  setMarkLen(int mlen);
//  void  setMarkColor(const QColor& clr);
  void  setMarkMiniLen(int mlen);
//public slots:
//  void  scroll(int);
};

class MEWScaleNN: public MEWScale
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  setBounds(float LL, float HL);
  void  setBoundLow(float LL);
  void  setBoundHigh(float HL);
  void  setBounds(double LL, double HL);
  void  setBoundLow(double LL);
  void  setBoundHigh(double HL);
};

class MEWScaleNM: public MEWScale
{
  Q_OBJECT
  friend class DrawBars;
};

//class MEWScaleTAP: public MEWScale
//{
//  Q_OBJECT
//  friend class DrawBars;
//public slots:
//  void  tap();
//};

class MEWScaleTAPNN: public MEWScaleNN
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  tap();
};

class MEWScaleTAPNM: public MEWScaleNM
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  tap();
};

#endif // BSDRAWSCALES_H
