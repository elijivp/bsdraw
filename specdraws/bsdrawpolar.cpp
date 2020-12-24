/// This file contains special derived class for show images
/// DrawPolar 
/// DrawPolar 
/// Created By: Elijah Vlasov
#include "bsdrawpolar.h"
#include "core/sheigen/bsshgenmain.h"

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
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 1800 + FshMainGenerator::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, const DPostmask& fsp, 
                                         ORIENTATION orient, SPLITPORTIONS splitPortions, const impulsedata_t& imp,
                                         unsigned int ovlscount, ovlfraginfo_t ovlsinfo[], char* to) const
  {
    FshMainGenerator fmg(to, allocatedPortions, splitPortions, imp, ovlscount, ovlsinfo);
    fmg.main_begin(FshMainGenerator::INIT_BYVALUE, m_bckclr, orient, fsp, m_samplesHorz, m_samplesVert);
    fmg.cintvar("allocatedPortions", (int)allocatedPortions);
    fmg.push("float polarrotate = 0.0;");

    fmg.push("vec3 mpi = vec3(3.14159265359, 1.57079632679, 6.28318530718);");
    fmg.push("vec2 datacoords = (relcoords - vec2(0.5, 0.5))*2;");
//    fmg.push("vec3 lenarcscal = vec3(length(datacoords), mpi[1] + atan(datacoords.x, datacoords.y) + mpi[2]*(1.0-step(0.0, datacoords.x))*(1.0 - step(0.0, datacoords.y)), 0.0);"
//             "lenarcscal[1] = mod(lenarcscal[1] + polarrotate*mpi[2], mpi[2]);"
//             "lenarcscal[2] = max((mpi[2]*lenarcscal[0]*dbounds.y/2.0)/polarHorz, 1);"
//             "datacoords = vec2(lenarcscal[1]/mpi[2], lenarcscal[0]);"
//             );
//    fmg.push("imoded = ivec2( int(mod(((lenarcscal[1]*lenarcscal[0]*dbounds.y/2.0)), lenarcscal[2])), int(mod(lenarcscal[0]*dbounds.y, float(iscaling.y))) );");
//    fmg.push("imrect  = ivec4(imoded.x, imoded.y, lenarcscal[2]-1, iscaling.y-1);");
    
//    fmg.push("vec3 lenarcscal = vec3(length(datacoords), mpi[1] + atan(datacoords.x, datacoords.y) + mpi[2]*(1.0-step(0.0, datacoords.x))*(1.0 - step(0.0, datacoords.y)), 0.0);"
//             "lenarcscal[1] = mod(lenarcscal[1] + polarrotate*mpi[2], mpi[2]);"
//             "lenarcscal[2] = max(((mpi[2]*lenarcscal[0]*dbounds.y/2.0)/polarHorz), 1);"
//             "datacoords = vec2(lenarcscal[1]/mpi[2], lenarcscal[0]);"
//             );
//    fmg.push("imoded = ivec2( int((mod((lenarcscal[1]*lenarcscal[0]*dbounds.y/2.0), lenarcscal[2]))), int(mod(lenarcscal[0]*dbounds.y, float(iscaling.y))) );");
//    fmg.push("imrect  = ivec4(imoded.x, imoded.y, lenarcscal[2]-1, iscaling.y-1);");
    
    fmg.push("vec4 lenarcscal = vec4(length(datacoords), mpi[1] + atan(datacoords.x, datacoords.y) + mpi[2]*(1.0-step(0.0, datacoords.x))*(1.0 - step(0.0, datacoords.y)), 0.0, 0.0);"
             "lenarcscal[1] = mod(lenarcscal[1] + polarrotate*mpi[2], mpi[2]);"
             "dbounds.x = (mpi[2]*lenarcscal[0]*dbounds_noscaled.y*iscaling.y);"
             );
    
    fmg.push("lenarcscal[2] = max(dbounds.x/dbounds_noscaled.x, 1.0);"
             "lenarcscal[3] = lenarcscal[1]*lenarcscal[0]*dbounds.y*iscaling.y;"
             "lenarcscal[3] = lenarcscal[3] - floor(lenarcscal[3]/lenarcscal[2])*lenarcscal[2];"
//             "float tempo = 0.0*min(abs(datacoords.x/datacoords.y), abs(datacoords.y/datacoords.x));"
//             "float tempo = 0.5*max(0.0, 1 - tan((mpi[1]/2.0 - abs(atan(datacoords.x, datacoords.y)))) );"
//             "float tempo = 0.5*1.0/(1.0 + 2*tan(abs(mpi[1]/2.0 - abs(atan(datacoords.x, datacoords.y)))) );"
             "float tempo = 0.0;"
             
             "imoded = ivec2( int( lenarcscal[3]*(1.0 + tempo) ), int(mod(lenarcscal[0]*dbounds.y, float(iscaling.y))) );"
             "imrect = ivec4(imoded.x, imoded.y, lenarcscal[2]-1, iscaling.y-1);"
             );

    
    fmg.push("datacoords = vec2(lenarcscal[1]/mpi[2], lenarcscal[0]);"
             "lenarcscal[2] = step(lenarcscal[0], 1.0);"
             );
    
    fmg.push( splitPortions == SL_NONE? "for (int i=0; i<countPortions; i++)" : "int i=icell[0];" );
    fmg.push( "{" );
    {
      fmg.value2D("float value", "datacoords");
      fmg.push("ovMix = max(ovMix, value);");
      
      if ( splitPortions == SL_NONE )
        fmg.push( "result = mix(result, result + texture(texPalette, vec2(value, float(i)/(allocatedPortions-1) )).rgb, lenarcscal[2]);"
//                  "result = mix(result, vec3(0), step(float(imoded.x), 3.0)*step(3.0, float(imoded.x)));"
                  );
      else
        fmg.push( "result = mix(result, texture(texPalette, vec2(value, 0.0)).rgb, (1.0 - step(countPortions, float(icell[0])))*lenarcscal[2]  );" );

      fmg.push("post_mask[0] = mix(post_mask[0], 1.0, (1.0 - step(value, post_mask[1]))*lenarcscal[2] );" );
    }
    fmg.push( "}" );
    fmg.main_end(fsp);
    return fmg.written();
  }
};
SheiGeneratorPolar::~SheiGeneratorPolar(){}



void DrawPolar::reConstructor(unsigned int samplesHorz, unsigned int samplesVert)
{
  m_matrixDimmA = samplesVert*2;
  m_matrixDimmB = samplesVert*2;
  m_portionSize = samplesHorz*samplesVert;
  deployMemory();
}

DrawPolar::DrawPolar(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions, unsigned int backgroundColor, SPLITPORTIONS splitGraphs):
  DrawQWidget(new SheiGeneratorPolar(samplesHorz, samplesVert, backgroundColor), portions, OR_LRTB, splitGraphs)
{
  reConstructor(samplesHorz, samplesVert);
}

void DrawPolar::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_matrixDimmA;
  *matrixDimmB = m_matrixDimmB;
  *scalingA = (unsigned int)sizeA <= m_matrixDimmB? 1 : (sizeA / m_matrixDimmB);  // only B
  *scalingB = (unsigned int)sizeB <= m_matrixDimmB? 1 : (sizeB / m_matrixDimmB);  // only B
  clampScaling(scalingA, scalingB);
}

unsigned int DrawPolar::colorBack() const
{
  unsigned int bc = ((SheiGeneratorPolar*)m_pcsh)->m_bckclr;
  if (bc == 0xFFFFFFFF)
    return DrawQWidget::colorBack();
  return bc;
}
