#ifndef QPALETTEBOX_H
#define QPALETTEBOX_H

#include <QComboBox>

class QPaletteBoxPrivate;

class QPaletteBox : public QComboBox
{
  Q_OBJECT
public:
  explicit QPaletteBox(QWidget *parent = 0);
  ~QPaletteBox();
  bool  isInverted() const;
public slots:
  void  setInverted(bool inverted);
protected:
  QPaletteBoxPrivate* d_ptr;
  Q_DISABLE_COPY(QPaletteBox)
  Q_DECLARE_PRIVATE(QPaletteBox)
};

#endif // QPALETTEBOX_H
