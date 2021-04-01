#ifndef PALETTES_STD_H
#define PALETTES_STD_H

/// Standard and trivial palettes
/// Created By: Elijah Vlasov

#include "bsipalette.h"

const unsigned int colors_std_GnYe[] = { 0x00002000, 0x0000ff00, 0x0000ffff };
const PaletteConstFWD<sizeof(colors_std_GnYe) / sizeof(unsigned int)>   paletteGnYe(colors_std_GnYe);
const PaletteConstBWD<sizeof(colors_std_GnYe) / sizeof(unsigned int)>   paletteGnYe_inv(colors_std_GnYe);
const unsigned int colors_std_BkWh[] = { 0x00000000, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkWh) / sizeof(unsigned int)>   paletteBkWh(colors_std_BkWh);
const PaletteConstBWD<sizeof(colors_std_BkWh) / sizeof(unsigned int)>   paletteBkWh_inv(colors_std_BkWh);
const unsigned int colors_std_BlWh[] = { 0x00ff0000, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BlWh) / sizeof(unsigned int)>   paletteBlWh(colors_std_BlWh);
const PaletteConstBWD<sizeof(colors_std_BlWh) / sizeof(unsigned int)>   paletteBlWh_inv(colors_std_BlWh);
const unsigned int colors_std_RdWh[] = { 0x000000ff, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_RdWh) / sizeof(unsigned int)>   paletteRdWh(colors_std_RdWh);
const PaletteConstBWD<sizeof(colors_std_RdWh) / sizeof(unsigned int)>   paletteRdWh_inv(colors_std_RdWh);
const unsigned int colors_std_GrGn[] = { 0x0044444444, 0x0000ff00 };
const PaletteConstFWD<sizeof(colors_std_GrGn) / sizeof(unsigned int)>   paletteGrGn(colors_std_GrGn);
const PaletteConstBWD<sizeof(colors_std_GrGn) / sizeof(unsigned int)>   paletteGrGn_inv(colors_std_GrGn);
const unsigned int colors_std_BkRdWh[] = { 0x00000000, 0x000000ff, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkRdWh) / sizeof(unsigned int)>   paletteBkRdWh(colors_std_BkRdWh);
const PaletteConstBWD<sizeof(colors_std_BkRdWh) / sizeof(unsigned int)>   paletteBkRdWh_inv(colors_std_BkRdWh);
const unsigned int colors_std_BkBlWh[] = { 0x00000000, 0x00ff0000, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkBlWh) / sizeof(unsigned int)>   paletteBkBlWh(colors_std_BkBlWh);
const PaletteConstBWD<sizeof(colors_std_BkBlWh) / sizeof(unsigned int)>   paletteBkBlWh_inv(colors_std_BkBlWh);
const unsigned int colors_std_BkGrWh[] = { 0x00000000, 0x0000ff00, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkGrWh) / sizeof(unsigned int)>   paletteBkGrWh(colors_std_BkGrWh);
const PaletteConstBWD<sizeof(colors_std_BkGrWh) / sizeof(unsigned int)>   paletteBkGrWh_inv(colors_std_BkGrWh);

const unsigned int colors_std_BkGrYeWh[] = { 0x00000000, 0x0000ff00, 0x0000ffff, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkGrYeWh) / sizeof(unsigned int)>   paletteBkGrYeWh(colors_std_BkGrYeWh);
const PaletteConstBWD<sizeof(colors_std_BkGrYeWh) / sizeof(unsigned int)>   paletteBkGrYeWh_inv(colors_std_BkGrYeWh);
const unsigned int colors_std_BkBlGrYeWh[] = { 0x00000000, 0x00ff0000, 0x0000ff00, 0x0000ffff, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkBlGrYeWh) / sizeof(unsigned int)>   paletteBkBlGrYeWh(colors_std_BkBlGrYeWh);
const PaletteConstBWD<sizeof(colors_std_BkBlGrYeWh) / sizeof(unsigned int)>   paletteBkBlGrYeWh_inv(colors_std_BkBlGrYeWh);

