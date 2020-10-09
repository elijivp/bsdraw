#ifndef _IDL_PRISM_H
#define _IDL_PRISM_H

#include "../bsipalette.h"

const unsigned int colors_idl_prism[] = {
                                         0x00000001, 0x00000005, 0x00000009, 0x0000000d, 0x00000011, 0x00000014, 0x00000018, 0x0000001c, 
                                         0x00000020, 0x00000024, 0x00000027, 0x0000002b, 0x0000002f, 0x00000033, 0x00000036, 0x0000003a, 
                                         0x0000003e, 0x00000042, 0x00000046, 0x00000049, 0x0000004d, 0x00000051, 0x00000055, 0x00000059, 
                                         0x0000005c, 0x00000060, 0x00000064, 0x00000068, 0x0000006c, 0x0000006f, 0x00000073, 0x00000077, 
                                         0x0000007b, 0x0000007f, 0x00000082, 0x00000086, 0x0000008a, 0x0000008e, 0x00000091, 0x00000095, 
                                         0x00000099, 0x0000009d, 0x000000a1, 0x000000a4, 0x000000a8, 0x000000ac, 0x000000b0, 0x000000b4, 
                                         0x000000b7, 0x000000bb, 0x000000bf, 0x000000c3, 0x000000c7, 0x000000ca, 0x000000ce, 0x000000d2, 
                                         0x000000d6, 0x000000d9, 0x000000dd, 0x000000e1, 0x000000e5, 0x000000e9, 0x000000ec, 0x000000f0, 
                                         0x000000f4, 0x000004f8, 0x000008fc, 0x00000cff, 0x000010fa, 0x000014f6, 0x000018f2, 0x00001cee, 
                                         0x000020ea, 0x000024e6, 0x000028e1, 0x00002cdd, 0x000030d9, 0x000034d5, 0x000038d1, 0x00003ccd, 
                                         0x000040c8, 0x000044c4, 0x000048c0, 0x00004cbc, 0x000050b8, 0x000054b4, 0x000058af, 0x00005cab, 
                                         0x000060a7, 0x000064a3, 0x0000689f, 0x00006c9b, 0x00007096, 0x00007492, 0x0000788e, 0x00007c8a, 
                                         0x00007f86, 0x00008382, 0x0000877d, 0x00008b79, 0x00008f75, 0x00009371, 0x0000976d, 0x00009b69, 
                                         0x00009f64, 0x0000a360, 0x0000a75c, 0x0000ab58, 0x0000af54, 0x0000b350, 0x0000b74b, 0x0000bb47, 
                                         0x0000bf43, 0x0000c33f, 0x0000c73b, 0x0000cb37, 0x0000cf32, 0x0000d32e, 0x0000d72a, 0x0000db26, 
                                         0x0000df22, 0x0000e31e, 0x0000e719, 0x0000eb15, 0x0000ef11, 0x0000f30d, 0x0000f709, 0x0003fb05, 
                                         0x0007ff00, 0x000bfb00, 0x000ff700, 0x0013f300, 0x0017ef00, 0x001aeb00, 0x001ee700, 0x0022e300, 
                                         0x0026df00, 0x002adb00, 0x002ed700, 0x0032d300, 0x0035cf00, 0x0039cb00, 0x003dc700, 0x0041c300, 
                                         0x0045bf00, 0x0049bb00, 0x004cb700, 0x0050b300, 0x0054af00, 0x0058ab00, 0x005ca700, 0x0060a300, 
                                         0x00649f00, 0x00679b00, 0x006b9700, 0x006f9300, 0x00738f00, 0x00778b00, 0x007b8700, 0x007f8200, 
                                         0x00827e00, 0x00867a00, 0x008a7600, 0x008e7200, 0x00926e00, 0x00966a00, 0x00996600, 0x009d6200, 
                                         0x00a15e00, 0x00a55a00, 0x00a95600, 0x00ad5200, 0x00b14e00, 0x00b44a00, 0x00b84600, 0x00bc4200, 
                                         0x00c03e00, 0x00c43a00, 0x00c83600, 0x00cc3200, 0x00cf2e00, 0x00d32a00, 0x00d72600, 0x00db2200, 
                                         0x00df1e00, 0x00e31a00, 0x00e61600, 0x00ea1200, 0x00ee0e00, 0x00f20a00, 0x00f60600, 0x00fa0200, 
                                         0x00fe0000, 0x00fc0000, 0x00f80000, 0x00f40000, 0x00f00000, 0x00ec0000, 0x00e80000, 0x00e40000, 
                                         0x00e00000, 0x00dc0000, 0x00d80000, 0x00d40000, 0x00d00000, 0x00cc0000, 0x00c80000, 0x00c40000, 
                                         0x00c00000, 0x00bc0000, 0x00b80000, 0x00b40000, 0x00b00000, 0x00ac0000, 0x00a80000, 0x00a40000, 
                                         0x00a00000, 0x009c0000, 0x00980000, 0x00940000, 0x00900000, 0x008c0000, 0x00880000, 0x00830000, 
                                         0x007f0000, 0x007b0000, 0x00770000, 0x00730000, 0x006f0000, 0x006b0000, 0x00670000, 0x00630000, 
                                         0x005f0000, 0x005b0000, 0x00570000, 0x00530000, 0x004f0000, 0x004b0000, 0x00470000, 0x00430000, 
                                         0x003f0000, 0x003b0000, 0x00370000, 0x00330000, 0x002f0000, 0x002b0000, 0x00270000, 0x00230000, 
                                         0x001f0000, 0x001b0000, 0x00170000, 0x00130000, 0x000f0000, 0x000b0000, 0x00070000, 0x00030000
};

const PaletteConstFWD<sizeof(colors_idl_prism) / sizeof(unsigned int)> palette_idl_prism(colors_idl_prism);
const PaletteConstBWD<sizeof(colors_idl_prism) / sizeof(unsigned int)> palette_idl_prism_inv(colors_idl_prism);

typedef PaletteConstFWD<sizeof(colors_idl_prism) / sizeof(unsigned int)> palette_idl_prism_t;

#endif // _IDL_PRISM_H