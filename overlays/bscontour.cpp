#include "bscontour.h"
#include "../core/sheigen/bsshgentrace.h"

static const char* contour_constants1 = 
    "ivec2 ibounds_noscaled = ivec2(datadimm_a, datadimm_b);"
    "ivec2 inoscaled = ivec2(floor(coords.pq * ibounds_noscaled));"
    "ivec2 iscaled = ivec2(floor(coords.pq * ibounds));"
    "ivec2 borders[8];"
    "borders[0] = ivec2(-1,-1);"
    "borders[1] = ivec2(-1,0);"
    "borders[2] = ivec2(-1,1);"
    "borders[3] = ivec2(0,1);"
    "borders[4] = ivec2(1,1);"
    "borders[5] = ivec2(1,0);"
    "borders[6] = ivec2(1,-1);"
    "borders[7] = ivec2(0,-1);";


OContour::OContour(float from, float to, const linestyle_t &kls, bool noscaled_contour): IOverlayTraced(kls), OVLCoordsOff(), OVLDimmsOff(),
  m_from(from), m_to(to), m_noscaled_contour(noscaled_contour)
{
}

int OContour::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("bnd", m_from, m_to);
    ocg.push( contour_constants1 );
    
    ocg.push( "for (int p=0; p<countPortions; p++){");
    {
      ocg.push( "_densvar = getValue2D(p, (inoscaled + vec2(0.49, 0.49))/vec2(ibounds_noscaled.xy));"
                "_insvar[0] = 1.0 - step(bnd.x, _densvar)*step(_densvar, bnd.y);"
                "for (int i=0; i<8; i++){"
                );
      {
        if (m_noscaled_contour)
          ocg.push("vec2 fcoords2 = (iscaled + borders[i] + vec2(0.49, 0.49))/vec2(ibounds.xy);");
        else
          ocg.push("vec2 fcoords2 = (inoscaled + borders[i] + vec2(0.49, 0.49))/vec2(ibounds_noscaled.xy);");
          
        ocg.push( "_densvar = getValue2D(p, fcoords2);"
                  "_insvar[1] = step(bnd.x, _densvar)*step(_densvar, bnd.y);"
//                  "_densvar = (inoscaled.x + 0.49)*abs(borders[i].y)/ibounds_noscaled.x + (inoscaled.y + 0.49)*abs(borders[i].x)/ibounds_noscaled.y;"
//                  "_insvar[2] = mix(_densvar, 0.0, abs(borders[i].x)*abs(borders[i].y));"
//                  "_densvar = mix(float(iscaling.x*abs(borders[i].y) + iscaling.y*abs(borders[i].x)), 1.0, abs(borders[i].x)*abs(borders[i].y));"
  //                "result = mix(result, vec3(1, _insvar[0], _densvar), _insvar[1]*_insvar[2] );"
                  
                  "result = mix(result, vec3(_insvar[0]*_insvar[1], 0, 1), step(result[0], _insvar[0]*_insvar[1]) );"    /// trace not finished
                "}");
      }
    }
    ocg.push("}");
    
  }
  ocg.goto_func_end(true);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

OContourPal::OContourPal(float from, float to, const IPalette* ipal, bool discrete, bool noscaled_contour): IOverlayHard(ipal, discrete), OVLCoordsOff(), OVLDimmsOff(),
  m_from(from), m_to(to), m_noscaled_contour(noscaled_contour)
{
}

int OContourPal::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  ocg.param_pass(); // Kostyl
  {
    ocg.var_const_fixed("bnd", m_from, m_to);
    ocg.push( contour_constants1 );
    ocg.push( "result = vec3(1.0,0.0,0.0);" );
    ocg.push( "_insvar[0] = getValue2D(0, (inoscaled + vec2(0.49, 0.49))/vec2(ibounds_noscaled.xy));"
              "_insvar[0] = 1.0 - step(bnd.x, _insvar[0])*step(_insvar[0], bnd.y);"
              );

    ocg.push( "for (int i=0; i<8; i++){");
    {
      if (m_noscaled_contour)
        ocg.push("vec2 fcoords2 = (iscaled + borders[i] + vec2(0.49, 0.49))/vec2(ibounds.xy);");
      else
        ocg.push("vec2 fcoords2 = (inoscaled + borders[i] + vec2(0.49, 0.49))/vec2(ibounds_noscaled.xy);");
        
      ocg.push( "_densvar = getValue2D(0, fcoords2);"
                "_insvar[1] = (_densvar - bnd.x) / (bnd.y - bnd.x);"
                "_insvar[2] = step(bnd.x, _densvar)*step(_densvar, bnd.y);"
                "mixwell = mix(mixwell, _insvar[1], _insvar[0]*_insvar[2]*step(sign(mixwell - _insvar[1]), 0.0) );"
              "}");
    }
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

OCover::OCover(float from, float to, float r, float g, float b, COVER_OTHER_PORTIONS cop): m_from(from), m_to(to), m_cover_r(r), m_cover_g(g), m_cover_b(b), m_cop(cop)
{
}

OCover::OCover(float from, float to, int inversive_algo, COVER_OTHER_PORTIONS cop): IOverlaySimple(inversive_algo), m_from(from), m_to(to), m_cover_r(0.0f), m_cover_g(0.0f), m_cover_b(0.0f), m_cop(cop)
{
}

int OCover::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("bnd", m_from, m_to);
    ocg.var_const_fixed("cover", m_cover_r, m_cover_g, m_cover_b);
    ocg.push( "result = cover;" );
    ocg.push( "for (int p=0; p<countPortions; p++){"
                "_densvar = getValue2D(p, coords.pq);"
                "_insvar[0] = max(_insvar[0], step(bnd.x, _densvar)*step(_densvar, bnd.y));"
                "_insvar[1] = max(_insvar[1], 1.0 - step(bnd.x, _densvar));"
                "_insvar[2] = max(_insvar[2], 1.0 - step(_densvar, bnd.y));"
              "}"
              );
    
    if (m_cop == COP_COVER)
      ocg.push("mixwell = _insvar[0];");
    else if (m_cop == COP_SAVEALL)
      ocg.push("mixwell = _insvar[0]*(1.0 - _insvar[1])*(1.0 - _insvar[2]);");
    else if (m_cop == COP_SAVELOWER)
      ocg.push("mixwell = _insvar[0]*(1.0 - _insvar[1]);");
    else if (m_cop == COP_SAVEUPPER)
      ocg.push("mixwell = _insvar[0]*(1.0 - _insvar[2]);");
    
  }
  ocg.goto_func_end(false);
  return ocg.written();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


OSlice::OSlice(float cover, float r, float g, float b): m_cover(cover), m_slice_r(r), m_slice_g(g), m_slice_b(b)
{
}

OSlice::OSlice(float cover, int inversive_algo): IOverlaySimple(inversive_algo), m_cover(cover), m_slice_r(0.0f), m_slice_g(0.0f), m_slice_b(0.0f)
{
}

int OSlice::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to, FshTraceGenerator::OINC_GETVALUE);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
    ocg.var_const_fixed("clr", m_slice_r, m_slice_g, m_slice_b);
    ocg.push( "result = clr;" );
    ocg.var_const_fixed("cover", m_cover);
    ocg.push( "for (int p=0; p<countPortions; p++){"
                "_densvar = cover*getValue2D(p, coords.pq);"
                "mixwell = max(mixwell, _densvar);"
              "}"
              );
    ocg.push( "mixwell = (1.0 - mixwell);" );
  }
  ocg.goto_func_end(false);
  return ocg.written();
}
