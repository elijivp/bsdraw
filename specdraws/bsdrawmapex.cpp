#include "bsdrawmapex.h"

#include <math.h>
#include <QElapsedTimer>

#include "../core/sheigen/bsshgenmain.h"

#define BIT_SET_ON(IDX) (1<<(IDX))

#pragma pack(push,1)
struct mappoint_t
{
  float   depth;
  float   res[2];
};


struct scpoint_t
{
  float   cos_;
  float   sin_;
  float   res;
};

#pragma pack(pop)


#define ETOP_CAST(x)  float(short(((x & 0xff00) >> 8) | ((x & 0x00ff) << 8)))

#define DMETERS

enum  { PB_LOW, PB_GAP_LOW, PB_GAP_HIGH, PB_HIGH };

class DrawMapExPrivate
{
public:
  DrawMapExPrivate(int map_x_size, int map_y_size): MAP_X_SIZE(map_x_size), MAP_Y_SIZE(map_y_size){}
  const int             MAP_X_SIZE;
  const int             MAP_Y_SIZE;
  
  int                   LIMIT_WIDTH;
  int                   LIMIT_HEIGHT;
  
  char                  mappath[1024];
  scpoint_t*            sctex;
  mappoint_t*           data;
  int                   datamin_x, datamin_y;
  int                   datamax_x, datamax_y;
  bool                  autolim_min, autolim_max;
  bool                  linsmooth;
  
  struct
  {
    double    lat, lon;
    float     ex, ey;   // etop coords
  }                     center;
  
  double                delta_lat, delta_lon;
  double                basicrange;
  double                zoom;
  
  float                 range() const { return basicrange/zoom; }
  
#ifdef DMETERS
  float                 mdx, mdy;
#else
  float                 view_dx, view_dy;
#endif
  
  float                 c_gab_dist_byx, c_gab_dist_byy;
  float                 c_gab_dist_byx_low, c_gab_dist_byx_high;
  
  float                 mm[4];
  float                 cc01[4];
  float                 palbnd[4];
  float                 mapbnd[3];
};

enum  { UP_SCTABLE, UP_MAP, UP_RANGE, UP_COORDS, UP_PALBORD, UP_DEPTHS   };
enum  { SDT_SCTABLE, SDT_MAP, SDT_MAPBOUND1, SDT_MAPBOUND2, SDT_RANGE, SDT_COORDS, SDT_PALBORD, SDT_DEPTHS   };


