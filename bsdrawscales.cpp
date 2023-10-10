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

#include <QFrame>
#include <QDoubleSpinBox>
#include <cfloat>

enum
{
  /// 1. marks mode
  DBMODE_STRETCHED_POW2=1,
  DBMODE_STRETCHED=2,
  DBMODE_STATIC=3
};

enum  {   SPACING_AFTERWIDTH=6, SPACING_AFTERHEIGHT=4 };


MarginElement::~MarginElement()
{
}

typedef void  (*posfunc_fn)(struct texts_t* t);

struct        texts_t
{
  QStaticText uin_text;
  QSize       uin_locsize;
  int         visible;
  
  int         pos_gap;
  QPoint      pos_mark;
  QPoint      pos_mark_dopdock;
  posfunc_fn  pos_update_fn;
  QPoint      c_pos_result;
  texts_t(): visible(0), pos_update_fn(nullptr) {}
};

struct        memark_t
{
  QPoint      anchor;
  QRect       rect;
};

inline void  assignText(texts_t* tt, const QString& text)
{
  tt->uin_text.setText(text);
  tt->uin_locsize = tt->uin_text.size().toSize();
  if (tt->pos_update_fn)
    tt->pos_update_fn(tt);
}

inline void  assignText(texts_t* tt, const QString& text, const QFont& m_font)
{
  assignText(tt, text);
  tt->uin_text.prepare(QTransform(), m_font);
}

inline void  assignText(texts_t* tt, const QString& text, bool m_fontReplaced, const QFont& m_font)
{
  assignText(tt, text);
  if (m_fontReplaced)
    tt->uin_text.prepare(QTransform(), m_font);
}

static const bool   BAR_VERT[] = { true, true, false, false };

template <int n>
void  posfunc_nn_mid(texts_t* t);

template <>   void  posfunc_nn_mid<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->pos_gap + t->uin_locsize.width(), t->uin_locsize.height()/2 + 1);  }
template <>   void  posfunc_nn_mid<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(-t->pos_gap, t->uin_locsize.height()/2 + 1);  }
template <>   void  posfunc_nn_mid<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width()/2, t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nn_mid<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width()/2, -t->pos_gap);  }

template <int n>
void  posfunc_nn_up(texts_t* t);

template <>   void  posfunc_nn_up<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->pos_gap + t->uin_locsize.width(), t->uin_locsize.height() + 1);  }
template <>   void  posfunc_nn_up<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(-t->pos_gap, t->uin_locsize.height() + 1);  }
template <>   void  posfunc_nn_up<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(/*t->uin_locsize.width()/2*0*/1, t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nn_up<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(/*t->uin_locsize.width()/2*0*/1, -t->pos_gap);  }

template <int n>
void  posfunc_nn_down(texts_t* t);

template <>   void  posfunc_nn_down<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->pos_gap + t->uin_locsize.width(), /*0*t->uin_locsize.height()/2 + */1);  }
template <>   void  posfunc_nn_down<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(-t->pos_gap, /*t->uin_locsize.height()/2 + */1);  }
template <>   void  posfunc_nn_down<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width(), t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nn_down<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width(), -t->pos_gap);  }

static posfunc_fn  POSFUNC_NN_MID[4]   = { posfunc_nn_mid<AT_LEFT>, posfunc_nn_mid<AT_RIGHT>, posfunc_nn_mid<AT_TOP>, posfunc_nn_mid<AT_BOTTOM> };
static posfunc_fn  POSFUNC_NN_UP[4]    = { posfunc_nn_up<AT_LEFT>, posfunc_nn_up<AT_RIGHT>, posfunc_nn_up<AT_TOP>, posfunc_nn_up<AT_BOTTOM> };
static posfunc_fn  POSFUNC_NN_DOWN[4]  = { posfunc_nn_down<AT_LEFT>, posfunc_nn_down<AT_RIGHT>, posfunc_nn_down<AT_TOP>, posfunc_nn_down<AT_BOTTOM> };

template <int n>
void  posfunc_nm_mid(texts_t* t);

template <>   void  posfunc_nm_mid<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->pos_gap + t->uin_locsize.width(), t->uin_locsize.height()/2 + 1);  }
template <>   void  posfunc_nm_mid<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(-t->pos_gap, t->uin_locsize.height()/2 + 1);  }
template <>   void  posfunc_nm_mid<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width()/2, t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nm_mid<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark - QPoint(t->uin_locsize.width()/2, -t->pos_gap);  }

template <int n>
void  posfunc_nm_up(texts_t* t);

template <>   void  posfunc_nm_up<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(t->pos_gap + t->uin_locsize.width(), t->uin_locsize.height());  }
template <>   void  posfunc_nm_up<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(-t->pos_gap, t->uin_locsize.height());  }
template <>   void  posfunc_nm_up<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(0, t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nm_up<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(0, -t->pos_gap);  }

template <int n>
void  posfunc_nm_down(texts_t* t);

template <>   void  posfunc_nm_down<AT_LEFT>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(t->pos_gap + t->uin_locsize.width(), 0);  }
template <>   void  posfunc_nm_down<AT_RIGHT>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(-t->pos_gap, 0);  }
template <>   void  posfunc_nm_down<AT_TOP>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(t->uin_locsize.width(), t->pos_gap + t->uin_locsize.height());  }
template <>   void  posfunc_nm_down<AT_BOTTOM>(texts_t* t)
{ t->c_pos_result = t->pos_mark_dopdock - QPoint(t->uin_locsize.width(), -t->pos_gap);  }

static posfunc_fn  POSFUNC_NM_MID[4]   = { posfunc_nm_mid<AT_LEFT>, posfunc_nm_mid<AT_RIGHT>, posfunc_nm_mid<AT_TOP>, posfunc_nm_mid<AT_BOTTOM> };
static posfunc_fn  POSFUNC_NM_UP[4]    = { posfunc_nm_up<AT_LEFT>, posfunc_nm_up<AT_RIGHT>, posfunc_nm_up<AT_TOP>, posfunc_nm_up<AT_BOTTOM> };
static posfunc_fn  POSFUNC_NM_DOWN[4]  = { posfunc_nm_down<AT_LEFT>, posfunc_nm_down<AT_RIGHT>, posfunc_nm_down<AT_TOP>, posfunc_nm_down<AT_BOTTOM> };




typedef bool  (*cross_fn)(texts_t* t, int d);

template <int n>
bool  crossing_up(texts_t* t, int d);

template <>   bool  crossing_up<AT_LEFT>(texts_t* t, int d)
{ return t->c_pos_result.y() + t->uin_locsize.height()/2 > d;  }
template <>   bool  crossing_up<AT_RIGHT>(texts_t* t, int d)
{ return t->c_pos_result.y() + t->uin_locsize.height()/2 > d;  }
template <>   bool  crossing_up<AT_TOP>(texts_t* t, int d)
{ return t->c_pos_result.x() + t->uin_locsize.width()/2 > d; }
template <>   bool  crossing_up<AT_BOTTOM>(texts_t* t, int d)
{ return t->c_pos_result.x() + t->uin_locsize.width()/2 > d;  }

template <int n>
bool  crossing_down(texts_t* t, int d);

template <>   bool  crossing_down<AT_LEFT>(texts_t* t, int d)
{ return t->c_pos_result.y() - t->uin_locsize.height()/2 < d;  }
template <>   bool  crossing_down<AT_RIGHT>(texts_t* t, int d)
{ return t->c_pos_result.y() - t->uin_locsize.height()/2 < d;  }
template <>   bool  crossing_down<AT_TOP>(texts_t* t, int d)
{ return t->c_pos_result.x() - t->uin_locsize.width()/2 < d;  }
template <>   bool  crossing_down<AT_BOTTOM>(texts_t* t, int d)
{ return t->c_pos_result.x() - t->uin_locsize.width()/2 < d;  }

static cross_fn  CROSS_UP[4]    = { crossing_up<AT_LEFT>, crossing_up<AT_RIGHT>, crossing_up<AT_TOP>, crossing_up<AT_BOTTOM> };
static cross_fn  CROSS_DOWN[4]  = { crossing_down<AT_LEFT>, crossing_down<AT_RIGHT>, crossing_down<AT_TOP>, crossing_down<AT_BOTTOM> };


class DSNumFormatter
{
  enum RFM {  RFM_FLOATING, RFM_INTEGER, RFM_INTEGER_ROUNDED  };
  RFM     rfm;
  
  int     precision_ge1, precision_l1;
  char    letter_ge1, letter_l1;
  bool    nonamed;
  
  int     precision_offset;
  bool    useThreshold;
  double  threshold_ge;
  int     precision_gethr;
  char    letter_gethr;
public:
  DSNumFormatter(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, bool meganamed=false):
    rfm(RFM_FLOATING), nonamed(!meganamed), precision_offset(0), useThreshold(false)
  {
    setFloating(symb_ge1, precis_ge1, symb_l1, precis_l1);
  }
  DSNumFormatter():
    rfm(RFM_FLOATING), nonamed(true), precision_offset(0), useThreshold(false)
  {
    setFloating('f', 1, 'g', 3, 100000.0, 'g', 2);
  }
  void  setFloating(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1)
  {
    rfm = RFM_FLOATING;
    letter_ge1 = symb_ge1;
    precision_ge1 = precis_ge1 + precision_offset; if (precision_ge1 < 0)  precision_ge1 = 0;
    letter_l1 = symb_l1;
    precision_l1 = precis_l1 + precision_offset; if (precision_l1 < 0)  precision_l1 = 0;
  }
  void  setFloating(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, double thresh_ge, char symb_gethr, int precis_gethr)
  {
    setFloating(symb_ge1, precis_ge1, symb_l1, precis_l1);
    useThreshold = true;
    threshold_ge = thresh_ge;
    letter_gethr = symb_gethr;
    precision_gethr = precis_gethr + precision_offset; if (precision_gethr < 0)  precision_gethr = 0;
  }
  void  setFloatingStrict(int pp){ rfm = RFM_FLOATING; precision_ge1 = pp; precision_l1 = pp; precision_gethr = pp; precision_offset = 0; }
  
  void  setPrecisionOffset(int po){ precision_offset = po;  }
  
  void  setInteger(){ rfm = RFM_INTEGER; }
  void  setIntegerRounded(){ rfm = RFM_INTEGER_ROUNDED; }
  
  int   maxPrecision() const
  {
    if (rfm != RFM_FLOATING)
      return 0;
    
    int result = precision_offset + (precision_l1 > precision_ge1? precision_l1 : precision_ge1);
    return result < 0? 0 : result;
  }
  
  void  setFloatingAuto(float ll, float hl)
  {
    float diap = qAbs(hl - ll);
    if (diap >= 1000.0f)
      setFloating('f', 1, 'g', 2);
    else if (diap > 10.0f || ll >= 1.0f || ll <= -1.0f)
      setFloating('f', 1, 'g', 2);
    else if (diap > 1.0f)
      setFloating('f', 1, 'g', 2);  // add count analysis
    else if (diap > 0.1f)
      setFloating('f', 2, 'f', 2);
    else
      setFloating('f', 3, 'f', 3);
  }
  
  template <typename T>
  QString operator()(T value_original) const
  {
    if (rfm >= RFM_INTEGER)
    {
      long int value = rfm == RFM_INTEGER_ROUNDED? (long int)qRound64(value_original) : (long int)(value_original);
      if (nonamed)
        return QString::number(value);
      long int valuep = value < 0? -value : value;
      static const char letters[] = { ' ', ' ',  'K',  'M',  'G',  'T',   'P',   'E',   'Z',   'Y',   ' '  };
      long int mlt = 1;
      for (unsigned int i=0; i<sizeof(letters)/sizeof(const char); i++)
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
      double value = double(value_original);
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
          if (texts[iprev].c_pos_result.y() + texts[iprev].uin_locsize.height() + 1 > texts[i].c_pos_result.y())
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
          if (texts[iprev].c_pos_result.y() < texts[i].c_pos_result.y() + texts[i].uin_locsize.height() + 1)
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
          if (texts[iprev].c_pos_result.x() < texts[i].c_pos_result.x() + texts[i].uin_locsize.width() + 4)
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
          if (texts[iprev].c_pos_result.x() + texts[iprev].uin_locsize.width() + 4 > texts[i].c_pos_result.x())
            texts[i].visible = 2;
          else
            iprev = i;
        }
    }
  }
  
  if (backward)
  {
    if (iprev > 0 && texts[countMaxiTotal-1].visible == 2)
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
  RF_NONE,  
  RF_SETBOUNDS, RF_SETENUMERATE, RF_SETTAPS, RF_SETTAPWDG,
  RF_UPBOUNDS, RF_UPTAPS
};

union relatedopts_t
{
  struct
  {
    float         LL, HL;
    float         MOD;
  }               rel_fixed;
  struct
  {
    mtap_qstring_fn  tapfn;
    const void*     param;
    int              slen;
  }               rel_tap_qstring;
  struct
  {
//    int       step;
    int       numcount;
    int       recycle;
    int       enfrom;
  }               rel_enumerate;
  
  relatedopts_t(){}
  relatedopts_t(const bounds_t bnd, float mod=0.0f){  rel_fixed.LL = bnd.LL; rel_fixed.HL = bnd.HL; rel_fixed.MOD = mod; }
  relatedopts_t(mtap_qstring_fn tapfn, const void* param, int maxlen){  rel_tap_qstring.tapfn = tapfn; rel_tap_qstring.param = param; rel_tap_qstring.slen = maxlen; /*rel_tap_qstring.sstr.reserve(maxlen); */ }
  relatedopts_t(int numcount, int enfrom, int recycle){  rel_enumerate.numcount = numcount;  rel_enumerate.enfrom = enfrom;  rel_enumerate.recycle = recycle; }
};

class MarginCallback
{
  int                     c_rtexttype;
  relatedopts_t           c_rdata;
  float                   c_reloffset;
  bool                    c_updatesetDimm;
protected:
  struct  tapcontent_t
  {
    int   lmoffset, lmardimm;
  } tapctt;
public:
  virtual ~MarginCallback();
  int cachedRTexttype() const { return c_rtexttype; }
  const relatedopts_t& cachedRdata() const { return c_rdata; }
public:
  bool bdContentUpdate(bool forcedupdate=true)
  {
    return bdContentUpdate(c_rtexttype, c_rdata, forcedupdate);
  }
  bool bdContentUpdate(int rtexttype, const relatedopts_t& rdata, bool forcedupdate=false)
  {
    bool docall = (rtexttype != c_rtexttype) | forcedupdate;
    c_rtexttype = rtexttype;
    if (rtexttype == RF_SETBOUNDS || rtexttype == RF_UPBOUNDS)
    {
      docall |= rtexttype == RF_SETBOUNDS ? true : 
                    (rdata.rel_fixed.LL != c_rdata.rel_fixed.LL) || (rdata.rel_fixed.HL != c_rdata.rel_fixed.HL);
      if (docall)
      {
        c_rdata.rel_fixed.LL = rdata.rel_fixed.LL;
        c_rdata.rel_fixed.HL = rdata.rel_fixed.HL;
        if (rtexttype == RF_SETBOUNDS)
          c_rdata.rel_fixed.MOD = rdata.rel_fixed.MOD;
        
        float ll = c_rdata.rel_fixed.LL + (c_rdata.rel_fixed.HL - c_rdata.rel_fixed.LL)*c_reloffset;
        float hl = c_rdata.rel_fixed.HL + (c_rdata.rel_fixed.HL - c_rdata.rel_fixed.LL)*c_reloffset;
        bdContentUpdateBounds(ll, hl, c_rdata.rel_fixed.MOD);
      }
    }
    else if (rtexttype == RF_SETTAPS || rtexttype == RF_UPTAPS)
    {
      docall = true;
      if (rtexttype == RF_SETTAPS)
        c_rdata = rdata;
      else
        c_rdata.rel_tap_qstring.param = rdata.rel_tap_qstring.param;
      QString sstr(c_rdata.rel_tap_qstring.slen, Qt::Uninitialized);
      bdContentUpdateTaps(sstr, c_rdata.rel_tap_qstring.tapfn, c_rdata.rel_tap_qstring.param, c_reloffset, tapctt);
    }
    else if (rtexttype == RF_SETENUMERATE)
    {
      docall |= (rdata.rel_enumerate.enfrom != c_rdata.rel_enumerate.enfrom) || (rdata.rel_enumerate.numcount != c_rdata.rel_enumerate.numcount);
      if (docall)
      {
        c_rdata = rdata;
        bdContentUpdateEnumerate(rdata.rel_enumerate.enfrom, rdata.rel_enumerate.numcount, rdata.rel_enumerate.recycle, c_reloffset);
      }
    }
    return docall;
  }
protected:
  virtual void  bdContentUpdateBounds(float LL, float HL, float MOD=0)=0;
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn, const void* param, float relatedoffset, const tapcontent_t&)=0;
  virtual void  bdContentUpdateEnumerate(int from, int count, int recycle, float relatedoffset)=0;
public:
  MarginCallback(): c_reloffset(0), c_updatesetDimm(false)
  {
    c_rtexttype = RF_NONE;
    c_rdata.rel_fixed.LL = c_rdata.rel_fixed.HL = 0;
    c_rdata.rel_enumerate.enfrom = c_rdata.rel_enumerate.numcount = -1;
    tapctt.lmoffset = 0;
    tapctt.lmardimm = 1;
  }
  
  void    setUpdateOnSetDimm(bool ups){ c_updatesetDimm = ups; }
  
  void    setRelatedOffset(int offset, bool autocallupdates=true)
  {
    tapctt.lmoffset = offset;
    if (tapctt.lmardimm != 0)     // ntf??
    {
      float reloffset = tapctt.lmoffset / tapctt.lmardimm;
      if (c_reloffset != reloffset)
      {
        c_reloffset = reloffset;
        if (autocallupdates)
          bdContentUpdate(c_rtexttype, c_rdata, autocallupdates);
      }
    }
  }
  void    storeDimm(int dimm/*, bool autocallupdates=false*/)
  {
    if (dimm != 0)    // ntf??
    {
      tapctt.lmardimm = dimm;
      c_reloffset = tapctt.lmoffset / tapctt.lmardimm;
//      if (autocallupdates)
//        bdContentUpdate(c_rtexttype, c_rdata, autocallupdates);
      if (c_updatesetDimm && dimm > 0)
        bdContentUpdate(c_rtexttype, c_rdata, c_updatesetDimm);
    }
  }
//                                c_rtexttype == RF_SETTAPS? c_rdata.rel_tap_qstring.slen : 0; }
};
MarginCallback::~MarginCallback(){}


