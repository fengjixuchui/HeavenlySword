//------------------------------------------------------------------------------------------
//!
//!	\file animation.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ANIM_ANIMATION_H
#define	ANIM_ANIMATION_H

#include "Gp/GpAnimator/GpAnimator.h"
#include "anim/AnimationHeader.h"

// Forward declarations
class	CAnimation;
class	CHierarchy;
class	CAnimator;
class	Transform;
class	AnimShortCut;

template < typename > class IntrusiveRefCountPtr;
typedef IntrusiveRefCountPtr< CAnimation > CAnimationPtr;

//------------------------------------------------------------------------------------------
//!
//!	CAnimation
//!	This object manages the playback of a single animation. The object is controlled
//!	by a CAnimator object (see animator.cpp). As with CAnimator, this object is
//!	is created through a static construction function, which allows us to reduce
//!	memory allocation overheads by combining hierarchy-specific allocations into
//!	the same object as the CAnimator itself. 
//!
//------------------------------------------------------------------------------------------
ALIGNTO_PREFIX( 128 ) class CAnimation
{
	public:
		//
		//	Construction from core data
		//
		static CAnimationPtr	Create(	const CAnimator *		animator,
										const uint32_t			uiShortNameHash,
										const CAnimationHeader*	pAnimHeader,
										const char*				pDebugAnimName );

		static CAnimationPtr	Create(	const CAnimator *		animator,
										const uint32_t			uiShortNameHash,
										const AnimShortCut *	pAnimShortcut,
										const char*				pDebugAnimName );

	public:
		//
		//	Ref-counting functionality.
		//
				// Decrement the ref-count and, if zero, delete the animation.
		void	Release			()					const;

				// Increment the reference count by one.
		void	AddRef			()					const		{ ++m_RefCount; }

		int32_t	GetRefCount		()					const		{ return m_RefCount; }

	public:
		//
		//	CAnimation functionality.
		//

		// Set up the flags that determine this animations behaviour
		int32_t	GetFlags		()						const	{ return m_iFlags; }
		void	SetFlags		( int32_t iFlags )				{ m_iFlags = iFlags; }
		void	SetFlagBits		( int32_t iFlagBits )			{ m_iFlags |= iFlagBits; }
		void	ClearFlagBits	( int32_t iFlagBits )			{ m_iFlags &= ~iFlagBits; }

		// Is this animation currently being used by an animator?
		bool	IsActive		()					const		{ return m_IsActive; }
		void	SetActive		( bool a )						{ m_IsActive = a; }

		// Get or set the current time (position) of the animation - currently set is slow because of no binary seek
		float	GetTime			() const						{ return m_fTime; }
		void	SetTime			( float fTime );
		void	SetPercentage	( float fPercentage );

		// What is the unscaled duration of the raw animation?
		float	GetDuration		()					const;

		// Find the current phase of this animation - NOT NECESSARILY TIME / DURATION
		float	GetPhasePosition()					const;

		// What is the total root movement that the raw animation would provide over its total duration - this
		// is the delta from the start of the animations - not the absolute value!
		CQuat	GetRootEndRotation		()			const;
		CPoint	GetRootEndTranslation	()			const;

		// How much tranlation did this animation supply in the last frame?
		CPoint	GetRootTranslationDelta	()			const;

		// Access to the speed at which the animation will play - 1.0f is the normal speed
		float	GetSpeed				()			const		{ return m_fSpeed; }
		void	SetSpeed				( float s )				{ m_fSpeed = s; }
		void	SetTimeRemaining		( float fTimeRemaining );								

		// Retrieve blend weight (not necessarily the weight used!)
		float	GetBlendWeight			()			const		{ return m_fBlendWeight; }

		// Set target blend weight - the clamp makes the system robust at the base level - GH
		void	SetBlendWeight			( float fBlendWeight );

		// Get the hash of the animation name - allows anim identification
		uint32_t	GetShortNameHash	()			const		{ return m_uiShortNameHash; }

		// Returns the animation header.. use with caution, please.  This is only used for debug purposes
		// at the moment to identify a running animation by searching for a matching header pointer in
		// the animation container of the parent enity?  Bonkers.
		const CAnimationHeader *GetAnimationHeader() const		{ return m_AnimationData->GetAnimClipDef(); }

		// Access to the priority of the animation - can only be set before adding to an animator
		int32_t	GetPriority				()			const		{ return m_Priority; }
		void	SetPriority				( int32_t p )			{ m_Priority = p; }

		// Get details of the root node animation at any time
		CPoint	GetRootTranslationAtTime( float fTime, const CHierarchy *hierarchy, bool bIsRelative = false );	
		CQuat	GetRootRotationAtTime	( float fTime, const CHierarchy *hierarchy );

		// ATG GpAnimation access - DO NOT USE WITHOUT ASKING ANDREW!
		GpAnimation *		GetGpAnimation	()					{ return m_AnimationData; }
		const GpAnimation *	GetGpAnimation	()		const		{ return m_AnimationData; }




		void TestFunc( CHierarchy *hierarchy );




	private:
		//
		//	Prevent copying and assignment.
		//
		CAnimation( const CAnimation & )				NOT_IMPLEMENTED;
		CAnimation &operator = ( const CAnimation & )	NOT_IMPLEMENTED;

		// Prevent ctor, dtor calls. must use Release instead
		CAnimation() : m_RefCount( 1 ) {};	
		~CAnimation();

	private:
		// One of these can look in here
		friend class CAnimator;

		// Speculative relative animation stuff - GILES
		CPoint	GetRootWorldPosition	( const Transform *root )	const;
		CQuat	GetRootWorldOrientation	( const Transform *root )	const;

		// Last update and phase link values are not available on the first update frame
		void				SetInitialUpdate			()							{ m_bInitialUpdate = true; }	

		// The update for all animations
		void				Update						();

		// This needs to be called on non phase linked animations before the main update call
		void				UpdateTime					( float fTimeStep );
		
		// These make up the equivelent of 'UpdateTime' for phase linked animations
		void				InitialLinkageUpdate		(	float fTimeStep, float &fLinkedWeightSum, float &fLinkedPositionSum,
															float &fLinkedSpeedSum, float &fLinkedRemainingWeight );
		void				ApplyLinkageChange			(	float fNewPosition );

		// Calculate the root translation delta acheived over the last frame
		void				CalcLocomotion				();

		bool				ShouldDelete				()				const		{ return m_ToBeDeleted; }

	private:
		CPoint				m_RootTranslationDelta;		// A modified version of the deltas calculated by GpAnimation, taking
		CQuat				m_RootRotationDelta;		// into account any phase-linking and/or looping fix-ups we need to do.

	public:	// This is public because Giles is lazy :)
		// Speculative relative animation stuff - GILES
		const Transform *	m_pobRelativeTransform;		// Pointer to the underlying animation transform on PC and PPU.

	private:
		int32_t				m_iFlags;					// Flags (Weight manually controlled, inhibit-auto-destruct etc)
		int32_t				m_Priority;					// How far down the hierarchy does this animation start? Precomputed on CAnimation creation and cached.
		uint32_t			m_uiShortNameHash;			// Hash ID of the short name for this anim - debug use only
		int32_t				m_LoopCount;				// How many times has this animation looped?

		float				m_fTime;					// Current time within the animation
		float				m_fPreviousTime;			// Previous time (saved at last call to CAnimation::Update)
		float				m_fSpeed;					// Animation speed (1.0f means play as authored)

		GpAnimation *		m_AnimationData;			// The ATG GpAnimation object - does all the actual work.
		const AnimShortCut*	m_pAnimShortCut;			// Pointer to creating def, used to fixup anim events later

		float				m_fBlendWeight;				// Target/requested blend weight
		mutable int32_t		m_RefCount;					// Our ref-count.

		GpAnimation::LocomotionKey	m_LocomotionKey;	// Key to represent locomotion data with the GpAnimation object.

		bool				m_bInitialUpdate;			// Is this animation on its first update?
		bool				m_IsActive;					// Is this animation active at the moment (i.e. in-use by an animator).
		bool				m_ToBeDeleted;				// Should the animator object delete this animation?
		bool				m_AddedToAnimatorThisFrame;	// Set to 'true' if this animation was added to an animator in the current frame.
}
ALIGNTO_POSTFIX( 128 );

