#ifndef DRAWCOREMAPEX_H
#define DRAWCOREMAPEX_H

#include "../core/bsqdraw.h"

class DrawMapEx: public DrawQWidget
{
  Q_OBJECT
protected:
  friend class DrawMapExPrivate;
  class DrawMapExPrivate*   pImpl;
public:
  DrawMapEx(const char* mappath, int map_x_size, int map_y_size, 
          unsigned int samplesA, unsigned int samplesB, ORIENTATION orient=OR_LRBT, SPLITPORTIONS splitPortions=SP_NONE);
  virtual     ~DrawMapEx();
public:
  void        setPaletteBounds(float pblow, float pbgaplow, float pbgaphigh, float pbhigh, bool update=true);
  void        paletteBounds(float* pblow, float* pbgaplow, float* pbgaphigh, float* pbhigh) const;
public:
  bool        loadTo(double lat, double lon);
  double      currentLat() const;
  double      currentLon() const;
  
  float       zoom() const;
  void        setZoom(float zoom, bool update = true);
  void        setZoomByRange(float range, bool update = true);
  
  bool        inCenter() const;
  void        goCenter();
  
  float       zeroLevel() const;
  void        setZeroLevel(float floor, bool update = true);
  
  void        setProflimitFloor(float value, bool update = true);
  void        setProflimitSurf(float value, bool update = true);
  void        setProflimit(float value_floor, float value_surf, bool update = true);
  void        setProflimitFloorAuto(bool update = true);
  void        setProflimitSurfAuto(bool update = true);
  void        setProflimitAuto(bool update = true);
public:
  float       metersInPixel() const;
  void        coordsLL(double* plat, double* plon) const;
  void        coordsLL(float x, float y, double* plat, double* plon) const;
  void        viewToMM(float x, float y);
  void        viewToMMRel(float dx, float dy);
  
  void        viewToLL(double lat, double lon, bool update = true);
  void        viewToLLRel(double dlat, double dlon, bool update = true);
  void        viewToRel(float dx, float dy, bool update = true);
  void        coordsLLOfViewCenter(double* plat, double* plon) const;
  void        coordsEEOfViewCenter(float* px, float* py) const;
  void        coordsOfViewCenter(float* px, float* py) const;
public:
  float       metersHorz() const { return metersInPixel()*sizeHorz(); }
  float       metersVert() const { return metersInPixel()*sizeVert(); }
  float       rectangularityVH() const { return float(sizeVert())/float(sizeHorz()); }
public:
  void        relpos(float* x01, float* y01) const;
public:
  float       depthByPIX(float xpix, float ypix, bool* valid=nullptr) const;
public:
  void        coordsOOByPix(float px, float py, float* pdx, float* pdy) const;    // OO - own, own
  float       distanceOO(float x1, float y1, float x2, float y2) const;    // OO - own, own
  float       distanceByTop() const;
  float       distanceByBottom() const;
  float       distanceByVert() const;
protected:
  virtual void            processGlLocation(int secidx, int secflags, int loc, int TEX);
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
  virtual int             sizeAndScaleChanged(bool changedDimmA, bool changedDimmB, bool changedScalingA, bool changedScalingB);
signals:
  void        sig_resized();
};


/////

class _MapReactorZoom: public QObject, public DrawEventReactor
{
  Q_OBJECT
  bool          doAppZoom;
  float         zoomLimits[2];
public:
  _MapReactorZoom(bool applyZoom, QObject* parent=nullptr);
  float         zoomClosest() const { return zoomLimits[0]; }
  float         zoomFarest() const { return zoomLimits[1]; }
protected:
  virtual bool  reactionWheel(class DrawQWidget* draw, OVL_REACTION_WHEEL orm, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void          zoomChanged(double zoom);
};

class MapExReactor: public _MapReactorZoom
{
  Q_OBJECT
  float         lx,ly;
public:
  MapExReactor(bool applyZoom, QObject* parent=nullptr);
protected:
  virtual bool  reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void          coordsChanged(double lat, double lon);
};

class MapExReactorSkol: public _MapReactorZoom
{
  Q_OBJECT
public:
  MapExReactorSkol(bool applyZoom, QObject* parent=nullptr);
protected:
  virtual bool  reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void          skolChanged(float x, float y, double lat, double lon);
};

class MapExReactorClickable: public _MapReactorZoom
{
  Q_OBJECT
  class QElapsedTimer*  qel;
  float         lx,ly;
public:
  MapExReactorClickable(bool applyZoom, QObject* parent=nullptr);
  ~MapExReactorClickable();
protected:
  virtual bool  reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void          clickedLM(float xpix, float ypix);
  void          doubleclicked(float xpix, float ypix);
  void          coordsChanged(double lat, double lon);
  void          actionRM(bool released, float xpix, float ypix);
};

#endif // DRAWCOREMAPEX_H
