#ifndef QPALETTEEXTRACTOR_H
#define QPALETTEEXTRACTOR_H

#include <QString>

#ifndef MAX_PALETTE_COLORS_COUNT
#define MAX_PALETTE_COLORS_COUNT 512
#endif

enum PEX_FLAGS
{
  PEF_NONE = 0,
  PEF_REMOVEFIRSTCOLOR = 1,
  PEF_REMOVELASTCOLOR = 2,
  
  PEF_NOMAXLIMIT = 4,
  
  PEF_CUTTER_STEP = 0x00,
  PEF_CUTTER_GROUP = 0x10,
  
};

enum IMAGE_DIRECTION
{ 
  IMD_NATIVE,
  IMD_HORIZONTAL,
  IMD_VERTICAL
};

struct pexresult_t
{
  bool      success;
  QString   absolutePath;
  QString   paletteName;
  QString   paletteHeaderText;
  unsigned int  countColors;
};

pexresult_t paletteExtractFromImage(const char* fname, int flags, IMAGE_DIRECTION imd = IMD_NATIVE, const char* resultName=nullptr);
pexresult_t paletteExtractFromImage(const QString& fname, int flags, IMAGE_DIRECTION imd = IMD_NATIVE, const char* resultName=nullptr);

bool  savePaletteExtract(const pexresult_t& pex);

#endif // QPALETTEEXTRACTOR_H
