#ifndef DRAWQWIDGET_H
#define DRAWQWIDGET_H

/// DrawCore does not contain any Qt code. DrawQWidget is an arrangement of DrawCore on Qt
/// Based on QOpenGLWidget, DrawQWidget also rearrange DrawCore methods on Qt slots
/// Qt Events also appears here
/// 
/// If you want operate pure bsdraws interface, use DrawCore
/// But if you want operate a connective (bsdraws+QWidget), use DrawQWidget
/// 
/// Created By: Elijah Vlasov

#include "bsdraw.h"


#include <QtGlobal>
#include <QVector>
#if QT_VERSION >= 0x050000
#include <QOpenGLWidget>
#ifndef BSGLSLOLD
#include <QOpenGLFunctions_4_2_Core>
#else
#include <QOpenGLFunctions>
#endif
#include <QOpenGLShaderProgram>
#elif QT_VERSION >= 0x040000
#include <QGLWidget>
#include <QGLFunctions>
#include <QGLShaderProgram>

#define QOpenGLWidget QGLWidget
#define QOpenGLFunctions QGLFunctions
#define QOpenGLShaderProgram QGLShaderProgram
#define QOpenGLShader QGLShader
#define initializeOpenGLFunctions initializeGLFunctions
#endif

class QScrollBar;
class QImage;
class BSQSetup;
inline QColor bsqcolor(unsigned int v){ return QColor((v)&0xFF, (v>>8)&0xFF, (v>>16)&0xFF); }

////////////

class   tftconstext_t
{
  int           hoid;
  int           recid;
  friend class  DrawQWidget;
  tftconstext_t(int hid, int rid): hoid(hid), recid(rid){}
public:
  tftconstext_t(const tftconstext_t& cpy): hoid(cpy.hoid), recid(cpy.recid) {}
};
class   tftwriting_t
{
  DrawQWidget*  pdraw;
  int           hoid;
  int           sloid;
  friend class  DrawQWidget;
  tftwriting_t(DrawQWidget* _pdraw, int hid, int sid): pdraw(_pdraw), hoid(hid), sloid(sid){}
public:
  tftwriting_t(const tftwriting_t& cpy): pdraw(cpy.pdraw), hoid(cpy.hoid), sloid(cpy.sloid) {}
  void    move(float fx, float fy);
  void    rotate(float anglerad);
  void    switchto(int recid);
};
////////////////


struct dcgeometry_t
{
  int  cttr_pre;           // left/top
  int  viewalign_pre;      // left/top
  int  length;             // width/height
  int  viewalign_post;     // left/top
  int  cttr_post;          // left/top
};

//////////////


class DrawQWidget:  public QOpenGLWidget, 
#ifndef BSGLSLOLD
                    protected QOpenGLFunctions_4_2_Core, 
#else
                    protected QOpenGLFunctions,
