#include "bscolorpad.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include <QFont>
#include <QList>

#include "bsdrawintensity.h"
#include "palettes/bspalettes_adv.h"
#include "overlays/bsfigures.h"
#include "overlays/bsinteractive.h"

#include <QMouseEvent>

typedef const PaletteSTD<256, false>   cpadpalette_t;

class BSColorpadIntensity: public DrawIntensity
{
  cpadpalette_t*            paletteActive;
  PaletteSTD<64, true>      paletteMeister;
private:
  const int                 BSW;
  const float               diap;
  const int                 flags;
  int                       cellWidth;
  int                       cellPosition;
  
  bool                      isMeister;
  float                     lastPaletteColor;
  
  float*                    cellValues;
  OActiveCursor*            ovlcursor;
  
  BSColorpadIntensity*      ppadNext;
  BSColorpad*               pEmitSignal;
  unsigned int              savedValue;
public:
  BSColorpadIntensity(cpadpalette_t* palmain, bool meister, int wcount, float diapsize, BSColorpadIntensity* pnext, int _flags, BSColorpad* pnp);
  ~BSColorpadIntensity();
  int   cellsCountWidth() const { return BSW; }
  int   colorflags() const { return flags; }
  float diapsize() const { return diap; }
  BSColorpadIntensity*      next() const { return ppadNext; }
public:
  void    setNext(BSColorpadIntensity* _next);
public:   /// recursives:
  void    recur_switchPalette(cpadpalette_t* ppal, bool lastmeister, bool lastpostmask);
  void    recur_resetBase();
  void    recur_presetValue(float base, bool doEmit);
  void    recur_setBase(float value, bool doEmit=true);
  void    recur_dropCell(bool doEmit=false);
  unsigned int  recur_value();
private:
  void    actionSelect(QPoint);
  void    actionRelease();
protected:
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
};

BSColorpadIntensity::BSColorpadIntensity(cpadpalette_t* palmain, bool meister,
  int wcount, float diapsize, BSColorpadIntensity* pnext, int _flags, BSColorpad* pnp):
  DrawIntensity((unsigned int)wcount, (unsigned int)1, 1),
  paletteActive(palmain),
  BSW(wcount), diap(diapsize), flags(_flags),
  ppadNext(pnext),
  pEmitSignal(pnp), savedValue(0)
{
  setScalingLimitsHorz(2);
//  setScalingLimitsV(16);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
  
  if (!meister)
    setDataPalette(palmain);
  cellPosition = BSW/2;
  isMeister = meister;
  
  
  cellValues = new float[BSW];
  if (diap == 1.0f || meister)
  {
    for (int i=0; i<BSW; i++)
      cellValues[i] = i/float(BSW-1);
  }
  else
    for (int i=0; i<BSW; i++)
      cellValues[i] = 0.0f;
  setData(cellValues);
  
  
  lastPaletteColor = 0.5f;
  
  ovlcursor = nullptr;
  linestyle_t kls = linestyle_inverse_1(1,0,0, OLS_OUTLINE);
  if (ppadNext || (ppadNext == nullptr && diap == 1.0f))
  {
    int ocid = ovlPushBack(ovlcursor = new OActiveCursor(false));
    float dsize = ppadNext? ppadNext->diapsize()*wcount/2.0f : 1.5f;
    for (int i=0; i<2; i++)
    {
      Ovldraw* ovl = new OFLine(OFLine::LT_VERT_BYBOTTOM, CR_ABSOLUTE, (i == 0? -1.0f : 1.0f)*dsize, -1, CR_RELATIVE, 0.0f, -1.0f, kls);
      ovl->setThickness(1.0f);
      ovlPushBack(ovl, ocid);
    }
  }
  else
  {
    int ocid = ovlPushBack(ovlcursor = new OActiveCursor(true));
//    Ovldraw* ovl = new OFLine(OFLine::LT_VERT_BYBOTTOM, CR_ABSOLUTE, 0.0f, -1, CR_RELATIVE, 0.0f, -1.0f, kls);
//    Ovldraw* ovl = new OFCircle(1.0f, CR_ABSOLUTE, 0.0f, 0.0f, CR_RELATIVE, 0.1f, kls);
    Ovldraw* ovl = new OFCross(CR_ABSOLUTE, 0.0f, 0.0f, CR_RELATIVE, 0.0f, 0.2f, kls);
    ovl->setThickness(1.0f);
    ovlPushBack(ovl, ocid);
  }
}

