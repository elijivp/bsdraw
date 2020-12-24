/// This file contains special class who describes Bars around bsdraws
/// Each Bar, or margin, allow you to locate scales, labels, lines, etc.. near bsdraw
/// Each Bar is a mini layout. He eats in one turn two special interfaces:
///   - MarginElement for place and draw
///   - MEQWrapper for control
/// Background and foreground ColorPolicy can be bsdraw-depended. Or You can make is standard by QPalette
/// Created By: Elijah Vlasov
#include "bsdrawscales.h"

#include "core/bsqdraw.h"
#include "palettes/bsipalette.h"

#include <QPainter>
#include <QResizeEvent>
#include <QStaticText>
#include <QVBoxLayout>
#include <QScrollBar>

//#include <QApplication>

MarginElement::~MarginElement()
{
}

struct        texts_t
{
  QStaticText uin_text;
  int         uarea_atto;
  QSize       uin_locsize;
  QPoint      uarea_pos;
  int         visible;
  texts_t(): uarea_atto(-1), uarea_pos(0,0), visible(0) {}
};

inline void  assignText(texts_t* tt, const QString& text)
{
  tt->uin_text.setText(text);
  QSize loc = tt->uin_text.size().toSize();
  if (tt->uarea_atto != -1)
  {
    int delta = tt->uin_locsize.width() - loc.width();
    if (tt->uarea_atto != AT_RIGHT)
      tt->uarea_pos.rx() += delta;
  }
  tt->uin_locsize = loc;
}

inline void  assignText(texts_t* tt, const QString& text, const QFont& font)
{
  assignText(tt, text);
  tt->uin_text.prepare(QTransform(), font);
}

inline void  assignText(texts_t* tt, const QString& text, bool fontReplaced, const QFont& font)
{
  assignText(tt, text);
  if (fontReplaced)
    tt->uin_text.prepare(QTransform(), font);
}




static const bool   BAR_VERT[] = { true, true, false, false };
static const bool   BAR_NEEDMOVE[] = { true, false, true, false };


typedef void  (*uptp_fn)(texts_t* t, int decay);

template <int n>
void  uptp_mid(texts_t* t, int decay);

template <>   void  uptp_mid<AT_LEFT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(decay + t->uin_locsize.width(), t->uin_locsize.height()/2 + 1);  }
template <>   void  uptp_mid<AT_RIGHT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(-decay, t->uin_locsize.height()/2 + 1);  }
template <>   void  uptp_mid<AT_TOP>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width()/2, decay + t->uin_locsize.height());  }
template <>   void  uptp_mid<AT_BOTTOM>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width()/2, -decay);  }

template <int n>
void  uptp_up(texts_t* t, int decay);

template <>   void  uptp_up<AT_LEFT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(decay + t->uin_locsize.width(), t->uin_locsize.height() + 1);  }
template <>   void  uptp_up<AT_RIGHT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(-decay, t->uin_locsize.height() + 1);  }
template <>   void  uptp_up<AT_TOP>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width()/2*0, decay + t->uin_locsize.height());  }
template <>   void  uptp_up<AT_BOTTOM>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width()/2*0, -decay);  }

template <int n>
void  uptp_down(texts_t* t, int decay);

template <>   void  uptp_down<AT_LEFT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(decay + t->uin_locsize.width(), 0*t->uin_locsize.height()/2 + 1);  }
template <>   void  uptp_down<AT_RIGHT>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(-decay, t->uin_locsize.height()/2*0 + 1);  }
template <>   void  uptp_down<AT_TOP>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width(), decay + t->uin_locsize.height());  }
template <>   void  uptp_down<AT_BOTTOM>(texts_t* t, int decay)
{ t->uarea_pos -= QPoint(t->uin_locsize.width(), -decay);  }

static uptp_fn  UPTP_MID[4]   = { uptp_mid<AT_LEFT>, uptp_mid<AT_RIGHT>, uptp_mid<AT_TOP>, uptp_mid<AT_BOTTOM> };
static uptp_fn  UPTP_UP[4]    = { uptp_up<AT_LEFT>, uptp_up<AT_RIGHT>, uptp_up<AT_TOP>, uptp_up<AT_BOTTOM> };
static uptp_fn  UPTP_DOWN[4]  = { uptp_down<AT_LEFT>, uptp_down<AT_RIGHT>, uptp_down<AT_TOP>, uptp_down<AT_BOTTOM> };


class DSNumFormatter
{
  int     precision_ge1, precision_l1;
  char    letter_ge1, letter_l1;
  bool    nonamed;
  
  bool    useThreshold;
  double  threshold_ge;
  int     precision_gethr;
  char    letter_gethr;
  
  enum RFM {  RFM_DEFAULT, RFM_DEFAULT_ROUNDED, RFM_INTEGER, RFM_INTEGER_ROUNDED  };
  RFM     rfm;
public:
  DSNumFormatter(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, bool meganamed=false)
  {
    setFormat(symb_ge1, precis_ge1, symb_l1, precis_l1, meganamed);
  }
  DSNumFormatter()
  {
    setFormat('f', 1, 'g', 3, 100000.0, 'g', 2);
  }
  void  setFormat(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, bool meganamed=false)
  {
    precision_ge1 = precis_ge1;
    precision_l1 = precis_l1;
    letter_ge1 = symb_ge1;
    letter_l1 = symb_l1;
    nonamed = !meganamed;
    useThreshold = false;
    rfm = RFM_DEFAULT;
  }
  void  setFormat(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, double thresh_ge, char symb_gethr, int precis_gethr, bool meganamed=false)
  {
    setFormat(symb_ge1, precis_ge1, symb_l1, precis_l1, meganamed);
    useThreshold = true;
    threshold_ge = thresh_ge;
    letter_gethr = symb_gethr;
    precision_gethr = precis_gethr;
    rfm = RFM_DEFAULT;
  }
  void  autoFormat(){     setFormat('f', 1, 'g', 3, 100000.0, 'g', 2);  }
  void  autoFormat(float ll, float hl)
  {
    float dhl = qAbs(hl - ll);
    if (dhl > 1000.0f)
      setFormat('f', 1, 'g', 2, 100000.0, 'g', 2);
    else if (dhl > 10.0f || ll >= 1.0f)
      setFormat('f', 1, 'f', 2, 100000.0, 'g', 2);
    else if (dhl > 1.0f)
      setFormat('f', 2, 'f', 3, 100000.0, 'g', 2);
    else
      setFormat('f', 3, 'f', 3, 100000.0, 'g', 2);
  }
  void  integerFormat(bool round){ rfm = round? RFM_INTEGER_ROUNDED : RFM_INTEGER; }
  
  template <typename T>
  QString operator()(T value_original)
  {
    if (rfm >= RFM_INTEGER)
    {
      long int value = rfm == RFM_INTEGER_ROUNDED? (long int)qRound64(value_original) : (long int)(value_original);
      if (nonamed)
        return QString::number(value);
      long int valuep = value < 0? -value : value;
      static const char letters[] = { ' ', ' ',  'K',  'M',  'G',  'T',   'P',   'E',   'Z',   'Y',   ' '  };
      long int mlt = 1;
      for (int i=0; i<sizeof(letters)/sizeof(const char); i++)
      {
        if (valuep > mlt)
        {
          mlt *= 1000;
          continue;
        }
        return QString::number(value/mlt + letters[i]);
      }
      return QString::number(value);
    }
    else
    {
      double value = rfm == RFM_DEFAULT_ROUNDED? double(qRound64(value_original)) : double(value_original);
      double valuep = value < 0? -value : value;
      char fmt_letter = valuep >= 1.0? useThreshold && valuep >= threshold_ge? letter_gethr : letter_ge1 : letter_l1;
      int  fmt_precision = valuep >= 1.0? useThreshold && valuep >= threshold_ge? precision_gethr : precision_ge1 : precision_l1;
      if (nonamed)
        return QString::number(value, fmt_letter, fmt_precision);
      
      int oletter = 0;
      static const char letters[] = { ' ',  'K',  'M',  'G',  'T',   'P',   'E',   'Z',   'Y',   ' '  };
      static double numbers[] = {     1e+3, 1e+6, 1e+9, 1e+12, 1e+15, 1e+18, 1e+21, 1e+24, 1e+26 };
      const int nummax = sizeof(numbers)/sizeof(double);
      while (valuep >= numbers[oletter])
        oletter++;
      if (oletter >= nummax)
        return QString::number(value, 'g', fmt_precision);
  
      return QString::number(value/(oletter == 0? 1.0 : numbers[oletter-1]), fmt_letter, fmt_precision) + 
          letters[oletter];
    }
  }
};


static void rearrange(bool vertical, bool mirrored, texts_t* texts, int countMaxiTotal, bool backward=true)
{
  int iprev=0;
  if (vertical)
  {
    if (mirrored)
    {
      for (int i=1; i<countMaxiTotal; i++)
        if (texts[i].visible)
        {
          if (texts[iprev].uarea_pos.y() + texts[iprev].uin_locsize.height() + 1 > texts[i].uarea_pos.y())
            texts[i].visible = 2;
          else
            iprev = i;
        }
    }
    else
    {
      for (int i=1; i<countMaxiTotal; i++)
        if (texts[i].visible)
        {
//          if (texts[iprev].uarea_pos.y() + texts[iprev].uin_locsize.height() + 2 < texts[i].uarea_pos.y())  !! bad boi
          if (texts[iprev].uarea_pos.y() < texts[i].uarea_pos.y() + texts[i].uin_locsize.height() + 1)
            texts[i].visible = 2;
          else
            iprev = i;
        }
    }
  }
  else
  {
    if (mirrored)
    {
      for (int i=1; i<countMaxiTotal; i++)
        if (texts[i].visible)
        {
          if (texts[iprev].uarea_pos.x() < texts[i].uarea_pos.x() + texts[i].uin_locsize.width() + 4)
            texts[i].visible = 2;
          else
            iprev = i;
        }
    }
    else
    {
      for (int i=1; i<countMaxiTotal; i++)
        if (texts[i].visible)
        {
          if (texts[iprev].uarea_pos.x() + texts[iprev].uin_locsize.width() + 4 > texts[i].uarea_pos.x())
            texts[i].visible = 2;
          else
            iprev = i;
        }
    }
  }
  
  if (backward)
  {
    if (texts[countMaxiTotal-1].visible == 2 && iprev > 0)
    {
      texts[countMaxiTotal-1].visible = 1;
      texts[iprev].visible = 0;
      for (int i=iprev-1; i>0 && texts[i].visible != 1; i--)
        if (texts[i].visible == 2)
        {
          int d2 = i;
          for (int j=i-1; j>0; j--)
          {
            if (texts[j].visible == 1)
            {
              texts[j].visible = 2;
              break;
            }
            if (texts[j].visible == 2)
              d2 = j;
          }
          texts[d2].visible = 1;
          break;
        }
    }
  }
  for (int i=1; i<countMaxiTotal; i++)
    if (texts[i].visible == 2)
      texts[i].visible = 0;
}

static void rectAlign(const QRect& area, Qt::Alignment alignment, QPoint* result, QPointF* sign);


enum  RELATED_FLAG { 
  RF_NONE=0,
  RF_SETBOUNDS=1, RF_SETENUMERATE=2,
  RF_SETTAPS=3,
  RF_SETTAPWDG=4
};

union relatedopts_t
{
  struct tt_t
  {
    float         LL, HL;
  }               rel_fixed;
  struct
  {
    mtap_qstring_fn  tapfn;
    const void*     param;
    int              slen;
  }               rel_tap_qstring;
//  struct
//  {
//    mtap_qwidget_fn  tapfn;
//    const void*     param;
//    int              maxperpdimm;
//  }               rel_tap_qwidget;
  struct
  {
//    int       step;
    int       numcount;
    int       recycle;
    int       enfrom;
  }               rel_enumerate;
  struct
  {
    int       step;
    int       recycle;
  }               rel_symbolate;
  
  relatedopts_t(){}
  relatedopts_t(const bounds_t bnd){  rel_fixed.LL = bnd.LL; rel_fixed.HL = bnd.HL; }
  relatedopts_t(mtap_qstring_fn tapfn, const void* param, int maxlen){  rel_tap_qstring.tapfn = tapfn; rel_tap_qstring.param = param; rel_tap_qstring.slen = maxlen; /*rel_tap_qstring.sstr.reserve(maxlen); */ }
//  relatedopts_t(mtap_qwidget_fn tapfn, const void* param, int maxperpendiculardimm){  rel_tap_qwidget.tapfn = tapfn; rel_tap_qwidget.param = param; rel_tap_qwidget.maxperpdimm = maxperpendiculardimm; }
  relatedopts_t(int numcount, int enfrom, int recycle){  rel_enumerate.numcount = numcount;  rel_enumerate.enfrom = enfrom;  rel_enumerate.recycle = recycle; }
};



class MarginBoundDepended
{
protected:
  int                     rtexttype;
  int                     lmoffset, lmardimm;
  relatedopts_t           rdata;
  float                   reloffset;
public:
  virtual ~MarginBoundDepended(){}
  virtual void relatedTextup()=0;
public:
  MarginBoundDepended(): rtexttype(RF_NONE), lmoffset(0), lmardimm(1), reloffset(0){}
  void    setRelatedOpts(int texttype, const relatedopts_t& reldata, bool autocalltextup=true)
  {
    rtexttype = texttype;
    rdata = reldata;
    if (autocalltextup)
      relatedTextup();
  }
  void    setRelatedOffset(int offset, bool autocalltextup=true)
  {
    lmoffset = offset;
    reloffset = lmoffset / lmardimm;
    if (autocalltextup)
      relatedTextup();
  }
  void    storeDimm(int dimm, bool autocalltextup=false)
  {
    lmardimm = dimm;
    reloffset = lmoffset / lmardimm;
    if (autocalltextup)
      relatedTextup();
  }
};

/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/


