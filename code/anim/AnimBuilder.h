//------------------------------------------------------------------------------------------
//!
//!	\file AnimBuilder.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ANIMBUILDER_H_
#define	ANIMBUILDER_H_

// Forward declarations
class	CHierarchy;
class	CAnimationHeader;
class	CAnimationTransform;

//------------------------------------------------------------------------------------------
//!
//!	AnimBuilder
//!	Contains some functionality for building up animations from a hierarchy
//!
//------------------------------------------------------------------------------------------
class AnimBuilder
{
public:

	// Create an animation from a hierarchy - uses the pose for keyframes
	static CAnimationHeader* CreateAnimation( const CHierarchy& obHierarchy );

	// Destroy what we have made - normally the data we have constructed sits in memory as a loaded resource
	static void DestroyAnimation( CAnimationHeader* pobAnimationHeader );

private:

	// Helper stuff to build the channel data
	static void CreateAnimKeyframes(	const CHierarchy&		obHierarchy, 
										short*					psAnimationToHierarchyLookup,
										CAnimationTransform*	pobAnimationTransformArray,
										int						iNumberOfAnimationTransforms,
										const CAnimationHeader *pobAnimHeader );

	// How long are we going to make our animations?
	static const float s_fStaticAnimDuration;
};

#endif	// !ANIMBUILDER_H_


