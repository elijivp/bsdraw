/// Overlays:   Value-depended, useful for highlight data
///   OContour. View: a < data[x] < b: colored borders for data
///   OContourPal. View: a < data[x] < b: colored borders for data corresponding with palette
///   OCover. View: a < data[x] < b: fill by color
///   OSlice. View: ... idk 
/// Created By: Elijah Vlasov
#include "bscontour.h"

#include "../core/sheigen/bsshgentrace.h"

static void make_contour_constants(FshOVCoordsConstructor* ocg, unsigned char checkmask)
{
  const char cc[] = 
                    "ivec2 inoscaled = ivec2(floor(coords * ov_indimms));"
                    "ivec2 iscaled = ivec2(floor(coords * ov_ibounds));";
  ocg->push(cc, sizeof(cc));
  int indcount=0, indicies[8] = { 0,0,0,0,0,0,0,0 };
  for (int i=0; i<8; i++)
    if (checkmask & (1 << i))
      indicies[indcount++] = i;
  ocg->var_inline("const int cellscount = %d;\n", indcount);
  ocg->var_inline("ivec2 cell[%d];\n", indcount);
  const char* bb[] = {    
                          "ivec2(-1,-1)",
                          "ivec2(-1,0)",
                          "ivec2(-1,1)",
                          "ivec2(0,1)",
                          "ivec2(1,1)",
                          "ivec2(1,0)",
                          "ivec2(1,-1);",
                          "ivec2(0,-1);"
                     };
  
  for (int i=0; i<indcount; i++)
  {
    ocg->var_inline("cell[%d] = ", i);  ocg->push(bb[indicies[i]]); ocg->push(";");
  }   
}




///////////////////////////////////////////////////////////////////////////////////////////////////////

OConst::OConst(color3f_t color): m_color(color)
{
}