class SheiGeneratorMapEx: public ISheiGenerator
{
  int     vertsize;
  int     iterations; 
public:
  SheiGeneratorMapEx(int _vertsize, int _iterations=16): vertsize(_vertsize), iterations(_iterations){}
  virtual const char*   shaderName() const {  return "MAP"; }
  virtual int           portionMeshType() const { return PMT_PSEUDO2D; }
  virtual unsigned int  shvertex_pendingSize() const  {  return VshMainGenerator2D::pendingSize(); }
  virtual unsigned int  shvertex_store(char* to) const {  return VshMainGenerator2D()(to); }
  virtual unsigned int  shfragment_pendingSize() const { return 2000; }
  virtual unsigned int  shfragment_uniforms(shuniformdesc_t* sfdarr, unsigned int limit)
  {
    strcpy(sfdarr[0].varname, "sctsampler");    sfdarr[0].type = DT_SAMP4;
    strcpy(sfdarr[1].varname, "mapsampler");    sfdarr[1].type = DT_SAMP4;
    strcpy(sfdarr[2].varname, "mapbound_a");    sfdarr[2].type = DT_1I;
    strcpy(sfdarr[3].varname, "mapbound_b");    sfdarr[3].type = DT_1I;
    strcpy(sfdarr[4].varname, "range");         sfdarr[4].type = DT_1F;
    strcpy(sfdarr[5].varname, "ddcc");          sfdarr[5].type = DT_4F;
    strcpy(sfdarr[6].varname, "mappalbord");    sfdarr[6].type = DT_4F;
    strcpy(sfdarr[7].varname, "mapdepbord");    sfdarr[7].type = DT_3F;
    return 8;
  }
  virtual void          shfragment_store(FshDrawComposer& fdc) const
  {
    fdc.cintvar("vertsize", vertsize);
    fdc.cintvar("iterations", iterations);
    fdc.cfloatvar("ee2rad", 1/60.0*M_PI/180.0);
    
#ifdef DMETERS
    fdc.push("vec2 ptc = vec2(ddcc[2], ddcc[3]);" SHNL);
    
    fdc.push("float dvert = float(ab_ibounds.y)/float(ab_ibounds.x);" SHNL);
    fdc.push("float ea=6378000;" SHNL);
    fdc.push("float eb=6357000;" SHNL);
    
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    fdc.push("float latM = (ptc.y + yside/2 - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("float cosM = cos(latM);  float sinM = sin(latM);" SHNL);
    fdc.push("float mympart = sqrt((ea*cosM)*(ea*cosM) + (eb*sinM)*(eb*sinM));" SHNL);
    fdc.push("float MyM=(ea*eb)*(ea*eb)/(mympart*mympart*mympart);" SHNL);
    fdc.push("float dy = MyM*ee2rad;" SHNL);
    fdc.push("float yd = (1.0 - xy_coords.y - 0.5)*range*dvert / dy;" SHNL);  // /1855
    
    fdc.push("float latT = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("float cosT = cos(latT);  float sinT = sin(latT);" SHNL);
    fdc.push("float Ny=(ea*ea)/sqrt((ea*cosT)*(ea*cosT) + (eb*sinT)*(eb*sinT));" SHNL);
    fdc.push("float tgtdist = abs(xy_coords.x - 0.5)*range;" SHNL);
    fdc.push("float dx = (Ny*cos((ddcc[3]-vertsize/2.0)*ee2rad)*ee2rad);" SHNL);
    fdc.push("float xd = tgtdist/dx*sign(xy_coords.x - 0.5);" SHNL);
    
    fdc.push("vec2  eecoords = vec2(ddcc[0]/dx + xd + mapbound_a/2.0, ddcc[1]/dy + yd + mapbound_b/2.0);" SHNL);
    fdc.push("vec2  tcoords = eecoords/vec2(mapbound_a, mapbound_b);" SHNL);  // + cc.xy
    
    fdc.push("vec3  mpx = texture(mapsampler, tcoords).rgb;" SHNL);
    fdc.push("float level = mpx[0] + mapdepbord[0];" SHNL);
    fdc.push("dvalue = mix(  mappalbord[0] + (mappalbord[1] - mappalbord[0])*(1.0 - level/mapdepbord[1]), "
                            "mappalbord[2] + (mappalbord[3] - mappalbord[2])*(level/mapdepbord[2]),"
                            "step(0.0, level) );" SHNL);
//    fdc.push("dvalue = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    fdc.push("result = result + texture(paletsampler, vec2(dvalue, 0.0)).rgb;" SHNL);
#else
    fdc.push("vec2 ptc = vec2(ddcc[0] + ddcc[2], ddcc[1] + ddcc[3]);" SHNL);
    
    fdc.push("float dvert = float(ab_ibounds.y)/float(ab_ibounds.x);" SHNL);
    fdc.push("float ea=6378000;" SHNL);
    fdc.push("float eb=6357000;" SHNL);
    
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    fdc.push("float latM = (ptc.y + yside/2 - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("float cosM = cos(latM);  float sinM = sin(latM);" SHNL);
    fdc.push("float mympart = sqrt((ea*cosM)*(ea*cosM) + (eb*sinM)*(eb*sinM));" SHNL);
    fdc.push("float MyM=(ea*eb)*(ea*eb)/(mympart*mympart*mympart);" SHNL);
    fdc.push("float dy = MyM*ee2rad;" SHNL);
    fdc.push("float yd = (1.0 - xy_coords.y - 0.5)*range*dvert / dy;" SHNL);  // /1855
    
    fdc.push("float latT = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("float cosT = cos(latT);  float sinT = sin(latT);" SHNL);
    fdc.push("float Ny=(ea*ea)/sqrt((ea*cosT)*(ea*cosT) + (eb*sinT)*(eb*sinT));" SHNL);
    fdc.push("float tgtdist = abs(xy_coords.x - 0.5)*range;" SHNL);
    fdc.push("float xd = tgtdist/(Ny*cos((ddcc[3]-vertsize/2.0)*ee2rad)*ee2rad)*sign(xy_coords.x - 0.5);" SHNL);
    
    fdc.push("vec2  eecoords = vec2(ddcc[0] + xd + mapbound_a/2.0, ddcc[1] + yd + mapbound_b/2.0);" SHNL);
    fdc.push("vec2  tcoords = eecoords/vec2(mapbound_a, mapbound_b);" SHNL);  // + cc.xy
    
    fdc.push("vec3  mpx = texture(mapsampler, tcoords).rgb;" SHNL);
    fdc.push("float level = mpx[0] + mapdepbord[0];" SHNL);
    fdc.push("dvalue = mix(  mappalbord[0] + (mappalbord[1] - mappalbord[0])*(1.0 - level/mapdepbord[1]), "
                            "mappalbord[2] + (mappalbord[3] - mappalbord[2])*(level/mapdepbord[2]),"
                            "step(0.0, level) );" SHNL);
    fdc.push("dvalue = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    fdc.push("result = result + texture(paletsampler, vec2(dvalue, 0.0)).rgb;" SHNL);
#endif
    
    
#ifdef RULER
    {
      fdc.push("float thick = 2.0;");
      fdc.push("vec2 pt  = vec2(ab_coords.x*ab_ibounds.x, ab_coords.y*ab_ibounds.y);" SHNL);
      fdc.push("vec2 ptb = vec2(measurer[0]*ab_ibounds.x, measurer[1]*ab_ibounds.y) - pt;" SHNL);
      fdc.push("vec2 pte = vec2(measurer[2]*ab_ibounds.x, measurer[3]*ab_ibounds.y) - pt;" SHNL);
      fdc.push("vec2 bz = vec2(step(pte.x, 0.0)*step(0.0, pte.x), step(pte.y, 0.0)*step(0.0, pte.y));" SHNL
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
    
    fdc.push("post_mask[0] = mix(1.0, post_mask[0], step(dvalue, post_mask[1]));" SHNL);
    //    fdc.push( splitPortions == SP_NONE? "for (int i=0; i<dataportions; i++)" SHNL : "int i = explicitPortion;" SHNL );
    //    fdc.push("{");
    //    {
    //      fdc.value2D("float value");
    ////      fdc.push("value = mix(dvalue, value);");
    //      fdc.push("value = dvalue;");    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ////      fdc.push("value = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    //      if ( splitPortions == SP_NONE )
    //        fdc.push("result = result + texture(paletsampler, vec2(value, float(i)/(allocatedPortions-1) )).rgb;" SHNL);
    //      else if (splitPortions & SPFLAG_COLORSPLIT)
    //        fdc.push("result = result + texture(paletsampler, vec2(float(i + value)/(allocatedPortions), 0.0)).rgb;" SHNL);
    //      else
    //        fdc.push("result.rgb = mix(texture(paletsampler, vec2(value, 0.0)).rgb, result.rgb, step(dataportions, float(explicitPortion)));" SHNL);
          
    //      fdc.push( "post_mask[0] = mix(1.0, post_mask[0], step(value, post_mask[1]));" SHNL);
    //    }
    //    fdc.push("}");
  }
};




DrawMapEx::DrawMapEx(const char* mappath, int map_x_size, int map_y_size, unsigned int samplesA, unsigned int samplesB, ORIENTATION orient, SPLITPORTIONS splitPortions): 
  DrawQWidget(DATEX_2D, new SheiGeneratorMapEx(map_y_size), 1, orient, splitPortions, 0xFFFFFFFF)
{
  m_dataDimmA = samplesA;
  m_dataDimmB = samplesB;
  m_portionSize = samplesA*samplesB;
  deployMemory();
  
  pImpl = new DrawMapExPrivate(map_x_size, map_y_size);
  strcpy(pImpl->mappath, mappath);
  pImpl->LIMIT_WIDTH = 3200 + 1;
  pImpl->LIMIT_HEIGHT = 2500 + 1;
  pImpl->sctex = new scpoint_t[1 + pImpl->MAP_Y_SIZE/2];
  
  double dlt = 1/60.0;
  for (int j=0; j<1+pImpl->MAP_Y_SIZE/2; j++)
  {
    pImpl->sctex[j].cos_ = cos(j*dlt*M_PI/180.0);
    pImpl->sctex[j].sin_ = sin(j*dlt*M_PI/180.0);
  }
  
  pImpl->data = new mappoint_t[pImpl->LIMIT_WIDTH*pImpl->LIMIT_HEIGHT];
  pImpl->datamin_x = pImpl->datamin_y = pImpl->datamax_x = pImpl->datamax_y = 0;
  pImpl->autolim_min = true;
  pImpl->autolim_max = true;
  pImpl->linsmooth = true;
  
  pImpl->center.lat = 0;
  pImpl->center.lon = 0;
  pImpl->basicrange = 500000.0;
  pImpl->zoom = 1.0;
  pImpl->center.ex = 0;
  pImpl->center.ey = 0;
#ifdef DMETERS
  pImpl->mdx = 0;
  pImpl->mdy = 0;
#else
  pImpl->view_dx = 0;
  pImpl->view_dy = 0;
#endif
  
  pImpl->cc01[0] = 0.0f;
  pImpl->cc01[1] = 0.0f;
  pImpl->cc01[2] = 0.0f;
  pImpl->cc01[3] = 0.0f;
  
  pImpl->palbnd[PB_LOW] = 0.0f;
  pImpl->palbnd[PB_GAP_LOW] = 0.5f;
  pImpl->palbnd[PB_GAP_HIGH] = 0.5f;
  pImpl->palbnd[PB_HIGH] = 1.0f;
  
  pImpl->mapbnd[0] = 0;
  pImpl->mapbnd[1] = -10800;
  pImpl->mapbnd[2] = 10800;
}

DrawMapEx::~DrawMapEx()
{
  delete []pImpl->sctex;
  delete []pImpl->data;
  delete pImpl;
}

void DrawMapEx::setPaletteBounds(float pblow, float pbgaplow, float pbgaphigh, float pbhigh, bool update)
{
  pImpl->palbnd[PB_LOW] = pblow;
  pImpl->palbnd[PB_GAP_LOW] = pbgaplow;
  pImpl->palbnd[PB_GAP_HIGH] = pbgaphigh;
  pImpl->palbnd[PB_HIGH] = pbhigh;
  DrawQWidget::vmanUpSec(BIT_SET_ON(UP_PALBORD));
}

void DrawMapEx::paletteBounds(float* pblow, float* pbgaplow, float* pbgaphigh, float* pbhigh) const
{
  *pblow = pImpl->palbnd[PB_LOW];
  *pbgaplow = pImpl->palbnd[PB_GAP_LOW];
  *pbgaphigh = pImpl->palbnd[PB_GAP_HIGH];
  *pbhigh = pImpl->palbnd[PB_HIGH];
}

bool DrawMapEx::loadTo(double lat, double lon)
{
  FILE* file = fopen(pImpl->mappath, "rb+");
  if (!file)
    return false;
  
  pImpl->center.lat = lat;
  pImpl->center.lon = lon;
  pImpl->center.ex = (lon + 180.0) * pImpl->MAP_X_SIZE/360.0;
  pImpl->center.ey = (90.0 - lat) * pImpl->MAP_Y_SIZE/180.0;
  int cx = qRound(pImpl->center.ex), cy = qRound(pImpl->center.ey);
 
  short* dataline = new short[pImpl->LIMIT_WIDTH];
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
  delete []dataline;
  fclose(file);
  
  pImpl->mdx = 0.0f;
  pImpl->mdy = 0.0f;
  pImpl->cc01[0] = 0.0f;
  pImpl->cc01[1] = 0.0f;
  pImpl->cc01[2] = pImpl->center.ex;
  pImpl->cc01[3] = pImpl->center.ey;
  
  float fmin = 1e+10, fmax = -1e+10;
  pImpl->datamin_x = pImpl->datamin_y = pImpl->datamax_x = pImpl->datamax_y = 0;
  for (int h=0; h<pImpl->LIMIT_HEIGHT; h++)
    for (int w=0; w<pImpl->LIMIT_WIDTH; w++)
    {
      if (fmin > pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth)
      {
        fmin = pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth;
        pImpl->datamin_x = w;
        pImpl->datamin_y = h;
      }
      if (fmax < pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth)
      {
        fmax = pImpl->data[pImpl->LIMIT_WIDTH*h + w].depth;
        pImpl->datamax_x = w;
        pImpl->datamax_y = h;
      }
    }
  pImpl->delta_lat = 1.0/60.0;
  pImpl->delta_lon = 1.0/60.0;
  pImpl->mapbnd[0] = 0.0f;
  if (pImpl->autolim_min)
    pImpl->mapbnd[1] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamin_y + pImpl->datamin_x].depth;
  if (pImpl->autolim_max)
    pImpl->mapbnd[2] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamax_y + pImpl->datamax_x].depth;
  
  DrawQWidget::vmanUpSec(BIT_SET_ON(UP_MAP) | BIT_SET_ON(UP_RANGE) | BIT_SET_ON(UP_COORDS));
  return true;
}

double DrawMapEx::currentLat() const { return pImpl->center.lat; }
double DrawMapEx::currentLon() const { return pImpl->center.lon; }

float     DrawMapEx::zoom() const { return pImpl->zoom; }
void      DrawMapEx::setZoom(float zoom, bool update)
{
  pImpl->zoom = zoom;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_RANGE));
}

void DrawMapEx::setZoomByRange(float range, bool update)
{
  pImpl->zoom = pImpl->basicrange/range;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_RANGE));
}


bool DrawMapEx::inCenter() const
{
#ifdef DMETERS
  return pImpl->mdx == 0.0f && pImpl->mdy == 0.0f;
#else
  return pImpl->view_dx == 0.0f && pImpl->view_dy == 0.0f;
#endif
}

void DrawMapEx::goCenter()
{
#ifdef DMETERS
  viewToMM(0,0);
#else
  viewToLL(0.0, 0.0);
#endif
}



float DrawMapEx::zeroLevel() const
{
  return pImpl->mapbnd[0];
}

void DrawMapEx::setZeroLevel(float floor, bool update)
{
  pImpl->mapbnd[0] = floor;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimitFloor(float value, bool update)
{
  pImpl->mapbnd[1] = value;
  pImpl->autolim_min = false;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimitSurf(float value, bool update)
{
  pImpl->mapbnd[2] = value;
  pImpl->autolim_max = false;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimit(float value_floor, float value_surf, bool update)
{
  pImpl->mapbnd[1] = value_floor;
  pImpl->mapbnd[2] = value_surf;
  pImpl->autolim_min = false; pImpl->autolim_max = false;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimitFloorAuto(bool update)
{
  pImpl->mapbnd[1] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamin_y + pImpl->datamin_x].depth;
  pImpl->autolim_min = true;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimitSurfAuto(bool update)
{
  pImpl->mapbnd[2] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamax_y + pImpl->datamax_x].depth;
  pImpl->autolim_max = true;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}

void DrawMapEx::setProflimitAuto(bool update)
{
  pImpl->mapbnd[1] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamin_y + pImpl->datamin_x].depth;
  pImpl->mapbnd[2] = pImpl->data[pImpl->LIMIT_WIDTH*pImpl->datamax_y + pImpl->datamax_x].depth;
  pImpl->autolim_min = true; pImpl->autolim_max = true;
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_DEPTHS));
}


void DrawMapEx::coordsLL(double* plat, double* plon) const
{
  *plat = pImpl->center.lat;
  *plon = pImpl->center.lon;
}

void DrawMapEx::coordsLL(float x, float y, double* plat, double* plon) const
{
  Q_ASSERT(false);
}

#ifdef DMETERS

float DrawMapEx::metersInPixel() const
{
  return pImpl->range()/float(sizeHorz());
}


void DrawMapEx::viewToMM(float x, float y, bool update)
{
  pImpl->mdx = x;
  pImpl->mdy = y;
  pImpl->cc01[0] = pImpl->mdx;
  pImpl->cc01[1] = pImpl->mdy;
  pImpl->cc01[2] = pImpl->center.ex;
  pImpl->cc01[3] = pImpl->center.ey;
  
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_COORDS));
}

