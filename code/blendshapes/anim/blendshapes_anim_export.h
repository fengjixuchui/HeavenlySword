//--------------------------------------------------
//!
//!	\file blendshapes_anim_export.h
//!
//! 
//--------------------------------------------------

#ifndef _BLENDSHAPES_ANIM_EXPORT_H_
#define _BLENDSHAPES_ANIM_EXPORT_H_

#ifdef _FINALISER
typedef u32 uint32_t;
#endif //_FINALISER

enum BSAnimFlags
{
	kBSAF_Looping				= ( 1 << 0 ),					//! this animation loops

	kBSAF_NoDelete				= ( 1 << 4 ),					//! Don't discard this anim after it's done. Stays on last keyframe
	
	kBSAF_Corrective			= ( 1 << 7 ),					//! UNUSED
};

//! channel usage type
enum BSAnimChannelType
{
	kBSAC_Invalid				=	-1,

	kBSAC_BSTargetWeight		=	0,				//! this channel refers to a blendshape target weight
	kBSAC_WrinkleWeight,							//! this channel refers to a wrinkle map weight
};



//! single curve-based weight keyframe
struct BSKeyframe
{
	float m_A;
	float m_B;
	float m_C;
	float m_D;

	BSKeyframe()
		: m_A( 0 )
		, m_B( 0 )
		, m_C( 0 )
		, m_D( 0 )
	{
		//empty
	}
};

//! each channels holds the weight animation for one target shape
struct BSAnimChannelExport
{
	uint32_t				m_numOfKeyframes;						//! how many keyframes are there? 
	float*					m_pKeyframeTimes;						//! array of keyframe times
	BSKeyframe*				m_pKeyframes;							//! array of keyframe structs
	BSAnimChannelType		m_type;	
	uint32_t				m_targetIndex;							//! channel target index (-1 if invalid). This index is context dependent on channel type

	BSAnimChannelExport()
		: m_numOfKeyframes( 0 )
		, m_pKeyframeTimes( 0 )
		, m_pKeyframes( 0 )
		, m_type( kBSAC_Invalid )
		, m_targetIndex( 0 )
	{
		// nothing
	}
};


struct BSAnimExport
{
	uint32_t				m_nameHash;												//! anim name (usually same as clip)
	uint32_t				m_blendShapesNameHash;									//! which set is this anim supposed to animate? (note that it might still be played onto any common set)
	uint32_t				m_clumpNameHash;
	uint32_t				m_uiHierarchyKey;
	float					m_duration;											//! total duration 
	
	BSAnimChannelExport*	m_pChannels;										//! array if anim channels
	uint32_t				m_numOfChannels;									//! how many channels?

	uint32_t				m_flags;											//! animation clip flags. See BSAnimFlags	

	BSAnimExport()
		: m_nameHash( 0 )
		, m_blendShapesNameHash( 0 )
		, m_clumpNameHash( 0 )
		, m_uiHierarchyKey( 0 )
		, m_duration( 0 )
		, m_pChannels( 0 )
		, m_numOfChannels( 0 )
		, m_flags( 0 )
	{
		// nada
	}
};	



#ifdef _FINALISER

BI_START( BSKeyframe )
	BI_RAWDATA( m_A )
	BI_RAWDATA( m_B )
	BI_RAWDATA( m_C )
	BI_RAWDATA( m_D )
BI_END( BSKeyframe )

BI_START( BSAnimChannelExport )
	BI_RAWDATA( m_numOfKeyframes )
	BI_POINTER_TO_MEMBERSIZE_ARRAY( m_pKeyframeTimes, m_numOfKeyframes, true  )
	BI_POINTER_TO_MEMBERSIZE_ARRAY( m_pKeyframes, m_numOfKeyframes, true )
	BI_RAWDATA( m_type )
	BI_RAWDATA( m_targetIndex )
BI_END( BSAnimChannelExport )

BI_START( BSAnimExport )
	BI_RAWDATA( m_nameHash )
	BI_RAWDATA( m_blendShapesNameHash )
	BI_RAWDATA( m_clumpNameHash )
	BI_RAWDATA( m_uiHierarchyKey )
	BI_RAWDATA( m_duration )
	BI_POINTER_TO_MEMBERSIZE_ARRAY( m_pChannels, m_numOfChannels, true  )
	BI_POINTER_TO_MEMBERSIZE_ARRAY( m_pIndexToTargetIDMap, m_numOfChannels, true )
	BI_RAWDATA( m_numOfChannels )
	BI_RAWDATA( m_flags )
BI_END( BSAnimExport )

//BI_START( BSAnimClipExport )
//	BI_RAWDATA( m_name )
//	BI_RAWDATA( m_clumpName )
//	BI_RAWDATA( m_uiHierarchyKey )
//	BI_POINTER_TO_MEMBERSIZE_ARRAY( m_pAnims, m_numOfAnims, true )
//	BI_RAWDATA( m_numOfAnims )
//	BI_RAWDATA( m_totalNumOfChannels )
//BI_END( BSAnimClipExport )

#endif //_FINALISER

#endif // end of _BLENDSHAPES_ANIM_EXPORT_H_
