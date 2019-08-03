/***************************************************************************************************
*
*	$Header:: /game/graphicsdevice.h 3     16/06/03 10:41 Simonb                                   $
*
*	A Wrapper Around the Xbox DirectX Graphics Device 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef GFX_GRAPHICSDEVICE_PC_H
#define GFX_GRAPHICSDEVICE_PC_H

//------------------------------------------------------------
//!
//! A wrapper around the PC graphics device.
//! Its contains and control the lowest level of the graphics
//! sub-system
//!
//------------------------------------------------------------

class GraphicsDevicePlatform
{
public:
	friend class GraphicsDevice;
	friend class HardwareCapabilities;

	//! Gets the device.
	IDirect3DDevice9* Get() const { return m_pDevice; }

	//! create a full screen device, if hwnd == 0 (default) create and manage the window internally
	bool InitFullScreen( int width, int height, const char* pTitle, void* hwnd = 0 );
	//! create a windowed device, if hwnd == 0 (default) create and manage the window internally
	bool InitWindowed(	int width, int height, const char* pTitle,void* hwnd = 0 );

	//! Complete tear down and restart (not tested!)
	void ResetDevice();

	//! the windows handle we consider the focus window (ownership depedent on creation params)
	void* GetHwnd() { return m_pFocusWindow; };

	//! are we in full screen mode
	bool GetFullScreen() const { return m_bFullScreen; }

private:
	//! capbs bit helpers, private so people have to use
	//! HardwareCapabilities(), which caches these results

	bool IsValidRenderTargetType( D3DFORMAT eType );
	bool IsValidTextureType( D3DFORMAT eType );
	bool IsValidDepthFormat( D3DFORMAT eType );
	bool IsValidAlphaBlendRTFormat( D3DFORMAT eType );
	bool SupportsHardwareShadowMaps();

	class GraphicsDevice*	pThis;	//<! Pointer back to our containering class

	IDirect3D9*			m_pDirect3D;			//!< The direct3d device.
	IDirect3DDevice9*	m_pDevice;				//!< The graphics device.
	void*				m_pFocusWindow;			//!< a windows handle
	bool				m_bOwnWindow;			//!< do own this windows
	bool				m_bFullScreen;			//!< Are we full screen
};


#endif // ndef _GRAPHICSDEVICE_PC_H