void DrawMapEx::viewToMMRel(float dx, float dy, bool update)
{
  viewToMM(pImpl->mdx + dx, pImpl->mdy + dy, update);
}

void DrawMapEx::relpos(float* x01, float* y01) const
{
  *x01 = 0.5f - pImpl->mdx/metersHorz();
  *y01 = 0.5f + pImpl->mdy/metersVert();
}


#else

void      DrawMapEx::viewToLL(double lat, double lon, bool update)
{
  viewToLLRel(pImpl->center.lat - lat, pImpl->center.lon - lon, update);
}

void      DrawMapEx::viewToLLRel(double dlat, double dlon, bool update)
{
  pImpl->view_dx = dlon / pImpl->delta_lon;
  pImpl->view_dy = dlat / pImpl->delta_lat;
//  pImpl->cc01[0] = 0.5f + pImpl->view_dx/pImpl->LIMIT_WIDTH;
//  pImpl->cc01[1] = 0.5f + pImpl->view_dy/pImpl->LIMIT_HEIGHT;
//  pImpl->cc01[0] = pImpl->center.ex + pImpl->view_dx;
//  pImpl->cc01[1] = pImpl->center.ey + pImpl->view_dy;
  pImpl->cc01[0] = pImpl->view_dx;
  pImpl->cc01[1] = pImpl->view_dy;
  pImpl->cc01[2] = pImpl->center.ex;
  pImpl->cc01[3] = pImpl->center.ey;
  
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_COORDS));
}
void      DrawMapEx::viewToRel(float dx, float dy, bool update)
{
  pImpl->view_dx += dx;
  pImpl->view_dy += dy;
//  pImpl->cc01[0] = 0.5f + pImpl->view_dx/pImpl->LIMIT_WIDTH;
//  pImpl->cc01[1] = 0.5f + pImpl->view_dy/pImpl->LIMIT_HEIGHT;
//  pImpl->cc01[0] = pImpl->center.ex + pImpl->view_dx;
//  pImpl->cc01[1] = pImpl->center.ey + pImpl->view_dy;
  pImpl->cc01[0] = pImpl->view_dx;
  pImpl->cc01[1] = pImpl->view_dy;
  pImpl->cc01[2] = pImpl->center.ex;
  pImpl->cc01[3] = pImpl->center.ey;
//  qDebug()<<dx<<dy<<"       "<<pImpl->cc01[0]<<pImpl->cc01[1]<<"                "<<update<<QString::number(BIT_SET_ON(2), 16);
  if (update)
    DrawQWidget::vmanUpSec(BIT_SET_ON(UP_COORDS));
}