class MarginTextformat
{
private:
  DSNumFormatter          numfmt;
  bool                    numfmt_locked;
  char*                   prefix, *postfix;
  char                    prefix_array[12], postfix_array[12];
  int                     c_prefix_len, c_postfix_len;
  int                     postfix_pex;
protected:
  QFont                   m_font;
  bool                    m_fontReplaced;
public:
  enum    {   PEX_FIRST=1, PEX_LAST=2, PEX_ALL=1|2|4  };
  MarginTextformat(): numfmt_locked(false), 
                      prefix(nullptr), postfix(nullptr), c_prefix_len(0), c_postfix_len(0), postfix_pex(PEX_ALL),
                      m_fontReplaced(false) {}
  MarginTextformat(const QFont& fnt):  numfmt_locked(false), 
                      prefix(nullptr), postfix(nullptr), c_prefix_len(0), c_postfix_len(0), postfix_pex(PEX_ALL),
                      m_font(fnt), m_fontReplaced(true) {}
  virtual ~MarginTextformat();
public:
//  int   precision() const { return numfmt.precision(); }
//  int rmaxlen() const {  return c_rtexttype == RF_SETBOUNDS? 7 + numfmt_locked*2 : c_rtexttype == RF_SETENUMERATE? /*c_rdata.rel_enumerate.numcount*/7 + numfmt_locked*2 :
//                                c_rtexttype == RF_SETTAPS? c_rdata.rel_tap_qstring.slen : 0; }
  
  int   precision(){  return numfmt.maxPrecision(); }
  int   max_symbols() const { return c_prefix_len + 3 + (numfmt.maxPrecision()) + c_postfix_len; }
public:
  enum  FMT { FMT_X0, FMT_X2, FMT_X4 };
  void    numformatUpdate(float LL, float HL)
  {
    if (numfmt_locked == false)
      numfmt.setFloatingAuto(LL, HL);
  }
  void    numformatOffset(int pp)
  {
    numfmt.setPrecisionOffset(pp);
  }
  void    numformatFix(int pp)
  { 
    numfmt_locked = true;
    numfmt.setFloatingStrict(pp);
  }
  
//  void    setFormat(char symb_ge1, int precis_ge1, char symb_l1, int precis_l1, bool meganamed=false)
//  {
//    numfmt_locked = false;
//    numfmt.setFormat(symb_ge1, precis_ge1, symb_l1, precis_l1, meganamed);
//  }
  void    numformatInteger(bool rounded)
  {
    numfmt_locked = true;
    rounded? numfmt.setIntegerRounded() : numfmt.setInteger();
  }
  
  void    setPrefix(const char* str)
  {
    if (str == nullptr) prefix = nullptr; 
    else
    {
      for (c_prefix_len=0; str[c_prefix_len] != '\0'; c_prefix_len++)
        prefix_array[c_prefix_len] = str[c_prefix_len];
      prefix_array[c_prefix_len] = '\0';
      prefix = prefix_array;
    }
  }
  void    setPostfix(const char* str)
  {
    if (str == nullptr) postfix = nullptr;
    else
    {
      for (c_postfix_len=0; str[c_postfix_len] != '\0'; c_postfix_len++)
        postfix_array[c_postfix_len] = str[c_postfix_len];
      postfix_array[c_postfix_len] = '\0';
      postfix = postfix_array;
    }
  }
  void    setPostfixPositions(int PEX)
  {
    postfix_pex = PEX;
  }
public:
  void  setFont(const QFont& fnt)
  {
    m_font = fnt;
    m_fontReplaced = true;
    tfContentUpdate();
  }
  const QFont&  font() const { return m_font; }
  QFont&        font() { return m_font; }
protected:
  virtual   void    tfContentUpdate()=0;
protected:
  void      turnPrefix(bool on)
  {
    prefix = on && c_prefix_len? prefix_array : nullptr;
  }
  void      turnPostfix(bool on)
  {
    postfix = on && c_postfix_len? postfix_array : nullptr;
  }
  bool      postfixLimited() const { return postfix_pex != PEX_ALL; }
  void      turnPostfixByPosition(int pos, int count)
  {
    bool on = postfix_pex == PEX_ALL ||
              (pos == 0 && postfix_pex & PEX_FIRST) ||
              (pos == count-1 && postfix_pex & PEX_LAST);
    turnPostfix(on);
  }
  QString&  redact(QString& sstr) const
  {
    if (prefix)   sstr = prefix + sstr;
    if (postfix)  sstr = sstr + postfix;
    return sstr;
  }
  QString   redact(const QString& sstr) const
  {
    QString result(sstr);
    if (prefix)   result = prefix + result;
    if (postfix)  result = result + postfix;
    return result;
  }
  QString   redact(float value) const
  {
    return redact(numfmt(value));
  }
  QString   redact(double value) const
  {
    return redact(numfmt(value));
  }
};
MarginTextformat::~MarginTextformat(){}


/********************************************************************************************************************/
/********************************************************************************************************************/
/***************************      SPACES&LABELS                             *****************************************/
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
  ~MarginSpace();
  void  setSpace(int newspace)
  {
    space = newspace;
  }
protected:
  virtual bool  updateArea(const uarea_t&, int){ return true; }
  virtual void  draw(QPainter&){}
  virtual void  sizeHint(ATTACHED_TO, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = space;
    *minsegm_main = *minsegm_pre = *minsegm_post = 0;
  }
  virtual void  changeColor(const QColor&){}
};
MarginSpace::~MarginSpace(){}


class MarginElementCached: public MarginElement
{
protected:
  friend class DrawBars;
private:
  uarea_t     cached_area;
  bool        filled;
protected:
  bool            mec_filled() const { return filled; }
  const uarea_t&  mec_area() const { return cached_area; }
protected:
  virtual void  draw(QPainter&){}
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (UPDATEFOR == UF_RESIZE)
    {
      filled = true;
      cached_area = area;
    }
    return true;
  }
  
protected:
  QColor        c_color;
  bool          c_color_redefined;
  virtual void  changeColor(const QColor& clr){  c_color = clr; c_color_redefined = true; }
public:
  MarginElementCached(): c_color_redefined(false)
  {
    filled = false;
  }
  MarginElementCached(QColor color): c_color(color), c_color_redefined(true)
  {
    filled = false;
  }
  ~MarginElementCached();
};

MarginElementCached::~MarginElementCached(){}


class MarginColoredSpace: public MarginElementCached
{
  int     space;
  QColor  color;
  QRect   dtrt;
  bool    useAllZone;
public:
  MarginColoredSpace(int space, QColor color, bool maxzone=true): MarginElementCached(color), useAllZone(maxzone) { this->space = space; }
  ~MarginColoredSpace();
  void  setSpace(int newspace){ space = newspace; }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    int d1 = useAllZone? 0 : area.segm_pre, d2 = (useAllZone? area.segment_full : area.segment_full - area.segm_post)-1;
    if (area.atto == AT_LEFT)
      dtrt.setCoords(area.atto_end, d1, area.atto_begin, d2);
    else if (area.atto == AT_RIGHT)
      dtrt.setCoords(area.atto_begin, d1, area.atto_end, d2);
    else if (area.atto == AT_TOP)
      dtrt.setCoords(d1, area.atto_end, d2, area.atto_begin);
    else if (area.atto == AT_BOTTOM)
      dtrt.setCoords(d1, area.atto_begin, d2, area.atto_end);
    return true;
  }
  virtual void  draw(QPainter& painter)
  { 
    painter.fillRect(dtrt, c_color);
  }
  virtual void  sizeHint(ATTACHED_TO, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = space;
    *minsegm_main = *minsegm_pre = *minsegm_post = 0;
  }
};
MarginColoredSpace::~MarginColoredSpace(){}


class MarginContour: public MarginElementCached
{
  int     space;
  int     count;
  QRect   dtrt[3];
  bool    useAllZone;
public:
  MarginContour(int space=0, bool maxzone=true):  count(space == 0? 1 : 3), useAllZone(maxzone) { this->space = space; }
  MarginContour(int space, QColor color, bool maxzone=true): MarginElementCached(color), 
                                                  count(space == 0? 1 : 3), useAllZone(maxzone) { this->space = space; }
  ~MarginContour();
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
    
    int d1 = useAllZone? 0 : area.segm_pre, d2 = (useAllZone? area.segment_full : area.segment_full - area.segm_post)-1;
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
//    MarginElementCached::draw(painter);
    if (c_color_redefined)
      for (int i=0; i<count; i++)
        painter.fillRect(dtrt[i], c_color);
    else
      for (int i=0; i<count; i++)
        painter.fillRect(dtrt[i], painter.brush());
  }
  virtual void  sizeHint(ATTACHED_TO, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = space == 0? 1 : space;
    *minsegm_main = *minsegm_pre = *minsegm_post = 0;
  }
};
MarginContour::~MarginContour(){}

  
class MarginLabel: public MarginElementCached
{
  texts_t         label;
  Qt::Alignment   alignment;
  Qt::Orientation orientation;
  QSize           orient_size;
  
  QTransform      ttransform;
  QPoint          ttranspoint;
  
  bool            useneispace;
  
  QFont           m_font;
  bool            m_fontReplaced;
//protected:
//  int             c_width, c_height;
private:
  void  prepare()
  {
    QSizeF loc = label.uin_text.size();
    label.uin_locsize = QSize(qRound(loc.width()), qRound(loc.height()));
    if (orientation == Qt::Horizontal)
    {
      if (m_fontReplaced)
        label.uin_text.prepare(QTransform(), m_font);
      orient_size = label.uin_locsize;
    }
    else if (orientation == Qt::Vertical)
    {
      ttransform.rotate(-90);
      label.uin_text.prepare(ttransform, m_font);
      orient_size = QSize(label.uin_locsize.height(), label.uin_locsize.width());
    }
//    qDebug()<<label.uin_locsize<<orient_size;
  }
public:
  MarginLabel(const QString& text, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal): 
    alignment(align), orientation(orient), useneispace(useNeighborSpace), m_fontReplaced(false)
  {
    setText(text);
  }
  MarginLabel(const QString& text, const QFont& fnt, bool useNeighborSpace=false, Qt::Alignment  align=Qt::AlignCenter, Qt::Orientation orient=Qt::Horizontal): 
    alignment(align), orientation(orient), useneispace(useNeighborSpace)
  {
    setText(text, fnt);
  }
  ~MarginLabel();
  void  setText(const QString& text)
  {
    label.uin_text.setText(text);
    prepare();
  }
  void  setText(const QString& text, const QFont& fnt)
  {
    label.uin_text.setText(text);
    m_font = fnt;
    m_fontReplaced = true;
    prepare();
  }
  void  setFont(const QFont& fnt)
  {
    m_font = fnt;
    m_fontReplaced = true;
    prepare();
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    int a1 =  area.atto_end;
    int a2 = area.atto_begin;
    int d = useneispace? area.segment_full : area.segm_main;
    int dd = useneispace? 0 : area.segm_pre + 1;
    
    const QRect& actualArea = area.atto == AT_LEFT?  QRect(a1, dd, a2 - a1, d) :
                              area.atto == AT_RIGHT? QRect(a2, dd, a1 - a2, d) :
                              area.atto == AT_TOP?   QRect(dd, a1, d, a2 - a1) :
                                                     QRect(dd, a2, d, a1 - a2);
    
    QPointF textOffsetSign;
    rectAlign(actualArea, alignment/*Qt::AlignLeft | Qt::AlignTop*/, &ttranspoint, &textOffsetSign);
    if (orientation == Qt::Vertical)
      textOffsetSign = QPointF(-(1.0 + textOffsetSign.y()), textOffsetSign.x());
    
    
//    label.pos_mark = 
    label.c_pos_result = QPoint(int(textOffsetSign.x()*(label.uin_locsize.width()-1)), int(textOffsetSign.y()*label.uin_locsize.height()));
    label.visible = 1;
    return true;
  }
  virtual void  draw(QPainter& painter)
  {
//    MarginElementCached::draw(painter);
    if (label.visible)
    {
      painter.save();
      painter.translate(ttranspoint);
      if (m_fontReplaced)
        painter.setFont(m_font);
      if (ttransform.type() != QTransform::TxNone)
        painter.setTransform(ttransform, true);
      painter.drawStaticText(label.c_pos_result, label.uin_text);
      painter.restore();
    }
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    if (BAR_VERT[atto])
    {
      *atto_size = orient_size.width();
      *minsegm_main = orient_size.height();
    }
    else
    {
      *atto_size = orient_size.height();
      *minsegm_main = orient_size.width();
    }
    *minsegm_pre = *minsegm_post = 0;
  }
};
MarginLabel::~MarginLabel(){}


/********************************************************************************************************************/
/********************************************************************************************************************/
/***************************      POINTER                                   *****************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/



class MarginSingle: public MarginElementCached, public MarginCallback
{
  int             round;
  int             mlenmaxi, mwid;
protected:
  bool            relative;
  float           c_LL, c_HL, c_MOD;
  memark_t        mark;
  float           pos_origin;
  float           pos_mark;
public:
  MarginSingle(int rounding, int _marklen, float pos, bool _relative):
    round(rounding), mlenmaxi(_marklen), mwid(_marklen? 1 : 0), 
    relative(_relative), c_LL(0.0f), c_HL(1.0f), c_MOD(0.0f),
    pos_origin(pos)
  {
    _update_single_pos();
  }
  ~MarginSingle();
  int   mlen() const { return mlenmaxi; }
  void  _update_single_pos(){  if (relative) pos_mark = (pos_origin - c_LL)/(c_HL - c_LL);  else  pos_mark = pos_origin; }
  virtual void  setPosition(float pos)
  {
    pos_origin = pos;
    _update_single_pos();
    if (mec_filled())
      _update_single_area(mec_area());
  }
  void    setMarkLength(int length){ mlenmaxi = length; }
  void    setMarkWidth(int width){ mwid = width; /*needRedrawByMark = true;*/ }
  void    setMarkSize(int length, int width){ mlenmaxi = length; mwid = width; /*needRedrawByMark = true;*/ }
protected:
  void _update_single_area(const uarea_t& area)
  {
    const int dimm_main = area.segm_main-1;
    float pm = BAR_VERT[area.atto] ^ area.mirrored? 1.0f - pos_mark : pos_mark;
    int offs = area.segm_pre + (round == 0? qRound(dimm_main*pm) : round == 1? int(dimm_main*pm) : int(dimm_main*pm) + 1);
    
    switch (area.atto)
    {
    case AT_LEFT:
      {
        mark.anchor = QPoint(area.atto_begin, offs);
        mark.rect.setCoords(area.atto_begin - (mlenmaxi-1), offs, area.atto_begin, offs);
        break;
      }
    case AT_RIGHT:
      {
        mark.anchor = QPoint(area.atto_begin, offs);
        mark.rect.setCoords(area.atto_begin, offs, area.atto_begin + (mlenmaxi-1), offs);
        break;
      }
    case AT_TOP:
      {
        mark.anchor = QPoint(offs, area.atto_begin);
        mark.rect.setCoords(offs, area.atto_begin - (mlenmaxi-1), offs, area.atto_begin);
        break;
      }
    case AT_BOTTOM:
      {
        mark.anchor = QPoint(offs, area.atto_begin);
        mark.rect.setCoords(offs, area.atto_begin, offs, area.atto_begin + (mlenmaxi-1));
        break;
      }
    }
//    if (BAR_VERT[area.atto])
//    {
//      float r = round == 0? qRound(i*d3) : round == 1? int(i*d3) : int(i*d3) + 1;
//      int offs = area.segm_pre + (;
//      int offs = area.mirrored? area.segm_pre + dimm_main*pos_mark:
//                                area.segm_pre + dimm_main*(1.0f - pos_mark);
//      int l1 = area.atto_begin;
//      int l2 = area.atto_begin + (area.atto == AT_LEFT? -(mlenmaxi-1) : (mlenmaxi-1));
//      mark.anchor = QPoint(l1, offs);
//      if (area.atto == AT_LEFT)
//        mark.rect.setCoords(l2, offs, l1, offs);
//      else
//        mark.rect.setCoords(l1, offs, l2, offs);
//    }
//    else
//    {
//      int offs = area.mirrored? area.segm_pre + dimm_main*(1.0f - pos_mark):
//                                area.segm_pre + dimm_main*pos_mark;
//      int l1 = area.atto_begin;
//      int l2 = area.atto_begin + (area.atto == AT_TOP? -(mlenmaxi-1) : (mlenmaxi-1));
//      mark.anchor = QPoint(offs, l1);
//      if (area.atto == AT_TOP)
//        mark.rect.setCoords(offs, l2, offs, l1);
//      else
//        mark.rect.setCoords(offs, l1, offs, l2);
//    }
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    
    _update_single_area(area);
    storeDimm(area.segm_main);
    return true;
  }
  virtual void  draw(QPainter& painter)
  {
    MarginElementCached::draw(painter);
    if (mlenmaxi)
    {
      if (c_color_redefined)
        painter.fillRect(mark.rect, c_color);
      else
        painter.fillRect(mark.rect, painter.brush());
    }
  }
  virtual void sizeHint(ATTACHED_TO /*atto*/, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = (mlenmaxi? mlenmaxi + 2 : 0);
    *minsegm_main = *minsegm_pre = *minsegm_post = 0;
  }
  
protected:
  virtual void  bdContentUpdateBounds(float ll, float hl, float mod)
  {
    c_LL = ll;    c_HL = hl;    c_MOD = mod;
    if (relative)
    {
      _update_single_pos();
      if (mec_filled())
        _update_single_area(mec_area());
    }
  }
  virtual void  bdContentUpdateTaps(QString& /*base*/, mtap_qstring_fn /*tapfn*/, const void* /*param*/, float /*relatedoffset*/, const tapcontent_t& /*tctt*/)
  {
  }
  virtual void  bdContentUpdateEnumerate(int from, int count, int /*recycle*/, float /*relatedoffset*/)
  {
    c_LL = from;    c_HL = from + count;    c_MOD = 0.0f;
    if (relative)
    {
      _update_single_pos();
//      pos_mark = c_LL 
      if (mec_filled())
       _update_single_area(mec_area());
    }
  }
};
MarginSingle::~MarginSingle(){}


