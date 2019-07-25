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


class IPalette;
class QPaletteBoxSg: public QPaletteBox
{
  Q_OBJECT
public:
  explicit QPaletteBoxSg(QWidget *parent = 0);
private slots:
  void  _redirChanged(int);
signals:
  void  currentChanged(const IPalette* ppal);
};

#endif // QPALETTEBOX_H
