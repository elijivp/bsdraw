#ifndef QPALETTEBOX_H
#define QPALETTEBOX_H

/// ComboBox with non-standard palettes in dropped list
/// You need include resource (!!) file with all non-standard palettes
/// Usage:
/// 
///     QPaletteBoxSg* qpb = new QPaletteBoxSg();
///     QObject::connect(qpb, SIGNAL(currentChanged(const IPalette*)), some_bsdraw, SLOT(slot_setDataPalette(const IPalette*)));
///     some_layout->addWidget(qpb);
/// 
///     QAbstractButton*  qcbPalInverted = new QCheckBox("invert Palettes");
///     QObject::connect(qcbPalInverted, SIGNAL(toggled(bool)), qpb, SLOT(setInverted(bool)));
///     some_layout->addWidget(qcbPalInverted);
/// 
/// 
/// Created By: Elijah Vlasov

#include <QComboBox>

enum PALETTE_SET
{
  PS_ADVANCED,
  PS_SPECIAL, 
  PS_SPECIAL2
};

class QPaletteBoxPrivate;

class IPalette;

class QPaletteBox : public QComboBox
{
  Q_OBJECT
public:
  explicit QPaletteBox(PALETTE_SET ps=PS_ADVANCED, QWidget *parent = 0);
  ~QPaletteBox();
  bool  isInverted() const;
  const IPalette* currentPalette() const;
//  QSize sizeHint() const override;
public slots:
  void  setInverted(bool inverted);
protected:
  QPaletteBoxPrivate* d_ptr;
  Q_DISABLE_COPY(QPaletteBox)
  Q_DECLARE_PRIVATE(QPaletteBox)
};

class QPaletteBoxSg: public QPaletteBox
{
  Q_OBJECT
public:
  explicit QPaletteBoxSg(PALETTE_SET ps=PS_ADVANCED, QWidget *parent = 0);
private slots:
  void  _redirChanged(int);
signals:
  void  currentChanged(const IPalette* ppal);
};

#endif // QPALETTEBOX_H