class MarginSpace: public MarginElement
{
  int space;
public:
  MarginSpace(int space)
  {
    this->space = space;
  }
  void  setSpace(int newspace)
  {
    space = newspace;
  }
protected:
  virtual bool  updateArea(const uarea_t&, int){ return true; }
  virtual void  draw(QPainter&){}
  virtual void  sizeHint(ATTACHED_TO, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    *atto_size = space;
    *mindly = *mindly1 = *mindly2 = 0;
  }
  virtual void  changeColor(const QColor&){}
};

class MarginElementCached: public MarginElement
{
  uarea_t       c_area;
  bool          c_haveArea;
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool inited = c_haveArea;
    bool equal = area == c_area;
    c_area = area;
    if (UPDATEFOR == UF_RESIZE)
    {
//      qDebug()<<"UpAr: resize, inited = "<<inited<<equal;
      c_haveArea = true;
      return inited ? !equal : true;
    }
    else if (UPDATEFOR == UF_APPEND)
    {
//      qDebug()<<"UpAr: append, inited = "<<inited;
      return inited;
    }
    return false;
  }
  
  const uarea_t&  cached_area() const { return c_area; }
//  bool            cached_haveArea() const { return c_haveArea; }
protected:
  QColor        c_color;
  bool          c_color_redefined;
  virtual void  changeColor(const QColor& clr){  c_color = clr; c_color_redefined = true; }
public:
  MarginElementCached(): c_haveArea(false), c_color_redefined(false)
  {
  }
  ~MarginElementCached();
};

MarginElementCached::~MarginElementCached(){}


class MarginContour: public MarginElementCached
{
  int     space;
  int     count;
  QRect   dtrt[3];
  bool    useAllZone;
public:
  MarginContour(int space=0, bool maxzone=true): count(space == 0? 1 : 3), useAllZone(maxzone)
  {
    this->space = space;
  }
  void  setSpace(int newspace)
  {
    space = newspace;
    count = space? 3 : 1;
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    
    int d1 = useAllZone? 0 : area.dly1, d2 = useAllZone? area.dlytotal : area.dlytotal - area.dly2 - 1;
    int nspace = space == 0? 0 : space-1;
    if (area.atto == AT_LEFT)
    {
      dtrt[0].setCoords(area.atto_begin - nspace, d1, area.atto_begin - nspace, d2);
      dtrt[1].setCoords(area.atto_begin - nspace, d1, area.atto_begin, d1);
      dtrt[2].setCoords(area.atto_begin - nspace, d2, area.atto_begin, d2);
    }
    else if (area.atto == AT_RIGHT)
    {
      dtrt[0].setCoords(area.atto_begin + nspace, d1, area.atto_begin + nspace, d2);
      dtrt[1].setCoords(area.atto_begin, d1, area.atto_begin + nspace, d1);
      dtrt[2].setCoords(area.atto_begin, d2, area.atto_begin + nspace, d2);
    }
    else if (area.atto == AT_TOP)
    {
      dtrt[0].setCoords(d1, area.atto_begin - nspace, d2, area.atto_begin - nspace);
      dtrt[1].setCoords(d1, area.atto_begin - nspace, d1, area.atto_begin);
      dtrt[2].setCoords(d2, area.atto_begin - nspace, d2, area.atto_begin);
    }
    else if (area.atto == AT_BOTTOM)
    {
      dtrt[0].setCoords(d1, area.atto_begin + nspace, d2, area.atto_begin + nspace);
      dtrt[1].setCoords(d1, area.atto_begin, d1, area.atto_begin + nspace);
      dtrt[2].setCoords(d2, area.atto_begin, d2, area.atto_begin + nspace);
    }
    return true;
  }
  virtual void  draw(QPainter& painter)
  { 
    if (c_color_redefined)
      for (int i=0; i<count; i++)
        painter.fillRect(dtrt[i], c_color);
    else
      for (int i=0; i<count; i++)
        painter.fillRect(dtrt[i], painter.brush());
  }
  virtual void  sizeHint(ATTACHED_TO, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    *atto_size = space == 0? 1 : space;
    *mindly = *mindly1 = *mindly2 = 0;
  }
};

  
class MarginLabel: public MarginElementCached
{
  texts_t         label;
  Qt::Alignment   alignment;
  Qt::Orientation orientation;
  QSize           orient_size;
  
  QTransform      ttransform;
  QPoint          ttranspoint;
  
  bool            useneispace;
  
  QFont           font;
  bool            fontReplaced;
//protected:
//  int             c_width, c_height;
private:
  void  prepare()
  {
    QSizeF loc = label.uin_text.size();
    label.uin_locsize = QSize(qRound(loc.width()), qRound(loc.height()));
    if (orientation == Qt::Horizontal)
    {
      if (fontReplaced)
        label.uin_text.prepare(QTransform(), font);
      orient_size = label.uin_locsize;
    }
    else if (orientation == Qt::Vertical)
    {
      ttransform.rotate(-90);
      label.uin_text.prepare(ttransform, font);
      orient_size = QSize(label.uin_locsize.height(), label.uin_locsize.width());
    }
//    qDebug()<<label.uin_locsize<<orient_size;
  }
public:
  MarginLabel(const QString& text, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal): 
    alignment(align), orientation(orient), useneispace(useNeighborSpace), fontReplaced(false)
  {
    setText(text);
  }
  MarginLabel(const QString& text, const QFont& fnt, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal): 
    alignment(align), orientation(orient), useneispace(useNeighborSpace)
  {
    setText(text, fnt);
  }
  void  setText(const QString& text)
  {
    label.uin_text.setText(text);
    prepare();
  }
  void  setText(const QString& text, const QFont& fnt)
  {
    label.uin_text.setText(text);
    font = fnt;
    fontReplaced = true;
    prepare();
  }
  void  setFont(const QFont& fnt)
  {
    font = fnt;
    fontReplaced = true;
    prepare();
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    int a1 =  area.atto_end;
    int a2 = area.atto_begin;
    int d = useneispace? area.dlytotal : area.dlytotal - area.dly1 - area.dly2  - area.dly3 - 1;
    int dd = useneispace? 0 : area.dly1 + 1;
    
    const QRect& actualArea = area.atto == AT_LEFT?  QRect(a1, dd, a2 - a1, d) :
                              area.atto == AT_RIGHT? QRect(a2, dd, a1 - a2, d) :
                              area.atto == AT_TOP?   QRect(dd, a1, d, a2 - a1) :
                                                QRect(dd, a2, d, a1 - a2);
    
    QPointF textOffsetSign;
    rectAlign(actualArea, alignment, &ttranspoint, &textOffsetSign);
    if (orientation == Qt::Vertical)
      textOffsetSign = QPointF(-(1.0 + textOffsetSign.y()), textOffsetSign.x());
    
    label.uarea_pos = QPoint(int(textOffsetSign.x()*label.uin_locsize.width()), int(textOffsetSign.y()*label.uin_locsize.height()));
    label.visible = 1;
    return true;
  }
  virtual void  draw(QPainter& painter)
  {
    if (label.visible)
    {
      painter.save();
      painter.translate(ttranspoint);
      if (fontReplaced)
        painter.setFont(font);
      if (ttransform.type() != QTransform::TxNone)
        painter.setTransform(ttransform, true);
      painter.drawStaticText(label.uarea_pos, label.uin_text);
      painter.restore();
    }
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    if (BAR_VERT[atto])
    {
      *atto_size = orient_size.width();
      *mindly = orient_size.height();
    }
    else
    {
      *atto_size = orient_size.height();
      *mindly = orient_size.width();
    }
    *mindly1 = *mindly2 = 0;
  }
}; 


/*
class MarginLabel: public MarginElementCached
{
  texts_t         label;
  Qt::Alignment   alignment;
  float           orientation;
  double          orient_sin, orient_cos;
  QSize           orient_size;
  
  QTransform      ttransform;
  QPoint          ttranspoint;
  
  bool            useneispace;
  
  QFont           font;
  bool            fontReplaced;
//protected:
//  int             c_width, c_height;
private:
  void  prepare()
  {
    QSizeF loc = label.uin_text.size();
    label.uin_locsize = QSize(qRound(loc.width()), qRound(loc.height()));
    if (orientation == 0.0f)
    {
      if (fontReplaced)
        label.uin_text.prepare(QTransform(), font);
      orient_size = label.uin_locsize;
    }
    else
    {
      ttransform.rotate(-orientation);
      label.uin_text.prepare(ttransform, font);
      
      double rad = double(orientation)*3.1415927/180.0;
      orient_sin = sin(rad);
      orient_cos = cos(rad);
      orient_size = QSize(qRound(orient_sin*loc.height() + orient_cos*loc.width()), 
                          qRound(orient_cos*loc.height() + orient_sin*loc.width()));
    }
  }
public:
  MarginLabel(const QString& text, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, float orientAngleGrad=0.0f): 
    alignment(align), orientation(orientAngleGrad), useneispace(useNeighborSpace), fontReplaced(false)
  {
    setText(text);
  }
  MarginLabel(const QString& text, const QFont& fnt, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, float orientAngleGrad=0.0f): 
    alignment(align), orientation(orientAngleGrad), useneispace(useNeighborSpace)
  {
    setText(text, fnt);
  }
  void  setText(const QString& text)
  {
    label.uin_text.setText(text);
    prepare();
  }
  void  setText(const QString& text, const QFont& fnt)
  {
    label.uin_text.setText(text);
    font = fnt;
    fontReplaced = true;
    prepare();
  }
  void  setFont(const QFont& fnt)
  {
    font = fnt;
    fontReplaced = true;
    prepare();
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    
    int a1 =  area.atto == AT_LEFT?   area.atto_pt_shared != -1? area.atto_pt_shared : area.atto_pt - orient_size.width() :
              area.atto == AT_RIGHT?  area.atto_pt_shared != -1? area.atto_pt_shared : area.atto_pt + orient_size.width() :
              area.atto == AT_TOP?    area.atto_pt_shared != -1? area.atto_pt_shared : area.atto_pt - orient_size.height() :
                                      area.atto_pt_shared != -1? area.atto_pt_shared : area.atto_pt + orient_size.height();
    int a2 = area.atto_pt;
    int d = useneispace? area.dlytotal : area.dlytotal - area.dly1 - area.dly2  - area.dly3 - 1;
    int dd = useneispace? 0 : area.dly1 + 1;
    
    const QRect& actualArea = area.atto == AT_LEFT?  QRect(a1, dd, a2 - a1, d) :
                              area.atto == AT_RIGHT? QRect(a2, dd, a1 - a2, d) :
                              area.atto == AT_TOP?   QRect(dd, a1, d, a2 - a1) :
                                                QRect(dd, a2, d, a1 - a2);
    
    QPointF textOffsetSign;
    rectAlign(actualArea, alignment, &ttranspoint, &textOffsetSign);
//    if (orientation != 0.0f)
//    {
      
//      textOffsetSign = QPointF(orient_sin*textOffsetSign.x() + orient_cos*textOffsetSign.y(), 
//                               -orient_cos*textOffsetSign.y() + orient_sin*textOffsetSign.x());
////      textOffsetSign = QPointF(-(1.0 + textOffsetSign.y()), textOffsetSign.x());
//      label.uarea_pos = QPoint(int(textOffsetSign.x()*locsize.height()), int(textOffsetSign.y()*locsize.width()));
//    }
//    else
//      label.uarea_pos = QPoint(int(textOffsetSign.x()*locsize.width()), int(textOffsetSign.y()*locsize.height()));
    QSize locsize = label.uin_locsize;
    if (orientation != 0.0f)
    {
//      textOffsetSign = QPointF(-(1.0 + textOffsetSign.y()), textOffsetSign.x());
      
//      textOffsetSign = QPointF(-orient_sin - orient_sin*textOffsetSign.y(),
//                               -orient_cos + orient_sin*textOffsetSign.x());
      
//      qDebug()<<orient_cos<<orient_sin;
//      textOffsetSign = QPointF(orient_cos*textOffsetSign.x() - orient_sin - orient_sin*textOffsetSign.y(),
//                               orient_sin*textOffsetSign.x() - orient_cos + orient_cos*textOffsetSign.y());
      
//      qDebug()<<"TOS: "<<textOffsetSign;
//      textOffsetSign = QPointF(textOffsetSign.x() +1, (1.0 + textOffsetSign.y()));
//      label.uarea_pos = QPoint(int(textOffsetSign.x()*locsize.height()), int(textOffsetSign.y()*locsize.width()));
      
//      label.uarea_pos = QPoint((orient_cos*textOffsetSign.x() + orient_sin*textOffsetSign.y())*locsize.width() + (orient_sin*textOffsetSign.y() + orient_cos*textOffsetSign.x())*locsize.height() , 
//                               orient_sin*textOffsetSign.y()*locsize.width() + orient_cos*textOffsetSign.y()*locsize.height() );
      
      double rad = double(orientation)*3.1415927/180.0;
      
      label.uarea_pos = QPoint(int(textOffsetSign.x()*locsize.width()), int(textOffsetSign.y()*locsize.height()));
      double hyp = 2.0*(sin(rad/2.0)*sqrt(locsize.width()*locsize.width()/4.0 + locsize.height()*locsize.height()/4.0));
      double an1 = M_PI/2.0 - (M_PI - rad)/2.0;
      label.uarea_pos = QPoint(label.uarea_pos.x() + sin(an1)*hyp, label.uarea_pos.y() + cos(an1)*hyp);
    }
    else
      label.uarea_pos = QPoint(int(textOffsetSign.x()*locsize.width()), int(textOffsetSign.y()*locsize.height()));
    
//    locsize.transpose();
    
    qDebug()<<"rc:"<<label.uarea_pos;
    label.visible = 1;
    return true;
  }
  virtual bool  draw(QPainter& painter)
  {
    if (label.visible)
    {
      painter.save();
      painter.translate(ttranspoint);
      if (fontReplaced)
        painter.setFont(font);
      if (ttransform.type() != QTransform::TxNone)
        painter.setTransform(ttransform, true);
      painter.drawStaticText(label.uarea_pos, label.uin_text);
      painter.restore();
    }
    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
//    int size_horz = label.uin_locsize.width();
//    int size_vert = label.uin_locsize.height();
//    if (orientation == Qt::Vertical)
//      qSwap(size_horz, size_vert);
    if (BAR_VERT[atto])
    {
      *atto_size = orient_size.width();
      *mindly = orient_size.height();
    }
    else
    {
      *atto_size = orient_size.height();
      *mindly = orient_size.width();
    }
//    qDebug()<<*atto_size<<*mindly;
    *mindly1 = *mindly2 = 0;
  }
};
*/


