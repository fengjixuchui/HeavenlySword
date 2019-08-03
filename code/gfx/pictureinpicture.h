//--------------------------------------------------
//!
//!	\file pictureinpicture.h
//!	objects managing final back buffer compositing
//! of multiple viewports.
//!
//--------------------------------------------------

#ifndef GFX_PIP_H
#define GFX_PIP_H

#ifdef PLATFORM_PC
#include "gfx/pictureinpicture_pc.h"
#elif PLATFORM_PS3
#include "gfx/pictureinpicture_ps3.h"
#endif

#endif
