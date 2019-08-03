//--------------------------------------------------
//!
//!	\file blendshapesbatchinfo_spu_ppu.h
//!	this thing only groups a few spu parameters together
//! to avoid hitting SPUArgumentList::MaxNumArguments
//!
//--------------------------------------------------

#ifndef _BLENDSHAPESBATCHINFO_SPU_PPU_H_
#define _BLENDSHAPESBATCHINFO_SPU_PPU_H_


#define BS_SPU_PARAM_VERTICES			0
#define BS_SPU_PARAM_TARGETS			1
#define BS_SPU_PARAM_WEIGHTS			2
#define BS_SPU_PARAM_MATRICES			3
#define	BS_SPU_PARAM_ADDITIONAL_INFO	4
#define BS_SPU_PARAM_TASKCOUNTER		5

struct BSSpuAdditionalInfo
{
	uint32_t	m_iNumOfTargets;
	uint32_t	m_iVertexStride;
	uint32_t	m_iNumOfVertices;
	uint32_t	m_iIndexOffset;
};

#endif // end of _BLENDSHAPESBATCHINFO_SPU_PPU_H_
