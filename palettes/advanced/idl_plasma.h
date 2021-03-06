#ifndef _IDL_PLASMA_H
#define _IDL_PLASMA_H

#include "../bsipalette.h"

const unsigned int colors_idl_plasma[] = {
                                          0x00091302, 0x000d1503, 0x00101704, 0x00141b05, 0x00181e06, 0x001c2308, 0x00202809, 0x00242e0a, 
                                          0x0028350b, 0x002c3c0c, 0x0030430d, 0x00344b0e, 0x0038530f, 0x003c5c10, 0x00406412, 0x00446d13, 
                                          0x00487514, 0x004c7e15, 0x004f8616, 0x00538e17, 0x00579618, 0x005a9d19, 0x005ea41b, 0x0061aa1c, 
                                          0x0065af1d, 0x0068b41e, 0x006bb91f, 0x006ebc20, 0x0071bf21, 0x0074c122, 0x0077c223, 0x007ac324, 
                                          0x007dc225, 0x0080c126, 0x0083bf27, 0x0085bc28, 0x0088b828, 0x008bb429, 0x008eaf2a, 0x0090a92b, 
                                          0x0093a32c, 0x00969c2d, 0x0099942e, 0x009c8d2e, 0x009f852f, 0x00a27c30, 0x00a57431, 0x00a96b32, 
                                          0x00ac6333, 0x00b05a34, 0x00b35235, 0x00b74a36, 0x00bb4237, 0x00bf3b38, 0x00c43439, 0x00c82d3a, 
                                          0x00cd283b, 0x00d1223c, 0x00d61e3d, 0x00db1a3e, 0x00e0173f, 0x00e51540, 0x00ea1342, 0x00ef1343, 
                                          0x00f41344, 0x00fa1445, 0x00ff1647, 0x00fa1848, 0x00f41c49, 0x00ef204b, 0x00ea254c, 0x00e52a4e, 
                                          0x00e0304f, 0x00db3750, 0x00d63e52, 0x00d14653, 0x00cc4d55, 0x00c85656, 0x00c45e57, 0x00bf6759, 
                                          0x00bc6f5a, 0x00b8785b, 0x00b4805c, 0x00b1885e, 0x00ae905f, 0x00ab9860, 0x00a89f61, 0x00a6a562, 
                                          0x00a4ab63, 0x00a1b164, 0x009fb665, 0x009eba66, 0x009cbd67, 0x009ac068, 0x0099c269, 0x0098c269, 
                                          0x0096c36a, 0x0095c26b, 0x0094c06b, 0x0093be6c, 0x0091bb6d, 0x0090b76d, 0x008eb26e, 0x008dad6e, 
                                          0x008ba76f, 0x008aa170, 0x00889a70, 0x00859271, 0x00838b71, 0x00818272, 0x007e7a73, 0x007b7273, 
                                          0x00776974, 0x00746175, 0x00705876, 0x006c5076, 0x00674877, 0x00634078, 0x005e3979, 0x0058327a, 
                                          0x00532c7b, 0x004d267c, 0x0047217d, 0x00411d7f, 0x003b1980, 0x00341681, 0x002e1483, 0x00271384, 
                                          0x00201286, 0x00191387, 0x00131489, 0x000c168a, 0x0005198c, 0x00fd1d8e, 0x00f7218f, 0x00f02691, 
                                          0x00ea2c93, 0x00e43294, 0x00de3996, 0x00d84098, 0x00d34899, 0x00cd509b, 0x00c9589d, 0x00c4609e, 
                                          0x00c069a0, 0x00bc72a1, 0x00b97aa3, 0x00b682a4, 0x00b38aa5, 0x00b092a7, 0x00ae9aa8, 0x00ada1a9, 
                                          0x00aba7aa, 0x00aaadab, 0x00a9b2ac, 0x00a9b7ad, 0x00a8bbad, 0x00a8beae, 0x00a8c0af, 0x00a8c2af, 
                                          0x00a8c3b0, 0x00a9c2b0, 0x00a9c2b1, 0x00a9c0b1, 0x00a9bdb1, 0x00aabab2, 0x00aab6b2, 0x00a9b1b2, 
                                          0x00a9acb3, 0x00a9a5b3, 0x00a89fb3, 0x00a798b3, 0x00a590b4, 0x00a388b4, 0x00a180b5, 0x009f78b5, 
                                          0x009c6fb5, 0x009867b6, 0x00945eb7, 0x009056b7, 0x008c4eb8, 0x008746b9, 0x00813eba, 0x007b37bb, 
                                          0x007530bc, 0x006e2abd, 0x006725be, 0x006020bf, 0x00581cc1, 0x005118c2, 0x004916c4, 0x004114c5, 
                                          0x003813c7, 0x003013c9, 0x002713ca, 0x001f15cc, 0x001717ce, 0x000e1ad0, 0x00061ed2, 0x00fd22d4, 
                                          0x00f628d6, 0x00ee2dd8, 0x00e734da, 0x00e03bdc, 0x00da42de, 0x00d44ae0, 0x00ce52e2, 0x00c95ae3, 
                                          0x00c563e5, 0x00c16be7, 0x00bd74e9, 0x00ba7cea, 0x00b785ec, 0x00b58ded, 0x00b394ee, 0x00b29cef, 
                                          0x00b1a3f0, 0x00b1a9f1, 0x00b1aff2, 0x00b2b4f3, 0x00b2b8f4, 0x00b4bcf5, 0x00b5bff5, 0x00b6c1f6, 
                                          0x00b8c2f6, 0x00bac3f6, 0x00bcc2f6, 0x00bec1f7, 0x00bfbff7, 0x00c1bcf7, 0x00c3b9f7, 0x00c4b5f7, 
                                          0x00c5b0f7, 0x00c6aaf7, 0x00c6a4f7, 0x00c69df7, 0x00c696f7, 0x00c58ef7, 0x00c486f7, 0x00c27ef7, 
                                          0x00c075f7, 0x00bd6df8, 0x00b964f8, 0x00b55cf8, 0x00b053f9, 0x00ab4bfa, 0x00a543fa, 0x009f3cfb, 
                                          0x009835fc, 0x00912efd, 0x008928fe, 0x008123fe, 0x00781efd, 0x006f1bfb, 0x006617fa, 0x005c15f8
};

const PaletteConstFWD<sizeof(colors_idl_plasma) / sizeof(unsigned int)> palette_idl_plasma(colors_idl_plasma);
const PaletteConstBWD<sizeof(colors_idl_plasma) / sizeof(unsigned int)> palette_idl_plasma_inv(colors_idl_plasma);

typedef PaletteConstFWD<sizeof(colors_idl_plasma) / sizeof(unsigned int)> palette_idl_plasma_t;

#endif // _IDL_PLASMA_H