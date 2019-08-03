/***************************************************************************************************
*
*	$Header:: /tools/export/exportstruct_anim.h 14    13/08/03 11:11 Ben                           $
*
*	Data structures & constant definitions shared between the game and associated export tools that
*	define our animation structure.
*
*	CHANGES
*
*	16/4/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	_EXPORTSTRUCT_ANIM_H
#define	_EXPORTSTRUCT_ANIM_H

#include "anim/CharacterBoneID.h"

// ANIM_VERSION should begin 'AS' on ps3, and 'AC' on PC - both currently being version '01' 
//	for details of spec see JIRA item: TLS-174. [PWC]

#ifdef PLATFORM_PC
#define	OLD_ANIM_VERSION	MAKE_ID( 'A', 'C', '0', '1' )
#define	ANIM_VERSION		MAKE_ID( 'A', 'C', '0', '2' )
#elif defined(PLATFORM_PS3)
#define OLD_ANIM_VERSION	MAKE_ID( 'A', 'S', '0', '1' )
#define	ANIM_VERSION		MAKE_ID( 'A', 'S', '0', '2' )
#endif

// Instead of using direct checks for a transform index of 0, please use this define
// when indexing the root transform. It makes the underlying code behaviour much clearer.
#define	ROOT_TRANSFORM	0

/***************************************************************************************************
*
*	ENUMERATION		CAnimation Flags
*
*	DESCRIPTION		Flags for the CAnimation object. These are copied from the flags in the
*					CAnimationHeader object, and as such it seemed sensible to put them here.
*
***************************************************************************************************/

enum	ANIMATION_FLAGS
{
	// These bits are normally set by the exporter...
	ANIMF_CHARACTER_ANIM		=	( 1 << 0 ),			// Is this a character animation (ie do hierarchy indices hold bone ID numbers?)

	// All these bits below are set by the application - they all indicate how the animation is to be used
	// Possibly how the animations are used should be split away from the more low level aniamtion evaluation
	// in the future.

	ANIMF_LOOPING				=	( 1 << 1 ),			// Is the animation looping?
	ANIMF_LOCOMOTING			=	( 1 << 2 ),			// Is the animation supposed to be moving/rotating the object in 3D space?
	ANIMF_PHASE_LINKED			=	( 1 << 3 ),			// Is the animation to be phase linked with other anims already playing?

//	ANIMF_MANUAL_WEIGHT_CONTROL	=	( 1 << 4 ),			// Is the weight controlled manually, or via blend-in/out times
														// NOTE: NO LONGER SUPPORTED AS A FLAG - *ALL* ANIMATIONS HAVE MANUAL WEIGHT CONTROL.
	ANIMF_INHIBIT_AUTO_DESTRUCT	=	( 1 << 5 ),			// If set, an animation won't be deleted when it completes or has a complete blend of zero?

	ANIMF_PHASE_OFFSET			=	( 1 << 6 ),			// If set this means that the anim will be used 180 out of phase - for character 'transitions'
	ANIMF_HALF_PHASE			=	( 1 << 7 ),			// If set this means that the animation should last
	ANIMF_RELATIVE_MOVEMENT		=	( 1 << 8 ),			// Provides movement relative to a matrix - needs to be locomoting too
	ANIMF_WAS_FORCIBLY_REMOVED	=	( 1 << 9 )
};


/***************************************************************************************************
*
*	ENUMERATION		Keyframe Flags
*
*	DESCRIPTION		Flags associated with each keyframe. Please note that the keyframe flags field
*					is only 8 bits in size, so don't go defining loads of flags.
*
***************************************************************************************************/

enum	KEYFRAME_FLAGS
{
	// These bits are normally set by the exporter...

	KEYF_IS_CONSTANT			=	( 1 << 0 ),			// Does this keyframe represent a constant that means we can avoid excess calculations?
};



