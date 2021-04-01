/// Overlays:   lines and selectors, saved into array (care max 1024 size)
///   OPTFill: same as OFPoint but with another realization
///   ODropPoints: array of not linked points
///   OPolyLine: array of linked points, base class
///   ODropLine: click = new line from last point
///   OBrush: move = new line from last point
///   OSelector: rectangle-selector
///   OSelectorCirc: circle-selector
/// Created By: Elijah Vlasov
#include "bspoints.h"

#include "../core/sheigen/bsshgentrace.h"
#include <math.h>
#include <memory.h>

OPTFill::OPTFill(const linestyle_t& kls): DrawOverlaySimple(),
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



ODropPoints::ODropPoints(unsigned int ptlimit, COORDINATION, float, const linestyle_t& kls): DrawOverlayTraced(kls),
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
      ocg.push("_fvar = (1.0+thick - clamp(distance(vec2(inormed), vec2(0.0,0.0)), 0.0, 1.0+thick))/(1.0+thick);");
      ocg.push("_mvar = vec2(_fvar, float(i));");
      ocg.push("result = mix(result, vec3(_mvar*_traceban, 0.0), 1 - step(abs(_mvar[0]) - abs(result[0]), 0.0) );");
    }
    ocg.param_for_end();
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

bool ODropPoints::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*)
{
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
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

OPolyLine::OPolyLine(unsigned int countPointsMax, const linestyle_t& kls): DrawOverlayTraced(kls),
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

int OPolyLine::fshTrace(int overlay, bool rotated, char *to) const
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

void  OPolyLine::setPointsCount(unsigned int newCount)
{
  ptCount = newCount;
}

void  OPolyLine::setPoint(int idx, float x, float y)
{
  if (idx < ptCountMax)
    ptdrops[idx] = ovlcoords_t(x, y);
}

void  OPolyLine::updatePoints()
{
  overlayUpdateParameter();
}

////////////////////////////////
ODropLine::ODropLine(unsigned int maxpoints, bool lastFollowsMouse, const linestyle_t& kls): OPolyLine(maxpoints, kls), followMoving(lastFollowsMouse)
{
}

bool ODropLine::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*)
{
  bool result = false;
  if (oreact == ORM_LMPRESS)
  {
    if (ptCount == ptCountMax)
      ptCount--;
    ptdrops[ptCount++] = ovlcoords_t(((float*)dataptr)[0], ((float*)dataptr)[1]);
    result = true;
  }
  else if (oreact == ORM_LMMOVE && followMoving)
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
  else if (oreact == ORM_LMRELEASE)
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
OBrush::OBrush(unsigned int memoryPoints, const linestyle_t& kls): OPolyLine(memoryPoints, kls)
{
}

bool OBrush::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*)
{
  bool result = false;
  if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
  {
    if (ptCount == ptCountMax)
      result = false;
    else
    {
      if (fabs(((float*)dataptr)[0] - ((float*)dataptr)[1]) >= 0.001f)
      {
        ptdrops[ptCount++] = ovlcoords_t(((float*)dataptr)[0], ((float*)dataptr)[1]);
        result = true;
      }
    }
  }
  else if (oreact == ORM_LMRELEASE)
  {
  }
  else if (oreact == ORM_RMPRESS)
  {
    ptCount = 0;
    result = true;
  }
  if (result)
    overlayUpdateParameter();
  return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

OSelectorReaction::OSelectorReaction(const linestyle_t &kls, float alpha, bool moveable, bool saveneg): 
  DrawOverlayTraced(kls), OVLCoordsDimmsLinked(CR_RELATIVE, 0.0f, 0.0f, 0.0f, 0.0f),
  m_alpha(alpha), m_phase(-1), m_move(moveable), m_neg(saveneg)
{
  if (alpha > 0.0f)
    m_linestyle.outside = OLS_OPACITY_LINEAR;
}

bool OSelectorReaction::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void* dataptr, bool*)
{
  bool result = false;
  
  if (m_move == true)
  {
    if (m_dimmsready)
    {
      ovlcoords_t clk((const float*)dataptr);
      if (oreact == ORM_LMPRESS)
      {
        if (inrect(clk))
        {
          m_dxy = clk;
          return true;
        }
      }
      else if (oreact == ORM_LMMOVE)
      {
        m_xy.x += clk.x - m_dxy.x;
        m_xy.y += clk.y - m_dxy.y;
        m_dxy = clk;
        result = true;
      }
    }
  }
  
  if (result == false)
  {
    if (oreact == ORM_LMPRESS || oreact == ORM_RMPRESS)
    {
      m_phase = oreact == ORM_LMPRESS? 1 : 0;
      m_xy = ovlcoords_t((const float*)dataptr);
      m_dxy = m_xy;
      m_wh = ovldimms2_t(0.0f, 0.0f);
      m_dimmsready = false;
      result = true;
    }
    else
    {
      if (m_phase == 1)
      {
        ovlcoords_t pdt((const float*)dataptr);
        m_wh.w = pdt.x - m_dxy.x;  if (m_wh.w < 0 && !m_neg){  m_wh.w = -m_wh.w; m_xy.x = pdt.x; }  //if (m_wh[0] < 0) m_wh[0] = -m_wh[0];
        m_wh.h = pdt.y - m_dxy.y;  if (m_wh.h < 0 && !m_neg){  m_wh.h = -m_wh.h; m_xy.y = pdt.y; } //if (m_wh[1] < 0) m_wh[1] = -m_wh[1];
        result = true;
      }
      if (oreact == ORM_LMRELEASE)
      {
        m_phase = 0;
        m_dimmsready = true;
      }
    }
  }
  
  if (result)
    overlayUpdateParameter();
  return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////


OSelector::OSelector(const linestyle_t& kls, float alpha, bool moveable): OSelectorReaction(kls, alpha, moveable, false)
{
}

int OSelector::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    {
      ocg.goto_normed();      
      ocg.trace_rect_xywh("idimms2", m_alpha);
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

OSelectorCirc::OSelectorCirc(const linestyle_t& kls, float alpha, bool moveable): OSelectorReaction(kls, alpha, moveable, true)
{
}

int OSelectorCirc::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    {
      ocg.goto_normed();
      ocg.push("float rr = sqrt(float(idimms2.x*idimms2.x + idimms2.y*idimms2.y));");
      ocg.push("if (rr > 0)");
      ocg.push("{");
        ocg.trace_circle_cc_begin("rr", "2");
        ocg.trace_circle_cc_end(m_alpha);
      ocg.push("}");
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}
