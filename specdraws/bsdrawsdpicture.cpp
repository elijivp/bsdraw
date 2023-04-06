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
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.push( "uniform highp sampler2D texground;"
              "uniform highp int       lenground;" );
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, m_bckclr, orient, fsp);
    
    fmg.push(   "vec4 pixsdp = texture(texground, abc_coords).rgba;" );
    
    if (m_mode == MODE_NONE)
    {
      fmg.push(   "result = mix(result, pixsdp.bgr, pixsdp.a);"
//                  "float value = texture(texdata, vec2(domain, 0.0)).r;"  // domain /float(lenground-1)
//                  "dvalue = max(dvalue, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step( value , post_mask[1]));"
                  );
    }
    else if (m_mode == MODE_MARKER)
    {
      fmg.push(
                  "float marker = mod(pixsdp[2]*255.0, 4.0)*16.0 + mod(pixsdp[1]*255.0, 4.0)*4.0 + mod(pixsdp[0]*255.0, 4.0);"
                  "float value = texture(texdata, vec2((marker - 1.0)/float(lenground-1), 0.0)).r;"
                  "value = palrange[0] + (palrange[1] - palrange[0])*value;"
                  "vec3  mmc = texture(texpalette, vec2(value, 0.0)).rgb;"
                  "result = mix(mix(result, pixsdp.bgr, pixsdp.a), mmc, 1.0 - step(marker, 0.0));"
  //                "result = pixsdp.bgr;"
  //                "mixwell = pixsdp.a;"
                  "dvalue = max(dvalue, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));"
                  
  
                  );
    }
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};
SheiGeneratorSDP::~SheiGeneratorSDP(){}



void DrawSDPicture::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_dataDimmA = samplesHorz;
  m_dataDimmB = samplesVert;
  m_portionSize = SDPSIZE_MARKER;
  
  m_groundType = GND_SDP;
  m_groundData = m_pImage->bits();
  m_groundDataWidth = m_pImage->width();
  m_groundDataHeight = m_pImage->height();
  
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
