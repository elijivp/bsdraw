#ifndef _TFM_SEISMIC_H
#define _TFM_SEISMIC_H
#include "../bstexpalette.h"
const unsigned int colors_tfm_seismic[] = {
                                           0x00150005, 0x00190006, 0x001c0008, 0x00200009, 0x0023000b, 0x0027000c, 0x002a000d, 0x002e000e, 
                                           0x00310010, 0x00350011, 0x00380012, 0x003c0013, 0x003f0014, 0x00430015, 0x00460016, 0x004a0016, 
                                           0x004e0017, 0x00510018, 0x00550018, 0x00580019, 0x005c0019, 0x0060001a, 0x0063001a, 0x0067001b, 
                                           0x006b001b, 0x006e001b, 0x0072001b, 0x0076001c, 0x0079001c, 0x007d001c, 0x0081001c, 0x0085001b, 
                                           0x0088001b, 0x008c001b, 0x0090001b, 0x0094001b, 0x0098001a, 0x009c001a, 0x00a00019, 0x00a40019, 
                                           0x00a80018, 0x00ac0018, 0x00b00017, 0x00b40016, 0x00b80015, 0x00bc0014, 0x00c00013, 0x00c40012, 
                                           0x00c80011, 0x00cd0010, 0x00d1000f, 0x00d5000e, 0x00d9000d, 0x00de000b, 0x00e2000a, 0x00e60009, 
                                           0x00eb0007, 0x00ef0006, 0x00f40004, 0x00f80002, 0x00fd0001, 0x00ff0302, 0x00ff0907, 0x00ff0f0b, 
                                           0x00ff150f, 0x00ff1b13, 0x00ff2117, 0x00ff261b, 0x00ff2c1f, 0x00ff3224, 0x00ff3728, 0x00ff3c2c, 
                                           0x00ff4230, 0x00ff4734, 0x00ff4c38, 0x00ff513d, 0x00ff5741, 0x00ff5c45, 0x00ff6149, 0x00ff664d, 
                                           0x00ff6a51, 0x00ff6f56, 0x00ff745a, 0x00ff795e, 0x00ff7d62, 0x00ff8266, 0x00ff866a, 0x00ff8b6e, 
                                           0x00ff8f73, 0x00ff9377, 0x00ff987b, 0x00ff9c7f, 0x00ffa083, 0x00ffa487, 0x00ffa88c, 0x00ffac90, 
                                           0x00ffb094, 0x00ffb498, 0x00ffb89c, 0x00ffbba0, 0x00ffbfa5, 0x00ffc3a9, 0x00ffc6ad, 0x00ffc9b1, 
                                           0x00ffcdb5, 0x00ffd0b9, 0x00ffd3bd, 0x00ffd7c2, 0x00ffdac6, 0x00ffddca, 0x00ffe0ce, 0x00ffe3d2, 
                                           0x00ffe6d6, 0x00ffe9db, 0x00ffebdf, 0x00ffeee3, 0x00fff1e7, 0x00fff3eb, 0x00fff6ef, 0x00fff8f3, 
                                           0x00fffbf8, 0x00fffdfc, 0x00fefeff, 0x00fafbff, 0x00f7f9ff, 0x00f3f6ff, 0x00eff3ff, 0x00ecf0ff, 
                                           0x00e8edff, 0x00e4eaff, 0x00e0e6ff, 0x00dde3ff, 0x00d9e0ff, 0x00d5ddff, 0x00d2daff, 0x00ced7ff, 
                                           0x00cad3ff, 0x00c6d0ff, 0x00c3cdff, 0x00bfc9ff, 0x00bbc6ff, 0x00b8c3ff, 0x00b4bfff, 0x00b0bcff, 
                                           0x00acb8ff, 0x00a9b5ff, 0x00a5b1ff, 0x00a1aeff, 0x009daaff, 0x009aa6ff, 0x0096a3ff, 0x00929fff, 
                                           0x008f9bff, 0x008b98ff, 0x008794ff, 0x008390ff, 0x00808cff, 0x007c89ff, 0x007885ff, 0x007581ff, 
                                           0x00717dff, 0x006d79ff, 0x006975ff, 0x006671ff, 0x00626dff, 0x005e69ff, 0x005b65ff, 0x005761ff, 
                                           0x00535cff, 0x004f58ff, 0x004c54ff, 0x004850ff, 0x00444cff, 0x004147ff, 0x004f43ff, 0x004c3fff, 
                                           0x00493aff, 0x004736ff, 0x004431ff, 0x00412dff, 0x003e29ff, 0x003c24ff, 0x003920ff, 0x00361bff, 
                                           0x003316ff, 0x003112ff, 0x002e0dff, 0x002b08ff, 0x002804ff, 0x002600ff, 0x002300ff, 0x002200ff, 
                                           0x002200ff, 0x002200fc, 0x002200f9, 0x002300f7, 0x002300f4, 0x002300f1, 0x002300ef, 0x002300ec, 
                                           0x002300e9, 0x002300e6, 0x002400e4, 0x002400e1, 0x002400de, 0x002400dc, 0x002400d9, 0x002400d6, 
                                           0x002400d4, 0x002400d1, 0x002400ce, 0x002400cc, 0x002400c9, 0x002400c6, 0x002400c4, 0x002400c1, 
                                           0x002400be, 0x002400bb, 0x002400b9, 0x002300b6, 0x002300b3, 0x002300b1, 0x002300ae, 0x002300ab, 
                                           0x002300a9, 0x002300a6, 0x002200a3, 0x002200a1, 0x0022009e, 0x0022009b, 0x00210099, 0x00210096, 
                                           0x00210093, 0x00210090, 0x0020008e, 0x0020008b, 0x00200088, 0x001f0086, 0x001f0083, 0x001f0080, 
                                           0x001e007e, 0x001e007b, 0x001e0078, 0x001d0076, 0x001d0073, 0x001c0070, 0x001c006e, 0x001b006b, 
                                           0x001b0068, 0x001b0065, 0x001a0063, 0x001a0060, 0x0019005d, 0x0019005b, 0x00180058, 0x00170055
};
const PaletteConstFWD<sizeof(colors_tfm_seismic) / sizeof(unsigned int)> palette_tfm_seismic(colors_tfm_seismic);
const PaletteConstBWD<sizeof(colors_tfm_seismic) / sizeof(unsigned int)> palette_tfm_seismic_inv(colors_tfm_seismic);
typedef PaletteConstFWD<sizeof(colors_tfm_seismic) / sizeof(unsigned int)> palette_tfm_seismic_t;
#endif // _TFM_SEISMIC_H