void      DrawMapEx::coordsLLOfViewCenter(double* plat, double* plon) const
{
  *plat = -(((pImpl->center.ey + pImpl->view_dy)*180.0)/pImpl->MAP_Y_SIZE - 90.0);
  *plon = ((pImpl->center.ex + pImpl->view_dx)*360.0/pImpl->MAP_X_SIZE) - 180.0;
}
void      DrawMapEx::coordsEEOfViewCenter(float* px, float* py) const
{
  *px = pImpl->center.ex + pImpl->view_dx;
  *py = pImpl->center.ey + pImpl->view_dy;
}
void      DrawMapEx::coordsOfViewCenter(float* px, float* py) const
{
  *px = pImpl->view_dx;
  *py = pImpl->view_dy;
}
#endif

float DrawMapEx::depthByPIX(float xpix, float ypix, bool* valid) const
{ 
  float xy[2] = { xpix/sizeHorz(), ypix/sizeVert() };
#ifdef DMETERS
  float ptc[2] = { pImpl->center.ex, pImpl->center.ey };
#else
  float ptc[2] = { pImpl->center.ex + pImpl->view_dx, pImpl->center.ey + pImpl->view_dy };
#endif
  float vertsize = pImpl->MAP_Y_SIZE;
  const float ee2rad = 1/60.0*M_PI/180.0;
  
#if 0
  float cos1 = cos((ptc[1] - vertsize/2.0)*ee2rad);
  float sin1 = sin((ptc[1] - vertsize/2.0)*ee2rad);
  float yside = xy[1] < 0.5f? -1 : 1;
  float cos2 = cos((ptc[1] + yside - vertsize/2.0)*ee2rad);
  float sin2 = sin((ptc[1] + yside - vertsize/2.0)*ee2rad);
  float dy = 6378137*atan2( fabs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );
  float yd = (/*1.0 - !!!!!*/xy[1] - 0.5)*(pImpl->range())/ dy;
  
  cos2 = cos((ptc[1] + yd - vertsize/2.0)*ee2rad);
  sin2 = sin((ptc[1] + yd - vertsize/2.0)*ee2rad);
  
  float tgtdist = abs(xy[0] - 0.5)*pImpl->range();
  cos1 = cos2;
  sin1 = sin2;
  float xds = pImpl->LIMIT_WIDTH/2.0;
  float xd = xds;
  for (int i=0; i<16; i++)
  {
    float dlon = xd*ee2rad;
    float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);
    float cp2 = cos2*sin(dlon);
    float curdist = 6378137*atan2( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin2 + cos1*cos2*cos(dlon)) );
    xds = xds/2;
    xd = xd + xds*( tgtdist - curdist < 0 ? -1 : tgtdist - curdist > 0 ? 1 : 0);
  }
  int x = pImpl->LIMIT_WIDTH/2 + pImpl->view_dx + xd;
  int y = pImpl->LIMIT_HEIGHT/2 + pImpl->view_dy + yd;
#else
  float dvert = float(sizeVert())/float(sizeHorz());
  const float ea=6378000;
  const float eb=6357000;
  float yside = xy[1] < 0.5f? -1 : 1;
  float latM = (ptc[1] + yside/2 - vertsize/2.0)*ee2rad;
  float cosM = cos(latM);  float sinM = sin(latM);
  float mympart = sqrt((ea*cosM)*(ea*cosM) + (eb*sinM)*(eb*sinM));
  float MyM=(ea*eb)*(ea*eb)/(mympart*mympart*mympart);
  float dy = MyM*ee2rad;
  float yd = (xy[1] - 0.5)*pImpl->range()*dvert / dy;  // /1855
  
  float latT = (ptc[1] + yd - vertsize/2.0)*ee2rad;
  float cosT = cos(latT);  float sinT = sin(latT);
  float Ny=(ea*ea)/sqrt((ea*cosT)*(ea*cosT) + (eb*sinT)*(eb*sinT));
  float tgtdist = abs(xy[0] - 0.5)*pImpl->range();
  float dx = Ny*cos((pImpl->center.ey-vertsize/2.0)*ee2rad)*ee2rad;
  float xd = tgtdist/dx*(xy[0] < 0.5f ? -1 : xy[0] > 0.5f ? 1 : 0);
  
#ifdef DMETERS
  int x = pImpl->LIMIT_WIDTH/2 + xd + pImpl->mdx/dx;
  int y = pImpl->LIMIT_HEIGHT/2 + yd + pImpl->mdy/dy;
#else
  int x = pImpl->LIMIT_WIDTH/2 + pImpl->view_dx + xd;
  int y = pImpl->LIMIT_HEIGHT/2 + pImpl->view_dy + yd;
#endif
#endif
  bool inside = true; 
  if (x < 0){ x = 0; inside = false; }
  if (x >= pImpl->LIMIT_WIDTH){ x = pImpl->LIMIT_WIDTH-1; inside = false; }
  if (y < 0){ y = 0; inside = false; }
  if (y >= pImpl->LIMIT_HEIGHT){ y = pImpl->LIMIT_HEIGHT-1; inside = false; }
  
  if (valid)  *valid = inside;
  return pImpl->data[y*pImpl->LIMIT_WIDTH + x].depth;
}

void      DrawMapEx::coordsOOByPix(float px, float py, float* pdx, float* pdy) const
{
  float w = this->sizeHorz(), h = this->sizeVert();
  *pdx = px/float(w);
  *pdy = py/float(h);
}

float     DrawMapEx::distanceOO(float x1, float y1, float x2, float y2) const
{
  float dvert = float(sizeVert())/float(sizeHorz());
  float dx = (x2-x1)*(pImpl->range());
  float dy = (y2-y1)*(pImpl->range()*dvert);
  return sqrt(dx*dx + dy*dy);
}

float DrawMapEx::distanceByTop() const
{
  return pImpl->range();
}