class MarginPointer: public MarginElementCached, public MarginBoundDepended
{
  texts_t         pointer;
  Qt::Alignment   alignment;
  float           position;
//  Qt::Orientation orientation;
//  QSize           orient_size;
//  QTransform      ttransform;
//  QPoint          ttranspoint;
  QFont           font;
  bool            fontReplaced;
  
  bool            textInnerPlaced;
  
  DSNumFormatter  numfmt;
  float           LL, HL;
public:
  MarginPointer(Qt::Alignment  align=Qt::AlignCenter/*, Qt::Orientation orient=Qt::Horizontal*/, bool innerTextPlacement=true): 
    alignment(align), position(-1.0f), /*orientation(orient), */fontReplaced(false), 
    textInnerPlaced(innerTextPlacement), LL(0.0f), HL(1.0f)
  {
//    assignText(&pointer, "000.00");
  }
  MarginPointer(const QFont& fnt, Qt::Alignment  align=Qt::AlignCenter/*, Qt::Orientation orient=Qt::Horizontal*/, bool innerTextPlacement=true): 
    alignment(align), position(-1.0f), /*orientation(orient), */
    font(fnt), fontReplaced(true), textInnerPlaced(innerTextPlacement), LL(0.0f), HL(1.0f)
  {
//    assignText(&pointer, "000.00", fnt);
  }
  void  setPosition(float pos01)
  {
    position = pos01;
    assignText(&pointer, numfmt(LL + position*(HL-LL)));
    update_position(cached_area());
    pointer.visible = position >= 0.0f? 1 : 0;
  }
  void  setPosition(float pos01_x, float pos01_y)
  {
    setPosition(BAR_VERT[cached_area().atto]? pos01_y : pos01_x);
  }
  void  setVisible(bool visible)
  {
    if (visible)
    {
      if (position < 0) position = 0;
      pointer.visible = 1;
    }
    else
      pointer.visible = 0;
  }
  bool  isVisible() const
  {
    return pointer.visible != 0;
  }
  void  setFont(const QFont& fnt)
  {
    font = fnt;
    fontReplaced = true;
    assignText(&pointer, numfmt(LL + position*(HL-LL)), fnt);
  }
protected:
  void update_position(const uarea_t& area)
  {
    const int& a1 = area.atto_end, &a2 = area.atto_begin;
    int d = area.dlytotal - area.dly1 - area.dly2  - area.dly3 - 1;
    int dd = area.dly1 + 1;
//    pointer.uarea_pos = area.atto == AT_LEFT?   QPoint(a1-2, dd + d - d*position) :
//                        area.atto == AT_RIGHT?  QPoint(a2+2, dd + d - d*position) :
//                        area.atto == AT_TOP?    QPoint(dd + d*position, a1):
//                                                QPoint(dd + d*position, a2);
    pointer.uarea_pos = area.atto == AT_LEFT?   QPoint(a1, dd + d - d*position) :
                        area.atto == AT_RIGHT?  QPoint(a2+2, dd + d - d*position) :
                        area.atto == AT_TOP?    QPoint(dd + d*position, a1):
                                                QPoint(dd + d*position, a2);
    pointer.uarea_pos -= BAR_VERT[area.atto]? QPoint(0, pointer.uin_locsize.height()/2) : QPoint(pointer.uin_locsize.width()/2, 0);
    if (textInnerPlaced)
    {
//      void  (*luptp)(texts_t*, int) = area.mirrored? UPTP_DOWN[area.atto] : UPTP_UP[area.atto];
//      void  (*luptp)(texts_t*, int) = area.mirrored? UPTP_UP[area.atto] : UPTP_DOWN[area.atto];
//      void  (*luptp)(texts_t*, int) = UPTP_MID[area.atto];
//      luptp(&pointer, 0);
    }
    storeDimm(d);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    
    update_position(area);
//    if (pointer.visible == 0)
//    pointer.visible = position >= 0.0f? 1 : 0;
//    QPointF textOffsetSign;
//    rectAlign(actualArea, alignment, &ttranspoint, &textOffsetSign);
////    if (orientation == Qt::Vertical)
////      textOffsetSign = QPointF(-(1.0 + textOffsetSign.y()), textOffsetSign.x());
    
////    label.uarea_pos = QPoint(int(textOffsetSign.x()*label.uin_locsize.width()), int(textOffsetSign.y()*label.uin_locsize.height()));
////    pointer.uarea_pos = QPoint(pointer.uin_locsize.width(), pointer.uin_locsize.height());
//    pointer.uarea_pos = QPoint(int(textOffsetSign.x()*pointer.uin_locsize.width()), int(textOffsetSign.y()*pointer.uin_locsize.height()));
//    pointer.visible = 1;
    return true;
  }
  virtual void  draw(QPainter& painter)
  {
    if (pointer.visible)
    {
      if (fontReplaced)
      {
        painter.save();
        painter.setFont(font);
        painter.drawStaticText(pointer.uarea_pos, pointer.uin_text);
        painter.restore();
      }
      else
        painter.drawStaticText(pointer.uarea_pos, pointer.uin_text);
      
//      painter.save();
//      painter.translate(ttranspoint);
//      if (fontReplaced)
//        painter.setFont(font);
////      if (ttransform.type() != QTransform::TxNone)
////        painter.setTransform(ttransform, true);
//      painter.drawStaticText(pointer.uarea_pos, pointer.uin_text);
//      painter.restore();
    }
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    QFontMetrics fm(font);
    QSize meansize(5 + fm.averageCharWidth()*6, fm.height());
    *atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    *mindly = BAR_VERT[atto]? meansize.height() : meansize.width();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *mindly1 = d_half_dly;
    *mindly2 = d_half_dly;
  }
  
  
  virtual void  relatedTextup()
  {
    if (rtexttype == RF_SETBOUNDS)
    {
      LL = rdata.rel_fixed.LL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      HL = rdata.rel_fixed.HL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      numfmt.autoFormat(LL, HL);
      assignText(&pointer, numfmt(LL + position*(HL-LL)));
    }
    else if (rtexttype == RF_SETTAPS)
    {
      QString sstr(rdata.rel_tap_qstring.slen, Qt::Uninitialized);
      float LLof = LL + (HL-LL)*reloffset;
      float HLof = HL + (HL-LL)*reloffset;
      rdata.rel_tap_qstring.tapfn(LLof + position*(HLof-LLof), lmardimm, lmoffset, rdata.rel_tap_qstring.param, sstr);
      assignText(&pointer, sstr);
    }
    else if (rtexttype == RF_SETENUMERATE)
    {
      LL = rdata.rel_enumerate.enfrom;
      HL = rdata.rel_enumerate.enfrom + rdata.rel_enumerate.numcount;
      numfmt.integerFormat(false);
      assignText(&pointer, numfmt(LL + position*(HL-LL)));  // ntf: reloffset
    }
  }
}; 


/************************************************************************************************************/

class MarginMarks: public MarginElementCached
{
public:
protected:
  int           algoType;
  struct        mark_t
  {
    QPoint      anchor;
    QRect       rect;
  }*            ua_marks;
  int*          ua_marklinks2;
  int           countMaxiNoted, countMaxiHided, countMini;
  int           pixStep_pixSpace;
  int           miniPerMaxi;
//  bool          miniPerMaxiFixed;
protected:
  int           mlenmaxi, mlenmini, mwid;
protected:
  int           c_dimm_main;
public:
  MarginMarks():
    algoType(DBMODE_STRETCHED_POW2),
    ua_marks(nullptr), ua_marklinks2(nullptr), countMaxiNoted(0), countMaxiHided(0), countMini(0), pixStep_pixSpace(0), 
    miniPerMaxi(0), mlenmaxi(4), mlenmini(2), mwid(1), c_dimm_main(0)
  {
  }
  ~MarginMarks()
  {
    if (ua_marks && ua_marklinks2)
    {
      delete []ua_marks;
      delete []ua_marklinks2;
    }
  }
  
  void  init(int algotype, int marksCount, int pixStep_pixSpacing, int miniPerMaxiLIMIT) /// marksCount = 0 for init later
  {
    algoType = algotype == DBMODE_DEFAULT? DBMODE_STRETCHED_POW2 : algotype;
    pixStep_pixSpace = pixStep_pixSpacing;
    miniPerMaxi = miniPerMaxiLIMIT;
    setMarksCount(marksCount);
  }
  
  
  virtual int setMarksCount(int marksCount)
  {
    if (marksCount)
    {
      if (ua_marks) delete []ua_marks;
      if (ua_marklinks2)  delete []ua_marklinks2;
      
      if (algoType == DBMODE_STRETCHED_POW2)
      {
        int mc = 2;
        while (2*mc + 1 <= marksCount)
          mc *= 2;
        marksCount = mc + 1;
      }
      countMaxiNoted = marksCount;
      countMaxiHided = 0;
      countMini = 0;
      
      ua_marks = new mark_t[marksCount + miniPerMaxi*countMaxiNoted];
      ua_marklinks2 = new int[marksCount];
      for (int i=0; i<marksCount; i++)
        ua_marklinks2[i] = i;
    }
    return marksCount;
  }
  void    setMarkLength(int length){ mlenmaxi = length; }
  void    setMarkMiniLength(int length){ mlenmini = length; }
  void    setMarkWidth(int width){ mwid = width; }
  void    setMarkSize(int length, int width, int lengthmini=4)
  { mlenmaxi = length; mlenmini = lengthmini; mwid = width; }
  
protected:
  
  int   calcCountStepAndJump(int totalCount, int pixLen, const int pixSpacing, int *step_out, int* jumpfrom_out, int* jumpto_out) const
  {
    float pixStep = pixLen/float(totalCount - 1);
    int result = 0, step=1, jumpfrom=0, jumpto=0;
//    qDebug()<<pixLen<<pixStep<<pixSpacing;
    if (pixLen <= pixSpacing)
    {
      result = 2;
      step = totalCount-1;
//      pref = "<pix";
    }
    else if (pixStep >= pixSpacing)
    {
      result = totalCount;
//      pref = QString::number(pixSpacing) + "> = standard";
    }
    else
    {    
      step = 2;
      while (pixStep*step < pixSpacing)
      {
        step++;
//        qDebug()<<step<<step*pixStep<<pixLen;
      }
//      qDebug()<<"step calc: "<<pixStep<<pixSpacing<<"... "<<step;
      if ((totalCount-1) % step == 0)
      {
//        pref = "odd";
        result = totalCount/step + 1;
      }
      else if ((totalCount-1)/3 <= step)
      {
        step = (totalCount-1)/2;
        jumpfrom = step;
        jumpto = totalCount - 1 - step;
//        pref = "<<<";
        result = 3;
      }
      else
      {
        const int right = int((totalCount-1)/step)*step;
        jumpfrom = right;
        jumpto = (totalCount-1);
        result = (totalCount)/step + (totalCount % step == 0? 0 : 1);
//        pref = QString("MaxiMUL");
      }
    }
//    qDebug()<<pref<<": "<<result<<"___"<<step<<"JUMP: "<<jumpfrom<<"->"<<jumpto;
    *step_out = step;     *jumpfrom_out = jumpfrom; *jumpto_out = jumpto;
    return result;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
//    for (int i=0; i<countMaxiNoted + (miniPerMaxi > 0? countMini : 0); i++)
//      painter.fillRect(ua_marks[i].rect, c_color_redefined? c_color : foregroundColor);
    if (c_color_redefined)
      for (int i=0; i<countMaxiNoted + (miniPerMaxi > 0? countMini : 0); i++)
        painter.fillRect(ua_marks[i].rect, c_color);
    else
      for (int i=0; i<countMaxiNoted + (miniPerMaxi > 0? countMini : 0); i++)
        painter.fillRect(ua_marks[i].rect, painter.brush());
  }
  
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool continueUpdate = MarginElementCached::updateArea(area, UPDATEFOR);
//    qDebug()<<"+"<<UPDATEFOR;
    if (continueUpdate == false && UPDATEFOR < UF_LVL1)
      return false;
    
//    const int dimm_main = area.dlytotal - area.dly1 - area.dly2 - area.dly3 - 1 - 1;
    const int dimm_main = area.dlytotal - area.dly1 - area.dly2 - area.dly3 - 1;
    if (c_dimm_main == dimm_main && BAR_NEEDMOVE[area.atto] && UPDATEFOR <= UF_LVL1)
      return false;
    c_dimm_main = dimm_main;
    
    const int countMaxiTotal = countMaxiNoted + countMaxiHided;
    int   over_recycle = 1;
    float over_deltapix[] = { 1, 1 };
    int   over_step[] =  { 1, 1 };  // each x element still alive
    int   jumpfrom(0), jumpto(0);  // center elements highlight
    int   over_count[] = { 0, 0 };
    int   minimod = 1;
    