BSColorpadIntensity::~BSColorpadIntensity()
{
  delete[] cellValues;
}

void BSColorpadIntensity::setNext(BSColorpadIntensity* _next)
{
  ppadNext = _next;
}

void BSColorpadIntensity::recur_switchPalette(cpadpalette_t* ppal, bool lastmeister, bool lastpostmask)
{
  paletteActive = ppal;
  bool isPostmask = false;
  isMeister = false;
  if (ppadNext == nullptr)
  {
    isMeister = lastmeister;
    isPostmask = lastpostmask;
  }
  
  if (!isMeister)
    setDataPalette(paletteActive);
  else
  {
    for (int i=0; i<BSW; i++)
      cellValues[i] = i/float(BSW-1);
    setData(cellValues);
  }
  
  setOverpattern(isPostmask? 
                   overpattern_any(OP_LINELEFT, color3f(0.2f,0.2f,0.2f)) :
                   overpattern_thrs_minus(OP_LINELEFT, 0.0f, color3f(0.2f,0.2f,0.2f)));
    
  if (ppadNext)
    ppadNext->recur_switchPalette(ppal, lastmeister, lastpostmask);
}

void BSColorpadIntensity::recur_resetBase()
{
  recur_setBase(lastPaletteColor);
}

void BSColorpadIntensity::recur_presetValue(float palclr01, bool doEmit)
{
//  qDebug()<<"Presetting: "<<palclr01;
  lastPaletteColor = palclr01;
  cellPosition = int(palclr01*(BSW-1));
  if (ovlcursor)
    ovlcursor->setCoordinates(palclr01, 0.5f);
  if (ppadNext)
  {
    ppadNext->recur_dropCell(false);
    ppadNext->recur_setBase(palclr01, doEmit);
  }
//    ppadNext->recur_setBase(cellValues[cellPosition], true);
}

template<int maxCount, bool discrete>
PaletteSTD<maxCount, discrete>*  fillPaletteBkWh(int centralColor, PaletteSTD<maxCount, discrete>* result)
{
  if (centralColor == 0) centralColor = 0x007F7F7F;
  
//  int half1 = result->count()/2; // nvm  + (result->count() % 2? 1:0);
//  int half2 = result->count()/2 - (result->count() % 2? 0:1);
  
  int half1 = (result->count()-1)/2;
  int half2 = (result->count()/2);
//  qDebug()<<"hh"<<half1<<half2;
  int b = (centralColor >> 16) & 0xFF, g = (centralColor >> 8) & 0xFF, r = (centralColor) & 0xFF;
  {
    float stepB = b/float(half1), stepG = g/float(half1), stepR = r/float(half1);
    for (int i=0; i<half1; i++)
      (*result)[i] = (int(stepB*i)<<16) + (int(stepG*i)<<8) + int(stepR*i);
  }
  (*result)[half1] = centralColor;
  {
    float stepB = (255-b)/float(half2), stepG = (255-g)/float(half2), stepR = (255-r)/float(half2);
    for (int i=0; i<half2; i++)
      (*result)[half1+1+(half2 - 1 - i)] = ((255 - int(stepB*i))<<16) + ((255 - int(stepG*i))<<8) + (255 - int(stepR*i));
  }
  
  return result;
}


template<int maxCount, bool discrete>
int color2int(float clr, const PaletteSTD<maxCount, discrete>& pal)
{
  int count = pal.count();
  int c1 = int(clr*count);
  if (c1 == 0)
    return pal[c1];
  if (c1 >= count-1)
    return pal[count-1];
  unsigned int iclr1 = pal[c1], iclr2 = pal[c1 + 1];
  int b = (int((iclr2 >> 16) & 0xFF) + int((iclr1 >> 16) & 0xFF)) / 2;
  int g = (int((iclr2 >> 8) & 0xFF) + int((iclr1 >> 8) & 0xFF)) / 2;
  int r = (int((iclr2) & 0xFF) + int((iclr1) & 0xFF)) / 2;
  return (b<<16) + (g<<8) + r;
}

