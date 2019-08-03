/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_GRAPHICSDEVICE_H
#define GFX_GRAPHICSDEVICE_H

// include the platform-specific device implementation
#if defined(PLATFORM_PC)
#	include "gfx/graphicsdevice_pc.h"
#elif defined(PLATFORM_PS3)
#	include "gfx/graphicsdevice_ps3.h"
#endif

class GraphicsDevice : public Singleton<GraphicsDevice>
{
public:
	friend class GraphicsDevicePlatform;

	//! ctor
	GraphicsDevice();

	//! dtor
	~GraphicsDevice();
	
	//! the game refresh rate, half of GetDisplayRefreshRate()
#ifdef PLATFORM_PC
	float	GetGameRefreshRate() const { return _R( GetDisplayRefreshRate() ) * 0.5f; }
#else
	float	GetGameRefreshRate() const { return m_fGameRefreshRate; }
#endif

	//! platform specific stuff
	GraphicsDevicePlatform m_Platform;

private:

#ifdef PLATFORM_PC
	//! the dimensions of our graphics device back buffer (not the same as the 
	//! game back buffer on the PC in full screen mode)
	void	GetScreenResolution( int& iScreenX, int& iScreenY ) { iScreenX = m_iScreenX; iScreenY = m_iScreenY; }

	//! the refresh rate of the monitor (e.g 60Hz in fullscreen on PC, user setting in windowed)
	int		GetDisplayRefreshRate() const { return m_iDisplayRefreshRate; }

	int m_iDisplayRefreshRate;
	int m_iScreenX;
	int m_iScreenY;
#else
	float m_fGameRefreshRate;
#endif

};

// helper function for PC 
#if defined( PLATFORM_PC )
inline IDirect3DDevice9* GetD3DDevice()
{
	return GraphicsDevice::Get().m_Platform.Get();
}
#endif

#endif // ndef _GRAPHICSDEVICE_H
