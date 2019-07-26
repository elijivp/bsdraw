#-------------------------------------------------
#
# Project created by QtCreator 2017-12-14T18:34:15
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

#DEFINES += BSGLSLVER=130     # no "#version" string includes in shader if not defined
#DEFINES += BSGLSLVER=450     # no "#version" string includes in shader if not defined
#DEFINES += BSSHADER_DUMP     # write file fragshader.txt when compiling shader

TARGET = example
TEMPLATE = app

INCLUDEPATH += ../

QMAKE_CXXFLAGS += "-fno-exceptions"

unix:{
  LIBS += "-fopenmp"
  QMAKE_CXXFLAGS += "-fopenmp"
}
#win32: LIBS += "-lgomp"

SOURCES += main.cpp\
    mainwindow.cpp \
    ../core/bsoverlay.cpp \
    ../bsdrawgraph.cpp \
    ../bsdrawintensity.cpp \
    ../bsdrawrecorder.cpp \
    ../core/bsqdraw.cpp \
    ../overlays/bsborder.cpp \
    ../overlays/bsfigures.cpp \
    ../overlays/bsgrid.cpp \
    ../overlays/bspoints.cpp \
    ../overlays/bssprites.cpp \
    ../overlays/bstextstatic.cpp \
    ../overlays/special/bsmarks.cpp \
    ../overlays/special/bsblocker.cpp \
    ../overlays/bsinteractive.cpp \
    ../core/sheigen/bsshgencolor.cpp \
    ../core/sheigen/bsshgentrace.cpp \
    ../bsdrawdomain.cpp \
    ../core/sheigen/bsshgenmain.cpp \
    ../overlays/special/bstestprecision.cpp \
    ../overlays/bscontour.cpp \
    ../palettes/QPaletteBox.cpp \
    ../overlays/bsimage.cpp

HEADERS  += \
    mainwindow.h \
    ../core/bsidrawcore.h \
    ../core/bsdraw.h \
    ../core/bsoverlay.h \
    ../palettes/bsipalette.h \
    ../palettes/bstexpalette.h \
    ../palettes/bspalettes_adv.h \
    ../palettes/bspalettes_rgb.h \
    ../palettes/bspalettes_std.h \
    ../bsdrawgraph.h \
    ../bsdrawintensity.h \
    ../bsdrawrecorder.h \
    ../core/bsqdraw.h \
    ../core/sheigen/bsshei2d.h \
    ../overlays/bsgrid.h \
    ../overlays/bspoints.h \
    ../overlays/bstextstatic.h \
    ../overlays/bsfigures.h \
    ../overlays/bssprites.h \
    ../overlays/special/bsmarks.h \
    ../overlays/bsborder.h \
    ../overlays/special/bsblocker.h \
    ../overlays/bsinteractive.h \
    ../core/sheigen/bsshgentrace.h \
    ../core/sheigen/bsshgencolor.h \
    ../bsdrawdomain.h \
    ../core/sheigen/bsshgenmain.h \
    ../overlays/special/bstestprecision.h \
    ../overlays/bscontour.h \
    ../bsdecimators.h \
    ../palettes/QPaletteBox.h \
    ../overlays/bsimage.h \
    layout/bsqlayout.h \
    tests.h

RESOURCES += \
    ../palettes/QPaletteBox.qrc
