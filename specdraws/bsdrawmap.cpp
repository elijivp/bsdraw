#include "bsdrawmap.h"

#include <math.h>

#include "../core/sheigen/bsshgenmain.h"

#define BIT_SET_ON(IDX) (1<<(IDX))

#pragma pack(push,1)
struct mappoint_t
{
  float   depth;
  float   res[2];
};
#pragma pack(pop)


#define ETOP_CAST(x)  float(short(((x & 0xff00) >> 8) | ((x & 0x00ff) << 8)))

class MapRanger
{
  const int MAP_Y_SIZE;
  double dlt, dln;
  struct rr_t
  {
    double cos_;
    double sin_;
    double sin_2x2;
  }*   dds;
public:
  MapRanger(int /*map_x_size*/, int map_y_size, double delta_lat, double delta_lon): 
        MAP_Y_SIZE(map_y_size), dlt(delta_lat), dln(delta_lon)
  { 
    dds = new rr_t[1 + MAP_Y_SIZE/2];
    for (int j=0; j<1+MAP_Y_SIZE/2; j++)
    {
      dds[j].cos_ = cos(j*dlt*M_PI/180.0);
      dds[j].sin_ = sin(j*dlt*M_PI/180.0);
      dds[j].sin_2x2 = sin(2*j*dlt*M_PI/180.0);
      dds[j].sin_2x2 = dds[j].sin_2x2*dds[j].sin_2x2;
    }
  }
  ~MapRanger(){  delete []dds; }
  float getDistanceXY(float x1, float y1, float x2, float y2) const
  {
    y1 -= MAP_Y_SIZE/2;
    y2 -= MAP_Y_SIZE/2;
//    qDebug()<<fabs(x1-x2)<<y1<<y2<<"          ";
    int iy1, iy2;
    bool po1, po2;
    if (y1 < 0 && y2 < 0)       {  iy1 = -qRound(y1);  iy2 = -qRound(y2); po1 = true; po2 = true; }
    else if (y1 >= 0 && y2 < 0) {  iy1 = qRound(y1);   iy2 = -qRound(y2); po1 = true; po2 = false; }
    else if (y1 < 0 && y2 >= 0) {  iy1 = -qRound(y1);  iy2 = -qRound(y2); po1 = false; po2 = true; }
    else                        {  iy1 = qRound(y1);   iy2 = qRound(y2);  po1 = true; po2 = true;  }
    
    double dlon = (x2 - x1)*dln*M_PI/180.0;
    
    double cos_lat1 = dds[iy1].cos_;
    double cos_lat2 = dds[iy2].cos_;
    double sin_lat1 = po1 ? dds[iy1].sin_ : -dds[iy1].sin_;
    double sin_lat2 = po2 ? dds[iy2].sin_ : -dds[iy2].sin_;
    
    double cp1 = cos_lat1*sin_lat2 - sin_lat1*cos_lat2*cos(dlon);
    double cp2 = cos_lat2*sin(dlon);
    double tn = sqrt(cp1*cp1 + cp2*cp2)/(sin_lat1*sin_lat2 + cos_lat1*cos_lat2*cos(dlon));
    
    const double DEFAULT_EARTH_RADIUS = 6378137;
    float result = atan(tn)*DEFAULT_EARTH_RADIUS;
    return result;
  }
};

enum  { PB_LOW, PB_GAP_LOW, PB_GAP_HIGH, PB_HIGH };

class DrawMapPrivate
{
public:
  DrawMapPrivate(int map_x_size, int map_y_size): MAP_X_SIZE(map_x_size), MAP_Y_SIZE(map_y_size){}
  const int             MAP_X_SIZE;
  const int             MAP_Y_SIZE;
  
  int                   LIMIT_WIDTH;
  int                   LIMIT_HEIGHT;
  
  char                  mappath[1024];
  mappoint_t*           data;
  bool                  linsmooth;
  
  struct
  {
    double    lat, lon;
    float     ex, ey;   // etop coords
  }                     center;
  
