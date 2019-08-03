/***************************************************************************************************
*
*	DESCRIPTION		A Battlefield chunk is the basic execution unit of grunt updates, has
*					the local heightfield
*
*	NOTES
*
***************************************************************************************************/

#ifndef BATTLEFIELD_CHUNK_H_
#define BATTLEFIELD_CHUNK_H_

//! how many height field texels width per chunk
#define BF_CHUNK_WIDTH	32
//! how many height field texels height per chunk
#define BF_CHUNK_HEIGHT	32

//------------------------------------------------------
//!
//! A Battlefield chunk is a section of heightfield with
//! a bunch of grunts indices contained, 
//! its uploaded to an SPU and each grunt in this chunk 
//! logic is run, this gives us mostly spatial localised updates
//!
//------------------------------------------------------
struct BattlefieldChunk
{
	//! number of grunts referenced in this chunk
	uint16_t	m_NumGrunts;

	uint16_t	m_ChunkId;

	// the integer coord2 of the heightfield where this
	// chunk starts (i.e. a block 64x64 texels from the top 
	// left of the master height field would have 64,64 here
	uint16_t	m_TopCoord;
	uint16_t	m_LeftCoord;

	//! this chunk of heightfield currently 17K of mem (this could probably be a int16 normalised hf...)
	float	m_HeightField[(BF_CHUNK_WIDTH+2) * (BF_CHUNK_HEIGHT+2)];

};

// bring in the inlines
#include "army/battlefield_chunk.inl"

#endif //BATTLEFIELD_CHUNK_H_

