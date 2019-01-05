## Overview

0. Optional Defines (.pro file)
* BSGLSLVER <version> - prepends each shader with "#version <version>" string.  By default all shaders are compatible with glsl 1.30. Example: DEFINES += BSGLSLVER=130
* BSSHADER_DUMP - write file fragshader.txt when compiling shader
* BSOVERLAYSLIMIT - array of overlays has fixed static length (by default 32), which can be redefined with DEFINES += BSOVERLAYSLIMIT=100 or any other number

1. Class hierarchy

* Base class, containing most operation methods:	      DrawCore
* Next level, Qt implementation of shaders:             DrawQWidget
* Next level, 4 main realizations:                      DrawGraph, DrawRecorder, DrawIntencity, DrawDomain
* Next level, additional realizations following mains:  DrawIntensePoints, DrawGraphMove, DrawGraphMoveEx

2. Concept

Most useful methods are provided by base class DrawCore. Some implementation settings are passed into constructors. When draw is created a lot of parameters are not accessible from his object. The reason is recompilation of shader.

3. Methods

banAutoUpdate		exclude autoredraw in the event that you use timer (use method <update> provided by QOpenGLWidget) or you dont want refresh draw after settings changed.
autoUpdateBanned	true/false for selected redraw

countPortions		returns the amount of information displayed on draw at the same time
setBounds,
setBoundLow,
setBoundHigh		lower and higher float bounds will eq low and high colors of palette. (0.0, 1.0) by default.
bounds			returns current bounds
setContrast		coeff in formula k*x + b, same for palette. (0.0, 1.0) by default.
contrast		returns current contrast

setScalingLimitsH		sets horizontal expansion limits when layout resizes your draw
setScalingLimitsV		sets vertical expansion limits
setScalingLimitsSynced same but draw will be resized simultaneously hors + vert
scalingLimitsH			returns horizontal limits
scalingLimitsV			returns vertical limits

setDataPalette			some useful palettes you can find in bspalettes_std.h, bspalettes_adv.h, bspalettes_rgb.h
setDataPaletteDiscretion	sets simple smooth option for palette

setData							float data array of size countPortions*samples*[lines]. Size is not passed into method because size is constant for draw.
clearData						all data is set to lower bound

directions					1D or 2D data used

setOrientation			mirroring horizontal and vertical
orientation					returns current orientation

ovlPushBack					add overlay and choose if will be overlay destroyed by draw when other references to overlay will be disconnected. Optional: root overlay is a source of coordinates
ovlReplace					sets new overlay and returns old
ovlGet							returns overlay by index. ovl numeration starts from 1
ovlPopBack					remove and returns last overlay. If draw is owner and no more references found it will destroy overlay and returns nullptr
ovlClearAll					remove all overlays. Destroy behavior is like in ovlPopBack

setPostMask         sets effects applyed after data is drawed. Cells/lines etc
postProcess					returns current postProcess

setClearColor				sets color used by fill free area around draw
setClearByPalette		clearcolor is lowest palette color