  double                delta_lat, delta_lon;
  double                zoom;
  
  float                 view_dx, view_dy;
  
  float                 c_gab_dist_byx, c_gab_dist_byy;
  float                 c_gab_dist_byx_low, c_gab_dist_byx_high;
  
  float                 mm[4];
  float                 cc01[3];
  float                 palbnd[4];
  float                 mapbnd[3];
  MapRanger*            ranger;
};


class SheiGeneratorMap: public ISheiGenerator
{
public:
  virtual const char*   shaderName() const {  return "MAP"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize(const impulsedata_t& imp, unsigned int ovlscount) const { return 1200 + FshDrawConstructor::basePendingSize(imp, ovlscount); }
  virtual unsigned int  shfragment_store(unsigned int allocatedPortions, ORIENTATION orient, SPLITPORTIONS splitPortions, 
                                         const impulsedata_t& imp, const overpattern_t& fsp, float fspopacity, 
                                         ovlfraginfo_t ovlsinfo[], unsigned int ovlscount, 
                                         locbackinfo_t locbackinfo[], unsigned int* locbackcount,
                                         char* to) const
  {
    globvarinfo_t globvars[] = {  { DT_SAMP4, "mapsampler" },
                                  { DT_1I,    "mapbound_a" },
                                  { DT_1I,    "mapbound_b" },
                                  { DT_3F,    "cc" },
                                  { DT_4F,    "mappalbord" },
                                  { DT_3F,    "mapdepbord" },
    };
    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, sizeof(globvars)/sizeof(globvars[0]), globvars, ovlscount, ovlsinfo);
    fmg.getLocbacks(locbackinfo, locbackcount);
    fmg.main_begin(FshDrawConstructor::INITBACK_BYPALETTE, 0, orient, fsp); //FshDrawConstructor::INITBACK_BYZERO
    fmg.push("vec2 mmii = vec2(float(mapbound_a)/ab_ibounds.x, float(mapbound_b)/ab_ibounds.y);" SHNL);
    fmg.push("vec2  tcoords = (vec2(xy_coords.x, 1.0-xy_coords.y) - vec2(0.5))/mmii/vec2(cc.z) + cc.xy;" SHNL);  // + cc.xy
    fmg.push("vec3  mpx = texture(mapsampler, tcoords).rgb;" SHNL);
    fmg.push("float level = mpx[0] + mapdepbord[0];" SHNL);
    fmg.push("dvalue = mix(  mappalbord[0] + (mappalbord[1] - mappalbord[0])*(1.0 - level/mapdepbord[1]), "
                            "mappalbord[2] + (mappalbord[3] - mappalbord[2])*(level/mapdepbord[2]),"
                            "step(0.0, level) );" SHNL);
    fmg.push("dvalue = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    fmg.push("result = result + texture(paletsampler, vec2(dvalue, 0.0)).rgb;" SHNL);
#ifdef RULER
    {
      fmg.push("float thick = 2.0;");
      fmg.push("vec2 pt  = vec2(ab_coords.x*ab_ibounds.x, ab_coords.y*ab_ibounds.y);" SHNL);
      fmg.push("vec2 ptb = vec2(measurer[0]*ab_ibounds.x, measurer[1]*ab_ibounds.y) - pt;" SHNL);
      fmg.push("vec2 pte = vec2(measurer[2]*ab_ibounds.x, measurer[3]*ab_ibounds.y) - pt;" SHNL);
      fmg.push("vec2 bz = vec2(step(pte.x, 0.0)*step(0.0, pte.x), step(pte.y, 0.0)*step(0.0, pte.y));" SHNL
               "vec2 cz = vec2(1.0/(pte.x-ptb.x), 1.0/(pte.y-ptb.y));" SHNL
               "float dist = abs( (1-bz[0])*(1-bz[1])*(ptb.y*cz[1] - ptb.x*cz[0])/sqrt(cz[0]*cz[0] + cz[1]*cz[1]) + ptb.x*bz[0] + ptb.y*bz[1] );" SHNL
               "float dz = step(length((ptb + pte)/2.0), length(pte-ptb)/2.0 + 1.0 + thick);" SHNL // center point and radius
               "float ez = 1.0 - step(mod(int(sqrt(length(ptb)*length(ptb) - dist*dist)), 30), 2.0);" SHNL
               "result = mix(result, vec3(0.0), (1.0 - clamp((dist-1)/(1.0+thick), 0.0, 1.0))*dz*ez);" SHNL 
               "result = mix(result, vec3(0.0), (1.0 - clamp((dist)/(1.0+thick), 0.0, 1.0))*dz*ez);" SHNL 
               "result = mix(result, vec3(1.0), (1.0 - clamp((0.5+dist)/(1.0+thick), 0.0, 1.0))*dz*ez);" SHNL 
      );
    }
#endif
    
    fmg.push("post_mask[0] = mix(1.0, post_mask[0], step(dvalue, post_mask[1]));" SHNL);
    //    fmg.push( splitPortions == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
    //    fmg.push("{");
    //    {
    //      fmg.value2D("float value");
    ////      fmg.push("value = mix(dvalue, value);");
    //      fmg.push("value = dvalue;");    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //      fmg.push("value = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    //      if ( splitPortions == SP_NONE )
    //        fmg.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" SHNL);
    //      else if (splitPortions & SPFLAG_COLORSPLIT)
    //        fmg.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" SHNL);
    //      else
    //        fmg.push("result.rgb = mix(texture(paletsampler, vec2(value, 0.0)).rgb, result.rgb, step(dataportions, float(explicitPortion)));" SHNL);
          
    //      fmg.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" SHNL);
    //    }
    //    fmg.push("}");
    fmg.main_end(fsp, fspopacity);
    return fmg.written();
  }
};




DrawMap::DrawMap(const char* mappath, int map_x_size, int map_y_size, unsigned int samplesA, unsigned int samplesB, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorMap(), 1, orient, splitPortions)
{
  m_dataDimmA = samplesA;
  m_dataDimmB = samplesB;
  m_portionSize = samplesA*samplesB;
  deployMemory();
  
  pImpl = new DrawMapPrivate(map_x_size, map_y_size);
  strcpy(pImpl->mappath, mappath);
  pImpl->LIMIT_WIDTH = 3000 + 1;
  pImpl->LIMIT_HEIGHT = 2500 + 1;
  pImpl->data = new mappoint_t[pImpl->LIMIT_WIDTH*pImpl->LIMIT_HEIGHT];
  pImpl->linsmooth = true;
  
  pImpl->center.lat = 0;
  pImpl->center.lon = 0;
  pImpl->zoom = 1.0;
  pImpl->center.ex = 0;
  pImpl->center.ey = 0;
  pImpl->view_dx = 0;
  pImpl->view_dy = 0;
  pImpl->ranger = nullptr;
  
  pImpl->cc01[0] = 0.5f;
  pImpl->cc01[1] = 0.5f;
  pImpl->cc01[2] = 1.0f;
  
  pImpl->palbnd[PB_LOW] = 0.0f;
  pImpl->palbnd[PB_GAP_LOW] = 0.5f;
  pImpl->palbnd[PB_GAP_HIGH] = 0.5f;
  pImpl->palbnd[PB_HIGH] = 1.0f;
  
  pImpl->mapbnd[0] = 0;
  pImpl->mapbnd[1] = -10800;
  pImpl->mapbnd[2] = 10800;
}

DrawMap::~DrawMap()
{
  delete []pImpl->data;
  if (pImpl->ranger)
    delete pImpl->ranger;
  delete pImpl;
}

void DrawMap::setPaletteBounds(float pblow, float pbgaplow, float pbgaphigh, float pbhigh, bool update)
{
  pImpl->palbnd[PB_LOW] = pblow;
  pImpl->palbnd[PB_GAP_LOW] = pbgaplow;
  pImpl->palbnd[PB_GAP_HIGH] = pbgaphigh;
  pImpl->palbnd[PB_HIGH] = pbhigh;
  DrawQWidget::vmanUpSec(BIT_SET_ON(3));
}

void DrawMap::paletteBounds(float* pblow, float* pbgaplow, float* pbgaphigh, float* pbhigh) const
{
  *pblow = pImpl->palbnd[PB_LOW];
  *pbgaplow = pImpl->palbnd[PB_GAP_LOW];
  *pbgaphigh = pImpl->palbnd[PB_GAP_HIGH];
  *pbhigh = pImpl->palbnd[PB_HIGH];
}

bool DrawMap::loadTo(double lat, double lon)
{
  FILE* file = fopen(pImpl->mappath, "rb+");
  if (!file)
    return false;
  
  pImpl->center.lat = lat;
  pImpl->center.lon = lon;
  pImpl->center.ex = (lon + 180.0) * pImpl->MAP_X_SIZE/360.0;
  pImpl->center.ey = (lat >= 0 ? 90.0 - lat : 90.0 + fabs(lat)) * pImpl->MAP_Y_SIZE/180.0;
  int cx = qRound(pImpl->center.ex), cy = qRound(pImpl->center.ey);
 
  short dataline[pImpl->LIMIT_WIDTH];
  for (int h=0; h<pImpl->LIMIT_HEIGHT; h++)
  {
    int y = cy - pImpl->LIMIT_HEIGHT/2 + h;
    if (y < 0)                          y += pImpl->MAP_Y_SIZE;
    else if (y >= pImpl->MAP_Y_SIZE)    y -= pImpl->MAP_Y_SIZE;
    
    int fyoffs = y*pImpl->MAP_X_SIZE;
    
    int hcycles = 1;
    int xbords[2][2] = {  { cx - pImpl->LIMIT_WIDTH/2, cx  - pImpl->LIMIT_WIDTH/2 + pImpl->LIMIT_WIDTH }, {0,0}  };
    if (xbords[0][0] < 0)
    {
      xbords[1][0] = 0;
      xbords[1][1] = xbords[0][1];
      xbords[0][0] = pImpl->MAP_X_SIZE + xbords[0][0];
      xbords[1][0] = pImpl->MAP_X_SIZE;
      hcycles += 1;
    }
    else if (xbords[0][1] >= pImpl->MAP_X_SIZE)
    {
      xbords[1][0] = 0;
      xbords[1][1] = xbords[0][1] - pImpl->MAP_X_SIZE;
      xbords[0][1] = pImpl->MAP_X_SIZE;
      hcycles += 1;
    }
    
    short* dl = dataline;
    for (int hc=0; hc<hcycles; hc++)
    {
      int ind_byte = (fyoffs + xbords[hc][0])*sizeof(short);
      fseek(file, ind_byte, SEEK_SET);
      int count = xbords[hc][1] - xbords[hc][0];
      fread(dl, sizeof(short), count, file);
      dl += count;
    }
    
    for (int w=0; w<pImpl->LIMIT_WIDTH; w++)
      pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth = ETOP_CAST(dataline[w]);
  } // for h
  
  fclose(file);
  
  float fmin = 1e+10, fmax = -1e+10;
  for (int h=0; h<pImpl->LIMIT_HEIGHT; h++)
    for (int w=0; w<pImpl->LIMIT_WIDTH; w++)
    {
      if (fmin > pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth) fmin = pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth;
      if (fmax < pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth) fmax = pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth;
    }
  pImpl->delta_lat = 1.0/60.0;
  pImpl->delta_lon = 1.0/60.0;
  pImpl->mapbnd[0] = 0.0f;
  pImpl->mapbnd[1] = -2000.0f;
  pImpl->mapbnd[2] = 2000.0f;
  
  pImpl->ranger = new MapRanger(pImpl->MAP_X_SIZE, pImpl->MAP_Y_SIZE, pImpl->delta_lat, pImpl->delta_lon);
  DrawQWidget::vmanUpSec(BIT_SET_ON(0) | BIT_SET_ON(1));
  return true;
}

float     DrawMap::zoom() const { return pImpl->zoom; }
void      DrawMap::setZoom(float zoom, bool update)
{
  pImpl->zoom = zoom;
  pImpl->cc01[2] = pImpl->zoom;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(2));
}

