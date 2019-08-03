/***************************************************************************************************
*
*	DESCRIPTION		ChannelWalker functions on SPUs.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <basetypes_spu.h>
#include <debug_spu.h>

#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"

#include "animation_spu.h"

#include "anim/ChannelWalker.h"

#include "core/exportstruct_keyframe.h"
#include "animation_spu_structs.h"

//**************************************************************************************
//	TODO: Optimise implementation using inline intrinsics.
//**************************************************************************************
v128 SPUAnim::Evaluate( const ChannelWalker *channel_walker, const NAnim::Channel *channel, const NAnim::KeyframeData &keyframe_data )
{
	ntError( reinterpret_cast< int32_t >( channel_walker ) < 0x30000 );
	ntError( reinterpret_cast< int32_t >( channel ) < 0x30000 );

	// Create a vector version of our blend_factor - same value in each xyzw component.
	const CVector blend_factor( spu_splats( channel_walker->m_fKeyframeBlendFactor ) );

	// Evaluate it..
	// We calculate as a quat because then we can normalise without casting if we actually have a rotation keyframe.
	CQuat result( ( ( ( ( ( keyframe_data.m_D * blend_factor ) + keyframe_data.m_C ) * blend_factor ) + keyframe_data.m_B ) * blend_factor ) + keyframe_data.m_A );

	if ( channel->m_Class == KEYFRAME_CLASS_ROTATION )
	{
		result.Normalise();
	}

	return result.Quadword();
}

//**************************************************************************************
//	TODO: Optimise implementation using inline intrinsics.
//**************************************************************************************
void SPUAnim::Seek( ChannelWalker *channel_walker, const NAnim::Channel *channel, float time, const NAnim::Keyframe *keyframes, int32_t keyframe_array_ea, float anim_duration, int32_t channel_idx )
{
	ntError( reinterpret_cast< int32_t >( channel_walker ) < 0x30000 );
	ntError( reinterpret_cast< int32_t >( channel ) < 0x30000 );
	ntError( reinterpret_cast< int32_t >( keyframes ) < 0x30000 );




/*

	const NAnim::Keyframe *ls_keyframe = ResolveOffset< const NAnim::Keyframe >( channel->m_KeyframeArrayOffset, channel );
	ntError( channel_walker->m_iKeyframeIndex >= 0 && channel_walker->m_iKeyframeIndex < channel->m_NumKeyframes-1 );


	ntAssert_p( channel_walker->m_iKeyframeIndex >= 0 && channel_walker->m_iKeyframeIndex < channel->m_NumKeyframes-1, ("m_iKeyframeIndex=%i, m_iNumberOfKeyframes=%i", channel_walker->m_iKeyframeIndex, channel->m_NumKeyframes) );
	ntAssert( time >=0.0f );
	ntAssert( time <= ls_keyframe[ channel->m_NumKeyframes-1 ].m_Time );	// duration is last keytime

	// seek to the segment containing the desired time
	while ( time > ls_keyframe[ channel_walker->m_iKeyframeIndex+1 ].m_Time )
		++( channel_walker->m_iKeyframeIndex );		// seek forward

	while ( time < ls_keyframe[ channel_walker->m_iKeyframeIndex ].m_Time )
		--( channel_walker->m_iKeyframeIndex );		// seek back

	// now set the position along the segment
	const float fSegmentDuration = ls_keyframe[ channel_walker->m_iKeyframeIndex+1 ].m_Time - ls_keyframe[ channel_walker->m_iKeyframeIndex ].m_Time;
	channel_walker->m_fKeyframeBlendFactor = ( time - ls_keyframe[ channel_walker->m_iKeyframeIndex ].m_Time ) / fSegmentDuration;

	
	
	ntAssert_p( channel_walker->m_iKeyframeIndex >= 0 && channel_walker->m_iKeyframeIndex < channel->m_NumKeyframes-1, ("m_iKeyframeIndex=%i, m_iNumberOfKeyframes=%i", channel_walker->m_iKeyframeIndex, channel->m_NumKeyframes) );
	CurrentKeyframes[ channel_idx ] = ls_keyframe[ channel_walker->m_iKeyframeIndex ].m_Data;
*/



	ntAssert_p( channel_walker->m_iKeyframeIndex >= 0 && channel_walker->m_iKeyframeIndex < channel->m_NumKeyframes-1, ("m_iKeyframeIndex=%i, m_iNumberOfKeyframes=%i", channel_walker->m_iKeyframeIndex, channel->m_NumKeyframes) );
	ntAssert( time >=0.0f );


	// Cache keyframe times.
	float keyframe_end_time, keyframe_start_time;

	// Cache keyframe index;
	int32_t keyframe_idx = channel_walker->m_iKeyframeIndex;

	// Is the current keyframe ok?
	if ( time >= keyframes[ 1 ].m_Time && time <= keyframes[ 2 ].m_Time )
	{
		ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes-1 );
		keyframe_start_time = keyframes[ 1 ].m_Time;
		keyframe_end_time = keyframes[ 2 ].m_Time;
		CurrentKeyframes[ channel_idx ] = keyframes[ 1 ].m_Data;
	}
	// What about the next keyframe?
	else if ( keyframe_idx < channel->m_NumKeyframes-1 && time >= keyframes[ 2 ].m_Time && time <= keyframes[ 3 ].m_Time )
	{
		keyframe_idx++;
		ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes-1 );
		keyframe_start_time = keyframes[ 2 ].m_Time;
		keyframe_end_time = keyframes[ 3 ].m_Time;

		CurrentKeyframes[ channel_idx ] = keyframes[ 2 ].m_Data;
	}
	// Previous keyframe?
	else if ( keyframe_idx > 0 && time >= keyframes[ 0 ].m_Time && time <= keyframes[ 1 ].m_Time )
	{
		keyframe_idx--;
		ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes-1 );
		keyframe_start_time = keyframes[ 0 ].m_Time;
		keyframe_end_time = keyframes[ 1 ].m_Time;

		CurrentKeyframes[ channel_idx ] = keyframes[ 0 ].m_Data;
	}
	else
	{
		//
		//	HACK. STOPS EXPORTER BUG FROM FRYING THE ANIM SYSTEM. [ARV].
		//
//		ntError_p( channel->m_NumKeyframes > 2, ("If we only have two keyframes we should always have found the next/prev keyframe above...") );
		if ( channel->m_NumKeyframes == 2 )
		{
			if ( keyframe_idx < channel->m_NumKeyframes - 2 )
			{
				// Just move onto the next keyframe.
				CurrentKeyframes[ channel_idx ] = keyframes[ 1 ].m_Data;
				keyframe_idx++;
			}
			else
			{
				// Keep the current keyframe.
				CurrentKeyframes[ channel_idx ] = keyframes[ 0 ].m_Data;
			}

			ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes - 1 );
			channel_walker->m_iKeyframeIndex = keyframe_idx;

			channel_walker->m_fKeyframeBlendFactor = 0.0f;	// This will fuck-over the anim-blending, but it won't crash.

			return;
		}
		//
		//	END HACK.
		//


		// Bugger. Have to do a proper search - since this is horrible anyway, it should really be a binary search.
		// Work out whether to seach forwards or backwards.
		static NAnim::Keyframe keyframe_buffer[ 2 ] __attribute__((aligned( 16 )));
		ntDMA_ID kb_id = ntDMA::GetFreshID();

		bool search_backwards = time < keyframes[ 1 ].m_Time;
		if ( search_backwards )
		{
			// We know it's not the previous keyframe so skip that one.
			keyframe_idx--;

			keyframe_start_time = keyframes[ 0 ].m_Time;

			do
			{
				ntError( keyframe_idx > 0 );
				ntError( keyframe_idx < channel->m_NumKeyframes-1 );

				ntDMA::Params params;
				params.Init32(	&( keyframe_buffer[ 0 ] ), keyframe_array_ea + sizeof( NAnim::Keyframe )*( keyframe_idx-1 ),
								sizeof( NAnim::Keyframe ), kb_id );
				ntDMA::DmaToSPU( params );
				ntDMA::StallForCompletion( kb_id );	// Yuck. Stall.

				keyframe_end_time = keyframe_start_time;
				keyframe_start_time = keyframe_buffer[ 0 ].m_Time;

				keyframe_idx--;
			}
			while ( time < keyframe_buffer[ 0 ].m_Time );

			ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes-1 );
			ntError( keyframe_start_time < keyframe_end_time );

			CurrentKeyframes[ channel_idx ] = keyframe_buffer[ 0 ].m_Data;
		}
		else
		{
			// We know it's not the next keyframe so skip that one.
			keyframe_end_time = keyframes[ 1 ].m_Time;

			do
			{
				ntError( keyframe_idx >= 0 );
				ntError_p( keyframe_idx < channel->m_NumKeyframes-1, ("keyframe_idx=%i, num_keyframes=%i, time=%f, keyframe.time=%f", keyframe_idx, channel->m_NumKeyframes, time, keyframe_buffer[ 0 ].m_Time) );

				ntDMA::Params params;
				params.Init32(	&( keyframe_buffer[ 0 ] ), keyframe_array_ea + sizeof( NAnim::Keyframe )*( keyframe_idx+1 ),
								sizeof( NAnim::Keyframe ), kb_id );
				ntDMA::DmaToSPU( params );
				ntDMA::StallForCompletion( kb_id );	// Yuck. Stall.

				keyframe_start_time = keyframe_end_time;
				keyframe_end_time = keyframe_buffer[ 0 ].m_Time;

				keyframe_idx++;
				if ( keyframe_idx == channel->m_NumKeyframes-1 )
				{
					// Handles the case where time=duration.
					break;
				}
			}
			while ( time >= keyframe_end_time );

			keyframe_idx--;

			ntError( keyframe_start_time <= time && ( time < keyframe_end_time || time == anim_duration ) );
			ntError( keyframe_idx >= 0 && keyframe_idx < channel->m_NumKeyframes-1 );
			ntError( keyframe_start_time < keyframe_end_time );

			CurrentKeyframes[ channel_idx ] = keyframe_buffer[ 0 ].m_Data;
		}

		ntDMA::FreeID( kb_id );
	}

	// now set the position along the segment
	ntError( keyframe_start_time < keyframe_end_time );
	ntError( fabsf( keyframe_end_time - keyframe_start_time ) > 0.0001f );
	channel_walker->m_fKeyframeBlendFactor = ( time - keyframe_start_time ) / ( keyframe_end_time - keyframe_start_time );
	channel_walker->m_iKeyframeIndex = keyframe_idx;
}

