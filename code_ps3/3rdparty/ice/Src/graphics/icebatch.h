/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 8/23/05 
 */
 
#ifndef ICEBATCH_H
#define ICEBATCH_H

namespace Ice
{
	namespace Bucketer
	{
		void BatchDiscretePmObject( 
			Ice::Graphics::DiscretePmObject const *object, 
			SMath::Transform *transforms,
			F32 const distance);
		void BatchDiscretePmObjects( 
			Ice::Graphics::DiscretePmObject const *object, 
			U64 const numObjects, 
			SMath::Transform **transformsList,
			F32 const *distances, 
			U64 *partEnableBits);
		void BatchDiscretePmObjects( 
			Ice::Graphics::DiscretePmObject const *object, 
			U64 const numObjects, 
			SMath::Transform **transformsList,
			F32 const *distances);
		void BatchContinuousPmObject(const Ice::Graphics::ContinuousPmObject *object);

		void RenderBatches(const U64 variation);
		void RenderBatchesInSphere(const U64 variation, const SMath::Vec4 &sphere);
		void ClearBatches();
	}
}

#endif // ICEBATCH_H
