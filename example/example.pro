#-------------------------------------------------
#
# Project created by QtCreator 2017-12-14T18:34:15
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

DEFINES += BSGLSLVER=130     # no "#version" string includes in shader if not defined
#DEFINES += BSGLSLVER=450     # no "#version" string includes in shader if not defined
DEFINES += BS_TAU_PROPERTY_OFF

#DEFINES += BSGLSLOLD

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
    ../specdraws/bsdrawpolar.cpp \
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
    ../overlays/bsimage.cpp \
    ../bsdrawscales.cpp \
    ../specdraws/bsdrawempty.cpp \
    ../overlays/special/bssnowflake.cpp \
    ../specdraws/bsdrawsdpicture.cpp \
    ../core/bsqoverlay.cpp \
    ../overlays/special/bsbounded.cpp \
    ../palettes/QPaletteExtractor.cpp \
    ../specdraws/bsdrawhint.cpp \
    mainwindow.cpp

HEADERS  += \
    ../specdraws/bsdrawpolar.h \
    mainwindow.h \
    ../core/bsidrawcore.h \
    ../core/bsdraw.h \
    ../core/bsoverlay.h \
    ../palettes/bsipalette.h \
    ../palettes/bspalettes_adv.h \
    ../palettes/bspalettes_rgb.h \
    ../palettes/bspalettes_std.h \
    ../bsdrawgraph.h \
    ../bsdrawintensity.h \
    ../bsdrawrecorder.h \
    ../core/bsqdraw.h \
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
    tests.h \
    ../bsdrawscales.h \
    ../specdraws/bsdrawempty.h \
    ../overlays/special/bssnowflake.h \
    ../specdraws/bsdrawsdpicture.h \
    ../core/sheigen/bsshgenparams.h \
    ../core/bsqoverlay.h \
    ../overlays/special/bsbounded.h \
    ../palettes/advanced/gnu_coffee.h \
    ../palettes/advanced/idl_waves.h \
    ../palettes/advanced/idl_yiBuGn.h \
    ../palettes/advanced/tfm_hot_metal.h \
    ../palettes/advanced/idl_BuPuGn.h \
    ../palettes/advanced/tfm_grayscale_banded.h \
    ../palettes/advanced/idl_Or.h \
    ../palettes/advanced/gnu_latte.h \
    ../palettes/advanced/tfm_peanut_butter_and_jerry.h \
    ../palettes/advanced/idl_BuPu.h \
    ../palettes/advanced/idl_wavesBu.h \
    ../palettes/advanced/mat_hsv.h \
    ../palettes/advanced/idl_prism.h \
    ../palettes/advanced/idl_BuGn.h \
    ../palettes/advanced/idl_linearBuWh.h \
    ../palettes/advanced/idl_pinkGn.h \
    ../palettes/advanced/idl_stern_special.h \
    ../palettes/advanced/tfm_rainbow.h \
    ../palettes/advanced/tfm_space.h \
    ../palettes/advanced/mat_pink.h \
    ../palettes/advanced/idl_stripesPuRd.h \
    ../palettes/advanced/idl_OrPu.h \
    ../palettes/advanced/mat_cool.h \
    ../palettes/advanced/idl_hue_sat_lightness_2.h \
    ../palettes/advanced/idl_GyRd.h \
    ../palettes/advanced/tfm_morning_glory.h \
    ../palettes/advanced/idl_prGn.h \
    ../palettes/advanced/idl_eos_a.h \
    ../palettes/advanced/idl_piyg.h \
    ../palettes/advanced/tfm_malachite.h \
    ../palettes/advanced/idl_Gy.h \
    ../palettes/advanced/tfm_rose.h \
    ../palettes/advanced/idl_pastel1.h \
    ../palettes/advanced/idl_BuYeGnRd.h \
    ../palettes/advanced/idl_bgBr.h \
    ../palettes/advanced/tfm_lava_waves.h \
    ../palettes/advanced/idl_accent.h \
    ../palettes/advanced/mat_jet.h \
    ../palettes/advanced/tfm_saturn.h \
    ../palettes/advanced/idl_paired.h \
    ../palettes/advanced/idl_standard_gammaii.h \
    ../palettes/advanced/idl_volcano.h \
    ../palettes/advanced/idl_peppermint.h \
    ../palettes/advanced/idl_dark2.h \
    ../palettes/advanced/idl_rainbowBk.h \
    ../palettes/advanced/idl_hue_sat_lightness_1.h \
    ../palettes/advanced/mat_winter.h \
    ../palettes/advanced/idl_hue_sat_value_2.h \
    ../palettes/advanced/idl_pastels.h \
    ../palettes/advanced/tfm_purple_haze.h \
    ../palettes/advanced/idl_linearGnWh.h \
    ../palettes/advanced/idl_yiRdGn.h \
    ../palettes/advanced/idl_Bu.h \
    ../palettes/advanced/mat_bone.h \
    ../palettes/advanced/gnu_PuYe.h \
    ../palettes/advanced/idl_plasma.h \
    ../palettes/advanced/idl_yiGn.h \
    ../palettes/advanced/idl_mac_style.h \
    ../palettes/advanced/idl_Rd.h \
    ../palettes/advanced/idl_OrRd.h \
    ../palettes/advanced/idl_PuRd.h \
    ../palettes/advanced/idl_set3.h \
    ../palettes/advanced/idl_beach.h \
    ../palettes/advanced/gnu_flowers.h \
    ../palettes/advanced/idl_RdPu.h \
    ../palettes/advanced/idl_set2.h \
    ../palettes/advanced/idl_BuGnWhRd.h \
    ../palettes/advanced/idl_rainbow_18.h \
    ../palettes/advanced/idl_yiRdBu.h \
    ../palettes/advanced/gnu_oceanBuGn.h \
    ../palettes/advanced/idl_spectral.h \
    ../palettes/advanced/tfm_seismic.h \
    ../palettes/advanced/idl_steps.h \
    ../palettes/advanced/idl_rainbow_2.h \
    ../palettes/advanced/idl_hardcandy.h \
    ../palettes/advanced/mat_spring.h \
    ../palettes/advanced/idl_pastelBuRd.h \
    ../palettes/advanced/idl_hue_sat_value_1.h \
    ../palettes/advanced/idl_Pu.h \
    ../palettes/advanced/idl_ocean.h \
    ../palettes/advanced/mat_hot.h \
    ../palettes/advanced/tfm_carnation.h \
    ../palettes/advanced/idl_Gn.h \
    ../palettes/advanced/mat_autumn.h \
    ../palettes/advanced/idl_pastel2.h \
    ../palettes/advanced/idl_rainbow.h \
    ../palettes/advanced/idl_haze.h \
    ../palettes/advanced/idl_16_level.h \
    ../palettes/advanced/tfm_supernova.h \
    ../palettes/advanced/tfm_ether.h \
    ../palettes/advanced/mat_summer.h \
    ../palettes/advanced/gnu_rainbow.h \
    ../palettes/advanced/tfm_apricot.h \
    ../palettes/advanced/idl_yiBrOr.h \
    ../palettes/advanced/idl_eos_b.h \
    ../palettes/advanced/mat_parula.h \
    ../palettes/advanced/idl_temperatureRd.h \
    ../palettes/advanced/idl_2BuRd.h \
    ../palettes/advanced/idl_set1.h \
    ../palettes/advanced/idl_exponentialGnWh.h \
    ../palettes/advanced/idl_BuRd.h \
    ../palettes/advanced/idl_linearBkWh.h \
    ../palettes/advanced/idl_rainbowWh.h \
    ../palettes/advanced/gnu_hotsteel.h \
    ../palettes/advanced/mat_copper.h \
    ../palettes/advanced/idl_nature.h \
    ../palettes/QPaletteExtractor.h \
    ../palettes/bspalettes_spec.h \
    ../specdraws/bsdrawhint.h \
    ../core/bsgraphopts.h

RESOURCES += \
    ../palettes/QPaletteBox.qrc \
    resources.qrc
