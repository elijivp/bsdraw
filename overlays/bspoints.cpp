#include "bspoints.h"

#include "../core/sheigen/bsshgentrace.h"
#include <math.h>
#include <memory.h>

OPTFill::OPTFill(const linestyle_t& kls): IOverlaySimple(),
  OVLCoordsDynamic(CR_RELATIVE, 0.0f, 0.0f), OVLDimmsOff(), m_fill(kls)
{
}
  

int OPTFill::fshTrace(int overlay, bool rotated, char *to) const
{ 
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.goto_normed();
  ocg.var_fixed("clr", m_fill.r, m_fill.g, m_fill.b);
  ocg.push("result = clr; mixwell = step(distance(vec2(inormed), vec2(0.0,0.0)), 1.0);");
  ocg.goto_func_end(false);
  return ocg.written();
}


///////////////////////////////////////////////////////////



ODropPoints::ODropPoints(unsigned int ptlimit, COORDINATION, float, const linestyle_t& kls): IOverlayTraced(kls),
  OVLCoordsOff(), OVLDimmsOff(), ptCountMax(ptlimit)
{
  ptdrops = new ovlcoords_t[ptCountMax];
  memset(ptdrops, 0, sizeof(ovlcoords_t)*ptCountMax);
//  ptRound[0] = ptRound[1] = 0;
  ptStart = ptCount = 0;
  _dm_coords.count = ptCountMax;
  _dm_coords.data = ptdrops;
  appendUniform(DT_ARR2, &_dm_coords);
  appendUniform(DT_1I, &ptStart);
  appendUniform(DT_1I, &ptCount);
}
  

