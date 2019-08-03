#include "listspace_ps3.h"
#include "gfx/listspace.h"

#include "gfx/rendercontext.h"
#include "gfx/renderersettings.h"
#include "gfx/renderable.h"
#include "anim/transform.h"

#include "core/frustum.h"

// SPU-exec related includes
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/sputask_ps3.h"

// Structures for data exchange between PPU and SPU
#include "spu/clipper/clipper_dmadata.h"

// profiling gatso
#include "core/gatso.h"

using namespace ClipperData;

namespace 
{
	const char ClipperElf[] = "clipper_spu_ps3.mod";


//void PrintMatrix(const char* name, const float* data)
//{
//    ntPrintf("%s : \n", name);
//    for (unsigned int row = 0; row < 4; ++ row)
//    {
//        ntPrintf("%f %f %f %f\n", data[row * 4 + 0], data[row * 4 + 1], data[row * 4 + 2], data[row * 4 + 3]);
//    }
//}
//
//void PrintPlane(const CPlane& plane)
//{
//	ntPrintf("x: %f y: %f z: %f d: %f\n", plane.GetNormal().X(), plane.GetNormal().Y(), plane.GetNormal().Z(), plane.GetDistance());
//}
//
//void PrintFrustum(const char* name, CullingFrustum& frustum)
//{
//	ntPrintf("%s :\n", name);
//	for (unsigned int plane = 0; plane < 6; ++ plane)
//	{
//		ntPrintf("plane[%i] : ", plane);
//		PrintPlane(*frustum.GetPlane(plane));
//	}
//}
//
//void PrintPoint(const CPoint& pnt)
//{
//	ntPrintf("x: %f y: %f z: %f \n", pnt.X(), pnt.Y(), pnt.Z());
//
//}
//
//void PrintAABB(const char* name, CAABB& aabb)
//{
//	ntPrintf("%s : \n", name);
//
//	ntPrintf("Min : ");
//	PrintPoint(aabb.Min()); 
//	ntPrintf("Max : ");
//	PrintPoint(aabb.Max());
//}

}


union MakePpuPtr
{
	MakePpuPtr(uint32_t in) 
	{
		m_ptr32[1] = in;	m_ptr32[0] = 0;
	}

	template <typename PtrType>
	operator PtrType*() const
	{
		return (PtrType*)m_ptr64;
	}

	uint64_t m_ptr64;
	uint32_t m_ptr32[2];
};

union MakeSpuPtr
{
	template <typename PtrType>
	MakeSpuPtr(const PtrType in) : m_ptr64((uint64_t)in) {}

	operator uint32_t() const
	{
		return m_ptr32[1];
	}


	uint64_t m_ptr64;
	uint32_t m_ptr32[2];
};
struct DMATransfer
{
	static const uint64_t s_sentinel = (uint64_t)-1;

	static bool IsDMATransferComplete(void* startAddress, size_t byteSize)
	{
		uint64_t*	addr = (uint64_t*)startAddress;
		if (s_sentinel == addr[0])
		{
			return false;
		}

		if (s_sentinel == addr[(byteSize >> 3) - 1])
		{
			return false;
		}

		return true;
	}

	static void PrepareNewDmaTransfer(void* startAddress, size_t byteSize)
	{
		uint64_t*	addr			=	(uint64_t*)startAddress;
		addr[0]						=	s_sentinel;
		addr[(byteSize >> 3) - 1]	=	s_sentinel;
	}
};

class ListSpaceClipper : public IFrustumClipper
{
private:
	static const unsigned int s_tasksPerBatch = 5;

    friend IFrustumClipper*   CreateClipper(ListSpace* listSpace);
    friend void               DestroyClipper(IFrustumClipper* clipper);

private:

	struct Task
	{
		Task() : m_currIndex(0)
		{
			size_t actualSize;
			m_input = (DMA_In*)DMABuffer::DMAAlloc(sizeof(DMA_In), &actualSize);
			m_inputDmaBuffer	= DMABuffer(m_input, actualSize);
			m_output = (DMA_Out*)DMABuffer::DMAAlloc(sizeof(DMA_Out), &actualSize);
			m_outputDmaBuffer	= DMABuffer(m_output, actualSize);
		}
		~Task()
		{
			DMABuffer::DMAFree(m_input);
			DMABuffer::DMAFree(m_output);
		}

