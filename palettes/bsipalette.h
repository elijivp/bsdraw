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
  virtual void          paletteData(const void** clrarr, unsigned int* count, unsigned int* format) const =0;
  virtual unsigned int  first() const=0;
  virtual unsigned int  last() const=0;
  virtual bool          paletteDiscretion() const=0;
  virtual ~IPalette(){}
public:
  enum  { FMT_UNKNOWN,
          FMT_UNSIGNED_BYTE,
          FMT_FLOAT
        };
};


///////////////////////       Palette realization: Optimal Template-based

template<int maxCount, bool _discretion>
class PaletteSTD: public IPalette
{
protected:
  unsigned int  palbuf[maxCount];
public:
  enum  { TOTAL = maxCount };
  virtual void    paletteData(const void** clrarr, unsigned int* count, unsigned int* format) const
  {
    *clrarr = (const void*)&palbuf;
    *count = maxCount;
    *format = FMT_UNSIGNED_BYTE;
  }
  virtual unsigned int  first() const {  return palbuf[0];  }
  virtual unsigned int  last() const {  return palbuf[maxCount-1];  }
  virtual bool          paletteDiscretion() const { return _discretion; }
  unsigned int    operator[](int i) const {  return palbuf[i]; }
  unsigned int&   operator[](int i) {  return palbuf[i]; }
  unsigned int    count() const {  return maxCount; }
  unsigned int    size() const {  return maxCount; }
public:
  PaletteSTD(){}
  template <int otherMaxCount, bool otherDiscretion>  PaletteSTD(const PaletteSTD<otherMaxCount, otherDiscretion>& cpy)
  {
    int minimax = maxCount > otherMaxCount? otherMaxCount : maxCount;
    for (int i=0; i<minimax; i++)  palbuf[i] = cpy.palbuf[i];
  }
  template <int otherMaxCount, bool otherDiscretion> PaletteSTD(const PaletteSTD<otherMaxCount, otherDiscretion>& cpy, unsigned int otherfirstcolor)
  {
    int minimax = maxCount > otherMaxCount? otherMaxCount : maxCount; 
    for (int i=1; i<minimax; i++)  palbuf[i] = cpy.palbuf[i];
    palbuf[0] = otherfirstcolor;
  }
};


template<int maxCount, bool discretion=false>
class PaletteConstFWD: public PaletteSTD<maxCount, discretion>
{
public:
  PaletteConstFWD(const unsigned int *pbuf)
  {
    for (int i=0; i<maxCount; i++)
      PaletteSTD<maxCount, discretion>::palbuf[i] = pbuf[i];
  }
};

template<int maxCount, bool discretion=false>
class PaletteConstBWD: public PaletteSTD<maxCount, discretion>
{
public:
  PaletteConstBWD(const unsigned int *pbuf)
  {
    for (int i=0; i<maxCount; i++)
      PaletteSTD<maxCount, discretion>::palbuf[i] = pbuf[maxCount - 1 - i];
  }
};


