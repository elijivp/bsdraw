#ifndef DRAWCORESDP_H
#define DRAWCORESDP_H

/// This file contains special derived class for show images
/// DrawSDPicture with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawSDPicture with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
///   You must especially prepare image (paint him by markers) before use
/// 
/// Example:
/// #include "specdraws/bsdrawsdpicture.h"
/// 
/// DrawSDPicture* draw = new DrawSDPicture(SAMPLES, LINES, "path_to_picture.png");
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"

class QImage;

enum { SDPSIZE_NONE =0, SDPSIZE_MARKER=63 };

/// Dont Forget: setData accept pointer to 63 floats
class DrawSDPicture: public DrawQWidget
{
  QImage*       m_pImage;
  bool          m_allowNoscaledResize;
protected:
  void*         m_sdpData;
  unsigned int  m_sdpDataWidth, m_sdpDataHeight;
  bool          m_sdpMipMapping;
protected:
  void  reConstructor(unsigned int samplesHorz, unsigned int samplesVert);
public:
  DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, QImage* img, unsigned int backgroundColor=0x00000000, bool allowNoscaledResize=true);
  DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, const char* imagepath, unsigned int backgroundColor=0x00000000, bool allowNoscaledResize=true);
  
  DrawSDPicture(QImage* img, float sizeMultiplier=1.0f, unsigned int backgroundColor=0x00000000);
  DrawSDPicture(const char* imagepath, float sizeMultiplier=1.0f, unsigned int backgroundColor=0x00000000);
  ~DrawSDPicture();
  QImage*           getImage(){ return m_pImage; }
  const QImage*     getImage() const { return m_pImage; }
  
  bool              isNull() const;
  void              setMipMapping(bool v);
  bool              mipMapping() const;
protected:
  virtual void            processGlLocation(int secidx, int secflags, int loc, int TEX);
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
  virtual unsigned int    colorBack() const;
};

class BSQMarkerSelector: public BSQProactiveSelectorBase
{
  Q_OBJECT
  bool  m_emitEmptyMarker;
public:
  BSQMarkerSelector(bool emitEmptyMarker, OVL_REACTION_MOUSE action=ORM_LMPRESS, OVL_REACTION_MOUSE drop=ORM_RMPRESS): 
    BSQProactiveSelectorBase(action, drop), m_emitEmptyMarker(emitEmptyMarker) {}
  virtual bool  reactionMouse(DrawQWidget*, OVL_REACTION_MOUSE, const coordstriumv_t*, bool* /*doStop*/);
signals:
  void  markerClicked(int);
  void  markerDropped();
};

#endif // DRAWCORESDP_H