int OConst::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("clr", m_color.r, m_color.g, m_color.b);
    ocg.push( "result = clr;" );
    ocg.push( "mixwell = 1.0;" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

OContour::OContour(float from, float to, const linestyle_t &kls, bool noscaled_contour, unsigned char checkmask): 
  Ovldraw_ColorTraced(kls), OVLCoordsOff(), OVLDimmsOff(),
  m_from(from), m_to(to), m_noscaled_contour(noscaled_contour), m_checkmask(checkmask)
{
}

int OContour::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    if (m_checkmask)
    {
      ocg.var_const_fixed("bnd", m_from, m_to);
      make_contour_constants(&ocg, m_checkmask);
      
      ocg.push( "for (int p=0; p<countPortions; p++){");
      {
        ocg.push( "_fvar = getValue2D(p, (inoscaled + vec2(0.49, 0.49))/vec2(ov_indimms.xy));"
                  "_mvar[0] = 1.0 - step(bnd.x, _fvar)*step(_fvar, bnd.y);"
                  "for (int i=0; i<cellscount; i++){"
                  );
        {
          if (m_noscaled_contour)
            ocg.push("vec2 fcoords2 = (iscaled + cell[i] + vec2(0.49, 0.49))/vec2(ov_ibounds.xy);");
          else
            ocg.push("vec2 fcoords2 = (inoscaled + cell[i] + vec2(0.49, 0.49))/vec2(ov_indimms.xy);");
            
          ocg.push( "_fvar = getValue2D(p, fcoords2);"
                    "_mvar[1] = step(bnd.x, _fvar)*step(_fvar, bnd.y);"
                    "result = mix(result, vec3(_mvar[0]*_mvar[1], 0, 1), step(result[0], _mvar[0]*_mvar[1]) );"    /// in_variant not finished
                  "}");
        }
      }
      ocg.push("}");
    }
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

OContourPal::OContourPal(float from, float to, const IPalette* ipal, bool discrete, bool noscaled_contour, unsigned char checkmask): 
  Ovldraw_ColorThroughPalette(ipal, discrete), OVLCoordsOff(), OVLDimmsOff(),
  m_from(from), m_to(to), m_noscaled_contour(noscaled_contour), m_checkmask(checkmask)
{
}

int OContourPal::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    if (m_checkmask)
    {
      ocg.var_const_fixed("bnd", m_from, m_to);
      make_contour_constants(&ocg, m_checkmask);
    
      ocg.push( "result = vec3(0.0,0.0,0.0);" );    // ?Dafuq?
      ocg.push( "_mvar[0] = getValue2D(0, (inoscaled + vec2(0.49, 0.49))/vec2(ov_indimms.xy));"
                "_mvar[0] = 1.0 - step(bnd.x, _mvar[0])*step(_mvar[0], bnd.y);"
                );
  
      ocg.push( "for (int i=0; i<8; i++){");
      {
        if (m_noscaled_contour)
          ocg.push("vec2 fcoords2 = (iscaled + cell[i] + vec2(0.49, 0.49))/vec2(ov_ibounds.xy);");
        else
          ocg.push("vec2 fcoords2 = (inoscaled + cell[i] + vec2(0.49, 0.49))/vec2(ov_indimms.xy);");
          
        ocg.push( "_fvar = getValue2D(0, fcoords2);"
                  "_mvar[1] = (_fvar - bnd.x) / (bnd.y - bnd.x);"
                  "_fvar = step(bnd.x, _fvar)*step(_fvar, bnd.y);"
                  "mixwell = mix(mixwell, _mvar[1], _mvar[0]*_fvar*step(sign(mixwell - _mvar[1]), 0.0) );"
                "}");
      }
    }
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

OCover::OCover(float from, float to, float r, float g, float b, COVER_OTHER_PORTIONS cop): m_from(from), m_to(to), m_cover_r(r), m_cover_g(g), m_cover_b(b), m_cop(cop)
{
}

OCover::OCover(float from, float to, int inversive_algo, COVER_OTHER_PORTIONS cop): Ovldraw_ColorForegoing(inversive_algo), m_from(from), m_to(to), m_cover_r(0.0f), m_cover_g(0.0f), m_cover_b(0.0f), m_cop(cop)
{
}

int OCover::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("bnd", m_from, m_to);
    ocg.var_const_fixed("cover", m_cover_r, m_cover_g, m_cover_b);
    ocg.push( "result = cover;"
              "vec3 maxes = vec3(0);"
              );
    ocg.push( "for (int p=0; p<countPortions; p++){"
                "_fvar = getValue2D(p, coords);"
                "maxes[0] = max(maxes[0], step(bnd.x, _fvar)*step(_fvar, bnd.y));"
                "maxes[1] = max(maxes[1], 1.0 - step(bnd.x, _fvar));"
                "maxes[2] = max(maxes[2], 1.0 - step(_fvar, bnd.y));"
              "}"
              );
    
    if (m_cop == COP_COVER)
      ocg.push("mixwell = maxes[0];");
    else if (m_cop == COP_SAVEALL)
      ocg.push("mixwell = maxes[0]*(1.0 - maxes[1])*(1.0 - maxes[2]);");
    else if (m_cop == COP_SAVELOWER)
      ocg.push("mixwell = maxes[0]*(1.0 - maxes[1]);");
    else if (m_cop == COP_SAVEUPPER)
      ocg.push("mixwell = maxes[0]*(1.0 - maxes[2]);");
    
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


OSlice::OSlice(float cover, float r, float g, float b): m_cover(cover), m_slice_r(r), m_slice_g(g), m_slice_b(b)
{
}

OSlice::OSlice(float cover, int inversive_algo): Ovldraw_ColorForegoing(inversive_algo), 
  m_cover(cover), m_slice_r(0.0f), m_slice_g(0.0f), m_slice_b(0.0f)
{
}

int OSlice::fshOVCoords(int overlay, bool switchedab, char *to) const
{
  FshOVCoordsConstructor  ocg(this->uniforms(), overlay, to, FshOVCoordsConstructor::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("clr", m_slice_r, m_slice_g, m_slice_b);
    ocg.push( "result = clr;" );
    ocg.var_const_fixed("cover", m_cover);
    ocg.push( "for (int p=0; p<countPortions; p++){"
                "_fvar = cover*getValue2D(p, coords);"
                "mixwell = max(mixwell, _fvar);"
              "}"
              );
    ocg.push( "mixwell = (1.0 - mixwell);" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