void BSColorpadIntensity::recur_setBase(float palclr01, bool doEmit)
{
  lastPaletteColor = palclr01;
  if (isMeister)
  {
    int midx = palclr01*(paletteActive->count()-1);
    if (midx < 0)  midx = 0;
    else if (midx >= paletteActive->count()) midx = paletteActive->count()-1;
    
//    qDebug()<<"forpresets: "<<cellPosition<<BSW;
//    qDebug()<<"formeister: "<<palclr01<<"clr "<<QString::number((*paletteActive)[midx], 16);
    fillPaletteBkWh((*paletteActive)[midx], &paletteMeister);
    setDataPalette(&paletteMeister);
    if (doEmit)
    {
//      float lateclr01 = paletteMeister[(cellPosition*(paletteMeister.size))/(BSW-1)];
//      int idx = int((cellPosition*(paletteMeister.size-1))/float(BSW-1) + 0.49f);
      int idx = int((cellPosition*(paletteMeister.count()-1))/float(BSW-1) + 0.49f);
//      qDebug()<<"EZE"<<cellPosition<<idx;
//      qDebug()<<"Triplet: "<<QString::number(paletteMeister[idx-1], 16)<<
//        QString::number(paletteMeister[idx], 16)<<
//        QString::number(paletteMeister[idx+1], 16);
      emit pEmitSignal->valueChanged(savedValue = paletteMeister[idx]);
    }
  }
  else
  {
//    qDebug()<<"BASE before"<<palclr01<<" (diap)"<<diap<<cellPosition<<BSW;
    if (diap != 1.0f)
    {
      int half = BSW/2;
      float fstep = diap / BSW;
      cellValues[half] = palclr01;
      for (int i=0; i<half; ++i)
        cellValues[half - 1 - i] = cellValues[half] - fstep*i;
      for (int i=half+1; i<BSW; ++i)
        cellValues[i] = cellValues[half] + fstep*(i - half - 1);
      setData(cellValues);
      palclr01 = cellValues[cellPosition];
    }
//    qDebug()<<"BASE aftere"<<palclr01;
    if (ppadNext)
      ppadNext->recur_setBase(palclr01);
    else if (doEmit)
      emit pEmitSignal->valueChanged(savedValue = color2int(palclr01, *paletteActive));
  }
}

void BSColorpadIntensity::recur_dropCell(bool doEmit)
{
  if (ovlcursor)
    ovlcursor->setCoordinates(0.5f, 0.5f);
  
  cellPosition = BSW/2;
  if (isMeister)
  {
    if (doEmit)
      emit pEmitSignal->valueChanged(savedValue = paletteMeister[(cellPosition*(paletteMeister.count()-1))/(BSW-1)]);
  }
  else
  {
    if (ppadNext)
      ppadNext->recur_setBase(cellValues[cellPosition]);
    else if (doEmit)
      emit pEmitSignal->valueChanged(savedValue = color2int(cellValues[cellPosition], *paletteActive));
  }
}

unsigned int  BSColorpadIntensity::recur_value()
{
  if (isMeister)  return savedValue;
  if (ppadNext) return ppadNext->recur_value();
  return savedValue;
}

void BSColorpadIntensity::actionSelect(QPoint coords)
{
  {
    cellWidth = width()/BSW;      /// ntf!
  
    int cellX = int(coords.x()/float(cellWidth));  // + 0.49
    if (cellX >= BSW || cellX < 0)
      return;
  
    cellPosition = cellX;
  }
  
  if (isMeister)
  {
    emit pEmitSignal->valueChanged(savedValue = paletteMeister[(cellPosition*(paletteMeister.count()-1))/(BSW-1)]);
  }
  else
  {
    lastPaletteColor = cellValues[cellPosition];
    if (ppadNext)
      ppadNext->recur_setBase(cellValues[cellPosition]);
    else
      emit pEmitSignal->valueChanged(savedValue = color2int(cellValues[cellPosition], *paletteActive));
  }
}

