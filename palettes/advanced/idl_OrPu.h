#ifndef _IDL_ORPU_H
#define _IDL_ORPU_H

#include "../bsipalette.h"

const unsigned int colors_idl_OrPu[] = {
                                        0x00083c80, 0x00083d82, 0x00083e84, 0x00083f86, 0x00084088, 0x0008418a, 0x0007438d, 0x0007448f, 
                                        0x00074591, 0x00074693, 0x00074795, 0x00074897, 0x00074a99, 0x00074b9b, 0x00074c9d, 0x00074f9f, 
                                        0x000750a1, 0x000751a3, 0x000652a5, 0x000653a7, 0x000654a9, 0x000655ab, 0x000656ad, 0x000657af, 
                                        0x000659b1, 0x00065ab3, 0x00075bb5, 0x00075cb7, 0x00085eb9, 0x00085fba, 0x000960bc, 0x000962be, 
                                        0x000a63c0, 0x000b65c2, 0x000b66c3, 0x000c68c5, 0x000c69c7, 0x000d6bc9, 0x000d6cca, 0x000e6ecc, 
                                        0x000e70ce, 0x000f71cf, 0x000f73d1, 0x001075d3, 0x001077d4, 0x001178d6, 0x00117ad7, 0x00127cd9, 
                                        0x00137eda, 0x001380dc, 0x001482dd, 0x001484df, 0x001586e0, 0x001988e2, 0x001c8ae3, 0x00208ce5, 
                                        0x00238ee6, 0x002690e7, 0x002a92e9, 0x002d94ea, 0x003096eb, 0x003498ec, 0x00379aed, 0x003a9cef, 
                                        0x003e9ef0, 0x0041a0f1, 0x0044a2f2, 0x0047a4f3, 0x004ba6f4, 0x004ea8f5, 0x0051aaf6, 0x0054acf7, 
                                        0x0058aef8, 0x005bb0f9, 0x005eb2fa, 0x0061b4fb, 0x0064b6fb, 0x0067b8fc, 0x006bbafc, 0x006ebbfd, 
                                        0x0071bdfd, 0x0074bffd, 0x0077c1fd, 0x007ac3fd, 0x007dc4fd, 0x0080c6fd, 0x0083c8fd, 0x0086c9fe, 
                                        0x0089cbfe, 0x008ccdfe, 0x008fcefe, 0x0092d0fe, 0x0095d1fe, 0x0098d3fe, 0x009bd4fe, 0x009ed6fe, 
                                        0x00a1d7fe, 0x00a4d8fe, 0x00a7dafe, 0x00aadbfe, 0x00addcfd, 0x00b0ddfd, 0x00b3dffd, 0x00b5e0fd, 
                                        0x00b8e1fd, 0x00bbe2fd, 0x00bee3fd, 0x00c1e4fd, 0x00c4e5fc, 0x00c6e6fc, 0x00c9e7fc, 0x00cce8fc, 
                                        0x00cfe9fc, 0x00d1eafb, 0x00d4ebfb, 0x00d7ecfb, 0x00daedfb, 0x00dcedfa, 0x00dfeefa, 0x00e2effa, 
                                        0x00e4f0fa, 0x00e7f1f9, 0x00eaf2f9, 0x00ecf3f9, 0x00eff3f8, 0x00f2f4f8, 0x00f4f5f7, 0x00f7f6f7, 
                                        0x00f7f7f7, 0x00f6f6f5, 0x00f6f5f4, 0x00f5f4f3, 0x00f5f2f2, 0x00f4f1f1, 0x00f4f0ef, 0x00f3efee, 
                                        0x00f3eeed, 0x00f2edec, 0x00f2eceb, 0x00f2ebea, 0x00f1e9e8, 0x00f1e8e7, 0x00f0e7e6, 0x00f0e6e5, 
                                        0x00efe4e4, 0x00efe3e3, 0x00eee2e1, 0x00eee1e0, 0x00eddfdf, 0x00eddede, 0x00eddcdc, 0x00ecdbdb, 
                                        0x00ecdada, 0x00ebd8d9, 0x00ebd7d7, 0x00ead5d6, 0x00e9d4d5, 0x00e7d2d3, 0x00e6d1d2, 0x00e5cfd0, 
                                        0x00e4cecf, 0x00e3ccce, 0x00e2cacc, 0x00e1c9cb, 0x00e1c7c9, 0x00e0c5c8, 0x00dfc4c6, 0x00dec2c5, 
                                        0x00ddc0c3, 0x00dcbec1, 0x00dbbdc0, 0x00dabbbe, 0x00d9b9bc, 0x00d8b7bb, 0x00d7b5b9, 0x00d6b3b7, 
                                        0x00d5b1b6, 0x00d4b0b4, 0x00d2aeb2, 0x00d1acb1, 0x00d0aaaf, 0x00cfa8ad, 0x00cda6ab, 0x00cca4a9, 
                                        0x00caa1a8, 0x00c99fa6, 0x00c89da4, 0x00c69ba2, 0x00c599a0, 0x00c3979f, 0x00c2949d, 0x00c0929b, 
                                        0x00bf9099, 0x00bd8e97, 0x00bc8b95, 0x00ba8993, 0x00b98791, 0x00b78490, 0x00b6828e, 0x00b57f8c, 
                                        0x00b37d8a, 0x00b27a88, 0x00b07886, 0x00af7584, 0x00ad7383, 0x00ac7081, 0x00ab6e7f, 0x00a96b7d, 
                                        0x00a8687b, 0x00a66679, 0x00a56378, 0x00a46076, 0x00a25e74, 0x00a15b72, 0x009f5870, 0x009e556f, 
                                        0x009d526d, 0x009b4f6b, 0x009a4d69, 0x00994a68, 0x00974766, 0x00964464, 0x00944162, 0x00923e61, 
                                        0x00913b5f, 0x008f385d, 0x008e345c, 0x008c315a, 0x008a2e58, 0x00892b57, 0x00872855, 0x00852754, 
                                        0x00832552, 0x00812450, 0x007f224f, 0x007d214d, 0x007b1f4c, 0x00791e4a, 0x00761c49, 0x00741b47, 
                                        0x00721946, 0x006f1844, 0x006d1643, 0x006b1441, 0x00681340, 0x0066113e, 0x0063103d, 0x00610e3b, 
                                        0x005e0d39, 0x005c0b38, 0x00590a36, 0x00570835, 0x00550733, 0x00530532, 0x00500430, 0x004f022e
};

const PaletteConstFWD<sizeof(colors_idl_OrPu) / sizeof(unsigned int)> palette_idl_OrPu(colors_idl_OrPu);
const PaletteConstBWD<sizeof(colors_idl_OrPu) / sizeof(unsigned int)> palette_idl_OrPu_inv(colors_idl_OrPu);

typedef PaletteConstFWD<sizeof(colors_idl_OrPu) / sizeof(unsigned int)> palette_idl_OrPu_t;

#endif // _IDL_ORPU_H