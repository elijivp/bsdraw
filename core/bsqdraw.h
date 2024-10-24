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

struct tftgetermetrics_t
{
  int     maxtextlen;
  int     design_width;     // every desing max width by QFontMetrics, for maxtextlen' symbols
  int     design_height;    // every desing height by QFontMetrics
  int     design_ht;        // QFontMetrics::accent
  int     design_hb;        // QFontMetrics::descent
  int     design_ld;        // QFontMetrics::leading
};

enum { TFT_TEXTMAXLEN=64 };
struct tftgeterbook_t;
tftgeterbook_t*   tftgeterbook_alloc(QFont font, int maxtextlen=TFT_TEXTMAXLEN, int limitcolumns=1, QColor color=QColor(0,0,0));
int               tftgeterbook_addtext(tftgeterbook_t* th, const char* text);
int               tftgeterbook_addtexts(tftgeterbook_t* th, int count, const char* texts[]);
void              tftgeterbook_release(tftgeterbook_t* th);

tftgetermetrics_t tftgeterbook_metrics(const tftgeterbook_t* th);
QFont             tftgeterbook_font(const tftgeterbook_t* th);


class   tftstatic_t
{
  int           hoid;
  int           dsgid;
  friend class  DrawQWidget;
  tftstatic_t(int hid, int rid): hoid(hid), dsgid(rid){}
public:
  tftstatic_t(const tftstatic_t& cpy): hoid(cpy.hoid), dsgid(cpy.dsgid) {}
};

#ifndef TFT_DEFAULT_ALLOW_DYNAMIC_RT
#define TFT_DEFAULT_ALLOW_DYNAMIC_RT true
#endif

class   tftdynamic_t
{
  DrawQWidget*  pdraw;
  int           hoid;
  int           wriid;
  int           sloid;
  friend class  DrawQWidget;
  tftdynamic_t(DrawQWidget* _pdraw, int hid, int wid, int sid): pdraw(_pdraw), hoid(hid), wriid(wid), sloid(sid){}
public:
  tftdynamic_t(): pdraw(nullptr), hoid(-1), wriid(-1), sloid(-1) {}
  bool    attached() const { return pdraw != nullptr; }
  void    detach(){ pdraw = nullptr; }
  bool    valid() const { return pdraw != nullptr && sloid != -1; }
  
  tftdynamic_t(const tftdynamic_t& cpy): pdraw(cpy.pdraw), hoid(cpy.hoid), wriid(cpy.wriid), sloid(cpy.sloid) {}
  tftdynamic_t& operator=(const tftdynamic_t& cpy){ pdraw = cpy.pdraw; hoid = cpy.hoid; wriid = cpy.wriid; sloid = cpy.sloid; return *this; }
  bool    move(float fx, float fy);
  bool    move_x(float fx);
  bool    move_y(float fy);
  bool    rotate(float anglerad);
  bool    setopacity(float opacity);
  bool    switchto(int dsgid);
  bool    setup(int dsgid, float fx, float fy, float opacity=0);
public:
  int     width() const;
  int     height() const;
  QSize   size() const;
public:
  bool operator == (const tftdynamic_t& d2){ return pdraw == d2.pdraw && hoid == d2.hoid && wriid == d2.wriid && sloid == d2.sloid; }
  bool operator != (const tftdynamic_t& d2){ return !(*this == d2); }
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
    SF_PALETSAMPLER,
    _SF_COUNT
  };
  enum  {   MAX_OPTIONALS=32,  MAX_TEXTURES=96 };
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
  void    palettePrepare(const IPalette *ppal, bool discrete, int levels, float range0=0.0f, float range1=1.0f);
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
  virtual void leaveEvent(QEvent *event);
