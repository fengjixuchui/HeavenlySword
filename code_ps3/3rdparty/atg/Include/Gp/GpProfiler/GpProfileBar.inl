//--------------------------------------------------------------------------------------------------
/**
	@file		GpProfileBar.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_PROFILE_BAR_INL
#define GP_PROFILE_BAR_INL

//--------------------------------------------------------------------------------------------------

// Available only in profile-enabled builds!

#ifdef ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::StartTimer(u64 ticks)
{
	m_startTick = ticks;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::StopTimer(u64 ticks)
{
	m_pTickMarkers[m_numTickMarkers - 1].m_numTicks += ticks - m_startTick;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::AddTimerMarker(u64 ticks)
{
	AddTimerMarker(ticks, kDefaultTickMarkerColours[m_numTickMarkers % kMaxDefaultTickMarkerColours]);
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::StartTimer32Bit(u32 ticks)
{
	m_startTick = ticks;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::StopTimer32Bit(u32 ticks)
{
	// :NOTE: We subtract ticks using unsigned 32-bit arithematic so underflow wraps correctly!
	m_pTickMarkers[m_numTickMarkers - 1].m_numTicks += ticks - (u32) m_startTick;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::AddTimerMarker32Bit(u32 ticks)
{
	AddTimerMarker32Bit(ticks, kDefaultTickMarkerColours[m_numTickMarkers % kMaxDefaultTickMarkerColours]);
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::ResetWatermark(uint restartDelay)
{
	m_maxTicksDelay = restartDelay;
	m_maxTicks = 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::SetVisibility(bool isVisible)
{
	m_isVisible = isVisible;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::SetDrawMode(DrawMode mode)
{
	m_drawMode = mode;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::SetInitialColour(GcColour_arg colour)
{
	m_initialColour = colour;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::SetAutoColours(	GcColour_arg colour1st,
											GcColour_arg colour2nd,
											GcColour_arg colour3rd,
											GcColour_arg colour4th)
{
	m_autoColours[0] = colour1st;
	m_autoColours[1] = colour2nd;
	m_autoColours[2] = colour3rd;
	m_autoColours[3] = colour4th;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBar::SetTicksToSecondsScalar(float ticksToSecs)
{
	FW_ASSERT(ticksToSecs != 0.0f);
	
	m_ticksToSecs = ticksToSecs;
}

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------
	
//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------
	
inline GpProfileBar& GpProfileBarManager::GetProfileBar(GpProfiler::UnitId barId)
{
	FW_ASSERT(ms_instance.m_isInitialised);
	
	return ms_instance.m_profileBars[GetBarIndex(barId)];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::SetDrawOrigin(float x, float y)
{
	FW_ASSERT((x >= 0.0f) && (x <= 1.0f));
	FW_ASSERT((y >= 0.0f) && (y <= 1.0f));
		
	ms_instance.m_drawOriginX = x;
	ms_instance.m_drawOriginY = y;
	
	RequestStaticPrimsUpdate();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::GetDrawOrigin(float* pX, float* pY)
{
	FW_ASSERT(pX && pY);
	
	*pX = ms_instance.m_drawOriginX;
	*pY = ms_instance.m_drawOriginY;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::SetDrawWidth(float width)
{
	FW_ASSERT((width > 0.0f) && (width <= 1.0f));
	
	ms_instance.m_drawWidth = width;
	
	RequestStaticPrimsUpdate();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline float GpProfileBarManager::GetDrawWidth()
{
	return ms_instance.m_drawWidth; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::SetTransparency(float alpha)
{
	FW_ASSERT((alpha >= 0.0f) && (alpha <= 1.0f));
	
	ms_instance.m_transparency = alpha;
	
	RequestStaticPrimsUpdate();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline float GpProfileBarManager::GetTransparency()
{
	return ms_instance.m_transparency;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::SetFrameStartTick(u64 tick)
{
	ms_instance.m_frameStartTick = tick;	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline u64 GpProfileBarManager::GetFrameStartTick()
{
	return ms_instance.m_frameStartTick;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfileBarManager::SetRefreshRate(float refreshRate)
{
	ms_instance.m_refreshRate = refreshRate; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------
	
inline void GpProfileBarManager::SetNewDisplayResolution(float displayWidth, float displayHeight)
{
	GpProfileBarManager::SetDisplayDependentConstants(displayWidth, displayHeight);
	
	RequestStaticPrimsUpdate();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline uint GpProfileBarManager::GetBarIndex(uint barId)
{
	uint	i;

	FW_ASSERT(barId != 0);
	
	for (i = 0; !(barId & 0x1); barId >>= 1)
		++i;
	
	FW_ASSERT(i < GpProfiler::kNumUnits);		// Ensure index is valid

	return i;
}

//--------------------------------------------------------------------------------------------------

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_PROFILE_BAR_INL

