//--------------------------------------------------------------------------------------------------
/**
	@file		GpRsxProfiler.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_RSX_PROFILER_INL
#define GP_RSX_PROFILER_INL

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

inline GpRsxProfiler::Marker::Marker(const char* pName, GcColour_arg colour)
	:
	m_pName(pName),
	m_colour(colour),
	
	m_timestamp(0),
	m_time(0)
{

}
		
//--------------------------------------------------------------------------------------------------

inline const char*	GpRsxProfiler::Marker::GetName() const			{ return m_pName; }
inline GcColour		GpRsxProfiler::Marker::GetColour() const		{ return m_colour; }

inline u64			GpRsxProfiler::Marker::GetTimestamp() const		{ return m_timestamp; }
inline u64			GpRsxProfiler::Marker::GetTime() const			{ return m_time; }

//--------------------------------------------------------------------------------------------------

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_RSX_PROFILER_INL
