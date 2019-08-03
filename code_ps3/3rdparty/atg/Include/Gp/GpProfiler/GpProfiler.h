//--------------------------------------------------------------------------------------------------
/**
	@file		GpProfiler.h

	@brief		Common GpProfiler contants, enums, and helpers.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_PROFILE_H
#define GP_PROFILE_H

//--------------------------------------------------------------------------------------------------

// Available only in profile-enabled builds!

#ifdef ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
//  NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@namespace	GpProfiler

	@brief	Common GpProfiler contants, enums, and helpers.
**/
//--------------------------------------------------------------------------------------------------

namespace GpProfiler
{
	// Hardware unit identifiers.
	
	enum UnitId
	{
		kPpuId	= 0x0001,
		
		kRsxId	= 0x0002,
		
		kSpu0Id	= 0x0004,
		kSpu1Id	= 0x0008,
		kSpu2Id	= 0x0010,
		kSpu3Id	= 0x0020,
		kSpu4Id	= 0x0040,
		kSpu5Id	= 0x0080,
			
		kAllUnitIds = kPpuId | kRsxId | kSpu0Id | kSpu1Id | kSpu2Id | kSpu3Id | kSpu4Id | kSpu5Id
	};
	
	
	// Hardware unit indices.
	
	enum UnitIndex
	{
		kPpuIndex, kRsxIndex, kSpu0Index, kSpu1Index, kSpu2Index, kSpu3Index, kSpu4Index, kSpu5Index
	};

	
	// No. of hardware units.
	
	static const uint	kNumUnits = 8;


	// Helper Functions
	
	UnitId		GetSpuId(uint spuIndex);
	
}
// namespace GpProfiler 

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline GpProfiler::UnitId GpProfiler::GetSpuId(uint spuIndex)
{
	FW_ASSERT(spuIndex < 6);
	
	return (GpProfiler::UnitId) (0x1u << (kSpu0Index + spuIndex));
}

//--------------------------------------------------------------------------------------------------

#endif // ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------

#endif // GP_PROFILER_H
