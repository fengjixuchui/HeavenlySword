//--------------------------------------------------------------------------------------------------
/**
	@file		GpProfileBar.h

	@brief		Profile bar visualisation.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_PROFILE_BAR_H
#define GP_PROFILE_BAR_H

//--------------------------------------------------------------------------------------------------

// Available only in profile-enabled builds!

#ifdef ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/time_util.h>

#include <Gc/GcColour.h>

#include <Gp/GpProfiler/GpProfiler.h>
#include <Gp/GpProfiler/Internal/GpProfilerDrawHelpers.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpProfileBarManager;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpProfileBar

	@brief	Histogram bar to visualise profiling information.
**/
//--------------------------------------------------------------------------------------------------

class GpProfileBar : public FwNonCopyable
{
public:

	// Enumerations
	
	enum DrawMode
	{
		kDrawModeAutoColour,
		kDrawModeShowMarkers
	};
	
	
	// Operations
	
	void	Reset();
	
	void	StartTimer(u64 ticks);
	void	StopTimer(u64 ticks);
	
	void	AddTimerMarker(u64 ticks);
	void	AddTimerMarker(u64 ticks, GcColour_arg colour);
	
	void	StartTimer32Bit(u32 ticks);
	void	StopTimer32Bit(u32 ticks);
	
	void	AddTimerMarker32Bit(u32 ticks);
	void	AddTimerMarker32Bit(u32 ticks, GcColour_arg colour);

	void	ResetWatermark(uint restartDelay = 0);
	
	
	// Accessors
	
	void	SetVisibility(bool isVisible);
	void	SetDrawMode(DrawMode mode);
	
	void	SetInitialColour(GcColour_arg colour);
	
	void	SetAutoColours(	GcColour_arg colour1st = GcColour(Gc::kColourGreen),
							GcColour_arg colour2nd = GcColour(Gc::kColourYellow),
							GcColour_arg colour3rd = GcColour(Gc::kColourOrange),
							GcColour_arg colour4th = GcColour(Gc::kColourRed));
	
	void	SetTicksToSecondsScalar(float ticksToSecs);


private:

	// Friends
	
	friend class GpProfileBarManager;
	
	
	// Constants

	static const uint		kMaxAutoColours					= 4;
	static const uint		kMaxDefaultTickMarkerColours	= 16;
	static const GcColour	kDefaultTickMarkerColours[kMaxDefaultTickMarkerColours];


	// Structure Definitions
	
	struct TickMarker
	{
		u64			m_numTicks;								///< No. of ticks elapsed
		GcColour	m_colour;								///< Bar colour representing elapsed time
	};
	
	
	// Attributes
	
	u64				m_startTick;							///< Current start tick
	
	uint			m_maxTickMarkers;						///< Maximum no. of tick markers
	uint			m_numTickMarkers;						///< No. of valid tick markers
	TickMarker*		m_pTickMarkers;							///< Tick Markers
	
	uint			m_maxTicksDelay;						///< Delay (in frames) before watermark is tracked
	u64				m_maxTicks;								///< Watermark i.e. maximum no. of ticks
	
	float			m_ticksToSecs;							///< Scalar to convert ticks to seconds
	
	bool			m_isVisible;							///< True if profile bar is visible, false otherwise
	DrawMode		m_drawMode;								///< Draw mode
	
	GcColour		m_initialColour;						///< Initial profile bar colour (for kDrawModeShowMarkers mode)
	GcColour		m_autoColours[kMaxAutoColours];			///< Colours for auto-colouring
		
	bool			m_updateStaticPrims;					///< True if static marker primitives need updating
			
	GpProfilerDrawHelpers	m_staticPrims;	  				///< Static primitives - compute once, draw many frames
	GpProfilerDrawHelpers	m_dynamicPrims;	  				///< Dynamic primitives - compute and draw each frame

	
	// Static Attributes
	
	static float	ms_ndcPixelWidth;
	static float	ms_ndcPixelHeight;
		  
	static float	ms_rulerMarkerHeight;
	static float	ms_barHeight;
	static float	ms_totalBarHeight;
	static float	ms_nextBarOffsetY;


