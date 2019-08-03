/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <spu_intrinsics.h>
#include "icebatchjob.h"

class Multiplier {
	VF32 m_factor;
public:
	Multiplier(F32 fFactor) : m_factor(spu_splats(fFactor)) {}
	~Multiplier() {}

	void Apply(VF32 &data) { data = spu_mul(data, m_factor); }
};

Multiplier multiplier(2.0f);

extern "C" void BatchJobPluginMain(Ice::BatchJob::DispatcherFunctionArgs params, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap)
{
	U32F numData = (U32F)Ice::BatchJob::ExtractParamU16(params, 0);
	VF32* pData = (VF32*)Ice::BatchJob::ExtractParamPointer(params, 1, memoryMap);

	for (U32F i = 0; i < numData; ++i) {
		multiplier.Apply(pData[i]);
	}
}
