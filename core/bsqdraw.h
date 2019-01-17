#ifndef DRAWCORESP_H
#define DRAWCORESP_H

#include "bsdraw.h"

#include <QtGlobal>
#include <QVector>
#if QT_VERSION >= 0x050000
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
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

class ISheiGenerator
{
public:
  virtual   const char*   shaderName() const =0;
  virtual   unsigned int  shvertex_pendingSize() const =0;
  virtual   unsigned int  shvertex_store(char* to) const =0;
  virtual   unsigned int  shfragment_pendingSize(unsigned int ovlscount) const =0;
  virtual   unsigned int  shfragment_store(const DPostmask&, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const =0;
  
public:
  enum      { PMT_PSEUDO2D, PMT_FORCE1D }; /// PORTION_MESH_TYPE
  virtual   int           portionMeshType() const =0;
  virtual   ~ISheiGenerator(){}
};

class QScrollBar;
class DrawQWidget: public QOpenGLWidget, protected QOpenGLFunctions, public DrawCore
{
  Q_OBJECT
  enum  SHEIFIELD  {  SF_DATA, SF_PALETTE, 
                      SF_DOMAIN, SF_PORTIONSIZE, 
                      SF_COUNTPORTIONS, SF_CHNL_HORZ, SF_CHNL_VERT, SF_CHNL_HORZSCALING, SF_CHNL_VERTSCALING,
  //                    SF_BOUNDS,
  //                    SF_CONTRAST,
                      _SF_COUNT
                   };
protected:
  char*                   m_vshmem, *m_fshmem;
  unsigned int            m_vshalloc, m_fshalloc;
  ISheiGenerator*         m_pcsh;
  int                     m_portionMeshType;
    
  int                     m_locations[_SF_COUNT];
  QOpenGLShaderProgram    m_ShaderProgram;
  float                   m_SurfaceVertex[8];
protected:
  unsigned int            m_matrixLmSize;
  bool                    m_sbStatic;
  int                     m_cttrLeft, m_cttrTop, m_cttrRight, m_cttrBottom;
protected:
  unsigned int            m_texAll[96];
  unsigned int            m_texOvlCount;
  enum  { HT_MATRIX=0, HT_PAL, HT_DMN, HT_OVERLAYSSTART };
public:
  DrawQWidget(ISheiGenerator* pcsh, unsigned int portions, ORIENTATION orient);
  ~DrawQWidget();
  
  void  connectScrollBar(QScrollBar*, bool staticView=false, bool setOrientation=true);
public slots:
//    void update();    /// inherited from QWidget!
//    void repaint();   /// inherited from QWidget!
  void    slot_setBoundLow(float);
  void    slot_setBoundHigh(float);
  void    slot_setContrast(float k, float b);
  void    slot_setDataPalette(const IPalette*);
  void    slot_setData(const float*);
  void    slot_setData(QVector<float>);
  void    slot_fillData(float);
  void    slot_clearData();
  
  void    slot_enableAutoUpdate(bool);
  void    slot_disableAutoUpdate(bool);
  void    slot_enableAutoUpdateByData(bool);
  void    slot_disableAutoUpdateByData(bool);
protected:
  void    palettePrepare(const IPalette *ppal, bool discrete, int levels);
  void    applyHardPendings();
  void    initializeGL();
  void    paintGL();
protected:
  virtual void innerUpdate(REDRAWBY);
  virtual void innerResize();
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
private:
  void  store_crd_clk(OVL_REACTION oreact, unsigned int x, unsigned int y);
  const char*   vardesc(SHEIFIELD);
protected slots:
  virtual void  slideLmHeight(int);
protected:
  class MemExpand2D
  {
  protected:
    unsigned int  pc, ps;
  protected:
    unsigned int  memoryLines;
    unsigned int  filled;
    unsigned int  current;
    float*        m_extendeddataarr;
  public:
    MemExpand2D(unsigned int portionsCount, unsigned int portionSize, unsigned int linesMemory);
    ~MemExpand2D();
  public:
    void  onSetData(const float* data);
    void  onSetData(const float* data, DataDecimator* decim);
    void  onClearData();
    bool  onFillData(int portion, int pos, float* rslt) const;
  };
protected:
  class MemExpand1D
  {
  protected:
    unsigned int  pc, ps, pm;
  protected:
    bool          rounded;
    unsigned int  current;
    float*        m_extendeddataarr;
  public:
    MemExpand1D(unsigned int portionsCount, unsigned int portionSize, unsigned int memorySize);
    ~MemExpand1D();
  public:
    void  onSetData(const float* data, unsigned int newsize);
    void  onSetData(const float* data, unsigned int newsize, DataDecimator* decim);
    void  onClearData();
    void  onFillData(int pos, float* rslt, float emptyfill) const;
  };
};


#endif // DRAWCORESP_H
