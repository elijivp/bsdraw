#ifndef _IDL_OCEAN_H
#define _IDL_OCEAN_H

#include "../bsipalette.h"

const unsigned int colors_idl_ocean[] = {
                                         0x0093433a, 0x009e433e, 0x00a94342, 0x00b44346, 0x00bf434a, 0x00ca434f, 0x00d54253, 0x00e04257, 
                                         0x00ec415b, 0x00f7415f, 0x00fc4063, 0x00f13f67, 0x00e63e6b, 0x00db3e6f, 0x00d03d73, 0x00c53c77, 
                                         0x00bb3b7b, 0x00b13a7f, 0x00a73983, 0x009d3886, 0x0093378a, 0x008a358d, 0x00813491, 0x00783394, 
                                         0x00703297, 0x0068319a, 0x0060309d, 0x005930a0, 0x00522fa2, 0x004b2ea4, 0x00452da7, 0x00402da9, 
                                         0x003b2cab, 0x00362cad, 0x00322cae, 0x002e2bb0, 0x002a2bb1, 0x00282bb2, 0x00252cb3, 0x00232cb3, 
                                         0x00222cb4, 0x00212db4, 0x00212eb4, 0x00212fb4, 0x002230b4, 0x002331b4, 0x002532b3, 0x002734b2, 
                                         0x002936b1, 0x002d37b0, 0x00303aaf, 0x00343cad, 0x00393eac, 0x003e40aa, 0x004343a8, 0x004946a6, 
                                         0x005049a3, 0x00564ca1, 0x005e4f9e, 0x0065529c, 0x006d5599, 0x00755896, 0x007e5c93, 0x00875f8f, 
                                         0x0090638c, 0x00996788, 0x00a36a85, 0x00ad6e81, 0x00b7727d, 0x00c2767a, 0x00cc7976, 0x00d77d72, 
                                         0x00e2816e, 0x00ed846a, 0x00f88866, 0x00fb8c61, 0x00ef8f5d, 0x00e49259, 0x00d99655, 0x00ce9951, 
                                         0x00c29c4d, 0x00b79f49, 0x00aca145, 0x00a1a441, 0x0096a63d, 0x008ca939, 0x0081ab35, 0x0077ad31, 
                                         0x006dae2d, 0x0063b02a, 0x005ab126, 0x0050b223, 0x0047b31f, 0x003eb31c, 0x0036b419, 0x002eb416, 
                                         0x0026b413, 0x001fb410, 0x0018b30e, 0x0012b30c, 0x000cb209, 0x0006b107, 0x0001af05, 0x0004ae04, 
                                         0x0008ac02, 0x000caa00, 0x000fa800, 0x0012a600, 0x0015a300, 0x0017a100, 0x00189e00, 0x00199b00, 
                                         0x00199800, 0x00199500, 0x00189200, 0x00178f00, 0x00168c00, 0x00138900, 0x00118600, 0x000e8200, 
                                         0x000a7f01, 0x00067c03, 0x00017904, 0x00047606, 0x00097208, 0x000f700a, 0x00156d0d, 0x001c6a0f, 
                                         0x00236712, 0x002b6515, 0x00336317, 0x003b611a, 0x00435f1e, 0x004c5d21, 0x00555c24, 0x005f5a28, 
                                         0x0069592b, 0x0073592f, 0x007d5833, 0x00875837, 0x0092583a, 0x009d583e, 0x00a75942, 0x00b25a46, 
                                         0x00be5b4b, 0x00c95c4f, 0x00d45e53, 0x00df6057, 0x00eb635b, 0x00f6655f, 0x00fd6863, 0x00f26b67, 
                                         0x00e76f6c, 0x00dc7270, 0x00d17674, 0x00c67b77, 0x00bc7f7b, 0x00b2847f, 0x00a88883, 0x009e8d87, 
                                         0x0094938a, 0x008b988d, 0x00829d91, 0x0079a394, 0x0071a997, 0x0069af9a, 0x0061b59d, 0x005abaa0, 
                                         0x0053c0a2, 0x004cc6a5, 0x0046cca7, 0x0040d2a9, 0x003bd8ab, 0x0036dead, 0x0032e4ae, 0x002ee9b0, 
                                         0x002befb1, 0x0028f4b2, 0x0026f9b3, 0x0024feb3, 0x0022fbb4, 0x0021f6b4, 0x0021f2b4, 0x0021eeb4, 
                                         0x0022eab4, 0x0023e7b4, 0x0024e4b3, 0x0027e1b2, 0x0029deb1, 0x002cdcb0, 0x0030daaf, 0x0034d8ad, 
                                         0x0039d7ac, 0x003ed6aa, 0x0043d6a8, 0x0049d5a6, 0x004fd6a3, 0x0056d6a1, 0x005dd79e, 0x0064d89c, 
                                         0x006cda99, 0x0075dc96, 0x007dde92, 0x0086e18f, 0x008fe48c, 0x0099e788, 0x00a2ea85, 0x00acee81, 
                                         0x00b6f27d, 0x00c1f679, 0x00cbfb76, 0x00d6fe72, 0x00e1fa6e, 0x00ecf56a, 0x00f7ef65, 0x00eaf669, 
                                         0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 
                                         0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 
                                         0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 
                                         0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 
                                         0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669, 0x00eaf669
};

const PaletteConstFWD<sizeof(colors_idl_ocean) / sizeof(unsigned int)> palette_idl_ocean(colors_idl_ocean);
const PaletteConstBWD<sizeof(colors_idl_ocean) / sizeof(unsigned int)> palette_idl_ocean_inv(colors_idl_ocean);

typedef PaletteConstFWD<sizeof(colors_idl_ocean) / sizeof(unsigned int)> palette_idl_ocean_t;

#endif // _IDL_OCEAN_H