    if (algoType == DBMODE_STATIC)
    {
      over_deltapix[0] = float(pixStep_pixSpace);
      countMaxiNoted = dimm_main / pixStep_pixSpace + 1;
      if (countMaxiNoted > countMaxiTotal)
        countMaxiNoted = countMaxiTotal;
      countMini = 0;
      int miniover=0;
      if (miniPerMaxi)
      {
        over_recycle += 1;
        minimod = miniPerMaxi + 1;
        miniover = int(((dimm_main % pixStep_pixSpace) / float(pixStep_pixSpace)) * (miniPerMaxi + 1));
        if (miniover < 0)
          miniover = 0;
        countMini = miniPerMaxi*(countMaxiNoted-1) + miniover;
        over_deltapix[1] = over_deltapix[0] / minimod;
      }    
      over_count[0] = countMaxiNoted;
      over_count[1] = minimod*(countMaxiNoted-1) + miniover + 1;
    }
    else
    {
      over_deltapix[0] = dimm_main/float(countMaxiTotal - 1);
      if (algoType == DBMODE_STRETCHED_POW2)
      {
        countMaxiNoted = countMaxiTotal;
        if (over_deltapix[0] < pixStep_pixSpace && countMaxiNoted > 2)
        {
          float odp = dimm_main/float(countMaxiNoted - 1);
          while (odp < pixStep_pixSpace)
          {
            countMaxiNoted -= countMaxiNoted/2;
            odp = dimm_main/float(countMaxiNoted - 1);
            over_step[0] *= 2;
    //        qDebug()<<countMaxiNoted<<over_deltapix[0]<<pixStep_pixSpace;
            if (countMaxiNoted < 3)
              break;
          }
        }
      }
      else
      {
        countMaxiNoted = calcCountStepAndJump(countMaxiTotal, dimm_main, pixStep_pixSpace, &over_step[0], &jumpfrom, &jumpto);
      }
      countMini = 0;
      if (miniPerMaxi && countMaxiNoted == countMaxiTotal)
      {
        if (over_deltapix[0]/(miniPerMaxi+1) > 4)
        {
          over_recycle += 1;
          minimod = miniPerMaxi + 1;
          countMini = miniPerMaxi*(countMaxiNoted-1);
          over_step[1] = 1;
          over_deltapix[1] = over_deltapix[0] / minimod;
        }
      }
      over_count[0] = countMaxiTotal;
      over_count[1] = minimod*(countMaxiNoted-1);
    }
    
    countMaxiHided = countMaxiTotal - countMaxiNoted;
    
    int   over_mlen[] = { mlenmaxi, mlenmini };
    int m=0;
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
      ua_marklinks2[i] = -1;
    
    for (int o=0; o<2; o++)
    {
      if (BAR_VERT[area.atto])
      {
        int d2 = area.mirrored? area.dly1 : area.dlytotal - 1 - area.dly2 - area.dly3;
        float d3 = area.mirrored? -over_deltapix[o] : over_deltapix[o];
        int l1 = area.atto_begin;
        int l2 = area.atto_begin + (area.atto == AT_LEFT? -(over_mlen[o]-1) : (over_mlen[o]-1));
        for (int i=0; i<over_count[o]; i += over_step[o])
        {
          if (o == 1 && i % minimod == 0)  continue;
          if (o == 0) ua_marklinks2[i] = m;
          int offs = d2 - qRound(i*d3);
          ua_marks[m].anchor = QPoint(l1, offs);
          if (area.atto == AT_LEFT)
            ua_marks[m].rect.setCoords(l2, offs, l1, offs);
          else
            ua_marks[m].rect.setCoords(l1, offs, l2, offs);
          m++;
          if (jumpfrom && i + over_step[0] == jumpfrom){ i = jumpto - over_step[0]; jumpfrom = 0; }
        }
      }
      else
      {
        int d2 = area.mirrored? area.dlytotal - 1 - area.dly2 - area.dly3 : area.dly1;
        float d3 = area.mirrored? -over_deltapix[o] : over_deltapix[o];
        int l1 = area.atto_begin;
        int l2 = area.atto_begin + (area.atto == AT_TOP? -(over_mlen[o]-1) : (over_mlen[o]-1));
        for (int i=0; i<over_count[o]; i += over_step[o])
        {
          if (o == 1 && i % minimod == 0)  continue;
          if (o == 0) ua_marklinks2[i] = m;
          int offs = d2 + qRound(i*d3);
          ua_marks[m].anchor = QPoint(offs, l1);
          if (area.atto == AT_TOP)
            ua_marks[m].rect.setCoords(offs, l2, offs, l1);
          else
            ua_marks[m].rect.setCoords(offs, l1, offs, l2);
          m++;
          if (jumpfrom && i + over_step[0] == jumpfrom){ i = jumpto - over_step[0]; jumpfrom = 0; }
        }
      }
    } // for over
    
    return true;
  }
  virtual void sizeHint(ATTACHED_TO /*atto*/, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    *atto_size = mlenmaxi + 2;
    *mindly = 2;
    *mindly1 = *mindly2 = 0;
  }
};


/**************************/


class MarginMarksTexted: public MarginMarks, public MarginBoundDepended
{
protected:
  texts_t*                texts;
  QFont                   font;
  DSNumFormatter          numfmt;
  bool                    fontReplaced;
public:
  MarginMarksTexted(): texts(nullptr), fontReplaced(false){}
  virtual ~MarginMarksTexted(){  if (texts)  delete []texts; }
  
  
  int rmaxlen() const {  return rtexttype == RF_SETBOUNDS? 7 : rtexttype == RF_SETENUMERATE? rdata.rel_enumerate.numcount :
                                rtexttype == RF_SETTAPS? rdata.rel_tap_qstring.slen : 0; }
  
  void  setFont(const QFont& fnt)
  {
    if (font != fnt)
      font = fnt;
    fontReplaced = true;
//    c_re_updateArea(UF_LVL2);
  }
public:
};

class Margin1Mark1Text: public MarginMarksTexted
{
  bool  textInnerPlaced;
public:
  ~Margin1Mark1Text();
  Margin1Mark1Text(bool innerTextPlacement): textInnerPlaced(innerTextPlacement){}
  virtual int setMarksCount(int marksCount)
  {
    marksCount = MarginMarks::setMarksCount(marksCount);
    texts = new texts_t[marksCount];
    return marksCount;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    MarginMarks::draw(painter);
    if (fontReplaced)
      painter.setFont(font);
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].uarea_pos, texts[i].uin_text);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool continueUpdate = MarginMarks::updateArea(area, UPDATEFOR);
    if (continueUpdate == false && UPDATEFOR < UF_LVL2)
      return false;
    
    storeDimm(c_dimm_main);
    
    const int decays[] = { 3 + mlenmaxi, 3 + mlenmaxi, 0 + mlenmaxi, 0 + mlenmaxi };
    void  (*luptp_mid)(texts_t*, int) = UPTP_MID[area.atto];
    void  (*luptp_beg)(texts_t*, int) = area.mirrored? UPTP_DOWN[area.atto] : UPTP_UP[area.atto];
    void  (*luptp_end)(texts_t*, int) = area.mirrored? UPTP_UP[area.atto] : UPTP_DOWN[area.atto];
    
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
    {
      if (ua_marklinks2[i] != -1)
      {
        texts[i].uarea_pos = ua_marks[ua_marklinks2[i]].anchor;
        texts[i].uarea_atto = area.atto;
        if (textInnerPlaced && i == 0)
          luptp_beg(&texts[i], decays[area.atto]);
        else if (textInnerPlaced && i == countMaxiNoted + countMaxiHided - 1)
          luptp_end(&texts[i], decays[area.atto]);
        else
          luptp_mid(&texts[i], decays[area.atto]);
        texts[i].visible = 1;
      }
      else
        texts[i].visible = 0;
    }
    rearrange(BAR_VERT[area.atto], area.mirrored, texts, countMaxiNoted + countMaxiHided);
    
    return true;
  }
  
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    MarginMarks::sizeHint(atto, atto_size, mindly, mindly1, mindly2);
    QFontMetrics fm(font);
    QSize meansize(5 + fm.averageCharWidth()*rmaxlen(), fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *atto_size += d_atto_size;
    *mindly1 = d_half_dly;
    *mindly2 = d_half_dly;
  }
public:
  virtual void  relatedTextup()
  {
    if (rtexttype == RF_SETBOUNDS)
    {
      float LL = rdata.rel_fixed.LL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      float HL = rdata.rel_fixed.HL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      float bndstep = qAbs(HL - LL)/(countMaxiNoted + countMaxiHided - 1);
      numfmt.autoFormat(LL, HL);
      for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
        assignText(&texts[i], numfmt(LL + i*bndstep), fontReplaced, font);
    }
    else if (rtexttype == RF_SETTAPS)
    {
      QString sstr(rdata.rel_tap_qstring.slen, Qt::Uninitialized);
      for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
      {
        rdata.rel_tap_qstring.tapfn(i, lmardimm, lmoffset, rdata.rel_tap_qstring.param, sstr);
        assignText(&texts[i], sstr, fontReplaced, font);
      }
    }
    else if (rtexttype == RF_SETENUMERATE)
    {
//      if (recycle != -1)  num = num % recycle;
      for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
      {
        int num = rdata.rel_enumerate.enfrom + i + int((countMaxiNoted + countMaxiHided)*reloffset);
        assignText(&texts[i], QString::number(num), fontReplaced, font);
      }
    }
  }
};
Margin1Mark1Text::~Margin1Mark1Text(){}

class MarginMinTexts: public MarginMarksTexted
{
  bool  textInnerPlaced;
public:
  ~MarginMinTexts();
  MarginMinTexts(bool innerTextPlacement): textInnerPlaced(innerTextPlacement){}
  virtual int setMarksCount(int marksCount)
  {
    marksCount = MarginMarks::setMarksCount(marksCount);
    texts = new texts_t[2];
    return marksCount;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    MarginMarks::draw(painter);
    if (fontReplaced)
      painter.setFont(font);
    for (int i=0; i<2; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].uarea_pos, texts[i].uin_text);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool continueUpdate = MarginMarks::updateArea(area, UPDATEFOR);
    if (continueUpdate == false && UPDATEFOR < UF_LVL2)
      return false;
    
    storeDimm(c_dimm_main);
    
    const int decays[] = { 3 + mlenmaxi, 3 + mlenmaxi, 0 + mlenmaxi, 0 + mlenmaxi };
    int lmp[] = { 0, algoType == DBMODE_STATIC? countMaxiNoted - 1 : countMaxiNoted + countMaxiHided - 1 };
    
    for (int i=0; i<2; i++)
      if (ua_marklinks2[lmp[i]] != -1)
      {
        texts[i].uarea_pos = ua_marks[ua_marklinks2[lmp[i]]].anchor;
        texts[i].uarea_atto = area.atto;
        texts[i].visible = 1;
      }
      else
        texts[i].visible = 0;
    
    if (textInnerPlaced)
    {
      UPTP_UP[area.atto](&texts[area.mirrored? 1 : 0], decays[area.atto]);
      UPTP_DOWN[area.atto](&texts[area.mirrored? 0 : 1], decays[area.atto]);
    }
    else
    {
      UPTP_MID[area.atto](&texts[0], decays[area.atto]);
      UPTP_MID[area.atto](&texts[1], decays[area.atto]);
    }
    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    MarginMarks::sizeHint(atto, atto_size, mindly, mindly1, mindly2);
    QFontMetrics fm(font);
    QSize meansize(5 + fm.averageCharWidth()*rmaxlen(), fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *atto_size += d_atto_size;
    *mindly1 = d_half_dly;
    *mindly2 = d_half_dly;
  }
public:
  virtual void  relatedTextup()
  {
    if (rtexttype == RF_SETBOUNDS)
    {
      float LL = rdata.rel_fixed.LL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      float HL = rdata.rel_fixed.HL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      numfmt.autoFormat(LL, HL);
      assignText(&texts[0], numfmt(LL), fontReplaced, font);
      assignText(&texts[1], numfmt(HL), fontReplaced, font);
    }
    else if (rtexttype == RF_SETTAPS)
    {
      QString sstr(rdata.rel_tap_qstring.slen, Qt::Uninitialized);
      rdata.rel_tap_qstring.tapfn(0, lmardimm, lmoffset, rdata.rel_tap_qstring.param, sstr);
      assignText(&texts[0], sstr, fontReplaced, font);
      rdata.rel_tap_qstring.tapfn(1, lmardimm, lmoffset, rdata.rel_tap_qstring.param, sstr);
      assignText(&texts[1], sstr, fontReplaced, font);
    }
    else if (rtexttype == RF_SETENUMERATE)
    {
      int num = rdata.rel_enumerate.enfrom;
      assignText(&texts[0], QString::number(num), fontReplaced, font);
      assignText(&texts[1], QString::number(num+1), fontReplaced, font);
    }
  }
};
MarginMinTexts::~MarginMinTexts(){}

/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/

class MarginMarksTextBetween: public MarginMarksTexted
{
protected:
  bool      showLastEnumer;
  unsigned int submod;
public:
  ~MarginMarksTextBetween();
  MarginMarksTextBetween(): showLastEnumer(false), submod(1) {}