float DrawMap::zeroLevel() const
{
  return pImpl->mapbnd[0];
}

void DrawMap::setZeroLevel(float floor, bool update)
{
  pImpl->mapbnd[0] = floor;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(3));
}


void      DrawMap::viewToLL(double lat, double lon, bool update)
{
  viewToLLRel(pImpl->center.lat - lat, pImpl->center.lon - lon, update);
}
void      DrawMap::viewToLLRel(double dlat, double dlon, bool update)
{
  pImpl->view_dx = dlon / pImpl->delta_lon;
  pImpl->view_dy = dlat / pImpl->delta_lat;
  pImpl->cc01[0] = 0.5f + pImpl->view_dx/pImpl->LIMIT_WIDTH;
  pImpl->cc01[1] = 0.5f + pImpl->view_dy/pImpl->LIMIT_HEIGHT;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(2));
}
void      DrawMap::viewToRel(float dx, float dy, bool update)
{
  pImpl->view_dx += dx;
  pImpl->view_dy += dy;
  pImpl->cc01[0] = 0.5f + pImpl->view_dx/pImpl->LIMIT_WIDTH;
  pImpl->cc01[1] = 0.5f + pImpl->view_dy/pImpl->LIMIT_HEIGHT;
//  qDebug()<<dx<<dy<<"       "<<pImpl->cc01[0]<<pImpl->cc01[1]<<"                zoo="<<zoom()<<update<<QString::number(BIT_SET_ON(2), 16);
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(2));
}

