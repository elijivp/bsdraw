#include <QApplication>
#include "mainwindow.h"

/*  elirupost@gmail.com
 * 
 if (rabota == volk)
  rabota.doRun(&les);
*/

//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)1
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)2
#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)8
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)10
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)12
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)13
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)15
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)18
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)21
//#define MW_TEST_NOCHOOSE_DEFAULT (tests_t)26
#ifndef MW_TEST_NOCHOOSE_DEFAULT
#include <QInputDialog>
#endif

int main(int argc, char *argv[])
{  
  QApplication a(argc, argv);

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
  QString result = QInputDialog::getItem(nullptr, "bsdraw", prepend + "Choose test:", tests, 1, false, &ok);
  if (!ok)  return 0;
  MW_TEST = (tests_t)tests.indexOf(result);
#endif
  MainWindow w(MW_TEST);
  w.move(100, 200);
//  w.resize(1400, 1000);
  w.show();
  return a.exec();
}
