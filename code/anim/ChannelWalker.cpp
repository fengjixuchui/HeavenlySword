//------------------------------------------------------------------------------------------
//!
//!	\file ChannelWalker.cpp
//!
//------------------------------------------------------------------------------------------
#include "anim/ChannelWalker.h"

#include "core/exportstruct_anim.h"
#include "core/exportstruct_keyframe.h"

//------------------------------------------------------------------------------------------
//!
//!	ChannelWalker::Seek
//! we could do a binary search here instead.
//! probably not worth it unless new time is significantly different to current time.
//! maybe use a heuristic, eg:
//! if abs( newtime - oldtime ) > 0.2 seconds then
//!	binary-search seek
//! else
//!	linear seek
//!
//------------------------------------------------------------------------------------------
void ChannelWalker::Seek( const NAnim::Channel *pobChannel, float fTime )
{
	const NAnim::Keyframe *keyframes = ResolveOffset< const NAnim::Keyframe >( pobChannel->m_KeyframeArrayOffset, pobChannel );

	ntAssert( m_iKeyframeIndex >= 0 && m_iKeyframeIndex < pobChannel->m_NumKeyframes-1 );
	ntAssert( fTime >=0.0f );

	// HACK There seem to be numerical inaccuracyies between m_pobAnimationHeader->m_fDuration 
	// and the last keyframe (1 bit in a float
	// so for now we clamp fTime to the maximum keyframe time
	if ( fTime > keyframes[ pobChannel->m_NumKeyframes-1 ].m_Time )
	{
		fTime = keyframes[ pobChannel->m_NumKeyframes-1 ].m_Time;
	}
	ntAssert( fTime <= keyframes[ pobChannel->m_NumKeyframes-1 ].m_Time );	// duration is last keytime

	// seek to the segment containing the desired time
	while ( fTime > keyframes[ m_iKeyframeIndex+1 ].m_Time )
		++m_iKeyframeIndex;		// seek forward

	while ( fTime < keyframes[ m_iKeyframeIndex ].m_Time )
		--m_iKeyframeIndex;		// seek back

	// now set the position along the segment
	const float fSegmentDuration = keyframes[ m_iKeyframeIndex+1 ].m_Time - keyframes[ m_iKeyframeIndex ].m_Time;
	m_fKeyframeBlendFactor = ( fTime - keyframes[ m_iKeyframeIndex ].m_Time ) / fSegmentDuration;
}

//------------------------------------------------------------------------------------------
//!
//!	ChannelWalker::Evaluate
//!	Generic evaluation of a channel.
//!	This is now the only place in the code that keyframes are actually evaluated. This is
//!	A Good Thing because we can replace this function with a lovely hand-crafted SPU
//!	intrinsic version at some point without worrying about shafting everything with a
//!	big knarly stick.
//!
//------------------------------------------------------------------------------------------
v128 ChannelWalker::Evaluate( const NAnim::Channel *channel ) const
{
	// Get a pointer to the keyframe array
	const NAnim::Keyframe &keyframe = ResolveOffset< const NAnim::Keyframe >( channel->m_KeyframeArrayOffset, channel )[ m_iKeyframeIndex ];

	// Evaluate it..
	const CVector blend_factor( m_fKeyframeBlendFactor );

	// We calculate as a quat because then we can normalise without casting if we actually have a rotation keyframe.
	CQuat result( ( ( ( ( ( keyframe.m_Data.m_D * blend_factor ) + keyframe.m_Data.m_C ) * blend_factor ) + keyframe.m_Data.m_B ) * blend_factor ) + keyframe.m_Data.m_A );

	if ( channel->m_Class == KEYFRAME_CLASS_ROTATION )
	{
		result.Normalise();
	}

	return result.Quadword();
}