float DrawMapEx::distanceByBottom() const
{
  return pImpl->range();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////

#define BIT_CHECK_OFF(IDX) ((secflags & (1<<IDX)) == 0)

void DrawMapEx::processGlLocation(int secidx, int secflags, int loc, int TEX)
{
  if (secidx == SDT_SCTABLE)
  { 
    if (BIT_CHECK_OFF(UP_SCTABLE))
      return;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
    glTexImage2D(  GL_TEXTURE_2D, 0, gl_internalFormat, 1 + pImpl->MAP_Y_SIZE/2, 1, 0, gl_format, gl_texture_type, pImpl->sctex);
    
    m_ShaderProgram.setUniformValue(loc, TEX);
  }
  else if (secidx == SDT_MAP)
  { 
    if (BIT_CHECK_OFF(UP_MAP))
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
  else if (secidx == SDT_MAPBOUND1 || secidx == SDT_MAPBOUND2)
  {
    if (BIT_CHECK_OFF(UP_MAP))
      return;
    m_ShaderProgram.setUniformValue(loc, secidx == SDT_MAPBOUND1 ? pImpl->LIMIT_WIDTH : pImpl->LIMIT_HEIGHT);
  }
  else if (secidx == SDT_RANGE)
  {
    if (BIT_CHECK_OFF(UP_RANGE))
      return;
    float range = pImpl->range();
    m_ShaderProgram.setUniformValue(loc, range);
  }
  else if (secidx == SDT_COORDS)
  {
    if (BIT_CHECK_OFF(UP_COORDS))
      return;
//    qDebug()<<"IN latlon        "<<pImpl->center.lat<<pImpl->center.lon;
//    qDebug()<<"IN IT!!!         "<<pImpl->cc01[0]<<pImpl->cc01[1]<<pImpl->cc01[2]<<pImpl->cc01[3];
    m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)pImpl->cc01);
  }
  else if (secidx == SDT_PALBORD)
  {
    if (BIT_CHECK_OFF(UP_PALBORD))
      return;
    m_ShaderProgram.setUniformValue(loc, *(const QVector4D*)pImpl->palbnd);
  }
  else if (secidx == SDT_DEPTHS)
  {
    if (BIT_CHECK_OFF(UP_DEPTHS))
      return;
    m_ShaderProgram.setUniformValue(loc, *(const QVector3D*)pImpl->mapbnd);
  }
//  else if (secidx == 6)
//  {
//    if (BIT_CHECK_OFF(4))
//      return;
//  }
}

void DrawMapEx::sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const
{
  *matrixDimmA = m_dataDimmA;
  *matrixDimmB = m_dataDimmB;
  *scalingA = (unsigned int)sizeA <= m_dataDimmA? 1 : (sizeA / m_dataDimmA);
  *scalingB = (unsigned int)sizeB <= m_dataDimmB? 1 : (sizeB / m_dataDimmB);
  clampScaling(scalingA, scalingB);
}

int DrawMapEx::sizeAndScaleChanged(bool changedDimmA, bool changedDimmB, bool changedScalingA, bool changedScalingB)
{
  if (changedDimmA || changedDimmB || changedScalingA || changedScalingB)
    emit sig_resized();
}






_MapReactorZoom::_MapReactorZoom(bool applyZoom, QObject* parent): QObject(parent)
{
  doAppZoom = applyZoom;
}

bool _MapReactorZoom::reactionWheel(DrawQWidget* draw, OVL_REACTION_WHEEL orm, const coordstriumv_t* ct, bool*)
{
  DrawMapEx* self = (DrawMapEx*)draw;
  
  if (orm == ORW_AWAY || orm == ORW_TOWARD)
  {
    float mulc = orm == ORW_AWAY ? 1.05f : 1.0f/1.05f;
    if (doAppZoom)
    {
      float nextzoom = self->zoom()*mulc;
      if (nextzoom < 0.0001f)     nextzoom = 0.0001f;
      else if (nextzoom > 24.0f)   nextzoom = 24.0f;
        self->setZoom(nextzoom);
      emit zoomChanged(nextzoom);
    }
    else
    {
      emit zoomChanged(mulc);
    }
    return true;
  }
  return false;
}




MapExReactor::MapExReactor(bool applyZoom, QObject* parent): _MapReactorZoom(applyZoom, parent)
{
}

bool MapExReactor::reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool*)
{
  DrawMapEx* self = (DrawMapEx*)draw;
  
  float x = ct->fx_pix;
  float y = ct->fy_pix;
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
  {
    if (orm == ORM_LMMOVE)
    {
      self->viewToMMRel((lx - x)*self->metersInPixel(), (ly - y)*self->metersInPixel());
    }
    lx = x;
    ly = y;
    if (orm == ORM_LMMOVE)
    {
      double lat, lon;
      self->coordsLL(&lat, &lon);
      emit coordsChanged(lat, lon);
    }
    return true;
  }
  
  if (orm == ORM_RMPRESS)
  {
  }
  return false;
}

/////////////////////////////////


MapExReactorSkol::MapExReactorSkol(bool applyZoom, QObject* parent): _MapReactorZoom(applyZoom, parent)
{
}

bool MapExReactorSkol::reactionMouse(DrawQWidget *draw, OVL_REACTION_MOUSE orm, const coordstriumv_t *ct, bool *)
{
  DrawMapEx* self = (DrawMapEx*)draw;
  
  float x = ct->fx_pix;
  float y = ct->fy_pix;
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
  {
    double lat, lon;
    self->coordsLL(&lat, &lon);
    emit skolChanged(ct->fx_01, ct->fy_01, lat, lon);
    return true;
  }
  else if (orm == ORM_RMPRESS)
  {
  }
  return false;
}


/////////////////////////////////

MapExReactorClickable::MapExReactorClickable(bool applyZoom, QObject* parent): _MapReactorZoom(applyZoom, parent)
{
  qel = new QElapsedTimer();
}

MapExReactorClickable::~MapExReactorClickable()
{
  delete qel;
}

bool MapExReactorClickable::reactionMouse(DrawQWidget* draw, OVL_REACTION_MOUSE orm, const coordstriumv_t* ct, bool*)
{
  DrawMapEx* self = (DrawMapEx*)draw;
  
  float x = ct->fx_pix;
  float y = ct->fy_pix;
  if (orm == ORM_LMPRESS || orm == ORM_LMMOVE)
  {
    if (orm == ORM_LMPRESS)
    {
      qel->start();
    }
    if (orm == ORM_LMMOVE)
    {
      qel->invalidate();
      self->viewToMMRel((lx - x)*self->metersInPixel(), (ly - y)*self->metersInPixel());
    }
    lx = x;
    ly = y;
    if (orm == ORM_LMMOVE)
    {
      double lat, lon;
      self->coordsLL(&lat, &lon);
      emit coordsChanged(lat, lon);
    }
    return true;
  }
  if (orm == ORM_LMRELEASE)
  {
    if (qel->isValid() && qel->elapsed() < 400)
    {
      double lat, lon;
      self->coordsLL(&lat, &lon);
      emit clicked(ct->fx_pix, ct->fy_pix, lat, lon);
      return true;
    }
  }
  
  
  if (orm == ORM_RMPRESS)
  {
  }
  return false;
}