  void  init_wide(int algotype, int marksCount, int pixStep_pixSpacing, unsigned int step, bool alwaysShowLast)
  {
    MarginMarks::init(algotype, marksCount, pixStep_pixSpacing, 0);
    showLastEnumer = alwaysShowLast;
    submod = step;
  }
  virtual int   setMarksCount(int marksCount)
  {
    marksCount = MarginMarks::setMarksCount(marksCount);
    if (marksCount)
      texts = new texts_t[marksCount-1];
    return marksCount;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    MarginMarks::draw(painter);
    if (fontReplaced)
      painter.setFont(font);
    for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].uarea_pos, texts[i].uin_text);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool continueUpdate = MarginMarks::updateArea(area, UPDATEFOR);
    if (continueUpdate == false && UPDATEFOR < UF_LVL2)
      return false;
    
    storeDimm(c_dimm_main);
    
    const int decayH = 2, decayV = 2, total = countMaxiNoted + countMaxiHided - 1;

    int last=0, subctr = 0;
    for (int i=0; i<total; )
    {
      if (ua_marklinks2[i] != -1)
      {
        int j=i+1;
        for (; j<total+1; j++)
        {
          if (ua_marklinks2[j] != -1)
            break;
          else
            texts[j].visible = 0;
        }

        if (algoType == DBMODE_STATIC && j > total)
          texts[i].visible = 0;
        else
        {
          texts[last = i].visible = subctr++ % int(submod) == 0? 1 : 0;
          QPoint& mcur = ua_marks[ua_marklinks2[i]].anchor, &mnext = j == total+1? ua_marks[ua_marklinks2[i]].anchor : ua_marks[ua_marklinks2[j]].anchor;
//          qDebug()<<i<<ua_marklinks2[i]<<j<<ua_marklinks2[j]<<total+1<<-(mnext - mcur).y()/2;
          if (area.atto == AT_LEFT)
            texts[i].uarea_pos = mcur - QPoint(decayH + texts[i].uin_locsize.width(), -(mnext - mcur).y()/2 + texts[i].uin_locsize.height()/2);
          else if (area.atto == AT_RIGHT)
            texts[i].uarea_pos = mcur - QPoint(-decayH, -(mnext - mcur).y()/2 + texts[i].uin_locsize.height()/2);
          else if (area.atto == AT_TOP)
            texts[i].uarea_pos = mcur - QPoint(-(mnext - mcur).x()/2 + texts[i].uin_locsize.width()/2, decayV + texts[i].uin_locsize.height());
          else if (area.atto == AT_BOTTOM)
            texts[i].uarea_pos = mcur - QPoint(-(mnext - mcur).x()/2 + texts[i].uin_locsize.width()/2, -decayV);
          texts[i].uarea_atto = area.atto;
        }
        i = j;
      }
      else
        texts[i++].visible = 0;
    }
    
    if (showLastEnumer)
    {
      if (last && texts[total-1].visible != 1)
      {
        texts[total - 1].visible = 1;
        if (last != total-1)
        {
          texts[last].visible = 0;
          texts[total - 1].uarea_pos = texts[last].uarea_pos;
        }
      }
    }
    rearrange(BAR_VERT[area.atto], area.mirrored, texts, total);

    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    MarginMarks::sizeHint(atto, atto_size, mindly, mindly1, mindly2);
    QFontMetrics fm(font);
    QSize meansize(3 + fm.averageCharWidth()*rmaxlen(), fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width()+2 : meansize.height()+2;
//    int d_half_dly = BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    int d_half_dly = 0;
    *atto_size = qMax(*atto_size, d_atto_size);
    *mindly1 += d_half_dly;
    *mindly2 += d_half_dly;
  }
public: 
  virtual void  relatedTextup()
  {
    if (rtexttype == RF_SETBOUNDS)
    {
      float LL = rdata.rel_fixed.LL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      float HL = rdata.rel_fixed.HL + (rdata.rel_fixed.HL - rdata.rel_fixed.LL)*reloffset;
      float bndstep = qAbs(HL - LL)/(countMaxiNoted + countMaxiHided - 1);
      numfmt.autoFormat(LL, HL);
      for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
        assignText(&texts[i], numfmt(LL + i*bndstep), fontReplaced, font);
    }
    else if (rtexttype == RF_SETTAPS)
    {
      QString sstr(rdata.rel_tap_qstring.slen, Qt::Uninitialized);
      for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
      {
        rdata.rel_tap_qstring.tapfn(i, lmardimm, lmoffset, rdata.rel_tap_qstring.param, sstr);
        assignText(&texts[i], sstr, fontReplaced, font);
      }
    }
    else if (rtexttype == RF_SETENUMERATE)
    {
//      if (recycle != -1)  num = num % recycle;
      for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
      {
        int num = rdata.rel_enumerate.enfrom + i;
        assignText(&texts[i], QString::number(num), fontReplaced, font);
      }
    }
  }
};
MarginMarksTextBetween::~MarginMarksTextBetween(){}


/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/

class MarginMarksWidgetBetween: public MarginMarks
{
protected:
  static const int decayH = 2;
  static const int decayV = 2;
protected:
  bool      showLastEnumer;
  unsigned int submod;
  unsigned int maxperpendiculardimm;
  
  QVector<QWidget*>   widgets;
  
  union wdgfill_t
  {
    struct
    {
      bool                doretap;
      mtap_qwidget_fn     ftor;
      void*               fpm;
    } wf_dynamic;
    struct
    {
      QWidget**           wlist;
      unsigned int        wcount;
    } wf_static;
  }
                      wfdetails;
  enum  { WF_OFF, WF_STATIC, WF_DYNAMIC }   wftype;
  
  QWidget*            parent;
public:
  ~MarginMarksWidgetBetween();
  MarginMarksWidgetBetween(): showLastEnumer(false), submod(1), maxperpendiculardimm(0), wftype(WF_OFF)
  {}

  void  init_wide(int algotype, int marksCount, int pixStep_pixSpacing, unsigned int step, bool alwaysShowLast, unsigned int maxperpdimm, mtap_qwidget_fn ftor, void* fpm, QWidget* prnt)
  {
    MarginMarks::init(algotype, marksCount, pixStep_pixSpacing, 0);
    showLastEnumer = alwaysShowLast;
    submod = step;
    maxperpendiculardimm = maxperpdimm;
    {
      wftype = WF_DYNAMIC;
      wfdetails.wf_dynamic.doretap = true;
      wfdetails.wf_dynamic.ftor = ftor;
      wfdetails.wf_dynamic.fpm = fpm;
    }
    parent = prnt;
  }
  void  init_wide(int algotype, int marksNwidgetsCount, int pixStep_pixSpacing, unsigned int step, bool alwaysShowLast, unsigned int maxperpdimm, QWidget* wdgs[], QWidget* prnt)
  {
    MarginMarks::init(algotype, marksNwidgetsCount, pixStep_pixSpacing, 0);
    showLastEnumer = alwaysShowLast;
    submod = step;
    maxperpendiculardimm = maxperpdimm;
    if (marksNwidgetsCount > 1)
    {
      wftype = WF_STATIC;
      wfdetails.wf_static.wcount = marksNwidgetsCount - 1;
      wfdetails.wf_static.wlist = new QWidget*[wfdetails.wf_static.wcount];
      for (unsigned int i=0; i<wfdetails.wf_static.wcount; i++)
      {
        wdgs[i]->setParent(prnt);
//        wdgs[i]
        wfdetails.wf_static.wlist[i] = wdgs[i];
      }
    }
    parent = prnt;
  }
  void  retap()
  {
    if (wftype == WF_DYNAMIC)
    {
      wfdetails.wf_dynamic.doretap = true;
      updateArea(cached_area(), UF_LVL3);
    }
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
//    qDebug()<<area.dlytotal<<cached_area().dlytotal;
    bool continueUpdate = MarginMarks::updateArea(area, UPDATEFOR);
//    qDebug()<<continueUpdate;
    if (continueUpdate == false && UPDATEFOR < UF_LVL2)
      return false;
    
//    storeDimm(c_dimm_main);
    
    const int total = countMaxiNoted + countMaxiHided - 1;
    
    if (wftype == WF_DYNAMIC && wfdetails.wf_dynamic.doretap)
    {
      for (int i=0; i<widgets.count(); i++)
        widgets[i]->deleteLater();
      widgets.resize(total);
      for (int i=0; i<total; i++)
      {
        widgets[i] = wfdetails.wf_dynamic.ftor(i, ua_marks[ua_marklinks2[i]].anchor.x(), c_dimm_main, wfdetails.wf_dynamic.fpm);
        widgets[i]->setParent(parent);
        if (BAR_VERT[area.atto])
          widgets[i]->setMaximumWidth(maxperpendiculardimm);
        else
          widgets[i]->setMaximumHeight(maxperpendiculardimm);
      }
      wfdetails.wf_dynamic.doretap = false;
    }
    else if (wftype == WF_STATIC)
    {
      if (total > widgets.size())
      {
        int ad = widgets.size();
        for (int i=ad; i<total; i++)
          widgets.push_back(wfdetails.wf_static.wlist[i]);
      }
      else if (total < widgets.size())
      {
        int ad = widgets.size();
        for (int i=ad; i<total; i++)
          widgets[i]->setVisible(false);
        widgets.resize(total);
      }
    }
    

    int last=0, subctr = 0;
    for (int i=0; i<total; )
    {
      if (ua_marklinks2[i] != -1)
      {
        int j=i+1;
        for (; j<total+1; j++)
        {
          if (ua_marklinks2[j] != -1)
            break;
          else
            widgets[j]->setVisible(false);
        }

        if (algoType == DBMODE_STATIC && j > total)
          widgets[i]->setVisible(false);
        else
        {
          widgets[last = i]->setVisible(subctr++ % int(submod) == 0);
          QPoint& mcur = ua_marks[ua_marklinks2[i]].anchor; //, &mnext = j == total+1? ua_marks[ua_marklinks2[i]].anchor : ua_marks[ua_marklinks2[j]].anchor;
//          if (area.atto == AT_LEFT)
//            widgets[i]->move(mcur - QPoint(decayH + qMin<int>(widgets[i]->width(), maxperpendiculardimm), -(mnext - mcur).y()/2 + texts[i].uin_locsize.height()/2));
//          else if (area.atto == AT_RIGHT)
//            texts[i].uarea_pos = mcur - QPoint(-decayH, -(mnext - mcur).y()/2 + texts[i].uin_locsize.height()/2);
//          else if (area.atto == AT_TOP)
//            texts[i].uarea_pos = mcur - QPoint(-(mnext - mcur).x()/2 + texts[i].uin_locsize.width()/2, decayV + texts[i].uin_locsize.height());
//          else if (area.atto == AT_BOTTOM)
//            texts[i].uarea_pos = mcur - QPoint(-(mnext - mcur).x()/2 + texts[i].uin_locsize.width()/2, -decayV);
              
          if (area.atto == AT_LEFT)
            widgets[i]->move(mcur - QPoint(-decayH + qMin<int>(widgets[i]->width(), maxperpendiculardimm), decayV));
          else if (area.atto == AT_RIGHT)
            widgets[i]->move(mcur - QPoint(-decayH, decayV));
          else if (area.atto == AT_TOP)
            widgets[i]->move(mcur - QPoint(-decayH, decayV + qMin<int>(widgets[i]->height(), maxperpendiculardimm)));
          else if (area.atto == AT_BOTTOM)
            widgets[i]->move(mcur - QPoint(-decayH, -decayV));
              
          widgets[i]->setVisible(true);
        }
        i = j;
      }
      else
        widgets[i++]->setVisible(false);
    }
    
    if (showLastEnumer && last && !widgets[total-1]->isVisible())
    {
      widgets[total - 1]->setVisible(true);
      if (last != total-1)
        widgets[last]->setVisible(false);
    }
    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* mindly, int* mindly1, int* mindly2) const
  {
    MarginMarks::sizeHint(atto, atto_size, mindly, mindly1, mindly2);
//    int d_atto_size = BAR_VERT[atto]? meansize.width()+2 : meansize.height()+2;
    int d_atto_size = maxperpendiculardimm + (BAR_VERT[atto]? decayH : decayV);
    int d_half_dly = 0;
    *atto_size = qMax(*atto_size, d_atto_size);
    *mindly1 += d_half_dly;
    *mindly2 += d_half_dly;
  }
public:
};
MarginMarksWidgetBetween::~MarginMarksWidgetBetween(){}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define RAKOFLAG

class MarginBoundDepended;
class MarginElement;
class DrawBars_impl
{
public:
  DrawBars_impl():
    c_mirroredHorz(false), c_mirroredVert(false),
    drawCoreInited(false){}
  
  int           c_hint_draw_width, c_hint_draw_height;
  bool          c_mirroredHorz, c_mirroredVert;
  
  int           c_width, c_height, c_overheadRight, c_overheadBottom;
  QMargins      c_margins;
  int           c_width_margins, c_height_margins;
  
  bool          expandNeighborBarsIfNeed;
#ifdef RAKOFLAG
  bool          rakoflag;
#endif
  
  int           clr_policy;
  QBrush        c_back_brush, c_front_brush;
  QPen          c_front_pen;
  
  struct        
  {
    int         summ;
    int         dly1;
    int         dly2;
    int         c_size;
  }             ttr[4];
  void          preresize(const QSize& size){ c_width = size.width(); c_height = size.height(); }
public:
  struct        melem_t
  {
    MarginElement*  pme;
    MEQWrapper*     pwp;
    int             offset, length;
    bool            shared, visible, ivbanned;
    melem_t(){}
    melem_t(MarginElement* _pme, MEQWrapper* _pwp, bool isshared, bool interventBanned): 
      pme(_pme), pwp(_pwp), offset(0), length(0), shared(isshared), visible(true), ivbanned(interventBanned) {}
  };
  QVector<melem_t>   elems[4];
  QVector<MarginBoundDepended*>   elemsBoundDepended;
  QVector<MarginBoundDepended*>   elemsScrollDepended;
  typedef QVector<melem_t>::iterator melem_iterator_t;
  bool  drawCoreInited;

  void        clearTTR(ATTACHED_TO atto)
  {
    ttr[atto].summ = ttr[atto].dly1 = ttr[atto].dly2 = /*ttr[atto].c_dly = */ttr[atto].c_size = 0;
  }
  void        clearTTR()
  {
    for (int i=0; i<4; i++)
      clearTTR(ATTACHED_TO(i));
  }
  void        updateTTR()
  {
//    const int pr[4][2] = {  {AT_TOP, AT_BOTTOM}, {AT_TOP, AT_BOTTOM}, {AT_LEFT, AT_RIGHT}, {AT_LEFT, AT_RIGHT}  };
//    for (int i=0; i<4; i++)
//    {
////      ttr[i].c_dly = qMax(ttr[pr[i][0]].dly1, ttr[pr[i][1]].dly1);
//      int    c_dly = qMax(ttr[pr[i][0]].dly1, ttr[pr[i][1]].dly1);
//      ttr[i].c_size = qMax(ttr[i].summ, c_dly);
//    }
    ttr[AT_LEFT].c_size = qMax(ttr[AT_LEFT].summ, qMax(ttr[AT_TOP].dly1, ttr[AT_BOTTOM].dly1));
    ttr[AT_RIGHT].c_size = qMax(ttr[AT_RIGHT].summ, qMax(ttr[AT_TOP].dly2, ttr[AT_BOTTOM].dly2));
    ttr[AT_TOP].c_size = qMax(ttr[AT_TOP].summ, qMax(ttr[AT_LEFT].dly1, ttr[AT_RIGHT].dly1));
    ttr[AT_BOTTOM].c_size = qMax(ttr[AT_BOTTOM].summ, qMax(ttr[AT_LEFT].dly2, ttr[AT_RIGHT].dly2));
  }
  
