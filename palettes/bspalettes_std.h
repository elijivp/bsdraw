#ifndef PALETTES_STD_H
#define PALETTES_STD_H

/// Standard and trivial palettes
/// Created By: Elijah Vlasov

#include "bsipalette.h"


// Use PaletteBords (from bsipalette.h) for more detailed palettes
inline PaletteConstFWD<2> constructPalette(unsigned int color1, unsigned int color2)
{
  unsigned int tmp[] = { color1, color2 };
  return PaletteConstFWD<2>(tmp);
}

// Use PaletteBords (from bsipalette.h) for more detailed palettes
inline PaletteConstFWD<3> constructPalette(unsigned int color1, unsigned int color2, unsigned int color3)
{
  unsigned int tmp[] = { color1, color2, color3 };
  return PaletteConstFWD<3>(tmp);
}


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
const unsigned int colors_std_GnWh[] = { 0x0000ff00, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_GnWh) / sizeof(unsigned int)>   paletteGnWh(colors_std_GnWh);
const PaletteConstBWD<sizeof(colors_std_GnWh) / sizeof(unsigned int)>   paletteGnWh_inv(colors_std_GnWh);

const unsigned int colors_std_hBlWh[] = { 0x00770000, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_hBlWh) / sizeof(unsigned int)>   palettehBlWh(colors_std_hBlWh);
const PaletteConstBWD<sizeof(colors_std_hBlWh) / sizeof(unsigned int)>   palettehBlWh_inv(colors_std_hBlWh);
const unsigned int colors_std_hRdWh[] = { 0x00000077, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_hRdWh) / sizeof(unsigned int)>   palettehRdWh(colors_std_hRdWh);
const PaletteConstBWD<sizeof(colors_std_hRdWh) / sizeof(unsigned int)>   palettehRdWh_inv(colors_std_hRdWh);
const unsigned int colors_std_hGnWh[] = { 0x00007700, 0x00ffffff };
const PaletteConstFWD<sizeof(colors_std_hGnWh) / sizeof(unsigned int)>   palettehGnWh(colors_std_hGnWh);
const PaletteConstBWD<sizeof(colors_std_hGnWh) / sizeof(unsigned int)>   palettehGnWh_inv(colors_std_hGnWh);

const unsigned int colors_std_GrGn[] = { 0x00444444, 0x0000ff00 };
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


/// Standart preinstalled palette:
const unsigned int colors_creature[] = { 
  0x00010100, 0x00030300, 0x00050500, 0x00070700, 0x00090900, 0x000b0a00, 0x000d0c00, 0x000f0e00,
  0x00110f00, 0x00131100, 0x00151200, 0x00171300, 0x00191400, 0x001b1500, 0x001d1600, 0x001f1700, 
  0x00211800, 0x00231900, 0x00251a00, 0x00271a00, 0x00291b00, 0x002b1c00, 0x002d1c00, 0x002f1c00,
  0x00311d00, 0x00331d00, 0x00351d00, 0x00371d00, 0x00391d00, 0x003b1d00, 0x003d1d00, 0x003f1c00,
  0x00411c00, 0x00431b00, 0x00451b00, 0x00471a00, 0x00491a00, 0x004b1900, 0x004d1800, 0x004f1700,
  0x00511600, 0x00531500, 0x00551400, 0x00571300, 0x00591200, 0x005b1000, 0x005d0f00, 0x005f0d00, 
  0x00610c00, 0x00630a00, 0x00650900, 0x00670700, 0x00690500, 0x006b0300, 0x006d0100, 0x006f0001, 
  0x00710003, 0x00730006, 0x00750008, 0x0077000a, 0x0079000d, 0x007b0010, 0x007d0012, 0x007f0015, 
  0x00800018, 0x0082001b, 0x0084001e, 0x00860021, 0x00880024, 0x008a0027, 0x008c002a, 0x008e002d, 
  0x00900031, 0x00920034, 0x00940038, 0x0096003c, 0x0098003f, 0x009a0043, 0x009c0047, 0x009e004b, 
  0x009d0050, 0x009b0155, 0x0099025a, 0x00970360, 0x00950465, 0x0093056a, 0x00910670, 0x008f0775, 
  0x008e087a, 0x008c097f, 0x008a0a85, 0x00880b8a, 0x00860c8f, 0x00840d95, 0x00820e9a, 0x00800f9f, 
  0x007e10a5, 0x007d11aa, 0x007b12af, 0x007913b4, 0x007714ba, 0x007515bf, 0x007316c4, 0x007117ca, 
  0x006f18cf, 0x006e19d4, 0x006c1ad9, 0x006a1bdf, 0x00681ce4, 0x00661de9, 0x00641eef, 0x00621ff4, 
  0x005e21ff, 0x005a23ff, 0x005725ff, 0x005427ff, 0x005129ff, 0x004e2bff, 0x004c2dff, 0x00492fff, 
  0x004631ff, 0x004433ff, 0x004135ff, 0x003f37ff, 0x003d39ff, 0x003b3bff, 0x003d3fff, 0x003f44ff,
  0x004149ff, 0x00434eff, 0x004552ff, 0x004757ff, 0x00495bff, 0x004b5fff, 0x004d64ff, 0x004f68ff,
  0x00516cff, 0x005371ff, 0x005575ff, 0x005779ff, 0x00597dff, 0x005b81ff, 0x005d84ff, 0x005f88ff, 
  0x00618cff, 0x006390ff, 0x006593ff, 0x006797ff, 0x00699bff, 0x006b9eff, 0x006da1ff, 0x006fa5ff, 
  0x0071a8ff, 0x0073abff, 0x0075afff, 0x0077b2ff, 0x0079b5ff, 0x007bb8ff, 0x007dbbff, 0x007fbeff, 
  0x0080c1ff, 0x0082c3ff, 0x0084c6ff, 0x0086c9ff, 0x0088cbff, 0x008aceff, 0x008cd0ff, 0x008ed3ff, 
  0x0090d5ff, 0x0092d8ff, 0x0094daff, 0x0096dcff, 0x0098deff, 0x009ae0ff, 0x009ce2ff, 0x009ee4ff, 
  0x00a0e6ff, 0x00a2e8ff, 0x00a4eaff, 0x00a6ecff, 0x00a8edff, 0x00aaefff, 0x00acf1ff, 0x00aef2ff, 
  0x00b0f4ff, 0x00b2f5ff, 0x00b4f6ff, 0x00b6f8ff, 0x00b8f9ff, 0x00bafaff, 0x00bcfbff, 0x00befcff, 
  0x00c0fdff, 0x00c2feff, 0x00c4ffff, 0x00c6fffe, 0x00c8fffd, 0x00cafffd, 0x00ccfffc, 0x00cefffc, 
  0x00d0fffb, 0x00d2fffa, 0x00d4fffa, 0x00d6fffa, 0x00d8fff9, 0x00dafff9, 0x00dcfff9, 0x00defff9,
  0x00e0fff9, 0x00e2fff9, 0x00e4fff9, 0x00e6fff9, 0x00e8fff9, 0x00eafff9, 0x00ecfffa, 0x00eefffa, 
  0x00f0fffa, 0x00f2fffb, 0x00f4fffb, 0x00f6fffc, 0x00f8fffc, 0x00fafffd, 0x00fcfffe, 0x00feffff
};
    
static const PaletteConstFWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature(colors_creature);
static const PaletteConstBWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature_inv(colors_creature);

typedef PaletteConstFWD<sizeof(colors_creature) / sizeof(unsigned int)> palette_creature_t;


#endif // PALETTES_STD_H