/*

    FshDrawConstructor fmg(to, allocatedPortions, splitPortions, imp, sizeof(globvars)/sizeof(globvars[0]), globvars, ovlscount, ovlsinfo);
    fdc.getLocbacks(locbackinfo, locbackcount);
    fdc.main_begin(FshDrawConstructor::INITBACK_BYPALETTE, 0, orient, fsp); //FshDrawConstructor::INITBACK_BYZERO
    fdc.cintvar("vertsize", vertsize);
    fdc.cfloatvar("rr", 500000.0f);
//    fdc.cfloatvar("rr", 50.0f);
    
    fdc.push("vec2 ptc = vec2(ddcc[0] + ddcc[2], ddcc[1] + ddcc[3]);" SHNL);
    fdc.push("float ytgtdist = abs(xy_coords.y - 0.5)*rr;" SHNL);
    fdc.push("float yds = mapbound_b/2;" SHNL);
    fdc.push("float yd = 0;" SHNL);
//    fdc.push("vec2 cs_lat1 = texture(sctsampler, vec2(abs(ptc.y - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL);
//    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*1/60.0*3.1415927/180.0);" SHNL);
//    fdc.push("float sin1 = cos((ptc.y-vertsize/2.0)*1/60.0*3.1415927/180.0);" SHNL);
    fdc.push("vec2 cs_lat1 = vec2(cos((ptc.y-vertsize/2.0)*1/60.0*3.1415927/180.0), sin((ptc.y-vertsize/2.0)*1/60.0*3.1415927/180.0));" SHNL);
    fdc.push("vec2 cs_lat2;" SHNL);
    
    fdc.push("for (int i=0; i<30; i++)" SHNL
             "{" SHNL
    );                                                                    //mix(yd, -yd, step(0.5, xy_coords.y)) 
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
    fdc.push("float a = (ptc.y + yd - vertsize/2.0)*1/60.0*3.1415927/180.0;" SHNL);
    fdc.push("cs_lat2 = vec2(cos(a), sin(a));" SHNL);
    fdc.push("float cp1 = cs_lat1[0]*cs_lat2[1] - cs_lat1[1]*cs_lat2[0];" SHNL);
    fdc.push("float curdist = 6378137*atan( abs(cp1)/(cs_lat1[1]*cs_lat2[1] + cs_lat1[0]*cs_lat2[0]) );" SHNL);
//    fdc.push("if (abs(curdist - ytgtdist) < 100)  break;" SHNL);
    fdc.push("yds = yds/2;" SHNL);
    fdc.push("yd = yd + mix(-yds, yds, step(abs(curdist), ytgtdist))*mix(1, -1, step(0.5, xy_coords.y));" SHNL);
    fdc.push("}" SHNL);

#if 0
    fdc.push("float xtgtdist = abs(xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xds = mapbound_a/2;" SHNL);
    fdc.push("float xd = 0;" SHNL);                                     //mix(yd, -yd, step(0.5, xy_coords.y))
    fdc.push("for (int i=0; i<20; i++)" SHNL
             "{" SHNL
    );
    fdc.push("float dlon = xd*1/60.0*3.1415927/180.0;" SHNL);
    fdc.push("float cp1 = cs_lat1[0]*cs_lat2[1] - cs_lat1[1]*cs_lat2[0]*cos(dlon);" SHNL);
    fdc.push("float cp2 = cs_lat2[0]*sin(dlon);" SHNL);
    fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2)/(cs_lat1[1]*cs_lat2[1] + cs_lat1[0]*cs_lat2[0]*cos(dlon)) );" SHNL);
//    fdc.push("curdist = rr/20*i;" SHNL);
    
//    fdc.push("if (abs(curdist - xtgtdist) < 100)  break;" SHNL);
    fdc.push("xds = xds/2;" SHNL);
    fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), xtgtdist))*mix(1, -1, step(0.5, xy_coords.x));" SHNL);
    fdc.push("}" SHNL);
#elif 1
    fdc.push("float dlon = 1/60.0*3.1415927/180.0;" SHNL);
    fdc.push("float cp1 = cs_lat2[0]*cs_lat2[1] - cs_lat2[1]*cs_lat2[0]*cos(dlon);" SHNL);
    fdc.push("float cp2 = cs_lat2[0]*sin(dlon);" SHNL);
    fdc.push("float mindist = 6378137*atan(sqrt(cp1*cp1 + cp2*cp2)/(cs_lat2[1]*cs_lat2[1] + cs_lat2[0]*cs_lat2[0]*cos(dlon)));" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
#elif 0
    fdc.push("float dlon = 1/60.0*3.1415927/180.0;" SHNL);
    fdc.push("float cp1 = cs_lat1[0]*cs_lat2[1] - cs_lat1[1]*cs_lat2[0]*cos(dlon);" SHNL);
    fdc.push("float cp2 = cs_lat2[0]*sin(dlon);" SHNL);
    fdc.push("float mindist = 6378137*atan(sqrt(cp1*cp1 + cp2*cp2)/(cs_lat1[1]*cs_lat2[1] + cs_lat1[0]*cs_lat2[0]*cos(dlon)));" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
#elif 1
    fdc.push("float xd = (xy_coords.x - 0.5)*mapbound_a;" SHNL);
#endif
*/
/*

  
#if 0
    fdc.push("float xtgtdist = abs(xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xds = mapbound_a/2;" SHNL);
    fdc.push("float xd = 0;" SHNL);                                     //mix(yd, -yd, step(0.5, xy_coords.y))
    fdc.push("for (int i=0; i<20; i++)" SHNL
             "{" SHNL
    );
    fdc.push("float dlon = xd*ee2rad;" SHNL);
    fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2)/(sin1*sin2 + cos1*cos2*cos(dlon)) );" SHNL);
//    fdc.push("curdist = rr/20*i;" SHNL);
    
//    fdc.push("if (abs(curdist - xtgtdist) < 100)  break;" SHNL);
    fdc.push("xds = xds/2;" SHNL);
    fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), xtgtdist))*mix(1, -1, step(0.5, xy_coords.x));" SHNL);
    fdc.push("}" SHNL);
#elif 1


#elif 0
    fdc.push("float dlon = ee2rad;" SHNL);
    fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float mindist = 6378137*atan(sqrt(cp1*cp1 + cp2*cp2)/(sin1*sin2 + cos1*cos2*cos(dlon)));" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
#elif 1
    fdc.push("float xd = (xy_coords.x - 0.5)*mapbound_a;" SHNL);
*/

#if 0

#elif 0
    fdc.push("float a = (ptc.y - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("cos2 = cos(a);" SHNL);
    fdc.push("sin2 = sin(a);" SHNL);
    fdc.push("float dlon = ee2rad;" SHNL);
    fdc.push("float cp1 = cos2*sin2 - sin2*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float adist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin2*sin2 + cos2*cos2*cos(dlon)) );" SHNL);
    
    
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
    
    
    
    fdc.push("float ac = ;" SHNL);
    fdc.push("float dlon = ee2rad;" SHNL);
    fdc.push("float cp1 = cos1*sin1 - sin1*cos1*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos1*sin(dlon);" SHNL);
    fdc.push("xx[i] = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin1 + cos1*cos1*cos(dlon)) );" SHNL);
    
    
    fdc.push("for (int i=0; i<2; i++)" SHNL
             "{" SHNL
    ); 
    {
      fdc.push("float b = (1+i)*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin1 - sin1*cos1*cos(b);" SHNL);
      fdc.push("float cp2 = cos1*sin(b);" SHNL);
      fdc.push("xx[i] = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin1 + cos1*cos1*cos(b)) );" SHNL);
    }
    fdc.push("}" SHNL);
//    fdc.push("float xs = log(xx[1]) / log(xx[0]);" SHNL);
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xd = log(xtgtdist)/log(xx[0]) + 1;" SHNL);
    fdc.push("float xs = xx[1]/xx[0];" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/xx[0];" SHNL);
    
    
    
    fdc.push("float ytgtdist = abs(xy_coords.y - 0.5)*rr;" SHNL);
    fdc.push("float yds = mapbound_b/2;" SHNL);
    fdc.push("float yd = 0;" SHNL);
    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float cos2, sin2;" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    
    fdc.push("for (int i=0; i<16; i++)" SHNL
             "{" SHNL
    ); 
    {
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
      fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
      fdc.push("cos2 = cos(a);" SHNL);
      fdc.push("sin2 = sin(a);" SHNL);
      fdc.push("float curdist = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
//    fdc.push("if (abs(curdist - ytgtdist) < 100)  break;" SHNL);
      fdc.push("yds = yds/2;" SHNL);
      fdc.push("yd = yd + mix(-yds, yds, step(abs(curdist), ytgtdist)) * yside;" SHNL);
    }
    fdc.push("}" SHNL);
    
    fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("cos2 = cos(a);" SHNL);
    fdc.push("sin2 = sin(a);" SHNL);
    
    fdc.push("float dlon = ee2rad;" SHNL);
    fdc.push("float cp1 = cos2*sin2 - sin2*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float mindist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin2*sin2 + cos2*cos2*cos(dlon)) );" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
    
    fdc.push("float tgtdist = length(xy_coords - vec2(0.5))*rr;" SHNL);
    fdc.push("float xside = -2*step(0.5, xy_coords.x) + 1;" SHNL);
    fdc.push("float xds = xd/16;" SHNL);
    
    fdc.push("for (int i=0; i<8; i++)" SHNL
             "{" SHNL
    );
    {
      fdc.push("dlon = xd*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
      fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
      fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin2 + cos1*cos2*cos(dlon)) );" SHNL);
      fdc.push("xds = xds/2;" SHNL);
      fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), tgtdist)) * xside;" SHNL);
    }
    fdc.push("}" SHNL);
