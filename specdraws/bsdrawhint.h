#ifndef DRAWCOREHINT_H
#define DRAWCOREHINT_H

/// This file contains special derived class for show images
/// DrawSDPicture with option SDPSIZE_NONE does nothing, just shows images on 2D place (space evals throught sizeAndScaleHint)
/// DrawSDPicture with option SDPSIZE_MARKER analyse image for markers and fill them by corresponding colors
///   You must especially prepare image (paint him by markers) before use
/// 
/// Example:
/// #include "specdraws/bsdrawsdpicture.h"
/// 
/// DrawSDPicture* draw = new DrawSDPicture(SAMPLES, LINES, "path_to_picture.png");
/// ...
/// draw->setData(some_float_data);
/// 
/// Created By: Elijah Vlasov

#include "../core/bsqdraw.h"


enum  DRAWHINT
{
  DH_LINE = 0x00,     /// second digit ignored
  DH_FILL = 0x10,     /// second digit ignored
  DH_DIAGONAL = 0x20, /// second digit ignored
  DH_TRIANGLE = 0x30, /// second digit is for count of triangles
  DH_SAW = 0x40,      /// second digit is for count of saws
  DH_MEANDER = 0x50,  /// second digit is for count of meanders
  
  DH_AUTOMARGIN_0=0x000,
  DH_AUTOMARGIN_1=0x100,  DH_AUTOMARGIN_2=0x200,  DH_AUTOMARGIN_3=0x300,  DH_AUTOMARGIN_4=0x400,  DH_AUTOMARGIN_5=0x500,
  DH_AUTOMARGIN_6=0x600,  DH_AUTOMARGIN_7=0x700,  DH_AUTOMARGIN_8=0x800,  DH_AUTOMARGIN_9=0x900,  DH_AUTOMARGIN_10=0xA00,
  DH_AUTOMARGIN_11=0xB00,  DH_AUTOMARGIN_12=0xC00,  DH_AUTOMARGIN_13=0xD00,  DH_AUTOMARGIN_14=0xE00,  DH_AUTOMARGIN_15=0xF00
};

class DrawGraph;
class DrawHint: public DrawQWidget
{
public:
  DrawHint(const DrawGraph* pdg, int portion, int flags=0, ORIENTATION orient=OR_LRTB, unsigned int backgroundColor=0xFFFFFFFF, float additionalMixWithBackground=0.0f);
  DrawHint(float value, ORIENTATION orient=OR_LRTB, unsigned int backgroundColor=0xFFFFFFFF, float additionalMixWithBackground=0.0f);
public:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};

#endif // DRAWCOREHINT_H
