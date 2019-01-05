#ifndef PALETTES_STD_H
#define PALETTES_STD_H

#include "bstexpalette.h"

const unsigned int colorsGnYe[] = { 0x00002000, 0x0000ff00, 0x0000ffff };
const PaletteConstFWD<sizeof(colorsGnYe) / sizeof(unsigned int)>   paletteGnYe(colorsGnYe);
const PaletteConstBWD<sizeof(colorsGnYe) / sizeof(unsigned int)>   paletteGnYe_inv(colorsGnYe);
const unsigned int colorsBkWh[] = { 0x00000000, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkWh) / sizeof(unsigned int)>   paletteBkWh(colorsBkWh);
const PaletteConstBWD<sizeof(colorsBkWh) / sizeof(unsigned int)>   paletteBkWh_inv(colorsBkWh);
const unsigned int colorsBlWh[] = { 0x00ff0000, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBlWh) / sizeof(unsigned int)>   paletteBlWh(colorsBlWh);
const PaletteConstBWD<sizeof(colorsBlWh) / sizeof(unsigned int)>   paletteBlWh_inv(colorsBlWh);
const unsigned int colorsRdWh[] = { 0x000000ff, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsRdWh) / sizeof(unsigned int)>   paletteRdWh(colorsRdWh);
const PaletteConstBWD<sizeof(colorsRdWh) / sizeof(unsigned int)>   paletteRdWh_inv(colorsRdWh);
const unsigned int colorsGrGn[] = { 0x0044444444, 0x0000ff00 };
const PaletteConstFWD<sizeof(colorsGrGn) / sizeof(unsigned int)>   paletteGrGn(colorsGrGn);
const PaletteConstBWD<sizeof(colorsGrGn) / sizeof(unsigned int)>   paletteGrGn_inv(colorsGrGn);
const unsigned int colorsBkRdWh[] = { 0x00000000, 0x000000ff, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkRdWh) / sizeof(unsigned int)>   paletteBkRdWh(colorsBkRdWh);
const PaletteConstBWD<sizeof(colorsBkRdWh) / sizeof(unsigned int)>   paletteBkRdWh_inv(colorsBkRdWh);
const unsigned int colorsBkBlWh[] = { 0x00000000, 0x00ff0000, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkBlWh) / sizeof(unsigned int)>   paletteBkBlWh(colorsBkBlWh);
const PaletteConstBWD<sizeof(colorsBkBlWh) / sizeof(unsigned int)>   paletteBkBlWh_inv(colorsBkBlWh);
const unsigned int colorsBkGrWh[] = { 0x00000000, 0x0000ff00, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkGrWh) / sizeof(unsigned int)>   paletteBkGrWh(colorsBkGrWh);
const PaletteConstBWD<sizeof(colorsBkGrWh) / sizeof(unsigned int)>   paletteBkGrWh_inv(colorsBkGrWh);

const unsigned int colorsBkGrYeWh[] = { 0x00000000, 0x0000ff00, 0x0000ffff, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkGrYeWh) / sizeof(unsigned int)>   paletteBkGrYeWh(colorsBkGrYeWh);
const PaletteConstBWD<sizeof(colorsBkGrYeWh) / sizeof(unsigned int)>   paletteBkGrYeWh_inv(colorsBkGrYeWh);
const unsigned int colorsBkBlGrYeWh[] = { 0x00000000, 0x00ff0000, 0x0000ff00, 0x0000ffff, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkBlGrYeWh) / sizeof(unsigned int)>   paletteBkBlGrYeWh(colorsBkBlGrYeWh);
const PaletteConstBWD<sizeof(colorsBkBlGrYeWh) / sizeof(unsigned int)>   paletteBkBlGrYeWh_inv(colorsBkBlGrYeWh);

const unsigned int colorsBkGyGyGyWh[] = { 0x00000000, 0x00555555, 0x00777777, 0x00AAAAAA, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkGyGyGyWh) / sizeof(unsigned int)>   paletteBkGyGyGyWh(colorsBkGyGyGyWh);
const PaletteConstBWD<sizeof(colorsBkGyGyGyWh) / sizeof(unsigned int)>   paletteBkGyGyGyWh_inv(colorsBkGyGyGyWh);
const unsigned int colorsBkBlBlBlWh[] = { 0x00000000, 0x00ff0000, 0x00ff5555, 0x00ffAAAA, 0x00ffffff };
const PaletteConstFWD<sizeof(colorsBkBlBlBlWh) / sizeof(unsigned int)>   paletteBkBlBlBlWh(colorsBkBlBlBlWh);
const PaletteConstBWD<sizeof(colorsBkBlBlBlWh) / sizeof(unsigned int)>   paletteBkBlBlBlWh_inv(colorsBkBlBlBlWh);

const unsigned int colorsBkRdGnBu[] = { 0x00000000, 0x000000ff, 0x0000ff00, 0x00ff0000 };
const PaletteConstFWD<sizeof(colorsBkRdGnBu) / sizeof(unsigned int)>   paletteBkRdGnBu(colorsBkRdGnBu);
const PaletteConstBWD<sizeof(colorsBkRdGnBu) / sizeof(unsigned int)>   paletteBkRdGnBu_inv(colorsBkRdGnBu);

const unsigned int colorsRGBMIX[] = { 0x00000000, 0x000000ff, 0x00000000, 0x0000ff00, 0x00000000, 0x00ff0000 };
const PaletteConstFWD<sizeof(colorsRGBMIX) / sizeof(unsigned int)>   paletteRGBMIX(colorsRGBMIX);
const PaletteConstBWD<sizeof(colorsRGBMIX) / sizeof(unsigned int)>   paletteRGBMIX_inv(colorsRGBMIX);


#endif // PALETTES_STD_H