template<int maxCount>
class PaletteBORDS: public PaletteSTD<maxCount, true>
{
public:
  PaletteBORDS(unsigned int clr1) { for (int i=0; i<maxCount; i++)  PaletteSTD<maxCount, true>::palbuf[i] = clr1; }
#define PCD_CAST(clr, brd)  b1 = b2; b2 = int(brd*maxCount);    if (b2 > b1){ if (b2 > maxCount) b2 = maxCount; for (int i=b1; i<b2; i++)  PaletteSTD<maxCount, true>::palbuf[i] = clr; if (b2 == maxCount) return; }
#define PCD_LAST(clr)       b1 = b2; b2 = maxCount;                                                             for (int i=b1; i<b2; i++)  PaletteSTD<maxCount, true>::palbuf[i] = clr;
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_LAST(clr2); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_LAST(clr3); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_LAST(clr4); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4, float brd4, unsigned int clr5)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_LAST(clr5); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_LAST(clr6); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, unsigned int clr7)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_CAST(clr6, brd6); PCD_LAST(clr7); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, unsigned int clr7, float brd7, unsigned int clr8)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_LAST(clr8); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_LAST(clr9); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9, unsigned int clr10)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_LAST(clr10); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9,
               unsigned int clr10, float brd10, unsigned int clr11)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); 
      PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_CAST(clr10, brd10); PCD_LAST(clr11); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9,
               unsigned int clr10, float brd10, unsigned int clr11, float brd11, unsigned int clr12)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); 
      PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_CAST(clr10, brd10); PCD_CAST(clr11, brd11); PCD_LAST(clr12); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9,
               unsigned int clr10, float brd10, unsigned int clr11, float brd11, 
               unsigned int clr12, float brd12, unsigned int clr13)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); 
      PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_CAST(clr10, brd10); PCD_CAST(clr11, brd11); 
      PCD_CAST(clr12, brd12);  PCD_LAST(clr13); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9,
               unsigned int clr10, float brd10, unsigned int clr11, float brd11, 
               unsigned int clr12, float brd12, unsigned int clr13, float brd13, unsigned int clr14)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); 
      PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_CAST(clr10, brd10); PCD_CAST(clr11, brd11); 
      PCD_CAST(clr12, brd12);  PCD_CAST(clr13, brd13);  PCD_LAST(clr14); }
  PaletteBORDS(unsigned int clr1, float brd1, unsigned int clr2, float brd2, unsigned int clr3, float brd3, 
               unsigned int clr4, float brd4, unsigned int clr5, float brd5, unsigned int clr6, float brd6, 
               unsigned int clr7, float brd7, unsigned int clr8, float brd8, unsigned int clr9, float brd9,
               unsigned int clr10, float brd10, unsigned int clr11, float brd11, 
               unsigned int clr12, float brd12, unsigned int clr13, float brd13, 
               unsigned int clr14, float brd14, unsigned int clr15)
    { int b2=0, b1; PCD_CAST(clr1, brd1); PCD_CAST(clr2, brd2); PCD_CAST(clr3, brd3); PCD_CAST(clr4, brd4); PCD_CAST(clr5, brd5); 
      PCD_CAST(clr6, brd6); PCD_CAST(clr7, brd7); PCD_CAST(clr8, brd8); PCD_CAST(clr9, brd9); PCD_CAST(clr10, brd10); PCD_CAST(clr11, brd11); 
      PCD_CAST(clr12, brd12);  PCD_CAST(clr13, brd13);  PCD_CAST(clr14, brd14);  PCD_LAST(clr15); }
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
  unsigned int          m_count;
  bool                  m_discretion;
  bool                  m_owner;
public:
  virtual void    paletteData(const void** clrarr, unsigned int* count, unsigned int* format) const
  {
    *clrarr = (const void*)m_palbuf;
    *count = m_count;
    *format = FMT_UNSIGNED_BYTE;
  }
  virtual unsigned int  first() const {  return m_palbuf[0];  }
  virtual unsigned int  last() const {  return m_palbuf[m_count-1];  }
  virtual bool          paletteDiscretion() const { return m_discretion; }
  unsigned int    operator[](int i) const {  return m_palbuf[i]; }
  unsigned int    count() const {  return m_count; }
  void            setDiscretion(bool _discretion){  m_discretion = _discretion; }
public:
  PalettePArray(const unsigned int* buf, unsigned int count, bool owner, bool discretion=false): 
    m_palbuf(buf), m_count(count), m_discretion(discretion), m_owner(owner){}
  ~PalettePArray(){  if (m_owner)  delete []m_palbuf;  }
};

#include "stdarg.h"