void BSColorpadIntensity::actionRelease()
{
  recur_dropCell(true);
}

void BSColorpadIntensity::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() & Qt::LeftButton)
    actionSelect(event->pos());
  else if (event->buttons() & Qt::RightButton)
  {
    if ((flags & CPF_RIGHTFINISH) == 0) // && !(event->buttons && Qt::LeftButton) already!
      actionRelease();
    else
      emit pEmitSignal->originalFinished();
  }
  DrawIntensity::mousePressEvent(event);
}

void BSColorpadIntensity::mouseReleaseEvent(QMouseEvent* event)
{
  DrawIntensity::mouseReleaseEvent(event);
}

void BSColorpadIntensity::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() & Qt::LeftButton)
    actionSelect(event->pos());
  DrawIntensity::mouseMoveEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "visual/layout/bsqlayout.h"
#include <QWidget>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>
#include <QTimer>

// bsqlayout includes
#include <QSplitter>
#include <QFormLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QTabBar>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMouseEvent>

#include <QBitArray>

#include <core/sheigen/bsshgentrace.h>

struct palmem_t
{
  cpadpalette_t*  palette;
  int             collapse;
  bool            blockmeisters;
  palmem_t(cpadpalette_t* cpp=nullptr, int cps=0, bool bm=false): palette(cpp), collapse(cps), blockmeisters(bm) {}
};
struct palswitch_t
{
  cpadpalette_t*                palette;
  QBitArray                     originalRow;
  bool                          hasMeister;
};

class BSColorpadPrivate
{
public:
  BSColorpadIntensity*      pcpad;
  QList<palswitch_t>        cswitches;
  int                       cswcurrent;
  QVector<BSColorpadIntensity*> origins;
  QLineEdit*                pedit;
};