  void        resizeBars(int UPDATEFOR, int width, int height, int overheadRight, int overheadBottom)
  {
//    qDebug()<<"before, after:"<<c_width<<width<<overheadRight;//<<"____"<<c_height<<height;
    c_width = width; c_height = height; c_overheadRight = overheadRight; c_overheadBottom = overheadBottom;
    reupdateBars(UPDATEFOR);
  }
  
  void        reupdateBars(int UPDATEFOR) // Any cttr changed -> all areas need to be updated
  {
    for (int i=0; i<elems[0].count(); i++)
    {
      int apt = ttr[AT_LEFT].c_size - 1 - elems[0][i].offset;
      MarginElement::uarea_t area = { AT_LEFT, apt, apt - elems[0][i].length, c_height, 
                                      ttr[AT_TOP].c_size, ttr[AT_BOTTOM].c_size, c_overheadBottom, c_mirroredVert };
      elems[0][i].pme->updateArea(area, UPDATEFOR);
    }
    for (int i=0; i<elems[1].count(); i++)
    {
      int apt = c_width - 1 - ttr[AT_RIGHT].c_size + 1 + elems[1][i].offset;
      MarginElement::uarea_t area = { AT_RIGHT, apt, apt + elems[1][i].length, c_height, 
                                      ttr[AT_TOP].c_size, ttr[AT_BOTTOM].c_size, c_overheadBottom, c_mirroredVert };
      elems[1][i].pme->updateArea(area, UPDATEFOR);
    }
    for (int i=0; i<elems[2].count(); i++)
    {
      int apt = ttr[AT_TOP].c_size - 1 - elems[2][i].offset;
      MarginElement::uarea_t area = { AT_TOP, apt, apt - elems[2][i].length, c_width, 
                                      ttr[AT_LEFT].c_size, ttr[AT_RIGHT].c_size, c_overheadRight, c_mirroredHorz };
      elems[2][i].pme->updateArea(area, UPDATEFOR);
    }
    for (int i=0; i<elems[3].count(); i++)
    {
      int apt = c_height - 1 - ttr[AT_BOTTOM].c_size + 1 + elems[3][i].offset;
      MarginElement::uarea_t area = { AT_BOTTOM, apt, apt + elems[3][i].length, c_width,
                                      ttr[AT_LEFT].c_size, ttr[AT_RIGHT].c_size, c_overheadRight, c_mirroredHorz };
      elems[3][i].pme->updateArea(area, UPDATEFOR);
    }
  }
  int         recalcTTRforBar(ATTACHED_TO atto)
  {
    int total=0, total_prev=0;
    int maxdly1=0, maxdly2=0;
    for (melem_iterator_t iter=elems[atto].begin(); iter!=elems[atto].end(); iter++)
    {
#if 0 // old variant of shared area
      if (iter->shared && total != 0)
      {
        iter->offset = total_prev;
        iter->length = total - total_prev;
      }
      else
      {
        int atto_size, mindly, curdly1, curdly2;
        iter->pme->sizeHint(atto, &atto_size, &mindly, &curdly1, &curdly2);
        iter->offset = total;
        iter->length = atto_size;
        total_prev = total;
        total += atto_size;
        if (iter->ivbanned == false)
        {
          if (maxdly1 < curdly1)  maxdly1 = curdly1;
          if (maxdly2 < curdly2)  maxdly2 = curdly2;
        }
      }
#else
      int atto_size, mindly, curdly1, curdly2;
      iter->pme->sizeHint(atto, &atto_size, &mindly, &curdly1, &curdly2);
      
      if (iter->shared && total != 0)
      {
        iter->length = atto_size;
//        if (total_prev < atto_size)
//          total_prev = atto_size;
        iter->offset = total_prev/* + iter->length*/;
      }
      else
      {
        iter->offset = total;
        iter->length = atto_size;
        total_prev = total;
        total += atto_size;
        if (iter->ivbanned == false)
        {
          if (maxdly1 < curdly1)  maxdly1 = curdly1;
          if (maxdly2 < curdly2)  maxdly2 = curdly2;
        }
      }
#endif
    }
    int delta = total - ttr[atto].summ;
    ttr[atto].summ = total;
    ttr[atto].dly1 = maxdly1;
    ttr[atto].dly2 = maxdly2;
    updateTTR();
    return delta;
  }
  
  bool        excludeElement(MarginElement* me, ATTACHED_TO* atto=nullptr)
  {
    for (int d=0; d<4; d++)
      for (melem_iterator_t iter=elems[d].begin(); iter!=elems[d].end(); iter++)
        if (iter->pme == me)
        {
          if (atto) *atto = ATTACHED_TO(d);
          elems[d].erase(iter);
          return true;
        }
    return false;
  }
public:
//  void  setDefaultColors(QWidget* wdg)
//  {
//    QPalette pal;
//    wdg->setAutoFillBackground(true);
//    wdg->setPalette(pal);
//    clr_back = pal.color(QPalette::Window);
//    clr_front = pal.color(QPalette::WindowText);
//  }
//  void  setColors(QWidget* wdg, const QColor& bckg, const QColor& frg)
//  {
//    QPalette pal;
//    pal.setColor(QPalette::Window, clr_back = bckg);
//    pal.setColor(QPalette::WindowText, clr_front = frg);
////    pal.setBrush(QPalette::WindowText, 
////    wdg->setAutoFillBackground(true);
//    wdg->setPalette(pal);
//  }
  
  void  updatePalette(QWidget* wdg, const QColor& clr_back, const QColor& clr_front)
  {
    QPalette pal(wdg->palette());
    pal.setColor(QPalette::Window, clr_back);
    pal.setColor(QPalette::WindowText, clr_front);
    wdg->setPalette(pal);
    
    c_back_brush = clr_back;
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
  
  void  updatePalette(QWidget* wdg, const QColor& clr_front)
  {
    QPalette pal(wdg->palette());
    pal.setColor(QPalette::WindowText, clr_front);
    wdg->setPalette(pal);
    
    c_back_brush = pal.color(QPalette::Window);
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
  
  void  upbackPalette(QWidget* wdg)
  {
    c_back_brush = wdg->palette().color(QPalette::Window);
    QColor clr_front = wdg->palette().color(QPalette::WindowText);
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
};

//#define COLOR3(clr) (clr >> 16) & 0xFF, (clr >> 8) & 0xFF, (clr) & 0xFF
#define COLOR3(clr) (clr) & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF

inline bool isDrawPaletteCP(int cp)
{
  return cp == DrawBars::CP_FROM_DRAWBACK || cp == DrawBars::CP_FROM_DRAWPALETTE || cp == DrawBars::CP_FROM_DRAWPALETTE_INV;
}


DrawBars::DrawBars(DrawQWidget* pdraw, COLORS colorsPolicy, QWidget *parent) : QWidget(parent), pDraw(pdraw)
{
  this->setAutoFillBackground(false);
  pImpl = new DrawBars_impl();
  setColorPolicy(colorsPolicy);

//  pImpl->rakoflag = false;
  pImpl->clearTTR();

  pDraw->setParent(this);
  pDraw->move(pImpl->ttr[AT_LEFT].summ, pImpl->ttr[AT_TOP].summ);
  pDraw->show();
  
  pImpl->c_mirroredHorz = orientationMirroredHorz(pDraw->orientation());
  pImpl->c_mirroredVert = orientationMirroredVert(pDraw->orientation());
  
  pImpl->c_hint_draw_width = pDraw->size().width();
  pImpl->c_hint_draw_height = pDraw->size().height();
  
  pImpl->c_width = pImpl->c_height = 0;
  
  pImpl->c_width_margins = 0;
  pImpl->c_height_margins = 0;
  
//  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

DrawBars::~DrawBars()
{
  for (int d=0; d<4; d++)
    for (int i=0; i<pImpl->elems[d].count(); i++)
      delete pImpl->elems[d][i].pme;
  
  delete pImpl;
}

void DrawBars::setColorPolicy(DrawBars::COLORS cp)
{
  pImpl->clr_policy = cp;
  if (isDrawPaletteCP(cp))
  {
    const IPalette* ppal = pDraw->dataPalette();
    unsigned int  fc = ppal->first(), lc = ppal->last();
    if (cp == CP_FROM_DRAWPALETTE_INV)
      qSwap(fc, lc);
    else if (cp == CP_FROM_DRAWBACK)
      fc = pDraw->colorBack();
    pImpl->updatePalette(this, QColor(COLOR3(fc)), QColor(COLOR3(lc)));
  }
  else if (cp == CP_WHITE)
    pImpl->updatePalette(this, QColor(Qt::white));
  else if (cp == CP_BLACK)
    pImpl->updatePalette(this, QColor(Qt::black));
  else                                        /// strict movement to default
  {
    pImpl->clr_policy = CP_DEFAULT;
    pImpl->upbackPalette(this);
  }
}

void DrawBars::setColors(const QColor& backgroundColor, const QColor& foregroundColor)
{
  pImpl->clr_policy = -1;
  pImpl->updatePalette(this, backgroundColor, foregroundColor);
}

void DrawBars::setColors(unsigned int backgroundColor, unsigned int foregroundColor)
{
  pImpl->clr_policy = -1;
  pImpl->updatePalette(this, QColor(COLOR3(backgroundColor)), QColor(COLOR3(foregroundColor)));
}

//void DrawBars::slot_setBackgroundColor(const QColor& color)
//{
//  setColors(color, pImpl->clr_front);
//}

void DrawBars::slot_setForegroundColor(const QColor& color)
{
//  setColors(pImpl->clr_back, color);
  pImpl->updatePalette(this, color);
}

DrawQWidget*  DrawBars::getDraw()
{
  return pDraw;
}

const DrawQWidget*  DrawBars::getDraw() const
{
  return pDraw;
}

DrawQWidget*  DrawBars::replaceDraw(DrawQWidget* newdraw)
{
  DrawQWidget* old = pDraw;
  pDraw = newdraw;
  update();
  return old;
}

int DrawBars::barSize(ATTACHED_TO atto) const
{
  return pImpl->ttr[atto].c_size;
}


#define PDRAWMOVE     pDraw->move(pImpl->c_margins.left() + pImpl->ttr[AT_LEFT].c_size, pImpl->c_margins.top() + pImpl->ttr[AT_TOP].c_size);



MEQWrapper*   DrawBars::addMarginElement(ATTACHED_TO atto, MarginElement* pme, MEQWrapper* pwp, bool sharedWithPrev, bool interventBanned)
{
//  bool cttr_updated = true;
  ATTACHED_TO atto_was = atto;
  if (pImpl->excludeElement(pme, &atto_was))
    pImpl->recalcTTRforBar(atto_was);
  
  pme->relatedInit(pDraw);
  pImpl->elems[int(atto)].push_back(DrawBars_impl::melem_t(pme, pwp, sharedWithPrev, interventBanned));
  pImpl->recalcTTRforBar(atto);
  
  pImpl->reupdateBars(MarginElement::UF_APPEND);
  
//  qDebug()<<">>>>"<<pImpl->ottrLeft<<pImpl->ottrTop<<pImpl->ottrRight<<pImpl->ottrBottom;
  
//  if (cttr_updated && (BAR_NEEDMOVE[atto] || BAR_NEEDMOVE[atto_was]))
  PDRAWMOVE;
  
//  update();
#ifdef RAKOFLAG
  if (pImpl->rakoflag == false)
#endif
  {
    updateGeometry();
    setGeometry(0,0, 
                pImpl->c_width_margins + pImpl->ttr[AT_LEFT].c_size + pImpl->c_hint_draw_width + pImpl->ttr[AT_RIGHT].c_size,
                pImpl->c_height_margins + pImpl->ttr[AT_TOP].c_size + pImpl->c_hint_draw_height + pImpl->ttr[AT_BOTTOM].c_size);
  }
  
  if (pwp != nullptr)
  {
    pwp->m_pme = pme;
    pwp->m_premote = this;
  }
//    updateGeometry();
  return pwp;
}






MEWLabel* DrawBars::addLabel(ATTACHED_TO atto, int flags, QString text, Qt::Alignment align, Qt::Orientation orient)
{
  MarginLabel*  pme = new MarginLabel(text, this->font(), flags & DBF_LABELAREA_FULLBAR, align, orient);
  return (MEWLabel*)addMarginElement(atto, pme, new MEWLabel, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWSpace* DrawBars::addSpace(ATTACHED_TO atto, int space)
{
  MarginElement*  pme = new MarginSpace(space);
  return (MEWSpace*)addMarginElement(atto, pme, new MEWSpace, false, false);
}

MEWSpace* DrawBars::addContour(ATTACHED_TO atto, int space, bool maxzone)
{
  MarginElement*  pme = new MarginContour(space, maxzone);
  return (MEWSpace*)addMarginElement(atto, pme, new MEWSpace, false, false);
}

MEWPointer* DrawBars::addPointerFixed(ATTACHED_TO atto, int flags, float LL, float HL)
{
  MarginPointer*  pme = new MarginPointer(Qt::AlignCenter, false/*flags & DBF_NOTESINSIDE*/);
  pme->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL)));
  return (MEWPointer*)addMarginElement(atto, pme, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWPointer* DrawBars::addPointerDrawUniSide(ATTACHED_TO atto, int flags)
{
  MarginPointer*  pme = new MarginPointer(Qt::AlignCenter, false/*flags & DBF_NOTESINSIDE*/);
  int sizeDimm = BAR_VERT[atto]? int(pDraw->sizeDataVert() + 1) : int(pDraw->sizeDataHorz() + 1);
  pme->setRelatedOpts(RF_SETENUMERATE, relatedopts_t(sizeDimm-1, 0/*flags & DBF_ENUMERATE_FROMZERO? 0 : 1*/, -1));
  return (MEWPointer*)addMarginElement(atto, pme, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWPointer* DrawBars::addPointerDrawGraphB(ATTACHED_TO atto, int flags)
{
  MarginPointer*  pme = new MarginPointer(Qt::AlignCenter, false/*flags & DBF_NOTESINSIDE*/);
  pme->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(pDraw->bounds()));
  pImpl->elemsBoundDepended.push_back(pme);
  return (MEWPointer*)addMarginElement(atto, pme, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}


#define DB_EXTRACT_MODE(flags, DEFAULTMODE)  (((flags)&0x3) == 0? DEFAULTMODE : (flags)&0x3)

MEWScale* DrawBars::addScaleEmpty(ATTACHED_TO atto, int flags, int fixedCount, int pixStep_pixSpacing, int miniPerMaxiLIMIT)
{
  MarginMarks*  pme = new MarginMarks();
  pme->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), fixedCount, pixStep_pixSpacing, miniPerMaxiLIMIT);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = fixedCount*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = fixedCount*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, pme, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleNN* DrawBars::addScaleFixed(ATTACHED_TO atto, int flags, float LL, float HL, int fixedCount, int pixStep_pixSpacing, int miniPerMaxiLIMIT)
{
  MarginMarksTexted* mmt;
  if (flags & DBF_ONLY2NOTES)
    mmt = new MarginMinTexts(flags & DBF_NOTESINSIDE);
  else
    mmt = new Margin1Mark1Text(flags & DBF_NOTESINSIDE);

  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), fixedCount, pixStep_pixSpacing, miniPerMaxiLIMIT);
  mmt->setFont(this->font());
  mmt->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL)));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = fixedCount*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = fixedCount*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScaleNN*)addMarginElement(atto, mmt, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

inline int countMaxNumbers(int marksCount)
{
  int n = 1;
  while (marksCount % 10 != 0)
  {
    marksCount /= 10;
    n++;
  }
  return n;
}

MEWScaleNM* DrawBars::addScaleEnumerator(ATTACHED_TO atto, int flags, int marksCount, int pixStep_pixSpacing, unsigned int step, bool alwaysShowLast)
{
  MarginMarksTextBetween* mmt = new MarginMarksTextBetween();
  mmt->init_wide(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixStep_pixSpacing, step, alwaysShowLast);
  mmt->setFont(this->font());
  mmt->setRelatedOpts(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(marksCount-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksCount-1)*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = (marksCount-1)*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, mmt, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleNN* DrawBars::addScaleTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksCount, int pixStep_pixSpacing)
{
  MarginMarksTexted* mmt = new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixStep_pixSpacing, 0);
  mmt->setFont(this->font());
  mmt->setRelatedOpts(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksCount*(pixStep_pixSpacing);
    else                    pImpl->c_hint_draw_width = marksCount*(pixStep_pixSpacing);
//    updateGeometry();
  }
  return (MEWScaleNN*)addMarginElement(atto, mmt, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleNM* DrawBars::addScaleTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksCount, int pixStep_pixSpacing)
{
  MarginMarksTextBetween* mmt = new MarginMarksTextBetween();
  mmt->init_wide(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixStep_pixSpacing, 1, true);
  mmt->setFont(this->font());
  mmt->setRelatedOpts(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksCount-1)*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = (marksCount-1)*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, mmt, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleNM* DrawBars::addScaleTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param, int marksCount, int pixStep_pixSpacing)
{
  MarginMarksWidgetBetween* mmt = new MarginMarksWidgetBetween();
  mmt->init_wide(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixStep_pixSpacing, 1, true, maxperpendiculardimm, fn, param, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksCount-1)*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = (marksCount-1)*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, mmt, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleNM* DrawBars::addScaleWidgetsNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int pixStep_pixSpacing)
{
  MarginMarksWidgetBetween* mmt = new MarginMarksWidgetBetween();
  mmt->init_wide(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED), marksNwidgetsCount, pixStep_pixSpacing, 1, true, maxperpendiculardimm, wdgs, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksNwidgetsCount-1)*pixStep_pixSpacing;
    else                    pImpl->c_hint_draw_width = (marksNwidgetsCount-1)*pixStep_pixSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, mmt, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScale* DrawBars::addScaleDrawUniSide(ATTACHED_TO atto, int flags, int pixSpacing, unsigned int step)
{
  MarginMarksTextBetween* mmt = new MarginMarksTextBetween();
  int sizeDimm = BAR_VERT[atto]? int(pDraw->sizeDataVert() + 1) : int(pDraw->sizeDataHorz() + 1);
  mmt->init_wide(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED), sizeDimm, pixSpacing, step, flags & DBF_ENUMERATE_SHOWLAST);
  mmt->setRelatedOpts(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(sizeDimm-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (sizeDimm-1)*(pixSpacing);
    else                    pImpl->c_hint_draw_width = (sizeDimm)*(pixSpacing);
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, mmt, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScale* DrawBars::addScaleDrawUniSide(ATTACHED_TO atto, int flags, float LL, float HL, int pixSpacing, int miniPerMaxiLIMIT)
{
//  Margin1Mark1Text* mmt = new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  MarginMarksTexted* mmt = flags & DBF_ONLY2NOTES? (MarginMarksTexted*)new MarginMinTexts(flags & DBF_NOTESINSIDE) : 
                                                  (MarginMarksTexted*)new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  int sizeDimm = BAR_VERT[atto]? int(pDraw->sizeDataVert() + 1) : int(pDraw->sizeDataHorz() + 1);
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED), sizeDimm, pixSpacing, miniPerMaxiLIMIT);
  mmt->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL)));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = sizeDimm*(pixSpacing);
    else                    pImpl->c_hint_draw_width = sizeDimm*(pixSpacing);
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, mmt, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScale* DrawBars::addScaleDrawGraphB(ATTACHED_TO atto, int flags, int marksCount, int pixSpacing, int miniPerMaxiLIMIT)
{
  MarginMarksTexted* mmt = flags & DBF_ONLY2NOTES? (MarginMarksTexted*)new MarginMinTexts(flags & DBF_NOTESINSIDE) : 
                                                  (MarginMarksTexted*)new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixSpacing, miniPerMaxiLIMIT);
  mmt->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(pDraw->bounds()));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksCount*(pixSpacing);
    else                    pImpl->c_hint_draw_width = marksCount*(pixSpacing);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(/*(MarginBoundDepended*)*/mmt);
  return (MEWScale*)addMarginElement(atto, mmt, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScale* DrawBars::addScaleDrawGraphB(ATTACHED_TO atto, int flags, float LL, float HL, int marksCount, int pixSpacing, int miniPerMaxiLIMIT)
{
  MarginMarksTexted* mmt = flags & DBF_ONLY2NOTES? (MarginMarksTexted*)new MarginMinTexts(flags & DBF_NOTESINSIDE) : 
                                                  (MarginMarksTexted*)new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STRETCHED_POW2), marksCount, pixSpacing, miniPerMaxiLIMIT);
  mmt->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL)));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksCount*(pixSpacing);
    else                    pImpl->c_hint_draw_width = marksCount*(pixSpacing);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(mmt);
  return (MEWScale*)addMarginElement(atto, mmt, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleTAP*          DrawBars::addScaleDrawRecorderB(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, const void* param, int miniPerMaxiLIMIT)
{
  MarginMarksTexted* mmt = flags & DBF_ONLY2NOTES? (MarginMarksTexted*)new MarginMinTexts(flags & DBF_NOTESINSIDE) : 
                                                  (MarginMarksTexted*)new Margin1Mark1Text(flags & DBF_NOTESINSIDE);
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STATIC), marksCount, pixStep, miniPerMaxiLIMIT);
  mmt->setRelatedOpts(RF_SETTAPS, relatedopts_t(mtfn, param, maxtextlen));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksCount*(pixStep);
    else                    pImpl->c_hint_draw_width = marksCount*(pixStep);
