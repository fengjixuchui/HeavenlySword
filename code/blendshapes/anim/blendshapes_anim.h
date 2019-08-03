//--------------------------------------------------
//!
//!	\file blendshapes_anim.h
//!	blendshape animation instance players/containers
//!
//--------------------------------------------------

#ifndef _BLENDSHAPES_ANIM_H_
#define _BLENDSHAPES_ANIM_H_

#include "blendshapes/anim/blendshapes_anim_export.h"
#include "blendshapes/blendshapes_managers.h"

class BSAnimation;
//typedef	CSharedPtr< BSAnimation > BSAnimation*;
typedef BSAnimation* BSAnimationPtr_t;

//--------------------------------------------------
//!
//!	BlendShape animation instance
//!	This class provides animation instance functionality
//! Each anim instance is unique and holds playback data
//! such as speed, current time, etc.  Note that, on deletion, 
//! it does not release the BSAnimExport pointer.
//! It's up to the manager to load/delete clip data
//!
//--------------------------------------------------
class BSAnimation : CNonCopyable
{
public:
	//! el-cheapo static constructor. (yes, with twooooo allocations. But at least I can change it afterwards...) 
	//! \param pBSAnimExp The bsanim header
	//! \param bSyncFirstUpdate Delay update for the 1st frame in order to keep in sync the the standard animation system
	static BSAnimation* Create( BSAnimHeaderPtr_t pBSAnimExp, bool bSyncFirstUpdate = true );

	BSAnimation( BSAnimHeaderPtr_t pBSAnimExp,  bool bSyncFirstUpdate = true );

	//! standard dtor
	~BSAnimation();

	//! gets the animation header name
	uint32_t			GetNameHash( void ) const;
	//! name of the target blendshape set
	uint32_t			GetBlendShapesNameHash( void ) const;
	//! gets the target clump this was intended for
	uint32_t			GetClumpNameHash( void ) const;
	//! gets the target hierarchy key this was intended for
	uint32_t			GetHierarchyKey( void ) const;


	int					GetFlags( void ) const	{ return m_iFlags; }
	void				SetFlags( int iFlags )	{ m_iFlags = iFlags; }
	void				SetFlagBits( int iFlagBits ) { m_iFlags |= iFlagBits; }
	void				ClearFlagBits( int iFlagBits )	{ m_iFlags &= ~iFlagBits; }


	//! number of channels in this anim
	u_int				GetNumOfChannels( void ) const;
	//! anim index to target index conversion. The result is context dependent
	int					GetChannelTargetIndex( u_int channelIndex ) const;
	//! get the channel type
	BSAnimChannelType	GetChannelType( u_int channelIndex ) const;


	//! get total duration for this anim (all channels have the same duration)
	float				GetDuration( void ) const;
	//! current time
	float				GetTime( void ) const;
	//! current playing speed
	float				GetSpeed( void ) const;
	//! the overall anim blend factor. Not to be confused the GetTargetWeightXXX()
	float				GetBlendFactor( void ) const;
	//! set the animation blend factor for the whole anim. Clamped to [0.0, 1.0]
	void				SetBlendFactor( float w ); 
	//! set playback speed
	void				SetSpeed( float speed );


	//! resets time to zero, walkers positions, and resulting weights
	void				Reset();
	//! update time and blend keyframes. Resulting weights are cached for later retrieval 
	void				Update( float timeStep );


	//! target weights accessors
	float				GetTargetWeightByIndex( u_int index ) const;
	const float*		GetTargetWeights() const;
	

	//! this instance should be destroyed after update. It's up to the animator to do this
	bool				IsFinished() const;

private:

	//! disallow direct construction
//	BSAnimation( BSAnimHeaderPtr_t pBSAnimExp,  bool bSyncFirstUpdate );

	
	struct BSChannelWalker
	{
		u_int m_keyframeIndex;
		const BSAnimChannelExport* pChannel;
		float Update( float time );

		BSChannelWalker();
	};

	const BSAnimChannelExport* GetChannel( u_int index ) const;

