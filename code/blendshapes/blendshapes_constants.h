//--------------------------------------------------
//!
//!	\file blendshapes_constants.h
//! Just some blendshape specific constants
//!
//--------------------------------------------------

#ifndef _BLENDSHAPES_COMMON_CONSTANTS_H_
#define _BLENDSHAPES_COMMON_CONSTANTS_H_

namespace blendshapes
{
	//! max number of total blenshape targets
	static const unsigned int MAX_TARGETS = 70;
	//! max number of deltas per target
	static const unsigned int MAX_DELTAS = 2400;
	//! max vertices per blend batch
	static const unsigned int MAX_VERTS_PER_BATCH = 3200;
	//! max number of animations playable at once on a given BSAnimator instance
	static const unsigned int MAX_ANIMS = 4;
	//! number of wrinkle areas
	static const unsigned int MAX_WRINKLE_AREAS = 8;

	static const unsigned int MAX_WEIGHTS = MAX_WRINKLE_AREAS + MAX_TARGETS;


	static const unsigned int BS_SPU_PARAM_VERTEX_BUFFER	= 0;
	static const unsigned int BS_SPU_PARAM_TARGETS_BUFFER	= 1;
	static const unsigned int BS_SPU_PARAM_WEIGHTS_BUFFER	= 2;
	static const unsigned int BS_SPU_PARAM_NUM_OF_TARGETS	= 3;
	static const unsigned int BS_SPU_PARAM_INDEX_OFFSET		= 4;
	static const unsigned int BS_SPU_PARAM_NUM_OF_VERTS		= 5;
	static const unsigned int BS_SPU_PARAM_VERTEX_STRIDE	= 6;
	static const unsigned int BS_SPU_PARAM_MATRICES_BUFFER	= 7;
};

#endif // end of _BLENDSHAPES_COMMON_CONSTANTS_H_