	// Operations
	
	static void	SetDisplayDependentConstants(float displayWidth, float displayHeight);
	
	void		Initialise(uint numVSyncsPerFrame, uint maxTickMarkers);
	void		Destroy();
	
	void		Draw(GcContext& context, float x, float y, float alpha);
	
	void		DrawStaticMarkers(float x, float y, float alpha);
	void		DrawBarSegment(float sx, float ex, float y, GcColour_arg startColour, GcColour_arg endColour, float alpha);
	void		DrawWatermark(float x, float y, float alpha);
	
	void		RequestStaticPrimsUpdate() { m_updateStaticPrims = true; }
	
	
	// Accessors
	
	bool		IsVisible() const	{ return m_isVisible; }
};

//--------------------------------------------------------------------------------------------------
/**
	@class	GpProfileBarManager

	@brief	Profile bar manager.
**/
//--------------------------------------------------------------------------------------------------

class GpProfileBarManager : public FwNonCopyable
{
public:

	// Initialisation and Shutdown Functions
	
	static void		Initialise(uint numVSyncsPerFrame = 1, float refreshRate = 60.0f, uint maxTickMarkers = 128);
	static void		Shutdown();
	
	
	// Reset
	
	static void		Reset(uint barIds);
	
	
	// Draw Functions
	
	static void		Draw(GcContext& context = GcKernel::GetContext());
	
	
	// Helpers
	
	static void		UpdateRsxProfileBar();
	
	
	// Accessors
	
	static GpProfileBar&	GetProfileBar(GpProfiler::UnitId barId);
	
	static void		SetVisibility(uint barIds, bool isVisible);
	static void		SetDrawMode(uint barIds, GpProfileBar::DrawMode mode);
	
	static void		SetDrawOrigin(float x, float y);
	static void		GetDrawOrigin(float* pX, float* pY);
	
	static void		SetDrawWidth(float width);
	static float	GetDrawWidth();
	
	static void		SetTransparency(float alpha);
	static float	GetTransparency();

	static void		SetFrameStartTick(u64 tick);
	static u64		GetFrameStartTick();
	
	static void		SetRefreshRate(float refreshRate);

	static void		SetNewDisplayResolution(float	displayWidth = GcKernel::GetDisplayWidth(),
											float	displayHeight = GcKernel::GetDisplayHeight());

private:

	// Friends
	
	friend class GpProfileBar;

	
	// Structure Definitions
	
	// Aggregated member-attributes for TOC usage reduction.
	
	struct Instance
	{
		bool			m_isInitialised;
		
		float			m_numVSyncsPerFrame;		///< No. V-Syncs in a 'game' frame
		float			m_refreshRate;				///< Display refresh rate
		
		float			m_drawOriginX;			   	///< Draw orign (in NDC)
		float			m_drawOriginY;
		float			m_drawWidth;				///< Draw width (in NDC)
		float			m_transparency;				///< [0..1] => [fully transparent..opaque]
		
		GpProfileBar	m_profileBars[GpProfiler::kNumUnits];
	
		bool			m_updateStaticPrims;		///< True if static marker primitives need updating
		
		GpProfilerDrawHelpers	m_staticPrims;		///< Static primitives - compute once, draw many frames
		
		float			m_vsyncMarkerWidth;
		float			m_vsyncMarkerHeight;
	
		u64				m_frameStartTick;			///< Timebase tick at the start of the current frame
		
		Instance() : m_isInitialised(false) {}
	};
	

	// Attributes
	
	static Instance		ms_instance;				///< Aggregated instance attributes
	

	// Operations
	
	static void		SetDisplayDependentConstants(float displayWidth, float displayHeight);
	
	static float	GetNumVSyncsPerFrame()		{ return ms_instance.m_numVSyncsPerFrame; }
	static float	GetRefreshRate()			{ return ms_instance.m_refreshRate; }
	
	static void		RequestStaticPrimsUpdate();
	
	static void		DrawStaticMarkers(float x, float y, float alpha);
	