		RenderableIn& AddInput()
		{
			if (m_currIndex >= s_renderablesPerTask)
			{
				ntAssert_p(m_currIndex < s_renderablesPerTask, ("Too many renderables"));
				--m_currIndex;
			}

			return	m_input -> m_renderables[m_currIndex ++];
		}

		bool IsEmpty()
		{
			return 0 == m_currIndex;
		}

		DMABuffer		m_inputDmaBuffer;
		DMABuffer		m_outputDmaBuffer;
		DMA_In*			m_input;
		DMA_Out*		m_output;

		uint32_t		m_currIndex;
	};

private:
    ListSpaceClipper(ListSpace* parent) : m_parent(parent)
	{
		ElfManager::Get().Load(ClipperElf);
        ntAssert(m_parent);
	}

	~ListSpaceClipper()
	{
	}

public:
    void SetVisibleFrustum( const float* pShadowPercents )
    {

	    CGatso::Start( "ListSpace::SetVisibleFrustum" );
	    CGatso::Start( "ListSpace::SetVisibleFrustum.Prolog" );

	    CMatrix const& viewMatrix = RenderingContext::Get()->m_worldToView;	
        ListSpace::RenderableListType   const&  renderables = m_parent -> GetRenderableList();

        CAABB  AABBs[NUM_AABBS];

        for (unsigned int frustum = 0; frustum < NUM_AABBS; ++ frustum)
        {
            AABBs[frustum] = CAABB(CONSTRUCT_INFINITE_NEGATIVE);
        }

	    // calculate shadow clipping planes
	    if( pShadowPercents != 0  )
	    {
		    const CCamera* pCamera = RenderingContext::Get()->m_pCullCamera;
		    const float zdist = pCamera->GetZFar() - pCamera->GetZNear();
		    float zs[NUM_SHADOW_PLANES];
		    for( unsigned int i=0; i < NUM_SHADOW_PLANES; i ++ )
		    {
			    zs[i] = pCamera -> GetZNear() + zdist * pShadowPercents[i] * 0.01f;
			    RenderingContext::Get()->m_shadowPlanes[i] = CVector( 0, 0, -1, zs[i] );
		    }
	    }

	    SetData(pShadowPercents);

	    // Erase should be preffered over clear as it does not deallocate memory
	    RenderingContext::Get()->m_aVisibleRenderables.erase(RenderingContext::Get()->m_aVisibleRenderables.begin(), RenderingContext::Get()->m_aVisibleRenderables.end());
	    RenderingContext::Get()->m_aShadowCastingRenderables.erase(RenderingContext::Get()->m_aShadowCastingRenderables.begin(), RenderingContext::Get()->m_aShadowCastingRenderables.end());

	    CGatso::Stop( "ListSpace::SetVisibleFrustum.Prolog" );

	    CGatso::Start( "ListSpace::SetVisibleFrustum.Main" );
	    if( CRendererSettings::bEnableCulling )
	    {
            ListSpace::RenderableListType::const_iterator obIt	= renderables.begin();

		    unsigned int numTasks = s_tasksPerBatch;
		    uint32_t	allTasksReady = (1 << numTasks) - 1;
		    uint32_t	availableTasks = allTasksReady;
            unsigned short  renderableIndex = 0;
		    while(obIt != renderables.end() || availableTasks != allTasksReady)
		    {
			    // Start new tasks
			    for (unsigned int task = 0; task < numTasks; ++task)
			    {
				    // Add renderables for process
				    unsigned int numElements = 0;
				    if (availableTasks & (1 << task) && obIt != renderables.end())
				    {
					    while (obIt != renderables.end() &&  numElements < s_renderablesPerTask)
					    {
						    CRenderable* pobRenderable = *obIt;

							pobRenderable -> m_FrameFlags = 0;

						    if( pobRenderable->IsRendering() || pobRenderable->IsShadowCasting() )
						    {
                                AddRenderable(task, renderableIndex);
							    ++numElements;
						    }
                            ntAssert(renderableIndex < renderables.size());
						    ++obIt;
                            ++renderableIndex;
					    }
    					
					    // Start SPU task
 					    if ( LaunchTask(task) )
						{
							availableTasks ^= (1 << task); // set task to busy state
						}

				    }
			    }

			    CGatso::Start( "ListSpace::SetVisibleFrustum.Stall" );
			    // Check if any data has arrived back from SPUs
			    for (unsigned int task = 0; task < numTasks; ++task)
			    {
				    if (!(availableTasks & ((1 << task))))
				    {
					    if(ProcessTask(task, AABBs))
					    {
						    availableTasks ^= (1 << task); // set task to available
					    }
				    }
			    }
			    CGatso::Stop( "ListSpace::SetVisibleFrustum.Stall" );
		    }

	    }
	    else
	    {
            for( ListSpace::RenderableListType::const_iterator obIt = renderables.begin(); obIt != renderables.end(); ++obIt )
		    {
			    CRenderable* pobRenderable = *obIt;
			    if( pobRenderable->IsRendering() )
			    {
				    // no culling so everything is visible (no shadows either)
				    RenderingContext::Get()->m_aVisibleRenderables.push_back( pobRenderable );
				    pobRenderable->m_FrameFlags =	CRenderable::RFF_VISIBLE;
			    }
		    }
	    }

	    CGatso::Stop( "ListSpace::SetVisibleFrustum.Main" );
	    CGatso::Start( "ListSpace::SetVisibleFrustum.Epilogue" );

	    if( pShadowPercents != 0 )
	    {
    #if !defined(SHADOW_AABB_VIEW)
            for (unsigned int aabb = 0; aabb < NUM_AABBS; ++ aabb)
            {
                AABBs[aabb].Transform(viewMatrix);
            }
    #endif

            for ( unsigned int casterAABB = 0; casterAABB < NUM_CASTER_AABB; ++ casterAABB )
            {
                RenderingContext::Get()->m_shadowCasterAABB[casterAABB] = AABBs[casterAABB];
            }
            for ( unsigned int frustumAABB = 0; frustumAABB < NUM_FRUSTUM_AABB; ++ frustumAABB )
            {
                RenderingContext::Get()->m_shadowFrustumAABB[frustumAABB] = AABBs[NUM_CASTER_AABB + frustumAABB ];
            }
	    }

        //for (int i = 0; i < 4; ++i)
        //{
        //    ntPrintf("CasterAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowCasterAABB[i].Min().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Z(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Z());
        //    ntPrintf("FrustrumAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowFrustumAABB[i].Min().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Z(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Z());
        //}

        //ntPrintf("Visibles: %i, ShadowCasters: %i \n", RenderingContext::Get()->m_aVisibleRenderables.size(), RenderingContext::Get()->m_aShadowCastingRenderables.size());

	    CGatso::Stop( "ListSpace::SetVisibleFrustum.Epilogue" );
	    CGatso::Stop( "ListSpace::SetVisibleFrustum" );

    }

