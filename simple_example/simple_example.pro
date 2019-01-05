#-------------------------------------------------
#
# Project created by QtCreator 2018-12-06T18:50:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  CONFIG += c++11
}
lessThan(QT_MAJOR_VERSION, 5) {
  QT += opengl
  DEFINES += "nullptr=0"
  CONFIG += c++0x
}

TARGET = simple_example
TEMPLATE = app

DEFINES += BSGLSLVER=130
DEFINES += BSOVERLAYSLIMIT=5

SOURCES += main.cpp\
        MainWindow.cpp \
    ../bsdrawgraph.cpp \
    ../bsdrawintensity.cpp \
    ../core/bsqdraw.cpp \
    ../core/sheigen/bsshgenmain.cpp

HEADERS  += MainWindow.h \
    ../bsdrawintensity.h \
    ../bsdrawgraph.h \
    ../core/bsdraw.h \
    ../core/bsidrawcore.h \
    ../core/bsqdraw.h \
    ../core/sheigen/bsshgenmain.h