class MarginPointer: public MarginSingle, public MarginTextformat
{
  ORIENTATION     orient;
  bool            textInnerPlaced;
  texts_t         pointer;
  Qt::Alignment   alignment;
  float           c_value;
  bool            f_assigned;
public:
  MarginPointer(int rounding, int _marklen, float pos, bool _relative, ORIENTATION _orient, bool textinner, Qt::Alignment  align=Qt::AlignCenter/*, Qt::Orientation orient=Qt::Horizontal*/):
    MarginSingle(rounding, _marklen, pos, _relative),
    orient(_orient), textInnerPlaced(textinner), alignment(align), c_value(-1.0f), f_assigned(false)
  {
    _update_pointer_pos(pos_mark);
    pointer.visible = 1;
  }
  MarginPointer(int rounding, int _marklen, float pos, bool _relative, ORIENTATION _orient, bool textinner, const QFont& fnt, Qt::Alignment  align=Qt::AlignCenter/*, Qt::Orientation orient=Qt::Horizontal*/): 
    MarginSingle(rounding, _marklen, pos, _relative), MarginTextformat(fnt),
    orient(_orient), textInnerPlaced(textinner), alignment(align), c_value(-1.0f),  f_assigned(false)
  {
    _update_pointer_pos(pos_mark);
    pointer.visible = 1;
  }
  ~MarginPointer();
  float _update_pointer_pos(float p){ c_value = c_LL + p*(c_HL-c_LL); if (c_MOD != 0.0f) c_value -= int(c_value/c_MOD)*c_MOD;  return c_value; }
  virtual void  setPosition(float pos01)
  {
    MarginSingle::setPosition(pos01);
    assignText(&pointer, redact(_update_pointer_pos(pos_mark)));
    if (mec_filled())
      _update_pointer_area(mec_area(), true);
  }
  void  setPositionBifunc(float pos01, float posBi)
  {
    MarginSingle::setPosition(pos01);
    assignText(&pointer, redact(_update_pointer_pos(posBi)));
//    qDebug()<<"Position : "<<pos01<<" self : "<<pos_mark;
    if (mec_filled())
      _update_pointer_area(mec_area(), true);
  }
  void  setPosition(float pos01_A, float pos01_B)
  {
    if (mec_filled())
    {
      bool v = BAR_VERT[mec_area().atto]? orientationMirroredVert(orient) : orientationMirroredHorz(orient);
      bool m = orientationTransposed(orient) ^ BAR_VERT[mec_area().atto];
//      MarginSingle::setPosition(m? pos01_B : pos01_A);
      float pos01 = m? pos01_B : pos01_A;
      setPositionBifunc( v ? 1.0f - pos01 : pos01, pos01);
    }
  }
  void  setVisible(bool visible)
  {
    pointer.visible = visible? 1 : 0;
  }
  bool  isVisible() const
  {
    return pointer.visible != 0;
  }
protected:
  virtual   void    tfContentUpdate(){  assignText(&pointer, redact(c_value), m_font); }
protected:
  void _update_pointer_area(const uarea_t& area, bool updateSingle)
  {
    if (updateSingle)
      MarginSingle::_update_single_area(area);
    const int gaps[] = { 3 + mlen(), 3 + mlen(), 0 + mlen(), 0 + mlen()};
    pointer.pos_mark = mark.anchor;
    pointer.pos_gap = gaps[area.atto];
    pointer.pos_update_fn = POSFUNC_NN_MID[area.atto];
    if (textInnerPlaced)
    {
      int d1 = area.segm_pre + (area.segm_main-1);
      int d2 = area.segm_pre;
      if (CROSS_UP[area.atto](&pointer, area.mirrored? d2 : d1))
        pointer.pos_update_fn = BAR_VERT[area.atto]? POSFUNC_NN_UP[area.atto] : POSFUNC_NN_DOWN[area.atto];
      else if (CROSS_DOWN[area.atto](&pointer, area.mirrored? d1 : d2))
        pointer.pos_update_fn = BAR_VERT[area.atto]? POSFUNC_NN_DOWN[area.atto] : POSFUNC_NN_UP[area.atto];
    }
    pointer.pos_update_fn(&pointer);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginSingle::updateArea(area, UPDATEFOR) == false)
      return false;
    
    if (!f_assigned)
    {
      assignText(&pointer, redact(_update_pointer_pos(pos_mark)));
      f_assigned = true;
    }
    _update_pointer_area(area, false);
    return true;
  }
  
  virtual void  draw(QPainter& painter)
  {
    if (pointer.visible)
    {
      MarginSingle::draw(painter);
      if (m_fontReplaced)
      {
        painter.save();
        painter.setFont(m_font);
        painter.drawStaticText(pointer.c_pos_result, pointer.uin_text);
        painter.restore();
      }
      else
        painter.drawStaticText(pointer.c_pos_result, pointer.uin_text);
    }
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    MarginSingle::sizeHint(atto, atto_size, minsegm_main, minsegm_pre, minsegm_post);
    QFontMetrics fm(m_font);
    int maxsymbols = cachedRTexttype() == RF_SETTAPS? cachedRdata().rel_tap_qstring.slen : max_symbols();
    QSize meansize(SPACING_AFTERWIDTH + fm.averageCharWidth()*maxsymbols, SPACING_AFTERHEIGHT + fm.height());
    *atto_size += (BAR_VERT[atto]? meansize.width() : meansize.height());
    *minsegm_main += BAR_VERT[atto]? meansize.height() : meansize.width();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *minsegm_pre += d_half_dly;
    *minsegm_post += d_half_dly;
  }
  
protected:
  virtual void  bdContentUpdateBounds(float ll, float hl, float mod)
  {
    MarginSingle::bdContentUpdateBounds(ll, hl, mod);
    MarginTextformat::numformatUpdate(c_LL, c_HL);
    if (relative)
    {
      if (f_assigned && mec_filled())
        _update_pointer_area(mec_area(), true);
    }
    else
      assignText(&pointer, redact(_update_pointer_pos(pos_mark)));
  }
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn tapfn, const void* param, float relatedoffset, const tapcontent_t& tctt)
  {
    MarginSingle::bdContentUpdateTaps(base, tapfn, param, relatedoffset, tctt);
    float LLof = c_LL + (c_HL-c_LL)*relatedoffset;
    float HLof = c_HL + (c_HL-c_LL)*relatedoffset;
    tapfn(int(LLof + pos_mark*(HLof-LLof)), tctt.lmardimm, tctt.lmoffset, param, base);
    assignText(&pointer, redact(base));
  }
  virtual void  bdContentUpdateEnumerate(int from, int count, int recycle, float relatedoffset)
  {
    MarginSingle::bdContentUpdateEnumerate(from, count, recycle, relatedoffset);
    MarginTextformat::numformatInteger(false);
    assignText(&pointer, redact(_update_pointer_pos(pos_mark)));  // ntf: reloffset
  }
}; 
MarginPointer::~MarginPointer(){}


/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/***************************      MARKS                                     *****************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/

class MarginMarksBase: public MarginElementCached, public MarginCallback, public MarginTextformat
{
protected:
  int           mlenmaxi, mlenmini, mwid;
protected:
  MarginMarksBase(): mlenmaxi(4), mlenmini(2), mwid(1){}
  ~MarginMarksBase();
  int   mlen() const { return mlenmaxi; }
public:
  void    setMarkLength(int length){ mlenmaxi = length; /*needRedrawByMark = true;*/ }
  void    setMarkMiniLength(int length){ mlenmini = length; /*needRedrawByMark = true;*/ }
  void    setMarkWidth(int width){ mwid = width; /*needRedrawByMark = true;*/ }
  void    setMarkSize(int length, int width, int lengthmini=4)
  { mlenmaxi = length; mlenmini = lengthmini; mwid = width; /*needRedrawByMark = true;*/ }
protected:
  virtual   void    tfContentUpdate(){  if (mec_filled()) MarginElementCached::updateArea(mec_area(), UF_CONTENT); }
};
MarginMarksBase::~MarginMarksBase(){}


typedef MarginMarksBase MarginComplicated;

/************************************************************************************************************/

class MarginINMarks: public MarginMarksBase
{
protected:
  int           algoType;
  memark_t*     ua_marks;
  int*          ua_marklinks2;
  int           countMaxiNoted, countMaxiHided, countMini;
  int           pixStep_pixSpace;
  int           miniPerMaxi;
  int           round; // 0 - default rounding, 1 - no round, 2 - no round and +1 for all except first and last
public:
  MarginINMarks(): //needRedrawByMark(false),
    algoType(DBMODE_STRETCHED_POW2),
    ua_marks(nullptr), ua_marklinks2(nullptr), countMaxiNoted(0), countMaxiHided(0), countMini(0), pixStep_pixSpace(0), 
    miniPerMaxi(0), round(0)
  {
  }
  ~MarginINMarks();
  
  void  init(int algotype, int marksLimit, int pixStep_minSpacing, int roundsteps, int _miniPerMaxi) /// marksLimit = 0 for init later
  {
    algoType = algotype;
    pixStep_pixSpace = pixStep_minSpacing;
    if (pixStep_pixSpace <= 0)
      pixStep_pixSpace = INT_MAX;
    miniPerMaxi = _miniPerMaxi;
    round = roundsteps;
    setMarksCount(marksLimit);
  }
  
  
  virtual int setMarksCount(int marksLimit)
  {
    if (marksLimit)
    {
      if (ua_marks) delete []ua_marks;
      if (ua_marklinks2)  delete []ua_marklinks2;
      countMaxiNoted = marksLimit;
      countMaxiHided = 0;
      countMini = 0;
      
      ua_marks = new memark_t[marksLimit + miniPerMaxi*countMaxiNoted];
      ua_marklinks2 = new int[marksLimit];
      for (int i=0; i<marksLimit; i++)
        ua_marklinks2[i] = i;
    }
    return marksLimit;
  }  
protected:
  
  int   calcCountStepAndJump(int totalCount, int pixLen, const int minSpacing, int *step_out, int* jumpfrom_out, int* jumpto_out) const
  {
    float pixStep = pixLen/float(totalCount - 1);
    int result = 0, step=1, jumpfrom=0, jumpto=0;
//    qDebug()<<pixLen<<pixStep<<minSpacing;
    if (pixLen <= minSpacing)
    {
      result = 2;
      step = totalCount-1;
//      pref = "<pix";
    }
    else if (pixStep >= minSpacing)
    {
      result = totalCount;
//      pref = QString::number(minSpacing) + "> = standard";
    }
    else
    {    
      step = 2;
      while (pixStep*step < minSpacing)
      {
        step++;
//        qDebug()<<step<<step*pixStep<<pixLen;
      }
//      qDebug()<<"step calc: "<<pixStep<<minSpacing<<"... "<<step;
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
//  virtual bool  needredraw() const {  return MarginElementCached::needredraw() | needRedrawByMark; }
  virtual void  draw(QPainter& painter)
  {
//    MarginElementCached::draw(painter);
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
//    needRedrawByMark = false;
  }
  
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)
      return false;
    
//    if ((mec_filled() && mec_area().segm_main == area.segm_main) && BAR_NEEDMOVE[area.atto])
//      return false;
    
    const int dimm_main = area.segm_main - 1;
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
        if (over_deltapix[0] < pixStep_pixSpace)
        {
          int centralmark = countMaxiNoted%2;
          float odp = dimm_main/float(countMaxiNoted - 1);
          while (odp < pixStep_pixSpace && countMaxiNoted > 2)
          {
            int hp = (countMaxiNoted - centralmark)/2;
            int dv;
            if (hp % 2 == 0)
              dv = 2;
            else if (hp % 3 == 0)
              dv = 3;
            else break;
            
            int rm = ((dv-1)*countMaxiNoted)/dv;
            if ((countMaxiNoted - rm) % 2 != 1)
              break;
            countMaxiNoted -= rm;
            odp = dimm_main/float(countMaxiNoted - 1);
            over_step[0] *= dv;
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
        if (over_deltapix[0]/(miniPerMaxi+1) > 5)
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
      if (over_count[o] == 0)
        continue;
      if (BAR_VERT[area.atto])
      {
        int d2 = area.mirrored? area.segm_pre : area.segm_pre + area.segm_main - 1;
        float d3 = area.mirrored? -over_deltapix[o] : over_deltapix[o];
        int l1 = area.atto_begin;
        int l2 = area.atto_begin + (area.atto == AT_LEFT? -(over_mlen[o]-1) : (over_mlen[o]-1));
        for (int i=0; i<over_count[o]; i += over_step[o])
        {
          if (o == 1 && i % minimod == 0)  continue;
          if (o == 0) ua_marklinks2[i] = m;
          int offs = d2 - (round == 0 || i == 0 || i == over_count[0]-1? qRound(i*d3) : round == 1? int(i*d3) : int(i*d3) + 1);
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
        int d2 = area.mirrored? area.segm_pre + area.segm_main - 1 : area.segm_pre;
        float d3 = area.mirrored? -over_deltapix[o] : over_deltapix[o];
        int l1 = area.atto_begin;
        int l2 = area.atto_begin + (area.atto == AT_TOP? -(over_mlen[o]-1) : (over_mlen[o]-1));
        for (int i=0; i<over_count[o]; i += over_step[o])
        {
          if (o == 1 && i % minimod == 0)  continue;
          if (o == 0) ua_marklinks2[i] = m;
          int offs = d2 + (round == 0 || i == 0 || i == over_count[0]-1? qRound(i*d3) : round == 1? int(i*d3) : int(i*d3) + 1);
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
  virtual void sizeHint(ATTACHED_TO /*atto*/, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = mlenmaxi + 2;
    *minsegm_main = 1;
    *minsegm_pre = *minsegm_post = 0;
  }
protected:
  virtual void  bdContentUpdateBounds(float, float, float=0){}
  virtual void  bdContentUpdateTaps(QString&, mtap_qstring_fn, const void*, float, const tapcontent_t&){}
  virtual void  bdContentUpdateEnumerate(int, int, int, float){}
};
MarginINMarks::~MarginINMarks()
{
  if (ua_marks && ua_marklinks2)
  {
    delete []ua_marks;
    delete []ua_marklinks2;
  }
}

/**************************/


class MarginINMarksTextToMark: public MarginINMarks
{
  texts_t*                texts;
  bool                    textInnerPlaced;
public:
  MarginINMarksTextToMark(bool innerTextPlacement): textInnerPlaced(innerTextPlacement) {}
  ~MarginINMarksTextToMark();
  virtual int setMarksCount(int marksLimit)
  {
    marksLimit = MarginINMarks::setMarksCount(marksLimit);
    texts = new texts_t[marksLimit];
    return marksLimit;
  }
protected:
//  virtual bool  needredraw() const {  return MarginINMarksTexted::needredraw(); }
  virtual void  draw(QPainter& painter)
  {
    MarginINMarks::draw(painter);
    {
      if (m_fontReplaced)
        painter.setFont(m_font);
      for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
        if (texts[i].visible)
          painter.drawStaticText(texts[i].c_pos_result, texts[i].uin_text);
    }
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool ua = MarginINMarks::updateArea(area, UPDATEFOR);
    if (ua == false && (UPDATEFOR != UF_CONTENT))
      return false;
    
    storeDimm(area.segm_main);
    
    const int gaps[] = { 3 + mlenmaxi, 3 + mlenmaxi, 0 + mlenmaxi, 0 + mlenmaxi };
    void  (*lposfunc_nn_mid)(texts_t*) = POSFUNC_NN_MID[area.atto];
    void  (*lposfunc_nn_fnbeg)(texts_t*) = area.mirrored? POSFUNC_NN_DOWN[area.atto] : POSFUNC_NN_UP[area.atto];
    void  (*lposfunc_nn_fnend)(texts_t*) = area.mirrored? POSFUNC_NN_UP[area.atto] : POSFUNC_NN_DOWN[area.atto];
    
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
    {
      texts[i].pos_gap = gaps[area.atto];
      if (ua_marklinks2[i] != -1)
      {
        texts[i].pos_mark = ua_marks[ua_marklinks2[i]].anchor;
        if (textInnerPlaced && i == 0)
          texts[i].pos_update_fn = lposfunc_nn_fnbeg;
        else if (textInnerPlaced && i == countMaxiNoted + countMaxiHided - 1)
          texts[i].pos_update_fn = lposfunc_nn_fnend;
        else
          texts[i].pos_update_fn = lposfunc_nn_mid;
        texts[i].visible = 1;
        texts[i].pos_update_fn(&texts[i]);
      }
      else
        texts[i].visible = 0;
    }
    rearrange(BAR_VERT[area.atto], area.mirrored, texts, countMaxiNoted + countMaxiHided);
    
    return true;
  }
  
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    MarginINMarks::sizeHint(atto, atto_size, minsegm_main, minsegm_pre, minsegm_post);
    QFontMetrics fm(m_font);
    int maxsymbols = cachedRTexttype() == RF_SETTAPS? cachedRdata().rel_tap_qstring.slen : max_symbols();
    QSize meansize(SPACING_AFTERWIDTH + fm.averageCharWidth()*maxsymbols, SPACING_AFTERHEIGHT + fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *atto_size += d_atto_size;
    *minsegm_pre = d_half_dly;
    *minsegm_post = d_half_dly;
  }
protected:
  virtual void  bdContentUpdateBounds(float LL, float HL, float mod)
  {
    const int total = countMaxiNoted + countMaxiHided;
    float bndstep = (HL - LL)/(total - 1);
    MarginTextformat::numformatUpdate(LL, HL);
    
    if (mod != 0.0f)
    {
      float pv = LL;
      for (int i=0; i<total; i++)
      {
        float v = LL + i*bndstep;
        v -= int(v/mod)*mod;
        if (i == total-1 && pv > v)
        {
          if (v < mod/120.0f)
            v += mod;
        }
        if (postfixLimited())
          turnPostfixByPosition(i, total);
        assignText(&texts[i], redact(v), m_fontReplaced, m_font);
        pv = v;
      }
    }
    else
      for (int i=0; i<total; i++)
      {
        if (postfixLimited())
          turnPostfixByPosition(i, total);
        assignText(&texts[i], redact(LL + i*bndstep), m_fontReplaced, m_font);
      }
  }
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn tapfn, const void* param, float /*relatedoffset*/, const tapcontent_t& tctt)
  {
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, countMaxiNoted + countMaxiHided);
      tapfn(i, tctt.lmardimm, tctt.lmoffset, param, base);
      assignText(&texts[i], redact(base), m_fontReplaced, m_font);
    }
  }
  virtual void  bdContentUpdateEnumerate(int from, int /*count*/, int /*recycle*/, float relatedoffset)
  {
    MarginTextformat::numformatInteger(false);
    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, countMaxiNoted + countMaxiHided);
      int num = from + i + int((countMaxiNoted + countMaxiHided)*relatedoffset);
//      if (recycle != -1)  num = num % recycle;
      assignText(&texts[i], redact(QString::number(num)), m_fontReplaced, m_font);
    }
  }
};
MarginINMarksTextToMark::~MarginINMarksTextToMark(){}

class MarginINMarks2only: public MarginINMarks
{
  texts_t*                texts;
  bool                    textInnerPlaced;
public:
  MarginINMarks2only(bool innerTextPlacement): textInnerPlaced(innerTextPlacement){}
  ~MarginINMarks2only();
  virtual int setMarksCount(int marksLimit)
  {
    marksLimit = MarginINMarks::setMarksCount(marksLimit);
    texts = new texts_t[2];
    return marksLimit;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    MarginINMarks::draw(painter);
    if (m_fontReplaced)
      painter.setFont(m_font);
    for (int i=0; i<2; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].c_pos_result, texts[i].uin_text);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool ua = MarginINMarks::updateArea(area, UPDATEFOR);
    if (ua == false && (UPDATEFOR != UF_CONTENT))
      return false;
    
    storeDimm(area.segm_main);
    
    const int gaps[] = { 3 + mlenmaxi, 3 + mlenmaxi, 0 + mlenmaxi, 0 + mlenmaxi };
    int lmp[] = { 0, algoType == DBMODE_STATIC? countMaxiNoted - 1 : countMaxiNoted + countMaxiHided - 1 };
    
