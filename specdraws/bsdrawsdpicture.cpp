/// This file contains special derived class for show images
/// DrawSDPicture with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawSDPicture with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
/// Created By: Elijah Vlasov
#include "bsdrawsdpicture.h"

#include "core/sheigen/bsshgenmain.h"

class SheiGeneratorSDP: public ISheiGenerator
{
public:
  enum MODE { MODE_NONE, MODE_MARKER63 };
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
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 300 + FshMainGenerator::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, 
                                         ORIENTATION orient, SPLITPORTIONS splitPortions, const impulsedata_t& imp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.push( "uniform highp sampler2D texGround;"
              "uniform highp int       countGround;" );
    fmg.main_begin(FshMainGenerator::INIT_BYVALUE, m_bckclr, orient, fsp);
    
    fmg.push(   "vec4 pixsdp = texture(texGround, relcoords).rgba;" );
    
    if (m_mode == MODE_NONE)
    {
      fmg.push(   "result = mix(result, pixsdp.bgr, pixsdp.a);"
//                  "float value = texture(texData, vec2(domain, 0.0)).r;"  // domain /float(countGround-1)
//                  "ovMix = max(ovMix, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step( value , post_mask[1]));"
                  );
    }
    else if (m_mode == MODE_MARKER63)
    {
      fmg.push(
                  "float marker = mod(pixsdp[2]*255.0, 4.0)*16.0 + mod(pixsdp[1]*255.0, 4.0)*4.0 + mod(pixsdp[0]*255.0, 4.0);"
                  "float value = texture(texData, vec2((marker - 1.0)/float(countGround-1), 0.0)).r;"
                  "vec3  mmc = texture(texPalette, vec2(value, 0.0)).rgb;"
                  "result = mix(mix(result, pixsdp.bgr, pixsdp.a), mmc, 1.0 - step(marker, 0.0));"
  //                "result = pixsdp.bgr;"
  //                "mixwell = pixsdp.a;"
                  "ovMix = max(ovMix, value);"
//                  "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));"
                  
  
                  );
    }
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorSDP::~SheiGeneratorSDP(){}



void DrawSDPicture::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = samplesVert;
  m_portionSize = SDPSIZE_MARKER;
  
  m_groundType = GND_SDP;
  m_groundData = m_pImage->bits();
  m_groundDataWidth = m_pImage->width();
  m_groundDataHeight = m_pImage->height();
  
  deployMemory();
}

DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, QImage* img, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(samplesHorz, samplesVert);
}


DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, const char* imagepath, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  QImage orig(imagepath);
  m_pImage = new QImage(orig.convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();

  reConstructor(samplesHorz, samplesVert);
}

DrawSDPicture::DrawSDPicture(QImage* img, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(qRound(m_pImage->width()*sizeMultiplier), qRound(m_pImage->height()*sizeMultiplier));
}

DrawSDPicture::DrawSDPicture(const char* imagepath, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(DATEX_DD, new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  QImage orig(imagepath);
  m_pImage = new QImage(orig.convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();

  reConstructor(qRound(m_pImage->width()*sizeMultiplier), qRound(m_pImage->height()*sizeMultiplier));
}

bool DrawSDPicture::isNull() const
{
  return m_pImage->isNull();
}

void DrawSDPicture::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);
  clampScaling(scalingA, scalingB);
}

unsigned int DrawSDPicture::colorBack() const
{
  unsigned int bc = ((SheiGeneratorSDP*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}




bool BSQMarkerSelector::overlayReactionMouse(DrawQWidget* qwdg, OVL_REACTION_MOUSE oreact, const void* dataptr, bool* doStop)
{
  if (oreact == m_action)
  {
    *doStop = true;
    int x = ((const float*)dataptr)[2], y = ((const float*)dataptr)[3];
    const int* dst = (const int*)((DrawSDPicture*)qwdg)->getImage()->scanLine(y);
    int marker = ((dst[x]>>16 & 0x3) << 4) | ((dst[x]>>8 & 0x3) << 2) | ((dst[x]& 0x3 << 0));
    if (m_emitEmptyMarker || (marker > 0 && marker <= SDPSIZE_MARKER))
      emit markerClicked(marker);
  }
  else if (oreact == m_drop)  emit markerDropped();
  return false;
}
