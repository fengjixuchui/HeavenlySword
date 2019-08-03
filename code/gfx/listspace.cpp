//--------------------------------------------------
//!
//!	\file listspace.cpp
//!	A simple space that internally just holds unsorted lists.
//!
//--------------------------------------------------

#include "gfx/listspace.h"
#include "gfx/camera.h"
#include "gfx/renderer.h"
#include "gfx/renderable.h"
#include "gfx/meshinstance.h"


#include "core/gatso.h"
#include "core/frustum.h"

#include "anim/transform.h"

#include "core/visualdebugger.h"

#ifdef  PLATFORM_PS3
#define USE_SPU_CLIPPER
#include "gfx/listspace_ps3.h"
#endif

//#define TEST_CLIPPER

namespace 
{
    const unsigned int MaxRenderables = 400;

}

/***************************************************************************************************
*
*	FUNCTION		ListSpace::ListSpace
*
*	DESCRIPTION		Reserves enough space in a vector, creates platform-specific implementation
*
***************************************************************************************************/
ListSpace::ListSpace()
#ifdef USE_SPU_CLIPPER
: m_clipperImpl(CreateClipper(this))
#else
: m_clipperImpl(NULL)
#endif
{
    // TODO: find out how many renderables will we actually have
    m_obRenderables.reserve(MaxRenderables);

}


/***************************************************************************************************
*
*	FUNCTION		ListSpace::~ListSpace
*
*	DESCRIPTION		
*
***************************************************************************************************/
ListSpace::~ListSpace()
{
	ntAssert(m_obRenderables.empty());
	ntAssert(m_obCallbackList.empty());
#ifdef USE_SPU_CLIPPER
	DestroyClipper(m_clipperImpl);
#endif

}


/***************************************************************************************************
*
*	FUNCTION		ListSpace::AddRenderable
*
*	DESCRIPTION		Adds a renderable to the space.
*
***************************************************************************************************/

void ListSpace::AddRenderable( CRenderable* pobRenderable )
{
	ntAssert( ntstd::find( m_obRenderables.begin(), m_obRenderables.end(), pobRenderable ) == m_obRenderables.end() );
	m_obRenderables.push_back( pobRenderable );

    IFrameFlagsUpdateCallback*  callBack = pobRenderable -> GetUpdateFrameFlagsCallback();
    if ( callBack )
    {
        m_obCallbackList.push_back(callBack);
    }
}

/***************************************************************************************************
*
*	FUNCTION		ListSpace::RemoveRenderable
*
*	DESCRIPTION		Removes a renderable from the space.
*
***************************************************************************************************/

void ListSpace::RemoveRenderable( CRenderable* pobRenderable )
{
    RenderableListType::iterator iter = ntstd::find( m_obRenderables.begin(), m_obRenderables.end(), pobRenderable );
    ntAssert(iter != m_obRenderables.end());
    if (iter != m_obRenderables.end())
    {
	    m_obRenderables.erase( iter );

        IFrameFlagsUpdateCallback*  callBack = pobRenderable -> GetUpdateFrameFlagsCallback();
        if ( callBack )
        {
            CallbackListType::iterator  cb_iter = ntstd::find( m_obCallbackList.begin(), m_obCallbackList.end(), callBack );
            ntAssert(cb_iter != m_obCallbackList.end());

            m_obCallbackList.erase(cb_iter);
        }
    }
}

//#define SHADOW_AABB_VIEW 1