    for (int i=0; i<2; i++)
      if (ua_marklinks2[lmp[i]] != -1)
      {
        texts[i].pos_mark = ua_marks[ua_marklinks2[lmp[i]]].anchor;
        texts[i].pos_gap = gaps[area.atto];
//        texts[i].pos_update_fn = textInnerPlaced? (area.mirrored? POSFUNC_NN_UP[area.atto] : POSFUNC_NN_DOWN[area.atto]) : 
//                                                  POSFUNC_NN_MID[area.atto];
        if (textInnerPlaced)
        {
          texts[i].pos_update_fn = area.mirrored ^ (i == 0)? POSFUNC_NN_UP[area.atto] : POSFUNC_NN_DOWN[area.atto];
        }
        else
          texts[i].pos_update_fn = POSFUNC_NN_MID[area.atto];
        
        texts[i].visible = 1;
        
        texts[i].pos_update_fn(&texts[i]);
      }
      else
      {
        texts[i].visible = 0;
        texts[i].pos_update_fn = nullptr;
      }
    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    MarginINMarks::sizeHint(atto, atto_size, minsegm_main, minsegm_pre, minsegm_post);
    QFontMetrics fm(m_font);
    int maxsymbols = cachedRTexttype() == RF_SETTAPS? cachedRdata().rel_tap_qstring.slen : max_symbols();
    QSize meansize(SPACING_AFTERWIDTH + fm.averageCharWidth()*maxsymbols, SPACING_AFTERHEIGHT + fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *atto_size += d_atto_size;
    *minsegm_pre = d_half_dly;
    *minsegm_post = d_half_dly;
  }
protected:
  virtual void  bdContentUpdateBounds(float LL, float HL, float mod)
  {
    MarginTextformat::numformatUpdate(LL, HL);
    if (mod != 0.0f)
    {
      LL -= int(LL/mod)*mod;
      HL -= int(HL/mod)*mod;
      if (HL <= LL)
      {
        if (HL < mod/120.0f)
          HL += mod;
      }
    }
    for (int i=0; i<2; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, 2);
      assignText(&texts[i], redact(i == 0? LL : HL), m_fontReplaced, m_font);
    }
  }
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn tapfn, const void* param, float /*relatedoffset*/, const tapcontent_t& tctt)
  {
    for (int i=0; i<2; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, 2);
      tapfn(i, tctt.lmardimm, tctt.lmoffset, param, base);
      assignText(&texts[i], redact(base), m_fontReplaced, m_font);
    }
  }
  virtual void  bdContentUpdateEnumerate(int from, int /*count*/, int /*recycle*/, float /*relatedoffset*/)
  {
    MarginTextformat::numformatInteger(false);
    for (int i=0; i<2; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, 2);
      assignText(&texts[i], redact(QString::number(from + i)), m_fontReplaced, m_font);
    }
  }
};
MarginINMarks2only::~MarginINMarks2only(){}

/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/

enum  { DOCK_PREV, DOCK_BETWEEN, DOCK_NEXT };
  
class MarginINMarksTextBetweenMark: public MarginINMarks
{
protected:
  texts_t*      texts;
  int           docking;
  bool          showLastEnumer;
  unsigned int  submod;
public:
  MarginINMarksTextBetweenMark(): docking(DOCK_BETWEEN), showLastEnumer(false), submod(1) { }
  ~MarginINMarksTextBetweenMark();

  void  init_wide(int algotype, int marksLimit, int pixStep_minSpacing, unsigned int step, int roundsteps, bool alwaysShowLast, int dockto)
  {
    MarginINMarks::init(algotype, marksLimit, pixStep_minSpacing, roundsteps, 0);
    docking = dockto;
    showLastEnumer = alwaysShowLast;
    submod = step;
  }
  virtual int   setMarksCount(int marksLimit)
  {
    marksLimit = MarginINMarks::setMarksCount(marksLimit);
    if (marksLimit)
      texts = new texts_t[marksLimit-1 + 1];
    return marksLimit;
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    MarginINMarks::draw(painter);
    if (m_fontReplaced)
      painter.setFont(m_font);
    for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].c_pos_result, texts[i].uin_text);
  }
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool ua = MarginINMarks::updateArea(area, UPDATEFOR);
    if (ua == false && (UPDATEFOR != UF_CONTENT))
      return false;
    
    storeDimm(area.segm_main);
    
    const int total = countMaxiNoted + countMaxiHided - 1;
    int last0=0, last1=0, subctr = 0;
    
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
        last1 = last0;
        last0 = i;
        if (algoType == DBMODE_STATIC && j > total)
          texts[i].visible = 0;
        else
        {
          texts[i].visible = (subctr++ % int(submod) == 0)? 1 : 0;
          QPoint& mcur = ua_marks[ua_marklinks2[i]].anchor, &mnext = j == total+1? ua_marks[ua_marklinks2[i]].anchor : ua_marks[ua_marklinks2[j]].anchor;
////          qDebug()<<i<<ua_marklinks2[i]<<j<<ua_marklinks2[j]<<total+1<<-(mnext - mcur).y()/2;
          _recalcPos(i, area.atto, mcur, mnext, docking);
        }
        i = j;
      }
      else
        texts[i++].visible = 0;
    }
              
    if (showLastEnumer)
    {
      if (last0 && texts[total-1].visible != 1)
      {
        texts[total - 1].visible = 1;
        if (last0 != total-1)
        {
          texts[last0].visible = 0;
          if (algoType == DBMODE_STATIC)
          {
//            texts[total - 1].uarea_pos = texts[last].uarea_pos;
//            _recalcPos(total-1, area.atto, ua_marks[ua_marklinks2[last1]].anchor, ua_marks[ua_marklinks2[last0]].anchor, docking);  // why dock_prev? intuition
            _recalcPos(total-1, area.atto, ua_marks[ua_marklinks2[last1]].anchor, ua_marks[ua_marklinks2[last0]].anchor, DOCK_NEXT);
          }
          else
          {
#if 1
//            _recalcPos(total-1, area.atto, ua_marks[ua_marklinks2[last]].anchor, ua_marks[ua_marklinks2[total]].anchor, docking == DOCK_PREV? DOCK_NEXT : docking == DOCK_NEXT? DOCK_PREV : DOCK_BETWEEN);
//            _recalcPos(total-1, area.atto, ua_marks[ua_marklinks2[last0]].anchor, ua_marks[ua_marklinks2[total]].anchor, docking == DOCK_PREV? DOCK_NEXT : docking == DOCK_NEXT? DOCK_PREV : DOCK_BETWEEN);
            _recalcPos(total-1, area.atto, ua_marks[ua_marklinks2[last0]].anchor, ua_marks[ua_marklinks2[total]].anchor, DOCK_NEXT);
#else
            texts[total - 1].uarea_pos = texts[last].uarea_pos;
#endif
          }
        }
      }
    }
    rearrange(BAR_VERT[area.atto], area.mirrored, texts, total);
    return true;
  }
  void  _recalcPos(int i, ATTACHED_TO atto, const QPoint& mcur, const QPoint& mnext, int locdock)
  {
    int dx = mnext.x() - mcur.x(), dy = mnext.y() - mcur.y();
    int ddo = 8;
    texts[i].pos_gap = 2;
    texts[i].pos_mark = QPoint(mcur.x() + dx/2, mcur.y() + dy/2);
    if (locdock == 0)
      texts[i].pos_mark_dopdock = QPoint(mcur.x() + dx/ddo, mcur.y() + dy/ddo);
    else if (locdock == 1)
      texts[i].pos_mark_dopdock = texts[i].pos_mark;
    else
      texts[i].pos_mark_dopdock = QPoint(mnext.x() - dx/ddo, mnext.y() - dy/ddo);
    texts[i].pos_update_fn =  locdock == 0? POSFUNC_NM_UP[atto] : 
                              locdock == 1? POSFUNC_NM_MID[atto] : 
                                            POSFUNC_NM_DOWN[atto];
    if (!texts[i].uin_locsize.isNull())
      texts[i].pos_update_fn(&texts[i]);
  }
        
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    MarginINMarks::sizeHint(atto, atto_size, minsegm_main, minsegm_pre, minsegm_post);
    QFontMetrics fm(m_font);
    int maxsymbols = cachedRTexttype() == RF_SETTAPS? cachedRdata().rel_tap_qstring.slen : max_symbols();
    QSize meansize(SPACING_AFTERWIDTH + fm.averageCharWidth()*maxsymbols, SPACING_AFTERHEIGHT + fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
//    int d_half_dly = BAR_VERT[atto]? meansize.height()/4 : meansize.width()/4;  // ? /4 ?
    int d_half_dly = 0;
    *atto_size = qMax(*atto_size, d_atto_size);
    *minsegm_pre += d_half_dly;
    *minsegm_post += d_half_dly;
  }
protected:
  virtual void  bdContentUpdateBounds(float LL, float HL, float mod)
  {
    const int total = countMaxiNoted + countMaxiHided - 1;
    float bndstep = qAbs(HL - LL)/total;
    MarginTextformat::numformatUpdate(LL, HL);
    if (mod != 0.0f)
    {
      float pv = LL;
      for (int i=0; i<total; i++)
      {
        if (postfixLimited())
          turnPostfixByPosition(i, total);
        float v = LL + i*bndstep;
        v -= int(v/mod)*mod;
        if (i == total-1 && pv > v)
        {
          if (v < mod/120.0f)
            v += mod;
//          v = pv + bndstep;
        }
        assignText(&texts[i], redact(v), m_fontReplaced, m_font);
        pv = v;
      }
    }
    else
      for (int i=0; i<total; i++)
      {
        if (postfixLimited())
          turnPostfixByPosition(i, total);
        assignText(&texts[i], redact(LL + i*bndstep), m_fontReplaced, m_font);
      }
  }
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn tapfn, const void* param, float /*relatedoffset*/, const tapcontent_t& tctt)
  {
    for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, countMaxiNoted + countMaxiHided - 1);
      tapfn(i, tctt.lmardimm, tctt.lmoffset, param, base);
      assignText(&texts[i], redact(base), m_fontReplaced, m_font);
    }
  }
  virtual void  bdContentUpdateEnumerate(int from, int , int , float )
  {
    MarginTextformat::numformatInteger(false);
    for (int i=0; i<countMaxiNoted + countMaxiHided - 1; i++)
    {
      if (postfixLimited())
        turnPostfixByPosition(i, countMaxiNoted + countMaxiHided - 1);
      int num = from + i;
      assignText(&texts[i], redact(QString::number(num)), m_fontReplaced, m_font);
    }
  }
};
MarginINMarksTextBetweenMark::~MarginINMarksTextBetweenMark(){}


/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/

class MarginINMarksWidgetBetween: public MarginINMarks
{
protected:
  static const int gapH = 2;
  static const int gapV = 2;
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
  MarginINMarksWidgetBetween(): showLastEnumer(false), submod(1), maxperpendiculardimm(0), wftype(WF_OFF){}
  ~MarginINMarksWidgetBetween();

  void  init_wide(int algotype, int marksLimit, int pixStep_minSpacing, unsigned int step, int roundsteps, bool alwaysShowLast, unsigned int maxperpdimm, mtap_qwidget_fn ftor, void* fpm, QWidget* prnt)
  {
    MarginINMarks::init(algotype, marksLimit, pixStep_minSpacing, roundsteps, 0);
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
  void  init_wide(int algotype, int marksNwidgetsCount, int pixStep_minSpacing, unsigned int step, int roundsteps, bool alwaysShowLast, unsigned int maxperpdimm, QWidget* wdgs[], QWidget* prnt)
  {
    MarginINMarks::init(algotype, marksNwidgetsCount, pixStep_minSpacing, roundsteps, 0);
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
      if (mec_filled())
        updateArea(mec_area(), UF_CONTENT);
    }
  }
protected:
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    bool ua = MarginINMarks::updateArea(area, UPDATEFOR);
    if (ua == false && (UPDATEFOR != UF_CONTENT))
      return false;
    
    const int total = countMaxiNoted + countMaxiHided - 1;
    
    if (wftype == WF_DYNAMIC && wfdetails.wf_dynamic.doretap)
    {
      for (int i=0; i<widgets.count(); i++)
        widgets[i]->deleteLater();
      widgets.resize(total);
      for (int i=0; i<total; i++)
      {
        widgets[i] = wfdetails.wf_dynamic.ftor(i, ua_marks[ua_marklinks2[i]].anchor.x(), area.segm_main, wfdetails.wf_dynamic.fpm);
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
          if (area.atto == AT_LEFT)
            widgets[i]->move(mcur - QPoint(-gapH + qMin<int>(widgets[i]->width(), maxperpendiculardimm), gapV));
          else if (area.atto == AT_RIGHT)
            widgets[i]->move(mcur - QPoint(-gapH, gapV));
          else if (area.atto == AT_TOP)
            widgets[i]->move(mcur - QPoint(-gapH, gapV + qMin<int>(widgets[i]->height(), maxperpendiculardimm)));
          else if (area.atto == AT_BOTTOM)
            widgets[i]->move(mcur - QPoint(-gapH, -gapV));
              
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
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    MarginINMarks::sizeHint(atto, atto_size, minsegm_main, minsegm_pre, minsegm_post);
//    int d_atto_size = BAR_VERT[atto]? meansize.width()+2 : meansize.height()+2;
    int d_atto_size = maxperpendiculardimm + (BAR_VERT[atto]? gapH : gapV);
    int d_half_dly = 0;
    *atto_size = qMax(*atto_size, d_atto_size);
    *minsegm_pre += d_half_dly;
    *minsegm_post += d_half_dly;
  }
public:
};
MarginINMarksWidgetBetween::~MarginINMarksWidgetBetween(){}


/************************************************************************************************************/


class MarginEXMarks: public MarginMarksBase
{
protected:
  memark_t*     ua_marks;
  int           countMaxiNoted, countMaxiHided, countMini;
  float         odd, oddbase;
  int           pixSpace;
  int           miniPerMaxi;
  int           divalgo;
  int           round; // 0 - rounding, 1 - no round, 2 - no round and +1 for all except first and last
  double        c_multiplier;
protected:
  bool          textInnerPlaced;
  texts_t*      texts;
public:
  MarginEXMarks(bool innerTextPlacement): 
    ua_marks(nullptr), countMaxiNoted(0), countMaxiHided(0), countMini(0), pixSpace(0), 
    miniPerMaxi(0), round(0), c_multiplier(0),
    textInnerPlaced(innerTextPlacement), texts(nullptr)
  {
  }
  ~MarginEXMarks();
  
  void  init(int marksLimit, float ODD, float ODDbase, int minSpacing, int roundsteps, int _miniPerMaxi, int _divalgo) /// marksLimit = 0 for init later
  {
    Q_ASSERT(ODD != 0.0f);
    odd = ODD;
    oddbase = ODDbase;
    pixSpace = minSpacing;
    miniPerMaxi = _miniPerMaxi;
    round = roundsteps;
    divalgo = _divalgo;
    
    if (marksLimit)
    {
      if (ua_marks) delete []ua_marks;
      countMaxiNoted = marksLimit;
      countMaxiHided = 0;
      countMini = 0;
      
      ua_marks = new memark_t[marksLimit + (marksLimit-1)*miniPerMaxi];
      texts = new texts_t[marksLimit];
    }
  }
protected:
  virtual void  draw(QPainter& painter)
  {
    if (c_color_redefined)
      for (int i=0; i<countMaxiNoted + (miniPerMaxi > 0? countMini : 0); i++)
        painter.fillRect(ua_marks[i].rect, c_color);
    else
      for (int i=0; i<countMaxiNoted + (miniPerMaxi > 0? countMini : 0); i++)
        painter.fillRect(ua_marks[i].rect, painter.brush());
    
    for (int i=0; i<countMaxiNoted; i++)
      if (texts[i].visible)
        painter.drawStaticText(texts[i].c_pos_result, texts[i].uin_text);
  }
  
