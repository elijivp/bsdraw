#ifndef BSGRAPHOPTS_H
#define BSGRAPHOPTS_H

class IPalette;

enum  BSGRAPHTYPE {
                    GT_DOTS,                /// most simple, most lightweight
                    GT_LINTERP_B,           /// linear interpolation through B-oriented straight lines
                    GT_LINTERP_BSS,         /// linear interpolation through B-oriented straight lines and glsl smoothstep method
                    GT_LINTERP_D,           /// linear interpolation diagonal
                    GT_HISTOGRAM,           /// histogram
                    GT_HISTOGRAM_CROSSMAX,  /// histogram with max of many graphs on cross
                    GT_HISTOGRAM_CROSSMIN,  /// histogram with all graphs on cross
                    GT_HISTOGRAM_MESH,      /// histogram with meshing graphs
                    GT_HISTOGRAM_SUM,       /// histogram with rgb-sum graphs
                    GT_HISTOGRAM_LASTBACK,  /// histogram with last portion is backcolor
                };

enum  BSDESCALING {
                    DE_NONE,                /// no descaling (default)
                    DE_LINTERP,             /// descaling through linear interpolation
                      DE_LINTERP_SCALINGLEFT = DE_LINTERP,
                      DE_LINTERP_SCALINGCENTER,
                    DE_CENTER,              /// only central point left
                    DE_TRIANGLE,            /// ... additional smooth descaling algos
                    DE_TRIANGLE2, 
                    DE_HYPERB,
                    DE_SINTERP,             /// halfsmooth descaling through 3 points
                    DE_QINTERP              /// smooth descaling through 4 points
                };

enum  BSPOSTRECT {
                    PR_STANDARD,
                    PR_VALUEAROUND,
                    PR_VALUEONLY,
                    PR_SUMMARY
                };

struct  graphopts_t
{
  BSGRAPHTYPE     graphtype;
  BSDESCALING     descaling;
  float           opacity;
  int             dotsize;                  /// >0 : abs size; <0 : scaling + size
  float           dotsmooth;                /// 0..2. 0.5 optimal, smooth quality
  float           smooth;                   /// -0.4 .. 2.0. 0.5 optimal; <=-1.0 for special 8bit nosmooth
  BSPOSTRECT      postrect;

