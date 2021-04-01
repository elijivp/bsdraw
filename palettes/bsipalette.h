#ifndef IPALETTE_H
#define IPALETTE_H

/// Base classes for palettes
/// Created By: Elijah Vlasov


/// UInt -> R,G,B                     RED                GREEN               BLUE
#define DRAWCLREXTRACT_U3I(clr)    (clr) & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF
#define DRAWCLREXTRACT_U3F(clr)    ((clr) & 0xFF)/255.0f, ((clr >> 8) & 0xFF)/255.0f, ((clr >> 16) & 0xFF)/255.0f

/// QColor -> R, G, B
#define DRAWCLREXTRACT_Q3I(QColor) QColor.red(), QColor.green(), QColor.blue()
#define DRAWCLREXTRACT_Q3F(QColor) float(QColor.redF()), float(QColor.greenF()), float(QColor.blueF())



///////////////////////       Main palette interface

class IPalette
{
public:
  virtual void          getPalette(const void** clrarr, unsigned int* count, unsigned int* format) const =0;
  virtual unsigned int  first() const=0;
  virtual unsigned int  last() const=0;
  virtual ~IPalette(){}
public:
  enum  { FMT_UNKNOWN,
          FMT_UNSIGNED_BYTE,
          FMT_FLOAT
        };
};


///////////////////////       Palette realization: Optimal Template-based

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
  virtual unsigned int  first() const {  return palbuf[0];  }
  virtual unsigned int  last() const {  return palbuf[maxCount-1];  }
  unsigned int    operator[](int i) const {  return palbuf[i]; }
  unsigned int&   operator[](int i) {  return palbuf[i]; }
  unsigned int    count() const {  return maxCount; }
  unsigned int    size() const {  return maxCount; }
public:
  PaletteSTD(){}
  template <int otherMaxCount>
  PaletteSTD(const PaletteSTD<otherMaxCount>& cpy){  int minimax = maxCount > otherMaxCount? otherMaxCount : maxCount; for (int i=0; i<minimax; i++)  palbuf[i] = cpy.palbuf[i]; }
  template <int otherMaxCount>
  PaletteSTD(const PaletteSTD<otherMaxCount>& cpy, unsigned int otherfirstcolor)
  {  int minimax = maxCount > otherMaxCount? otherMaxCount : maxCount; for (int i=1; i<minimax; i++)  palbuf[i] = cpy.palbuf[i];  palbuf[0] = otherfirstcolor;  }
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


template<int maxCount>
class PaletteBORDS: public PaletteSTD<maxCount>
{
public:
  PaletteBORDS(unsigned int clr1){ for (int i=0; i<maxCount; i++)  PaletteSTD<maxCount>::palbuf[i] = clr1; }
#define PCD_CAST(clr, brd)  b1 = b2; b2 = int(brd*maxCount);    if (b2 > b1){ if (b2 > maxCount) b2 = maxCount; for (int i=b1; i<b2; i++)  PaletteSTD<maxCount>::palbuf[i] = clr; if (b2 == maxCount) return; }
#define PCD_LAST(clr)       b1 = b2; b2 = maxCount;                                                             for (int i=b1; i<b2; i++)  PaletteSTD<maxCount>::palbuf[i] = clr;
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2)
    {   int b2=0, b1; PCD_CAST(clr1, brd1); PCD_LAST(clr2); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3)
    {   int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_LAST(clr3); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4)
    {   int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_LAST(clr4); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4, float brd4, unsigned int clr5)
    {   int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_LAST(clr5); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6)
    {   int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_LAST(clr6); }
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


///////////////////////       Palette realization: Pointer-based

class PalettePArray: public IPalette
{
protected:
  const unsigned int*   m_palbuf;
  unsigned int    m_count;
  bool            m_owner;
public:
  virtual void    getPalette(const void** clrarr, unsigned int* count, unsigned int* format) const
  {
    *clrarr = (const void*)m_palbuf;
    *count = m_count;
    *format = FMT_UNSIGNED_BYTE;
  }
  virtual unsigned int  first() const {  return m_palbuf[0];  }
  virtual unsigned int  last() const {  return m_palbuf[m_count-1];  }
  unsigned int    operator[](int i) const {  return m_palbuf[i]; }
  unsigned int    count() const {  return m_count; }
public:
  PalettePArray(const unsigned int* buf, unsigned int count, bool owner): m_palbuf(buf), m_count(count), m_owner(owner){}
  ~PalettePArray(){  if (m_owner)  delete []m_palbuf;  }
};

#endif // IPALETTE_H
