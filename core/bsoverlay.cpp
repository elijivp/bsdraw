/// This file contains useful/technical types, base classes, conversion functions for all types of draws
/// Midway realisations for Overlays
/// If you need to create your own Overlay, use other overlays as examples
/// Created By: Elijah Vlasov
#include "bsoverlay.h"
#include "sheigen/bsshgencolor.h"

int DrawOverlaySimple::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_COLORED);
  if (m_inversive != 0)
    ocg.color_by_inversive(m_inversive);
  ocg.goto_func_end();
  return ocg.written();
}

int DrawOverlayTraced::fshColor(int overlay, char* to) const
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

int DrawOverlayHard::fshColor(int overlay, char* to) const
{
  FshColorGenerator ocg(overlay, to);
  ocg.goto_func_begin(FshColorGenerator::CGV_TEXTURED);
  ocg.goto_func_end();
  return ocg.written();
}

