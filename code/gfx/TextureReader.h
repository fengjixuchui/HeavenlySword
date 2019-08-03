//--------------------------------------------------
//!
//!	\file TextureReader.h
//!	Class that wraps the getting and setting of 
//! texture values
//!
//--------------------------------------------------

#ifndef _TEX_READER_H
#define _TEX_READER_H

#if defined( PLATFORM_PC )
#	include "gfx/texturereader_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/texturereader_ps3.h"
#endif

#endif