// #ifndef JAM_EXPORTER
// 
// struct	CAnimationHeader;
// struct	CAnimationTransform;
// class	AnimBuilder;
// namespace NAnim { struct Channel; }
// 
// /***************************************************************************************************
// *
// *	CLASS			CAnimationHeader
// *
// *	DESCRIPTION		This class defines the header of animation data. This header would be the start
// *					of loaded data, and is assumed to be read-only in state. All of the other classes
// *					used by the animation format are located in memory after this header. Where
// *					pointers exist, they are stored in the file as offsets from the start of the
// *					CAnimationHeader object. They are resolved upon load.
// *
// ***************************************************************************************************/
// ALIGNTO_PREFIX( 16 ) struct CAnimationHeader
// {
// 	CQuat					m_RootEndRotation;					// For locomoting animations, these two members hold the rotation and translation at
// 	CPoint					m_RootEndTranslation;				// the very end of the animation. This is needed to ensure correct delta calculation.
// 
// 	uint32_t				m_VersionTag;						// Type & Version Tag ( e.g. "AN01" )
// 	uint32_t				m_Flags;							// Character or environmental animation?
// 	uint32_t				m_HierarchyKey;						// Key to ensure that non-character animations are playing on the right hierarchy..
// 	int32_t					m_Priority;							// Animation priority (starting hierarchy level)
// 
// 	int32_t					m_NumAnimationTransforms;			// How many transforms are being affected by this animation
// 	int32_t					m_TotalNumberOfChannels;			// Total number of channels (across all transforms)
// 
// 	float					m_Duration;							// Animation duration
// 
// 	uint32_t				m_AnimNameHash;
// 
// 	uint32_t				m_AnimHeaderLength;					// Length of the header and all anim info (channels, keyframes etc...) as a contiguous block in memory.
// 
// 	// This will work as long as the offset isn't larger than 2^32... which i seriously hope it isn't!
// 	DEF_OFFSET( int16_t )				m_AnimationToHierarchyLookupOffset;	// Pointer to lookup array to go from animation to hierarchy transform index (will contain bone ID for characters)
// 	DEF_OFFSET( CAnimationTransform )	m_AnimationTransformArrayOffset;
// 	DEF_OFFSET( NAnim::Channel )		m_ChannelArrayOffset;
// 
// private:
// 	// Private to ensure proper deallocation.
// 	~CAnimationHeader() {}	
// }
// ALIGNTO_POSTFIX( 16 );
// 
// //
// //	This is what's actually loaded - it's then munged by the AnimLoader into the AnimHeader class above.
// //	NOTE: This is very temporary and should be removed by an animation format change. [ARV].
// //
// ALIGNTO_PREFIX( 16 ) struct CAnimationHeader_Old
// {
// 	uint32_t				m_uiVersionTag;						// Type & Version Tag ( e.g. "AN01" )
// 	uint32_t				m_iFlags;							// Character or environmental animation?
// 	uint32_t				m_uiHierarchyKey;					// Key to ensure that non-character animations are playing on the right hierarchy..
// 	int32_t					m_iPriority;						// Animation priority (starting hierarchy level)
// 
// 	float					m_fDuration;						// Animation duration
// 	int32_t					m_iNumberOfAnimationTransforms;		// How many transforms are being affected by this animation
// 
// 	// The below member replaces a pointer. Pointers are 8-bytes on PPU, so we pad out the first 4-bytes.
// 	// This will work as long as the offset isn't larger than 2^32... which i seriously hope it isn't!
// 	DEF_OFFSET( CAnimationTransform )	m_pobAnimationTransformArrayOffset;
// 
// 	int32_t					m_iTotalNumberOfChannels;			// Total number of channels (across all transforms)
// 
// 	// 36 bytes to here. CQuat is 16-byte aligned so its
// 	// offset within the struct will be 48 bytes.
// 	CQuat					m_obRootEndRotation;				// For locomoting animations, these two members hold the rotation and translation at
// 	CPoint					m_obRootEndTranslation;				// the very end of the animation. This is needed to ensure correct delta calculation.
// 
// 	DEF_OFFSET( int16_t )	m_psAnimationToHierarchyLookupOffset;	// Pointer to lookup array to go from animation to hierarchy transform index (will contain bone ID for characters)
// 
// 	float					m_fMinimumBlendInTime;				// TO BE IMPLEMENTED :	We need to store the minimum possible blend time for this animation, which is computed by
// 																//						looking for when we cross 180 degrees of rotation. Currently this is 0.0f, but it's not
// 																//						used in the animation system as yet. When this is implemented, we'll need to bump the 
// 																//						animation version tag.
// 	uint32_t				m_uiAnimNameHash;
// 
// #	ifdef _DEBUG
// 			const char *		m_pcAnimName;
// #		else
// 	uint32_t				m_uiPad[ 1 ];
// #		endif
// 
// 	// we must have a header memory footprint of 92 bytes so we have enough room for the
// 	// new animation header class
// #ifdef PLATFORM_PC
// 	uint32_t				m_uiPad2[ 3 ];
// #	else
// 		uint32_t				m_PtrPad3[ 2 ];
// #	endif	// _DEBUG
// 
// #	ifndef PLATFORM_PC
// 		// Pad to 112 bytes.
// 		uint32_t				m_PadToEnd;
// #	endif
// 
// private:
// 	// Private to ensure proper deallocation.
// 	~CAnimationHeader_Old() {}	
// }
// ALIGNTO_POSTFIX( 16 );
// 
// /***************************************************************************************************
// *	
// *	CLASS			CAnimationTransform
// *
// *	DESCRIPTION		This class defines a wrapper around a group of animating channels allowing them
// *					to be identified with a particular transform. The transform index stored in this
// *					object is *not* a hierarchy index. If an animation affects 3 transforms in total
// *					then the index will be in the range 0 to 2.. see?
// *
// ***************************************************************************************************/
// class	CAnimationTransform_Old
// {
// public:
// 	short					m_sTransformIndex;					// What is the index of the transform that this applies to?
// 	short					m_sNumberOfChannels;				// How many channels are animating on this transform
// 	DEF_OFFSET( NAnim::Channel )	m_pobAnimationChannelArrayOffset;
// };
// struct CAnimationTransform
// {
// 	int16_t							m_TransformIndex;			// What is the index of the transform that this applies to?
// 	int16_t							m_NumChannels;				// How many channels are animating on this transform
// 
// 	DEF_OFFSET( NAnim::Channel )	m_AnimationChannelOffset;	// The offset from this transfrom to the first of its channels.
// };
// 
// #endif // !JAM_EXPORTER


#endif	//_EXPORTSTRUCT_ANIM_H
