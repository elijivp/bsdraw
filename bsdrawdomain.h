#ifndef DRAWDOMAIN_H
#define DRAWDOMAIN_H

/// King of draws!
/// DrawDomain its a draw, who allow you to mark a canvas by 'domains'
/// each will be paitned on color, corresponding with 1D-array data
/// 
/// Example:
/// DrawDomain* draw = new DrawDomain(SAMPLES, LINES, 1, false, OR_LRBT, true);
/// {
///   DIDomain& ddm = *draw->domain();
///   for (int j=0; j<SAMPLES; j++)
///   {
///     ddm.start();
///     for (int r=0; r<LINES/2; r++)
///       ddm.includePixel(int(LINES/2 + sin(j/(2.0*M_PI*8))*LINES/4 - LINES/4 + r), j);
///     ddm.finish();
///   }
/// }
/// ...
/// draw->setData(some_float_data);
/// 
/// This will paint included_pixels by colors from palette, corresponding data
/// In this example one domain is one of [SAMPLES] vertical lines
/// 
/// 
/// Created By: Elijah Vlasov

#include "core/bsqdraw.h"

class DIDomain
{
protected:
  unsigned int      m_width, m_height;
  unsigned int*     m_count;
  float*            m_dataptr;
  bool              m_incBackground;
private:
  DIDomain(){}
  void _init(unsigned int width, unsigned int height, bool incbackground, unsigned int* count, float *dataptr);
  friend class DrawDomain;
public:
  unsigned int      count() const { return *m_count; }
  unsigned int      width() const { return m_width; }
  unsigned int      height() const { return m_height; }
  float*            rawdata() const { return m_dataptr; }
  bool              isBackgroundDomain() const { return m_incBackground; }
public:
  void  start();
  void  finish();
  
  void  includeRow(int row);
  void  includeColumn(int column);
  void  includeRect(int left, int top, int width, int height);
  void  includePixel(int r, int c);
  
  void  includeRowFree(int row);
  void  includeColumnFree(int column);
  void  includeRectFree(int left, int top, int width, int height);
  void  includePixelFree(int r, int c);
  
  bool  isFree(int r, int c) const;
  
  void  excludeRow(int row);
  void  excludeColumn(int column);
  void  excludeRect(int left, int top, int width, int height);
  void  excludePixel(int r, int c);
};


class DrawDomain: public DrawQWidget
{
public:
protected:
  DIDomain m_domain;
public:
  DrawDomain(unsigned int samplesHorz, unsigned int samplesVert, unsigned int portions=1, bool isBckgrndDomain=true, ORIENTATION orient=OR_LRBT, bool holdmemorytilltheend=false);
  DrawDomain(const DIDomain& cpy, unsigned int portions=1, ORIENTATION orient=OR_LRBT, bool holdmemorytilltheend=false);
  ~DrawDomain();
public:
  DIDomain*         domain();         /// NULL if data was cleared (holdmemtilltheend was turned on in constructor and first draw invoked PC_GROUND)
  const DIDomain*   domain() const;   /// --//--
  unsigned int      domainsCount() const;
protected:
  virtual void            sizeAndScaleHint(int sizeA, int sizeB, unsigned int* matrixDimmA, unsigned int* matrixDimmB, unsigned int* scalingA, unsigned int* scalingB) const;
};

#endif // DRAWDOMAIN_H
