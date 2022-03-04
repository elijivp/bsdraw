/// ComboBox with non-standard palettes in dropped list
/// You need include resource file with all non-standard palettes
/// Created By: Elijah Vlasov
#include "QPaletteBox.h"

//#define DRAWSTATICTEXTCRUSH

#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <QImage>
#include <QPainter>
#ifdef DRAWSTATICTEXTCRUSH
#include <QStaticText>
#endif
#include "bspalettes_adv.h"
#include "bspalettes_spec.h"


#ifdef DRAWSTATICTEXTCRUSH
typedef QStaticText   palboxname_t;
#else
typedef QString       palboxname_t;
#endif


static const int TEXT_AREA_WIDTH = 32;
static const QSize  PREDEFINED_SIZE = QSize(256, 16);

class PaletteItem: public QAbstractItemDelegate
{
  QSize         imageDimms;
  QImage*       palImages;
  palboxname_t* palNames;
public:
  PaletteItem(QSize imageDimmsSize, QImage* pImages, palboxname_t* pNames): 
    imageDimms(imageDimmsSize), palImages(pImages), palNames(pNames)
  {
  }
  ~PaletteItem();
  
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
    int r = index.row();
    int y = option.rect.y();
#ifdef DRAWSTATICTEXTCRUSH
    painter->drawStaticText(QPoint(0 + TEXT_AREA_WIDTH - 4 - palNames[r].size().toSize().width(), y), palNames[r]);
#else
//      painter->drawText(QPoint(0, y), palNames[r]);
    painter->drawText(QRect(2, y, TEXT_AREA_WIDTH-2, imageDimms.height()), 0, palNames[r]);
#endif
    
    painter->drawImage(QPoint(TEXT_AREA_WIDTH, y), palImages[r]);
  }
  virtual QSize sizeHint(const QStyleOptionViewItem&,const QModelIndex&) const {  return imageDimms;  }
};

PaletteItem::~PaletteItem(){}


//template <const char* const PNAMES[], const IPalette* const PPALS[], const IPalette* const PPALS_INV[]>
//struct PaletteSet
//{
//  enum { MAX_PP = sizeof(PNAMES) / sizeof(const char*) };
//  QSize     imageDimms;
//  QImage    palImages[MAX_PP];
//  QImage    palImagesInverted[MAX_PP];
//  palboxname_t  palNames[MAX_PP];
//  bool inited;
//public:
//  PaletteSet(): imageDimms(0,0), inited(false) {}
//  bool isInited() { return inited; }
//  unsigned int width() { /*return TEXT_AREA_WIDTH;*/ return imageDimms.width(); }
//  unsigned int count() { return MAX_PP; }
//  void init(const QString& basePath)
//  {
//    for (unsigned int i=0; i<MAX_PP; i++)
//    {
//      palImages[i].load(basePath + PNAMES[i]);
////      if (palImages[i].size() > PREDEFINED_SIZE)
////      {
////        palImages[i].resize(PREDEFINED_SIZE);
////      }
//      palImagesInverted[i] = palImages[i].mirrored(true);
      
//#ifdef DRAWSTATICTEXTCRUSH
//      palNames[i].setText( QString::number(i) );
//      palNames[i].prepare();
//#else
//      palNames[i] = QString::number(i);
//#endif
//    }
//    imageDimms = palImages[0].size();
//    imageDimms += QSize(0,1) + QSize(TEXT_AREA_WIDTH, 0);
//    inited = true;
//  }
//  PaletteItem* createPaletteItem(bool inverted)
//  {
//    return new PaletteItem(imageDimms, inverted? palImagesInverted : palImages, palNames);
//  }
//  const IPalette*   paletteData(int idx, bool inverted) const { return inverted? PPALS_INV[idx] : PPALS[idx]; }
//};

//static PaletteSet<ppalettenames_adv, ppalettes_adv, ppalettes_adv_inv>   pset_adv;
//static PaletteSet<ppalettenames_spec, ppalettes_spec, ppalettes_spec_inv>   pset_spec;


template <int MAX_PP>
struct PaletteSet
{
  const char*   RESOURCE;
  const char**  PNAMES;
  const IPalette** PPALS;
  const IPalette** PPALS_INV;
  
//  enum { MAX_PP = sizeof(PNAMES) / sizeof(const char*) };
  QSize     imageDimms;
  QImage    palImages[MAX_PP];
  QImage    palImagesInverted[MAX_PP];
  palboxname_t  palNames[MAX_PP];
  bool inited;
public:
  PaletteSet(const char* rscpath, const char** pnames, const IPalette** ppals, const IPalette** ppals_inv): 
    RESOURCE(rscpath), PNAMES(pnames), PPALS(ppals), PPALS_INV(ppals_inv),
    imageDimms(0,0), inited(false) {}
  bool isInited() { return inited; }
  unsigned int width() { /*return TEXT_AREA_WIDTH;*/ return imageDimms.width(); }
  unsigned int count() { return MAX_PP; }
  void init()
  {
    for (unsigned int i=0; i<MAX_PP; i++)
    {
      palImages[i].load(QString(RESOURCE) + PNAMES[i]);
      if (palImages[i].size() != PREDEFINED_SIZE)
      {
        palImages[i] = palImages[i].scaled(PREDEFINED_SIZE);
      }
      palImagesInverted[i] = palImages[i].mirrored(true);
      
#ifdef DRAWSTATICTEXTCRUSH
      palNames[i].setText( QString::number(i) );
      palNames[i].prepare();
#else
      palNames[i] = QString::number(i);
#endif
    }
    imageDimms = palImages[0].size();
    imageDimms += QSize(0,1) + QSize(TEXT_AREA_WIDTH, 0);
    inited = true;
  }
  PaletteItem* createPaletteItem(bool inverted)
  {
    return new PaletteItem(imageDimms, inverted? palImagesInverted : palImages, palNames);
  }
  const IPalette*   paletteData(int idx, bool inverted) const { return inverted? PPALS_INV[idx] : PPALS[idx]; }
};

