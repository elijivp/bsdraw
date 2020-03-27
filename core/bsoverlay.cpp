#include "bsoverlay.h"
#include "sheigen/bsshgencolor.h"

int IOverlaySimple::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_COLORED);
  if (m_inversive != 0)
    ocg.color_by_inversive(m_inversive);
  ocg.goto_func_end();
  return ocg.written();
}

int IOverlayTraced::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_TRACED);
  {
    ocg.mixwell_by_traced(m_linestyle);
    ocg.color_by_traced(m_linestyle);
  }
  ocg.goto_func_end();
  return ocg.written();
}

int IOverlayHard::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_TEXTURED);
  {
    ocg.mixwell_by_traced(m_linestyle);
  }
  ocg.goto_func_end();
  return ocg.written();
}

