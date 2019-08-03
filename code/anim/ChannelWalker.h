//------------------------------------------------------------------------------------------
//!
//!	\file ChannelWalker.h
//!
//------------------------------------------------------------------------------------------

#ifndef	CHANNELWALKER_H_
#define	CHANNELWALKER_H_

// Forward declarations.
namespace NAnim { struct Channel; }
class SPUAnim;

//------------------------------------------------------------------------------------------
//!
//!	ChannelWalker
//!	Maintains a position in an animation channel and provides functions to
//!	evaluate the channel at that point.
//!	
//!	The pointer to the channel is _not_ held in this structure. Hence all the fns
//!	require you to pass the channel in as a parameter.
//!	
//!	It's fine to use this class outside of CAnimator/CAnimation.
//!	For example, for pick-up animations you may need to find the initial offset
//!	of the root node in order to position the player correctly.
//!	
//!	It may be worth adding a binary seek ability to this class - it depends in the
//!	complexity of our planned animation usage.
//!
//------------------------------------------------------------------------------------------
class ChannelWalker
{
	public:
		// set to beginning of channel
		void	Clear			()	{ m_iKeyframeIndex = 0; m_fKeyframeBlendFactor = 0.0f; }

		// Set current position to fTime
		void	Seek			( const NAnim::Channel * pobChannel, float fTime );

		// Generic evaluation.
		v128	Evaluate		( const NAnim::Channel *channel )		const;

	private:
		// Allow SPU code to look in here.
		friend class SPUAnim;

		// The current keyframe index (left of blend equation) : right is +1
		int32_t					m_iKeyframeIndex;			

		// The blend factor
		float					m_fKeyframeBlendFactor;				
};

#endif // !CHANNELWALKER_H_