void      DrawMap::coordsLLOfViewCenter(double* plat, double* plon) const
{
  *plat = -(((pImpl->center.ey + pImpl->view_dy)*180.0)/pImpl->MAP_Y_SIZE - 90.0);
  *plon = ((pImpl->center.ex + pImpl->view_dx)*360.0/pImpl->MAP_X_SIZE) - 180.0;
}
void      DrawMap::coordsEEOfViewCenter(float* px, float* py) const
{
  *px = pImpl->center.ex + pImpl->view_dx;
  *py = pImpl->center.ey + pImpl->view_dy;
}
void      DrawMap::coordsOfViewCenter(float* px, float* py) const
{
  *px = pImpl->view_dx;
  *py = pImpl->view_dy;
}

void      DrawMap::coordsOOByPix(float px, float py, float* pdx, float* pdy) const
{
  float w = this->width(), h = this->height();
//  *pdx = pImpl->center.ex + pImpl->view_dx + w/2.0f - (px/pImpl->zoom/w)/2.0f;
//  *pdy = pImpl->center.ey + pImpl->view_dy + h/2.0f - (py/pImpl->zoom/h)/2.0f;
  *pdx = pImpl->center.ex + pImpl->view_dx + (px - w/2.0f)/pImpl->zoom;
  *pdy = pImpl->center.ey + pImpl->view_dy + (py - h/2.0f)/pImpl->zoom;
}

