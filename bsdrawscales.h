#ifndef BSDRAWSCALES_H
#define BSDRAWSCALES_H

#include <QWidget>

//#include "core/bsqdraw.h"

class DrawQWidget;
class MarginElement;
class DrawBars;

class MarginHolder: public QObject
{
  Q_OBJECT
protected:
  MarginElement*  m_pme;
  DrawBars*       m_premote;
  MarginHolder(MarginElement* pme, DrawBars* remote): m_pme(pme), m_premote(remote){}
  friend class DrawBars;
protected:
  void remoteUpdate();
  void remoteRebound();
public:
  MarginHolder(const MarginHolder& cpy);
  virtual ~MarginHolder();
  
  bool  empty() const { return m_pme == nullptr; }
  bool  assigned() const { return m_premote != nullptr; }
  
public slots:
  /// remote recall functions (no own operations because of async removing)
  void  remove();
  void  removeAndLeftSpace();
  void  setVisible(bool);
  void  moveToAnotherSide();
};

typedef void  (*mtap_qstring_fn)(int marknum, QString& reservedResult);


template <int recycle>
void  standard_tap_symbolate(int marknum, QString& reservedResult)
{
  const char* latins = "abcdefghijklmnopqrstuvwxyz";
  const int   latinslen = recycle < 0 || recycle > 26? 26 : recycle;
  reservedResult = latins[marknum % latinslen];
}

enum  ATTACHED_TO { AT_LEFT, AT_RIGHT, AT_TOP, AT_BOTTOM };

class MarginElement
{
public:
  virtual ~MarginElement();
protected:
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
protected:
  enum UPDATEFOR { UF_NONE=-2, UF_APPEND=-1, UF_RESIZE=0, UF_LVL1, UF_LVL2, UF_LVL3,      UF_FORCED=99999  };
  virtual bool  updateArea(const uarea_t& uarea, int UPDATEFOR)=0;
  virtual void  draw(QPainter&, const QColor& /*foregroundColor*/)=0;
  virtual void  sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const =0;
  virtual void  relatedInit(const DrawQWidget*) {  }
  friend class DrawBars;
  friend class DrawBars_impl;
};

//bool operator==(const MarginElement::uarea_t& cmp, const MarginElement::uarea_t& cmp2)
//{
//  return cmp.atto == cmp2.atto && cmp.atto_pt == cmp2.atto_pt && cmp.atto_pt_shared == cmp2.atto_pt_shared && cmp.dlytotal == cmp2.dlytotal && cmp.dly1 == cmp2.dly1 && cmp.dly2 == cmp2.dly2 && cmp.mirrored == cmp2.mirrored;
//}

class MELabel;
class MESpace;
class MEScaleNN;      // NN - 1 note per 1 mark
class MEScaleNM;      // NM notes between marks
class MEScaleTAP;
class MarginHolderMark;

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
                  CP_FROM_DRAWPALETTE_INV   // use DrawQWidget dataPalette inversed colors
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
  void                addMarginElement(ATTACHED_TO atto, MarginElement* me, bool sharedWithPrev, bool interventBanned);
  bool                addMarginElement(ATTACHED_TO atto, MarginHolder* me, bool sharedWithPrev=false);