	//void AddRenderable(unsigned int task, const CRenderable* obj)
    void AddRenderable(unsigned int task, unsigned int renderableIndex)
	{
		CGatso::Start( "ListSpace::AddRenderable" );

        ListSpace::RenderableListType   const&  renderables = m_parent -> GetRenderableList();

#ifndef	_RELEASE
		if (renderableIndex >= renderables.size())
		{
			ntBreakpoint();  // exception crash check
		}
#endif

		CRenderable* obj = renderables.at(renderableIndex);

		unsigned short flags = rifNoFlags;        
		if (obj -> IsShadowCasting())
		{
			flags |= (1 << rifCastShadow);
		}
        if (obj -> IsShadowRecieving())
		{
			flags |= (1 << rifReceiveShadow);
		}
		if (obj -> IsRendering())
		{
			flags |= (1 << rifRendering);
		}

		if( flags != rifNoFlags )
		{
			RenderableIn& newSlot = m_batchedTasks[task].AddInput();

			ntAssert(renderableIndex < 65535);

#ifndef CLIPPER_DMA_FROM_SPU
			newSlot.m_renderableID = (unsigned short)renderableIndex;
			CAABB const& AABB = obj -> GetWorldSpaceAABB();
			newSlot.m_AABB.Min()	  = AABB.Min();
			newSlot.m_AABB.Max()	  = AABB.Max();
#else
			newSlot.m_renderableID = (uint32_t)&obj -> GetBounds();
			newSlot.m_transformAddr = reinterpret_cast<uint32_t>(&const_cast<CMatrix&>(obj -> GetTransform() -> GetWorldMatrixFast()));
#endif
			//newSlot.m_renderableID = MakeSpuPtr(obj);
			ntAssert(renderableIndex < renderables.size());

			newSlot.m_flags           = flags;
		}

		CGatso::Stop( "ListSpace::AddRenderable" );

	}

