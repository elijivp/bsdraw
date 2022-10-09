/// This file contains special derived class for show images
/// DrawPolar 
/// DrawPolar 
/// Created By: Elijah Vlasov
#include "bsdrawpolar.h"
#include "core/sheigen/bsshgenmain.h"

#include <QPainter>
#include <QtMath>

class SheiGeneratorPolar: public ISheiGenerator
{
public:
  unsigned int  m_samplesHorz, m_samplesVert;
  unsigned int  m_bckclr;
public:
  SheiGeneratorPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int backgroundColor): 
    m_samplesHorz(samplesHorz), m_samplesVert(samplesVert), m_bckclr(backgroundColor){}
  ~SheiGeneratorPolar();
public:
  virtual const char*   shaderName() const {  return "POLAR"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 1800 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.push("uniform highp float viewturn;");
    fmg.main_begin(FshDrawConstructor::INIT_BYVALUE, m_bckclr, orient, fsp, m_samplesHorz, m_samplesVert);
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);

    fmg.push("vec3 mpi = vec3(3.14159265359, 1.57079632679, 6.28318530718);");
    fmg.push("vec2 datacoords = (abc_coords - vec2(0.5, 0.5))*2;");
    
    fmg.push("vec4 lenarcscal = vec4(length(datacoords), mpi[1] + atan(datacoords.x, datacoords.y) + mpi[2]*(1.0-step(0.0, datacoords.x))*(1.0 - step(0.0, datacoords.y)), 0.0, 0.0);"
             "lenarcscal[1] = mod(lenarcscal[1] + viewturn*mpi[2], mpi[2]);"
             "dbounds.x = (mpi[2]*lenarcscal[0]*dbounds_noscaled.y*ab_iscaler.y);"
             );
    
    fmg.push("lenarcscal[2] = max(dbounds.x/dbounds_noscaled.x, 1.0);"
             "lenarcscal[3] = lenarcscal[1]*lenarcscal[0]*dbounds.y*ab_iscaler.y;"
             "lenarcscal[3] = lenarcscal[3] - floor(lenarcscal[3]/lenarcscal[2])*lenarcscal[2];"
//             "float tempo = 0.0*min(abs(datacoords.x/datacoords.y), abs(datacoords.y/datacoords.x));"
//             "float tempo = 0.5*max(0.0, 1 - tan((mpi[1]/2.0 - abs(atan(datacoords.x, datacoords.y)))) );"
//             "float tempo = 0.5*1.0/(1.0 + 2*tan(abs(mpi[1]/2.0 - abs(atan(datacoords.x, datacoords.y)))) );"
             "float tempo = 0.0;"
             
             "immod = ivec2( int( lenarcscal[3]*(1.0 + tempo) ), int(mod(lenarcscal[0]*dbounds.y, float(ab_iscaler.y))) );"
             "imrect = ivec4(immod.x, immod.y, lenarcscal[2]-1, ab_iscaler.y-1);"
             );

    
    fmg.push("datacoords = vec2(lenarcscal[1]/mpi[2], lenarcscal[0]);"
             "lenarcscal[2] = step(lenarcscal[0], 1.0);"
             );
    
    fmg.push( splitPortions == SP_NONE? "for (int i=0; i<portions; i++)" : "int i = explicitPortion;" );
    fmg.push( "{" );
    {
      fmg.value2D("float value", "datacoords");
      fmg.push("value = palrange[0] + (palrange[1] - palrange[0])*value;");
      fmg.push("dvalue = max(dvalue, value);");
      
      if ( splitPortions == SP_NONE )
        fmg.push( "result = mix(result, result + texture(texpalette, vec2(value, float(i)/(allocatedPortions-1) )).rgb, lenarcscal[2]);"
//                  "result = mix(result, vec3(0), step(float(immod.x), 3.0)*step(3.0, float(immod.x)));"
                  );
      else
        fmg.push( "result = mix(result, texture(texpalette, vec2(value, 0.0)).rgb, (1.0 - step(countPortions, float(explicitPortion)))*lenarcscal[2]  );" );

//      fmg.push("post_mask[0] = mix(post_mask[0], 1.0, (1.0 - step(value, post_mask[1]))*lenarcscal[2] );" );
      fmg.push("post_mask[0] = mix(post_mask[0], 1.0, (1.0 - step(value, post_mask[1]))*lenarcscal[2] );"
               "post_mask[3] = mix(post_mask[3], 1.0, (1.0 - step(value, post_mask[1]))*lenarcscal[2] );"
               );
//      fmg.push("post_mask[0] = mix(0.0, post_mask[0], lenarcscal[2] );" );
    }
    fmg.push( "}" );
    fmg.push("post_mask[3] = mix(0.0, post_mask[3], lenarcscal[2]);"
             );
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorPolar::~SheiGeneratorPolar(){}