public:
  MELabel             addLabel(ATTACHED_TO atto, int flags, QString text, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal/*, float orientAngleGrad=0.0f*/);
  MESpace             addSpace(ATTACHED_TO atto, int space);
  MESpace             addContour(ATTACHED_TO atto, int space=0, bool maxzone=false);
  MarginHolderMark    addScaleEmpty(ATTACHED_TO atto, int flags, int fixedCount=11, int pixStep_pixSpacing=30, int miniPerMaxiLIMIT=9);
  MEScaleNN           addScaleFixed(ATTACHED_TO atto, int flags, float LL, float HL, int fixedCount=11, int pixStep_pixSpacing=50, int miniPerMaxiLIMIT=9);
  MEScaleNM           addScaleEnumerator(ATTACHED_TO atto, int flags, int marksCount, int pixStep_pixSpacing, unsigned int step=1, bool alwaysShowLast=false);
  
  MEScaleNN           addScaleTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, int marksCount=11, int pixStep_pixSpacing=30);
  MEScaleNM           addScaleTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, int marksCount=11, int pixStep_pixSpacing=30);
  
  MarginHolderMark    addScaleDrawUniSide(ATTACHED_TO atto, int flags, int pixSpacing, unsigned int step=1);
  MarginHolderMark    addScaleDrawUniSide(ATTACHED_TO atto, int flags, float LL, float HL, int pixSpacing, int miniPerMaxiLIMIT=9);
  MarginHolderMark    addScaleDrawGraphB(ATTACHED_TO atto, int flags, int marksCount=11, int pixSpacing=30, int miniPerMaxiLIMIT=9);
  MarginHolderMark    addScaleDrawGraphB(ATTACHED_TO atto, int flags, float LL, float HL, int marksCount=11, int pixSpacing=30, int miniPerMaxiLIMIT=9);
  
  MEScaleTAP          addScaleDrawRecorderB(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, int miniPerMaxiLIMIT=0);
  MEScaleTAP          addScaleDrawRecorderNM(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, int miniPerMaxiLIMIT=0);
public:
  void                retrieveMarginHolder(MarginHolder*, bool replaceWithEqSpace);
  void                setVisible(MarginElement*, bool);
  void                switchToAnotherSide(MarginElement*);
  void                swapBars(ATTACHED_TO);
  
  void                removeAllMarginElements(bool squeeze=false);
protected:
  void                elemSizeHintChanged(MarginElement* me);
  friend class        MarginHolder;
protected:
  virtual QSize minimumSizeHint() const;
//  virtual QSize sizeHint() const;
  virtual void  resizeEvent(QResizeEvent *event);
  virtual void  paintEvent(QPaintEvent *event);
public:
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
  void    slot_setBackgroundColor(const QColor& color);
  void    slot_setForegroundColor(const QColor& color);
  
  void    slot_swapBarsLR();
  void    slot_swapBarsTB();
  
  void    slot_updatedDataPalette();
  void    slot_updatedBounds();
  void    slot_updatedOrientation();
};

/**********************************************************************************************************************/
/**********************************************************************************************************************/


class MarginHolderTexted: public MarginHolder
{
  Q_OBJECT
protected:
//  using MarginHolder::MarginHolder;   // awwww fuck you C11
  friend class DrawBars;
  MarginHolderTexted(MarginElement* pme, DrawBars* remote);
public:
  MarginHolderTexted(const MarginHolderTexted& cpy);
public slots:
  void  setFont(const QFont& font);
};

class MELabel: public MarginHolderTexted
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MELabel(MarginElement* pme, DrawBars* remote);
public slots:
  void  setText(const QString& text);
};

class MESpace: public MarginHolder
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MESpace(MarginElement* pme, DrawBars* remote);
public:
  MESpace(const MESpace& cpy);
public slots:
  void  setSpace(int space);
};

class MarginHolderMark: public MarginHolder
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MarginHolderMark(MarginElement* pme, DrawBars* remote);
public:
  MarginHolderMark(const MarginHolderMark& cpy);
public slots:
  void  setFont(const QFont& font);
  void  setMarkLen(int mlen);
  void  setMarkColor(const QColor& clr);
  void  setMarkMiniLen(int mlen);
};

class MEScaleNN: public MarginHolderMark
{
  Q_OBJECT
//  int   owntype;
protected:
  friend class DrawBars;
  MEScaleNN(MarginElement* pme, DrawBars* remote);
public:
  MEScaleNN(const MEScaleNN& cpy);
public slots:
  void  setBounds(float LL, float HL);
};

class MEScaleNM: public MarginHolderMark
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MEScaleNM(MarginElement* pme, DrawBars* remote);
public:
  MEScaleNM(const MEScaleNM& cpy);
};

class MEScaleTAP: public MarginHolderMark
{
  Q_OBJECT
protected:
  friend class DrawBars;
  MEScaleTAP(MarginElement* pme, DrawBars* remote);
public:
  MEScaleTAP(const MEScaleTAP& cpy);
public slots:
  void  tap();
};

#endif // BSDRAWSCALES_H