//    updateGeometry();
  }
  pImpl->elemsScrollDepended.push_back(mmt);
  return (MEWScaleTAP*)addMarginElement(atto, mmt, new MEWScaleTAP, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

MEWScaleTAP* DrawBars::addScaleDrawRecorderNM(ATTACHED_TO atto, int flags, int marksCount, int pixStep, mtap_qstring_fn mtfn, int maxtextlen, const void* param, int miniPerMaxiLIMIT)
{
  MarginMarksTexted* mmt = new MarginMarksTextBetween();
  mmt->init(DB_EXTRACT_MODE(flags, DBMODE_STATIC), marksCount, pixStep, miniPerMaxiLIMIT);
  mmt->setRelatedOpts(RF_SETTAPS, relatedopts_t(mtfn, param, maxtextlen));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksCount-1)*(pixStep);
    else                    pImpl->c_hint_draw_width = (marksCount-1)*(pixStep);
//    updateGeometry();
  }
  pImpl->elemsScrollDepended.push_back(mmt);
  return (MEWScaleTAP*)addMarginElement(atto, mmt, new MEWScaleTAP, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED);
}

void DrawBars::retrieveMElement(MEQWrapper* mw, bool replaceWithEqSpace)
{
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pwp == mw)
      {
        mw->m_premote = nullptr;
        
        if (replaceWithEqSpace)
        {
          int space=0, dly,dly1,dly2;
          iter->pme->sizeHint(ATTACHED_TO(d), &space, &dly, &dly1, &dly2);
          iter->pme = new MarginSpace(space);   /// ntf!
          update();
        }
        else
        {
          pImpl->elems[d].erase(iter);
          int delta = pImpl->recalcTTRforBar(ATTACHED_TO(d));
          PDRAWMOVE;
          
          QSize pr = QSize(pImpl->c_width + (d == 0 || d == 1? delta : 0) , pImpl->c_height + (d == 2 || d == 3? delta : 0));
          
          pImpl->preresize(pr);
          pImpl->reupdateBars(MarginElement::UF_FORCED);
          updateGeometry();
          resize(pr);
//          update();
        }
        return;
      }
}

void DrawBars::setVisible(MEQWrapper* pwp, bool visible)
{
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pwp == pwp)
      {
        iter->visible = visible;
        update();
        return;
      }
}


void DrawBars::switchToAnotherSide(MEQWrapper* pwp)
{
  ATTACHED_TO switched[] = {  AT_RIGHT, AT_LEFT, AT_BOTTOM, AT_TOP   };
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pwp == pwp)
      {
        DrawBars_impl::melem_t elem = *iter;
        pImpl->elems[d].erase(iter);
        pImpl->recalcTTRforBar(ATTACHED_TO(d));
        pImpl->elems[int(switched[d])].push_back(elem);
        pImpl->recalcTTRforBar(switched[d]);
        
        pImpl->reupdateBars(MarginElement::UF_FORCED);
        PDRAWMOVE;
        update();
        return;
      }
}


//void DrawBars::changeColor(MEQWrapper* pwp, const QColor& clr)
//{
////  for (int d=0; d<4; d++)
////    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
////      if (iter->pwp == pwp)
////      {
////        iter->pme->changeColor(clr);
////      }
//}

void DrawBars::swapBars(ATTACHED_TO atto)
{
  ATTACHED_TO switched[] = {  AT_RIGHT, AT_LEFT, AT_BOTTOM, AT_TOP   };
  ATTACHED_TO atto_sw = switched[atto];
  qSwap(pImpl->elems[atto], pImpl->elems[atto_sw]);
  pImpl->recalcTTRforBar(atto);
  pImpl->recalcTTRforBar(atto_sw);
  pImpl->reupdateBars(MarginElement::UF_FORCED);
  PDRAWMOVE;
  update();
}

void DrawBars::removeAllMElements(bool squeeze)
{
  for (int d=0; d<4; d++)
  {
    for (int i=0; i<pImpl->elems[d].count(); i++)
      delete pImpl->elems[d][i].pme;
    pImpl->elems[d].clear();
  }
  if (squeeze)
  {
    pImpl->clearTTR();
    pDraw->setGeometry(pImpl->c_margins.left(), pImpl->c_margins.top(), 
                       this->width() - pImpl->c_width_margins, this->height() - pImpl->c_height_margins);
    updateGeometry();
  }
  update();
}

void DrawBars::elemSizeHintChanged(MarginElement* me)
{
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pme == me)
      {
        int delta = pImpl->recalcTTRforBar(ATTACHED_TO(d));
//        if (BAR_NEEDMOVE[d])
        PDRAWMOVE;
        if (delta > 0)
          updateGeometry();
        else if (delta < 0)
          resize(pImpl->ttr[AT_LEFT].c_size + pDraw->width() + pImpl->ttr[AT_RIGHT].c_size, 
                 pImpl->ttr[AT_TOP].c_size +  pDraw->height() + pImpl->ttr[AT_BOTTOM].c_size);
        
        pImpl->reupdateBars(MarginElement::UF_FORCED);
        return;
      }
}
QSize DrawBars::minimumSizeHint() const
{
//  qDebug()<<pImpl->c_hint_draw_height<<pImpl->c_height_margins<<pImpl->ttr[AT_BOTTOM].c_size;
//  qDebug()<<pImpl->ottrLeft<<pImpl->ottrTop<<pImpl->ottrRight<<pImpl->ottrBottom;
  return QSize(pImpl->c_width_margins + pImpl->ttr[AT_LEFT].c_size + pImpl->c_hint_draw_width + pImpl->ttr[AT_RIGHT].c_size, 
               pImpl->c_height_margins + pImpl->ttr[AT_TOP].c_size + pImpl->c_hint_draw_height + pImpl->ttr[AT_BOTTOM].c_size);
}

//QSize DrawBars::sizeHint() const
//{
//  return pDraw->size() + QSize(pImpl->cttrLeft + pImpl->cttrRight, pImpl->cttrTop + pImpl->cttrBottom);
//}

void DrawBars::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  
  pImpl->c_margins = contentsMargins();
  int margin_width = pImpl->c_margins.left() + pImpl->c_margins.right();
  int margin_height = pImpl->c_margins.top() + pImpl->c_margins.bottom();
  
  if (margin_width != pImpl->c_width_margins || margin_height != pImpl->c_height_margins) 
  {
    pImpl->c_width_margins = margin_width;
    pImpl->c_height_margins = margin_height;
    PDRAWMOVE;
    updateGeometry();
  }
  QSize dsize = event->size() - QSize(pImpl->ttr[AT_LEFT].c_size + pImpl->ttr[AT_RIGHT].c_size,
                                      pImpl->ttr[AT_TOP].c_size + pImpl->ttr[AT_BOTTOM].c_size)
                - QSize(pImpl->c_width_margins, pImpl->c_height_margins);
  
  int dwidth, dheight;
  
