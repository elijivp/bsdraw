#ifndef IPALETTE_H
#define IPALETTE_H

#define BSCGET_RED(uint)  ((uint &0xFF)/255.0f)
#define BSCGET_GREEN(uint)  ((uint >> 8 &0xFF)/255.0f)
#define BSCGET_BLUE(uint)  ((uint >> 16 &0xFF)/255.0f)

class IPalette
{
public:
  virtual void          getPalette(const void** clrarr, unsigned int* count, unsigned int* format) const =0;
  virtual unsigned int  first() const=0;
  virtual unsigned int  last() const=0;
  virtual ~IPalette(){}
public:
  enum  { FMT_UNKNOWN,
          FMT_UNSIGNED_BYTE, // GL_UNSIGNED_BYTE
          FMT_FLOAT
        };
};

class PalettePTR: public IPalette
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
//  unsigned int&   operator[](int i) {  return m_palbuf[i]; }
  unsigned int    count() const {  return m_count; }
public:
  PalettePTR(const unsigned int* buf, unsigned int count, bool owner): m_palbuf(buf), m_count(count), m_owner(owner){}
//  PalettePTR()
//  {  int minimax = m_count > otherMaxCount? otherMaxCount : m_count; for (int i=1; i<minimax; i++)  m_palbuf[i] = cpy.palbuf[i];  m_palbuf[0] = otherfirstcolor;  }
  ~PalettePTR(){  if (m_owner)  delete []m_palbuf;  }
};

#endif // IPALETTE_H