void ListSpace::SetVisibleFrustum( const float* pShadowPercents ) const
{
    ntstd::for_each(m_obCallbackList.begin(), m_obCallbackList.end(), ntstd::mem_fun(&IFrameFlagsUpdateCallback::PreUpdate));
    SetVisibleFrustumImpl(pShadowPercents);
    ntstd::for_each(m_obCallbackList.begin(), m_obCallbackList.end(), ntstd::mem_fun(&IFrameFlagsUpdateCallback::PostUpdate));

    //if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F8 ) )
    //{
    //    ntPrintf("----------------------------------------------------------------------\n");
    //    float* viewMatrix = RenderingContext::Get()->m_worldToView;	
    //    float* projMatrix = RenderingContext::Get()->m_viewToScreen;
    //    float* fullMatrix = RenderingContext::Get()->m_worldToScreen;
    //    const CCamera* pCamera = RenderingContext::Get()->m_pCullCamera;
    //    float zNear = pCamera->GetZNear();
    //    float zFar = pCamera->GetZFar();
    //    ntPrintf("Near : %f Far : %f\n", zNear, zFar);
    //    PrintMatrix("View", viewMatrix);
    //    PrintMatrix("Proj", projMatrix);
    //    PrintMatrix("Full", fullMatrix);
    //    for (int i = 0; i < 4; ++i)
    //    {
    //        ntPrintf("CasterAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowCasterAABB[i].Min().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Z(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Z());
    //        ntPrintf("FrustrumAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowFrustumAABB[i].Min().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Z(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Z());
    //    }

    //    ntPrintf("Visibles: %i, ShadowCasters: %i \n", RenderingContext::Get()->m_aVisibleRenderables.size(), RenderingContext::Get()->m_aShadowCastingRenderables.size());
    //    ntPrintf("----------------------------------------------------------------------\n");
    //}

}

//#ifdef TEST_CLIPPER

#ifndef _GOLD_MASTER
unsigned int CountVisibles(const ListSpace* list)
{
	unsigned int numVisibles = 0;
	int numTotalSpeedGrass = 0;
	for ( ListSpace::RenderableListType::const_iterator iter = list -> GetRenderableList().begin();
		iter != list -> GetRenderableList().end(); ++ iter )
	{
		if ((*iter) -> GetRenderableType() == CRenderable::RT_SPEED_GRASS && (*iter) ->  IsRendering())
		{
			numTotalSpeedGrass ++;

			CAABB aabb = (*iter) -> GetWorldSpaceAABB();
			g_VisualDebug -> RenderAABB(aabb.Min(), aabb.Max(), 0xff0000ff, 0);
		}
		if ((*iter) ->  IsRendering() && ((*iter) -> m_FrameFlags & CRenderable::RFF_VISIBLE) && ((*iter) -> GetRenderableType() == CRenderable::RT_SPEED_GRASS))
		{
			numVisibles ++;
		}
	}

	char buffer[1024];
	snprintf(buffer, 1024, "Num Total SpeedGrass : %d", numTotalSpeedGrass);
	g_VisualDebug -> Printf2D(800, 90, 0xffffffff, 0, buffer);


	return numVisibles;
}
#endif
//#endif

typedef RenderingContext::CONTEXT_DATA::RenderableVector RVector;

