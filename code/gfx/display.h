#if !defined(GFX_DISPLAY_H)
#define GFX_DISPLAY_H
//-----------------------------------------------------
//!
//!	\file gfx\display.h
//! A Display is an object representing a TV or monitor
//! currently its largely a null object, as we only support
//! one etc. 
//!
//----------------------------------------------------

//-----------------------------------------------------
//!
//!	
//!
//----------------------------------------------------
class DisplayManager : public Singleton< DisplayManager >
{
public:
	void ConfigureDisplay();

	// used by the game, determines our 'virtual' display
	inline float GetInternalWidth() const { return m_fWidthInternal; }
	inline float GetInternalHeight() const { return m_fHeightInternal; }
	inline float GetInternalAspect() const { return m_fAspectInternal; }

	// the output devices actual dimesions and aspect (NOT! a function of dimension)
	inline float GetDeviceWidth() const { return m_fWidthDevice; }
	inline float GetDeviceHeight() const { return m_fHeightDevice; }
	inline float GetDeviceAspect() const { return m_fAspectDevice; }

#ifdef PLATFORM_PS3
	// INTERNAL 'VIRTUAL' DISPLAY MODES SUPPORTED BY GAME
	enum INTERNAL_MODE
	{
		FOUR_THREE_480,		// 640x480
		FOUR_THREE_720,		// 960x720
		FOUR_THREE_1080,	// 1440x1080

		SIXTEEN_NINE_480,	// 853x480
		SIXTEEN_NINE_720,	// 1280x720
		SIXTEEN_NINE_1080,	// 1920x1080
	};

	// DISPLAY OUTPUT MODES SUPPORTED BY PS3
	enum OUTPUT_MODE
	{
		// use a 720 x 480 out buffer, at 59.94 Hz
		NTSC_480_4_3,
		NTSC_480_16_9,

		// use a 720 x 576 out buffer, at 50 Hz
		PAL_576_4_3,
		PAL_576_16_9,

		// use a 1280 x 720 out buffer, at 59.94 Hz
		HD_720,

		// use a 1920 x 1080 out buffer, at 59.94 Hz
		HD_1080,
	};

	inline INTERNAL_MODE	GetInternalMode() const { return m_eInternalMode; }
	inline OUTPUT_MODE		GetOutputMode() const { return m_eOutputMode; }

	inline bool DeviceMatchesInternal() const { return m_bDeviceMatchesInternal; }
	inline bool AspectMatchesInternal() const { return m_bAspectMatchesInternal; }
#else
	inline bool IsFullScreen() const { return m_bFullscreen; }
#endif

private:
	float	m_fWidthInternal;
	float	m_fHeightInternal;
	float	m_fAspectInternal;

	float	m_fWidthDevice;
	float	m_fHeightDevice;
	float	m_fAspectDevice;

#ifdef PLATFORM_PS3
	INTERNAL_MODE	m_eInternalMode;
	OUTPUT_MODE		m_eOutputMode;
	bool			m_bDeviceMatchesInternal;
	bool			m_bAspectMatchesInternal;
#else
	bool			m_bFullscreen;
#endif
};

#endif // GFX_DISPLAY_H
