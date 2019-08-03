/***************************************************************************************************
*
*	$Header:: /game/keyframetypes.h 9     16/05/03 15:23 Dean                                      $
*
*	Keyframe types used in the animation system
*
*	CHANGES
*
*	4/4/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	_KEYFRAMETYPES_H
#define	_KEYFRAMETYPES_H

#include "core/exportstruct_keyframe.h"

/***************************************************************************************************
*
*	CLASS			CKeyframeGenerator
*
*	DESCRIPTION		This static class is used to arbitrate access to keyframe generation functions
*					that are referenced using the enumerated keyframe type. The actual defintion 
*					of the function table is within keyframetypes.cpp.
*
***************************************************************************************************/

class	CKeyframeGenerator
{
public:
	static CVector	Generate( KEYFRAME_TYPE eKeyframeType, const void* pvKeyframeArray, int iIndex, float fBlendFactor )
	{
		ntAssert( eKeyframeType < NUMBER_OF_KEYFRAME_TYPES );
		KEYFRAME_GENERATE_FN	pfnGenerator = m_gapfnKeyframeGeneration[ eKeyframeType ];
		ntAssert( pfnGenerator );	
		return (pfnGenerator)( pvKeyframeArray, iIndex, fBlendFactor );
	};

private:
	typedef	CVector	(*KEYFRAME_GENERATE_FN)( const void* pvKeyframeArray, int iIndex, float fBlendFactor );
	static KEYFRAME_GENERATE_FN	m_gapfnKeyframeGeneration[ NUMBER_OF_KEYFRAME_TYPES ];
};


/***************************************************************************************************
*	
*	CLASS			CKeyframe<type>
*
*	DESCRIPTION		This class defines the actual keyframe structure for an animation channel. At
*					the moment it looks like the pretty standard rotation & translation pairs, but
*					we should be able to expand this to handle curve based animations. Note that I'm
*					not saying that we have enough CPU to handle curves.. that's different.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CKeyframeStandardRotation	:	public	CKeyframe_STANDARD_ROTATION
{
public:
	static	CVector	Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor );
};

ALIGNTO_PREFIX(16) class CKeyframeStandardTranslation	:	public CKeyframe_STANDARD_TRANSLATION
{
public:
	static	CVector	Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor );
};

ALIGNTO_PREFIX(16) class CKeyframeCurveRotation	:	public CKeyframe_CURVE
{
public:
	static	CVector	Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor );
};

ALIGNTO_PREFIX(16) class CKeyframeCurveTranslation	:	public CKeyframe_CURVE
{
public:
	static	CVector	Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor );
};


#endif	//_KEYFRAMETYPES_H
