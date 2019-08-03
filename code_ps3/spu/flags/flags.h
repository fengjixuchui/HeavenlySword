#ifndef _FLAGS_H_
#define _FLAGS_H_

//--------------------------------------------------
//!
//!	\file flags.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "data.h"

class Code
{
public:
	static void ApplyForces( const FlagIn& __restrict__ flagIn,
		PointDynamic* __restrict__ pCurrent, const PointDynamic* __restrict__ pBefore, const PointDynamic* __restrict__ pEvenBefore, VertexDynamic* __restrict__ pMesh);
	static void SatisfyConstraint(const FlagIn& __restrict__ flagIn, PointDynamic* __restrict__ pCurrent);
	static void GenerateMesh(const FlagIn& __restrict__ flagIn, const PointDynamic* __restrict__ pCurrent,
		VertexDynamic* __restrict__ pMesh, FlagOut* __restrict__ pFlagOut);
	static void CopyPoints(const FlagIn& __restrict__ flagIn, PointDynamic* __restrict__ pCurrent, const PointDynamic* __restrict__ pBefore);

}; // end of class Code


#endif // end of _FLAGS_H_
