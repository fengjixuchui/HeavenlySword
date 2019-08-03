/***************************************************************************************************
*
* Defines and constants that control the flow of PPU and SPU batch renderer code
*
***************************************************************************************************/

#ifndef BATCHRENDERER_PPU_SPU_H_
#define BATCHRENDERER_PPU_SPU_H_

#include "heresy/heresy_capi.h"

#define BATCH_RENDER_SPU_ELF	"bren_spu_ps3.mod"

// our SPU batch renderer supports different rendering passes..
enum eSpuBatchRendPasses
{
	kRendPassShadowMap = 0,
	kRendPassPreZ,
	kRendPassColorOpaque,
	kRendPassColorTransparent,

	kRendPassUnknown,
};

// that's a list of all the arguments we need to pass to the batch renderer
enum eSpuBatchRendArguments
{
	kBatchRendPixelShadersMem = 0,
	kBatchRendContext,
	kBatchRendPushBufManagement,
	kBatchRendPushBufHeader,
	kBatchRendPushBufPointer,
	kBatchRendInstanceCount,
	kBatchRendMaterialInstanceData,
	kBatchRendPerBatchData,
//	kBatchRendPerBatchData, 

	kBatchRendUnknownArg,
};

struct BatchRenderRenderContext
{
	CMatrix				m_worldToView;						//!< The transform from world space to view space.
	CMatrix				m_worldToScreen;					//!< The transform from world space to screen space.
	CMatrix				m_viewToScreen;						//!< The transform from view space to screen.
	CMatrix				m_shadowMapProjection[4];			//!< The transform's from world space to shadow map space.
	CMatrix				m_SHMatrices[3];
	// 10 Matrices = 40 QWords

	CDirection			m_reflectanceCol;					//!< Reflectance colour
	CDirection			m_keyDirection;						//!< Direction of keylight
	CDirection			m_toKeyLight;						//!< Direction to keylight src
//	KeyLightNode		m_keyLight;							//!< Misc keylight params
	CDirection			m_shadowDirection;					//!< Direction of the shadow
	CDirection			m_sunDirection;						//!< Direction of the sun for atmospheric haze
	CDirection			m_keyColour;						//!< Colour of the keylight
	CDirection			m_skyColour;						//!< Colour of the sky
	// 7 Qwords

	CPoint				m_eyePos;							//!< eyepos 
//	CVector				m_shadowMapParam;					//!< <sm width, sm height, 1 / sm width, 1 / sm height>
	CVector				m_shadowMapPlanesW;					//!< <m_shadowPlanes[1].W, m_shadowPlanes[2].W, m_shadowPlanes[3].W, 0>
	CVector				m_shadowRadii[4];					//!< shadow radii for each section
	CVector				m_gameTime;							//!< <time, 0, 0, 0>
	CVector				m_depthHazeA;						//!< Depth Haze A
	CVector				m_depthHazeG;						//!< Depth Haze G
	CVector				m_depthHazeBeta1PlusBeta2;			//!< Depth Haze b1+b2
	CVector				m_depthHazeOneOverBeta1PlusBeta2;	//!< Depth Haze 1/(b1+b2)
	CVector				m_depthHazeBetaDash1;				//!< Depth Haze b'1
	CVector				m_depthHazeBetaDash2;				//!< Depth Haze b'2
	CVector				m_depthHazeSunColour;				//!< Sun Colour
	CVector				m_ExposureAndToneMapConsts;			//!< Exposure and tone mapping constants
	//11 Qwords

	float				m_fCameraFOVAngle;
	float				m_fScreenAspectRatio;
	float				m_fUnused1;
	float				m_fUnused0;

	CVector				m_RenderTargetSize;
	//1 Qword

	ScissorRegion		m_shadowScissor[4];					//!< used to render 4 maps to a single texture
	//4 Qword


	Heresy_Texture		m_reflectanceMap;					//!< Reflectance map
	Heresy_Texture		m_pShadowMap;						//!< all the registers for the shadow map
	Heresy_Texture		m_pStencilTarget;					//!< all the registers for the stencil target
	Heresy_Texture		m_pIrradianceCache;					//!< all the registers for the irradiance cache
	// 16 Qwords

	// 79 Qwords =  1264 bytes
	void InitContext( void );
};

struct SPUPushBufferManagement
{
	// pushbuffer address and size
	// at the moment, we assume uncontended access to our chunk of pushbuffer
	// so the main program must allocate these pushbuffers exclusively for this spu
	// its also responsable for issuing a pushbuffer call at the appropiate place
	// the SPU is responsible for a returning GPU ret 
	// NOTE: this is streamed out, so does not have to fit into SPU RAM

	// note these are EA pushbuffers not LS ones, so address should be main ram
	Heresy_PushBuffer	m_pOutPushBuffer;
	Heresy_GlobalDataT	m_HeresyGlobal;
};

#define FRAME_FLAG_MASK 0xFFFF0000
#define BONE_COUNT_MASK 0x0000FFFF


struct BatchData
{
	CMatrix obReconstructionMatrix;
	uint32_t BoneCount;
	uint32_t renderingPass;
	void* pBoneIndices;
	bool bApplyPosReconstMatrix;
	bool bCollapseReconstMatrix;
	bool bReUsePixelShader;
	bool bJumpPreDraw;
	bool bJumpPostDraw;
};

struct MaterialInstanceData
{
	CMatrix obObjectToWorld;
	uint32_t ui32FrameFlag;
	void* pSkinMatrixArray;
};

//	9 QWords -> 144 bytes

// Push buffers size

#define MAX_BATCH_RENDER_JOBS_PER_FRAME 1024

class DependencyCounter;
class SPUTask;

// that's just a stupid container class
class BatchRenderer 
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
	static DependencyCounter*			m_pBatchesDependency;

	static uint32_t						*m_pPushBuffer;
	static uint32_t						m_uiDependencyCounter;

	static MaterialInstanceData			*m_pInstanceData;	
	static BatchRenderRenderContext		*m_pContext;
	static SPUPushBufferManagement		*m_pSpuPBManagement;
	const static Heresy_PushBuffer		*m_pPushBufferHeader;

	static uintptr_t					m_pJobsMemory;
	static DoubleEnderFrameAllocatorC	m_pDoubleBufferedJobsMem;

};

#endif // BATCHRENDERER_PPU_SPU_H_

