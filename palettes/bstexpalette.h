#ifndef BSTEXPALETTE_H
#define BSTEXPALETTE_H

#include "bsipalette.h"

template<int maxCount>
class PaletteSTD: public IPalette
{
protected:
  unsigned int palbuf[maxCount];
public:
  virtual void    getPalette(const void** clrarr, unsigned int* count, unsigned int* format) const
  {
    *clrarr = (const void*)&palbuf;
    *count = maxCount;
    *format = FMT_UNSIGNED_BYTE;
  }
  virtual unsigned int  firstColor() const {  return palbuf[0];  }
public:
  enum  { size = maxCount };
  PaletteSTD(){}
  template <int otherMaxCount>
  PaletteSTD(const PaletteSTD<otherMaxCount>& cpy){  int minimax = maxCount > otherMaxCount? otherMaxCount : maxCount; for (int i=0; i<minimax; i++)  palbuf[i] = cpy.palbuf[i]; }
  template <int otherMaxCount>
  PaletteSTD(const PaletteSTD<otherMaxCount>& cpy, unsigned int otherfirstcolor): PaletteSTD(cpy){  palbuf[0] = otherfirstcolor;  }
//  PaletteSTD(const PaletteSTD<maxCount>& cpy, unsigned int firstcolor){  for (int i=1; i<maxCount; i++)  palbuf[i] = cpy.palbuf[i]; palbuf[0] = firstcolor; }
//  PaletteSTD(const PaletteSTD<maxCount>& cpy){  for (int i=0; i<maxCount; i++)  palbuf[i] = cpy.palbuf[i]; }
//  PaletteSTD(const PaletteSTD<maxCount>& cpy, unsigned int firstcolor){  for (int i=1; i<maxCount; i++)  palbuf[i] = cpy.palbuf[i]; palbuf[0] = firstcolor; }
};

template<int maxCount>
class PaletteConstFWD: public PaletteSTD<maxCount>
{
public:
  PaletteConstFWD(const unsigned int *pbuf){ for (int i=0; i<maxCount; i++)  PaletteSTD<maxCount>::palbuf[i] = pbuf[i]; }
};

template<int maxCount>
class PaletteConstBWD: public PaletteSTD<maxCount>
{
public:
  PaletteConstBWD(const unsigned int *pbuf){ for (int i=0; i<maxCount; i++)  PaletteSTD<maxCount>::palbuf[i] = pbuf[maxCount - 1 - i]; }
};


/* QT_VERSION >= x050000 usage:
 *    
 * 
 * 
 *  QOpenGLFunctions  qoglf(QOpenGLContext::currentContext());
    qoglf.glActiveTexture(GL_TEXTURE0 + ai);
    qoglf.glBindTexture(GL_TEXTURE_2D, bindto);
    qoglf.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, GLsizei(bufsize) / levels, levels, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)buf);
    if (discrete)
    {
      qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    else
    {
      qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qoglf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 *
*/
    

#endif // BSTEXPALETTE_H