	static uint		GetBarIndex(uint barId);
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpProfileBar.inl>

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

#define GP_PROFILE_BAR_MANAGER_RESET(barIds)														\
{																									\
	GpProfileBarManager::Reset(barIds);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_PROFILE_BAR_MANAGER_DRAW(context)														\
{																									\
	GpProfileBarManager::Draw(context);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_SPU_PROFILE_BAR_START_TIMER(barId, ticks)   												\
{																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(barId);							\
	profileBar.StartTimer32Bit(ticks);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_SPU_PROFILE_BAR_STOP_TIMER(barId, ticks)													\
{																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(barId);							\
	profileBar.StopTimer32Bit(ticks);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_SPU_PROFILE_BAR_ADD_TIMER_MARKER(barId, ticks)  											\
{																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(barId);							\
	profileBar.AddTimerMarker32Bit(ticks);															\
}

//--------------------------------------------------------------------------------------------------

#define GP_SPU_PROFILE_BAR_ADD_TIMER_MARKER_COLOUR(barId, ticks, colour)							\
{																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(barId);							\
	profileBar.AddTimerMarker32Bit(ticks, colour);													\
}

//--------------------------------------------------------------------------------------------------

#define GP_PPU_PROFILE_BAR_START_TIMER()															\
{																									\
	u64		timebase;																				\
	SYS_TIMEBASE_GET(timebase);																		\
																									\
	GpProfileBarManager::SetFrameStartTick(timebase);												\
																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(GpProfiler::kPpuId);			\
	profileBar.StartTimer(timebase);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_PPU_PROFILE_BAR_STOP_TIMER()																\
{																									\
	u64		timebase;																				\
	SYS_TIMEBASE_GET(timebase);																		\
																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(GpProfiler::kPpuId);			\
	profileBar.StopTimer(timebase);																	\
}

//--------------------------------------------------------------------------------------------------

#define GP_PPU_PROFILE_BAR_ADD_TIMER_MARKER()														\
{																									\
	u64		timebase;																				\
	SYS_TIMEBASE_GET(timebase);																		\
																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(GpProfiler::kPpuId);			\
	profileBar.AddTimerMarker(timebase);															\
}

//--------------------------------------------------------------------------------------------------

#define GP_PPU_PROFILE_BAR_ADD_TIMER_MARKER_COLOUR(colour) 											\
{																									\
	u64		timebase;																				\
	SYS_TIMEBASE_GET(timebase);																		\
																									\
	GpProfileBar&	profileBar = GpProfileBarManager::GetProfileBar(GpProfiler::kPpuId);			\
	profileBar.AddTimerMarker(timebase, colour);													\
}

//--------------------------------------------------------------------------------------------------
	
#define GP_RSX_PROFILE_BAR_UPDATE()																	\
{																									\
	GpProfileBarManager::UpdateRsxProfileBar();														\
}

//--------------------------------------------------------------------------------------------------

#else // ATG_PROFILE_ENABLED

#define GP_PROFILE_BAR_MANAGER_RESET(barIds)								{}
#define GP_PROFILE_BAR_MANAGER_DRAW(context)								{}

#define GP_SPU_PROFILE_BAR_START_TIMER(barId, ticks)						{}
#define GP_SPU_PROFILE_BAR_STOP_TIMER(barId, ticks)							{}
#define GP_SPU_PROFILE_BAR_ADD_TIMER_MARKER(barId, ticks)					{}
#define GP_SPU_PROFILE_BAR_ADD_TIMER_MARKER_COLOUR(barId, ticks, colour)	{}

#define GP_PPU_PROFILE_BAR_START_TIMER()									{}
#define GP_PPU_PROFILE_BAR_STOP_TIMER()										{}
#define GP_PPU_PROFILE_BAR_ADD_TIMER_MARKER()								{}
#define GP_PPU_PROFILE_BAR_ADD_TIMER_MARKER_COLOUR(colour)					{}

#define GP_RSX_PROFILE_BAR_UPDATE()											{}

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_PROFILE_BAR_H