#ifdef RAKOFLAG
  bool resizeDrawWillAfterBars = pImpl->rakoflag || !pDraw->isVisible();
  pImpl->rakoflag = false;
#else
  bool resizeDrawWillAfterBars = !pDraw->isVisible();
#endif
  if (resizeDrawWillAfterBars)
    pDraw->fitSize(dsize.width(), dsize.height(), &dwidth, &dheight);
  else
  {
    pDraw->resize(dsize);
    dwidth = (int)pDraw->sizeHorz();
    dheight = (int)pDraw->sizeVert();
  }
  if (/*isVisible() && */height() > 5 && width() > 5)
  {
    pImpl->resizeBars(MarginElement::UF_RESIZE, width() - pImpl->c_width_margins, height() - pImpl->c_height_margins, 
                       pDraw->rawResizeModeNoScaled()? 0 : dsize.width() - dwidth,
                       pDraw->rawResizeModeNoScaled()? 0 : dsize.height() - dheight
                                                       );
//    qDebug()<<event->size()<<width()<<height();
  }
  if (resizeDrawWillAfterBars)
    pDraw->resize(dsize);
}

void DrawBars::paintEvent(QPaintEvent* event)
{
  QWidget::paintEvent(event);
  QPainter painter;
  painter.begin(this);
//  painter.setRenderHint(QPainter::NonCosmeticDefaultPen);
  
  painter.fillRect(rect(), pImpl->c_back_brush);
  
  painter.setRenderHint(QPainter::Antialiasing, false);
//  painter.setRenderHint(QPainter::Antialiasing, false);
//  painter.setRenderHint(QPainter::TextAntialiasing, false);
//  painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
//  painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
//  painter.setRenderHint(QPainter::NonCosmeticDefaultPen, false);
//  painter.setRenderHint(QPainter::Qt4CompatiblePainting, false);
  
//  QPen pp = painter.pen();
//  pp.setWidth(0);
//  pp.setCapStyle(Qt::FlatCap);
//  pp.setJoinStyle(Qt::MiterJoin);
//  pp.setCosmetic(false);
////  pp.setStyle(Qt::DashLine);
//  pp.setStyle(Qt::SolidLine);
//  painter.setPen(pp);
//  painter.drawPoint
  
  
  painter.setPen(pImpl->c_front_pen);
  painter.setBrush(pImpl->c_front_brush);
  painter.translate(QPoint(pImpl->c_margins.left(), pImpl->c_margins.top()));
  for (int d=0; d<4; d++)
    for (int i=0; i<pImpl->elems[d].count(); i++)
    {
      if (pImpl->elems[d][i].visible)
        pImpl->elems[d][i].pme->draw(painter);
    }
  painter.end();
}

bool DrawBars::event(QEvent* ev)
{
  if (ev->type() == QEvent::PaletteChange)
  {
    if (isDrawPaletteCP(pImpl->clr_policy))
      ;
    else if (pImpl->clr_policy == CP_WHITE)
      pImpl->updatePalette(this, QColor(Qt::white));
    else if (pImpl->clr_policy == CP_BLACK)
      pImpl->updatePalette(this, QColor(Qt::black));
    else if (pImpl->clr_policy == CP_DEFAULT) /// not-strict movement! for not to lose -1 (manual) color_policy
      pImpl->upbackPalette(this);
  }
  return QWidget::event(ev);
}

void DrawBars::connectScrollBar(QScrollBar* qsb, bool staticView, bool setOrientation)
{
  pDraw->connectScrollBar(qsb, staticView, setOrientation);
  QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(scrollDataTo(int)));
}

void DrawBars::slot_setScalingH(int sh){  pDraw->slot_setScalingH(sh);  }
void DrawBars::slot_setScalingV(int sv){  pDraw->slot_setScalingV(sv);  }

void DrawBars::slot_setBounds(float low, float high){ pDraw->slot_setBounds(low, high); slot_updatedBounds(); }
void DrawBars::slot_setBoundLow(float v){  pDraw->slot_setBoundLow(v); slot_updatedBounds(); }
void DrawBars::slot_setBoundHigh(float v){  pDraw->slot_setBoundHigh(v); slot_updatedBounds();  }
void DrawBars::slot_setContrast(float k, float b){  pDraw->slot_setContrast(k, b);  slot_updatedBounds();  }
void DrawBars::slot_setContrastK(float v){  pDraw->slot_setContrastK(v);  slot_updatedBounds(); }
void DrawBars::slot_setContrastB(float v){  pDraw->slot_setContrastB(v);  slot_updatedBounds(); }
void DrawBars::slot_setDataTextureInterpolation(bool v){  pDraw->slot_setDataTextureInterpolation(v);  }
void DrawBars::slot_setDataPalette(const IPalette* v){  pDraw->slot_setDataPalette(v); slot_updatedDataPalette();  }
void DrawBars::slot_setDataPaletteDiscretion(bool v){  pDraw->slot_setDataPaletteDiscretion(v);  }
void DrawBars::slot_setData(const float* v){  pDraw->slot_setData(v);  }
void DrawBars::slot_setData(const QVector<float>& v){  pDraw->slot_setData(v);  }
void DrawBars::slot_fillData(float v){  pDraw->slot_fillData(v);  }
void DrawBars::slot_clearData(){  pDraw->slot_clearData();  }

void DrawBars::slot_setMirroredHorz(){  pDraw->slot_setMirroredHorz(); slot_updatedOrientation();  }
void DrawBars::slot_setMirroredVert(){  pDraw->slot_setMirroredVert(); slot_updatedOrientation(); }
void DrawBars::slot_setPortionsCount(int count){  pDraw->slot_setPortionsCount(count);  }

void DrawBars::slot_enableAutoUpdate(bool v){  pDraw->slot_enableAutoUpdate(v);  }
void DrawBars::slot_disableAutoUpdate(bool v){  pDraw->slot_disableAutoUpdate(v);  }
void DrawBars::slot_enableAutoUpdateByData(bool v){  pDraw->slot_enableAutoUpdateByData(v);  }
void DrawBars::slot_disableAutoUpdateByData(bool v){  pDraw->slot_disableAutoUpdateByData(v);  }

void DrawBars::slot_swapBarsLR(){   swapBars(AT_LEFT);  }
void DrawBars::slot_swapBarsTB(){   swapBars(AT_TOP);   }

void DrawBars::slot_updatedDataPalette()
{
  if (isDrawPaletteCP(pImpl->clr_policy))
  {
    const IPalette* ppal = pDraw->dataPalette();
    unsigned int  fc = ppal->first(), lc = ppal->last();
    if (pImpl->clr_policy == CP_FROM_DRAWPALETTE_INV)
      qSwap(fc, lc);
    else if (pImpl->clr_policy == CP_FROM_DRAWBACK)
      fc = pDraw->colorBack();
    pImpl->updatePalette(this, QColor(COLOR3(fc)), QColor(COLOR3(lc)));
  }
}

void DrawBars::slot_updatedBounds()
{
  bounds_t bnd = pDraw->bounds();
  int cnt = pImpl->elemsBoundDepended.count();
  if (cnt)
  {
    for (int i=0; i<cnt; i++)
    {
      pImpl->elemsBoundDepended[i]->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bnd));
    }
    update();
  }
}

void DrawBars::slot_updatedOrientation()
{
  bool oldMH = pImpl->c_mirroredHorz, oldMV = pImpl->c_mirroredVert;
  ORIENTATION orient = pDraw->orientation();
  pImpl->c_mirroredHorz = orientationMirroredHorz(orient);
  pImpl->c_mirroredVert = orientationMirroredVert(orient);
  if (pImpl->c_mirroredHorz != oldMH || pImpl->c_mirroredVert != oldMV)
  {
    pImpl->reupdateBars(MarginElement::UF_FORCED);
    update();
  }
}

void DrawBars::scrollDataTo(int)
{
  int cnt = pImpl->elemsScrollDepended.count();
  if (cnt)
  {
    int sv = pDraw->scrollValue();
    for (int i=0; i<cnt; i++)
    {
      pImpl->elemsScrollDepended[i]->setRelatedOffset(sv);
//      pImpl->elemsBoundDepended[i]->relatedTextup();
    }
    update();
  }
}


static void rectAlign(const QRect& area, Qt::Alignment alignment, QPoint* result, QPointF* sign)
{
  QPoint pt;  QPointF sg;  //QPointF sp;
  if (alignment == Qt::AlignCenter)
  {
    pt = area.center();
    sg = QPointF(-0.5, -0.5);
  }
  else if (alignment & Qt::AlignLeft)
  {
    if (alignment & Qt::AlignTop)
    {
      pt = QPoint(area.left(), area.top());
      sg = QPointF(0, 0);
    }
    else if (alignment & Qt::AlignBottom)
    {
      pt = QPoint(area.left(), area.bottom());
      sg = QPointF(0, -1);
    }
    else
    {
      pt = QPoint(area.left(), area.center().y());
      sg = QPointF(0, -0.5);
    }
  }
  else if (alignment & Qt::AlignRight)
  {
    if (alignment & Qt::AlignTop)
    {
      pt = QPoint(area.right(), area.top());
      sg = QPointF(-1, 0);
    }
    else if (alignment & Qt::AlignBottom)
    {
      pt = QPoint(area.right(), area.bottom());
      sg = QPointF(-1, -1);
    }
    else
    {
      pt = QPoint(area.right(), area.center().y());
      sg = QPointF(-1, -0.5);
    }
  }
  else
  {
    if (alignment & Qt::AlignTop)
    {
      pt = QPoint(area.center().x(), area.top());
      sg = QPointF(-0.5, 0);
    }
    else if (alignment & Qt::AlignBottom)
    {
      pt = QPoint(area.center().x(), area.bottom());
      sg = QPointF(-0.5, -1);
    }
    else
    {
      pt = area.center();
      sg = QPointF(-0.5, -0.5);
    }
  }
  *result = pt; *sign = sg;
}


/************************************************************************************************/


void MEQWrapper::remoteUpdate()
{
  m_premote->update();
}

void MEQWrapper::remoteRebound()
{
  m_premote->elemSizeHintChanged(m_pme);
  m_premote->update();
}

MEQWrapper::MEQWrapper(): m_pme(nullptr), m_premote(nullptr)
{
}

MEQWrapper::~MEQWrapper()
{
}

void MEQWrapper::remove()
{
  m_premote->retrieveMElement(this, false);
}

void MEQWrapper::removeAndLeftSpace()
{
  m_premote->retrieveMElement(this, true);
}

void MEQWrapper::setVisible(bool visible)
{
  m_premote->setVisible(this, visible);
}

void MEQWrapper::moveToAnotherSide()
{
  m_premote->switchToAnotherSide(this);
}

void MEQWrapper::changeColor(const QColor& clr)
{
  m_pme->changeColor(clr);
  remoteUpdate();
}



/////////////////////////////////////////////////////////////////////////////////////


void MEQWTexted::setFont(const QFont& font)
{
  ((MarginLabel*)m_pme)->setFont(font);
  remoteUpdate();
}


void MEWLabel::setText(const QString& text)
{
  ((MarginLabel*)m_pme)->setText(text);
  remoteUpdate();
}


void MEWSpace::setSpace(int space)
{
  ((MarginSpace*)m_pme)->setSpace(space);
  remoteRebound();
}



/***/
  class MEPointerOProactive: public DrawOverlayProactive
  {
    MarginPointer*  m_ptr;
    DrawBars*       m_premote;
  public:
    MEPointerOProactive(MarginPointer* ptr, DrawBars* premote): m_ptr(ptr), m_premote(premote)
    {   if (m_premote) m_premote->update(); }
    MEPointerOProactive(MarginPointer* ptr, DrawBars* premote, float f0x, float f0y): m_ptr(ptr), m_premote(premote)
    {   m_ptr->setPosition(f0x, f0y); if (m_premote) m_premote->update(); }
    virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE oreact, const void *dataptr, bool*)
    {
      if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
      {
        m_ptr->setPosition(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
        if (m_premote)  m_premote->update();
      }
      else if (oreact == ORM_RMPRESS)
      {
        m_ptr->setVisible(false);
        if (m_premote)  m_premote->update();
      }
      return true;
    }
  };

DrawOverlayProactive* MEWPointer::createProactive()
{
  return new MEPointerOProactive((MarginPointer*)m_pme, m_premote);
}

DrawOverlayProactive* MEWPointer::createProactive(float start_x, float start_y)
{
  return new MEPointerOProactive((MarginPointer*)m_pme, m_premote, start_x, start_y);
}

void MEWPointer::setPosition(float pos01)
{
  ((MarginPointer*)m_pme)->setPosition(pos01);
//  remoteRebound();
  remoteUpdate();
}

/////////////////////////////////////////////////////////////////////////////////////

void MEWScale::setFont(const QFont& font)
{
  ((MarginMarksTexted*)m_pme)->setFont(font);
//  remoteUpdate();
  remoteRebound();
}

void MEWScale::setMarkLen(int mlen)
{
  ((MarginMarks*)m_pme)->setMarkLength(mlen);
  remoteRebound();
}

void MEWScale::setMarkMiniLen(int mlen)
{
  ((MarginMarks*)m_pme)->setMarkMiniLength(mlen);
  remoteRebound();
}

//void MEWScale::scroll(int offset)
//{
//  ((MarginMarksTexted*)m_pme)->setRelatedOffset(offset, true);
//  remoteUpdate();
//}

void MEWScaleNN::setBounds(float LL, float HL)
{
  ((MarginMarksTexted*)m_pme)->setRelatedOpts(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL)));
  remoteUpdate();
}

void MEWScaleTAP::tap()
{
  ((MarginMarksTexted*)m_pme)->relatedTextup();
  remoteUpdate();
}