BSColorpad::BSColorpad(COLORPATTERN cp, int flags, QWidget* parent): QWidget(parent)
{
  pImpl = new BSColorpadPrivate;
  pImpl->pcpad = nullptr;
  pImpl->pedit = nullptr;
  
  int minheight = 0;
  {
    palmem_t pmMAT =        palmem_t(&palette_mat_hsv, 0);
    palmem_t pmRAINBOW =    palmem_t(&palette_idl_rainbow, 0);
    palmem_t pmBKWH =       palmem_t(&palette_idl_linearBkWh, 2, true);
    
    palmem_t      palettes[10];
    unsigned int  pcount=0;
    if (cp == CPP_MAT)                  palettes[pcount++] = pmMAT;
    else if (cp == CPP_MAT_N_GRAY){     palettes[pcount++] = pmMAT;       palettes[pcount++] = pmBKWH;    }
    else if (cp == CPP_RAINBOW){        palettes[pcount++] = pmRAINBOW;   }
    else if (cp == CPP_RAINBOW_N_GRAY){ palettes[pcount++] = pmRAINBOW;   palettes[pcount++] = pmBKWH;    }
    else if (cp == CPP_GRAY){           palettes[pcount++] = pmBKWH;   }
    
    int levels = flags & CPF_LEVELS_SUPPRESS? 1 : flags & CPF_LEVELS_MINIMIZE? 2 : 3;
    
    {
      int lvlmin=5;
      for (int i = 0; i < pcount; ++i)
      {
        if (palettes[i].collapse == 0)
        {
          lvlmin = 0;
          break;
        }
        if (lvlmin > palettes[i].collapse)
          lvlmin = palettes[i].collapse;
      }
      if (lvlmin)
        levels = lvlmin;
    }
    
    pImpl->origins.reserve(levels);
    bool dewhite = flags & CPF_DEWHITELAST;
    
    BS_INIT_DERIVED(QVBoxLayout);
    {
      for (int i=0; i<levels; i++)
      {
        float diap = 1.0f;
        for (int j=int(levels)-1; j>i; j--)
          diap *= 0.2f;
        int wcount = (levels == 1 || i != 0? 74 : 14) + 1;
        
        bool isLast = i == 0 && levels > 1;
        bool isMeister = isLast && !dewhite && !palettes[0].blockmeisters;
        
        BSColorpadIntensity* bcp = new BSColorpadIntensity(
                                      palettes[0].palette, 
                                      isMeister,
                                      wcount, diap, pImpl->pcpad, flags, this);
        if (isLast)
        {
          if (!isMeister)
            ;
        }
        else
        {
          if ((flags & CPF_INTERPOFF) == 0)
            bcp->setDataTextureInterpolation(true);
        }
        BSADD(bcp);
        pImpl->pcpad = bcp;
        pImpl->origins.push_front(bcp);
      }
      
      if (pcount > 1 || flags & CPF_OWNFIELD)
      {
        BS_START_LAYOUT_HMAX_VMAX(QHBoxLayout)
        {
//          unsigned int reservedWidth = 0;
          if (pcount > 1)
          {
            QAbstractButton*  pPalSwitch;
            if (flags & CPF_OWNFIELD)
            {
              pPalSwitch = new QPushButton(QString::fromUtf8("ЧБ"));
              
              BSADD(pPalSwitch, 0, Qt::AlignLeft);
            }
            else
            {
  //            pPalSwitch = new QPushButton(QString::fromUtf8("черно-белая"));
              pPalSwitch = new QPushButton(QString::fromUtf8("ЧБ"));
              BSADD(pPalSwitch, 0, Qt::AlignLeft);
            }
            pPalSwitch->setCheckable(true);
            QObject::connect(pPalSwitch, SIGNAL(clicked()), this, SLOT(slotloc_switchpalettes()));
          }
          
          
          if (flags & CPF_OWNFIELD)
          {
            pImpl->pedit = new QLineEdit;
            pImpl->pedit->setAlignment(Qt::AlignRight);
            pImpl->pedit->setMaximumWidth(96);
            pImpl->pedit->setReadOnly(true);
            BSADD(pImpl->pedit, 0, Qt::AlignCenter);
            
            QObject::connect(this, SIGNAL(valueChanged(unsigned int)), this, SLOT(slotloc_updatefield(unsigned int)));
          }
//          BSLAYOUT->addStruts(20);
        }
        BS_STOP
      }
    }
    
    
    for (unsigned int i = 0; i < pcount; ++i)
    {
      palswitch_t pc;
      pc.palette = palettes[i].palette;
      
      pc.originalRow.resize(pImpl->origins.size());
      if (palettes[i].collapse == 0)
        pc.originalRow.fill(true);
      else if (palettes[i].collapse == 1)
        pc.originalRow[0] = true;
      else if (palettes[i].collapse == 2)
      {
        pc.originalRow[0] = true;
        pc.originalRow[pImpl->origins.size()-1] = true;
      }
      else
        Q_ASSERT(false);
      
      pc.hasMeister = !dewhite && !palettes[i].blockmeisters && pc.originalRow.count(true) > 1;
      
      pImpl->cswitches.push_back(pc);
    }
    
    
    minheight = 20+levels*30;
  }
  pImpl->cswcurrent = 0;
  
//  setMinimumSize(100, pColors->cellsCountHeight()*20);
//  setMinimumSize(pImpl->pcpad->cellsCountWidth()*6, minheight);
  setMinimumSize(pImpl->pcpad->cellsCountWidth()*3, minheight);
}

BSColorpad::~BSColorpad()
{
  delete pImpl;
}

void BSColorpad::setScalingLimitsH(unsigned int scmin, unsigned int scmax)
{
  pImpl->pcpad->setScalingLimitsHorz(scmin, scmax);
}

void BSColorpad::slotloc_updatefield(unsigned int value)
{
  pImpl->pedit->setText(QString().sprintf("#%08X", value));
}

void BSColorpad::slotloc_switchpalettes()
{
  {
//    QAbstractButton* qab = (QAbstractButton*)sender();
//    qab->setText(v? 
  }
  int cswnext = pImpl->cswcurrent + 1;
  if (cswnext >= pImpl->cswitches.count())
    cswnext = 0;
  
  switchPalette(cswnext);
}