void ListSpace::SetVisibleFrustumImpl( const float* pShadowPercents ) const
{
#ifdef USE_SPU_CLIPPER
    ntAssert(m_clipperImpl);
    m_clipperImpl -> SetVisibleFrustum( pShadowPercents );

	//unsigned int numSPU = CountVisibles(this);

	//char buffer[1024];
	//snprintf(buffer, 1024, "SPU : %d", numSPU);
	//g_VisualDebug -> Printf2D(200, 20, 0xffffffff, 0, buffer);


#ifdef TEST_CLIPPER
	static int diff = 0;


	RVector	backupVector(RenderingContext::Get()->m_aVisibleRenderables.begin(), RenderingContext::Get()->m_aVisibleRenderables.end());

	unsigned int numSPU = CountVisibles(this);

	ntAssert(numSPU == RenderingContext::Get()->m_aVisibleRenderables.size());

	char buffer[1024];
	snprintf(buffer, 1024, "SPU : %d", numSPU);
	g_VisualDebug -> Printf2D(20, 20, 0xffffffff, 0, buffer);

	SetVisibleFrustumImplPPU(pShadowPercents);

	unsigned int numPPU = CountVisibles(this);
    	
	snprintf(buffer, 1024, "PPU : %d", numPPU);
	g_VisualDebug -> Printf2D(20, 50, 0xffffffff, 0, buffer);

	diff = numSPU - numPPU;

	if (numSPU - numPPU != 0)
	{
		//diff = numSPU - numPPU;

		std::sort(backupVector.begin(), backupVector.end());
		std::sort(RenderingContext::Get()->m_aVisibleRenderables.begin(), RenderingContext::Get()->m_aVisibleRenderables.end());
		bool doesInclude = std::includes(backupVector.begin(), backupVector.end(), RenderingContext::Get()->m_aVisibleRenderables.begin(), RenderingContext::Get()->m_aVisibleRenderables.end());

		RVector	diffVector;

		if (doesInclude)
		{
			std::set_difference(backupVector.begin(), backupVector.end(), RenderingContext::Get()->m_aVisibleRenderables.begin(), RenderingContext::Get()->m_aVisibleRenderables.end(), back_inserter(diffVector));

			unsigned int count = 0;
			for (RVector::iterator iter = diffVector.begin(); iter != diffVector.end(); ++ iter)
			{
				CAABB aabb = (*iter) -> GetWorldSpaceAABB();
				ntPrintf("%d) Min: %f %f %f   ----  Max: %f %f %f\n",
					count ++, aabb.Min().X(), aabb.Min().Y(), aabb.Min().Z(),
					aabb.Max().X(), aabb.Max().Y(), aabb.Max().Z());
}

		}


	}
	snprintf(buffer, 1024, "Diff : %d", diff);
	g_VisualDebug -> Printf2D(20, 80, 0xff0000ff, 0, buffer);

	RenderingContext::Get()->m_aVisibleRenderables.resize(0);
	RenderingContext::Get()->m_aVisibleRenderables.insert(RenderingContext::Get()->m_aVisibleRenderables.begin(), backupVector.begin(), backupVector.end());

#endif
#else
	SetVisibleFrustumImplPPU(pShadowPercents);

	//unsigned int numPPU = CountVisibles(this);

	//char buffer[1024];
	//snprintf(buffer, 1024, "Num SpeedGrass : %d", numPPU);
	//g_VisualDebug -> Printf2D(800, 20, 0xffffffff, 0, buffer);

	//snprintf(buffer, 1024, "Total size : %d", m_obRenderables.size());
	//g_VisualDebug -> Printf2D(800, 50, 0xffffffff, 0, buffer);


#endif
	
}