  virtual bool  updateArea(const uarea_t& area, int UPDATEFOR)
  {
    if (MarginElementCached::updateArea(area, UPDATEFOR) == false)    // forced for strict for bounds
      return false;
    
    storeDimm(area.segm_main);
    
    recalculate(area);
    
    return true;
  }
  virtual void sizeHint(ATTACHED_TO atto, int* atto_size, int* minsegm_main, int* minsegm_pre, int* minsegm_post) const
  {
    *atto_size = mlenmaxi + 2;
    *minsegm_main = 1;
    QFontMetrics fm(m_font);
    int maxsymbols = cachedRTexttype() == RF_SETTAPS? cachedRdata().rel_tap_qstring.slen : max_symbols();
    QSize meansize(SPACING_AFTERWIDTH + fm.averageCharWidth()*maxsymbols, SPACING_AFTERHEIGHT + fm.height());
    int d_atto_size = BAR_VERT[atto]? meansize.width() : meansize.height();
    int d_half_dly = textInnerPlaced? 0 : BAR_VERT[atto]? meansize.height()/2 : meansize.width()/2;
    *atto_size += d_atto_size;
    *minsegm_pre = d_half_dly;
    *minsegm_post = d_half_dly;
  }
protected:
  virtual   void    tfContentUpdate(){  if (mec_filled()) MarginElementCached::updateArea(mec_area(), UF_CONTENT); }
protected:
  virtual void  bdContentUpdateBounds(float LL, float HL, float /*mod*/)
  {
    MarginTextformat::numformatUpdate(LL, HL);
    if (mec_filled())
    {
      c_multiplier = 0;
      recalculate(mec_area());
    }
  }
  virtual void  bdContentUpdateTaps(QString& base, mtap_qstring_fn tapfn, const void* param, float relatedoffset, const tapcontent_t& tctt)
  {
//    for (int i=0; i<countMaxiNoted + countMaxiHided; i++)
//    {
//      tapfn(i, tctt.lmardimm, tctt.lmoffset, param, base);
//      assignText(&texts[i], redact(base), m_fontReplaced, m_font);
//    }
    if (mec_filled())
    {
      c_multiplier = 0;
    }
  }
  virtual void  bdContentUpdateEnumerate(int from, int count, int recycle, float relatedoffset)
  {
    MarginTextformat::numformatInteger(false);
    if (mec_filled())
    {
      c_multiplier = 0;
      recalculate(mec_area());
    }
  }
private:
  float modcast(float v){ return v - int(v/cachedRdata().rel_fixed.MOD)*cachedRdata().rel_fixed.MOD; }
  float modcast2(float v, float pv)
  {
    float result = v - int(v/cachedRdata().rel_fixed.MOD)*cachedRdata().rel_fixed.MOD;
    if (result < pv && result < oddbase + cachedRdata().rel_fixed.MOD/120.0f)
      result += cachedRdata().rel_fixed.MOD;
    return result;
  }
  void  recalculate(const uarea_t& area)
  {
    const int countMaxiTotal = countMaxiNoted + countMaxiHided;
    float LL = cachedRdata().rel_fixed.LL, HL = cachedRdata().rel_fixed.HL;
    bool isMOD = cachedRdata().rel_fixed.MOD != 0.0f;
    const int gaps[] = { 3 + mlenmaxi, 3 + mlenmaxi, 0 + mlenmaxi, 0 + mlenmaxi };
    bool  doChangeLabel = false;
    
    int l1 = area.atto_begin;
    int l2 = area.atto_begin + (area.atto == AT_LEFT || area.atto == AT_TOP? -(mlenmaxi-1) : (mlenmaxi-1));
    bool MVORIG = BAR_VERT[area.atto] ^ !area.mirrored;   // original plus-based formula
    
    if (LL > HL)
    {
      MVORIG = !MVORIG;
      float tmp = LL; LL = HL; HL = tmp;
    }
    
    float multiplier;
    if (LL == HL)
      multiplier = 1.0f;
    else
    {
      double d_multiplier = 1.0;
      double countReglineF = (HL - LL)/odd;
      const int countMaxiInside = countMaxiTotal - 2;
      
      int divs0[] = { 1000, 10, 5, 2 }; // fixed len 4
      int divs1[] = { 1500, 15, 5, 3 };
      int divs2[] = { 100000, 1000, 100, 10 };
      int* divs = divalgo == 2? divs2 : divalgo == 1? divs1 : divs0;
      for (int d=0; d<4; d++)
      {
        while (countReglineF/divs[d] > countMaxiInside)
        {
          countReglineF /= divs[d];
          d_multiplier *= divs[d];
        }
      }
      if (countReglineF > countMaxiInside)
      {
        countReglineF /= divs[4-1];
        d_multiplier *= divs[4-1];
      }

//      qDebug()<<"<<<"<<area.segm_main<<pixSpace<<countReglineF;
      if (pixSpace > 0 && countReglineF > 1)
      {
        if (area.segm_main > pixSpace)
        {
          int inversedCount = int(countReglineF);
          while (area.segm_main / inversedCount < pixSpace)
          {
            inversedCount /= 2;
            d_multiplier *= 2;
          }
        }
        else
        {
          d_multiplier *= area.segm_main;
        }
      }
      
      doChangeLabel = d_multiplier != c_multiplier;
      c_multiplier = d_multiplier;
      multiplier = float(d_multiplier);
    }
    
    int m=0;
    float fpxStep=0;  // multiplied true bnd-offset by odd
    int pxBegin=0;
    float fpxFirst=0;  // dimm-offset by odd
    float fpxLast=0;
    int pxEnd=0;
    {
      float mover = LL;
      const int dimm_main = area.segm_main - 1;
//      pxBegin = MVORIG? area.segm_pre : area.segm_pre + area.segm_main - 1;
//      pxEnd = MVORIG? area.segm_pre + area.segm_main - 1 : area.segm_pre;
      pxBegin = MVORIG? area.segm_pre : area.segm_pre + dimm_main;
      pxEnd = MVORIG? area.segm_pre + dimm_main : area.segm_pre;
      
      ua_marks[m].anchor = area.atto == AT_LEFT || area.atto == AT_RIGHT ? QPoint(l1, pxBegin) : QPoint(pxBegin, l1);
      if (area.atto == AT_LEFT)         ua_marks[m].rect.setCoords(l2, pxBegin, l1, pxBegin);
      else if (area.atto == AT_RIGHT)   ua_marks[m].rect.setCoords(l1, pxBegin, l2, pxBegin);
      else if (area.atto == AT_TOP)     ua_marks[m].rect.setCoords(pxBegin, l2, pxBegin, l1);
      else if (area.atto == AT_BOTTOM)  ua_marks[m].rect.setCoords(pxBegin, l1, pxBegin, l2);
      texts[m].pos_mark = ua_marks[m].anchor;
      texts[m].pos_gap = gaps[area.atto];
      texts[m].pos_update_fn = textInnerPlaced ? area.mirrored? POSFUNC_NN_DOWN[area.atto] : POSFUNC_NN_UP[area.atto] : POSFUNC_NN_MID[area.atto];
      if (doChangeLabel)
      {
        turnPostfixByPosition(0, countMaxiTotal);
        assignText(&texts[m], redact(isMOD? modcast(mover): mover), m_fontReplaced, m_font);
      }
      texts[m].pos_update_fn(&texts[m]);
      texts[m].visible = 1;
      m++;
      
      {
        if (doChangeLabel)
        {
          turnPostfixByPosition(1, countMaxiTotal);
        }
        float odd1 = multiplier*odd;
        mover = (LL - oddbase) - int((LL - oddbase)/odd1)*odd1;     // bnd-offset by odd
        if (mover < 0)
          mover = LL - (odd1 + mover) + odd1;
        else
          mover = LL - mover + odd1;
        
//        int dlen0 = (mover - LL)/(HL - LL)*area.segm_main;
//        fpxFirst = pxBegin + (MVORIG? dlen0 : -dlen0);
//        fpxStep = (MVORIG? odd1 : -odd1)/(HL-LL)*area.segm_main;
        
        fpxFirst = pxBegin + (MVORIG? mover - LL : -(mover - LL))/(HL - LL)*dimm_main;
        fpxStep = (MVORIG? odd1 : -odd1)/(HL-LL)*dimm_main;
        
#define FAST_ASSIGN_TEXT \
          texts[m].pos_mark = ua_marks[m].anchor; \
          texts[m].pos_gap = gaps[area.atto]; \
          texts[m].pos_update_fn = POSFUNC_NN_MID[area.atto]; \
          if (doChangeLabel) \
            assignText(&texts[m], redact(isMOD? modcast(mover): mover), m_fontReplaced, m_font); \
          texts[m].pos_update_fn(&texts[m]);
        
        if (area.atto == AT_LEFT)
        {
          while (mover < HL)
          {
            Q_ASSERT(m + 2 <= countMaxiTotal);
            float pos1 = (m - 1)*fpxStep;
            switch (round)
            {
            case 0: fpxLast = qRound(fpxFirst + pos1); break;
            case 1: fpxLast = int(fpxFirst + pos1); break;
            case 2: fpxLast = int(fpxFirst + pos1) + 1; break;
            }
            ua_marks[m].anchor = QPoint(l1, fpxLast);
            ua_marks[m].rect.setCoords(l2, fpxLast, l1, fpxLast);
            FAST_ASSIGN_TEXT
            mover += odd1;
            m++;
          }
        }
        else if (area.atto == AT_RIGHT) 
        {
          while (mover < HL)
          {
            Q_ASSERT(m + 2 <= countMaxiTotal);
            float pos1 = (m - 1)*fpxStep;
            switch (round)
            {
            case 0: fpxLast = qRound(fpxFirst + pos1); break;
            case 1: fpxLast = int(fpxFirst + pos1); break;
            case 2: fpxLast = int(fpxFirst + pos1) + 1; break;
            }
            ua_marks[m].anchor = QPoint(l1, fpxLast);
            ua_marks[m].rect.setCoords(l1, fpxLast, l2, fpxLast);
            FAST_ASSIGN_TEXT
            mover += odd1;
            m++;
          }
        }
        else if (area.atto == AT_TOP)
        {
          while (mover < HL)
          {
            Q_ASSERT(m + 2 <= countMaxiTotal);
            float pos1 = (m - 1)*fpxStep;
            switch (round)
            {
            case 0: fpxLast = qRound(fpxFirst + pos1); break;
            case 1: fpxLast = int(fpxFirst + pos1); break;
            case 2: fpxLast = int(fpxFirst + pos1) + 1; break;
            }
            ua_marks[m].anchor = QPoint(fpxLast, l1);
            ua_marks[m].rect.setCoords(fpxLast, l2, fpxLast, l1);
            FAST_ASSIGN_TEXT
            mover += odd1;
            m++;
          }
        }
        else if (area.atto == AT_BOTTOM) 
        {
          while (mover < HL)
          {
            Q_ASSERT(m + 2 <= countMaxiTotal);
            float pos1 = (m - 1)*fpxStep;
            switch (round)
            {
            case 0: fpxLast = qRound(fpxFirst + pos1); break;
            case 1: fpxLast = int(fpxFirst + pos1); break;
            case 2: fpxLast = int(fpxFirst + pos1) + 1; break;
            }
            ua_marks[m].anchor = QPoint(fpxLast, l1);
            ua_marks[m].rect.setCoords(fpxLast, l1, fpxLast, l2);
            FAST_ASSIGN_TEXT
            mover += odd1;
            m++;
          }
        }
        
#define FAST_CHECK_FOR_HIDE_TEXT(xy) \
      int pxPrev = ua_marks[0].anchor.xy(); \
      if (MVORIG) for (int i=1; i<m; i++){ \
                    if (ua_marks[i].anchor.xy() - pxPrev > pixSpace) \
                    { \
                      texts[i].visible = 1; \
                      pxPrev = ua_marks[i].anchor.xy(); \
                    } \
                    else \
                      texts[i].visible = 0; \
                  } \
      else        for (int i=1; i<m; i++){ \
                    if (pxPrev - ua_marks[i].anchor.xy() > pixSpace) \
                    { \
                      texts[i].visible = 1; \
                      pxPrev = ua_marks[i].anchor.xy(); \
                    } \
                    else \
                      texts[i].visible = 0; \
                  } 
        
        if (area.atto == AT_LEFT)
        {
          FAST_CHECK_FOR_HIDE_TEXT(y)
        }
        else if (area.atto == AT_RIGHT) 
        {
          FAST_CHECK_FOR_HIDE_TEXT(y)
        }
        else if (area.atto == AT_TOP)
        {
          FAST_CHECK_FOR_HIDE_TEXT(x)
        }
        else if (area.atto == AT_BOTTOM) 
        {
          FAST_CHECK_FOR_HIDE_TEXT(x)
        }
        
//        for (int i=1; i<m; i++)
//          texts[i].visible = 1;
        
        if (m > 1)
        {
//          if (texts[m-1].visible)
//            texts[m-1].visible = (MVORIG ? (pxEnd - fpxLast) : -(pxEnd - fpxLast)) > (BAR_VERT[area.atto] ? texts[m-1].uin_locsize.height() : texts[m-1].uin_locsize.width())? 1 : 0;
          if (texts[m-1].visible)
            texts[m-1].visible = (MVORIG ? (pxEnd - fpxLast) : -(pxEnd - fpxLast)) > pixSpace? 1 : 0;
        }
      }
      
      ua_marks[m].anchor = area.atto == AT_LEFT || area.atto == AT_RIGHT ? QPoint(l1, pxEnd) : QPoint(pxEnd, l1);
      if (area.atto == AT_LEFT)         ua_marks[m].rect.setCoords(l2, pxEnd, l1, pxEnd);
      else if (area.atto == AT_RIGHT)   ua_marks[m].rect.setCoords(l1, pxEnd, l2, pxEnd);
      else if (area.atto == AT_TOP)     ua_marks[m].rect.setCoords(pxEnd, l2, pxEnd, l1);
      else if (area.atto == AT_BOTTOM)  ua_marks[m].rect.setCoords(pxEnd, l1, pxEnd, l2);
      texts[m].pos_mark = ua_marks[m].anchor;
      texts[m].pos_gap = gaps[area.atto];
      texts[m].pos_update_fn = textInnerPlaced ? area.mirrored? POSFUNC_NN_UP[area.atto] : POSFUNC_NN_DOWN[area.atto] : POSFUNC_NN_MID[area.atto];
      if (doChangeLabel)
      {
        turnPostfixByPosition(countMaxiTotal-1, countMaxiTotal);
        assignText(&texts[m], redact(isMOD? modcast2(HL, mover): HL), m_fontReplaced, m_font);
      }
      texts[m].pos_update_fn(&texts[m]);
      texts[m].visible = 1;
      m++;
    }
    
    countMaxiNoted = m;
    countMaxiHided = countMaxiTotal - countMaxiNoted;
    Q_ASSERT(countMaxiHided >= 0);
    
    if (miniPerMaxi && qAbs(fpxStep)/(miniPerMaxi+1) > 5 && m > 2)
    {
      countMini = m;
      l2 = area.atto_begin + (area.atto == AT_LEFT || area.atto == AT_TOP? -(mlenmini-1) : (mlenmini-1));
      float step2 = fpxStep/(miniPerMaxi+1);
      
      float pxMini = fpxFirst;
      int   oddplus = 0, ncount=(m-2-1)*(miniPerMaxi + 1);
//        if (MVORIG)
      {
        if ((fpxFirst - pxBegin + 1) / step2 >= 2)
        {
          pxMini -= int((fpxFirst - pxBegin) / step2) * step2;
          oddplus = (fpxFirst - pxBegin) / step2;
          ncount += oddplus;
        }
        if ((pxEnd - fpxLast + 1) / step2 >= 2)
        {
          ncount += (pxEnd - fpxLast + 1) / step2;
        }
      }
      for (int i=0; i<ncount; i++)
      {
        if ((i - oddplus) % (miniPerMaxi+1) == 0)
          continue;
        int offs = int  ( round == 0  ? qRound(pxMini + i*step2) : 
                          round == 1  ? pxMini + i*step2 : 
                                        pxMini + (i*step2) + 1  );
//        ua_marks[m].anchor = area.atto == AT_LEFT || area.atto == AT_RIGHT ? QPoint(l1, offs) : QPoint(offs, l1);
        if (area.atto == AT_LEFT)         ua_marks[m].rect.setCoords(l2, offs, l1, offs);
        else if (area.atto == AT_RIGHT)   ua_marks[m].rect.setCoords(l1, offs, l2, offs);
        else if (area.atto == AT_TOP)     ua_marks[m].rect.setCoords(offs, l2, offs, l1);
        else if (area.atto == AT_BOTTOM)  ua_marks[m].rect.setCoords(offs, l1, offs, l2);
        m++;
      }
      countMini = m - countMini;
    }
    else
      countMini = 0;
    Q_ASSERT(m <= countMaxiTotal + (countMaxiTotal-1)*miniPerMaxi);
//    qDebug()<<m<<countMaxiTotal + (countMaxiTotal-1)*miniPerMaxi;
  }
};
MarginEXMarks::~MarginEXMarks()
{
  if (ua_marks)
    delete []ua_marks;
  if (texts)
    delete []texts; 
}


/**************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define RAKOFLAG

class MarginCallback;
class MarginElement;
class DrawBars_impl
{
public:
  DrawBars_impl(): 
    c_mirroredHorz(false), c_mirroredVert(false),
    main_opacity(0.0f), drawBoundsUpdater(false), drawCoreInited(false)
  {
  }
  
  int           c_hint_draw_width, c_hint_draw_height;
  bool          c_mirroredHorz, c_mirroredVert;
  
  int           c_width, c_width_dva, c_width_dvb;
  int           c_height, c_height_dva, c_height_dvb;
  QMargins      c_margins;
  int           c_scalingHorz, c_scalingVert;
  
  bool          expandNeighborBarsIfNeed;
#ifdef RAKOFLAG
  bool          rakoflag;
#endif
  
  int           clr_policy;
  bool          c_back_fill;
  QBrush        c_back_brush, c_front_brush;
  QPen          c_front_pen;
  
  float         main_opacity;
  
  bool          drawBoundsUpdater;
  
  struct        
  {
    int         summ;
    int         segm_minimal;
    int         segm_pre;
    int         segm_post;
    int         c_size;
  }             ttr[4];
public:
  enum  { MIRALG_NONE, MIRALG_IGNORE, MIRALG_INVERT };
  
  struct        melem_t
  {
    MarginElement*  pme;
    MEQWrapper*     pwp;
    int             offset, length;
    bool            shared, visible, ivbanned;
    int             miralg;
    melem_t(){}
    melem_t(MarginElement* _pme, MEQWrapper* _pwp, bool isshared, bool interventBanned, int _miralg): 
      pme(_pme), pwp(_pwp), offset(0), length(0), 
      shared(isshared), visible(true), ivbanned(interventBanned), miralg(_miralg) {}
  };
  QVector<melem_t>   elems[4];
  QVector<MarginCallback*>    elemsBoundDepended;
  QVector<MarginCallback*>    elemsScrollDepended;
  QVector<MarginComplicated*> elemsBoundsSetupDepended;
  typedef QVector<melem_t>::iterator melem_iterator_t;
  bool  drawCoreInited;
  
  void        clearTTR(ATTACHED_TO atto)
  {
    ttr[atto].summ = ttr[atto].segm_minimal = ttr[atto].segm_pre = ttr[atto].segm_post = /*ttr[atto].c_dly = */ttr[atto].c_size = 0;
  }
  void        clearTTR()
  {
    for (int i=0; i<4; i++)
      clearTTR(ATTACHED_TO(i));
  }
  void        updateTTR()
  {
    ttr[AT_LEFT].c_size = qMax(ttr[AT_LEFT].summ, qMax(ttr[AT_TOP].segm_pre, ttr[AT_BOTTOM].segm_pre));
    ttr[AT_RIGHT].c_size = qMax(ttr[AT_RIGHT].summ, qMax(ttr[AT_TOP].segm_post, ttr[AT_BOTTOM].segm_post));
    ttr[AT_TOP].c_size = qMax(ttr[AT_TOP].summ, qMax(ttr[AT_LEFT].segm_pre, ttr[AT_RIGHT].segm_pre));
    ttr[AT_BOTTOM].c_size = qMax(ttr[AT_BOTTOM].summ, qMax(ttr[AT_LEFT].segm_post, ttr[AT_RIGHT].segm_post));
  }
  
  void        reupdateBars(int UF_LEFT, int UF_RIGHT, int UF_TOP, int UF_BOTTOM) // Any cttr changed -> all areas need to be updated
  {
    MarginElement::uarea_t areaVert = { AT_LEFT, 
                                        0,0, 
          ttr[AT_TOP].c_size + c_height_dva, c_height - ttr[AT_TOP].c_size - ttr[AT_BOTTOM].c_size - c_height_dva - c_height_dvb, ttr[AT_BOTTOM].c_size + c_height_dvb,
                                        c_height,
                                        c_mirroredVert, c_scalingVert
                                      };
    
    if (UF_LEFT != -1 && areaVert.segm_main > 0)
      for (int i=0; i<elems[0].count(); i++)
      {
        int apt = ttr[AT_LEFT].c_size - 1 - elems[0][i].offset;
        areaVert.atto_begin = apt;
        areaVert.atto_end = apt - elems[0][i].length;
        areaVert.mirrored = elems[0][i].miralg == MIRALG_IGNORE? elems[0][i].miralg == MIRALG_INVERT? true : false : elems[0][i].miralg == MIRALG_INVERT? !c_mirroredVert : c_mirroredVert;
        elems[0][i].pme->updateArea(areaVert, UF_LEFT);
      }
    
    areaVert.atto = AT_RIGHT;
    
    if (UF_RIGHT != -1 && areaVert.segm_main > 0)
      for (int i=0; i<elems[1].count(); i++)
      {
//        int apt = c_width - 1 - ttr[AT_RIGHT].c_size + 1 + elems[1][i].offset;    DAFUQ + 1
        int apt = c_width - 1 - ttr[AT_RIGHT].c_size + elems[1][i].offset;
        areaVert.atto_begin = apt;
        areaVert.atto_end = apt + elems[1][i].length;
        areaVert.mirrored = elems[1][i].miralg == MIRALG_IGNORE? elems[1][i].miralg == MIRALG_INVERT? true : false : elems[1][i].miralg == MIRALG_INVERT? !c_mirroredVert : c_mirroredVert;
        elems[1][i].pme->updateArea(areaVert, UF_RIGHT);
      }
    
    MarginElement::uarea_t areaHorz = { AT_TOP, 
                                        0,0, 
            ttr[AT_LEFT].c_size + c_width_dva, c_width - ttr[AT_LEFT].c_size - ttr[AT_RIGHT].c_size - c_width_dva - c_width_dvb, ttr[AT_RIGHT].c_size + c_width_dvb,
                                        c_width, 
                                        c_mirroredHorz, c_scalingHorz
                                      };
    
    if (UF_TOP != -1 && areaHorz.segm_main > 0)
      for (int i=0; i<elems[2].count(); i++)
      {
        int apt = ttr[AT_TOP].c_size - 1 - elems[2][i].offset;
        areaHorz.atto_begin = apt;
        areaHorz.atto_end = apt - elems[2][i].length;
        areaHorz.mirrored = elems[2][i].miralg == MIRALG_IGNORE? elems[2][i].miralg == MIRALG_INVERT? true : false : elems[2][i].miralg == MIRALG_INVERT? !c_mirroredHorz : c_mirroredHorz;
        elems[2][i].pme->updateArea(areaHorz, UF_TOP);
      }
    
    areaHorz.atto = AT_BOTTOM;
    
    if (UF_BOTTOM != -1 && areaHorz.segm_main > 0)
      for (int i=0; i<elems[3].count(); i++)
      {
//        int apt = c_height - 1 - ttr[AT_BOTTOM].c_size + 1 + elems[3][i].offset;    DAFUQ + 1
        int apt = c_height - 1 - ttr[AT_BOTTOM].c_size + elems[3][i].offset;
        areaHorz.atto_begin = apt;
        areaHorz.atto_end = apt + elems[3][i].length;
        areaHorz.mirrored = elems[3][i].miralg == MIRALG_IGNORE? elems[3][i].miralg == MIRALG_INVERT? true : false : elems[3][i].miralg == MIRALG_INVERT? !c_mirroredHorz : c_mirroredHorz;
        elems[3][i].pme->updateArea(areaHorz, UF_BOTTOM);
      }
  }
  
  void        reupdateBars_oneSide(ATTACHED_TO atto)
  {
    int flags[] = { -1, -1, -1, -1 };
    flags[atto] = MarginElement::UF_FORCED;
    reupdateBars(flags[0], flags[1], flags[2], flags[3]);
  }
  
  void        reupdateBars_symmetric(ATTACHED_TO atto)
  {
    const int sym[] = { AT_RIGHT, AT_LEFT, AT_BOTTOM, AT_TOP };
    int flags[] = { -1, -1, -1, -1 };
    flags[atto] = MarginElement::UF_FORCED;
    flags[sym[atto]] = MarginElement::UF_FORCED;
    reupdateBars(flags[0], flags[1], flags[2], flags[3]);
  }
  
  void        reupdateBars_forced()
  {
    reupdateBars(MarginElement::UF_FORCED, MarginElement::UF_FORCED, MarginElement::UF_FORCED, MarginElement::UF_FORCED);
  }
  
  
  void    recalcTTRforBar(ATTACHED_TO atto, int* delta_summ=nullptr, int* delta_segm=nullptr)
  {
    int total=0, total_prev=0;
    int segm_min=-1;
    int maxsegm_pre=0, maxsegm_post=0;
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
        int atto_size, minsegm_main, cursegm_pre, cursegm_post;
        iter->pme->sizeHint(atto, &atto_size, &minsegm_main, &cursegm_pre, &cursegm_post);
        iter->offset = total;
        iter->length = atto_size;
        total_prev = total;
        total += atto_size;
        if (iter->ivbanned == false)
        {
          if (maxsegm_pre < cursegm_pre)  maxsegm_pre = cursegm_pre;
          if (maxsegm_post < cursegm_post)  maxsegm_post = cursegm_post;
        }
      }
