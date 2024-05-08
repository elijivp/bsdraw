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

TARGET = simple_example_2D_with_scales
TEMPLATE = app

INCLUDEPATH += ../

DEFINES += BSGLSLVER=130


SOURCES += main.cpp\
    ../bsdrawintensity.cpp \
    ../bsdrawscales.cpp \
    ../core/bsqdraw.cpp \
    ../core/sheigen/bsshgenmain.cpp

HEADERS  += ../bsdrawintensity.h \
    ../bsdrawscales.h \
    ../core/bsdraw.h \
    ../core/bsqdraw.h \
    ../core/sheigen/bsshgenmain.h
