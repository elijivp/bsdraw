#ifndef _MAT_BONE_H
#define _MAT_BONE_H

#include "../bsipalette.h"

const unsigned int colors_mat_bone[] = {
                                        0x00000000, 0x00000000, 0x00000000, 0x00010000, 0x00020000, 0x00030101, 0x00040202, 0x00060303, 
                                        0x00070404, 0x00080505, 0x00090606, 0x000a0707, 0x000c0808, 0x000d0808, 0x000e0909, 0x000f0a0a, 
                                        0x00110b0b, 0x00120c0c, 0x00130d0d, 0x00140e0e, 0x00150f0f, 0x00170f0f, 0x00181010, 0x00191111, 
                                        0x001a1212, 0x001c1313, 0x001d1414, 0x001e1515, 0x001f1616, 0x00201717, 0x00221717, 0x00231818, 
                                        0x00241919, 0x00251a1a, 0x00261b1b, 0x00281c1c, 0x00291d1d, 0x002a1e1e, 0x002b1f1f, 0x002d1f1f, 
                                        0x002e2020, 0x002f2121, 0x00302222, 0x00312323, 0x00332424, 0x00342525, 0x00352626, 0x00362727, 
                                        0x00382727, 0x00392828, 0x003a2929, 0x003b2a2a, 0x003c2b2b, 0x003e2c2c, 0x003f2d2d, 0x00402e2e, 
                                        0x00412e2e, 0x00422f2f, 0x00443030, 0x00453131, 0x00463232, 0x00473333, 0x00493434, 0x004a3535, 
                                        0x004b3636, 0x004c3636, 0x004d3737, 0x004f3838, 0x00503939, 0x00513a3a, 0x00523b3b, 0x00543c3c, 
                                        0x00553d3d, 0x00563e3e, 0x00573e3e, 0x00583f3f, 0x005a4040, 0x005b4141, 0x005c4242, 0x005d4343, 
                                        0x005e4444, 0x00604545, 0x00614646, 0x00624646, 0x00634747, 0x00654848, 0x00664949, 0x00674a4a, 
                                        0x00684b4b, 0x00694c4c, 0x006b4d4d, 0x006c4d4d, 0x006d4e4e, 0x006e4f4f, 0x00705050, 0x00715151, 
                                        0x00725252, 0x00735353, 0x00745554, 0x00745655, 0x00755755, 0x00765856, 0x00775957, 0x00785b58, 
                                        0x00795c59, 0x007a5d5a, 0x007b5e5b, 0x007c5f5c, 0x007c615d, 0x007d625d, 0x007e635e, 0x007f645f, 
                                        0x00806660, 0x00816761, 0x00826862, 0x00836963, 0x00836a64, 0x00846c64, 0x00856d65, 0x00866e66, 
                                        0x00876f67, 0x00887168, 0x00897269, 0x008a736a, 0x008b746b, 0x008b756c, 0x008c776c, 0x008d786d, 
                                        0x008e796e, 0x008f7a6f, 0x00907b70, 0x00917d71, 0x00927e72, 0x00937f73, 0x00938074, 0x00948274, 
                                        0x00958375, 0x00968476, 0x00978577, 0x00988678, 0x00998879, 0x009a897a, 0x009b8a7b, 0x009b8b7c, 
                                        0x009c8d7c, 0x009d8e7d, 0x009e8f7e, 0x009f907f, 0x00a09180, 0x00a19381, 0x00a29482, 0x00a29583, 
                                        0x00a39683, 0x00a49784, 0x00a59985, 0x00a69a86, 0x00a79b87, 0x00a89c88, 0x00a99e89, 0x00aa9f8a, 
                                        0x00aaa08b, 0x00aba18b, 0x00aca28c, 0x00ada48d, 0x00aea58e, 0x00afa68f, 0x00b0a790, 0x00b1a991, 
                                        0x00b2aa92, 0x00b2ab93, 0x00b3ac93, 0x00b4ad94, 0x00b5af95, 0x00b6b096, 0x00b7b197, 0x00b8b298, 
                                        0x00b9b399, 0x00b9b59a, 0x00bab69b, 0x00bbb79b, 0x00bcb89c, 0x00bdba9d, 0x00bebb9e, 0x00bfbc9f, 
                                        0x00c0bda0, 0x00c1bea1, 0x00c1c0a2, 0x00c2c1a2, 0x00c3c2a3, 0x00c4c3a4, 0x00c5c5a5, 0x00c6c6a6, 
                                        0x00c7c7a7, 0x00c8c8a9, 0x00c9c9aa, 0x00c9c9ab, 0x00cacaad, 0x00cbcbae, 0x00ccccaf, 0x00cdcdb1, 
                                        0x00ceceb2, 0x00cfcfb4, 0x00d0d0b5, 0x00d1d1b6, 0x00d1d1b8, 0x00d2d2b9, 0x00d3d3bb, 0x00d4d4bc, 
                                        0x00d5d5bd, 0x00d6d6bf, 0x00d7d7c0, 0x00d8d8c1, 0x00d8d8c3, 0x00d9d9c4, 0x00dadac6, 0x00dbdbc7, 
                                        0x00dcdcc8, 0x00ddddca, 0x00dedecb, 0x00dfdfcd, 0x00e0e0ce, 0x00e0e0cf, 0x00e1e1d1, 0x00e2e2d2, 
                                        0x00e3e3d3, 0x00e4e4d5, 0x00e5e5d6, 0x00e6e6d8, 0x00e7e7d9, 0x00e8e8da, 0x00e8e8dc, 0x00e9e9dd, 
                                        0x00eaeade, 0x00ebebe0, 0x00ecece1, 0x00edede3, 0x00eeeee4, 0x00efefe5, 0x00efefe7, 0x00f0f0e8, 
                                        0x00f1f1ea, 0x00f2f2eb, 0x00f3f3ec, 0x00f4f4ee, 0x00f5f5ef, 0x00f6f6f0, 0x00f7f7f2, 0x00f7f7f3, 
                                        0x00f8f8f5, 0x00f9f9f6, 0x00fafaf7, 0x00fbfbf9, 0x00fcfcfa, 0x00fdfdfc, 0x00fefefd, 0x00fffffe
};

const PaletteConstFWD<sizeof(colors_mat_bone) / sizeof(unsigned int)> palette_mat_bone(colors_mat_bone);
const PaletteConstBWD<sizeof(colors_mat_bone) / sizeof(unsigned int)> palette_mat_bone_inv(colors_mat_bone);

typedef PaletteConstFWD<sizeof(colors_mat_bone) / sizeof(unsigned int)> palette_mat_bone_t;

#endif // _MAT_BONE_H