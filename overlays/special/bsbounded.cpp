/// Overlays:   data bound-depended (low and high data bounds)
///   OValueLine. View: horizontal line across value
/// Created By: Elijah Vlasov
#include "bsbounded.h"
#include "../../core/sheigen/bsshgentrace.h"
#include "../../core/sheigen/bsshgencolor.h"

OValueLine::OValueLine(float value): DrawOverlayTraced(), OVLCoordsOff(), OVLDimmsOff(), m_value(value) {}
OValueLine::OValueLine(float value, const linestyle_t& linestyle): 
  DrawOverlayTraced(linestyle), OVLCoordsOff(), OVLDimmsOff(), m_value(value){}

int OValueLine::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_DATABOUNDS); //databounds
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.goto_normed();
  {
    ocg.var_const_fixed("value", m_value);
    ocg.push("int zerooffset = int(((value-databounds[0])/(databounds[1]-databounds[0]))*ibounds.y + 0.49);");
    ocg.trace_linehorz_l(nullptr, nullptr, "zerooffset", nullptr);
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


_OLevelSet::_OLevelSet(unsigned int maxcount, linestyle_t* pkls, line_t* poffsets, COORDINATION cn, bool isstatic, float margin1, float margin2): 
  DrawOverlayTraced(), m_activecount(0), m_cn(cn), m_static(isstatic), m_mr1(margin1), m_mr2(margin2), 
  m_lt(LT_HORZ), m_kls(pkls), m_data(poffsets)
{
  for (int i=0; i<maxcount; i++)
    m_data[i].activated = 0.0f;
  
  appendUniform(DT_1I, &m_activecount);
  dtarr.count = maxcount;
  dtarr.data = m_data;
  appendUniform(DT_ARR2, &dtarr);
}

int _OLevelSet::lineAdd(float offset, bool update)
{
  int idx;
  for (idx = 0; idx<dtarr.count; idx++)
    if (m_data[idx].activated == 0.0f)
      break;
  lineShow(idx, offset, update);
  return idx < dtarr.count ? idx : -1;
}

int _OLevelSet::lineNextFreeIndex() const
{
  int idx;
  for (idx = 0; idx<dtarr.count; idx++)
    if (m_data[idx].activated == 0.0f)
      break;
  return idx < dtarr.count ? idx : -1;
}

void _OLevelSet::lineShow(int idx, float offset, bool update)
{
  if (idx < dtarr.count)
  {
    if (m_data[idx].activated == 0.0f)
    {
      m_data[idx].activated = 1.0f;
      m_activecount++;
    }
    m_data[idx].offset = offset;
    if (update) overlayUpdateParameter();
  } 
}

void _OLevelSet::lineShow(int idx, bool update)
{
  if (idx < dtarr.count)
  {
    if (m_data[idx].activated == 0.0f)
    {
      m_data[idx].activated = 1.0f;
      m_activecount++;
    }
    if (update) overlayUpdateParameter();
  } 
}

void _OLevelSet::lineHide(int idx, bool update)
{
  if (idx >= 0 && idx < dtarr.count)
  {
    if (m_data[idx].activated > 0.0f)
    {
      m_data[idx].activated = 0.0f;
      m_activecount--;
      if (update) overlayUpdateParameter();
    }
  }
}

void _OLevelSet::clear(bool update)
{
  if (m_activecount)
  {
    for (int i=0; i<dtarr.count; i++)
      m_data[i].activated = 0.0f;
    m_activecount = 0;
    if (update) overlayUpdateParameter();
  }
}

int _OLevelSet::fshTrace(int overlay, bool rotated, char* to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_DATABOUNDS);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
    ocg.param_alias("count");
    ocg.push("if (count <= 0) return vec4(result, mixwell);");
    
    ocg.var_const_fixed("total", int(dtarr.count));
    
    ocg.var_fixed("cs_mr1", m_mr1);
    ocg.var_fixed("cs_mr2", m_mr2);
    int fc = ocg.add_movecs_pixing(m_cn);
    if (m_lt == LT_HORZ)//_BYLEFT || m_lt == LT_HORZ_BYRIGHT)
    {
      ocg.movecs_pix_x("cs_mr1", fc);
      ocg.movecs_pix_x("cs_mr2", fc);
    }
    else
    {
      ocg.movecs_pix_y("cs_mr1", fc);
      ocg.movecs_pix_y("cs_mr2", fc);
    }
    
    ocg.var_array_ff_empty("oc", dtarr.count);
    ocg.push("int lid = 0;");
    ocg.push("float loffs = 9999;");
    
    ocg.push("for (int i=0; i<total; i++)"
             "{"
             );
    {
      ocg.push("oc[i] = "); ocg.param_get(); ocg.push("[i];");
      if (m_static == false)
        ocg.push("oc[i][1] = (oc[i][1] - databounds[0])/(databounds[1]-databounds[0]);");
      
      if (m_lt == LT_HORZ) //_BYLEFT || m_lt == LT_HORZ_BYRIGHT)
      {
        ocg.movecs_pix_y("oc[i][1]", fc);
        ocg.push("float rs = oc[i][1] - float(icoords.y);");
      }
      else
      {
        ocg.movecs_pix_x("oc[i][1]", fc);
        ocg.push("float rs = oc[i][1] - float(icoords.x);");
      }
      ocg.push("float cup = step(1.0, oc[i][0]) * step(abs(rs), abs(loffs));");
      ocg.push("lid = int(mix(lid, i, cup));");
      ocg.push("loffs = int(mix(loffs, rs, cup));");
    }
    ocg.param_for_end();
    
    if (m_lt == LT_HORZ)//_BYLEFT || m_lt == LT_HORZ_BYRIGHT)
      ocg.push("ivec2 inormed = icoords - ivec2(0, oc[lid][1]);");
    else
      ocg.push("ivec2 inormed = icoords - ivec2(oc[lid][1], 0);");
    {
      if (false)  ;
      else if (m_lt == LT_HORZ)             ocg.trace_linehorz_l("ibounds.x - cs_mr1 - cs_mr2", "cs_mr1");
      //        else if (m_lt == LT_HORZ_BYRIGHT)     ocg.trace_linehorz_r("cs_mr2", "cs_mr1");
      //        else if (m_lt == LT_VERT_BYTOP)       ocg.trace_linevert_t("cs_mr2", "cs_mr1");
      else if (m_lt == LT_VERT)             ocg.trace_linevert_b("ibounds.y - cs_mr1 - cs_mr2", "cs_mr1");
    }
    ocg.push("result[2] = lid;");
  }
  ocg.goto_func_end(true);
  //    qDebug()<<to;
  return ocg.written();
}