#endif
                    public DrawCore
{
  Q_OBJECT
  enum  SHEIFIELD  {  
    SF_DATASAMPLER, SF_DATADIMM_A, SF_DATADIMM_B, SF_DATAPORTIONS, SF_DATAPORTIONSIZE, SF_DATARANGE, 
    SF_SCALER_A, SF_SCALER_B,
    SF_PALETSAMPLER, SF_PALETRANGE,
    _SF_COUNT
  };
  enum  {   MAX_OPTIONALS=32,  MAX_TEXTURES=96 };
//    SF_DOMAIN, SF_GROUND=SF_DOMAIN, SF_PORTIONSIZE, 
//    SF_VIEW_TURN,
//    _SF_COUNT
//                   };
protected:
  bool                    m_compileOnInitializeGL; /// true by default
  char*                   m_vshmem, *m_fshmem;
  unsigned int            m_vshalloc, m_fshalloc;
  ISheiGenerator*         m_pcsh;
  int                     m_portionMeshType;
    
  int                     m_locationPrimary[_SF_COUNT];
  struct locbackaft_t
  {
    int     location;
    bool    istexture;
  }                       m_locationSecondary[MAX_OPTIONALS];
  unsigned int            m_locationSecondaryCount;
  
  QOpenGLShaderProgram    m_ShaderProgram;
  float                   m_SurfaceVertex[8];
protected:
  unsigned int            m_matrixLmSize;
  bool                    m_sbStatic;
  int                     m_cttrLeft, m_cttrTop, m_cttrRight, m_cttrBottom;
  float                   c_dpr, c_dpr_inv;
  int                     c_width, c_height;
protected:
  float                   m_viewAlignHorz;
  float                   m_viewAlignVert;
  float                   m_viewTurn;
protected:
  unsigned int            m_textures[MAX_TEXTURES];
  unsigned int            m_texturesCount;
public:
  DrawQWidget(DATAASTEXTURE datex, ISheiGenerator* pcsh, unsigned int portions, 
              ORIENTATION orient, SPLITPORTIONS splitPortions, unsigned int emptycolor);
  ~DrawQWidget();
  
  void  compileShaderNow();
  void  compileWhenInitializeGL(bool cflag);
  void  connectScrollBar(QScrollBar*, bool staticView=false, bool setOrientation=true);
  void  fitSize(int width_in, int height_in, dcsizecd_t* sz_horz, dcsizecd_t* sz_vert, dcgeometry_t* gm_horz, dcgeometry_t* gm_vert) const;
  float devicePixelRatio() const{ return c_dpr; }
public:
  void    setViewAlign(float dvaHorz, float dvaVert){  m_viewAlignHorz = dvaHorz; m_viewAlignVert = dvaVert; callWidgetUpdate(); }
  void    setViewAlignHorz(float dva){  m_viewAlignHorz = dva; callWidgetUpdate(); }
  float   viewAlignHorz() const {  return m_viewAlignHorz; }
  void    setViewAlignVert(float dva){  m_viewAlignVert = dva; callWidgetUpdate(); }
  float   viewAlignVert() const {  return m_viewAlignVert; }
public slots:
  void    slot_compileShader();
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
  void    slot_setDataPalette(const IPalette*);
  void    slot_setDataPaletteDiscretion(bool);
  void    slot_setDataPaletteRangeStart(float);
  void    slot_setDataPaletteRangeStop(float);
  void    slot_setDataPaletteRange(float, float);
  void    slot_setData(const float*);
  void    slot_setData(QVector<float>);
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
  
  void    slot_ovlReplace(int idx, Ovldraw* ovl); // for not-owners only
  
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
protected:
  void    palettePrepare(const IPalette *ppal, bool discrete, int levels);
  void    initCollectAndCompileShader();
  void    initializeGL();
  void    paintGL();
  void    resizeGL(int w, int h);
protected:
  virtual void    processGlLocation(int secidx, int secflags, int loc, int TEX){}
protected:
  virtual void callWidgetUpdate();
  virtual void innerRescale();
  virtual void innerUpdateGeometry();
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;
//  virtual bool event(QEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mouseDoubleClickEvent(QMouseEvent *);
#if QT_CONFIG(wheelevent)
  virtual void wheelEvent(QWheelEvent *event);
#endif
//  virtual void resizeEvent(QResizeEvent *event); no need. we afterreact in resizeGL method
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void showEvent(QShowEvent*);
private:
  void  store_crd_clk(OVL_REACTION_MOUSE oreact, int x, int y);
public:
  virtual  int    scrollValue() const;
  unsigned int    lmSize() const;
public:
  dcgeometry_t          geometryHorz() const;
  dcgeometry_t          geometryVert() const;
public slots:
  virtual void    scrollDataTo(int);
  virtual void    scrollDataToAbs(int);
protected:  
  class MemExpand2D
  {
  protected:
    const unsigned int  pc, ps;
    const unsigned int  memoryLines;
  public:
    struct mem_t
    {
      unsigned int  filled;
      unsigned int  current;
      float*        extendeddataarr;
    };
  protected:
    struct mem_t  mb;
  public:
    MemExpand2D(unsigned int portionsCount, unsigned int portionSize, unsigned int linesMemory);
    ~MemExpand2D();
  public:
    void  onSetData(const float* data);
    void  onSetData(const float* data, DataDecimator* decim);
    void  onClearData();
    bool  onFillData(unsigned int portion, int pos, float* rslt) const;
    bool  onFillDataBackward(unsigned int portion, int pos, float* rslt) const;
  public:
    unsigned int onCollectData(unsigned int portion, int pos, unsigned int sampleHorz, float* result, unsigned int countVerts, bool reverse) const;
  public:
    unsigned int  filled() const { return mb.filled; }  // in floats
    unsigned int  nonfilled() const { return memoryLines - mb.filled; } // in floats
    unsigned int  total() const { return memoryLines; }  // in floats
  public:
    mem_t   extendeddataarr_replicate();
    mem_t   extendeddataarr_replace(mem_t);
    
    void    extendeddataarr_release(mem_t md) const;
  };
protected:
  class MemExpand1D
  {
  protected:
    const unsigned int  pc, pt;
    unsigned int  pm;
    bool          anchoring;
  protected:
    bool          rounded;
    unsigned int  current;
    float*        m_extendeddataarr;
  public:
    MemExpand1D(unsigned int portionsCount, unsigned int portionSize, unsigned int additionalMemorySizeFor1Portion, bool anchor=false);
    ~MemExpand1D();
    unsigned int  rangeMemsize(unsigned int memorySize);  // between 0 and startmemsize
    void  setAnchoring(bool a){ anchoring = a; }
  public:
    void  onSetData(const float* data, unsigned int newsize);
    void  onSetData(const float* data, unsigned int newsize, DataDecimator* decim);
    void  onClearData();
    void  onFillData(int offsetBack, unsigned int samples, float* rslt, float emptyfill) const;
  public:
    unsigned int  allowed() const { return pm; }    // in floats
    unsigned int  filled() const { return rounded? pm : current; }  // in floats
    unsigned int  nonfilled() const { return rounded? 0 : pm - current; } // in floats
    unsigned int  total() const { return pt; }  // in floats
    float*        rawData() { return m_extendeddataarr; }
  };
public:
  enum  { TFT_HOLDERS=16, TFT_MAXAREA=32, TFT_TEXTMAXLEN=64, TFT_SLOTLIMIT=1024,
    TFT_STATIC=0,
    TFT_DYNAMIC=1
  };
  
  struct  TFTslot
  {
    int           recid;
    tftslot_t     slotinfo;
    int           pinger;   // only for dynamic
    int           ponger;   // only for dynamic
  };
  
  
  struct  tftpage_t
  {
    struct  record_t
    {
      char  text[TFT_TEXTMAXLEN+1];
      int   width;
    };
    QImage*       ctx_img;
    record_t*     records;
    int           recordscount;
  };
  
  struct  TFTholder
  {
    QFont                     font;
    int                       maxtextlen;
    int                       limrows;   // by font
    int                       limcols;
    
    int                       record_width;
    int                       record_height, record_ht, record_hb, record_ld;
    
    BSQSetup*                 ctx_setup;
#ifndef BSGLSLOLD
    std::vector<tftpage_t>    recbook;
#else
    tftpage_t                 recbook;
#endif
    
    std::vector<TFTslot>      writings[2];
      
    int                       pinger;
    int                       ponger;
    
    int                       _location;
    char                      _varname[64];
    int                       _location_i, _location_c;
    char                      _varname_i[64], _varname_c[64];
  };
  
private:
  int           m_holder_current = -1;
  TFTholder*    m_holders[TFT_HOLDERS];
  int           _tft_allocHolder(QFont font, int maxtextlen=TFT_TEXTMAXLEN, int limitcolumns=1);
  int           _tft_total(int hoid) const { return m_holders[hoid]->limrows*m_holders[hoid]->limcols; }
  int           _tft_pushRecord(TFTholder*  holder, const char* text);
  QImage*       _tft_allocateImage(int width, int height);
  TFTholder*    _tft_inf_takeHolder();
public:
  bool            tftRegisterHolding(const QFont& font, int maxtextlen=TFT_TEXTMAXLEN, int limitcolumns=1);
  bool            tftSwitchHolding(int hoid);
                  /// Every new record has incremented index
  int             tftAddRecord(const char* text);
  int             tftAddRecords(int count, const char* text[]);
public:
  tftwriting_t    tftPushDynamic(const char* text, COORDINATION cr, float fx, float fy);
  tftwriting_t    tftPushDynamic(const char* text, COORDINATION cr, float fx, float fy, float rotate);
  
  tftconstext_t   tftPushStatic(const char* text, COORDINATION cr, float fx, float fy);
  tftconstext_t   tftPushStatic(const char* text, COORDINATION cr, float fx, float fy, float rotate);
  
  
  tftwriting_t    tftGetSlot(int hoid, int sloid);
  tftwriting_t    tftGetSlot(int sloid);
  int             tftGetRecord(const tftwriting_t&) const;
  const char*     tftGetText(const tftwriting_t&) const;
public:
  int             tftCountRecords(int hoid) const;
  int             tftCountSlots(int hoid) const; 
  int             tftRecordsPerArea(int hoid);
public:
  
public: // tft operations
  bool            tftMove(int hoid, int sloid, float fx, float fy);
  bool            tftRotate(int hoid, int sloid, float anglerad);
  bool            tftSwitchTo(int hoid, int sloid, int recid);
  
public:
};