//#if QT_VERSION >= 0x050000
//#define QTCOMPAT_FORMAT QImage::Format_Alpha8
//#elif QT_VERSION >= 0x040000
//#define QTCOMPAT_FORMAT QImage::Format_ARGB32
//#endif

struct drawscaleelem_t
{
  QImage*   img;
  float     x,y;
  unsigned int w,h;
  float     angle;
};

#ifdef DEGRATOR_IS_SHIT

class Degrator
{
  unsigned int  eh;
  unsigned int  maxw;
  enum  { MAXTEX = 10, COUNT = 360 };
  
  struct texcolumn_t
  {
    QImage*       tex;
    unsigned char ew;
    int           p0, p1;
  } tc[MAXTEX];
  int tccount;
public:
  Degrator(const QFont& fnt)
  {
    QFontMetrics fm(fnt);
    eh = 0;
    
    int power0 = 0, power1 = 10;
//    char powstr[MAXTEX + 1] = "*";
    char powstr[MAXTEX + 1] = "'";
    char* ppowstr = &powstr[1];
    for (tccount=0; tccount<MAXTEX; tccount++)
    {
      if (power0 >= COUNT)  break;
      if (power1 > COUNT)   power1 = COUNT;
      
      tc[tccount].p0 = power0;
      tc[tccount].p1 = power1;
      
      *ppowstr++ = '3';
      *ppowstr = '\0';
      QSize fmsize = fm.size(0, powstr, 0);
      eh = fmsize.height();
      tc[tccount].ew = fmsize.width();
      tc[tccount].tex = new QImage(fmsize.width(), (power1-power0)*eh, QImage::Format_ARGB32);
      {
        tc[tccount].tex->fill(Qt::transparent);
        QPainter ptr(tc[tccount].tex);
        ptr.setFont(fnt);
        for (int i=power0; i<power1; i++)
          ptr.drawText(QRect(0, (power1 - 1 - i)*eh, tc[tccount].ew, eh), QString::number(i) + powstr[0]);
      }
      *tc[tccount].tex = tc[tccount].tex->mirrored();
      
      power0 = power1;
      power1 = power1 * 10;
    }
    
    maxw = tc[tccount-1].ew;
  }
  ~Degrator()
  {
    for (int i=0; i<tccount; i++)
      delete tc[i].tex;
  }
  unsigned int marginHorz() const { return maxw; }
  unsigned int marginVert() const { return eh; }
  
  int width(int degree) const { for (int i=0; i<tccount; i++) if (tc[i].p1 > degree) return tc[i].ew;  return 0; }
  const uchar* bits (int degree) const
  {
    for (int i=0; i<tccount; i++)
      if (tc[i].p1 > degree)
        return tc[i].tex->constBits() + tc[i].tex->bytesPerLine()*(degree - tc[i].p0)*eh;
    return nullptr;
  }
};

#endif

void DrawPolar::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_dataDimmA = samplesVert*2;
  m_dataDimmB = samplesVert*2;
  m_portionSize = samplesHorz*samplesVert;
  deployMemory();
  
#ifdef DEGRATOR_IS_SHIT
  m_dgt = new Degrator(font());
  
  m_cttrLeft = m_cttrRight = m_dgt->marginHorz() + 4;
  m_cttrTop = m_cttrBottom = m_dgt->marginVert() + 4;
  setContentsMargins(m_cttrLeft, m_cttrTop, m_cttrRight, m_cttrBottom);
