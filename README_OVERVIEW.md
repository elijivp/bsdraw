## Overview

0. Optional Defines (.pro file)
* BSGLSLVER <version> - prepends each shader with "#version <version>" string.  By default all shaders are compatible with glsl 1.30. Example: DEFINES += BSGLSLVER=130
* BSSHADER_DUMP - write file fragshader.txt when compiling shader
* BSOVERLAYSLIMIT - array of overlays has fixed static length (by default 32), which can be redefined with DEFINES += BSOVERLAYSLIMIT=100 or any other number

1. Class hierarchy

* Base class, containing most operation methods:	DrawCore
* Next level, Qt implementation of shaders:             DrawQWidget
* Next level, 4 main realizations:                      DrawGraph, DrawRecorder, DrawIntencity, DrawDomain
* Next level, additional realizations following mains:  DrawIntensePoints, DrawGraphMove, DrawGraphMoveEx

2. Concept

Most useful methods are provided by base class DrawCore. Some implementation settings are passed into constructors. When draw is created a lot of parameters are not accessible from his object. The reason is recompilation of shader.

3. Methods

* __banAutoUpdate__  exclude autoredraw in the event that you use timer (use method <update> provided by QOpenGLWidget) or you dont want refresh draw after settings changed.
* __autoUpdateBanned__  true/false for selected redraw

* __countPortions__  returns the amount of information displayed on draw at the same time
* __setBounds__,
* __setBoundLow__,
* __setBoundHigh__  lower and higher float bounds will eq low and high colors of palette. (0.0, 1.0) by default.
* __bounds__  returns current bounds
* __setContrast__  coeff in formula k*x + b, same for palette. (0.0, 1.0) by default.
* __contrast__  returns current contrast

* __setScalingLimitsH__  sets horizontal expansion limits when layout resizes your draw
* __setScalingLimitsV__  sets vertical expansion limits
* __setScalingLimitsSynced__  same but draw will be resized simultaneously hors + vert
* __scalingLimitsH__  returns horizontal limits
* __scalingLimitsV__  returns vertical limits

* __setDataPalette__  some useful palettes you can find in bspalettes_std.h, bspalettes_adv.h, bspalettes_rgb.h
* __setDataPaletteDiscretion__  sets simple smooth option for palette

* __setData__  float data array of size countPortions*samples*[lines]. Size is not passed into method because size is constant for draw.
* __clearData__  all data is set to lower bound

* __directions__  1D or 2D data used

* __setOrientation__  mirroring horizontal and vertical
* __orientation__  returns current orientation

* __ovlPushBack__  add overlay and choose if will be overlay destroyed by draw when other references to overlay will be disconnected. Optional: root overlay is a source of coordinates
* __ovlReplace__  sets new overlay and returns old
* __ovlGet__  returns overlay by index. ovl numeration starts from 1
* __ovlPopBack__  remove and returns last overlay. If draw is owner and no more references found it will destroy overlay and returns nullptr
* __ovlClearAll__  remove all overlays. Destroy behavior is like in ovlPopBack

* __setPostMask__  sets effects applyed after data is drawed. Cells/lines etc
* __postMask__  returns current effects

* __setClearColor__  sets color used by fill free area around draw
* __setClearByPalette__  clearcolor is lowest palette color