float     DrawMap::distanceOO(float x1, float y1, float x2, float y2) const
{
  return pImpl->ranger == nullptr ? 0.0f : pImpl->ranger->getDistanceXY(x1, y1, x2, y2);
}

float DrawMap::distanceByTop() const
{
  float y = pImpl->center.ey + pImpl->view_dy - this->height()/2.0f/pImpl->zoom;
  float x = pImpl->center.ex + pImpl->view_dx;
  float x1 = x - this->width()/2.0f/pImpl->zoom, x2 = x + this->width()/2.0f/pImpl->zoom;
  return pImpl->ranger == nullptr ? 0.0f : pImpl->ranger->getDistanceXY(x1, y, x2, y);
}

float DrawMap::distanceByBottom() const
{
  float y = pImpl->center.ey + pImpl->view_dy + this->height()/2.0f/pImpl->zoom;
  float x = pImpl->center.ex + pImpl->view_dx;
  float x1 = x - this->width()/2.0f/pImpl->zoom, x2 = x + this->width()/2.0f/pImpl->zoom;
  return pImpl->ranger == nullptr ? 0.0f : pImpl->ranger->getDistanceXY(x1, y, x2, y);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////

#define BIT_CHECK_OFF(IDX) ((secflags & (1<<IDX)) == 0)

void DrawMap::processGlLocation(int secidx, int secflags, int loc, int TEX)
{
  if (secidx == 0)
  { 
    if (BIT_CHECK_OFF(0))
      return;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pImpl->linsmooth ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pImpl->linsmooth ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,    4);
    
    GLint   gl_internalFormat = GL_RGB32F;
    GLenum  gl_format = GL_RGB;
    GLenum  gl_texture_type = GL_FLOAT;
    glTexImage2D(  GL_TEXTURE_2D, 0, gl_internalFormat, pImpl->LIMIT_WIDTH, pImpl->LIMIT_HEIGHT, 0, gl_format, gl_texture_type, pImpl->data);
    
    m_ShaderProgram.setUniformValue(loc, TEX);
  }
  else if (secidx == 1 || secidx == 2)
  {
    if (BIT_CHECK_OFF(1))
      return;
    m_ShaderProgram.setUniformValue(loc, secidx == 1 ? pImpl->LIMIT_WIDTH : pImpl->LIMIT_HEIGHT);
  }
  else if (secidx == 3)
  {
    if (BIT_CHECK_OFF(0) && BIT_CHECK_OFF(1) && BIT_CHECK_OFF(2))
      return;
    qDebug()<<"DMdef, ee center = "<<pImpl->center.ex<<pImpl->center.ey;
    m_ShaderProgram.setUniformValue(loc, *(const QVector3D*)pImpl->cc01);
  }
  else if (secidx == 4 || secidx == 5)
  {
    if (BIT_CHECK_OFF(3))
      return;
    if (secidx == 4)
      m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)pImpl->palbnd);
    else if (secidx == 5)
      m_ShaderProgram.setUniformValue(loc, *(const QVector3D*)pImpl->mapbnd);
  }
