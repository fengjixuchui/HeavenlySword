/***************************************************************************************************
*
*	$Header:: /game/keyframetypes.cpp 11    16/05/03 15:23 Dean                                    $
*
*	Keyframe Types
*
*	CHANGES
*
*	4/4/2003	Dean	Created
*
***************************************************************************************************/

#include "anim/keyframetypes.h"

CKeyframeGenerator::KEYFRAME_GENERATE_FN	CKeyframeGenerator::m_gapfnKeyframeGeneration[ NUMBER_OF_KEYFRAME_TYPES ] = 
{
	CKeyframeStandardRotation::Generate,				// KEYFRAME_TYPE_STANDARD_ROTATION
	CKeyframeStandardTranslation::Generate,				// KEYFRAME_TYPE_STANDARD_TRANSLATION
	CKeyframeCurveRotation::Generate,					// KEYFRAME_TYPE_CURVE_ROTATION
	CKeyframeCurveTranslation::Generate,				// KEYFRAME_TYPE_CURVE_ROTATION
	NULL,												// KEYFRAME_TYPE_MESSAGE
};


/***************************************************************************************************
*
*	FUNCTION		CKeyframeStandardRotation::Generate
*
*	DESCRIPTION		Standard keyframe evaluation for slerp rotations
*
*	INPUTS			pvKeyframeArray		-	Pointer to an array of CKeyframe_STANDARD_ROTATION objects
*
*					iIndex				-	Index into array for keyframe
*
*					fBlendFactor		-	Amount of blend
*
*	RESULT			A CVector holding the result of keyframe evaluation.
*
***************************************************************************************************/

CVector	CKeyframeStandardRotation::Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor )
{
	// Get a pointer to the keyframe array
	const CKeyframeStandardRotation*	pobKeyframe = &( ( const CKeyframeStandardRotation* )pvKeyframeArray )[ iIndex ];

	// If we're completely weighted to the left index, then return it directly. 
	if ( fBlendFactor == 0.0f )
		return CVector( pobKeyframe->m_obFromRotation );

	// Generate a new keyframe, returning it by value. We could probably do with an inline version of this later..
	return CVector( CQuat::Slerp( pobKeyframe->m_obFromRotation, pobKeyframe->m_obToRotation, fBlendFactor ) );
}


/***************************************************************************************************
*
*	FUNCTION		CKeyframeStandardTranslation::Generate
*
*	DESCRIPTION		Standard keyframe evaluation for linear translations
*
*	INPUTS			pvKeyframeArray		-	Pointer to an array of CKeyframe_STANDARD_TRANSLATION objects
*
*					iIndex				-	Index into array for keyframe
*
*					fBlendFactor		-	Amount of blend
*
*	RESULT			A CVector holding the result of keyframe evaluation.
*
***************************************************************************************************/

CVector	CKeyframeStandardTranslation::Generate( const void* pvKeyframeArray, int iIndex,  float fBlendFactor )
{
	// Get a pointer to the keyframe array
	const CKeyframeStandardTranslation*	pobKeyframe = &( ( const CKeyframeStandardTranslation* )pvKeyframeArray )[ iIndex ];

	// If we're completely weighted to the left index, then return it directly. 
	if ( fBlendFactor == 0.0f )
		return CVector( pobKeyframe->m_obFromTranslation );

	// Generate a new keyframe, returning it by value
	return CVector( CPoint::Lerp( pobKeyframe->m_obFromTranslation, pobKeyframe->m_obToTranslation, fBlendFactor ) );
}



/***************************************************************************************************
*
*	FUNCTION		CKeyframeCurveRotation::Generate
*
*	DESCRIPTION		Curve keyframe evaluation for rotations
*
*	INPUTS			pvKeyframeArray		-	Pointer to an array of CKeyframe_CURVE objects
*
*					iIndex				-	Index into array for keyframe
*
*					fBlendFactor		-	Amount of blend
*
*	RESULT			A CVector holding the result of keyframe evaluation.
*
***************************************************************************************************/

CVector	CKeyframeCurveRotation::Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor )
{
	// Get a pointer to the keyframe array
	const CKeyframeCurveRotation*	pobKeyframe = &( ( const CKeyframeCurveRotation* )pvKeyframeArray )[ iIndex ];

	ntAssert(fabsf(pobKeyframe->m_obA.X())<100.0f);
	ntAssert(fabsf(pobKeyframe->m_obA.Y())<100.0f);
	ntAssert(fabsf(pobKeyframe->m_obA.Z())<100.0f);

	// If we're completely weighted to the left index, then return it directly. 
	if ( fBlendFactor == 0.0f )
		return CVector( pobKeyframe->m_obA );

	// Evaluate it..
	CVector	obBlendFactor( fBlendFactor );
	CQuat obResult( ( ( ( ( ( pobKeyframe->m_obD * obBlendFactor ) + pobKeyframe->m_obC ) * obBlendFactor ) + pobKeyframe->m_obB ) * obBlendFactor ) + pobKeyframe->m_obA );
	ntAssert(fabsf(obResult.X())<100.0f);
	ntAssert(fabsf(obResult.Y())<100.0f);
	ntAssert(fabsf(obResult.Z())<100.0f);
	obResult.Normalise();
	return CVector( obResult );
}


/***************************************************************************************************
*
*	FUNCTION		CKeyframeCurveTranslation::Generate
*
*	DESCRIPTION		Curve keyframe evaluation for translations
*
*	INPUTS			pvKeyframeArray		-	Pointer to an array of CKeyframe_CURVE objects
*
*					iIndex				-	Index into array for keyframe
*
*					fBlendFactor		-	Amount of blend
*
*	RESULT			A CVector holding the result of keyframe evaluation.
*
***************************************************************************************************/

CVector	CKeyframeCurveTranslation::Generate( const void* pvKeyframeArray, int iIndex, float fBlendFactor )
{
	// Get a pointer to the keyframe array
	const CKeyframeCurveTranslation*	pobKeyframe = &( ( const CKeyframeCurveTranslation* )pvKeyframeArray )[ iIndex ];

	// If we're completely weighted to the left index, then return it directly. 
	if ( fBlendFactor == 0.0f )
		return CVector( pobKeyframe->m_obA );

	// Evaluate it..
	CVector	obBlendFactor( fBlendFactor );
	return	( ( ( ( ( pobKeyframe->m_obD * obBlendFactor ) + pobKeyframe->m_obC ) * obBlendFactor ) + pobKeyframe->m_obB ) * obBlendFactor ) + pobKeyframe->m_obA;
}

