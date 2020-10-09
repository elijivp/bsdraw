/// Overlays:   data bound-depended (low and high data bounds)
///   OValueLine. View: horizontal line across value
/// Created By: Elijah Vlasov
#include "bsbounded.h"
#include "../../core/sheigen/bsshgentrace.h"

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