int ODropPoints::fshTrace(int overlay, bool rotated, char *to) const
{ 
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    int ptpixing = ocg.add_movecs_pixing(CR_RELATIVE);
    
    ocg.param_for_rarr_begin("point");
    {
      ocg.goto_normed("point.xy", ptpixing);
      ocg.push("_densvar = (1.0+density - clamp(distance(vec2(inormed), vec2(0.0,0.0)), 0.0, 1.0+density))/(1.0+density);");
      ocg.push("_insvar = vec3(_densvar, sign(_densvar), sign(_densvar)) * vec3(1, float(i)/float(rarr_len), rarr_len);");
      ocg.push("result = mix(result, _insvar, 1 - step(abs(_insvar[0]) - abs(result[0]), 0.0) );");
    }
    ocg.param_for_end();
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

bool ODropPoints::overlayReaction(OVL_REACTION oreact, const void* dataptr, bool*)
{
  if (oreact == OR_LMPRESS || oreact == OR_LMMOVE)
  {
    int placeTo = ptCount;
    if (ptCount == ptCountMax)
    {
      placeTo = ptStart;
      if (++ptStart >= ptCountMax)
        ptStart = 0;
    }
    else
      ptCount++;
    ptdrops[placeTo].x = ((float*)dataptr)[0];
    ptdrops[placeTo].y = ((float*)dataptr)[1];
    return true;
  }
  return false;
}


/******************/

DropsBase_::DropsBase_(unsigned int countPointsMax, const linestyle_t& kls): IOverlayTraced(kls),
  OVLCoordsStatic(CR_RELATIVE, 0.0f, 0.0f), OVLDimmsOff(), ptCountMax(countPointsMax)
{
  ptdrops = new ovlcoords_t[ptCountMax];
  memset(ptdrops, 0, sizeof(ovlcoords_t)*ptCountMax);
  ptCount = 0;
  _dm_coords.count = ptCountMax;
  _dm_coords.data = ptdrops;
  appendUniform(DT_ARR2, &_dm_coords);
  appendUniform(DT_1I, &ptCount);
}

int DropsBase_::fshTrace(int overlay, bool rotated, char *to) const
{ 
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    int ptpixing = ocg.add_movecs_pixing(CR_RELATIVE);
    
    ocg.param_for_oarr_begin("point", "nextpoint");
    {
      ocg.goto_normed("point.xy", ptpixing, true);
      ocg.var_static(DT_2F, "point_temp=nextpoint");
      ocg.movecs_pix("nextpoint", ptpixing);
      ocg.push("nextpoint = nextpoint - ioffset;");
      ocg.trace_line_from_normed_to("nextpoint");
      ocg.push("point = point_temp;");
    }
    ocg.param_for_end();
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

////////////////////////////////
ODropLine::ODropLine(unsigned int maxpoints, bool lastFollowsMouse, const linestyle_t& kls): DropsBase_(maxpoints, kls), followMoving(lastFollowsMouse)
{
}

bool ODropLine::overlayReaction(OVL_REACTION oreact, const void* dataptr, bool*)
{
  bool result = false;
  if (oreact == OR_LMPRESS)
  {
    if (ptCount == ptCountMax)
      ptCount--;
    ptdrops[ptCount++] = ovlcoords_t(((float*)dataptr)[0], ((float*)dataptr)[1]);
    result = true;
  }
  else if (oreact == OR_LMMOVE && followMoving)
  {
    if (ptCount == 1 && followMoving)
      ptdrops[ptCount++] = ovlcoords_t(((float*)dataptr)[0], ((float*)dataptr)[1]);
    else
    {
      ptdrops[ptCount-1].x = ((float*)dataptr)[0];
      ptdrops[ptCount-1].y = ((float*)dataptr)[1];
    }
    result = true;
  }
  else if (oreact == OR_LMRELEASE)
  {
    m_coords = ovlcoords_t(0.0f, 0.0f);
    for (unsigned int i=0; i<ptCount; i++)
    {
      m_coords.x += ptdrops[i].x;
      m_coords.y += ptdrops[i].y;
    }
    m_coords.x /= ptCount;
    m_coords.y /= ptCount;
  }
  if (result)
    overlayUpdateParameter();
  return result;
}

/////////////////////////////////////
OBrush::OBrush(unsigned int memoryPoints, const linestyle_t& kls): DropsBase_(memoryPoints, kls)
{
}

bool OBrush::overlayReaction(OVL_REACTION oreact, const void* dataptr, bool*)
{
  if (oreact == OR_LMPRESS || oreact == OR_LMMOVE)
  {
    if (ptCount == ptCountMax)
      return false;
    if (fabs(((float*)dataptr)[0] - ((float*)dataptr)[1]) < 0.001f)
      return false;
    ptdrops[ptCount++] = ovlcoords_t(((float*)dataptr)[0], ((float*)dataptr)[1]);
    return true;
  }
  else if (oreact == OR_LMRELEASE)
  {
    
  }
  return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

OSelector::OSelector(const linestyle_t &kls, float alpha): IOverlayTraced(kls), OVLCoordsDimmsLinked(CR_RELATIVE, 0.0f, 0.0f, 0.0f, 0.0f),
  m_alpha(alpha), m_phase(-1)
{
  if (alpha > 0.0f)
    m_linestyle.outside = OLS_OPACITY_LINEAR;
}

//bool OSelector::getRectangle(float *start_x, float *start_y, float *end_x, float *end_y)
//{
//  *start_x = m_coords.x;
//  *start_y = m_coords.y;
//  *end_x = m_sides.w;
//  *end_y = m_sides.h;
//  return m_phase == 0;
//}

int OSelector::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    {
      ocg.goto_normed();      
      ocg.trace_rect_xywh_begin(nullptr);
      ocg.trace_rect_xywh_end("idimms2", m_alpha);
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

bool OSelector::overlayReaction(OVL_REACTION oreact, const void* dataptr, bool*)
{
  bool result = false;
  if (oreact == OR_LMPRESS)
  {
    m_phase = 1;
    m_xy = ovlcoords_t((const float*)dataptr);
    m_wh = ovldimms2_t(0.0f, 0.0f);
    m_dimmsready = false;
    result = true;
  }
  else if (oreact == OR_LMMOVE)
  {
    if (m_phase == 1)
    {
      ovlcoords_t pdt((float*)dataptr);
      m_wh.w = pdt.x - m_xy.x;  //if (m_wh[0] < 0) m_wh[0] = -m_wh[0];
      m_wh.h = pdt.y - m_xy.y;  //if (m_wh[1] < 0) m_wh[1] = -m_wh[1];
      result = true;
    }
  }
  else if (oreact == OR_LMRELEASE)
  {
    m_phase = 0;
    m_dimmsready = true;
  }
  if (result)
    overlayUpdateParameter();
  return result;
}




