class BSQClickerXY: public QObject, public DrawEventReactor
{
  Q_OBJECT
public:
  BSQClickerXY(QObject* parent=nullptr);
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  clicked(float x01, float y01);
  void  released();
};

class BSQClickerPoint: public QObject, public DrawEventReactor
{
  Q_OBJECT
  OVL_REACTION_MOUSE  emitter;
public:
  BSQClickerPoint(OVL_REACTION_MOUSE em=ORM_LMPRESS, QObject* parent=nullptr);
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  clicked(QPoint);
};

class BSQMousePoint: public QObject, public DrawEventReactor
{
  Q_OBJECT
  OVL_REACTION_MOUSE  emitset[3];
public:
  enum MOUSEBUTTON  { MSP_LEFTBUTTON, MSP_RIGHTBUTTON };
  BSQMousePoint(MOUSEBUTTON btn, QObject* parent=nullptr);
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  active(QPoint);
  void  active(QPointF);
};

class BSQDoubleClicker: public QObject, public DrawEventReactor
{
  Q_OBJECT
public:
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  doubleClicked();
  void  doubleClicked(QPoint);
};

class BSQProactiveSelectorBase: public QObject, public DrawEventReactor
{
protected:
  OVL_REACTION_MOUSE    m_action;
  OVL_REACTION_MOUSE    m_drop;
  int                   m_startswith;
  bool                  m_inversed;
public:
  BSQProactiveSelectorBase(OVL_REACTION_MOUSE action, OVL_REACTION_MOUSE drop): 
    m_action(action), m_drop(drop), m_startswith(0), m_inversed(false){}
  ~BSQProactiveSelectorBase();
public:
  void  setStarts(int v){ m_startswith = v; }
  OVL_REACTION_MOUSE  action() const { return m_action; }
  void  setAction(OVL_REACTION_MOUSE action){ m_action = action; }
  bool  inversed() const { return m_inversed; }
  void  setInversed(bool inversed){ m_inversed = inversed; }
};