private:
  void  _applyMouseEvents(OVL_REACTION_MOUSE oreact, int x, int y);
  void  _applyMouseTracking(int x, int y);
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
  enum  { TFT_HOLDINGS=16, TFT_MAXAREA=32, 
          TFT_SLOTLIMIT_STATIC=512,
          TFT_SLOTLIMIT_DYNAMIC_CT=1024,
          TFT_SLOTLIMIT_DYNAMIC_RT=1024
        };
    
  struct  tftwriting_t
  {
    tftfrag_t     frag;
    bool          grouped;
  };
  
  struct  tftholding_t
  {
    tftgeterbook_t*           geterbook;
    bool                      geterbookowner;
    
    int                       _location_h;
    char                      _varname_h[64];
    int                       pinger_h, ponger_h;
    
    tftwriting_t              wri_statics_buffer[TFT_SLOTLIMIT_STATIC];     //  adding new static writing causes shader recompilation
    tftwriting_t              wri_dynamics_ct_buffer[TFT_SLOTLIMIT_DYNAMIC_CT];   //  adding new dynamic writing causes shader recompilation
    tftwriting_t              wri_dynamics_rt_buffer[TFT_SLOTLIMIT_DYNAMIC_RT];   //  adding new dynamic DONT causes shader recompilation
    
    struct wrilink_t
    {
      tftwriting_t*           array;
      unsigned int            count;
      unsigned int            limit;
    }                         wri_statics, wri_dynamics[_TFTD_COUNT];
    
    bool                      wri_grouping[_TFTR_COUNT];
    bool                      wri_enable_dynamic_RT;
      
    int                       _location_dt[_TFTD_COUNT], _location_ts;
    char                      _varname_dt[_TFTD_COUNT][64], _varname_ts[64];
    int                       pinger_dt[_TFTD_COUNT];
    int                       ponger_dt[_TFTD_COUNT];
    
    COORDINATION              wri_dynamic_rt_coordination;
    float                     wri_dynamic_rt_rotateangle;
  };
  
private:
  int             m_tfth_current;
  tftholding_t*   m_tfths[TFT_HOLDINGS];
  int             _tft_holding_alloc(tftgeterbook_t* th, bool isowner, bool enableDynamicRT);
  bool            _tft_holding_release(int idx);
  int             _tft_holding_design(tftholding_t*  holding, const char* text);
  int             _tft_holding_design(tftholding_t*  holding, int count, const char* texts[]);
private:
  int             _tft_add_writing(tftholding_t::wrilink_t*, const tftwriting_t&, bool grouped);
