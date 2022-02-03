/// Overlays:
///   OTestPrecision. View: technical overlay, need to be upgraded
/// Created By: Elijah Vlasov
#include "bstestprecision.h"
#include "../../core/sheigen/bsshgentrace.h"

//#include <QDebug>

OTestPrecision::OTestPrecision(): DrawOverlaySimple(),
  OVLCoordsOff(), OVLDimmsOff()
{
//  appendUniform(DT_1I, &m_blockstate);
}

int OTestPrecision::fshTrace(int overlay, bool rotated, char *to) const
{
  FshTraceGenerator  ocg(this->uniforms(), overlay, rotated, to);
  ocg.goto_func_begin<coords_type_t, dimms_type_t>(this, this);
  {
//    int rel = ocg.add_movecs_pixing(CR_ABSOLUTE_NOSCALED);
//    float pts[][2] = {  {0.0, 0.0}, {}  }
    int pts[][2] = {  {0, 0}, {0, 2}, {2, 0}, {1,1}, {2,2}  };
    for (unsigned int i=0; i<sizeof(pts)/sizeof(pts[0]); i++)
    {
      ocg.push("{");
      {
        ocg.var_fixed("newpoint", pts[i][0], pts[i][1]);
//        ocg.movecs_pix("newpoint", rel);
        ocg.push("ivec2 inormed = icoords - newpoint;");
        ocg.push( "result = vec3(0.8, 0.8, 0.8);" );
        ocg.push( "mixwell = mix(mixwell, 1.0, step(length(vec2(inormed)), 0.0));" );
      }
      ocg.push("}");
    }
  }
  ocg.goto_func_end(false);
//  qDebug()<<to;
  return ocg.written();
}


/*
bool OTestPrecision::overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* doStop)
{
  if (m_blockstate != 0)
  {
    *doStop = true;
    if (oreact == ORM_LMRELEASE)
    {
      m_blockstate = 0;
      return true;
    }
  }
  return false;
}
*/