void ListSpace::SetVisibleFrustumImplPPU( const float* pShadowPercents ) const
{
#if !defined(USE_SPU_CLIPPER) || defined(TEST_CLIPPER)

	CGatso::Start( "ListSpace::SetVisibleFrustum(OLD)" );

	static const float sm0MinSplitDistance = 1.f;
	static const float sm1MinSplitDistance = 10.f;
	static const float sm2MinSplitDistance = 30.f;
	//static const float sm3MinSplitDistance = 60.f;

	CMatrix viewMatrix = RenderingContext::Get()->m_worldToView;	

	// collide the frustum with each shadowlight's projected casters (generates silhouette images)
	
	CullingFrustum frustum( RenderingContext::Get()->m_worldToScreen );

	CVector* pShadowPlanes = 0;
	CullingFrustum aShadowFrustums[4];
	CullingFrustum* pShadowFrustums = 0;
	// calculate shadow clipping planes
	if( pShadowPercents != 0  )
	{
		const CCamera* pCamera = RenderingContext::Get()->m_pCullCamera;
		const float zdist = pCamera->GetZFar() - pCamera->GetZNear();
		float zs[5];
		for(int i=0;i < 5;i++)
		{
			zs[i] = pCamera->GetZNear() + zdist * (pShadowPercents[i] / 100.f);
			CVector shad0 = CVector( 0, 0,zs[i], 1.f) * RenderingContext::Get()->m_viewToScreen;

#ifdef PLATFORM_PS3
			RenderingContext::Get()->m_shadowPlanes[i] = CVector( 0, 0, -1, zs[i] );
#else
			RenderingContext::Get()->m_shadowPlanes[i] = CVector( 0, 0, -1, shad0.Z() / shad0.W() );
#endif
		}

		for(int i=0;i < 4;i++)
		{
			CMatrix projMatrix;
			projMatrix.Perspective( pCamera->GetFOVAngle(), RenderingContext::Get()->m_fScreenAspectRatio, zs[i+0], zs[i+1] );
			aShadowFrustums[i] = CullingFrustum( viewMatrix * projMatrix );
		}
		pShadowPlanes = RenderingContext::Get()->m_shadowPlanes;
		pShadowFrustums = aShadowFrustums;
	}

	// erase the contents of the visible list
	RenderingContext::Get()->m_aVisibleRenderables.clear();
	RenderingContext::Get()->m_aShadowCastingRenderables.clear();

	CAABB casterAABB0( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB casterAABB1( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB casterAABB2( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB casterAABB3( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB frustumAABB0( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB frustumAABB1( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB frustumAABB2( CONSTRUCT_INFINITE_NEGATIVE );
	CAABB frustumAABB3( CONSTRUCT_INFINITE_NEGATIVE );

	// add each visible renderable
	if( CRendererSettings::bEnableCulling )
	{
// how many renderables do we have
//		ScrntPrintf( "Renderable List size %d\n", m_obRenderables.size() );
		for( RenderableListType::const_iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
		{
			CRenderable* pobRenderable = *obIt;
			pobRenderable->m_FrameFlags = 0;
			if( pobRenderable->IsRendering() || pobRenderable->IsShadowCasting() )
			{

				if( pobRenderable->UpdateFrameFlags( &frustum, CRendererSettings::iShadowQuality, pShadowPlanes, pShadowFrustums ) )
				{
					if( pobRenderable->IsRendering() )
						RenderingContext::Get()->m_aVisibleRenderables.push_back( pobRenderable );
				}

				if( pobRenderable->m_FrameFlags & (	CRenderable::RFF_CAST_SHADOWMAP0 | 
													CRenderable::RFF_CAST_SHADOWMAP1 |
													CRenderable::RFF_CAST_SHADOWMAP2 |
													CRenderable::RFF_CAST_SHADOWMAP3 ) )
				{
					RenderingContext::Get()->m_aShadowCastingRenderables.push_back( pobRenderable );
				}
				CGatso::Start( "ListSpace::SetVisibleFrustum.Shadow" );

				// does this renderable have any shadow stuff to do 
				if( pobRenderable->m_FrameFlags & (	CRenderable::RFF_CAST_SHADOWMAP0 | CRenderable::RFF_RECIEVES_SHADOWMAP0 |
													CRenderable::RFF_CAST_SHADOWMAP1 | CRenderable::RFF_RECIEVES_SHADOWMAP1 |
													CRenderable::RFF_CAST_SHADOWMAP2 | CRenderable::RFF_RECIEVES_SHADOWMAP2 |
													CRenderable::RFF_CAST_SHADOWMAP3 | CRenderable::RFF_RECIEVES_SHADOWMAP3) )
				{
					CAABB worldAABB = pobRenderable->GetWorldSpaceAABB();

                    
					CAABB& box = worldAABB;
					// remove camera/object small movement causes nasty shadow map jittering
					float fractPart, intPart;
					static const float fTruncer = 1;
					fractPart = modff( box.Min().X() * fTruncer, &intPart );
					box.Min().X() = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
					fractPart = modff( box.Min().Y() * fTruncer, &intPart );
					box.Min().Y() = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
					fractPart = modff( box.Min().Z() * fTruncer, &intPart );
					box.Min().Z() = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
					fractPart = modff( box.Max().X() * fTruncer, &intPart );
					box.Max().X() = (intPart * (1.f / fTruncer)) + (1.f / fTruncer);
					fractPart = modff( box.Max().Y() * fTruncer, &intPart );
					box.Max().Y() = (intPart * (1.f / fTruncer)) + (1.f / fTruncer);
					fractPart = modff( box.Max().Z() * fTruncer, &intPart );
					box.Max().Z() = (intPart * (1.f / fTruncer)) + (1.f / fTruncer); 
					CAABB viewAABB = worldAABB;

#if defined(SHADOW_AABB_VIEW)
					viewAABB.Transform( viewMatrix );
#endif


					if( pobRenderable->m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
					{
						casterAABB0.Union( viewAABB );
					}
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
					{
						casterAABB1.Union( viewAABB );
					}
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
					{
						casterAABB2.Union( viewAABB );
					}
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
					{
						casterAABB3.Union( viewAABB );
					}

					if( pobRenderable->m_FrameFlags & CRenderable::RFF_RECIEVES_SHADOWMAP0 )
					{
                        (void)sm0MinSplitDistance;
						if( pobRenderable->m_FrameFlags & CRenderable::RFF_CLIPS_SHADOWMAP0)
						{
							CAABB clippedBox = pShadowFrustums[0].Intersect( worldAABB, sm0MinSplitDistance );
							if( clippedBox.IsValid() )
							{
#if defined(SHADOW_AABB_VIEW)
								clippedBox.Transform( viewMatrix );
#endif
								frustumAABB0.Union( clippedBox );
							}
						} else
						{
							frustumAABB0.Union( viewAABB );
						}
					}

					// compute the caster and shadow frustum AABB bounding volumes for shadow map 1
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_RECIEVES_SHADOWMAP1 )
					{
						if( pobRenderable->m_FrameFlags & CRenderable::RFF_CLIPS_SHADOWMAP1)
						{
							CAABB clippedBox = pShadowFrustums[1].Intersect( worldAABB, sm1MinSplitDistance );
							if( clippedBox.IsValid() )
							{
#if defined(SHADOW_AABB_VIEW)
								clippedBox.Transform( viewMatrix );
#endif
								frustumAABB1.Union( clippedBox );
							}
						} else 
						{
							frustumAABB1.Union( viewAABB );
						}
					}
					// compute the caster and shadow frustum AABB bounding volumes for shadow map 2
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_RECIEVES_SHADOWMAP2 )
					{
						if( pobRenderable->m_FrameFlags & CRenderable::RFF_CLIPS_SHADOWMAP2)
						{
							CAABB clippedBox = pShadowFrustums[2].Intersect( worldAABB, sm2MinSplitDistance );
							if( clippedBox.IsValid() )
							{
#if defined(SHADOW_AABB_VIEW)
								clippedBox.Transform( viewMatrix );
#endif
								frustumAABB2.Union( clippedBox );
							}
						} else
						{
							frustumAABB2.Union( viewAABB );
						}
					}
                    /*
					// compute the caster and shadow frustum AABB bounding volumes for shadow map 3
					if( pobRenderable->m_FrameFlags & CRenderable::RFF_RECIEVES_SHADOWMAP3  )
					{
						if( pobRenderable->m_FrameFlags & CRenderable::RFF_CLIPS_SHADOWMAP3)
						{
							CAABB clippedBox = pShadowFrustums[3].Intersect( worldAABB, sm3MinSplitDistance );
							if( clippedBox.IsValid() )
							{
#if defined(SHADOW_AABB_VIEW)
								clippedBox.Transform( viewMatrix );
#endif
								frustumAABB3.Union( clippedBox );
							}
						} else
						{
							frustumAABB3.Union( viewAABB );
						}
					}
                    */
				}
				CGatso::Stop( "ListSpace::SetVisibleFrustum.Shadow" );

			}
		}
	}
	else
	{
		for( RenderableListType::const_iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
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

	if( pShadowFrustums != 0 )
	{
#if !defined(SHADOW_AABB_VIEW)
		casterAABB0.Transform( viewMatrix );
		casterAABB1.Transform( viewMatrix );
		casterAABB2.Transform( viewMatrix );
		casterAABB3.Transform( viewMatrix );
		frustumAABB0.Transform( viewMatrix );
		frustumAABB1.Transform( viewMatrix );
		frustumAABB2.Transform( viewMatrix );
		frustumAABB3.Transform( viewMatrix );
#endif

		RenderingContext::Get()->m_shadowCasterAABB[0] = casterAABB0;
		RenderingContext::Get()->m_shadowCasterAABB[1] = casterAABB1;
		RenderingContext::Get()->m_shadowCasterAABB[2] = casterAABB2;
		RenderingContext::Get()->m_shadowCasterAABB[3] = casterAABB3;
		RenderingContext::Get()->m_shadowFrustumAABB[0] = frustumAABB0;
		RenderingContext::Get()->m_shadowFrustumAABB[1] = frustumAABB1;
		RenderingContext::Get()->m_shadowFrustumAABB[2] = frustumAABB2;
		RenderingContext::Get()->m_shadowFrustumAABB[3] = frustumAABB3;
	}

    //for (int i = 0; i < 4; ++i)
    //{
    //    ntPrintf("CasterAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowCasterAABB[i].Min().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Min().Z(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().X(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Y(), RenderingContext::Get()->m_shadowCasterAABB[i].Max().Z());
    //    ntPrintf("FrustrumAABB[%i] %f %f %f - %f %f %f\n", i, RenderingContext::Get()->m_shadowFrustumAABB[i].Min().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Min().Z(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().X(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Y(), RenderingContext::Get()->m_shadowFrustumAABB[i].Max().Z());
    //}
    //                                                      
    //ntPrintf("Visibles: %i, ShadowCasters: %i\n", RenderingContext::Get()->m_aVisibleRenderables.size(), RenderingContext::Get()->m_aShadowCastingRenderables.size());

	CGatso::Stop( "ListSpace::SetVisibleFrustum(OLD)" );
#else
	UNUSED(pShadowPercents);
#endif
}


inline bool MeshHeaderCompare(const CMeshInstance* left, const CMeshInstance* right)
{
	return left -> GetMeshHeader() < right -> GetMeshHeader();
}

static unsigned int uiMinInstPerBatch = 4;


void ListSpace::BatchRenderables(  RenderingContext::CONTEXT_DATA::RenderableVector &OldRenderables,
								   RenderingContext::CONTEXT_DATA::RenderableVector &NewRenderables, 
								   RenderingContext::CONTEXT_DATA::BatchableVector	&BatchableRenderables,
								   RenderingContext::CONTEXT_DATA::RenderableVector &BatchedRenderables ) const
{
	typedef RenderingContext::CONTEXT_DATA::BatchableVector	BatchVector;

	CGatso::Start( "ListSpace::BatchedLists" );
	// separate batch-able renderables from non batch-able ones
	for( unsigned int iRenderable = 0; iRenderable < OldRenderables.size(); ++iRenderable )
	{
		if (( OldRenderables[iRenderable]->GetRenderableType() == CRenderable::RT_MESH_INSTANCE ) &&
			(!( OldRenderables[iRenderable]->IsAlphaBlended() ))) 	
			BatchableRenderables.push_back( (CMeshInstance*)OldRenderables[iRenderable] );
		else
			NewRenderables.push_back( OldRenderables[iRenderable] );
	}

	// decide if we want to batch this stuff or not

	// sort batch-able renderables
	qsort( &BatchableRenderables[0], BatchableRenderables.size(), sizeof(CRenderable**), QsortBatchRenderableComparator );


	BatchVector::iterator leftIter = BatchableRenderables.begin();
	BatchVector::iterator rightIter(leftIter);

	do																						 
	{
		// employ binary search to find the last renderable of the same mesh header
		rightIter = std::upper_bound(leftIter, BatchableRenderables.end(), *leftIter, &MeshHeaderCompare);	 // needs to be ntstd as soon as it's added to ntsd.h

		// how many renderables of the same mesh header do we have
		size_t numInstances = ntstd::distance(leftIter, rightIter);

		// if it's less than uiMinInstPerBatch don't bother batching them
		if ( numInstances < uiMinInstPerBatch )
		{
			NewRenderables.insert(NewRenderables.end(), leftIter, rightIter);
		}
		else
		{
			BatchedRenderables.insert(BatchedRenderables.end(), leftIter, rightIter);
		}
		leftIter = rightIter;
	}
	while ( rightIter != BatchableRenderables.end() );


	CGatso::Stop( "ListSpace::BatchedLists" );
}


// Never forger that Dinkum ware STL is deallocating memory in vector::clear()
// so if a vector is reused in subsequent frames just resize it to 0 which effectively just resets its size to 0 without releasing memory
template <class T>
inline void VectorClear(ntstd::Vector<T*, Mem::MC_GFX>& vec)
{
	vec.resize(0);
}


void ListSpace::UpdateVisibleOpaqueAndAlphaLists() const
{
#ifdef PLATFORM_PS3
//	ntAssert(g_tempContainer);

	if ( CRendererSettings::bEnableBatchRenderer )
	{
		// clear some visible renderables vectors
		VectorClear(RenderingContext::Get()->m_aBatchableShadowCastingRenderables);
		VectorClear(RenderingContext::Get()->m_aBatchableVisibleRenderables);
		VectorClear(RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables);
		VectorClear(RenderingContext::Get()->m_aBatchedVisibleAlphaRenderables);
		VectorClear(RenderingContext::Get()->m_aBatchedShadowCastingRenderables);

		// Batch Shadow maps Renderables
		RenderingContext::CONTEXT_DATA::RenderableVector NewShadowCastingRenderables;

		uint32_t preBatchCount = RenderingContext::Get()->m_aShadowCastingRenderables.size();

		BatchRenderables( RenderingContext::Get()->m_aShadowCastingRenderables, NewShadowCastingRenderables, 
						RenderingContext::Get()->m_aBatchableShadowCastingRenderables,
						RenderingContext::Get()->m_aBatchedShadowCastingRenderables );

		// swap th shadow caster list for a new one that we have created (this will just swap the buffers of 2 vectors avoiding copying all the elements)
		RenderingContext::Get()->m_aShadowCastingRenderables.swap(NewShadowCastingRenderables); 

		//uint32_t postBatchCount = RenderingContext::Get()->m_aShadowCastingRenderables.size() + RenderingContext::Get()->m_aBatchedShadowCastingRenderables.size();

		//ntAssert( postBatchCount == preBatchCount );


		// Batch Opaque Renderables
		RenderingContext::CONTEXT_DATA::RenderableVector& NewVisibleRenderable = NewShadowCastingRenderables;
		VectorClear(NewVisibleRenderable);

		preBatchCount = RenderingContext::Get()->m_aVisibleRenderables.size();
		NewVisibleRenderable.reserve(preBatchCount);

		BatchRenderables( RenderingContext::Get()->m_aVisibleRenderables, NewVisibleRenderable, 
						RenderingContext::Get()->m_aBatchableVisibleRenderables,
						RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables );

		RenderingContext::Get()->m_aVisibleRenderables.swap(NewVisibleRenderable);

		//postBatchCount = RenderingContext::Get()->m_aVisibleRenderables.size() + RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables.size();

		//ntAssert( postBatchCount == preBatchCount );

	}
#endif

	CGatso::Start( "ListSpace::UnbatchedLists" );

	// calculate sort keys
	CMatrix *pWorldToViewMat = &RenderingContext::Get()->m_worldToView;
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleRenderables.size(); ++iRenderable )
		RenderingContext::Get()->m_aVisibleRenderables[iRenderable]->CalculateSortKey(pWorldToViewMat);
	
	// sort renderables
	qsort( &RenderingContext::Get()->m_aVisibleRenderables[0], 
			RenderingContext::Get()->m_aVisibleRenderables.size(), 
			sizeof(CRenderable**), 
			QsortRenderableComparator );

	// erase the contents of the visible lists
	VectorClear(RenderingContext::Get()->m_aVisibleOpaqueRenderables);
	VectorClear(RenderingContext::Get()->m_aVisibleAlphaRenderables);


	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleRenderables.size(); ++iRenderable )
	{
		if( (RenderingContext::Get()->m_aVisibleRenderables[iRenderable]->IsAlphaBlended() == false) )
			RenderingContext::Get()->m_aVisibleOpaqueRenderables.push_back( RenderingContext::Get()->m_aVisibleRenderables[iRenderable] );
		else
			RenderingContext::Get()->m_aVisibleAlphaRenderables.push_back( RenderingContext::Get()->m_aVisibleRenderables[iRenderable] );
	}

	// calculate sort keys for shadow maps
	CMatrix *pShadowMat = &RenderingContext::Get()->m_shadowMapProjection[0];
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aShadowCastingRenderables.size(); ++iRenderable )
		RenderingContext::Get()->m_aShadowCastingRenderables[iRenderable]->CalculateSortKey(pShadowMat);

	qsort( &RenderingContext::Get()->m_aShadowCastingRenderables[0], 
			RenderingContext::Get()->m_aShadowCastingRenderables.size(), 
			sizeof(CRenderable**), 
			QsortRenderableComparator );

	VectorClear(RenderingContext::Get()->m_aShadowCastingOpaqueRenderables);
	VectorClear(RenderingContext::Get()->m_aShadowCastingAlphaRenderables);

	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aShadowCastingRenderables.size(); ++iRenderable )
	{
		if( (RenderingContext::Get()->m_aShadowCastingRenderables[iRenderable]->IsAlphaBlended() == false) )
			RenderingContext::Get()->m_aShadowCastingOpaqueRenderables.push_back( RenderingContext::Get()->m_aShadowCastingRenderables[iRenderable] );
		else
			RenderingContext::Get()->m_aShadowCastingAlphaRenderables.push_back( RenderingContext::Get()->m_aShadowCastingRenderables[iRenderable] );
	}

	CGatso::Stop( "ListSpace::UnbatchedLists" );

}

int ListSpace::QsortRenderableComparator( const void* a, const void* b )
{
	const CRenderable* renderableA = *((const CRenderable**)a);
	const CRenderable* renderableB = *((const CRenderable**)b);

//	return renderableA->GetSortKey() < renderableB->GetSortKey();

	if ( renderableA->GetSortKey() < renderableB->GetSortKey() )
		return -1;
	else if ( renderableA->GetSortKey() > renderableB->GetSortKey() )
		return 1;
	else
		return 0;
}

int ListSpace::QsortBatchRenderableComparator( const void* a, const void* b )
{
	const CMeshInstance* renderableA = *((const CMeshInstance**)a);
	const CMeshInstance* renderableB = *((const CMeshInstance**)b);

	uint32_t keyA = (uint32_t)renderableA->GetMeshHeader();
	uint32_t keyB = (uint32_t)renderableB->GetMeshHeader();

	if ( keyA < keyB )
		return -1;
	else if ( keyA > keyB )
		return 1;
	else
		return 0;
}


ListSpace::RenderableListType const& ListSpace::GetRenderableList() const
{
    return m_obRenderables;
}
