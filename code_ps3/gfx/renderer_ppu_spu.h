/***************************************************************************************************
*
* Defines and constants that control the flow of PPU and SPU batch renderer code
*
***************************************************************************************************/

#ifndef RENDERER_PPU_SPU_H_
#define RENDERER_PPU_SPU_H_

#include "gfx/batchrender_ppu_spu.h"
#include "heresy/heresy_capi.h"

#define RENDERER_SPU_ELF	"renderer_spu_ps3.mod"

// that's a list of all the arguments we need to pass to the batch renderer
enum eSpuRendArguments
{
	kRendContext = 0,
	kRendPushBufManagement,
	kRendGlobalData,
	kRendMeshesData,

	kRendUnknownArg,
};

struct RendererData
{
	uint32_t renderingPass;
	uint32_t meshCount;
	uint32_t* pDestPixelShaders;
};

struct RendererMaterialInstanceData
{
	CMatrix obObjectToWorld;
	CMatrix obReconstructionMatrix;
	void* pBoneIndices;
	void* pSkinMatrixArray;
	const void* pPushBufferHeader;
	uint32_t ui32PushBufferHeaderSize;
	void* pPushBuffer;
	uint32_t ui32PushBufferSize;
	uint32_t ui32FrameFlag;
	uint32_t BoneCount;
	bool bApplyPosReconstMatrix;
	bool bCollapseReconstMatrix;
};

#ifndef _RELEASE
#define MAX_MESHES_PER_JOB 384	
#else
#define MAX_MESHES_PER_JOB 512
#endif

class DependencyCounter;
class SPUTask;

// that's just a stupid container class
class SpuRenderer 
{
public:
	static void Initialise(void);
	static void Destroy(void);
	static void PrePresent(void);
	static void PostPresent(void);
	static void Render( void* meshInstanceVector, eSpuBatchRendPasses renderingPass );

private:
	static bool							m_bInitialized;
	static SPUTask*						m_pRendererTask;
	static uint32_t						m_uiDependencyCounter;
	static uintptr_t					m_pJobsMemory;
	static DoubleEnderFrameAllocatorC	m_pDoubleBufferedJobsMem;
};

#endif // RENDERER_PPU_SPU_H_

