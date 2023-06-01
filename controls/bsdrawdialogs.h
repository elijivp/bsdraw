#ifndef BSDRAWDIALOGS_H
#define BSDRAWDIALOGS_H

#include <QDialog>
#include "../core/bsidrawcore.h"


struct overlayconfig_t
{
  bool            overt;
  ovlbasics_t     basics;
  COORDINATION    cn;
  float           center, step;
  linestyle_t     kls;
};

class BSGridDialog: public QDialog
{
  Q_OBJECT
public:
  enum  EXAMPLE { EXM_GREY, EXM_BLACK, EXM_WHITE, EXM_COLOR };
  explicit BSGridDialog(bool vertical=true, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  explicit BSGridDialog(EXAMPLE exm, bool vertical, const overlayconfig_t& bc, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  ~BSGridDialog();
public:
  overlayconfig_t result() const;
  Ovldraw*    createOverlay() const;
private slots:
  void  mocdoci(int);
  void  mocdocs(const QString&);
  void  mocdocp(int);
  void  mocdoct(int);
protected:
  class BSGridDialogPrivate*  d_ptr;
  Q_DISABLE_COPY(BSGridDialog)
  Q_DECLARE_PRIVATE(BSGridDialog)
};

#endif // BSDRAWDIALOGS_H