void BSColorpad::switchPalette(int idx)
{
  if (idx < 0 && idx >= pImpl->cswitches.count())
    return;
  
  palswitch_t& psw = pImpl->cswitches[pImpl->cswcurrent = idx];
  
  BSColorpadIntensity*  pcur = nullptr;
  for (int i = 0; i < pImpl->origins.size(); ++i)
  {
    if (psw.originalRow[i] == true)
    {
      if (pcur == nullptr)
        pImpl->pcpad = pImpl->origins[i];
      else
        pcur->setNext(pImpl->origins[i]);
      
      pcur = pImpl->origins[i];
      pImpl->origins[i]->setVisible(true);
    }
    else
      pImpl->origins[i]->setVisible(false);
  }
  pImpl->pcpad->recur_switchPalette(psw.palette, psw.hasMeister, psw.originalRow.count()>0 && !psw.hasMeister);
  pImpl->pcpad->recur_resetBase();
}

unsigned int BSColorpad::value() const
{
  return pImpl->pcpad->recur_value();
}

template<int maxCount, bool discrete>
float int2color(unsigned int iclr, const PaletteSTD<maxCount, discrete>& pal, int *mera=nullptr)
{
  int count = pal.count();
  int b = (iclr >> 16) & 0xFF, g = (iclr >> 8) & 0xFF, r = (iclr) & 0xFF;
  int minid = 0;
  int minweight = 255;
//    qDebug()<<QString().sprintf("#%08X", iclr)<<b<<g<<r;
  for (int i=0; i<count; i++)
  {
    int iclr2 = pal[i];
    int b2 = (iclr2 >> 16) & 0xFF, g2 = (iclr2 >> 8) & 0xFF, r2 = (iclr2) & 0xFF;
    b2 = abs(b2 - b);   g2 = abs(g2 - g);   r2 = abs(r2 - r);
    //    int mincur = b2 < g2? b2 < r2? b2 : r2 : g2 < r2? g2 : r2;
    //    int mincur = (b2 + g2 + r2)/3;
    int mincur = b2 > g2? ( b2 > r2? b2 : r2 > g2? r2 : g2 ) : g2 > r2? g2 : r2;
    if (mincur < minweight)
    {
      minweight = mincur;
      minid = i;
    }
//    qDebug()<<i<<QString().sprintf("#%08X", iclr2)<<": "<<b2<<g2<<r2<<"__________"<<mincur<<"__________"<<minid;
  }
//  qDebug()<<"RESULT: "<<minid<<QString().sprintf("#%08X", pal[minid]);
  if (mera) *mera = minweight;
  return float(minid)/(count-1);
}

void BSColorpad::presetValue(unsigned int value, bool doEmit)
{
  if (pImpl->pedit)
    slotloc_updatefield(value);
  
  int mera;
  float pos;
  int idx=-1;
  for (int i=0; i<pImpl->cswitches.count(); ++i)
  {
    int m;
    float p = int2color(value, *pImpl->cswitches[i].palette, &m);
    if (i == 0 || m < mera)
    {
      mera = m;
      idx = i;
      pos = p;
    }
//    qDebug()<<"p"<<i<<"= "<<m<<p;
  }
  
  if (idx != -1)
  {
//    qDebug()<<"PRESET START:"<<QString::number(value, 16)<<";    Preset pos:"<<pos;
    switchPalette(idx);
    pImpl->pcpad->recur_presetValue(pos, doEmit);
  }
}

/**********************************************************************************************************************/

#include <QDialog>

class BSColorcallPrivate
{
public:
  unsigned int          value;
  int                   npw;
  COLORPATTERN          cp;
  int                   flags;
  
  bool                  popupSave;
  QFrame*               popup;
  BSColorpad*           popupSavedPad;
  
  Qt::MouseButtons      btns;
  bool                  btnsDbl;
};

BSColorcall::BSColorcall(unsigned int value, COLORPATTERN cp, int flags, QWidget* parent): QLineEdit (parent)
{
  setAlignment(Qt::AlignRight);
  setMinimumWidth(30);
  
  pImpl = new BSColorcallPrivate;
  pImpl->cp = cp;
  pImpl->flags = flags;
//  setPopupOptions(np, columns, rows, flags);
  pImpl->value = value;
  setText(QString().sprintf("#%08X", value));
  
  pImpl->popupSave = false;
  pImpl->popup = nullptr;
  pImpl->popupSavedPad = nullptr;
  
  QObject::connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(slotloc_syncfield(const QString &)));
  
  pImpl->btns = Qt::NoButton;
  pImpl->btnsDbl = false;
  
  QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(slotloc_editing()));
}

