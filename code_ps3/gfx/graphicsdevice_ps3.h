#ifndef GFX_GRAPHICSDEVICE_PS3_H_
#define GFX_GRAPHICSDEVICE_PS3_H_

#ifndef GFX_RENDERERSETTINGS_H
#include "gfx/renderersettings.h"
#endif

//------------------------------------------------------------
//!
//! A wrapper around the PS3 graphics device.
//! Its contains and control the lowest level of the graphics
//! sub-system
//!
//------------------------------------------------------------
class GraphicsDevicePlatform
{
public:
	friend class GraphicsDevice;
	bool Init();

private:
	class GraphicsDevice*	pThis;	//<! Pointer back to our containering class
};

#endif //_GRAPHICSDEVICE_PS3_H_
