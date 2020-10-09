#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/// MainWindow for simple example
/// Created By: Elijah Vlasov

#include <QMainWindow>

class DrawQWidget;
class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
};

#endif // MAINWINDOW_H