//  else if (secidx == 6)
//  {
//    if (BIT_CHECK_OFF(4))
//      return;
//  }
}

void DrawMap::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}





MapReactorMove::MapReactorMove(QObject* parent): QObject(parent)
{
}

bool MapReactorMove::reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool*)
{
  DrawMap* self = (DrawMap*)draw;
  
  float x = ct->fx_pix;
  float y = ct->fy_pix;
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
  {
    if (orm == ORM_LMMOVE)
      self->viewToRel((lx - x)/self->zoom(), (ly - y)/self->zoom());
    lx = x;
    ly = y;
    if (orm == ORM_LMMOVE)
    {
      double lat, lon;
      self->coordsLLOfViewCenter(&lat, &lon);
      emit coordsChanged(lat, lon);
    }
    return true;
  }
  return false;
}

bool MapReactorMove::reactionWheel(DrawQWidget* draw, OVL_REACTION_WHEEL orm, const coordstriumv_t* ct, bool*)
{
  DrawMap* self = (DrawMap*)draw;
  
  if (orm == ORW_AWAY || orm == ORW_TOWARD)
  {
    float nextzoom = orm == ORW_AWAY ? self->zoom()*1.05f : self->zoom()*1/1.05f;
    if (nextzoom < 0.0001f)     nextzoom = 0.0001f;
    else if (nextzoom > 16.0f)   nextzoom = 16.0f;
    self->setZoom(nextzoom);
    emit zoomChanged(self->zoom());
    return true;
  }
  return false;
}
