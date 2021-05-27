#include <QApplication>
#include "mainwindow.h"

/*  elirupost@gmail.com
 * 
 if (rabota == volk)
  rabota.doRun(&les);
*/

//#define MW_PALETTE_EXTRACT
#ifdef MW_PALETTE_EXTRACT
#include "palettes/QPaletteExtractor.h"
#include <QDebug>
#endif


//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)14
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)15
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)34


#ifndef MW_TEST_NOCHOOSE_DEFAULT
//#include <QInputDialog>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#endif

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
#ifdef MW_PALETTE_EXTRACT
  pexresult_t pr = paletteExtractFromImage(
//                     "/home/elijah/Projects/bsdraw/palettes/special/spec_GnYe.png", 
//                     "/home/elijah/Projects/bsdraw/palettes/special/spec_BlGnYe.png", 
                     "/home/elijah/Projects/bsdraw/palettes/special/spec_BlYeRd.png", 
                     PEF_REMOVEFIRSTCOLOR | PEF_REMOVELASTCOLOR | PEF_CUTTER_GROUP);
  qDebug().noquote()<<pr.success<<pr.paletteHeaderText<<pr.countColors;
  savePaletteExtract(pr);
  return 0;
#endif

  tests_t   MW_TEST;
#ifdef MW_TEST_NOCHOOSE_DEFAULT
  MW_TEST = MW_TEST_NOCHOOSE_DEFAULT;
#else
#define REGTEST(A) #A,
  const char* testnames[] = {
  #include "tests.h"
  };
#undef REGTEST
  
  bool ok;
  QStringList   tests;
  for (unsigned int i=0; i<sizeof(testnames)/sizeof(const char*); i++)
  {
    QString tname(testnames[i]);
    tname = tname.replace('_', " ");
    tests<<QString::number(i) + ". " + tname;
  }
  QString prepend = "You can change GLSL version\nby changing BSGLSLVER in .pro file *\n\n";
//  QString result = QInputDialog::getItem(nullptr, "bsdraw", prepend + "Choose test:", tests, 1, false, &ok);
//  if (!ok)  return 0;
//  int result;
  {
    QDialog dlg;
    QVBoxLayout*  lay = new QVBoxLayout;
    lay->addWidget(new QLabel(prepend));
    QListWidget*    qlv = new QListWidget;
    qlv->addItems(tests);
    qlv->setSelectionMode(QAbstractItemView::SingleSelection);
    qlv->setCurrentRow(1);
    qlv->setMinimumHeight(qlv->sizeHintForRow(0)*tests.count() + 10);
    lay->addWidget(qlv);
    QPushButton*  btn = new QPushButton("Accept");
    QObject::connect(btn, SIGNAL(clicked()), &dlg, SLOT(accept()));
    lay->addWidget(btn);
    dlg.setLayout(lay);
    if (dlg.exec() != QDialog::Accepted)
      return 0;
    MW_TEST = (tests_t)qlv->currentIndex().row();
  }
//  MW_TEST = (tests_t)tests.indexOf(result);
#endif
  MainWindow w(MW_TEST);
  w.move(100, 200);
//  w.resize(1400, 1000);
  w.show();
  return a.exec();
}
