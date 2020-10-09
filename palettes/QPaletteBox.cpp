/// ComboBox with non-standard palettes in dropped list
/// You need include resource file with all non-standard palettes
/// Created By: Elijah Vlasov
#include "QPaletteBox.h"

#include <QAbstractItemDelegate>
#include <QImage>
#include <QPainter>
#include "bspalettes_adv.h"

class QPaletteBoxPrivate
{
public:
  enum { MAX_PP = sizeof(ppalettes_adv)/sizeof(const IPalette*) };
protected:
  class PaletteItem: public QAbstractItemDelegate
  {
    QSize         imageDimms;
    QImage*       palImages;
    unsigned int  palImagesCount;
  public:
    PaletteItem(QSize imageDimmsSize, QImage* pImages, unsigned int pImagesCount): imageDimms(imageDimmsSize), palImages(pImages), palImagesCount(pImagesCount){}
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
      painter->drawImage(QPoint(0,option.rect.y()), palImages[index.row()]);
    }
    virtual QSize sizeHint(const QStyleOptionViewItem&,const QModelIndex&) const {  return imageDimms;  }
  };
  
  static QSize     imageDimms;
  static QImage    palImages[MAX_PP];
  static QImage    palImagesInverted[MAX_PP];
  static bool inited;
public:
  static bool isInited() { return inited; }
  static unsigned int width() { return imageDimms.width(); }
  static void init(const QString& basePath)
  {
    for (unsigned int i=0; i<MAX_PP; i++)
    {
      palImages[i].load(basePath + ppalettes_names[i]);
      palImagesInverted[i] = palImages[i].mirrored(true);
    }
    imageDimms = palImages[0].size();
    imageDimms += QSize(0,1);
    inited = true;
  }
  static PaletteItem* createPaletteItem(bool inverted)
  {
    return new PaletteItem(imageDimms, inverted? palImagesInverted : palImages, MAX_PP);
  }
public:
  bool  bInverted;
};

bool  QPaletteBoxPrivate::inited = false;
QSize QPaletteBoxPrivate::imageDimms = QSize(0,0);
QImage QPaletteBoxPrivate::palImages[QPaletteBoxPrivate::MAX_PP];
QImage QPaletteBoxPrivate::palImagesInverted[QPaletteBoxPrivate::MAX_PP];

QPaletteBox::QPaletteBox(QWidget *parent): QComboBox(parent), d_ptr(new QPaletteBoxPrivate)
{
  if (!QPaletteBoxPrivate::isInited())
  {
    QPaletteBoxPrivate::init(":/advanced/");
  }
  Q_D(QPaletteBox);
  d->bInverted = false;
  this->setItemDelegate(QPaletteBoxPrivate::createPaletteItem(d->bInverted));
  for (unsigned int p=0; p<QPaletteBoxPrivate::MAX_PP; p++)
    this->addItem(QString::number(p));
  this->setMinimumWidth(QPaletteBoxPrivate::width());
  this->setMaximumWidth(QPaletteBoxPrivate::width());
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
  return d->bInverted? ppalettes_adv_inv[currentIndex()] : ppalettes_adv[currentIndex()];
}

void QPaletteBox::setInverted(bool inverted)
{
  Q_D(QPaletteBox);
  if (d->bInverted != inverted)
  {
    this->setItemDelegate(QPaletteBoxPrivate::createPaletteItem(inverted));
    d->bInverted = inverted;
    emit  currentIndexChanged(currentIndex());
  }
}

//////////////////////////////////////////////////////////////////////

QPaletteBoxSg::QPaletteBoxSg(QWidget* parent): QPaletteBox (parent)
{
  QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(_redirChanged(int)));
}

void QPaletteBoxSg::_redirChanged(int idx)
{
  Q_D(QPaletteBox);
  emit currentChanged(d->bInverted? ppalettes_adv_inv[idx] : ppalettes_adv[idx]);
}
