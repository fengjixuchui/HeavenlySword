//--------------------------------------------------------------------------------------------------
/**
	@file		GpSpuBlendData.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SPU_BLEND_DATA_H
#define GP_SPU_BLEND_DATA_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcStreamBuffer.h>
#include <jobapi/spumodule.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpSubMeshDef;
class MultiThreadSafeJobList;

//--------------------------------------------------------------------------------------------------
/**
	@class			GpSpuBlendData

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpSpuBlendData
{
public:
	GpSpuBlendData();
	~GpSpuBlendData();

	static  void Initialise();
	static  void Shutdown();

	void	Create(const GpSubMeshDef* pSubMeshDef);
	void	Update(const GcStreamBufferHandle& hStream, const float* pWeights, MultiThreadSafeJobList& jobList);

private:
	static const u32 kMaxVertsPerChunk = 1024;

	struct JobParams
	{
		u32		m_eaOutputAddr;
		u32		m_eaDeltaBufferAddr;
		u32		m_vert8Count;
		u32		m_targetCount;
		u32		m_deltaBufferSize;
		float	m_scale;
	} WWSJOB_ALIGNED(16);

	static SpuModuleHandle		ms_jobModule;

	int							m_vertCount;
	int							m_chunkCount;
	u32							m_targetCount;
	float						m_scale;
	int							m_outputBufferAddr;
	int							m_weightBufferAddr;
	float*						m_pVerts;
	short*						m_pDeltas;
	u32							m_commandListBufferSize;
	u8*							m_pCommandListBuffer;
};

#endif //GP_SPU_BLEND_DATA_H