//************************************************************************************************************
//************************************************************************************************************
//	Inline implementations.
//************************************************************************************************************
//************************************************************************************************************

//************************************************************************************************************
//	
//************************************************************************************************************
inline CPoint CAnimation::GetRootTranslationDelta() const
{
	return	m_RootTranslationDelta;
}

//************************************************************************************************************
//	
//************************************************************************************************************
inline CQuat CAnimation::GetRootEndRotation() const
{
	if ( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() == NULL )
	{
		return CQuat( CONSTRUCT_IDENTITY );
	}

	return	CQuat( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetEndDeltaRotation().QuadwordValue() );
}

//************************************************************************************************************
//	
//************************************************************************************************************
inline CPoint CAnimation::GetRootEndTranslation() const
{
	if ( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() == NULL )
	{
		return CPoint( CONSTRUCT_CLEAR );
	}

	return	CPoint( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetEndDeltaTranslation().QuadwordValue() );
}

//************************************************************************************************************
//	
//************************************************************************************************************
inline float CAnimation::GetDuration() const
{
	return m_AnimationData->GetDuration();
}

//************************************************************************************************************
// Set target blend weight - the clamp makes the system robust at the base level - GH
//************************************************************************************************************
inline void CAnimation::SetBlendWeight( float fBlendWeight )
{
	m_fBlendWeight = ntstd::Clamp( fBlendWeight, 0.0f, 1.0f );
	if ( m_fBlendWeight < EPSILON )
	{
		m_fBlendWeight = 0.0f;
	}
}

#ifdef PLATFORM_PC
#define ERROR_HIERARCHY_KEY		0x36AFD6D8
#else
#define ERROR_HIERARCHY_KEY		0x25058D9C
#endif

#endif	//ANIM_ANIMATION_H