class PaletteENUM: public PaletteSTD<24, true>
{
  float         m_k, m_b;
  int           m_states;
private:
  void  reconstruct(int count, ...)
  {
    m_states = count;
    const int maxCount = TOTAL;
    int single = maxCount / count;
    va_list rct;
    va_start(rct, count);
    for (int i=0; i<count; i++)
    {
      unsigned int clr = va_arg(rct, unsigned int);
      for (int j=0; j<single; j++)
        palbuf[i*single + j] = clr;
    }
    va_end(rct);
    for (int i=single*count; i<maxCount; i++)
      palbuf[i] = 0xFFFFFFFF;
    m_k = 1.0f / (maxCount) * single;
    m_b = 1.0f / (maxCount);
  }
public:
  float findex(int idx) const { return m_b + idx*m_k; }
  float operator()(int idx) const { return findex(idx); }
public:
  int   states() const { return m_states; }
  int   uniqueColors() const { return m_states; }
public:
  PaletteENUM(unsigned int clr1){ for (int i=0; i<TOTAL; i++)  palbuf[i] = clr1; m_k = 0.0f; m_b = 0.0f; m_states = 1; }
  PaletteENUM(unsigned int clr1, unsigned int clr2){ reconstruct(2, clr1, clr2); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3){ reconstruct(3, clr1, clr2, clr3); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4){ reconstruct(4, clr1, clr2, clr3, clr4); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5){ reconstruct(5, clr1, clr2, clr3, clr4, clr5); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6){ reconstruct(6, clr1, clr2, clr3, clr4, clr5, clr6); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7){ reconstruct(7, clr1, clr2, clr3, clr4, clr5, clr6, clr7); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8){ reconstruct(8, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9){ reconstruct(9, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10){ reconstruct(10, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11){ reconstruct(11, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19, unsigned int clr20){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19, clr20); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19, unsigned int clr20, unsigned int clr21){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19, clr20, clr21); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19, unsigned int clr20, unsigned int clr21, unsigned int clr22){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19, clr20, clr21, clr22); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19, unsigned int clr20, unsigned int clr21, unsigned int clr22, unsigned int clr23){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19, clr20, clr21, clr22, clr23); }
  PaletteENUM(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, 
                  unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12,
                  unsigned int clr13, unsigned int clr14, unsigned int clr15, unsigned int clr16, unsigned int clr17, unsigned int clr18,
                  unsigned int clr19, unsigned int clr20, unsigned int clr21, unsigned int clr22, unsigned int clr23, unsigned int clr24){ reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12, clr13, clr14, clr15, clr16, clr17, clr18, clr19, clr20, clr21, clr22, clr23, clr24); }
  
public:
  template <int maxCount>
  PaletteENUM(const PaletteBORDS<maxCount>& cpy)
  {
    int NM = TOTAL < maxCount? TOTAL : maxCount;
    for (int i=0; i<NM; i++)
      palbuf[i] = cpy.palbuf[i];
    m_k = 1.0f / (maxCount) * 1;
    m_b = 1.0f / (maxCount);
    m_states = NM;
  }
};


//template <unsigned int color> class PaletteENUM: public PaletteSTD<1, true> {
//public: PaletteENUM(){ 
//    PaletteSTD<1, true>::palbuf[0] = color;
//  } };

//template <unsigned int clr1, unsigned int clr2> class PaletteENUM: public PaletteSTD<2, true> {
//public: PaletteENUM(){ 
//    PaletteSTD<1, true>::palbuf[0] = color;
//  } };


/*
 * template<int maxCount>
class PaletteIMPACT: public PaletteSTD<maxCount, true>
{
  float       m_k, m_b;
private:
  void  reconstruct(int count, ...)
  {
    int single = maxCount / count;
    va_list rct;
    va_start(rct, count);
    for (int i=0; i<count; i++)
    {
      unsigned int clr = va_arg(rct, unsigned int);
      for (int j=0; j<single; j++)
        PaletteSTD<maxCount, true>::palbuf[i*single + j] = clr;
    }
    va_end(rct);
    for (int i=single*count; i<maxCount; i++)
      PaletteSTD<maxCount, true>::palbuf[i] = 0xFFFFFFFF;
    m_k = 1.0f / (maxCount) * single;
    m_b = 1.0f / (maxCount);
  }
public:
  float at(int idx) const { return m_b + idx*m_k; }
  float operator()(int idx) const { return at(idx); }
  PaletteIMPACT(unsigned int clr1){ for (int i=0; i<maxCount; i++)  PaletteSTD<maxCount, true>::palbuf[i] = clr1; m_k = 0.0f; m_b = 0.0f; }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2){ reconstruct(2, clr1, clr2); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3){ reconstruct(3, clr1, clr2, clr3); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4){ reconstruct(4, clr1, clr2, clr3, clr4); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5){ reconstruct(5, clr1, clr2, clr3, clr4, clr5); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6){ reconstruct(6, clr1, clr2, clr3, clr4, clr5, clr6); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7)
  { reconstruct(7, clr1, clr2, clr3, clr4, clr5, clr6, clr7); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7, unsigned int clr8)
  { reconstruct(8, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7, unsigned int clr8, unsigned int clr9)
  { reconstruct(9, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10)
  { reconstruct(10, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11)
  { reconstruct(11, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11); }
  PaletteIMPACT(unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned int clr4, unsigned int clr5, unsigned int clr6, unsigned int clr7, unsigned int clr8, unsigned int clr9, unsigned int clr10, unsigned int clr11, unsigned int clr12)
  { reconstruct(12, clr1, clr2, clr3, clr4, clr5, clr6, clr7, clr8, clr9, clr10, clr11, clr12); }
};
*/

#endif // IPALETTE_H
