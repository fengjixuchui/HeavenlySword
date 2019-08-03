//--------------------------------------------------
//!
//!	\file gfx/display.cpp
//!
//--------------------------------------------------

#ifdef PLATFORM_PS3
#include <Gc/GcKernel.h>
#endif

#include "gfx/display.h"
#include "gfx/renderersettings.h"
#include "game/shellconfig.h"

//-----------------------------------------------------
//!
//!	work out what genuine output mode were in	
//!
//----------------------------------------------------
void DisplayManager::ConfigureDisplay()
{
#ifdef PLATFORM_PS3
	m_eInternalMode = CRendererSettings::eDisplayMode;

	// setup internal display properties
	//------------------------------------------------
	switch ( m_eInternalMode )
	{
	case FOUR_THREE_480:
		m_fWidthInternal = 640.f;
		m_fHeightInternal = 480.f;
		break;

	case FOUR_THREE_720:
		m_fWidthInternal = 960.f;
		m_fHeightInternal = 720.f;
		break;

	case FOUR_THREE_1080:
		m_fWidthInternal = 1440.f;
		m_fHeightInternal = 1080.f;
		break;

	case SIXTEEN_NINE_480:
		m_fWidthInternal = 853.333333333333333f;
		m_fHeightInternal = 480.f;
		break;

	case SIXTEEN_NINE_720:
		m_fWidthInternal = 1280.f;
		m_fHeightInternal = 720.f;
		break;

	case SIXTEEN_NINE_1080:
		m_fWidthInternal = 1920.f;
		m_fHeightInternal = 1080.f;
		break;

	default:
		user_error_p( 0, ("Unsupported internal resolution: %d", m_eInternalMode ) );
		break;
	}

	bool bInternalWidescreen = false;

	if ((m_eInternalMode == FOUR_THREE_480) || (m_eInternalMode == FOUR_THREE_720) || (m_eInternalMode == FOUR_THREE_1080))
		m_fAspectInternal = 4.0f / 3.0f;
	else
	{
		m_fAspectInternal = 16.0f / 9.0f;
		bInternalWidescreen = true;
	}

	// setup output display properties
	//------------------------------------------------
	switch ( GcKernel::GetDisplayHeight() )
	{
	case 480:
		m_eOutputMode = (GcKernel::GetDisplayAspectRatio()==Gc::kAspectStandard) ? NTSC_480_4_3 : NTSC_480_16_9;
		ntError_p( GcKernel::GetDisplayWidth() == 720, ("Expecting horzontal rez of 720, got %d", GcKernel::GetDisplayWidth() ) );
		
		m_fWidthDevice = 720.f;
		m_fHeightDevice = 480.f;
		break;

	case 576:
		m_eOutputMode = (GcKernel::GetDisplayAspectRatio()==Gc::kAspectStandard) ? PAL_576_4_3 : PAL_576_16_9;
		ntError_p( GcKernel::GetDisplayWidth() == 720, ("Expecting horzontal rez of 720, got %d", GcKernel::GetDisplayWidth() ) );
		
		m_fWidthDevice = 720.f;
		m_fHeightDevice = 576.f;		
		break;

	case 720:
		m_eOutputMode = HD_720;
		ntError_p( GcKernel::GetDisplayWidth() == 1280, ("Expecting horzontal rez of 1280, got %d", GcKernel::GetDisplayWidth() ) );

		m_fWidthDevice = 1280.f;
		m_fHeightDevice = 720.f;
		break;

	case 1080:
		m_eOutputMode = HD_1080;
		ntError_p( GcKernel::GetDisplayWidth() == 1920, ("Expecting horzontal rez of 1920, got %d", GcKernel::GetDisplayWidth() ) );
		
		m_fWidthDevice = 1920.f;
		m_fHeightDevice = 1080.f;		
		break;

	default:
		user_error_p( 0, ("Unsupported output resolution: %d by %d.", GcKernel::GetDisplayWidth(), GcKernel::GetDisplayHeight() ) );
		break;
	};

	bool bExternalWidescreen = false;

	if ((m_eOutputMode == NTSC_480_4_3) || (m_eOutputMode == PAL_576_4_3))
		m_fAspectDevice = 4.0f / 3.0f;
	else
	{
		m_fAspectDevice = 16.0f / 9.0f;
		bExternalWidescreen = true;
	}

	// work out if our internal mode matches any output mode
	//------------------------------------------------------
	m_bDeviceMatchesInternal = false;

	if	(
		((m_eInternalMode == FOUR_THREE_480) && (m_eOutputMode == NTSC_480_4_3)) ||
		((m_eInternalMode == FOUR_THREE_480) && (m_eOutputMode == PAL_576_4_3)) ||
		((m_eInternalMode == SIXTEEN_NINE_480) && (m_eOutputMode == NTSC_480_16_9)) ||
		((m_eInternalMode == SIXTEEN_NINE_480) && (m_eOutputMode == PAL_576_16_9)) ||
		((m_eInternalMode == SIXTEEN_NINE_720) && (m_eOutputMode == HD_720)) ||
		((m_eInternalMode == SIXTEEN_NINE_1080) && (m_eOutputMode == HD_1080))
		)
		m_bDeviceMatchesInternal = true;

	m_bAspectMatchesInternal = false;
	if (bExternalWidescreen == bInternalWidescreen)
		m_bAspectMatchesInternal = true;

#else
	m_fWidthInternal = (float)g_ShellOptions->m_iBBWidth;
	m_fHeightInternal = (float)g_ShellOptions->m_iBBHeight;
	m_fAspectInternal = m_fWidthInternal / m_fHeightInternal;

	m_bFullscreen = g_ShellOptions->m_bFullscreen;

	if (m_bFullscreen)
	{
		m_fWidthDevice = (float)g_ShellOptions->m_iFSWidth;
		m_fHeightDevice = (float)g_ShellOptions->m_iFSHeight;
	}
	else
	{
		m_fWidthDevice = (float)g_ShellOptions->m_iBBWidth;
		m_fHeightDevice = (float)g_ShellOptions->m_iBBHeight;
	}
	m_fAspectDevice = m_fWidthDevice / m_fHeightDevice;
#endif
}
