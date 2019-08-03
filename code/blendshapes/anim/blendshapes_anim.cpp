#include "blendshapes/anim/blendshapes_anim.h"
#include "blendshapes/blendshapes_managers.h"
#include "core/user.h"

// to be used with the corresponding flag in finaliser
#define BS_NORMALISED_KEYFRAMES




//-----------------------------------------------------------------
//
//								BSAnimation 
//
//-----------------------------------------------------------------

BSAnimation* BSAnimation::Create( BSAnimHeaderPtr_t pBSAnimExp, bool bSyncFirstUpdate )
{
	return  NT_NEW_CHUNK (Mem::MC_PROCEDURAL) BSAnimation( pBSAnimExp, bSyncFirstUpdate ) ;
}


BSAnimation::BSAnimation( BSAnimHeaderPtr_t pBSAnimExp,  bool bSyncFirstUpdate )
	: m_pAnimExp( pBSAnimExp ),
	m_aChannelWalkers( NT_NEW_CHUNK (Mem::MC_PROCEDURAL)  BSChannelWalker[ pBSAnimExp->m_numOfChannels ] ),
	m_aWeights( NT_NEW_CHUNK (Mem::MC_PROCEDURAL) float[ pBSAnimExp->m_numOfChannels ] ),
	m_time ( 0.0f ),
	m_blendFactor( 1.0f ),
	m_speed( 1.0f ),
	m_bFinishedPlaying( false ),
	m_iFlags( pBSAnimExp->m_flags ),
	m_bFirstUpdate( bSyncFirstUpdate )
{
	//! fill the channel walker ptrs and map arrays
	for( u_int iChannel = 0 ; iChannel < GetNumOfChannels() ; ++iChannel )
	{
		m_aChannelWalkers[ iChannel ].pChannel = GetChannel( iChannel );
	}

	//! this is probably unnecessary as we always update weights before using them
	memset( m_aWeights.Get(), 0, GetNumOfChannels() * sizeof(float) );
}




BSAnimation::~BSAnimation( void )
{
	//nothing to do here anymore
}



void BSAnimation::Reset( void )
{
	m_bFirstUpdate = true;
	m_time = 0.0f;

	for ( u_int iChannel = 0; iChannel < GetNumOfChannels(); iChannel++ )
	{
		m_aChannelWalkers[ iChannel ].m_keyframeIndex = 0;
		m_aWeights[ iChannel ] = 0.0f;
	}

}


void BSAnimation::Update( float timeStep )
{
	// this mimics the standard anims behaviour. It's not really necessary here
	// but it makes it easier to keep both systems in sync. Note that it could be moved
	// to BSAnimator or even offset when exported but I'd rather leave it here to make things clearer
	if ( m_bFirstUpdate )
	{
		m_bFirstUpdate = false;
		return;
	}

	// update time
	m_time += timeStep * GetSpeed();
	const float duration = GetDuration();

	// fit time to a valid range and update deletion status if necessary
	if ( GetFlags() & kBSAF_Looping )
	{
		m_time = fmod( m_time, duration );
	}
	else  if ( m_time < 0.0f || m_time > duration )
	{
		m_time = ntstd::Clamp(m_time, 0.0f, duration );
		m_bFinishedPlaying = !( GetFlags() & kBSAF_NoDelete );
	}

	// update walkers and cache weights
	for ( u_int iChannel = 0; iChannel < GetNumOfChannels(); iChannel++ )
	{
		m_aWeights[ iChannel ] = m_aChannelWalkers[ iChannel ].Update( m_time );
	}
}


//--------------------------------------------------------------------------------------//
//
//						BSAnimation::BSChannelWalker
//
//--------------------------------------------------------------------------------------//

//--------------------------------------------------
//!
//!	Channel update function
//!	Nicked from the standard animation system to deal with single-component
//! channels.
//!	\return the resulting channel target weight ( note: note to be confused with the blend factor )
//!	\param time current time for this instance
//!
//--------------------------------------------------
float BSAnimation::BSChannelWalker::Update( float time )
{
	const float* pTimes = pChannel->m_pKeyframeTimes;

	ntAssert( time <= pTimes[ pChannel->m_numOfKeyframes - 1] && 
		m_keyframeIndex >= 0 &&
		m_keyframeIndex < pChannel->m_numOfKeyframes - 1 
	);

	// this comes from anim/channelwalker.cpp and it reads HACK. Dunno if it's necessary but
	// doesn't seem to hurt so I'll leave it for now
	if ( time > pTimes[ pChannel->m_numOfKeyframes - 1] )
		time = pTimes[ pChannel->m_numOfKeyframes - 1];

    while ( time > pTimes[ m_keyframeIndex + 1 ] )
		++m_keyframeIndex;

	while ( time < pTimes[ m_keyframeIndex ] )
		--m_keyframeIndex;

	const BSKeyframe& hKey = pChannel->m_pKeyframes[ m_keyframeIndex ];

#ifdef BS_NORMALISED_KEYFRAMES
	const float segmentDuration = pTimes[ m_keyframeIndex + 1 ] - pTimes[ m_keyframeIndex ];
	const float blendFactor = ( time - pTimes[ m_keyframeIndex ] ) / segmentDuration;
#else
	const float blendFactor = ( time - pTimes[ m_keyframeIndex ] ) ;
#endif //BS_NORMALISED_KEYFRAMES

	return ( ( hKey.m_D * blendFactor  + hKey.m_C ) * blendFactor  + hKey.m_B ) * blendFactor  + hKey.m_A;
}



BSAnimation::BSChannelWalker::BSChannelWalker()
: m_keyframeIndex( 0 )
{}