#elif 0
    fdc.push("float ytgtdist = abs(xy_coords.y - 0.5)*rr;" SHNL);
    fdc.push("float yds = mapbound_b/2;" SHNL);
    fdc.push("float yd = 0;" SHNL);
    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float cos2, sin2;" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    
    fdc.push("for (int i=0; i<16; i++)" SHNL
             "{" SHNL
    ); 
    {
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
      fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
      fdc.push("cos2 = cos(a);" SHNL);
      fdc.push("sin2 = sin(a);" SHNL);
      fdc.push("float curdist = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
//    fdc.push("if (abs(curdist - ytgtdist) < 100)  break;" SHNL);
      fdc.push("yds = yds/2;" SHNL);
      fdc.push("yd = yd + mix(-yds, yds, step(abs(curdist), ytgtdist)) * yside;" SHNL);
    }
    fdc.push("}" SHNL);
    
    fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
//    fdc.push("cos2 = cos(a);" SHNL);
//    fdc.push("sin2 = sin(a);" SHNL);
    fdc.push("cos1 = cos(a);" SHNL);
    fdc.push("sin1 = sin(a);" SHNL);
    
    fdc.push("float xx[2];" SHNL);
    fdc.push("for (int i=0; i<2; i++)" SHNL
             "{" SHNL
    ); 
    {
      fdc.push("float b = (1+i)*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin1 - sin1*cos1*cos(b);" SHNL);
      fdc.push("float cp2 = cos1*sin(b);" SHNL);
      fdc.push("xx[i] = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin1 + cos1*cos1*cos(b)) );" SHNL);
    }
    fdc.push("}" SHNL);
//    fdc.push("float xs = log(xx[1]) / log(xx[0]);" SHNL);
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xd = log(xtgtdist)/log(xx[0]) + 1;" SHNL);
    fdc.push("float xs = xx[1]/xx[0];" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/xx[0];" SHNL);
    
    
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xz = 1.0/log(xx[1])*log(xx[0]);" SHNL);
//    fdc.push("float xd = 1/log(xz)*log(abs(xtgtdist));" SHNL);
    
//    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
//    fdc.push("float yd = ytgtdist/(yy[0]*ys);" SHNL);
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xd = xtgtdist/(xx[0]*xs);" SHNL);
//    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
//    fdc.push("float yz = log(ys);" SHNL);
//    fdc.push("float yd = 1/yz*log(abs(ytgtdist));" SHNL);
    
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xz = log(xs);" SHNL);
//    fdc.push("float xd = 1/xz*log(abs(xtgtdist));" SHNL);
    
//    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
//    fdc.push("float yd = ytgtdist/(yy[0]*ys)*4;" SHNL);
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xd = xtgtdist/(xx[0]*xs)*4;" SHNL);
    
#elif 0
    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float cos2, sin2;" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    
    fdc.push("float yy[2];" SHNL);
    fdc.push("for (int i=0; i<2; i++)" SHNL
             "{" SHNL
    ); 
    {
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
      fdc.push("float a = (ptc.y + (1+i) - vertsize/2.0)*ee2rad;" SHNL);
      fdc.push("cos2 = cos(a);" SHNL);
      fdc.push("sin2 = sin(a);" SHNL);
      fdc.push("yy[i] = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
    }
    fdc.push("}" SHNL);
    fdc.push("float ys = yy[1] / yy[0];" SHNL);
    
    fdc.push("float xx[2];" SHNL);
    fdc.push("for (int i=0; i<2; i++)" SHNL
             "{" SHNL
    ); 
    {
      fdc.push("float b = (1+i)*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin1 - sin1*cos1*cos(b);" SHNL);
      fdc.push("float cp2 = cos1*sin(b);" SHNL);
      fdc.push("xx[i] = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin1 + cos1*cos1*cos(b)) );" SHNL);
    }
    fdc.push("}" SHNL);
    fdc.push("float xs = xx[1] / xx[0];" SHNL);
    
//    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
//    fdc.push("float yd = ytgtdist/(yy[0]*ys);" SHNL);
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xd = xtgtdist/(xx[0]*xs);" SHNL);
//    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
//    fdc.push("float yz = log(ys);" SHNL);
//    fdc.push("float yd = 1/yz*log(abs(ytgtdist));" SHNL);
    
//    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
//    fdc.push("float xz = log(xs);" SHNL);
//    fdc.push("float xd = 1/xz*log(abs(xtgtdist));" SHNL);
    
    fdc.push("float ytgtdist = ((1.0 - xy_coords.y) - 0.5)*rr;" SHNL);
    fdc.push("float yd = ytgtdist/(yy[0]*ys)*4;" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/(xx[0]);" SHNL);
    
#elif 0
    fdc.push("float ytgtdist = abs(xy_coords.y - 0.5)*rr;" SHNL);
    fdc.push("float yds = mapbound_b/2;" SHNL);
    fdc.push("float yd = 0;" SHNL);
    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float cos2, sin2;" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    
    fdc.push("for (int i=0; i<16; i++)" SHNL
             "{" SHNL
    ); 
    {
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
      fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
      fdc.push("cos2 = cos(a);" SHNL);
      fdc.push("sin2 = sin(a);" SHNL);
      fdc.push("float curdist = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
//    fdc.push("if (abs(curdist - ytgtdist) < 100)  break;" SHNL);
      fdc.push("yds = yds/2;" SHNL);
      fdc.push("yd = yd + mix(-yds, yds, step(abs(curdist), ytgtdist)) * yside;" SHNL);
    }
    fdc.push("}" SHNL);
    
    fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("cos2 = cos(a);" SHNL);
    fdc.push("sin2 = sin(a);" SHNL);
    
    fdc.push("float tgtdist = length(xy_coords - vec2(0.5))*rr;" SHNL);
//    fdc.push("float tgtdist = abs(xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xds = mapbound_a/2;" SHNL);
    fdc.push("float xd = 0;" SHNL);
    fdc.push("float xside = 2*step(0.5, xy_coords.x) - 1;" SHNL);
    
    fdc.push("for (int i=0; i<16; i++)" SHNL
             "{" SHNL
    );
    {
      fdc.push("float b = xd*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(b);" SHNL);
      fdc.push("float cp2 = cos2*sin(b);" SHNL);
      fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin2 + cos1*cos2*cos(b)) );" SHNL);
      fdc.push("xds = xds/2;" SHNL);
      fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), tgtdist)) * xside;" SHNL);
    }
    fdc.push("}" SHNL);
