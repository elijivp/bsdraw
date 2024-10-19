#include <QApplication>
#include "mainwindow.h"

/*
 if (rabota == volk)
  rabota.doRun(&les);
*/

//#define MW_PALETTE_EXTRACT
#ifdef MW_PALETTE_EXTRACT
#include "palettes/QPaletteExtractor.h"
#include <QDebug>
#endif


//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)2
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)5
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)9
#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)20
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)24
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)29
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)39
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)58

//#define ONLY_OVERVIEWS


#ifndef MW_TEST_NOCHOOSE_DEFAULT
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
#define REGTEST(TEST) #TEST,
  const char* testnames[] = {
  #include "tests.h"
  };
#undef REGTEST
  

#ifdef ONLY_OVERVIEWS
  const int tstart = 1, tstop = 21;
#else
  const int tstart = 0, tstop = sizeof(testnames)/sizeof(const char*);
#endif
  QStringList   tests;
  for (int i=tstart; i<tstop; i++)
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
    qlv->setCurrentRow(1 - tstart);
    qlv->setMinimumHeight(qlv->sizeHintForRow(0)*tests.count() + 10);
    lay->addWidget(qlv);
    QPushButton*  btn = new QPushButton("Accept");
    QObject::connect(btn, SIGNAL(clicked()), &dlg, SLOT(accept()));
    lay->addWidget(btn);
    dlg.setLayout(lay);
    if (dlg.exec() != QDialog::Accepted)
      return 0;
    MW_TEST = (tests_t)(tstart + qlv->currentIndex().row());
  }
#endif
  MainWindow w(MW_TEST);
  w.move(100, 200);
  w.show();
  return a.exec();
}