static PaletteSet<sizeof(ppalettenames_adv)/sizeof(const char*)>   pset_adv( ":/advanced/",
    (const char**)ppalettenames_adv, (const IPalette**)ppalettes_adv, (const IPalette**)ppalettes_adv_inv);

static PaletteSet<sizeof(ppalettenames_spec)/sizeof(const char*)>  pset_spec( ":/special/",
    (const char**)ppalettenames_spec, (const IPalette**)ppalettes_spec, (const IPalette**)ppalettes_spec_inv);


class QPaletteBoxPrivate
{
public:
  virtual bool isInited() const=0;
  virtual unsigned int width() const=0;
  virtual unsigned int count() const=0;
  virtual void init()=0;
  virtual PaletteItem* createPaletteItem(bool inverted)=0;
  virtual const IPalette* paletteData(int idx, bool inverted) const=0;
  virtual ~QPaletteBoxPrivate();
public:
  bool  bInverted;
};
QPaletteBoxPrivate::~QPaletteBoxPrivate(){}


template <class T>
class QPaletteBoxPrivateDetailed: public QPaletteBoxPrivate
{
  T& pset;
public:
  QPaletteBoxPrivateDetailed(T& pset_impl): pset(pset_impl) {}
  bool isInited() const { return pset.inited; }
  unsigned int width() const { /*return TEXT_AREA_WIDTH;*/ return pset.width(); }
  unsigned int count() const { return pset.count(); }
  void init(){ return pset.init(); }
  PaletteItem* createPaletteItem(bool inverted){ return pset.createPaletteItem(inverted); }
  virtual const IPalette* paletteData(int idx, bool inverted) const { return pset.paletteData(idx, inverted); }
};


QPaletteBox::QPaletteBox(PALETTE_SET ps, QWidget *parent): QComboBox(parent), d_ptr(
  ps == PS_ADVANCED? (QPaletteBoxPrivate*)new QPaletteBoxPrivateDetailed<PaletteSet<sizeof(ppalettenames_adv)/sizeof(const char*)>>(pset_adv) :
  ps == PS_SPECIAL? (QPaletteBoxPrivate*)new QPaletteBoxPrivateDetailed<PaletteSet<sizeof(ppalettenames_spec)/sizeof(const char*)>>(pset_spec) : 
  nullptr
  )
{
  Q_D(QPaletteBox);
  if (!d->isInited())
  {
    d->init();
  }
  d->bInverted = false;
  this->setItemDelegate(d->createPaletteItem(d->bInverted));
  for (unsigned int p=0; p<d->count(); p++)
    this->addItem(QString::number(p));
  
//  this->setMinimumWidth(QPaletteBoxPrivate::width());
//  this->setMaximumWidth(QPaletteBoxPrivate::width());
  
  this->view()->setMinimumWidth(d->width());
  this->setFixedWidth(88);
  
//  if (inversewdg)
//  {
//    d->qcbInverse = new QCheckBox("Инверт", this);
//    d->qcbInverse->move(QPaletteBoxPrivate::SELF_COMBO_WIDTH + 2, 0);
//  }
//  else
//    d->qcbInverse = nullptr;
}

QPaletteBox::~QPaletteBox()
{
  QAbstractItemDelegate*  qaid = itemDelegate();
  if (qaid) delete qaid;  // ownership, they said
  delete d_ptr;
}

bool QPaletteBox::isInverted() const
{
  const Q_D(QPaletteBox);
  return d->bInverted;
}

const IPalette* QPaletteBox::currentPalette() const
{
  const Q_D(QPaletteBox);
  return d->paletteData(currentIndex(), d->bInverted);
}
  
//QSize QPaletteBox::sizeHint() const
//{
//  const Q_D(QPaletteBox);
//  return QComboBox::sizeHint() + QSize(2,0) + d->qcbInverse->sizeHint();
//}

void QPaletteBox::setInverted(bool inverted)
{
  Q_D(QPaletteBox);
  if (d->bInverted != inverted)
  {
    this->setItemDelegate(d->createPaletteItem(inverted));
    d->bInverted = inverted;
    emit  currentIndexChanged(currentIndex());
  }
}

//////////////////////////////////////////////////////////////////////

QPaletteBoxSg::QPaletteBoxSg(PALETTE_SET ps, QWidget* parent): QPaletteBox(ps, parent)
{
  QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(_redirChanged(int)));
}

void QPaletteBoxSg::_redirChanged(int idx)
{
  Q_D(QPaletteBox);
  emit currentChanged(d->paletteData(idx, d->bInverted));
}