	//u_int TargetIDtoIndex( u_int id ) const;

private:
	//! the actual anim data header
	BSAnimHeaderPtr_t				m_pAnimExp;	
	//! used for evaluating keyframes over each channel
	CScopedArray<BSChannelWalker>	m_aChannelWalkers;
	//! resulting weight cache
	CScopedArray<float>				m_aWeights;	
	//! current time
	float							m_time;		
	//! overall animation blend factor
	float							m_blendFactor;
	//! playback speed
	float							m_speed;
	//! this anim is done playing (further updated will yield the same resulting keyframe) so delete it!
	bool							m_bFinishedPlaying;	
	//! instance specific flags
	int								m_iFlags;

	// HACK: bsanims should be delayed one frame in order to keep in sync with the
	// normal anims so...
	bool							m_bFirstUpdate;
};

//-----------------------------------------------------------------
//						BSAnimation INLINED METHODS
//-----------------------------------------------------------------

inline uint32_t BSAnimation::GetNameHash( void ) const
{
	return m_pAnimExp->m_nameHash;
}

inline uint32_t BSAnimation::GetBlendShapesNameHash( void ) const 
{ 
	return m_pAnimExp->m_blendShapesNameHash;
}

inline uint32_t BSAnimation::GetClumpNameHash( void ) const
{
	return m_pAnimExp->m_clumpNameHash;
}

inline uint32_t	BSAnimation::GetHierarchyKey( void ) const
{
	return m_pAnimExp->m_uiHierarchyKey;
}

inline float BSAnimation::GetDuration( void ) const
{
	return m_pAnimExp->m_duration;
}

inline float BSAnimation::GetTime( void ) const
{
	return m_time;
}

inline u_int BSAnimation::GetNumOfChannels( void ) const 
{
	return m_pAnimExp->m_numOfChannels;
}

inline float BSAnimation::GetSpeed( void ) const 
{
	return m_speed;
}

inline float BSAnimation::GetBlendFactor( void ) const
{
	return m_blendFactor;
}

inline void BSAnimation::SetBlendFactor( float w ) 
{
	m_blendFactor = ntstd::Clamp( w, 0.0f, 1.0f );
}

inline void BSAnimation::SetSpeed( float speed )
{
	m_speed = speed; 
}

inline float BSAnimation::GetTargetWeightByIndex( u_int index ) const
{
	ntAssert( index < GetNumOfChannels() );
	return m_aWeights[index];
}

//inline float BSAnimation::GetTargetWeightByTargetID( u_int id ) const DEPRECATED
//{
//	ntAssert( id < kBST_TotalNumOfTargets );
//	const int index = TargetIDtoIndex( id );
//	return ( index >= 0 ) ? m_aWeights[ index ] : 0.0f;			//! TODO_OZZ: return -1 if not found? ignore vs. error
//}

inline const float* BSAnimation::GetTargetWeights() const
{
	return m_aWeights.Get();
}

inline bool BSAnimation::IsFinished() const
{
	return m_bFinishedPlaying; 
}

inline const BSAnimChannelExport* BSAnimation::GetChannel( uint32_t index ) const
{
	ntAssert( index < GetNumOfChannels() );
	return m_pAnimExp->m_pChannels + index;
}

//inline u_int BSAnimation::TargetIDtoIndex( u_int id ) const DEPRECATED
//{
//	ntAssert( id < kBST_TotalNumOfTargets );
//	return m_aIDtoIndexMap[ id ];
//}

inline int BSAnimation::GetChannelTargetIndex( u_int channelIndex ) const
{
	ntAssert( channelIndex < GetNumOfChannels() );
	return m_pAnimExp->m_pChannels[ channelIndex ].m_targetIndex;
}

inline BSAnimChannelType BSAnimation::GetChannelType( u_int channelIndex ) const
{
	ntAssert( channelIndex < GetNumOfChannels() );
	return m_pAnimExp->m_pChannels[ channelIndex ].m_type; 
}


#endif // end of _BLENDSHAPES_ANIM_H_
