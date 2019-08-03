#ifndef _EXPLICITTEMPLATE_H_
#define _EXPLICITTEMPLATE_H_

//--------------------------------------------------
//!
//!	\file explicittemplate.h
//!	this file should contain all the explicit declaration of template
//! to avoid inlining too much and slow down compile time
//!
//--------------------------------------------------



#include "core/boostarray.h"



typedef Array<int,2> Pixel2;
typedef Array<int,3> Pixel3;
typedef Array<int,4> Pixel4;
typedef Array<float,2> Vec2;
typedef Array<float,3> Vec3;
typedef Array<float,4> Vec4;
typedef Array<float,16> Vec16;



#endif // end of _EXPLICITTEMPLATE_H_