int _OLevelSet::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_TRACED);
  {
    const char* prefix = "if (trace_on_pix[2] == ";
    const char* numi[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    const char* postfix = ".0) {";
    for (int i=0; i<dtarr.count; i++)
    {
      ocg.push(prefix); ocg.push(numi[i]);  ocg.push(postfix);
      ocg.mixwell_by_traced(m_kls[i]);
      ocg.color_by_traced(m_kls[i]);
      ocg.push("}");
    }
  }
  ocg.goto_func_end();
  return ocg.written();
}

OLevelSet::OLevelSet(int flags, int lstroke, int lspace, int ldots, COORDINATION cn, bool isstatic, float margin1, float margin2): _OLevelSetMemory<5>(cn, isstatic, margin1, margin2)
{
  linestyle_t tmp[] = { linestyle_red(lstroke,lspace,ldots), linestyle_green(lstroke,lspace,ldots), linestyle_bluesoft(lstroke,lspace,ldots), linestyle_orange(lstroke,lspace,ldots), linestyle_purple(lstroke,lspace,ldots) };
  const int flags_schema[][5] = {
    { 0,1,2,3,4 },  { 1,2,3,4,0 }, { 2,3,4,0,1 }, { 3,4,0,1,2 }, { 4,0,1,2,3 }, 
    { 1,2,0,3,4 },  { 2,1,0,3,4 }, { 2,0,1,3,4 }, { 1,0,2,3,4 }, { 0,2,1,3,4 },
    { 1,2,3,0,4 },  { 2,1,3,0,4 }, { 2,3,1,0,4 }, { 1,3,2,0,4 }, { 3,2,1,0,4 }
  };
    
  int idx = flags & 0xF;
  if (idx >= _FL_SCHEMAS_COUNT)
    idx = 0;
  for (int i=0; i<5; i++)
    transfer_kls[i] = tmp[flags_schema[idx][i]];
  
  float brightness[5];
  for (int i=0; i<5; i++)
    brightness[i] = float(((flags)>>(12 + i*4))&0xF)/0xF;
  for (int i=0; i<5; i++)
  {
    transfer_kls[i].r += brightness[i]*0.8f;
    if (transfer_kls[i].r > 1.0f) transfer_kls[i].r = 1.0f;
    transfer_kls[i].g += brightness[i]*0.8f;
    if (transfer_kls[i].g > 1.0f) transfer_kls[i].g = 1.0f;
    transfer_kls[i].b += brightness[i]*0.8f;
    if (transfer_kls[i].b > 1.0f) transfer_kls[i].b = 1.0f;
  }
}