#else
      int atto_size, cursegm_main, cursegm_pre, cursegm_post;
      iter->pme->sizeHint(atto, &atto_size, &cursegm_main, &cursegm_pre, &cursegm_post);
      
      if (iter->shared/* && total != 0*/)
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
        if (segm_min < cursegm_main)
          segm_min = cursegm_main;
        if (iter->ivbanned == false)
        {
          if (maxsegm_pre < cursegm_pre)  maxsegm_pre = cursegm_pre;
          if (maxsegm_post < cursegm_post)  maxsegm_post = cursegm_post;
        }
      }
#endif
    }
    if (delta_summ) *delta_summ = total - ttr[atto].summ;
    if (delta_segm) *delta_segm = segm_min - ttr[atto].segm_minimal;
    ttr[atto].summ = total;
    ttr[atto].segm_minimal = segm_min;
    ttr[atto].segm_pre = maxsegm_pre;
    ttr[atto].segm_post = maxsegm_post;
    updateTTR();
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
    
    c_back_fill = true;
    c_back_brush = clr_back;
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
  
  void  updatePalette(QWidget* wdg, const QColor& clr_front, bool getback=false)
  {
    QPalette pal(wdg->palette());
    pal.setColor(QPalette::WindowText, clr_front);
    wdg->setPalette(pal);
    
    c_back_fill = getback;
    if (c_back_fill)
      c_back_brush = pal.color(QPalette::Window);
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
  
  void  upbackPalette(QWidget* wdg)
  {
    c_back_fill = false;
//    c_back_brush = wdg->palette().color(QPalette::Window);
    QColor clr_front = wdg->palette().color(QPalette::WindowText);
    c_front_brush = clr_front;
    c_front_pen = clr_front;
  }
};

#define COLOR3(clr) (clr) & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF

inline bool isDrawPaletteCP(int cp)
{
  return cp == DrawBars::CP_FROM_DRAWBACK || cp == DrawBars::CP_FROM_DRAWPALETTE || cp == DrawBars::CP_FROM_DRAWPALETTE_INV;
}