#endif
}

DrawPolar::DrawPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, unsigned int backgroundColor, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_POLAR, new SheiGeneratorPolar(samplesHorz, samplesVert, backgroundColor), portions, OR_LRTB, splitGraphs)
{
  reConstructor(samplesHorz, samplesVert);
}

DrawPolar::DrawPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, float turn, float deltaScale01, unsigned int backgroundColor, SPLITPORTIONS splitGraphs):
  DrawQWidget(DATEX_POLAR, new SheiGeneratorPolar(samplesHorz, samplesVert, backgroundColor), portions, OR_LRTB, splitGraphs), m_deltaScale(deltaScale01)
{
  reConstructor(samplesHorz, samplesVert);
  m_viewTurn = turn;
}

DrawPolar::~DrawPolar()
{
#ifdef DEGRATOR_IS_SHIT
  delete m_dgt;
#endif
}

void DrawPolar::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmB? 1 : (sizeA / m_dataDimmB);  // only B
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);  // only B
  clampScaling(scalingA, scalingB);
}

unsigned int DrawPolar::colorBack() const
{
  unsigned int bc = ((SheiGeneratorPolar*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}

void DrawPolar::turn(float rotate01)
{
  if (m_viewTurn != rotate01)
  {
    m_viewTurn = rotate01;
    m_bitmaskPendingChanges |= PC_DATA | PC_SIZE | PC_PARAMS;
  //    if (m_spDivider != 0)
    if (m_splitterA > 1 || m_splitterB > 1)
      innerRescale();
    if (!autoUpdateBanned(RD_BYDATA) && !autoUpdateBanned(RD_BYSETTINGS)) callWidgetUpdate();
  }
}

void DrawPolar::turnDegree(float rotateDeg)
{
  turn(rotateDeg/360.0f);
}

void DrawPolar::turnRadian(float rotateRad)
{
  turn(rotateRad/(M_PI*2));
}

void DrawPolar::initializeGL()
{
  DrawQWidget::initializeGL();
  
}

void DrawPolar::paintGL()
{
//  qDebug()<<m_viewTurn;
  
  DrawQWidget::paintGL();
//  int countDataHorz = ((SheiGeneratorPolar*)m_pcsh)->m_samplesHorz;
//  if (!m_rawResizeModeNoScaled)
//  {
//    if (m_dataDimmSwitchAB)
//      glViewport(0, c_height - sizeA() - m_cttrTop - m_cttrBottom, sizeB() + m_cttrLeft + m_cttrRight, sizeA() + m_cttrTop + m_cttrBottom);
//    else
//      glViewport(0, c_height - sizeB() - m_cttrTop - m_cttrBottom, sizeA() + m_cttrLeft + m_cttrRight, sizeB() + m_cttrTop + m_cttrBottom);
//  }
  
//  float rx = (sizeA() + 2)/float(sizeA() + m_cttrLeft + m_cttrRight);
//  float ry = (sizeB() + 2)/float(sizeB() + m_cttrTop + m_cttrBottom);
//  double g2p = M_PI/180.0;
//  double g2p2 = (m_deltaScale - 0.25f + m_viewTurn)*M_PI*2.0;
//  const int degree_step = 15;
//  for (int i=0; i<360/degree_step; i++)
//  {
//    int degree = i*degree_step;
//    float x = rx*sin(g2p*degree + g2p2);
//    float y = ry*cos(g2p*degree + g2p2);
//    y -= (ry - y)*m_dgt->marginVert()/sizeB();
//    int ew = m_dgt->width(degree);
//    x -= (rx - x)*ew/sizeA();
//    glRasterPos2f(x,y);
//    glDrawPixels(ew, m_dgt->marginVert(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, m_dgt->bits(degree));
//  }
//  QPixmap qpm(100, 20);
//  QPainter pp(&qpm);
//  pp.drawText(0,0, "ABC");
//  glDrawPixels(qpm.width(), qpm.height(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, qpm.toImage().
}