	bool LaunchTask(unsigned int task)
	{
		CGatso::Start( "ListSpace::LaunchTask" );
		if (m_batchedTasks[task].IsEmpty())
		{                                                                     
			CGatso::Stop( "ListSpace::LaunchTask" );

			return false;
		}

		m_batchedTasks[task].m_input -> m_controlData.m_numRenderables = m_batchedTasks[task].m_currIndex;

		SPUTask spu_task( ElfManager::Get().GetProgram( ClipperElf ) );

		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, m_batchedTasks[task].m_inputDmaBuffer ), INPUT_SLOT);
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_OutputOnly, m_batchedTasks[task].m_outputDmaBuffer ), OUTPUT_SLOT);

		DMATransfer::PrepareNewDmaTransfer(m_batchedTasks[task].m_output, sizeof(*m_batchedTasks[task].m_output));

		Exec::RunTask( &spu_task );

		CGatso::Stop( "ListSpace::LaunchTask" );

		return true;
	}

	inline CRenderable* GetRenderableFromID(uint32_t id)
	{
		const CRenderable*	obj = reinterpret_cast<const CRenderable*>(4);
		const CAABB*		member = &obj -> m_obBounds;
		uint32_t			offset = (uint32_t)member - (uint32_t)obj;

		return (CRenderable*)(id - offset);
	}

	/// Returns true if the task has been successfully processed or false if the data has not arrived back from SPU yet
	bool ProcessTask(unsigned int task, CAABB  (&AABBs)[NUM_FRUSTUM_AABB + NUM_CASTER_AABB])
	{
		CGatso::Start( "ListSpace::ProcessTask" );
		if(!DMATransfer::IsDMATransferComplete(m_batchedTasks[task].m_output, sizeof(*m_batchedTasks[task].m_output)))
		{
			CGatso::Stop( "ListSpace::ProcessTask" );
			return false;
		}

#ifndef CLIPPER_DMA_FROM_SPU
        ListSpace::RenderableListType   const&  renderables = m_parent -> GetRenderableList();
#endif

		for (unsigned int visible = 0; visible < m_batchedTasks[task].m_output -> m_header.m_sizeVisible; ++ visible)
		{
#ifndef CLIPPER_DMA_FROM_SPU
            ntAssert(m_batchedTasks[task].m_output -> m_visibles[visible].m_renderableID < renderables.size())

//#ifndef	_RELEASE
//			if (m_batchedTasks[task].m_output -> m_visibles[visible].m_renderableID >= renderables.size())
//			{
//				ntBreakpoint();  // exception crash check
//			}
//#endif

            CRenderable* pobRenderable = renderables.at(m_batchedTasks[task].m_output -> m_visibles[visible].m_renderableID);
#else
			CRenderable* pobRenderable = GetRenderableFromID(m_batchedTasks[task].m_output -> m_visibles[visible].m_renderableID);
#endif
            ntAssert(pobRenderable);

            pobRenderable -> m_FrameFlags = m_batchedTasks[task].m_output -> m_visibles[visible].m_flags;
            if (pobRenderable -> IsRendering())
            {
				RenderingContext::Get()->m_aVisibleRenderables.push_back( pobRenderable );
            }

		}

		for (unsigned int shadowCaster = 0; shadowCaster < m_batchedTasks[task].m_output -> m_header.m_sizeShadowcaster; ++ shadowCaster)
		{
#ifndef CLIPPER_DMA_FROM_SPU
            ntAssert(m_batchedTasks[task].m_output -> m_shadowCasters[shadowCaster].m_renderableID < renderables.size())

//#ifndef	_RELEASE
//			if (m_batchedTasks[task].m_output -> m_shadowCasters[shadowCaster].m_renderableID >= renderables.size())
//			{
//				ntBreakpoint();  // exception crash check
//			}
//#endif

            CRenderable* pobRenderable = renderables.at(m_batchedTasks[task].m_output -> m_shadowCasters[shadowCaster].m_renderableID);
#else
			CRenderable* pobRenderable = GetRenderableFromID(m_batchedTasks[task].m_output -> m_shadowCasters[shadowCaster].m_renderableID);
#endif

            ntAssert(pobRenderable);

            pobRenderable -> m_FrameFlags = m_batchedTasks[task].m_output -> m_shadowCasters[shadowCaster].m_flags;

			RenderingContext::Get()->m_aShadowCastingRenderables.push_back( pobRenderable );
		}

        for (unsigned int frustum = 0; frustum < NUM_FRUSTUM_AABB + NUM_CASTER_AABB; ++ frustum)
        {
            AABBs[frustum].Union(m_batchedTasks[task].m_output -> m_header.m_frustums[frustum]);
        }


		m_batchedTasks[task].m_currIndex = 0;

		CGatso::Stop( "ListSpace::ProcessTask" );

		return true;
	
	}

	void SetData(float const* shadowPercents)
	{
		if( shadowPercents != 0  )
		{
			const CCamera* pCamera = RenderingContext::Get()->m_pCullCamera;

			for (unsigned int task = 0; task < s_tasksPerBatch; ++ task)
			{
				ClipperDataIn& dmaHeader = m_batchedTasks[task].m_input -> m_controlData;

				dmaHeader.m_worldToView = RenderingContext::Get()->m_worldToView;	
				dmaHeader.m_viewToScreen = RenderingContext::Get()->m_viewToScreen;
				dmaHeader.m_worldToScreen = RenderingContext::Get()->m_worldToScreen;
				dmaHeader.m_ZNear = pCamera->GetZNear();
				dmaHeader.m_ZFar  = pCamera->GetZFar();
				dmaHeader.m_aspectRatio = RenderingContext::Get()->m_fScreenAspectRatio;
				dmaHeader.m_FOV	  = pCamera->GetFOVAngle();
                dmaHeader.m_shadowDirection = RenderingContext::Get()->m_shadowDirection;

				for (unsigned int shadowPlane = 0; shadowPlane < NUM_SHADOW_PLANES; ++ shadowPlane)
				{
					dmaHeader.m_shadowPercents[shadowPlane] = shadowPercents[shadowPlane];
				}

                dmaHeader.m_shadowQuality = CRendererSettings::iShadowQuality;
			}

		}
		else
		{
			for (unsigned int task = 0; task < s_tasksPerBatch; ++ task)
			{
				ClipperDataIn& dmaHeader = m_batchedTasks[task].m_input -> m_controlData;

				dmaHeader.m_worldToView = RenderingContext::Get()->m_worldToView;	
				dmaHeader.m_viewToScreen = RenderingContext::Get()->m_viewToScreen;
				dmaHeader.m_worldToScreen = RenderingContext::Get()->m_worldToScreen;
                dmaHeader.m_shadowQuality = 0;
			}

		}
	}


private:
    Task	    m_batchedTasks[s_tasksPerBatch];
    ListSpace*  m_parent;

};

IFrustumClipper*   CreateClipper(ListSpace* listSpace)
{
    return NT_NEW_CHUNK( Mem::MC_GFX ) ListSpaceClipper(listSpace);
}
void                DestroyClipper(IFrustumClipper* clipper)
{
    NT_DELETE_CHUNK( Mem::MC_GFX, clipper);
}
