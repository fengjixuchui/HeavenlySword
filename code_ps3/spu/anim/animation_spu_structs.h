/***************************************************************************************************
*
*	DESCRIPTION		Animation on SPU specific structures.
*
*	NOTES
*
***************************************************************************************************/

#ifndef ANIMATION_SPU_STRUCTS_
#define ANIMATION_SPU_STRUCTS_

#ifndef __SPU__
#	error This header file is for use in PS3 SPU projects only.
#endif

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "core/exportstruct_keyframe.h"
#include "ntlib_spu/ntDma.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
struct KeyframeSet
{
	static const int32_t NumKeyframes = 4;
	NAnim::Keyframe m_Keyframes[ NumKeyframes ];

	ntDMA_ID		m_ID;
};

extern KeyframeSet *ChannelData;

static const int32_t MaxNumChannels = 256;
extern NAnim::KeyframeData *CurrentKeyframes;

#endif // !ANIMATION_SPU_STRUCTS_

