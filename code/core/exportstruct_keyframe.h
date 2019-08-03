/***************************************************************************************************
*
*	$Header:: /game/exportstruct_keyframe.h 5     15/05/03 11:15 Dean                              $
*
*	Data structures & constant definitions shared between the game and associated export tools that
*	define our keyframe structures.
*
*	CHANGES
*
*	16/4/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	_EXPORTSTRUCT_KEYFRAME_H
#define	_EXPORTSTRUCT_KEYFRAME_H

/***************************************************************************************************
*
*	ENUMERATION		KEYFRAME_CLASS
*
*	DESCRIPTION		These are our keyframe classes.
*					BIG IMPORTANT NOTE!
*						The order of the members of this enum must correspond to the order
*						of the members of the AnimtionTarget class. I'd really like to be able to
*						put down some static_asserts to make people aware of this if something
*						breaks/changes - however, GCC won't let me find the offset to a
*						class-member-variable at compile time. How crap. Because of this, I'm
*						putting in static_asserts for VC7.1 and will also call a function on
*						start-up that checks the same thing using run-time asserts (NAnim::DebugCheck).
*						If you want to know why I'm checking these things then look at Animation.cpp
*						in the CAnimation::Update function where the AnimationTargets are updated;
*						around line 700 or so.
*
***************************************************************************************************/
enum	KEYFRAME_CLASS
{
	KEYFRAME_CLASS_ROTATION,
	KEYFRAME_CLASS_TRANSLATION,
	KEYFRAME_CLASS_TRACKING,

	NUMBER_OF_KEYFRAME_CLASSES,
};

#ifdef MSC_VER
	// See BIG IMPORTANT NOTE in the KEYFRAME_CLASS description, above.
	static_assert_in_class( KEYFRAME_CLASS_ROTATION == offsetof( AnimationTarget, m_obRotation ), using_this_as_a_pointer_offset );
	static_assert_in_class( KEYFRAME_CLASS_TRANSLATION == offsetof( AnimationTarget, m_obTranslation ), using_this_as_a_pointer_offset );
	static_assert_in_class( KEYFRAME_CLASS_TRACKING == offsetof( AnimationTarget, m_obTracking ), using_this_as_a_pointer_offset );
#endif	// MSC_VER

static_assert_in_class( sizeof( v128 ) == sizeof( CPoint ), must_be_the_same_for_the_anim_pointer_arithmetic_to_work );
static_assert_in_class( sizeof( v128 ) == sizeof( CQuat ), must_be_the_same_for_the_anim_pointer_arithmetic_to_work );
static_assert_in_class( sizeof( v128 ) == sizeof( CVector ), must_be_the_same_for_the_anim_pointer_arithmetic_to_work );

namespace NAnim
{
	// See BIG IMPORTANT NOTE in the KEYFRAME_CLASS description, above.
	void DebugCheck();
}


/***************************************************************************************************
*
*	ENUMERATION		KEYFRAME_TYPE
*
*	DESCRIPTION		These are our keyframe types..
*
***************************************************************************************************/

enum	KEYFRAME_TYPE
{
//	KEYFRAME_TYPE_STANDARD_ROTATION,	// These two types now invalid. Everything should be a curve.
//	KEYFRAME_TYPE_STANDARD_TRANSLATION,

	KEYFRAME_TYPE_CURVE_ROTATION = 2,	// Have to set this to 2 manually for now because I've removed the above two types.
	KEYFRAME_TYPE_CURVE_TRANSLATION,

	NUMBER_OF_KEYFRAME_TYPES,
};


#ifndef JAM_EXPORTER
/***************************************************************************************************
*	
*	CLASS			Keyframe
*
*	DESCRIPTION		Defines a single curve-based keyframe.
*
***************************************************************************************************/
namespace NAnim_Old
{
	ALIGNTO_PREFIX( 16 ) struct Keyframe
	{
		CVector	m_obA;
		CVector	m_obB;
		CVector	m_obC;
		CVector	m_obD;
	}
	ALIGNTO_POSTFIX( 16 );
}
namespace NAnim
{
	//	Actual animation data for a keyframe - will be aligned to a
	//	16-byte boundary because CVector is aligned.
	struct KeyframeData
	{
		CVector	m_A;
		CVector	m_B;
		CVector	m_C;
		CVector	m_D;
	};

	struct Keyframe
	{
		KeyframeData	m_Data;
		float			m_Time;
		uint32_t		m_Flags;
	};
}

/***************************************************************************************************
*	
*	CLASS			CAnimationChannel
*
*	DESCRIPTION		This encapsulates all keyframes of a particular type (a channel) associated with
*					a particular transform.
*
*	NOTES			The instance of an animation will keep track of current positions within each
*					channel.
*
***************************************************************************************************/
namespace NAnim_Old
{
	struct Channel
	{
		KEYFRAME_CLASS			m_eKeyframeClass;					// What class of keyframe is it?
		KEYFRAME_TYPE			m_eKeyframeType;					// What kind of keyframe channel is this?
		int						m_iNumberOfKeyframes;				// How many keyframes does this channel have? Must be more than zero.
		int						m_iWalkerArrayIndex;				// Index into the instance-specific array of ChannelWalker objects

		DEF_OFFSET( float )				m_KeyframeTimesOffset;		// Offset from top of class to an array of times for keyframes.
		DEF_OFFSET( uint8_t )			m_KeyframeFlagsOffset;		// Offset from the top of class to an array of flags for keyframes.
		DEF_OFFSET( NAnim::Keyframe )	m_KeyframeArrayOffset;		// Offset from the top of class to an array of keyframes.
	};
}

namespace NAnim
{
	ALIGNTO_PREFIX( 16 ) struct Channel
	{
		KEYFRAME_CLASS		m_Class;							// What class is this channel? I.E. Rotation, Translation or Tracking.
		int32_t				m_NumKeyframes;						// How many keyframes does this channel have? Must be at least two.
		int32_t				m_WalkerArrayIndex;					// Which channel-walker should we be using?

		DEF_OFFSET( NAnim::Keyframe )	m_KeyframeArrayOffset;	// Offset to this channel's array of keyframes.
	}
	ALIGNTO_POSTFIX( 16 );

	static_assert_in_class( sizeof( Channel ) == 16, This_should_be_16_bytes_long_for_good_memory_and_dma_usage );
}
#endif // JAM_EXPORTER



#endif	//_EXPORTSTRUCT_KEYFRAME_H