DrawBars::DrawBars(DrawQWidget* pdraw, COLORS colorsPolicy, QWidget *parent) : QWidget(parent), pDraw(pdraw), pTool(nullptr)
{
  this->setAutoFillBackground(false);
  pImpl = new DrawBars_impl;
  setColorPolicy(colorsPolicy);

//  pImpl->rakoflag = false;
  pImpl->clearTTR();

  pDraw->setParent(this);
  pDraw->move(pImpl->ttr[AT_LEFT].summ, pImpl->ttr[AT_TOP].summ);
  pDraw->show();
  
  pImpl->c_mirroredHorz = orientationMirroredHorz(pDraw->orientation());
  pImpl->c_mirroredVert = orientationMirroredVert(pDraw->orientation());
  
  QSize sdraw = pDraw->size()/* * pDraw->devicePixelRatio()*/;
  pImpl->c_hint_draw_width = sdraw.width();
  pImpl->c_hint_draw_height = sdraw.height();
  
  pImpl->c_width = pImpl->c_height = 0;
  
  pImpl->c_margins = QMargins(0,0,0,0);
  
  pImpl->c_scalingHorz = pDraw->scalingHorz();
  pImpl->c_scalingVert = pDraw->scalingVert();
  
#ifdef RAKOFLAG
  pImpl->rakoflag = true;
#endif
  
  
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
  if (isDrawPaletteCP(cp) && pDraw->dataPalette()!= nullptr)
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

void DrawBars::setOpacity(float opacity)
{
  pImpl->main_opacity = opacity;
}

void DrawBars::enableDrawBoundsUpdater(bool v)
{
  pImpl->drawBoundsUpdater = v;
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

DrawQWidget*  DrawBars::getDraw(){ return pDraw;   }
DrawQWidget*  DrawBars::draw(){ return pDraw;   }
const DrawQWidget*  DrawBars::getDraw() const { return pDraw; }
const DrawQWidget*  DrawBars::draw() const { return pDraw; }

QRect DrawBars::getDrawGeometry() const
{
  return QRect(pImpl->c_margins.left() + pImpl->ttr[AT_LEFT].c_size, pImpl->c_margins.top() + pImpl->ttr[AT_TOP].c_size, pDraw->width(), pDraw->height());
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




MEQWrapper*   DrawBars::addMarginElement(ATTACHED_TO atto, MarginElement* pme, MEQWrapper* pwp, bool sharedWithPrev, bool interventBanned, int miralg)
{
//  bool cttr_updated = true;
  ATTACHED_TO atto_was = atto;
  if (pImpl->excludeElement(pme, &atto_was))
    pImpl->recalcTTRforBar(atto_was);
  
  pme->relatedInit(pDraw);
  pImpl->elems[int(atto)].push_back(DrawBars_impl::melem_t(pme, pwp, sharedWithPrev, interventBanned, miralg));
  pImpl->recalcTTRforBar(atto);
  pImpl->reupdateBars_oneSide(atto);
  
#ifdef RAKOFLAG
  if (pImpl->rakoflag == false)
#endif
  {
    rollbackGeometry();
  }
  
  if (pwp != nullptr)
  {
    pwp->m_pme = pme;
    pwp->m_premote = this;
  }
//    updateGeometry();
  return pwp;
}





inline int countMaxNumbers(int marksLimit)
{
  static int power[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };
  static const int pcount = sizeof(power)/sizeof(unsigned int);
  for (int i=0; i<pcount; i++)
    if (marksLimit < power[i])
      return i;
  return -1;
}

#define   MARG_OPTS_TEXT    if (flags & _DBF_PRECISION_GROUP) \
                            { \
                              int pp = ((flags & _DBF_PRECISION_GROUP) >> 16) - 1; \
                              const int precision_relatives[] = { 1,2,-1,2 }; \
                              if (pp < 4)   pmarg->numformatOffset(precision_relatives[pp]); \
                              else          pmarg->numformatFix(pp - 4); \
                            } \
                            flags & DBF_POSTFIX_TO_PREFIX? pmarg->setPrefix(postfix) : pmarg->setPostfix(postfix); \
                            pmarg->setPostfixPositions(flags & DBF_POSTFIX_ONLYLAST? MarginTextformat::PEX_LAST : MarginTextformat::PEX_ALL); \
                            pmarg->setFont(this->font());

#define   EXTRACT_MIRALG(f)  ((f >> 1) & 0x3)

#define DB_ROUNDING(flags)  (flags & DBF_MARKS_ROUNDING_ON? 0 : flags & DBF_MARKS_ROUNDING_OFF_INC? 2 : 1)
#define DB_NATIVE(flags)  ((flags & 0xF00000) == DBF_NATIVE_DIV_10 ? 2: (flags & 0xF00000) == DBF_NATIVE_DIV_15_5_3 ? 1: 0)

MEWLabel* DrawBars::addLabel(ATTACHED_TO atto, int flags, QString text, Qt::Alignment align, Qt::Orientation orient)
{
  MarginLabel*  pmarg = new MarginLabel(text, this->font(), flags & DBF_LABELAREA_FULLBAR, align, orient);
  return (MEWLabel*)addMarginElement(atto, pmarg, new MEWLabel, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWSpace* DrawBars::addSpace(ATTACHED_TO atto, int space)
{
  MarginElement*  pmarg = new MarginSpace(space);
  return (MEWSpace*)addMarginElement(atto, pmarg, new MEWSpace, false, false, 0);
}

MEWColoredSpace* DrawBars::addSpace(ATTACHED_TO atto, int space, QColor color, bool maxzone)
{
  MarginElement*  pmarg = new MarginColoredSpace(space, color, maxzone);
  return (MEWColoredSpace*)addMarginElement(atto, pmarg, new MEWColoredSpace, false, false, 0);
}

MEWSpace* DrawBars::addContour(ATTACHED_TO atto, int space, bool maxzone)
{
  MarginElement*  pmarg = new MarginContour(space, maxzone);
  return (MEWSpace*)addMarginElement(atto, pmarg, new MEWSpace, false, false, 0);
}

MEWSpace*DrawBars::addContour(ATTACHED_TO atto, int space, QColor color, bool maxzone)
{
  MarginElement*  pmarg = new MarginContour(space, color, maxzone);
  return (MEWSpace*)addMarginElement(atto, pmarg, new MEWSpace, false, false, 0);
}




MEWPointer* DrawBars::addPointerRelativeOwnbounds(ATTACHED_TO atto, int flags, float pos, float LL, float HL, int marklen, float MOD, const char* postfix)
{
  MarginPointer*  pmarg = new MarginPointer(DB_ROUNDING(flags), marklen, pos, false, pDraw->orientation(), flags & DBF_NOTESINSIDE, Qt::AlignCenter);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  return (MEWPointer*)addMarginElement(atto, pmarg, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWPointer*DrawBars::addPointerAbsoluteOwnbounds(ATTACHED_TO atto, int flags, float pos, float LL, float HL, int marklen, float MOD, const char* postfix)
{
  MarginPointer*  pmarg = new MarginPointer(DB_ROUNDING(flags), marklen, pos, true, pDraw->orientation(), flags & DBF_NOTESINSIDE, Qt::AlignCenter);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  return (MEWPointer*)addMarginElement(atto, pmarg, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWPointer* DrawBars::addPointerRelativeDrawbounds(ATTACHED_TO atto, int flags, float pos, int marklen, float MOD, const char* postfix)
{
  MarginPointer*  pmarg = new MarginPointer(DB_ROUNDING(flags), marklen, pos, false, pDraw->orientation(), flags & DBF_NOTESINSIDE, Qt::AlignCenter);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  pImpl->elemsBoundDepended.push_back(pmarg);
  return (MEWPointer*)addMarginElement(atto, pmarg, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWPointer*DrawBars::addPointerAbsoluteDrawbounds(ATTACHED_TO atto, int flags, float pos, int marklen, float MOD, const char* postfix)
{
  MarginPointer*  pmarg = new MarginPointer(DB_ROUNDING(flags), marklen, pos, true, pDraw->orientation(), flags & DBF_NOTESINSIDE, Qt::AlignCenter);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  pImpl->elemsBoundDepended.push_back(pmarg);
  return (MEWPointer*)addMarginElement(atto, pmarg, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWPointer* DrawBars::addEPointer01Auto(ATTACHED_TO atto, int flags, float pos, int marklen, const char* postfix)
{
  MarginPointer*  pmarg = new MarginPointer(DB_ROUNDING(flags), marklen, pos, false, pDraw->orientation(), flags & DBF_NOTESINSIDE, Qt::AlignCenter);
  MARG_OPTS_TEXT
  int sizeDimm = BAR_VERT[atto]? int(pDraw->sizeDataVert() + 1) : int(pDraw->sizeDataHorz() + 1);
  pmarg->bdContentUpdate(RF_SETENUMERATE, relatedopts_t(sizeDimm-1, flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1)); // ?????
  return (MEWPointer*)addMarginElement(atto, pmarg, new MEWPointer, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

////////////////////////////

MEWScale* DrawBars::addScalePixstepEmpty(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, int miniPerMaxi)
{
  MarginINMarks*  pmarg = new MarginINMarks();
  pmarg->init(DBMODE_STRETCHED, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*minSpacing;
    else                    pImpl->c_hint_draw_width = marksLimit*minSpacing;
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScaleSymmetricEmpty(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, int miniPerMaxi)
{
  MarginINMarks*  pmarg = new MarginINMarks();
  pmarg->init(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*minSpacing;
    else                    pImpl->c_hint_draw_width = marksLimit*minSpacing;
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScaleRollingEmpty(ATTACHED_TO atto, int flags, int marksLimit, int pixStep, int miniPerMaxi)
{
  MarginINMarks*  pmarg = new MarginINMarks();
  pmarg->init(DBMODE_STRETCHED_POW2, marksLimit, pixStep, DB_ROUNDING(flags), miniPerMaxi);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*pixStep;
    else                    pImpl->c_hint_draw_width = marksLimit*pixStep;
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}



MEWScaleNN*   DrawBars::addScaleNativeOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, float minSTEP, float minSTEPbase, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginEXMarks* pmarg = new MarginEXMarks(flags & DBF_NOTESINSIDE);
  pmarg->init(marksLimit, minSTEP, minSTEPbase, minSpacing, DB_ROUNDING(flags), miniPerMaxi, DB_NATIVE(flags));
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*minSpacing;
    else                    pImpl->c_hint_draw_width = marksLimit*minSpacing;
//    updateGeometry();
  }
//  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScaleNN*)addMarginElement(atto, pmarg, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNN* DrawBars::addScalePixstepOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg;
  if (flags & DBF_NOTE_BORDERS_ONLY)
    pmarg = new MarginINMarks2only(flags & DBF_NOTESINSIDE);
  else
    pmarg = new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);

  pmarg->init(DBMODE_STRETCHED, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*minSpacing;
    else                    pImpl->c_hint_draw_width = marksLimit*minSpacing;
//    updateGeometry();
  }
//  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScaleNN*)addMarginElement(atto, pmarg, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNN* DrawBars::addScaleSymmetricOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg;
  if (flags & DBF_NOTE_BORDERS_ONLY)
    pmarg = new MarginINMarks2only(flags & DBF_NOTESINSIDE);
  else
    pmarg = new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);

  pmarg->init(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*minSpacing;
    else                    pImpl->c_hint_draw_width = marksLimit*minSpacing;
//    updateGeometry();
  }
//  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScaleNN*)addMarginElement(atto, pmarg, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNN* DrawBars::addScaleRollingOwnbounds(ATTACHED_TO atto, int flags, float LL, float HL, int marksLimit, int pixStep, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg;
  if (flags & DBF_NOTE_BORDERS_ONLY)
    pmarg = new MarginINMarks2only(flags & DBF_NOTESINSIDE);
  else
    pmarg = new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);

  pmarg->init(DBMODE_STATIC, marksLimit, pixStep, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(bounds_t(LL, HL), MOD));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*pixStep;
    else                    pImpl->c_hint_draw_width = marksLimit*pixStep;
//    updateGeometry();
  }
//  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScaleNN*)addMarginElement(atto, pmarg, new MEWScaleNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScaleNativeDrawbounds(ATTACHED_TO atto, int flags, float minSTEP, float minSTEPbase, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginEXMarks* pmarg = new MarginEXMarks(flags & DBF_NOTESINSIDE);
  pmarg->init(marksLimit, minSTEP, minSTEPbase, minSpacing, DB_ROUNDING(flags), miniPerMaxi, DB_NATIVE(flags));
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  if (flags & DBF_MINSIZE_BY_MINSPACING)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(minSpacing);
    else                    pImpl->c_hint_draw_width = marksLimit*(minSpacing);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(/*(MarginCallback*)*/pmarg);
  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScalePixstepDrawbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                  (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STRETCHED, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  if (flags & DBF_MINSIZE_BY_MINSPACING)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(minSpacing);
    else                    pImpl->c_hint_draw_width = marksLimit*(minSpacing);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(/*(MarginCallback*)*/pmarg);
  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScaleSymmetricDrawbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                  (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  if (flags & DBF_MINSIZE_BY_MINSPACING)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(minSpacing);
    else                    pImpl->c_hint_draw_width = marksLimit*(minSpacing);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(/*(MarginCallback*)*/pmarg);
  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addScaleRollingDrawbounds(ATTACHED_TO atto, int flags, int marksLimit, int pixStep, int miniPerMaxi, float MOD, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                        (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STATIC, marksLimit, pixStep, DB_ROUNDING(flags), miniPerMaxi);
  MARG_OPTS_TEXT
  pmarg->bdContentUpdate(RF_SETBOUNDS, relatedopts_t(pDraw->bounds(), MOD));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(pixStep);
    else                    pImpl->c_hint_draw_width = marksLimit*(pixStep);
//    updateGeometry();
  }
  pImpl->elemsBoundDepended.push_back(/*(MarginCallback*)*/pmarg);
  pImpl->elemsBoundsSetupDepended.push_back(pmarg);
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}


//MEWScaleNM* DrawBars::addEScaleNativeOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step, const char* postfix)
//{
//  MarginEXMarks* pmarg = new MarginEXMarks();
//  pmarg->init(marksLimit, step, 0, minSpacing, DB_ROUNDING(flags), 0);
//                 flags & DBF_ENUMERATE_SHOWLAST, flags & DBF_DOCKTO_PREVMARK? 0 : flags & DBF_DOCKTO_NEXTMARK? 2 : 1);
//  MARG_OPTS_TEXT
  
//  pmarg->bdContentUpdate(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(marksLimit-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
//  if (flags & DBF_MINSIZE_BY_PIXSTEP)
//  {
//    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
//    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
////    updateGeometry();
//  }
//  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
//}

MEWScaleNM* DrawBars::addEScalePixstepOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  pmarg->init_wide(DBMODE_STRETCHED, marksLimit, minSpacing, step, DB_ROUNDING(flags), 
                 flags & DBF_ENUMERATE_SHOWLAST, flags & DBF_DOCKTO_PREVMARK? 0 : flags & DBF_DOCKTO_NEXTMARK? 2 : 1);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(marksLimit-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNM* DrawBars::addEScaleRollingOwnbounds(ATTACHED_TO atto, int flags, int marksLimit, int minSpacing, unsigned int step, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  pmarg->init_wide(DBMODE_STATIC, marksLimit, minSpacing, step, DB_ROUNDING(flags), 
                 flags & DBF_ENUMERATE_SHOWLAST, flags & DBF_DOCKTO_PREVMARK? 0 : flags & DBF_DOCKTO_NEXTMARK? 2 : 1);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(marksLimit-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScale* DrawBars::addEScalePixstepDrawbounds(ATTACHED_TO atto, int flags, int minSpacing, unsigned int step, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  MARG_OPTS_TEXT
  int sizeDimm = BAR_VERT[atto]? int(pDraw->sizeDataVert() + 1) : int(pDraw->sizeDataHorz() + 1);
  pmarg->init_wide(DBMODE_STRETCHED, sizeDimm, minSpacing, step, DB_ROUNDING(flags), flags & DBF_ENUMERATE_SHOWLAST, flags & DBF_DOCKTO_PREVMARK? DOCK_PREV : flags & DBF_DOCKTO_NEXTMARK? DOCK_NEXT : DOCK_BETWEEN);
  pmarg->bdContentUpdate(RF_SETENUMERATE, relatedopts_t(countMaxNumbers(sizeDimm-1), flags & DBF_ENUMERATE_FROMZERO? 0 : 1, -1));
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (sizeDimm-1)*(minSpacing);
    else                    pImpl->c_hint_draw_width = (sizeDimm)*(minSpacing);
//    updateGeometry();
  }
  return (MEWScale*)addMarginElement(atto, pmarg, new MEWScale, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}






MEWScaleTAPNN* DrawBars::addScalePixstepTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int minSpacing, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                        (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STRETCHED, marksLimit, minSpacing, DB_ROUNDING(flags), 0);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(minSpacing);
    else                    pImpl->c_hint_draw_width = marksLimit*(minSpacing);
//    updateGeometry();
  }
  return (MEWScaleTAPNN*)addMarginElement(atto, pmarg, new MEWScaleTAPNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNN* DrawBars::addScaleSymmetricTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int minSpacing, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                        (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, DB_ROUNDING(flags), 0);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(minSpacing);
    else                    pImpl->c_hint_draw_width = marksLimit*(minSpacing);
//    updateGeometry();
  }
  return (MEWScaleTAPNN*)addMarginElement(atto, pmarg, new MEWScaleTAPNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNN* DrawBars::addScaleRollingTapNN(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int pixStep, const char* postfix)
{
  MarginINMarks* pmarg = flags & DBF_NOTE_BORDERS_ONLY? (MarginINMarks*)new MarginINMarks2only(flags & DBF_NOTESINSIDE) : 
                                                        (MarginINMarks*)new MarginINMarksTextToMark(flags & DBF_NOTESINSIDE);
  pmarg->init(DBMODE_STATIC, marksLimit, pixStep, DB_ROUNDING(flags), 0);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = marksLimit*(pixStep);
    else                    pImpl->c_hint_draw_width = marksLimit*(pixStep);
//    updateGeometry();
  }
  if ((flags & DBF_NO_RETAP_ON_SCROLL) == 0)
    pImpl->elemsScrollDepended.push_back(pmarg);
  return (MEWScaleTAPNN*)addMarginElement(atto, pmarg, new MEWScaleTAPNN, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}


MEWScaleTAPNM* DrawBars::addScalePixstepTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int minSpacing, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  pmarg->init_wide(DBMODE_STRETCHED, marksLimit, minSpacing, 1, DB_ROUNDING(flags), true, flags & DBF_DOCKTO_PREVMARK? DOCK_PREV : flags & DBF_DOCKTO_NEXTMARK? DOCK_NEXT : DOCK_BETWEEN);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNM* DrawBars::addScaleSymmetricTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int minSpacing, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  pmarg->init_wide(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, 1, DB_ROUNDING(flags), true, flags & DBF_DOCKTO_PREVMARK? DOCK_PREV : flags & DBF_DOCKTO_NEXTMARK? DOCK_NEXT : DOCK_BETWEEN);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNM* DrawBars::addScaleRollingTapNM(ATTACHED_TO atto, int flags, mtap_qstring_fn fn, int maxtextlen, const void* param, int marksLimit, int pixStep, const char* postfix)
{
  MarginINMarksTextBetweenMark* pmarg = new MarginINMarksTextBetweenMark();
  pmarg->init_wide(DBMODE_STATIC, marksLimit, pixStep, 1, DB_ROUNDING(flags), true, flags & DBF_DOCKTO_PREVMARK? DOCK_PREV : flags & DBF_DOCKTO_NEXTMARK? DOCK_NEXT : DOCK_BETWEEN);
  MARG_OPTS_TEXT
  
  pmarg->bdContentUpdate(RF_SETTAPS, relatedopts_t(fn, param, maxtextlen));
  pmarg->setUpdateOnSetDimm(flags & DBF_RETAP_ON_RESIZE);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*pixStep;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*pixStep;
//    updateGeometry();
  }
  if ((flags & DBF_NO_RETAP_ON_SCROLL) == 0)
    pImpl->elemsScrollDepended.push_back(pmarg);
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}


MEWScaleTAPNM* DrawBars::addWScalePixstepTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param, int marksLimit, int minSpacing)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STRETCHED, marksLimit, minSpacing, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, fn, param, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNM* DrawBars::addWScaleSymmetricTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param, int marksLimit, int minSpacing)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STRETCHED_POW2, marksLimit, minSpacing, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, fn, param, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleTAPNM* DrawBars::addWScaleRollingTapNM(ATTACHED_TO atto, int flags, mtap_qwidget_fn fn, int maxperpendiculardimm, void* param, int marksLimit, int pixStep)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STATIC, marksLimit, pixStep, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, fn, param, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksLimit-1)*pixStep;
    else                    pImpl->c_hint_draw_width = (marksLimit-1)*pixStep;
//    updateGeometry();
  }
  return (MEWScaleTAPNM*)addMarginElement(atto, pmarg, new MEWScaleTAPNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}


MEWScaleNM* DrawBars::addWScalePixstepSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int minSpacing)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STRETCHED, marksNwidgetsCount, minSpacing, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, wdgs, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksNwidgetsCount-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksNwidgetsCount-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNM* DrawBars::addWScaleSymmetricSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int minSpacing)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STRETCHED_POW2, marksNwidgetsCount, minSpacing, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, wdgs, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksNwidgetsCount-1)*minSpacing;
    else                    pImpl->c_hint_draw_width = (marksNwidgetsCount-1)*minSpacing;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
}

MEWScaleNM* DrawBars::addWScaleRollingSetNM(ATTACHED_TO atto, int flags, int maxperpendiculardimm, int marksNwidgetsCount, QWidget* wdgs[], int pixStep)
{
  MarginINMarksWidgetBetween* pmarg = new MarginINMarksWidgetBetween();
  pmarg->init_wide(DBMODE_STATIC, marksNwidgetsCount, pixStep, 1, DB_ROUNDING(flags), true, maxperpendiculardimm, wdgs, this);
  if (flags & DBF_MINSIZE_BY_PIXSTEP)
  {
    if (BAR_VERT[atto])     pImpl->c_hint_draw_height = (marksNwidgetsCount-1)*pixStep;
    else                    pImpl->c_hint_draw_width = (marksNwidgetsCount-1)*pixStep;
//    updateGeometry();
  }
  return (MEWScaleNM*)addMarginElement(atto, pmarg, new MEWScaleNM, flags & DBF_SHARED, flags & DBF_INTERVENTBANNED, EXTRACT_MIRALG(flags));
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
          int space=0, dly,segm_pre,segm_post;
          iter->pme->sizeHint(ATTACHED_TO(d), &space, &dly, &segm_pre, &segm_post);
          iter->pme = new MarginSpace(space);   /// ntf!
          update();
        }
        else
        {
          pImpl->elems[d].erase(iter);
          int delta_summ, delta_segm;
          pImpl->recalcTTRforBar(ATTACHED_TO(d), &delta_summ, &delta_segm);
          
          QSize pr = QSize(pImpl->c_width + (d == 0 || d == 1? delta_summ : 0) , pImpl->c_height + (d == 2 || d == 3? delta_summ : 0));
          pImpl->c_width = pr.width();
          pImpl->c_height = pr.height();
          pImpl->reupdateBars_oneSide(ATTACHED_TO(d));
          
          PDRAWMOVE;
          updateGeometry();
          resize(pr);
//          update();
        }
        return;
      }
}

void DrawBars::setMElementVisible(MEQWrapper* pwp, bool v, bool autoupdate)
{
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pwp == pwp)
      {
        iter->visible = v;
        if (autoupdate)
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
        
        pImpl->reupdateBars_symmetric(ATTACHED_TO(d));
        PDRAWMOVE;
        update();
        return;
      }
}

void DrawBars::swapBars(ATTACHED_TO atto)
{
  ATTACHED_TO switched[] = {  AT_RIGHT, AT_LEFT, AT_BOTTOM, AT_TOP   };
  ATTACHED_TO atto_sw = switched[atto];
  qSwap(pImpl->elems[atto], pImpl->elems[atto_sw]);
  pImpl->recalcTTRforBar(atto);
  pImpl->recalcTTRforBar(atto_sw);
  pImpl->reupdateBars_symmetric(atto);
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
    rollbackGeometry();
//    pDraw->Geometry(pImpl->c_margins.left(), pImpl->c_margins.top(), 
//                       this->width() - pImpl->c_margins.left() - pImpl->c_margins.right(), 
//                       this->height() - pImpl->c_margins.top() - pImpl->c_margins.bottom());
//    updateGeometry();
  }
  update();
}

void DrawBars::mouseEvent(MarginElement::MOUSEEVENT mev, int x, int y)
{
  if (x < pImpl->c_margins.left() || y < pImpl->c_margins.top())
    return;
  x -= pImpl->c_margins.left();
  y -= pImpl->c_margins.top();
  if (x > pImpl->c_width || y > pImpl->c_height)
    return;
  
  bool doUpdate = false;
  if (x < pImpl->ttr[AT_LEFT].c_size)
  {
    int at = AT_LEFT;
    x = pImpl->ttr[AT_LEFT].c_size - 1 - x;
    for (int i=0; i<pImpl->elems[at].count(); i++)
    {
      if (pImpl->elems[at][i].visible && x >= pImpl->elems[at][i].offset && x < pImpl->elems[at][i].offset + pImpl->elems[at][i].length)
        pImpl->elems[at][i].pme->mouseEvent(mev, y, x - pImpl->elems[at][i].offset, pImpl->c_height, pImpl->elems[at][i].length, &doUpdate, pImpl->elems[at][i].pwp);
    }
  }
  if (x > pImpl->c_width - pImpl->ttr[AT_RIGHT].c_size)
  {
    int at = AT_RIGHT;
    x = x - (pImpl->c_width - pImpl->ttr[AT_RIGHT].c_size);
    for (int i=0; i<pImpl->elems[at].count(); i++)
    {
      if (pImpl->elems[at][i].visible && x >= pImpl->elems[at][i].offset && x < pImpl->elems[at][i].offset + pImpl->elems[at][i].length)
        pImpl->elems[at][i].pme->mouseEvent(mev, y, x - pImpl->elems[at][i].offset, pImpl->c_height, pImpl->elems[at][i].length, &doUpdate, pImpl->elems[at][i].pwp);
    }
  }
  if (y < pImpl->ttr[AT_TOP].c_size)
  {
    int at = AT_TOP;
    y = pImpl->ttr[AT_TOP].c_size - 1 - y;
    for (int i=0; i<pImpl->elems[at].count(); i++)
    {
      if (pImpl->elems[at][i].visible && y >= pImpl->elems[at][i].offset && y < pImpl->elems[at][i].offset + pImpl->elems[at][i].length)
        pImpl->elems[at][i].pme->mouseEvent(mev, x, y - pImpl->elems[at][i].offset, pImpl->c_width, pImpl->elems[at][i].length, &doUpdate, pImpl->elems[at][i].pwp);
    }
  }
  if (y > pImpl->c_height - pImpl->ttr[AT_BOTTOM].c_size)
  {
    int at = AT_BOTTOM;
    y = y - (pImpl->c_height - pImpl->ttr[AT_BOTTOM].c_size);
    for (int i=0; i<pImpl->elems[at].count(); i++)
    {
      if (pImpl->elems[at][i].visible && y >= pImpl->elems[at][i].offset && y < pImpl->elems[at][i].offset + pImpl->elems[at][i].length)
        pImpl->elems[at][i].pme->mouseEvent(mev, x, y - pImpl->elems[at][i].offset, pImpl->c_width, pImpl->elems[at][i].length, &doUpdate, pImpl->elems[at][i].pwp);
    }
  }
  
  if (doUpdate)
    update();
}

void DrawBars::rollbackGeometry()
{
  PDRAWMOVE;
  int height = pDraw->height();
  int hsegm = qMax(pImpl->ttr[AT_LEFT].segm_minimal, pImpl->ttr[AT_RIGHT].segm_minimal);
  if (pImpl->c_hint_draw_height < hsegm)  pImpl->c_hint_draw_height = hsegm;
  if (height < hsegm) height = hsegm;
  if (pImpl->c_hint_draw_height > height)  pImpl->c_hint_draw_height = height;
  
  int width = pDraw->width();
  int wsegm = qMax(pImpl->ttr[AT_TOP].segm_minimal, pImpl->ttr[AT_BOTTOM].segm_minimal);
  if (pImpl->c_hint_draw_width < wsegm)  pImpl->c_hint_draw_width = wsegm;
  if (width < wsegm) width = wsegm;
  if (pImpl->c_hint_draw_width > width)  pImpl->c_hint_draw_width = width;
//  updateGeometry(); // a nado li?
  setGeometry(0,0, 
              pImpl->c_margins.left() + pImpl->ttr[AT_LEFT].c_size + width + pImpl->ttr[AT_RIGHT].c_size + pImpl->c_margins.right(),
              pImpl->c_margins.top() + pImpl->ttr[AT_TOP].c_size + height + pImpl->ttr[AT_BOTTOM].c_size + pImpl->c_margins.bottom());
}

void DrawBars::elemSizeHintChanged(MarginElement* me)
{
  for (int d=0; d<4; d++)
    for (DrawBars_impl::melem_iterator_t iter=pImpl->elems[d].begin(); iter!=pImpl->elems[d].end(); iter++)
      if (iter->pme == me)
      {
        int delta_summ, delta_segm;
        pImpl->recalcTTRforBar(ATTACHED_TO(d), &delta_summ, &delta_segm);
//        PDRAWMOVE;
//        if (delta > 0)
//          updateGeometry();
//        else if (delta < 0)
//          resize(pImpl->ttr[AT_LEFT].c_size + pDraw->width() + pImpl->ttr[AT_RIGHT].c_size, 
//                 pImpl->ttr[AT_TOP].c_size +  pDraw->height() + pImpl->ttr[AT_BOTTOM].c_size);
        
        pImpl->reupdateBars_oneSide(ATTACHED_TO(d));
        rollbackGeometry();
        return;
      }
}
QSize DrawBars::minimumSizeHint() const
{
//  qDebug()<<pImpl->c_hint_draw_height<<pImpl->c_height_margins<<pImpl->ttr[AT_BOTTOM].c_size;
//  qDebug()<<pImpl->ottrLeft<<pImpl->ottrTop<<pImpl->ottrRight<<pImpl->ottrBottom;
  
//  return QSize(pImpl->c_width_margins + pImpl->ttr[AT_LEFT].c_size + pImpl->c_hint_draw_width + pImpl->ttr[AT_RIGHT].c_size, 
//               pImpl->c_height_margins + pImpl->ttr[AT_TOP].c_size + pImpl->c_hint_draw_height + pImpl->ttr[AT_BOTTOM].c_size);
  
  int dw = qMax(pImpl->c_hint_draw_width, int(pDraw->minimumWidth()*pDraw->devicePixelRatio()));
  int dh = qMax(pImpl->c_hint_draw_height, int(pDraw->minimumHeight()*pDraw->devicePixelRatio()));
  return QSize(pImpl->c_margins.left() + pImpl->ttr[AT_LEFT].c_size + dw + pImpl->ttr[AT_RIGHT].c_size + pImpl->c_margins.right(), 
               pImpl->c_margins.top() + pImpl->ttr[AT_TOP].c_size + dh + pImpl->ttr[AT_BOTTOM].c_size + pImpl->c_margins.bottom());
}

//QSize DrawBars::sizeHint() const
//{
//  return pDraw->size() + QSize(pImpl->cttrLeft + pImpl->cttrRight, pImpl->cttrTop + pImpl->cttrBottom);
//}

void DrawBars::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  const QSize esize = event->size();
  
  /***************************************************************************************************************/
  
  QMargins margins = contentsMargins();
  if (margins != pImpl->c_margins) 
  {
    pImpl->c_margins = margins;
    PDRAWMOVE;
    updateGeometry();
  }
  pImpl->c_width = esize.width() - (margins.left() + margins.right());
  pImpl->c_height = esize.height() - (margins.top() + margins.bottom());
  
  QSize dsize(pImpl->c_width -  (pImpl->ttr[AT_LEFT].c_size + pImpl->ttr[AT_RIGHT].c_size),
              pImpl->c_height - (pImpl->ttr[AT_TOP].c_size + pImpl->ttr[AT_BOTTOM].c_size));
  
#ifdef RAKOFLAG
  bool resizeDrawWillAfterBars = pImpl->rakoflag || !pDraw->isVisible();
  pImpl->rakoflag = false;
#else
  bool resizeDrawWillAfterBars = !pDraw->isVisible();
#endif

  dcsizecd_t    dcHorz, dcVert;
  dcgeometry_t  dsHorz, dsVert;
  if (resizeDrawWillAfterBars)
  {
    pDraw->fitSize(dsize.width(), dsize.height(), &dcHorz, &dcVert, &dsHorz, &dsVert);
  }
  else
  {
    pDraw->resize(dsize);
    dsHorz = pDraw->geometryHorz();
    dsVert = pDraw->geometryVert();
    dcHorz = pDraw->sizeComponentsHorz();
    dcVert = pDraw->sizeComponentsVert();
  }  
  
  pImpl->c_scalingHorz = dcHorz.scaling;
  pImpl->c_scalingVert = dcVert.scaling;
  
//  dsize = dsize / pDraw->devicePixelRatio();
//  qDebug()<<pDraw->height()<<pDraw->sizeVert()<<pDraw->minimumHeight()<<dheight<<esize.height();
//  if (this->accessibleName() == "Allo")
//  {
//    qDebug()<<"Allo  "<<resizeDrawWillAfterBars<<event->size()<<dsize<<QSize(dwidth, dheight);
//  }
  
  if (/*isVisible() && */height() > 2 && width() > 2)
  {
    float dpr = pDraw->devicePixelRatio();
    pImpl->c_width_dva = dsHorz.cttr_pre + dsHorz.viewalign_pre/dpr;
    pImpl->c_width_dvb = dsHorz.cttr_post + dsHorz.viewalign_post/dpr;
    pImpl->c_height_dva = dsVert.cttr_pre + dsVert.viewalign_pre/dpr;
    pImpl->c_height_dvb = dsVert.cttr_post + dsVert.viewalign_post/dpr;
    pImpl->reupdateBars(MarginElement::UF_RESIZE, MarginElement::UF_RESIZE, MarginElement::UF_RESIZE, MarginElement::UF_RESIZE);
  }
  else
  {
    pImpl->c_width_dva = pImpl->c_height_dva = pImpl->c_width_dvb = pImpl->c_height_dvb = 0;
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
  
  if (pImpl->c_back_fill)
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
  if (pImpl->main_opacity > 0.0f)
    painter.setOpacity(1.0f - pImpl->main_opacity);
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
  else if (ev->type() == QEvent::LayoutRequest)   /// child widget changed size implicitly
  {
//    qDebug()<<"EJ JO!";
    updateGeometry();
  }
  else
  {
//    if (this->objectName() == "VAU")
//    qDebug()<<ev->type();
  }
  return QWidget::event(ev);
}

void DrawBars::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    this->mouseEvent(event->button() == Qt::LeftButton? MarginElement::ME_LPRESS : MarginElement::ME_RPRESS, event->x(), event->y());
}

void DrawBars::mouseMoveEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    this->mouseEvent(event->button() == Qt::LeftButton? MarginElement::ME_LMOVE: MarginElement::ME_RMOVE, event->x(), event->y());
}

void DrawBars::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    this->mouseEvent(event->button() == Qt::LeftButton? MarginElement::ME_LRELEASE: MarginElement::ME_RRELEASE, event->x(), event->y());
}

void DrawBars::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    int x = event->x(), y = event->y();
    bool inarea = false;
    
    float LL=0, HL=1; int precision=3;
    for (int i=0; i<pImpl->elemsBoundsSetupDepended.count(); i++)
    {
      bool inside = false;
      const MarginElement::uarea_t& area = pImpl->elemsBoundsSetupDepended[i]->mec_area();
      switch (area.atto)
      {
      case AT_LEFT:
      {
        if (x > area.atto_end && x < area.atto_begin)
          inside = true;
        break;
      }
      case AT_RIGHT:
      {
        if (x > area.atto_begin && x < area.atto_end)
          inside = true;
        break;
      }
      case AT_TOP:
      {
        if (y > area.atto_end && y < area.atto_begin)
          inside = true;
        break;
      }
      case AT_BOTTOM:
      {
        if (y > area.atto_begin && y < area.atto_end)
          inside = true;
        break;
      }
      }
      if (inside)
      {
        int rtt = pImpl->elemsBoundsSetupDepended[i]->cachedRTexttype();
  //        const relatedopts_t& rdata = pImpl->elemsBoundsSetupDepended[i]->cachedRdata();
        if (rtt == RF_SETBOUNDS || rtt == RF_UPBOUNDS)
        {
          relatedopts_t rdata = pImpl->elemsBoundsSetupDepended[i]->cachedRdata();
          LL = rdata.rel_fixed.LL;
          HL = rdata.rel_fixed.HL;
          precision = pImpl->elemsBoundsSetupDepended[i]->precision() + 1;    // for type digit after current value
          inarea = true;
          break;
        }
      }
    }
    
    if (!inarea && pImpl->drawBoundsUpdater)
    {
//      qDebug()<<"DCLK!";
//      if (pDraw->rect().contains(x, y))
      {
        LL = pDraw->boundLow();
        HL = pDraw->boundHigh();
        precision=3;
        inarea = true;
      }
    }
    
    if (inarea)
    {
      QWidget* focused=nullptr;
      QFrame* boundsSetup = new QFrame;
      boundsSetup->setFrameStyle(QFrame::Box);
      boundsSetup->setWindowFlags(Qt::Popup);
      boundsSetup->setAttribute(Qt::WA_DeleteOnClose);
      QVBoxLayout*  lay = new QVBoxLayout();
      {
        float values[] = { HL, LL };
        for (int i=0; i<2; i++)
        {
          QDoubleSpinBox* qle = new QDoubleSpinBox();
          qle->setRange(-FLT_MAX, +FLT_MAX);
          qle->setDecimals(precision);
          qle->setValue(values[i]);
          qle->setAlignment(Qt::AlignRight);
          qle->setFixedWidth(120);
          if (i == 0)
            focused = qle;
          lay->addWidget(qle);
          if (i == 0)
            QObject::connect(qle, SIGNAL(valueChanged(double)), this, SLOT(toolUpdateBoundHigh(double)));
          else
            QObject::connect(qle, SIGNAL(valueChanged(double)), this, SLOT(toolUpdateBoundLow(double)));
        }
      }
      boundsSetup->setLayout(lay);
      boundsSetup->show();
      boundsSetup->move(this->mapToGlobal(event->pos()));
      focused->setFocus(Qt::TabFocusReason);
    }
  }
}

