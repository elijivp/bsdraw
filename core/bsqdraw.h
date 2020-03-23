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

class QScrollBar;
class DrawQWidget: public QOpenGLWidget, protected QOpenGLFunctions, public DrawCore
{
  Q_OBJECT
  enum  SHEIFIELD  {  SF_DATA, SF_PALETTE, 
                      SF_DOMAIN, SF_GROUND=SF_DOMAIN, SF_PORTIONSIZE, 
                      SF_COUNTPORTIONS, SF_DIMM_A, SF_DIMM_B, SF_CHNL_SCALING_A, SF_CHNL_SCALING_B,
  //                    SF_BOUNDS,
  //                    SF_CONTRAST,
                      _SF_COUNT
                   };
protected:
  bool                    m_compileOnInitializeGL; /// true by default
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
  enum  { HT_MATRIX=0, HT_PAL, HT_GND, HT_OVERLAYSSTART };
public:
  DrawQWidget(ISheiGenerator* pcsh, unsigned int portions, ORIENTATION orient);
  ~DrawQWidget();
  
  void  compileWhenInitializeGL(bool cflag);
  void  connectScrollBar(QScrollBar*, bool staticView=false, bool setOrientation=true);
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
  void    slot_setDataPalette(const IPalette*);
  void    slot_setDataPaletteDiscretion(bool);
  void    slot_setData(const float*);
  void    slot_setData(QVector<float>);
  void    slot_fillData(float);
  void    slot_clearData();
  
  void    slot_setMirroredHorz();
  void    slot_setMirroredVert();
  void    slot_setPortionsCount(int count);
  
  void    slot_enableAutoUpdate(bool);
  void    slot_disableAutoUpdate(bool);
  void    slot_enableAutoUpdateByData(bool);
  void    slot_disableAutoUpdateByData(bool);
protected:
  void    palettePrepare(const IPalette *ppal, bool discrete, int levels);
  void    initCollectAndCompileShader();
  void    initializeGL();
  void    paintGL();
  void    resizeGL(int w, int h);
protected:
  virtual void callWidgetUpdate();
  virtual void innerUpdateGeometry();
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
//  virtual void resizeEvent(QResizeEvent *event);
  
  virtual void keyPressEvent(QKeyEvent *event);
private:
  void  store_crd_clk(OVL_REACTION_MOUSE oreact, int x, int y);
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

