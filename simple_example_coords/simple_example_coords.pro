#-------------------------------------------------
#
# Project created by QtCreator 2018-12-06T18:50:00
#
#-------------------------------------------------

QT       += core gui

equals(QT_MAJOR_VERSION, 4) {
  QT += opengl
  DEFINES += "nullptr=0"
  CONFIG += c++0x
}
equals(QT_MAJOR_VERSION, 5) {
  QT += opengl widgets
  CONFIG += c++11
}
equals(QT_MAJOR_VERSION, 6) {
  QT += openglwidgets
}

TARGET = simple_example_1D
TEMPLATE = app

INCLUDEPATH += ../

DEFINES += BSGLSLVER=130

SOURCES += main.cpp\
    ../bsdrawgraph.cpp \
    ../core/bsoverlay.cpp \
    ../core/bsqdraw.cpp \
    ../core/bsqoverlay.cpp \
    ../core/sheigen/bsshgencolor.cpp \
    ../core/sheigen/bsshgenmain.cpp \
    ../core/sheigen/bsshgentrace.cpp \
    ../overlays/bsfigures.cpp \
    ../overlays/bsinteractive.cpp

HEADERS  += ../bsdrawgraph.h \
    ../core/bsdraw.h \
    ../core/bsoverlay.h \
    ../core/bsqdraw.h \
    ../core/bsqoverlay.h \
    ../core/sheigen/bsshgencolor.h \
    ../core/sheigen/bsshgenmain.h \
    ../core/sheigen/bsshgenparams.h \
    ../core/sheigen/bsshgentrace.h \
    ../overlays/bsfigures.h \
    ../overlays/bsinteractive.h