public:
  int             tftHoldingRegister(tftgeterbook_t* th, bool isowner, bool enableDynamicRT=TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  int             tftHoldingRegister(const QFont& font, int maxtextlen=TFT_TEXTMAXLEN, int limitcolumns=1, QColor color=QColor(0,0,0), bool enableDynamicRT=TFT_DEFAULT_ALLOW_DYNAMIC_RT);
  tftgeterbook_t* tftHoldingGetbook(int hoid);
  bool            tftHoldingGetbookOwner(int hoid);
  bool            tftHoldingSwitch(int hoid);
  int             tftHoldingRelease();
  void            tftHoldingSetColor(QColor clr);
  
                  /// Every new record has incremented index
  int             tftAddDesign(const char* text);
  int             tftAddDesigns(int count, const char* text[]);   // convert texts into records, returns first record id
public:
  QSize           tftHoldingDesignMaxSize() const;
  int             tftHoldingDesignMaxWidth() const;
  int             tftHoldingDesignMaxHeight() const;
public:
  ///  pushing new static writing causes shader recompilation
  tftstatic_t     tftPushStatic(int dsgid, COORDINATION cr, float fx, float fy, float rotate=0.0f);
  tftstatic_t     tftPushStatic(const char* text, COORDINATION cr, float fx, float fy, float rotate=0.0f);
  tftstatic_t     tftPushStatic(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy, float rotate=0.0f);
  tftstatic_t     tftPushStatic(const char* text, int ovlroot, COORDINATION cr, float fx, float fy, float rotate);
  void            tftEnableStaticClosestMode();
  void            tftDisableStaticClosestMode();
  
  ///  pushing new dynamic writing causes shader recompilation
                  /// FA - fixed angle, DA - rotateable
  tftdynamic_t    tftPushDynamicFA(int dsgid, COORDINATION cr, float fx, float fy);   // horizontal, unrotateable, more fast
  tftdynamic_t    tftPushDynamicFA(const char* text, COORDINATION cr, float fx, float fy);
  tftdynamic_t    tftPushDynamicFA(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy);   // horizontal, unrotateable, more fast
  tftdynamic_t    tftPushDynamicFA(const char* text, int ovlroot, COORDINATION cr, float fx, float fy);
  tftdynamic_t    tftPushDynamicDA(int dsgid, COORDINATION cr, float fx, float fy, float rotate);  // rotateable
  tftdynamic_t    tftPushDynamicDA(const char* text, COORDINATION cr, float fx, float fy, float rotate);
  tftdynamic_t    tftPushDynamicDA(int dsgid, int ovlroot, COORDINATION cr, float fx, float fy, float rotate);  // rotateable
  tftdynamic_t    tftPushDynamicDA(const char* text, int ovlroot, COORDINATION cr, float fx, float fy, float rotate);
  void            tftEnableDynamicClosestMode();    // special mode for fast draw for only 2 intersepting designs
  void            tftDisableDynamicClosestMode();
  
  ///  pushing new dynamic writings DONT causes shader recompilation
  void            tftSetHoldingDynamicRTCoordination(COORDINATION cr);
  COORDINATION    tftGetHoldingDynamicRTCoordination() const;
  void            tftSetHoldingDynamicRTRotateAngle(float angle);
  float           tfGetHoldingDynamicRTRotateAngle() const;
  tftdynamic_t    tftPushDynamicRT(int dsgid, float fx, float fy, float opacity=0.0f);
  tftdynamic_t    tftPushDynamicRT(const char* text, float fx, float fy, float opacity=0.0f);
  
  int             tftDesignsCount() const;
  int             tftDynamicsCTCount() const;
  int             tftDynamicsRTCount() const;
  
  tftdynamic_t    tftGetDynamicCT(int sloid);
  tftdynamic_t    tftGetDynamicRT(int sloid);
  int             tftDesignIndex(const tftdynamic_t&) const;
  int             tftDesignIndex(int sloid) const;
  const char*     tftText(const tftdynamic_t&) const;
  const char*     tftText(int sloid) const;
public:
  tftdynamic_t    tftDCT(int sloid);
  tftdynamic_t    tftDynamicCT(int sloid);
  bool            tftDynamicCTMove(int sloid, float fx, float fy);
  bool            tftDynamicCTMoveX(int sloid, float fx);
  bool            tftDynamicCTMoveY(int sloid, float fy);
  bool            tftDynamicCTRotate(int sloid, float anglerad);
  bool            tftDynamicCTOpacity(int sloid, float opacity);
  bool            tftDynamicCTSwitchTo(int sloid, int dsgid);
  bool            tftDynamicCTSetup(int sloid, int dsgid, float fx, float fy, float opacity=0.0f);
public:
  tftdynamic_t    tftDRT(int sloid);
  tftdynamic_t    tftDynamicRT(int sloid);
  bool            tftDynamicRTMove(int sloid, float fx, float fy);
  bool            tftDynamicRTMoveX(int sloid, float fx);
  bool            tftDynamicRTMoveY(int sloid, float fy);
//  bool            tftDynamicRTRotate(int sloid, float anglerad);
  bool            tftDynamicRTOpacity(int sloid, float opacity);
  bool            tftDynamicRTSwitchTo(int sloid, int dsgid);
  bool            tftDynamicRTSetup(int sloid, int dsgid, float fx, float fy, float opacity=0.0f);
public:
  int             tftGetDesignWidth(int dsgid) const;
  int             tftGetDesignHeight(int dsgid) const;
  QSize           tftGetDesignSize(int dsgid) const;
  
  int             tftGetDynamicWidth(int sloid) const;
  int             tftGetDynamicHeight(int sloid) const;
  QSize           tftGetDynamicSize(int sloid) const;
  
  int             tftGetDynamicRTWidth(int sloid) const;
  int             tftGetDynamicRTHeight(int sloid) const;
  QSize           tftGetDynamicRTSize(int sloid) const;
public: // tft operations
  bool            tftMove(const tftdynamic_t& d, float fx, float fy);
  bool            tftMoveX(const tftdynamic_t& d, float fx);
  bool            tftMoveY(const tftdynamic_t& d, float fy);
  bool            tftRotate(const tftdynamic_t& d, float anglerad);
  bool            tftSetOpacity(const tftdynamic_t& d, float opacity);
  bool            tftSwitchTo(const tftdynamic_t& d, int dsgid);
  bool            tftSetup(const tftdynamic_t& d, int dsgid, float fx, float fy, float opacity=0.0f);
public:
  int             tftGetWidth(const tftdynamic_t& d) const;
  int             tftGetHeight(const tftdynamic_t& d) const;
  QSize           tftGetSize(const tftdynamic_t& d) const;
};

class BSQClickerXY: public QObject, public DrawEventReactor
{
  Q_OBJECT
public:
  BSQClickerXY(QObject* parent=nullptr);
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  clicked(float x01, float y01);
};

class BSQClickerXY_WR: public QObject, public DrawEventReactor
{
  Q_OBJECT
public:
  BSQClickerXY_WR(QObject* parent=nullptr);
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


class BSQTrackerXY: public QObject, public DrawEventReactor
{
  Q_OBJECT
public:
  BSQTrackerXY(QObject* parent=nullptr);    // dont forget enable tracking by setMouseTracking(true)
  virtual bool  reactionTracking(class DrawQWidget*, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void  tracked(float x01, float y01, float xpix, float ypix);
};

////////////////////////////////////////

#endif // DRAWQWIDGET_H

