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
/// dbar->addScaleDrawUniSide(AT_TOP, 0, 21);
/// dbar->addScaleDrawGraphB(AT_LEFT, 0 | DBF_NOTESINSIDE, 21, 20);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include <QWidget>

class DrawQWidget;
class QScrollBar;
class DrawBars;



enum  ATTACHED_TO { AT_LEFT, AT_RIGHT, AT_TOP, AT_BOTTOM };

class MarginElement
{
public:
  virtual ~MarginElement();
protected:
  enum UPDATEFOR { UF_NONE=-2, UF_APPEND=-1, UF_RESIZE=0, UF_LVL1, UF_LVL2, UF_LVL3,      UF_FORCED=99999  };
  struct uarea_t
  {
    ATTACHED_TO atto;
    int atto_begin, atto_end;
    int dlytotal, dly1, dly2, dly3;
    bool mirrored;
    
    bool operator==(const uarea_t& cmp) const
    {
      return cmp.atto == atto && cmp.atto_begin == atto_begin && cmp.atto_end == atto_end && cmp.dlytotal == dlytotal && cmp.dly1 == dly1 && cmp.dly2 == dly2 && cmp.dly3 == dly3 && cmp.mirrored == mirrored;
    }
  };
  virtual bool  updateArea(const uarea_t& uarea, int UPDATEFOR)=0;
  virtual void  draw(QPainter&)=0;
  virtual void  sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const =0;
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





class MEWLabel;
class MEWSpace;
class MEWPointer;
class MEWScaleNN;      // NN - 1 note per 1 mark
class MEWScaleNM;      // NM notes between marks
class MEWScaleTAP;
class MEWScale;

enum   // DrawBarsFlags
{
  /// 1. marks mode
  DBMODE_DEFAULT=0,   // one of following 3
  DBMODE_STRETCHED_POW2=1,
  DBMODE_STRETCHED=2,
  DBMODE_STATIC=3,
  
  /// 2. flags
  DBF_SHARED=4,
  DBF_INTERVENTBANNED=8,
  
  DBF_ONLY2NOTES=16,
  DBF_NOTESINSIDE=32,
  
  DBF_LABELAREA_FULLBAR=64,
  
  DBF_ENUMERATE_FROMZERO=128,
  DBF_ENUMERATE_SHOWLAST=256,
  
  DBF_MINSIZE_BY_PIXSTEP=512
};


class DrawBars : public QWidget
{
  Q_OBJECT
  class DrawBars_impl*  pImpl;
  class DrawQWidget*    pDraw;
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
public:
  DrawQWidget*        getDraw();
  const DrawQWidget*  getDraw() const;
  DrawQWidget*        replaceDraw(DrawQWidget*);
public:
  int                 barSize(ATTACHED_TO atto) const;
  int                 barSizeLeft() const { return barSize(AT_LEFT); }
  int                 barSizeRight() const { return barSize(AT_RIGHT); }
  int                 barSizeTop() const { return barSize(AT_TOP); }
  int                 barSizeBottom() const { return barSize(AT_BOTTOM); }
public:
  MEQWrapper*         addMarginElement(ATTACHED_TO atto, MarginElement* pme, MEQWrapper* pwp, bool sharedWithPrev, bool interventBanned);
public:
  MEWLabel*           addLabel(ATTACHED_TO atto, int flags, QString text, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal/*, float orientAngleGrad=0.0f*/);
  MEWSpace*           addSpace(ATTACHED_TO atto, int space);
  MEWSpace*           addContour(ATTACHED_TO atto, int space=0, bool maxzone=false);
  
  MEWPointer*         addPointerFixed(ATTACHED_TO atto, int flags, float LL, float HL);
  MEWPointer*         addPointerDrawUniSide(ATTACHED_TO atto, int flags);
  MEWPointer*         addPointerDrawGraphB(ATTACHED_TO atto, int flags);
  
  MEWScale*           addScaleEmpty(ATTACHED_TO atto, int flags, int fixedCount=11, int pixStep_pixSpacing=30, int miniPerMaxiLIMIT=9);
  MEWScaleNN*         addScaleFixed(ATTACHED_TO atto, int flags, float LL, float HL, int fixedCount=11, int pixStep_pixSpacing=50, int miniPerMaxiLIMIT=9);
  MEWScaleNM*         addScaleEnumerator(ATTACHED_TO atto, int flags, int marksCount, int pixStep_pixSpacing, unsigned int step=1, bool alwaysShowLast=false);
  
