/// Overlays:   data bound-depended (low and high data bounds)
///   OLevel. View: horizontal line across value
/// Created By: Elijah Vlasov
#include "bsbounded.h"
#include "../../core/sheigen/bsshgentrace.h"
#include "../../core/sheigen/bsshgencolor.h"

OLevel::OLevel(float value, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsOff(), OVLDimmsOff(), m_value(value){}

int OLevel::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_DATABOUNDS); //datarange
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.goto_normed();
  {
    ocg.var_const_fixed("value", m_value);
    ocg.push("int zerooffset = int(((value-datarange[0])/(datarange[1]-datarange[0]))*ov_ibounds.y);");
    ocg.trace_linehorz_l(nullptr, nullptr, "zerooffset", nullptr);
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

OLevelVariable::OLevelVariable(float value, const linestyle_t& linestyle): 
  Ovldraw_ColorTraced(linestyle), OVLCoordsOff(), OVLDimmsOff(), m_value(value)
{
  appendUniform(DT_1F, &m_value);
}

void OLevelVariable::setLevel(float v, bool update)
{
  m_value = v;
  updateParameter(false, update);
}

int OLevelVariable::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_DATABOUNDS); //datarange
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.goto_normed();
  {
    ocg.param_alias("value");
    ocg.push("int zerooffset = int(((value-datarange[0])/(datarange[1]-datarange[0]))*ov_ibounds.y + 0.49);");
    ocg.trace_linehorz_l(nullptr, nullptr, "zerooffset", nullptr);
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////



ORecLine::ORecLine(const linestyle_t& linestyle): Ovldraw_ColorTraced(linestyle), OVLCoordsOff(), OVLDimmsOff(),
  ctr(0)
{
  for (int i=0; i<TOTAL; i++)
    links[i] = -1;
  _dm_coords.count = TOTAL;
  _dm_coords.data = links;
  appendUniform(DT_1I, &ctr);
  appendUniform(DT_ARRI, &_dm_coords);
}

void ORecLine::increment(bool activate, bool update)
{
  if (activate)
  {
    for (int i=LAST; i>0; i--)    //for (int i=LAST-1; i>0; i--)????????????????
      links[i] = links[i-1];
    links[0] = ctr;
  }
  ctr++;
  updateParameter(false, update);
}

int ORecLine::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_DATABOUNDS); //datarange
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
//  ocg.goto_normed();
  {
    ocg.param_alias("ctr");
    ocg.push("int m = 10000;");
    ocg.push("int v = -1;");
    ocg.param_for_arr_begin("offset");
    {
      ocg.push("int delta = icoords.y - (ctr - offset);");
      ocg.push("int apply = int(step(0, offset)*step(abs(delta), abs(m)));");
      ocg.push("m = int(mix(m, delta, apply));");
      ocg.push("v = int(mix(v, ctr - offset, apply));");
    }
    ocg.param_for_end();
    ocg.push("ioffset = ivec2(0, v);");
    ocg.goto_normed();
    ocg.trace_linehorz_l(nullptr, nullptr, nullptr, nullptr);
  }
  ocg.goto_func_end(true);
  return ocg.written();
}




//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


_OLevelSet::_OLevelSet(unsigned int maxcount, linestyle_t* pkls, line_t* poffsets, COORDINATION cn, bool isstatic, float margin1, float margin2): 
  Ovldraw(true), m_activecount(0), m_cn(cn), m_static(isstatic), m_mr1(margin1), m_mr2(margin2), 
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
    updateParameter(false, update);
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
    updateParameter(false, update);
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
      updateParameter(false, update);
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
    updateParameter(false, update);
  }
}

int _OLevelSet::fshOVCoords(int overlay, bool switchedab, char* to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_DATABOUNDS);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, nullptr);
  {
    ocg.param_alias("count");
    ocg.push("if (count <= 0) return vec4(result, mixwell);");
    
    ocg.var_const_fixed("total", int(dtarr.count));
    
    ocg.var_fixed("cs_mr1", m_mr1);
    ocg.var_fixed("cs_mr2", m_mr2);
    int fc = ocg.register_xyscaler_pixel(m_cn);
    if (m_lt == LT_HORZ)//_BYLEFT || m_lt == LT_HORZ_BYRIGHT)
    {
      ocg.xyscale_x_pixel("cs_mr1", fc);
      ocg.xyscale_x_pixel("cs_mr2", fc);
    }
    else
    {
      ocg.xyscale_y_pixel("cs_mr1", fc);
      ocg.xyscale_y_pixel("cs_mr2", fc);
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
        ocg.push("oc[i][1] = (oc[i][1] - datarange[0])/(datarange[1]-datarange[0]);");
      
      if (m_lt == LT_HORZ) //_BYLEFT || m_lt == LT_HORZ_BYRIGHT)
      {
        ocg.xyscale_y_pixel("oc[i][1]", fc);
        ocg.push("float rs = oc[i][1] - float(icoords.y);");
      }
      else
      {
        ocg.xyscale_x_pixel("oc[i][1]", fc);
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
      else if (m_lt == LT_HORZ)             ocg.trace_linehorz_l("ov_ibounds.x - cs_mr1 - cs_mr2", "cs_mr1");
      //        else if (m_lt == LT_HORZ_BYRIGHT)     ocg.trace_linehorz_r("cs_mr2", "cs_mr1");
      //        else if (m_lt == LT_VERT_BYTOP)       ocg.trace_linevert_t("cs_mr2", "cs_mr1");
      else if (m_lt == LT_VERT)             ocg.trace_linevert_b("ov_ibounds.y - cs_mr1 - cs_mr2", "cs_mr1");
    }
    ocg.push("result[2] = lid;");
  }
  ocg.goto_func_end(true);
  //    qDebug()<<to;
  return ocg.written();
}

int _OLevelSet::fshColor(int overlay, char* to) const
{
  FshOVColorConstructor ocg(overlay, to);
  ocg.goto_func_begin();
  {
    const char* prefix = "if (in_variant[2] == ";
    const char* numi[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    const char* postfix = ".0) {";
    for (int i=0; i<dtarr.count; i++)
    {
      ocg.push(prefix); ocg.push(numi[i]);  ocg.push(postfix);
      ocg.mixwell_by_traced(m_kls[i]);
      ocg.brushResult(m_kls[i]);
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


