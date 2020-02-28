#include "bsimage.h"

#include "../core/sheigen/bsshgentrace.h"

#include <QImage>
#include <QApplication>
#include <QPainter>

#include <QDebug>

/////////////////////////////////////////////////////////


static bool g_bs_detach_image = false;

void bs_detachFutureImages(bool dt){  g_bs_detach_image = dt; }



IOverlaySimpleImage::IOverlaySimpleImage(QImage *image, IMAGECONVERT icvt, bool autorotated, bool detach): m_autorotated(autorotated)
{
  m_imageowner=false;
  switch (icvt)
  {
  case IC_AUTO:       m_pImage = new QImage(image->convertToFormat(QImage::Format_ARGB32));   m_imageowner = true; break;
  case IC_ASIS:       m_pImage = image; m_imageowner = false;  break;
  case IC_ASISOWNER:  m_pImage = image; m_imageowner = true;  break;
  case IC_BLOCKALPHA: m_pImage = new QImage(image->convertToFormat(QImage::Format_RGB888));   m_imageowner = true; break;
  }
  if (!m_pImage->isNull())
  {
    if (detach)
      m_pImage->detach();
//    qDebug()<<m_pImage->isDetached();
    m_dmti.w = m_pImage->width();
    m_dmti.h = m_pImage->height();
    m_dmti.data = m_pImage->constBits();
//    qDebug()<<m_pImage->format();
  }
  else
  {
    m_dmti.data = nullptr;
  }
  m_dtype = icvt == IC_BLOCKALPHA? DT_TEX : DT_TEXA;
}

IOverlaySimpleImage::~IOverlaySimpleImage()
{
  if (m_imageowner && m_pImage != nullptr)
    delete m_pImage;
}

void IOverlaySimpleImage::reUpdate()
{
  IOverlay::updatePublic();
//  IOverlay::updateParameter(false);
}

/////////////////////////////////////////////////////////

OImageOriginal::OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w, float mult_h): 
  IOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsDynamic(cn, x, y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(m_dtype, &m_dmti);
}

OImageOriginal::OImageOriginal(QImage *image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic *pcoords, float offset_x, float offset_y, float mult_w, float mult_h):
  IOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsDynamic(pcoords->getCoordination(), offset_x, offset_y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(m_dtype, &m_dmti);
}

int OImageOriginal::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push("_densvar = step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*(1.0 - step(float(idimms2.x), float(inormed.x)))*(1.0 - step(float(idimms2.y), float(inormed.y)));");
    ocg.push( m_autorotated? "vec2  tcoords = inormed/vec2(idimms2.x-1, idimms2.y-1);" : "vec2  tcoords = inormed/vec2(idimms2.x-1, idimms2.y-1);");
    ocg.push( "vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, 1.0 - tcoords.y));");
    ocg.push("result = pixel.rgb;"
             "mixwell = _densvar*pixel.a;");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

//////////////////////////////////////////////////////////////





OImageStretched::OImageStretched(QImage *image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w, float mult_h):
  IOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsStatic(cn, x, y), OVLDimms2Static(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(m_dtype, &m_dmti);
}

OImageStretched::OImageStretched(QImage *image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic *pcoords, float offset_x, float offset_y, float mult_w, float mult_h):
  IOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsStatic(pcoords->getCoordination(), offset_x, offset_y), OVLDimms2Static(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(m_dtype, &m_dmti);
}

int OImageStretched::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push("_densvar = 1.0;");
    ocg.push( m_autorotated? "vec2 relcoords = coords.pq;" : "vec2 relcoords = coords.st;");
    ocg.push( "vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(relcoords.x, 1.0 - relcoords.y));");
    ocg.push("result = pixel.rgb;"
             "mixwell = _densvar*pixel.a;");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}
