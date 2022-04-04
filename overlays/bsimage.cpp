/// Overlays:   images through QImage
///   OImageOriginal. View: image with fixed size
///   OImageStretched. View: image with size depended of draw
/// Created By: Elijah Vlasov
#include "bsimage.h"

#include "../core/sheigen/bsshgentrace.h"

#include <QImage>
#include <QApplication>
#include <QPainter>

//#include <QDebug>

/////////////////////////////////////////////////////////


static bool g_bs_detach_image = false;

void bs_detachFutureImagesInConstructors(bool dt){  g_bs_detach_image = dt; }



OVLQImage::OVLQImage(QImage *image, IMAGECONVERT icvt, bool autorotated, bool detach):
  m_pImage(nullptr), m_autorotated(autorotated), m_imageowner(false), m_banalpha(false)
{
  assignImage(image, icvt, autorotated, detach);
}

OVLQImage::~OVLQImage()
{
  if (m_imageowner && m_pImage != nullptr)
    delete m_pImage;
}

void OVLQImage::banAlphaChannel(bool ban)
{
  m_banalpha = ban;
}

bool OVLQImage::isAlphaBanned() const
{
  return m_banalpha;
}

bool  OVLQImage::assignImage(QImage* image, IMAGECONVERT icvt, bool autorotated, bool detach)
{
  if (image != nullptr && image->isNull() == false)
  {
    if (m_imageowner && m_pImage != nullptr)
      delete m_pImage;
    
    m_imageowner = false;
    m_autorotated = autorotated;
    
    switch (icvt)
    {
    case IC_AUTO:       m_pImage = new QImage(image->convertToFormat(QImage::Format_ARGB32));   m_imageowner = true; break;
    case IC_ASIS:       m_pImage = image; m_imageowner = false;  break;
    case IC_ASISOWNER:  m_pImage = image; m_imageowner = true;  break;
    case IC_BLOCKALPHA: m_pImage = new QImage(image->convertToFormat(QImage::Format_RGB888));   m_imageowner = true; break;
//    default: return false;
    }
    if (!m_pImage->isNull())
    {
      if (detach)
        m_pImage->detach();
//      qDebug()<<detach<<"??"<<m_pImage->isDetached();
      
      m_dmti.type = icvt == IC_BLOCKALPHA? dmtype_image_t::RGB : dmtype_image_t::RGBA;
      m_dmti.w = m_pImage->width();
      m_dmti.h = m_pImage->height();
      m_dmti.data = m_pImage->constBits();
  //    qDebug()<<m_pImage->format();
    }
    else
    {
      m_dmti.data = nullptr;
      Q_ASSERT(m_pImage->isNull() == false);
    }
  }
  else
  {
    m_dmti.w = 1;
    m_dmti.h = 1;
    m_dmti.data = nullptr;
    m_dmti.type = dmtype_image_t::NONE;
    return false;
  }
  return true;
}



void DrawOverlaySimpleImage::reUpdate()
{
  updateParameter(false, true);
}

bool  DrawOverlaySimpleImage::setImage(QImage* image, OVLQImage::IMAGECONVERT icvt, bool autorotated, bool detach, bool update)
{
  bool result = assignImage(image, icvt, autorotated, detach);
//  updateParameter(true, update);
  updateParameter(false, update);
  return result;
}


/////////////////////////////////////////////////////////

OImageOriginal::OImageOriginal(QImage* image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w, float mult_h): 
  DrawOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsDynamic(cn, x, y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(DT_TEXTURE, &m_dmti);
}

OImageOriginal::OImageOriginal(QImage *image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic *pcoords, float offset_x, float offset_y, float mult_w, float mult_h):
  DrawOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsDynamic(pcoords->getCoordination(), offset_x, offset_y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(DT_TEXTURE, &m_dmti);
}

int OImageOriginal::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push("_fvar = step(0.0,float(inormed.x))*step(0.0,float(inormed.y))*step(float(inormed.x), float(idimms2.x - 1))*step(float(inormed.y), float(idimms2.y - 1));");
//    ocg.push( m_autorotated? "vec2  tcoords = inormed/vec2(idimms2.x, idimms2.y);" : "vec2  tcoords = vec2(inormed.x, idimms2.y - 1 - inormed.y) /vec2(idimms2.x, idimms2.y);");
    ocg.push( m_autorotated? 
                "vec2  tcoords = inormed/vec2(idimms2.x - 1, idimms2.y - 1);" :     // yep, -1
                "vec2  tcoords = vec2(inormed.x, idimms2.y - 1 - inormed.y) /vec2(idimms2.x - 1, idimms2.y - 1);" // yep, -1
                );
    ocg.push( "vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(tcoords.x, 1.0 - tcoords.y));");
    m_dmti.type == dmtype_image_t::RGB? ocg.push("result = pixel.rgb;") : ocg.push("result = pixel.bgr;");
    ocg.push(m_banalpha? "mixwell = _fvar;" : "mixwell = _fvar*pixel.a;");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

//////////////////////////////////////////////////////////////





OImageStretched::OImageStretched(QImage *image, IMAGECONVERT icvt, bool autorotated, COORDINATION cn, float x, float y, float mult_w, float mult_h):
  DrawOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsStatic(cn, x, y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(DT_TEXTURE, &m_dmti);
}

OImageStretched::OImageStretched(QImage *image, IMAGECONVERT icvt, bool autorotated, OVLCoordsStatic *pcoords, float offset_x, float offset_y, float mult_w, float mult_h):
  DrawOverlaySimpleImage(image, icvt, autorotated, g_bs_detach_image), OVLCoordsStatic(pcoords->getCoordination(), offset_x, offset_y), OVLDimms2Dynamic(CR_ABSOLUTE_NOSCALED, 0.0f, 0.0f)
{
  m_sides.w = m_dmti.w*mult_w;
  m_sides.h = m_dmti.h*mult_h;
  appendUniform(DT_TEXTURE, &m_dmti);
}

int OImageStretched::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.goto_normed();
    ocg.push("_fvar = 1.0;");
    ocg.push( m_autorotated? "vec2 relcoords = coords.pq;" : "vec2 relcoords = coords.st;");
    ocg.push( "vec4 pixel = texture(");  ocg.param_get(); ocg.push(", vec2(relcoords.x, 1.0 - relcoords.y));");
    m_dmti.type == dmtype_image_t::RGB? ocg.push("result = pixel.rgb;") : ocg.push("result = pixel.bgr;");
    ocg.push(m_banalpha? "mixwell = _fvar;" : "mixwell = _fvar*pixel.a;");
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