const unsigned int colors_std_BkGyGyGyWh[] = { 0x00000000, 0x00555555, 0x00777777, 0x00AAAAAA, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkGyGyGyWh) / sizeof(unsigned int)>   paletteBkGyGyGyWh(colors_std_BkGyGyGyWh);
const PaletteConstBWD<sizeof(colors_std_BkGyGyGyWh) / sizeof(unsigned int)>   paletteBkGyGyGyWh_inv(colors_std_BkGyGyGyWh);
const unsigned int colors_std_BkBlBlBlWh[] = { 0x00000000, 0x00ff0000, 0x00ff5555, 0x00ffAAAA, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_BkBlBlBlWh) / sizeof(unsigned int)>   paletteBkBlBlBlWh(colors_std_BkBlBlBlWh);
const PaletteConstBWD<sizeof(colors_std_BkBlBlBlWh) / sizeof(unsigned int)>   paletteBkBlBlBlWh_inv(colors_std_BkBlBlBlWh);

const unsigned int colors_std_BkRdGnBu[] = { 0x00000000, 0x000000ff, 0x0000ff00, 0x00ff0000 };
const PaletteConstFWD<sizeof(colors_std_BkRdGnBu) / sizeof(unsigned int)>   paletteBkRdGnBu(colors_std_BkRdGnBu);
const PaletteConstBWD<sizeof(colors_std_BkRdGnBu) / sizeof(unsigned int)>   paletteBkRdGnBu_inv(colors_std_BkRdGnBu);

const unsigned int colors_std_RGBMIX[] = { 0x00000000, 0x000000ff, 0x00000000, 0x0000ff00, 0x00000000, 0x00ff0000 };
const PaletteConstFWD<sizeof(colors_std_RGBMIX) / sizeof(unsigned int)>   paletteRGBMIX(colors_std_RGBMIX);
const PaletteConstBWD<sizeof(colors_std_RGBMIX) / sizeof(unsigned int)>   paletteRGBMIX_inv(colors_std_RGBMIX);

const IPalette* const ppalettes_std[] = { &paletteGnYe, &paletteBkWh, &paletteBlWh, &paletteRdWh, &paletteGrGn, 
                                          &paletteBkRdWh, &paletteBkBlWh, &paletteBkGrWh, &paletteBkGrYeWh, &paletteBkBlGrYeWh, 
                                          &paletteBkGyGyGyWh, &paletteBkBlBlBlWh, &paletteBkRdGnBu, &paletteRGBMIX };
const IPalette* const ppalettes_std_inv[] = { &paletteGnYe_inv, &paletteBkWh_inv, &paletteBlWh_inv, &paletteRdWh_inv, &paletteGrGn_inv, 
                                              &paletteBkRdWh_inv, &paletteBkBlWh_inv, &paletteBkGrWh_inv, &paletteBkGrYeWh_inv, &paletteBkBlGrYeWh_inv, 
                                              &paletteBkGyGyGyWh_inv, &paletteBkBlBlBlWh_inv, &paletteBkRdGnBu_inv, &paletteRGBMIX_inv };
const char*     const ppalettenames_std[] = { "std_GnYe", "std_BkWh", "std_BlWh", "std_RdWh", "std_GrGn", 
                                              "std_BkRdWh", "std_BkBlWh", "std_BkGrWh", "std_BkGrYeWh", "std_BkBlGrYeWh", 
                                              "std_BkGyGyGyWh", "std_BkBlBlBlWh", "std_BkRdGnBu", "std_RGBMIX"
                                            };


/// Trivial palettes:
const unsigned int colors_std_Bk[] = { 0x00000000, 0x00000000 };
const PaletteConstFWD<2>   paletteBk(colors_std_Bk);
const PaletteConstBWD<2>   paletteBk_inv(colors_std_Bk);
const unsigned int colors_std_Wh[] = { 0x00FFFFFF, 0x00FFFFFF };
const PaletteConstFWD<2>   paletteWh(colors_std_Wh);
const PaletteConstBWD<2>   paletteWh_inv(colors_std_Wh);

const unsigned int colors_std_Rd[] = { 0x000000FF, 0x000000FF };
const PaletteConstFWD<2>   paletteRd(colors_std_Rd);
const PaletteConstBWD<2>   paletteRd_inv(colors_std_Rd);
const unsigned int colors_std_Gn[] = { 0x0000FF00, 0x0000FF00 };
const PaletteConstFWD<2>   paletteGn(colors_std_Gn);
const PaletteConstBWD<2>   paletteGn_inv(colors_std_Gn);
const unsigned int colors_std_Bu[] = { 0x00FF0000, 0x00FF0000 };
const PaletteConstFWD<2>   paletteBu(colors_std_Bu);
const PaletteConstBWD<2>   paletteBu_inv(colors_std_Bu);



#endif // PALETTES_STD_H

