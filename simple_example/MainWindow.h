#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DrawQWidget;
class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  DrawQWidget* pDraw;
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
};

#endif // MAINWINDOW_H
