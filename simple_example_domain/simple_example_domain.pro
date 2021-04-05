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

TARGET = simple_example_domain
TEMPLATE = app

INCLUDEPATH += ../

DEFINES += BSGLSLVER=130

SOURCES += main.cpp\
    ../bsdrawdomain.cpp \
    ../core/bsqdraw.cpp \
    ../core/sheigen/bsshgenmain.cpp

HEADERS  += ../bsdrawdomain.h \
    ../core/bsdraw.h \
    ../core/bsqdraw.h \
    ../core/sheigen/bsshgenmain.h