void DrawBars::connectScrollBar(QScrollBar* qsb, bool staticView, bool setOrientation)
{
  pDraw->connectScrollBar(qsb, staticView, setOrientation);
  QObject::connect(qsb, SIGNAL(valueChanged(int)), this, SLOT(scrollDataTo(int)));
}

void DrawBars::slot_setScalingA(int sh){  pDraw->slot_setScalingA(sh);  }
void DrawBars::slot_setScalingB(int sv){  pDraw->slot_setScalingB(sv);  }
void DrawBars::slot_setScalingH(int sh){  pDraw->slot_setScalingH(sh);  }
void DrawBars::slot_setScalingV(int sv){  pDraw->slot_setScalingV(sv);  }

void DrawBars::slot_setBounds(float low, float high){ pDraw->slot_setBounds(low, high); slot_updatedBounds(); }
void DrawBars::slot_setBoundLow(float v){  pDraw->slot_setBoundLow(v); slot_updatedBounds(); }
void DrawBars::slot_setBoundHigh(float v){  pDraw->slot_setBoundHigh(v); slot_updatedBounds();  }
void DrawBars::slot_setBounds01(){ pDraw->slot_setBounds01(); slot_updatedBounds();  }
void DrawBars::slot_setContrast(float k, float b){  pDraw->slot_setContrast(k, b);  slot_updatedBounds();  }
void DrawBars::slot_setContrastK(float v){  pDraw->slot_setContrastK(v);  slot_updatedBounds(); }
void DrawBars::slot_setContrastKinv(float v){  pDraw->slot_setContrastKinv(v);  slot_updatedBounds(); }
void DrawBars::slot_setContrastB(float v){  pDraw->slot_setContrastB(v);  slot_updatedBounds(); }
void DrawBars::slot_setDataTextureInterpolation(bool v){  pDraw->slot_setDataTextureInterpolation(v);  }
void DrawBars::slot_setDataPalette(const IPalette* v){  pDraw->slot_setDataPalette(v); slot_updatedDataPalette();  }
void DrawBars::slot_setDataPaletteDiscretion(bool v){  pDraw->slot_setDataPaletteDiscretion(v);  }
void DrawBars::slot_setDataPaletteRangeStart(float v){ pDraw->setDataPaletteRangeStart(v); }
void DrawBars::slot_setDataPaletteRangeStop(float v){ pDraw->setDataPaletteRangeStop(v); }
void DrawBars::slot_setDataPaletteRange(float start, float stop){ pDraw->setDataPaletteRange(start, stop); }
void DrawBars::slot_setData(const float* v){  pDraw->slot_setData(v);  }
void DrawBars::slot_setData(const QVector<float>& v){  pDraw->slot_setData(v);  }
void DrawBars::slot_fillData(float v){  pDraw->slot_fillData(v);  }
void DrawBars::slot_clearData(){  pDraw->slot_clearData();  }

void DrawBars::slot_adjustBounds(){ pDraw->slot_adjustBounds(); slot_updatedBounds(); }
void DrawBars::slot_adjustBounds(unsigned int portion){ pDraw->slot_adjustBounds(portion); slot_updatedBounds(); }
void DrawBars::slot_adjustBounds(unsigned int start, unsigned int stop){ pDraw->slot_adjustBounds(start, stop); slot_updatedBounds(); }
void DrawBars::slot_adjustBounds(unsigned int start, unsigned int stop, unsigned int portion){ pDraw->slot_adjustBounds(start, stop, portion); slot_updatedBounds(); }
void DrawBars::slot_adjustBoundsWithSpacingAdd(float add2min, float add2max){ pDraw->slot_adjustBoundsWithSpacingAdd(add2min, add2max); slot_updatedBounds(); }
void DrawBars::slot_adjustBoundsWithSpacingMul(float mul2min, float mul2max){ pDraw->slot_adjustBoundsWithSpacingMul(mul2min, mul2max); slot_updatedBounds(); }

void DrawBars::slot_setMirroredHorz(){  pDraw->slot_setMirroredHorz(); slot_updatedOrientation();  }
void DrawBars::slot_setMirroredVert(){  pDraw->slot_setMirroredVert(); slot_updatedOrientation(); }
void DrawBars::slot_setPortionsCount(int count){  pDraw->slot_setPortionsCount(count);  }

void DrawBars::slot_enableAutoUpdate(bool v){  pDraw->slot_enableAutoUpdate(v);  }
void DrawBars::slot_disableAutoUpdate(bool v){  pDraw->slot_disableAutoUpdate(v);  }
void DrawBars::slot_enableAutoUpdateByData(bool v){  pDraw->slot_enableAutoUpdateByData(v);  }
void DrawBars::slot_disableAutoUpdateByData(bool v){  pDraw->slot_disableAutoUpdateByData(v);  }


////////////////////////////////////////////////////////////////////////
void DrawBars::slot_setBoundLow_dbl(double value){  pDraw->setBoundLow((float)value); slot_updatedBounds(); }
void DrawBars::slot_setBoundHigh_dbl(double value){ pDraw->setBoundHigh((float)value); slot_updatedBounds(); }
void DrawBars::slot_setContrast_dbl(double k, double b){ pDraw->setContrast((float)k, (float)b); slot_updatedBounds(); }
void DrawBars::slot_setContrastK_dbl(double k){ pDraw->setContrastK((float)k); slot_updatedBounds(); }
void DrawBars::slot_setContrastKinv_dbl(double k){ pDraw->setContrastKinv((float)k); slot_updatedBounds(); }
void DrawBars::slot_setContrastB_dbl(double b){ pDraw->setContrastB((float)b); slot_updatedBounds(); }
void DrawBars::slot_setDataPaletteRangeStart_dbl(double s){  pDraw->setDataPaletteRangeStart((float)s); }
void DrawBars::slot_setDataPaletteRangeStop_dbl(double s){  pDraw->setDataPaletteRangeStart((float)s); }
////////////////////////////////////////////////////////////////////////


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
    bool doupdate = false;
    for (int i=0; i<cnt; i++)
      doupdate |= pImpl->elemsBoundDepended[i]->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bnd), false);
    if (doupdate)
    {
      update();
//      static int up1;
//      qDebug()<<"updated"<<up1++;
    }
    else
    {
//      static int up2;
//      qDebug()<<"banned"<<up2++;
    }
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
    pImpl->reupdateBars(MarginElement::UF_FORCED, MarginElement::UF_FORCED, MarginElement::UF_FORCED, MarginElement::UF_FORCED);
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

void DrawBars::toolUpdateBoundHigh(double v)
{
  this->slot_setBoundHigh(v);
  if (pTool)
    pTool->bndUpdated(this->pDraw);
}

void DrawBars::toolUpdateBoundLow(double v)
{
  this->slot_setBoundLow(v);
  if (pTool)
    pTool->bndUpdated(this->pDraw);
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
  remoteUpdate();
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
  m_premote->setMElementVisible(this, visible);
}

void MEQWrapper::setVisibleWithoutUpdate(bool visible)
{
  m_premote->setMElementVisible(this, visible, false);
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


void MEQWTexted::setFont(const QFont& m_font)
{
  ((MarginLabel*)m_pme)->setFont(m_font);
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

void MEWColoredSpace::setSpace(int space)
{
  ((MarginColoredSpace*)m_pme)->setSpace(space);
  remoteRebound();
}


/***/
  class MEPointerReactor: public IOverlayReactor
  {
    MarginPointer*  m_pme;
    DrawBars*       m_premote;
  public:
    MEPointerReactor(MarginPointer* ptr, DrawBars* premote): m_pme(ptr), m_premote(premote)
    {   if (m_premote) m_premote->update(); }
    MEPointerReactor(MarginPointer* ptr, DrawBars* premote, float x01, float y01): m_pme(ptr), m_premote(premote)
    {   m_pme->setPosition(x01, y01); if (m_premote) m_premote->update(); }
    virtual ~MEPointerReactor();
    virtual void  overlayReactionVisible(bool v)
    {
      m_pme->setVisible(v);
      if (m_premote)  m_premote->update();
    }
    virtual bool  overlayReactionMouse(OVL_REACTION_MOUSE oreact, const coordstriumv_t* ct, bool* /*doStop*/)
    {
      if (oreact == ORM_LMPRESS || oreact == ORM_LMMOVE)
      {
//        m_pme->setPosition(((const float*)dataptr)[0], ((const float*)dataptr)[1]);
        m_pme->setPosition(ct->fx_rel, ct->fy_rel);
        if (m_premote)  m_premote->update();
      }
      else if (oreact == ORM_RMPRESS)
      {
        m_pme->setVisible(false);
        if (m_premote)  m_premote->update();
      }
      return false;
    }
  };
  MEPointerReactor::~MEPointerReactor(){}

IOverlayReactor* MEWPointer::createReactor()
{
  return new MEPointerReactor((MarginPointer*)m_pme, m_premote);
}

void MEWPointer::setPosition(float pos01)
{
  ((MarginPointer*)m_pme)->setPosition(pos01);
  remoteUpdate();
}

void MEWPointer::setPositionBifunc(float pos01, float posText)
{
  ((MarginPointer*)m_pme)->setPositionBifunc(pos01, posText);
  remoteUpdate();
}

void MEWPointer::setBounds(float LL, float HL)
{
  if ( ((MarginPointer*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(LL, HL))) )
    remoteUpdate();
}

void MEWPointer::setBoundLow(float LL)
{
  if ( ((MarginPointer*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(LL, ((MarginPointer*)m_pme)->cachedRdata().rel_fixed.HL))) )
    remoteUpdate();
}
void MEWPointer::setBoundHigh(float HL)
{
  if ( ((MarginPointer*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(((MarginPointer*)m_pme)->cachedRdata().rel_fixed.LL, HL))) )
    remoteUpdate();
}

void MEWPointer::setBounds(double LL, double HL){  setBounds(float(LL), float(HL)); }
void MEWPointer::setBoundLow(double LL){  setBoundLow(float(LL)); }
void MEWPointer::setBoundHigh(double HL){  setBoundHigh(float(HL)); }

/////////////////////////////////////////////////////////////////////////////////////


//void MEWScale::setPrefix(const char* str){ ((MarginMarksComplicated*)m_pme)->setPrefix(str); remoteUpdate(); }
//void MEWScale::setPostfix(const char* str){ ((MarginMarksComplicated*)m_pme)->setPostfix(str); remoteUpdate(); }

void MEWScale::updateTapParam(const void* param)
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate(RF_UPTAPS, relatedopts_t(nullptr, param, 0) ) )
    remoteUpdate();
}

void MEWScale::setFont(const QFont& m_font)
{
  ((MarginMarksBase*)m_pme)->setFont(m_font);
//  remoteUpdate();
  remoteRebound();
}

void MEWScale::setMarkLen(int mlen)
{
  ((MarginMarksBase*)m_pme)->setMarkLength(mlen);
  remoteRebound();
}

void MEWScale::setMarkMiniLen(int mlen)
{
  ((MarginMarksBase*)m_pme)->setMarkMiniLength(mlen);
  remoteRebound();
}

//void MEWScale::scroll(int offset)
//{
//  ((MarginMarksTexted*)m_pme)->setRelatedOffset(offset, true);
//  remoteUpdate();
//}

void MEWScaleNN::setBounds(float LL, float HL)
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(LL, HL))) )
    remoteUpdate();
}

void MEWScaleNN::setBoundLow(float LL)
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(LL, ((MarginMarksBase*)m_pme)->cachedRdata().rel_fixed.HL))) )
    remoteUpdate();
}
void MEWScaleNN::setBoundHigh(float HL)
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate(RF_UPBOUNDS, relatedopts_t(bounds_t(((MarginMarksBase*)m_pme)->cachedRdata().rel_fixed.LL, HL))) )
    remoteUpdate();
}

void MEWScaleNN::setBounds(double LL, double HL){  setBounds(float(LL), float(HL)); }
void MEWScaleNN::setBoundLow(double LL){  setBoundLow(float(LL)); }
void MEWScaleNN::setBoundHigh(double HL){  setBoundHigh(float(HL)); }

void MEWScaleTAPNN::tap()
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate() )
    remoteUpdate();
}

void MEWScaleTAPNM::tap()
{
  if ( ((MarginMarksBase*)m_pme)->bdContentUpdate() )
    remoteUpdate();
}