class BSQProactiveSelector: public BSQProactiveSelectorBase
{
  Q_OBJECT
public:
  BSQProactiveSelector(OVL_REACTION_MOUSE action=ORM_LMPRESS, OVL_REACTION_MOUSE drop=ORM_RMPRESS): BSQProactiveSelectorBase(action,drop) {}
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  selectionChanged(int);
  void  selectionDropped();
};

class BSQCellSelector: public BSQProactiveSelectorBase
{
  Q_OBJECT
public:
  BSQCellSelector(OVL_REACTION_MOUSE action=ORM_LMPRESS, OVL_REACTION_MOUSE drop=ORM_RMPRESS): BSQProactiveSelectorBase(action, drop) {}
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* doStop);
signals:
  void  selectionChanged(int cellA, int cellB);
  void  selectionDropped();
};

class BSQSelectorA: public BSQProactiveSelectorBase
{
  Q_OBJECT
public:
  BSQSelectorA(OVL_REACTION_MOUSE action=ORM_LMPRESS, OVL_REACTION_MOUSE drop=ORM_RMPRESS): BSQProactiveSelectorBase(action, drop) {}
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* doStop);
signals:
  void  selectionChanged(int);
  void  selectionDropped();
};

class BSQSelectorB: public BSQProactiveSelectorBase
{
  Q_OBJECT
public:
  BSQSelectorB(OVL_REACTION_MOUSE action=ORM_LMPRESS, OVL_REACTION_MOUSE drop=ORM_RMPRESS): BSQProactiveSelectorBase(action, drop) {}
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* doStop);
signals:
  void  selectionChanged(int);
  void  selectionDropped();
};

////////////////////////////////////////

#endif // DRAWQWIDGET_H