BSColorcall::~BSColorcall()
{
  if (pImpl->popup)
    delete pImpl->popup;
  delete pImpl;
}

void BSColorcall::setPopupOptions(COLORPATTERN cp, int flags)
{
  pImpl->cp = cp;
  pImpl->flags = flags;
}

void BSColorcall::doSavePopup()
{
  pImpl->popupSave = true;
}

void BSColorcall::setPopupButton(Qt::MouseButton button, bool doubleClick)
{
  pImpl->btns = button;
  pImpl->btnsDbl = doubleClick;
}

void BSColorcall::unsetPopupButtons()
{
  pImpl->btns = Qt::NoButton;
}

void BSColorcall::slotloc_editing()
{
  emit valueChanged(pImpl->value);
}

void BSColorcall::slotloc_updatefield(unsigned int value)
{
  setText(QString().sprintf("%08X", value));
  pImpl->value = value;
  emit editingFinished();
}

void BSColorcall::slotloc_syncfield(const QString& value)
{
  pImpl->value = value.toUInt(0, 16);
}

void BSColorcall::activatePopup()
{
  QFrame* popup = nullptr;
  BSColorpad* popad = nullptr;
  if (pImpl->popupSave && pImpl->popup)
  {
    popup = pImpl->popup;
    popad = pImpl->popupSavedPad;
  }
  else
  {
    popup = new QFrame(nullptr, Qt::Popup);
    BS_INIT(QVBoxLayout)
    BS_START_FRAME_V_HMAX_VMAX(BS_FRAME_PANEL, 2)
      popad = new BSColorpad(pImpl->cp, pImpl->flags);
      BSADD(popad);
      QObject::connect(popad, SIGNAL(valueChanged(unsigned int)), this, SLOT(slotloc_updatefield(unsigned int)));
      
      if (!pImpl->popupSave)
        QObject::connect(popad, SIGNAL(originalFinished()), popup, SLOT(deleteLater()));    /// autoclose
      else
      {
        QObject::connect(popad, SIGNAL(originalFinished()), popup, SLOT(hide()));    /// autoclose
        pImpl->popupSavedPad = popad;
      }
    BS_STOP
    popup->setLayout(BSLAYOUT);
    if (pImpl->popupSave)
      pImpl->popup = popup;
  }
  popad->presetValue(pImpl->value);
  popup->showMinimized(); /// move later!
  popup->move(this->mapToGlobal(QPoint(0 + width()/2 - popup->width()/2, 0 - popup->height())));
  popup->showNormal();
}

void BSColorcall::mousePressEvent(QMouseEvent* event)
{
  if (pImpl->btns.testFlag(event->button()) && !pImpl->btnsDbl)
    activatePopup();
}

void BSColorcall::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (pImpl->btns.testFlag(event->button()) && pImpl->btnsDbl)
    activatePopup();
}


/**********************************************************************************************************************/

BSColorcallInsquare::BSColorcallInsquare(BSColorcall* pnc, bool rejectDefaultButtons, QWidget* parent): QWidget(parent)
{
  BS_INIT_DERIVED(QHBoxLayout)
  if (rejectDefaultButtons)
    pnc->unsetPopupButtons();
  BSADD(pnc, 0);
  QPushButton* pMarker = new QPushButton;
  pMarker->setFlat(true);
//  pMarker->setContentsMargins(12,12,12,12);
  pMarker->setMinimumSize(10,10);
//  pMarker->setSizePolicy(QSizePolicy::Fixed , QSizePolicy::Minimum);
//  pMarker->setFixedWidth(10);
//  pMarker->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pMarker->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  pMarker->setStyleSheet("QPushButton { border: none; background-color: #663300; } QPushButton:pressed { border: none; background-color: #000000; } ");
  BSADD(pMarker, 1);
  QObject::connect(pMarker, SIGNAL(clicked()), pnc, SLOT(activatePopup()));
}
