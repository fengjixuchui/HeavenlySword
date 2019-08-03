//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MATHS_CONSTANTS_H
#define FW_MATHS_CONSTANTS_H

// Constants (these are in global scope on purpose - they are statically linked - see MSVC docs)

namespace	FwMaths
{
	const	float	kPi					=	3.1415926535897932384626433832795f;
	const	float	kTwoPi				=	6.283185307179586476925286766559f;
	const	float	kHalfPi				=	1.5707963267948966192313216916398f;
	const	float	kQuarterPi			=	0.785398163397448309615660845819876f;
	const	float	kOneOverPi			=	0.31830988618379067153776752674503f;
	const	float	kRootTwo			=	1.4142135623730950488016887242097f;
	const	float	kRootHalf			=	0.70710678118654752440084436210485f;
	const	float	kDegToRad			=	0.01745329f;
	const	float	kRadToDeg			=	57.29578f;
	const	float	kMaxPosFloat		=	3.402823466e+38f;
	const	float	kMinNegFloat		=	-3.402823466e+38f;
	const	float	kLog2				=	0.69314718056f;
	const	float	kLog10				=	2.30258509299f;
};

#endif // FW_MATHS_CONSTANTS_H