#elif 0
    fdc.push("float ytgtdist = abs(xy_coords.y - 0.5)*rr;" SHNL);
    fdc.push("float yds = mapbound_b/2;" SHNL);
    fdc.push("float yd = 0;" SHNL);
    fdc.push("float cos1 = cos((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y-vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float cos2, sin2;" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    
    fdc.push("for (int i=0; i<16; i++)" SHNL
             "{" SHNL
    ); 
    {
//    fdc.push("cs_lat2 = texture(sctsampler, vec2(abs(ptc.y + yd - vertsize/2.0)/(1.0 + vertsize/2.0), 0.0)).rg;" SHNL); //mix(yd, -yd, step(0.5, xy_coords.y)) 
      fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
      fdc.push("cos2 = cos(a);" SHNL);
      fdc.push("sin2 = sin(a);" SHNL);
      fdc.push("float curdist = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
//    fdc.push("if (abs(curdist - ytgtdist) < 100)  break;" SHNL);
      fdc.push("yds = yds/2;" SHNL);
      fdc.push("yd = yd + mix(-yds, yds, step(abs(curdist), ytgtdist)) * yside;" SHNL);
    }
    fdc.push("}" SHNL);
    
    fdc.push("float a = (ptc.y + yd - vertsize/2.0)*ee2rad;" SHNL);
    fdc.push("cos2 = cos(a);" SHNL);
    fdc.push("sin2 = sin(a);" SHNL);
    
    fdc.push("float dlon = ee2rad;" SHNL);
    fdc.push("float cp1 = cos2*sin2 - sin2*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float mindist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin2*sin2 + cos2*cos2*cos(dlon)) );" SHNL);
    fdc.push("float xtgtdist = (xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xd = xtgtdist/mindist;" SHNL);
    
    fdc.push("float tgtdist = length(xy_coords - vec2(0.5))*rr;" SHNL);
    fdc.push("float xside = -2*step(0.5, xy_coords.x) + 1;" SHNL);
    fdc.push("float xds = xd/16;" SHNL);
    
    fdc.push("for (int i=0; i<8; i++)" SHNL
             "{" SHNL
    );
    {
      fdc.push("dlon = xd*ee2rad;" SHNL);
      fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
      fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
      fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin2 + cos1*cos2*cos(dlon)) );" SHNL);
      fdc.push("xds = xds/2;" SHNL);
      fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), tgtdist)) * xside;" SHNL);
    }
    fdc.push("}" SHNL);
#endif
    
    
#if 0
    fdc.push("float xtgtdist = abs(xy_coords.x - 0.5)*rr;" SHNL);
    fdc.push("float xds = mapbound_a/2;" SHNL);
    fdc.push("float xd = 0;" SHNL);                                     //mix(yd, -yd, step(0.5, xy_coords.y))
    fdc.push("for (int i=0; i<20; i++)" SHNL
             "{" SHNL
    );
    fdc.push("float dlon = xd*ee2rad;" SHNL);
    fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
    fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
    fdc.push("float curdist = 6378137*atan( sqrt(cp1*cp1 + cp2*cp2)/(sin1*sin2 + cos1*cos2*cos(dlon)) );" SHNL);
//    fdc.push("curdist = rr/20*i;" SHNL);
    
//    fdc.push("if (abs(curdist - xtgtdist) < 100)  break;" SHNL);
    fdc.push("xds = xds/2;" SHNL);
    fdc.push("xd = xd + mix(-xds, xds, step(abs(curdist), xtgtdist))*mix(1, -1, step(0.5, xy_coords.x));" SHNL);
    fdc.push("}" SHNL);
#endif

    
    
#if 0
    fdc.push("float cos1 = cos((ptc.y - vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin1 = sin((ptc.y - vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float yside = -2*step(0.5, xy_coords.y) + 1;" SHNL);
    fdc.push("float cos2 = cos((ptc.y + yside - vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float sin2 = sin((ptc.y + yside - vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("float dy = 6378137*atan( abs(cos1*sin2 - sin1*cos2), (sin1*sin2 + cos1*cos2) );" SHNL);
    
    fdc.push("float yd = (1.0 - xy_coords.y - 0.5)*range / dy;" SHNL);
    fdc.push("cos2 = cos((ptc.y + yd - vertsize/2.0)*ee2rad);" SHNL);
    fdc.push("sin2 = sin((ptc.y + yd - vertsize/2.0)*ee2rad);" SHNL);
    
#if 0
    fdc.push("float tgtdist = length(xy_coords - vec2(0.5))*range;" SHNL);
    fdc.push("float xds = 1.4*mapbound_a/2.0;" SHNL);
#else
    fdc.push("float tgtdist = abs(xy_coords.x - 0.5)*range;" SHNL);
    fdc.push("cos1 = cos2;" SHNL);
    fdc.push("sin1 = sin2;" SHNL);
    fdc.push("float xds = mapbound_a/2.0;" SHNL);
#endif
    
    fdc.push("float xside = 2*step(0.5, xy_coords.x) - 1;" SHNL);
    fdc.push("float xd = xds;" SHNL);
    fdc.push("for (int i=0; i<iterations; i++)" SHNL
             "{" SHNL
    );
    {
      fdc.push("float dlon = xd*ee2rad;" SHNL); //*abs(cos(xd*ee2rad*6))
      fdc.push("float cp1 = cos1*sin2 - sin1*cos2*cos(dlon);" SHNL);
      fdc.push("float cp2 = cos2*sin(dlon);" SHNL);
      fdc.push("float carad = atan( sqrt(cp1*cp1 + cp2*cp2), (sin1*sin2 + cos1*cos2*cos(dlon)) );" SHNL);
      fdc.push("float curdist = 6378137*carad;" SHNL);
      fdc.push("xds = xds/2;" SHNL);
      fdc.push("xd = xd + xds*sign(tgtdist - curdist);" SHNL);
    }
    fdc.push("}" SHNL);
    fdc.push("xd = xd*sign(xy_coords.x - 0.5);" SHNL);
    
    fdc.push("vec2  eecoords = vec2(ddcc[0] + xd + mapbound_a/2.0, ddcc[1] + yd + mapbound_b/2.0);" SHNL);
    fdc.push("vec2  tcoords = eecoords/vec2(mapbound_a, mapbound_b);" SHNL);  // + cc.xy
    
    fdc.push("vec3  mpx = texture(mapsampler, tcoords).rgb;" SHNL);
    fdc.push("float level = mpx[0] + mapdepbord[0];" SHNL);
    fdc.push("dvalue = mix(  mappalbord[0] + (mappalbord[1] - mappalbord[0])*(1.0 - level/mapdepbord[1]), "
                            "mappalbord[2] + (mappalbord[3] - mappalbord[2])*(level/mapdepbord[2]),"
                            "step(0.0, level) );" SHNL);
    //fdc.push("dvalue = paletrange[0] + (paletrange[1] - paletrange[0])*dvalue;" SHNL);
    fdc.push("result = result + texture(paletsampler, vec2(dvalue, 0.0)).rgb;" SHNL);
#endif
    
