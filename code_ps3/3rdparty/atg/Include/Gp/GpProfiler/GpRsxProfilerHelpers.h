//--------------------------------------------------------------------------------------------------
/**
	@file		GpRsxProfilerHelpers.h

	@brief		RSX profiler helper functions.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_RSX_PROFILER_HELPERS_H
#define GP_RSX_PROFILER_HELPERS_H

//--------------------------------------------------------------------------------------------------

// Available only in profile-enabled builds!

#ifdef ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpRsxProfiler.h>
#include <Gp/GpProfiler/Internal/GpProfilerDrawHelpers.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpRsxProfilerHelpers

	@brief	Helper functions for the RSX profiler.
**/
//--------------------------------------------------------------------------------------------------

class GpRsxProfilerHelpers : public FwNonCopyable
{
public:

	// Enumerations
	
	enum TimeUnits { kTimeInMilliseconds, kTimeInFramePercentage };
	
	
	// Initialisation and Shutdown Functions
	
	static void		Initialise(uint maxTimerMarkers, uint numVSyncsPerFrame = 1, float refreshRate = 60.0f);
	
	static void 	Shutdown();
	
	
	// Helpers
	
	static void		PrintTimerMarkers(float x, float y, TimeUnits timeUnits = kTimeInFramePercentage);

	static void		SetPrintUpdateDelay(uint delay);
	static void		SetRefreshRate(float refreshRate);
   
   
private:

	// Prevent Construction and Destruction
	
	GpRsxProfilerHelpers();
	~GpRsxProfilerHelpers();
	
	
	// Structure Definitions
	
	// Aggregated member-attributes for TOC usage reduction.
	
	struct Instance
	{
		bool					m_isInitialised;
		
		float					m_numVSyncsPerFrame;		///< No. V-Syncs in a 'game' frame
		float					m_refreshRate;				///< Display refresh rate
		
		uint					m_printUpdateDelay;			///< Print update delay (in frames)
		uint					m_printUpdateCounter;		///< Print update counter
		
		uint					m_maxMarkers;				///< Maximum No. of timer markers that can be printed
		uint					m_numMarkers;				///< No. of timer markers to print
		GpRsxProfiler::Marker*	m_pMarkers;					///< Cached timer markers
		
		u64						m_totalTime;				///< Cached total RSX time (in nanoseconds)
		
		GpProfilerDrawHelpers	m_colourKeyPrims;			///< Colour key primitives.
		
		Instance() : m_isInitialised(false) {}
	};

	
	// Attributes
	
	static Instance		ms_instance;						///< Aggregated instance attributes
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------


inline void GpRsxProfilerHelpers::SetPrintUpdateDelay(uint delay)
{
	ms_instance.m_printUpdateDelay = delay;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief
**/
//--------------------------------------------------------------------------------------------------

inline void GpRsxProfilerHelpers::SetRefreshRate(float refreshRate)
{
	ms_instance.m_refreshRate = refreshRate; 
}

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

#define GP_RSX_PROFILER_PRINT_TIMER_MARKERS(x, y, timeUnits)								\
{																							\
	GpRsxProfilerHelpers::PrintTimerMarkers(x, y, timeUnits);								\
}

//--------------------------------------------------------------------------------------------------

#else // ATG_PROFILE_ENABLED

#define GP_RSX_PROFILER_PRINT_TIMER_MARKERS(x, y, timeUnits)								{}

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_RSX_PROFILER_HELPERS_H