  MEWScaleNN*         addScaleTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksCount=11, int pixStep_pixSpacing=30);
  MEWScaleNM*         addScaleTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param=nullptr, int marksCount=11, int pixStep_pixSpacing=30);
  
  MEWScale*           addScaleDrawUniSide(ATTACHED_TO atto, int flags, int pixSpacing, unsigned int step=1);
  MEWScale*           addScaleDrawUniSide(ATTACHED_TO atto, int flags, float LL, float HL, int pixSpacing, int miniPerMaxiLIMIT=9);
  MEWScale*           addScaleDrawGraphB(ATTACHED_TO atto, int flags, int marksCount=11, int pixSpacing=30, int miniPerMaxiLIMIT=9);
  MEWScale*           addScaleDrawGraphB(ATTACHED_TO atto, int flags, float LL, float HL, int marksCount=11, int pixSpacing=30, int miniPerMaxiLIMIT=9);
  
  MEWScaleTAP*        addScaleDrawRecorderB(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, const void* param=nullptr, int miniPerMaxiLIMIT=0);
  MEWScaleTAP*        addScaleDrawRecorderNM(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, const void* param=nullptr, int miniPerMaxiLIMIT=0);
public:
  void                retrieveMElement(MEQWrapper*, bool replaceWithEqSpace);
  void                setVisible(MEQWrapper*, bool);
  void                switchToAnotherSide(MEQWrapper*);
  void                changeColor(MEQWrapper*, const QColor& clr);
  void                swapBars(ATTACHED_TO);
  void                removeAllMElements(bool squeeze=false);
protected:
  void                elemSizeHintChanged(MarginElement* me);
  friend class        MEQWrapper;
protected:
  virtual QSize minimumSizeHint() const;
//  virtual QSize sizeHint() const;
  virtual void  resizeEvent(QResizeEvent *event);
  virtual void  paintEvent(QPaintEvent *event);
  virtual bool  event(QEvent*);
public:
  void    connectScrollBar(QScrollBar*, bool staticView=false, bool setOrientation=true);
signals:
  void    sig_allo();
public slots:
  void    slot_setScalingH(int);
  void    slot_setScalingV(int);
  void    slot_setBounds(float low, float high);
  void    slot_setBoundLow(float);
  void    slot_setBoundHigh(float);
  void    slot_setContrast(float k, float b);
  void    slot_setContrastK(float);
  void    slot_setContrastB(float);
  void    slot_setDataTextureInterpolation(bool);
  void    slot_setDataPalette(const class IPalette*);
  void    slot_setDataPaletteDiscretion(bool);
  void    slot_setData(const float*);
  void    slot_setData(const QVector<float>&);
  void    slot_fillData(float);
  void    slot_clearData();
  
  void    slot_setMirroredHorz();
  void    slot_setMirroredVert();
  void    slot_setPortionsCount(int count);
  
  void    slot_enableAutoUpdate(bool);
  void    slot_disableAutoUpdate(bool);
  void    slot_enableAutoUpdateByData(bool);
  void    slot_disableAutoUpdateByData(bool);
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
};

/**********************************************************************************************************************/
/**********************************************************************************************************************/


class MEQWTexted: public MEQWrapper
{
  Q_OBJECT
//  using MEQWrapper::MEQWrapper;   // awwww fuck you C11
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


class DrawOverlayProactive;
class MEWPointer: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
public:
  DrawOverlayProactive*    createProactive();
  DrawOverlayProactive*    createProactive(float start_x, float start_y);
public slots:
  void  setPosition(float pos01);
};

class MEWScale: public MEQWrapper
{
  Q_OBJECT
  friend class DrawBars;
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
};

class MEWScaleNM: public MEWScale
{
  Q_OBJECT
  friend class DrawBars;
};

class MEWScaleTAP: public MEWScale
{
  Q_OBJECT
  friend class DrawBars;
public slots:
  void  tap();
};

#endif // BSDRAWSCALES_H
