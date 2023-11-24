#ifndef DRAWCOREMAP_H
#define DRAWCOREMAP_H

/// DrawMap 
/// Input: 2D array [samplesVert]x[samplesHorz] ...
/// 
/// Example:
/// #include "bsdrawmap.h"
/// #include "palettes/bspalettes_std.h"
/// 
/// 
/// draw->setDataPalette(&paletteBkGrWh);
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"

class DrawMap: public DrawQWidget
{
  Q_OBJECT
protected:
  friend class DrawMapPrivate;
  class DrawMapPrivate*   pImpl;
public:
  DrawMap(const char* mappath, int map_x_size, int map_y_size, 
          unsigned int samplesA, unsigned int samplesB, ORIENTATION orient=OR_LRBT, SPLITPORTIONS splitPortions=SP_NONE);
  virtual     ~DrawMap();
public:
  void        setPaletteBounds(float pblow, float pbgaplow, float pbgaphigh, float pbhigh, bool update=true);
  void        paletteBounds(float* pblow, float* pbgaplow, float* pbgaphigh, float* pbhigh) const;
public:
  bool        loadTo(double lat, double lon);
  
  float       zoom() const;
  void        setZoom(float zoom, bool update = true);
  
  float       zeroLevel() const;
  void        setZeroLevel(float floor, bool update = true);
public:
  void        viewToLL(double lat, double lon, bool update = true);
  void        viewToLLRel(double dlat, double dlon, bool update = true);
  void        viewToRel(float dx, float dy, bool update = true);
  void        coordsLLOfViewCenter(double* plat, double* plon) const;
  void        coordsEEOfViewCenter(float* px, float* py) const;
  void        coordsOfViewCenter(float* px, float* py) const;
public:
  void        coordsOOByPix(float px, float py, float* pdx, float* pdy) const;        // OO - own, own
  float       distanceOO(float x1, float y1, float x2, float y2) const;     // OO - own, own
  float       distanceByTop() const;
  float       distanceByBottom() const;
protected:
  virtual void            processGlLocation(int secidx, int secflags, int loc, int TEX);
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};


class MapReactorMove: public QObject, public DrawEventReactor
{
  Q_OBJECT
  float         lx,ly;
public:
  MapReactorMove(QObject* parent=nullptr);
protected:
  virtual bool  reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool* /*doStop*/);
  virtual bool  reactionWheel(class DrawQWidget* draw, OVL_REACTION_WHEEL orm, const coordstriumv_t* ct, bool* /*doStop*/);
signals:
  void          coordsChanged(double lat, double lon);
  void          zoomChanged(double zoom);
};

#endif // DRAWCOREMAP_H
