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

#endif // IPALETTE_H