  // 1. typical linterp graph
  static graphopts_t goInterp(BSDESCALING ds, int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_B, ds, 0.0f, dotsize, dotsmooth, 0.5f, PR_STANDARD }; return result;  }
  static graphopts_t goInterp(float smoothcoef, BSDESCALING ds=DE_NONE, int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_B, ds, 0.0f, dotsize, dotsmooth, smoothcoef, PR_STANDARD}; return result; }
  
  static graphopts_t goInterpD(int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_D, DE_NONE, 0.0f, dotsize, dotsmooth, 0.25f, PR_STANDARD }; return result;  }
  static graphopts_t goInterpD(float smoothcoef, float dsmooth, int dotsize, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_D, DE_NONE, dsmooth, dotsize, dotsmooth, smoothcoef, PR_STANDARD}; return result; }
  
  static graphopts_t goInterpBSS(BSDESCALING ds, int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_BSS, ds, 0.0f, dotsize, dotsmooth, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goInterpBSS(float smoothcoef, BSDESCALING ds, int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_LINTERP_BSS, ds, 0.0f, dotsize, dotsmooth, smoothcoef, PR_STANDARD}; return result; }
  
  static graphopts_t goDots(int dotsize=0, float dotsmooth=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_DOTS, ds, 0.0f, dotsize, dotsmooth, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goDots(BSDESCALING ds, int dotsize=0, float dotsmooth=0.0f)
  { graphopts_t result = { GT_DOTS, ds, 0.0f, dotsize, dotsmooth, 0.0f, PR_STANDARD}; return result; }
  
  static graphopts_t goHistogram(float opacity=0.0f, BSDESCALING ds=DE_NONE, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { graphopts_t result = { GT_HISTOGRAM, ds, opacity, 0, 0.0f, smoothcoef, pr}; return result; }
  static graphopts_t goHistogram(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramCrossMax(float opacity=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMAX, ds, opacity, 0, 0.0f, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goHistogramCrossMax(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMAX, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramCrossMin(float opacity=0.0f, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMIN, ds, opacity, 0, 0.0f, 0.0f, PR_STANDARD}; return result; }
  static graphopts_t goHistogramCrossMin(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_CROSSMIN, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramMesh(float opacity=0.0f, BSDESCALING ds=DE_NONE, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { graphopts_t result = { GT_HISTOGRAM_MESH, ds, opacity, 0, 0.0f, smoothcoef, pr }; return result; }
  static graphopts_t goHistogramMesh(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_MESH, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramSum(float opacity=0.0f, BSDESCALING ds=DE_NONE, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { graphopts_t result = { GT_HISTOGRAM_SUM, ds, opacity, 0, 0.0f, smoothcoef, pr }; return result; }
  static graphopts_t goHistogramSum(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_SUM, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
  
  static graphopts_t goHistogramLastBack(float opacity=0.0f, BSDESCALING ds=DE_NONE, float smoothcoef=0.0f, BSPOSTRECT pr=PR_STANDARD)
  { graphopts_t result = { GT_HISTOGRAM_LASTBACK, ds, opacity, 0, 0.0f, smoothcoef, pr}; return result; }
  static graphopts_t goHistogramLastBack(BSPOSTRECT pr, BSDESCALING ds=DE_NONE)
  { graphopts_t result = { GT_HISTOGRAM_LASTBACK, ds, 0.0f, 0, 0.0f, 0.0f, pr}; return result; }
};

inline graphopts_t& operator+=(graphopts_t& go, BSDESCALING ds){ go.descaling = ds; return go; }
inline graphopts_t& operator+=(graphopts_t& go, BSPOSTRECT pr){ go.postrect = pr; return go; }
inline graphopts_t& operator+=(graphopts_t& go, float opacity){ go.opacity = opacity; return go; }
//inline graphopts_t& operator+=(graphopts_t& go, unsigned int backgroundcolor){ go.backcolor = backgroundcolor; return go; }

inline graphopts_t operator+(const graphopts_t& go, BSDESCALING ds){ graphopts_t result(go); result.descaling = ds; return result; }
inline graphopts_t operator+(const graphopts_t& go, BSPOSTRECT pr){ graphopts_t result(go); result.postrect = pr; return result; }
inline graphopts_t operator+(const graphopts_t& go, float opacity){ graphopts_t result(go); result.opacity = opacity; return result; }
//inline graphopts_t operator+(const graphopts_t& go, unsigned int backgroundcolor){ graphopts_t result(go); result.backcolor = backgroundcolor; return result; }



enum BSCOLORPOLICY          // IPalette split onto graphs
{ 
  CP_MONO,                // Once color, which is cstart + #portion*interval[cstart...cstop]
  CP_PAINTED,            // color, gradiented in palette, according with portions
  CP_PAINTED_GROSS,      // color, gradiented in palette, according with portions, nonlinear gradient
  CP_PAINTED_SYMMETRIC,  // color, gradiented in palette, according with portions, centre symmetrion
  CP_REPAINTED,               // color, gradiented in palette, ignoring portions
//    CP_REPAINTED_GROSS,         // color, gradiented in palette, ignoring portions, nonlinear gradient
  CP_PALETTE,           // portions ignored. value point direct onto palette
  CP_PALETTE_SPLIT       // portions ignored. value point direct onto palette + cstart
};

struct  coloropts_t
{
  BSCOLORPOLICY   cpolicy;
  float           cstart;
  float           cstop;
  unsigned int    backcolor;                /// color to use instead of palette color. 0xFFFFFFFF - not use
  
  static coloropts_t copts(){  coloropts_t copts = { CP_MONO, 0.0f, 1.0f, 0xFFFFFFFF }; return copts; }
  static coloropts_t copts(unsigned int backColor){  coloropts_t copts = { CP_MONO, 0.0f, 1.0f, backColor }; return copts; }
  static coloropts_t copts(BSCOLORPOLICY policy, float colorize_start, float colorize_stop, unsigned int backColor=0xFFFFFFFF)
  { coloropts_t copts = { policy, colorize_start, colorize_stop, backColor}; return copts; }
};


#endif // BSGRAPHOPTS_H
