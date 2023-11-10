/// This file contains special derived class for show images
/// DrawSDPicture with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawSDPicture with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
/// Created By: Elijah Vlasov
#include "bsdrawsdpicture.h"

#include "core/sheigen/bsshgenmain.h"

class SheiGeneratorSDP: public ISheiGenerator
{
public:
  enum MODE { MODE_NONE, MODE_MARKER };
  MODE          m_mode;
  unsigned int  m_bckclr;
public:
  SheiGeneratorSDP(MODE mode, unsigned int backgroundColor): m_mode(mode), m_bckclr(backgroundColor){}
  ~SheiGeneratorSDP();
public:
  virtual const char*   shaderName() const {  return "SDP"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 300 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         ovlfraginfo_t ovlsinfo[], unsigned int ovlscount,
                                         locbackinfo_t locbackinfo[], unsigned int* locbackcount,
                                         char* to) const
  {
    globvarinfo_t globvars[] = {  { DT_SAMP4, "sdpsampler" }  };
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, sizeof(globvars)/sizeof(globvars[0]), globvars, ovlscount, ovlsinfo);
    fmg.getLocbacks(locbackinfo, locbackcount);
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, m_bckclr, orient, fsp);
    
    fmg.push(   "vec4 pixsdp = texture(sdpsampler, abc_coords).rgba;" );
    
    if (m_mode == MODE_NONE)
    {
      fmg.push(   "result = mix(result, pixsdp.bgr, pixsdp.a);"
//                  "float value = texture(datasampler, vec2(domain, 0.0)).r;"  // domain /float(lenground-1)
//                  "dvalue = max(dvalue, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step( value , post_mask[1]));"
                  );
    }
    else if (m_mode == MODE_MARKER)
    {
      fmg.push(
                  "float marker = mod(pixsdp[2]*255.0, 4.0)*16.0 + mod(pixsdp[1]*255.0, 4.0)*4.0 + mod(pixsdp[0]*255.0, 4.0);"
                  "float value = texture(datasampler, vec2((marker - 1.0)/float(dataportionsize-1), 0.0)).r;"
                  "value = paletrange[0] + (paletrange[1] - paletrange[0])*value;"
                  "vec3  mmc = texture(paletsampler, vec2(value, 0.0)).rgb;"
                  "result = mix(mix(result, pixsdp.bgr, pixsdp.a), mmc, 1.0 - step(marker, 0.0));"
  //                "result = pixsdp.bgr;"
  //                "mixwell = pixsdp.a;"
                  "dvalue = max(dvalue, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));"
                  
  
                  );
    }
    fmg.main_end(fsp, fspopacity);
    
    qDebug()<<to;
    return fmg.written();
  }
};
SheiGeneratorSDP::~SheiGeneratorSDP(){}



void DrawSDPicture::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = samplesVert;
  m_portionSize = SDPSIZE_MARKER;
  
  m_sdpData = m_pImage->bits();
  m_sdpDataWidth = m_pImage->width();
  m_sdpDataHeight = m_pImage->height();
  m_sdpMipMapping = false;
  
  deployMemory();
}

DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, QImage* img, unsigned int backgroundColor, bool allowNoscaledResize):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER, backgroundColor), 1, OR_LRTB),
  m_allowNoscaledResize(allowNoscaledResize)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(samplesHorz, samplesVert);
}


DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, const char* imagepath, unsigned int backgroundColor, bool allowNoscaledResize):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER, backgroundColor), 1, OR_LRTB), 
  m_allowNoscaledResize(allowNoscaledResize)
{
  QImage orig(imagepath);
  m_pImage = new QImage(orig.convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();

  reConstructor(samplesHorz, samplesVert);
}

DrawSDPicture::DrawSDPicture(QImage* img, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER, backgroundColor), 1, OR_LRTB)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(qRound(m_pImage->width()*sizeMultiplier), qRound(m_pImage->height()*sizeMultiplier));
}

DrawSDPicture::DrawSDPicture(const char* imagepath, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER, backgroundColor), 1, OR_LRTB)
{
  QImage orig(imagepath);
  m_pImage = new QImage(orig.convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();

  reConstructor(qRound(m_pImage->width()*sizeMultiplier), qRound(m_pImage->height()*sizeMultiplier));
}

DrawSDPicture::~DrawSDPicture()
{
  delete m_pImage;
}

bool DrawSDPicture::isNull() const
{
  return m_pImage->isNull();
}

void  DrawSDPicture::setMipMapping(bool v)
{ 
  m_sdpMipMapping = v;
  DrawQWidget::vmanUpSec();
}
bool  DrawSDPicture::mipMapping() const
{
  return m_sdpMipMapping;
}


void DrawSDPicture::processGlLocation(int /*secidx*/, int /*secflags*/, int loc, int TEX)
{
  glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, m_sdpDataWidth, m_sdpDataHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_sdpData);
//    if (m_sdpMipMapping) glGenerateMipmap( GL_TEXTURE_2D );
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_sdpMipMapping? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
  glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
  
  glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
  
  m_ShaderProgram.setUniformValue(loc, TEX);
}

void DrawSDPicture::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  if (m_allowNoscaledResize)
  {
    *matrixDimmA = sizeA / m_scalingA;
    *matrixDimmB = sizeB / m_scalingB;
    *scalingA = m_scalingA;
    *scalingB = m_scalingB;
  }
  else
  {
    *matrixDimmA = m_dataDimmA;
    *matrixDimmB = m_dataDimmB;
    *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
    *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  }
  clampScaling(scalingA, scalingB);
}

unsigned int DrawSDPicture::colorBack() const
{
  unsigned int bc = ((SheiGeneratorSDP*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}




bool BSQMarkerSelector::reactionMouse(DrawQWidget* qwdg, OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
    int x = ct->fx_pix, y = ct->fy_pix;
    const int* dst = (const int*)((DrawSDPicture*)qwdg)->getImage()->scanLine(y);
    int marker = ((dst[x]>>16 & 0x3) << 4) | ((dst[x]>>8 & 0x3) << 2) | ((dst[x]& 0x3 << 0));
    if (m_emitEmptyMarker || (marker > 0 && marker <= SDPSIZE_MARKER))
      emit markerClicked(marker);
  }
  else if (oreact == m_drop)  emit markerDropped();
  return false;
}
