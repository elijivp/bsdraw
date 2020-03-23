#include "bsdrawsdpicture.h"

#include "core/sheigen/bsshgenmain.h"

class SheiGeneratorSDP: public ISheiGenerator
{
public:
  enum MODE { MODE_NONE, MODE_MARKER63 };
private:
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
  virtual unsigned int  shfragment_pendingSize(unsigned int ovlscount) const { return FshMainGenerator::basePendingSize(ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int /*allocatedPortions*/, const DPostmask& fsp, ORIENTATION orient, unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, orient, ovlscount, ovlsinfo);
    fmg.push( "uniform highp sampler2D texGround;"
              "uniform highp int       countGround;" );
    fmg.goto_func_begin(FshMainGenerator::INIT_BYVALUE, m_bckclr, fsp);
    
    fmg.push(   "vec4 pixsdp = texture(texGround, relcoords).rgba;" );
    
    if (m_mode == MODE_NONE)
    {
      fmg.push(   "result = mix(result, pixsdp.bgr, pixsdp.a);"
//                  "float value = texture(texData, vec2(domain, 0.0)).r;"  // domain /float(countGround-1)
//                  "ovMix = max(ovMix, value);"
//                  "ppb_sfp[0] = mix(1.0, ppb_sfp[0], step( value , ppb_sfp[1]));"
                  );
    }
    else if (m_mode == MODE_MARKER63)
    {
      fmg.push(
                  "float marker = mod(pixsdp[2]*255.0, 4.0)*9.0 + mod(pixsdp[1]*255.0, 4.0)*3.0 + mod(pixsdp[0]*255.0, 4.0);"
                  "float value = texture(texData, vec2((marker - 1.0)/float(countGround-1), 0.0)).r;"
                  "vec3  mmc = texture(texPalette, vec2(value, 0.0)).rgb;"
                  "result = mix(mix(result, pixsdp.bgr, pixsdp.a), mmc, 1.0 - step(marker, 0.0));"
  //                "result = pixsdp.bgr;"
  //                "mixwell = pixsdp.a;"
                  "ovMix = max(ovMix, value);"
//                  "ppb_sfp[0] = mix(1.0, ppb_sfp[0], step(value, ppb_sfp[1]));"
                  
  
                  );
    }
    fmg.goto_func_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorSDP::~SheiGeneratorSDP(){}



void DrawSDPicture::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_matrixDimmA = samplesHorz;
  m_matrixDimmB = samplesVert;
  m_portionSize = 63;
  
  m_groundType = GND_SDP;
  m_groundData = m_pImage->bits();
  m_groundDataWidth = m_pImage->width();
  m_groundDataHeight = m_pImage->height();
  
  deployMemory();
}

DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, QImage* img, unsigned int backgroundColor):
  DrawQWidget(new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(samplesHorz, samplesVert);
}


DrawSDPicture::DrawSDPicture(unsigned int samplesHorz, unsigned int samplesVert, const char* imagepath, unsigned int backgroundColor):
  DrawQWidget(new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  QImage orig(imagepath);
  m_pImage = new QImage(orig.convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();

  reConstructor(samplesHorz, samplesVert);
}

DrawSDPicture::DrawSDPicture(QImage* img, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
{
  m_pImage = new QImage(img->convertToFormat(QImage::Format_ARGB32));
  m_pImage->detach();
  reConstructor(qRound(m_pImage->width()*sizeMultiplier), qRound(m_pImage->height()*sizeMultiplier));
}

DrawSDPicture::DrawSDPicture(const char* imagepath, float sizeMultiplier, unsigned int backgroundColor):
  DrawQWidget(new SheiGeneratorSDP(SheiGeneratorSDP::MODE_MARKER63, backgroundColor), 1, OR_LRTB)
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

void DrawSDPicture::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB)
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmA? 1 : (sizeA / m_matrixDimmA);
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);
  clampScaling(scalingA, scalingB);
}
