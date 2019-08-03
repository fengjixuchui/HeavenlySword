//--------------------------------------------------------------------------------------------------
/**
	@file		GpRsxProfiler.h

	@brief		Helper class for profiling RSX push buffer exection.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_RSX_PROFILER_H
#define GP_RSX_PROFILER_H

//--------------------------------------------------------------------------------------------------

// Available only in profile-enabled builds!

#ifdef ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcKernel.h>
#include <Gc/GcColour.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpRsxProfiler

	@brief	Helper class for profiling RSX push buffer exection.
**/
//--------------------------------------------------------------------------------------------------

class GpRsxProfiler : public FwNonCopyable
{
public:

	// Timer Marker Class Definition

	class Marker
	{
	public:
		
		// Construction
		
		Marker() {}
		
		
		// Accessors
		
		const char*		GetName() const;
		GcColour		GetColour() const;
		
		u64				GetTimestamp() const;		// Get timestamp (in nanoseconds)
		u64				GetTime() const;			// Get time taken (in nanoseconds)
		
	private:
	
		// Friend
		
		friend class	GpRsxProfiler;
		 
		
		// Attributes
		
		const char*				m_pName; 			///< Marker name (useful for visualisation)
		GcColour				m_colour;			///< Marker colour (also useful for visualisation)
		
		u64						m_timestamp;		///< Report timestamp i.e. marker start time
		u64						m_time;				///< Marker time duration (in nanoseconds)
		
		// Construction
		
		Marker(const char* pName, GcColour_arg colour);
	};
	
	
	// Constants
	
	static const uint	kDefaultMaxTimerMarkers	= 32;
	
	
	// Initialisation and Shutdown Functions
	
	static uint		QueryHostMemorySizeInBytes(uint maxTimerMarkers = kDefaultMaxTimerMarkers);
	
	static void		Initialise(uint maxTimerMarkers = kDefaultMaxTimerMarkers, void* pHostMemory = NULL);
	
	static void		Shutdown();
	
	
	// Accessors
	
	static u32				GetNumReadableTimerMarkers();
    
	static const Marker*	GetReadableTimerMarker(uint index);
	
	static u64				GetTotalTime();
	
	
	// Macro Interface Implementation Functions
	//
	// :NOTE: Do NOT call these functions directly - use the macro interface provided below instead.
	
	static void		StartTimer(GcContext& context);
	
	static void		StopTimer(GcContext& context);
	
	static void		AddTimerMarker(GcContext& context, const char* pName, GcColour_arg colour);


private:
    
	// Prevent Construction and Destruction
	
	GpRsxProfiler();
	~GpRsxProfiler();


	// Constants
	
	static const char*	kStartTimerMarkerName;
	static const char*	kStopTimerMarkerName;

    
	// Structure Definitions
	
	// Aggregated member-attributes for TOC usage reduction.
	
	struct Instance
	{
		bool					m_isInitialised;			///< Is initialised flag
        
		uint					m_maxMarkers;				///< Maximum no. of timer markers

		uint					m_addMarkersBufferIdx;		///< Buffer index of timer markers added this frame
		uint					m_readMarkersBufferIdx;		///< Buffer index of timer markers that can be read this frame
		uint					m_numMarkers[3];			///< No. of timer markers added to the push buffer
		Marker*					m_pMarkers[3];				///< Timer markers added to the push buffer
		
		uint					m_ppuAccessReportsBufferIdx;///< Buffer index of the PPU accessible reports
		void*					m_pReportHostMemory;		///< Report host memory storage - NULL if user-specified
		Ice::Render::Report*	m_pReports[2];	   			///< Double buffered reports
		u32						m_reportsOffset[2];			///< Double buffered reports (as host memory offsets)
	
		Instance() : m_isInitialised(false) {}
	};
	

	// Attributes
	
	static Instance				ms_instance;				///< Aggregated instance attributes
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpRsxProfiler.inl>

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

#define GP_RSX_PROFILER_START_TIMER(context)														\
{																									\
	GpRsxProfiler::StartTimer(context);													    		\
}

//--------------------------------------------------------------------------------------------------

#define GP_RSX_PROFILER_STOP_TIMER(context)															\
{																									\
	GpRsxProfiler::StopTimer(context);																\
}

//--------------------------------------------------------------------------------------------------

#define GP_RSX_PROFILER_ADD_TIMER_MARKER(context, pName, colour)									\
{																									\
	GpRsxProfiler::AddTimerMarker(context, pName, colour);											\
}

//--------------------------------------------------------------------------------------------------

#else // ATG_PROFILE_ENABLED

#define GP_RSX_PROFILER_START_TIMER(context)							{}
#define GP_RSX_PROFILER_STOP_TIMER(context)								{}
#define GP_RSX_PROFILER_ADD_TIMER_MARKER(context, pName, colour)		{}

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_RSX_PROFILER_H
