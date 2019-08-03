#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"
#include "ntlib_spu/boundingvolumes_spu.h"
#include "clipper_dmadata.h"

#ifdef CLIPPER_DMA_FROM_SPU
#include "ntlib_spu/ntDmaList.h"
#endif

using namespace ClipperData;

#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) < (b)) ? (b) : (a)

namespace ClipperSPU
{
    const unsigned int MAX_SHADOW_FRUSTUMS = NUM_SHADOW_PLANES - 1;

#ifdef CLIPPER_DMA_FROM_SPU
	CAABB	g_boundingBoxes[s_renderablesPerTask];
#endif

	typedef vector unsigned int vi128;

	enum RENDERABLE_FRAME_FLAGS
	{
		RFF_CLEAR				= 0,		//!< no frame flags not visible, not casting 
		RFF_VISIBLE				= (1 << 0),	//!< the renderable is visible this frame
		RFF_CLIPPING			= (1 << 1), //!< this renderable clips (intersects) at least one clip plane of the frustum.

		RFF_CAST_SHADOWMAP0		= (1 << 2), //!< this renderable casts shadows on shadowmap0
		RFF_CAST_SHADOWMAP1		= (1 << 3), //!< this renderable casts shadows on shadowmap1
		RFF_CAST_SHADOWMAP2		= (1 << 4), //!< this renderable casts shadows on shadowmap2
		RFF_CAST_SHADOWMAP3		= (1 << 5), //!< this renderable casts shadows on shadowmap3

		RFF_RECIEVES_SHADOWMAP0	= (1 << 6), //!< this renderable recieves shadows on shadowmap0
		RFF_RECIEVES_SHADOWMAP1	= (1 << 7), //!< this renderable recieves shadows on shadowmap1
		RFF_RECIEVES_SHADOWMAP2	= (1 << 8), //!< this renderable recieves shadows on shadowmap2
		RFF_RECIEVES_SHADOWMAP3	= (1 << 9), //!< this renderable recieves shadows on shadowmap3

		RFF_CLIPS_SHADOWMAP0	= (1 << 10), //!< this renderable receiving clips the shadows frustumon shadowmap0
		RFF_CLIPS_SHADOWMAP1	= (1 << 11), //!< this renderable receiving clips the shadows frustumon shadowmap1
		RFF_CLIPS_SHADOWMAP2	= (1 << 12), //!< this renderable receiving clips the shadows frustumon shadowmap2
		RFF_CLIPS_SHADOWMAP3	= (1 << 13), //!< this renderable receiving clips the shadows frustumon shadowmap3
	};


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
//
//void PrintFrustum(const char* name, const CullingFrustum& frustum)
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
//void PrintAABB(const char* name, const CAABB& aabb)
//{
//	ntPrintf("%s : \n", name);
//
//	ntPrintf("Min : ");
//	PrintPoint(aabb.Min()); 
//	ntPrintf("Max : ");
//	PrintPoint(aabb.Max());
//}


	bool SweptSpherePlaneIntersect(float& t0, float& t1, const CPlane* plane, const CPoint* center, const float radius, const CDirection* sweepDir)
	{
		float b_dot_n = plane->GetNormal().Dot( CDirection(*center) ) + plane->GetDistance();
		float d_dot_n = plane->GetNormal().Dot( *sweepDir );

		if (d_dot_n == 0.f)
		{
			if (b_dot_n <= radius)
			{
				//  effectively infinity
				t0 = 0.f;
				t1 = 1e32f;
				return true;
			}
			else
				return false;
		}
		else
		{
			float tmp0 = ( radius - b_dot_n) / d_dot_n;
			float tmp1 = (-radius - b_dot_n) / d_dot_n;
			t0 = min(tmp0, tmp1);
			t1 = max(tmp0, tmp1);
			return true;
		}
	}

/*
    //  test if a sphere is within the view frustum
    template <unsigned int TRUE_VALUE>
    unsigned int TestSphere(CullingFrustum const& frustum, const CSphereBound* sphere)
    {
        bool inside = true;
	    CDirection center( sphere->GetPosition().X(), sphere->GetPosition().Y(), sphere->GetPosition().Z() ); 
	    float radius = sphere->GetRadius();

        for (int i=0; (i<6) && inside; i++)
        {
		    inside = inside && ((frustum.GetPlane(i) -> GetNormal().Dot(center) + frustum.GetPlane(i) -> GetDistance() + radius) >= 0.f);
        }

        return inside ? TRUE_VALUE : 0;
    }
    */

    

    //  test if a sphere is within the view frustum
    // see the FP implementation above
    template <unsigned int TRUE_VALUE>
    unsigned int TestSphere(CullingFrustum const& frustum, const CSphereBound* sphere)
    {
		const v128 ZeroVector = (v128){0, 0, 0, 0};

        CPoint_SOA center(sphere->GetPosition().QuadwordValue());

        v128 rad = spu_splats(sphere->GetRadius());

		v128 dist = {0, 0, 0, 0};
        dist = spu_insert(frustum.GetPlane(0) -> GetDistance(), dist, 0);
        dist = spu_insert(frustum.GetPlane(1) -> GetDistance(), dist, 1);
        dist = spu_insert(frustum.GetPlane(2) -> GetDistance(), dist, 2);
        dist = spu_insert(frustum.GetPlane(3) -> GetDistance(), dist, 3);

        dist = spu_add(dist, rad);

        CPoint_SOA  normals( frustum.GetPlane(0) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(1) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(2) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(3) -> GetNormal().QuadwordValue() );

        v128 dotResult = CPoint_SOA::Dot(normals, center);

        dist = spu_add(dotResult, dist);

        vector unsigned int compResult = spu_cmpgt(dist, ZeroVector);
        //compResult = spu_xor(compResult, compResult);
        vector unsigned int resultVec = spu_gather(compResult);
        if (spu_extract(resultVec, 0) != 15)
        {
            return 0;
        }

        dist = spu_insert(frustum.GetPlane(4) -> GetDistance(), dist, 0);
        dist = spu_insert(frustum.GetPlane(5) -> GetDistance(), dist, 1);

        dist = spu_add(dist, rad);

        CPoint_SOA  normals2( frustum.GetPlane(4) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(5) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(5) -> GetNormal().QuadwordValue(),
            frustum.GetPlane(5) -> GetNormal().QuadwordValue() );
        dotResult = CPoint_SOA::Dot(normals2, center);
        dist = spu_add(dotResult, dist);

        vector unsigned int compResult2 = spu_cmpgt(dist, ZeroVector);
        resultVec = spu_gather(compResult2);
        //if ((spu_extract(resultVec, 0) & 3) != 3)
        if ((spu_extract(resultVec, 0) & 12) != 12)
        {
            return 0;
        }

        return TRUE_VALUE;

    }
      

    template <unsigned int TRUE_VALUE>
	unsigned int TestSweptSphere(CullingFrustum const& frustum, const CSphereBound *sphere, const CDirection *sweepDir)
	{
		ntAssert( sphere );
		ntAssert( sweepDir );

		CDirection dirSweep( sweepDir->X(), sweepDir->Y(), sweepDir->Z() );
		CPoint center( sphere->GetPosition().X(), sphere->GetPosition().Y(), sphere->GetPosition().Z() ); 
		float radius = sphere->GetRadius();

		//  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
		//  for all points >0, displace sphere along the sweep driection.  if the displaced sphere
		//  is inside the frustum, return TRUE.  else, return FALSE
		float displacements[12];
		int cnt = 0;
		float a, b;
		unsigned int inFrustum = 0;

		for (int i=0; i<6; i++)
		{
			if (SweptSpherePlaneIntersect(a, b, frustum.GetPlane(i), &center, radius, &dirSweep))
			{
				if (a>=0.f)
					displacements[cnt++] = a;
				if (b>=0.f)
					displacements[cnt++] = b;
			}
		}

		for (int i=0; i<cnt; i++)
		{
			CSphereBound dispSphere;
			dispSphere.SetPosition( sphere->GetPosition() + displacements[i] * (*sweepDir) );
			dispSphere.SetRadius( sphere->GetRadius() * 1.1f ); // spot the fudge factor...
			inFrustum |= TestSphere<TRUE_VALUE>(frustum, &dispSphere);
		}

        return inFrustum;
     }

	//  Tests if an AABB is inside/intersecting the view frustum
	unsigned int TestBox(CullingFrustum const& frustum, CAABB const& aabb )
	{
        unsigned int retVal = CullingFrustum::COMPLETELY_INSIDE;

		for (int planeIndex = 0; planeIndex < 6; planeIndex++)
		{
			CPlane const*	plane				= frustum.GetPlane(planeIndex);
			CDirection const& normal			= plane -> GetNormal();

			v128  min	= aabb.Min().QuadwordValue();
			v128  max	= aabb.Max().QuadwordValue();

			v128 min1 = (v128)spu_and((vi128)min, frustum.m_nVertexLUT[planeIndex]);
			v128 max1 = (v128)spu_andc((vi128)max, frustum.m_nVertexLUT[planeIndex]);

			v128 max2 = (v128)spu_and((vi128)max, frustum.m_nVertexLUT[planeIndex]);
			v128 min2 = (v128)spu_andc((vi128)min, frustum.m_nVertexLUT[planeIndex]);

			CDirection nVertex(spu_or(min1, max1));
			CDirection pVertex(spu_or(min2, max2));


			if (  __builtin_expect( normal.Dot( nVertex ) < - frustum.GetPlane(planeIndex) -> GetDistance(), 0 ) )
				return CullingFrustum::COMPLETELY_OUTSIDE;

			if ( normal.Dot( pVertex ) + frustum.GetPlane(planeIndex) -> GetDistance() < 0.f )
				retVal = CullingFrustum::PARTIALLY_INSIDE;
		}

		return retVal;
	}

//#define USE_LUT

#ifndef USE_LUT
	inline unsigned short FastTestBox(CullingFrustum const& frustum, CAABB const& aabb)
	{
		for (unsigned short planeIndex = 0; planeIndex < 6; ++ planeIndex)
		{
			CPlane const*	plane				= frustum.GetPlane(planeIndex);
			CDirection const& normal			= plane -> GetNormal();

			v128  min	= aabb.Min().QuadwordValue();
			v128  max	= aabb.Max().QuadwordValue();
			min = (v128)spu_and((vi128)min, frustum.m_nVertexLUT[planeIndex]);
			max = (v128)spu_andc((vi128)max, frustum.m_nVertexLUT[planeIndex]); 

			CDirection vertex(spu_or(min, max));                                

			if ( __builtin_expect( normal.Dot( vertex ) < - plane -> GetDistance(), 0) )
			{
				return RFF_CLEAR;
			}
		}

		return (RFF_CLIPPING | RFF_VISIBLE);
	}
#else
	inline unsigned short FastTestBox(CullingFrustum const& frustum, CAABB const& aabb)
	{
		for (unsigned short planeIndex = 0; planeIndex < 6; ++ planeIndex)
		{
			CPlane const*	plane				= frustum.GetPlane(planeIndex);
			CDirection const& normal			= plane -> GetNormal();

			int nV = spu_extract(spu_gather(frustum.m_nVertexLUT[planeIndex]), 0);

			CDirection vertex(	(nV&1)?aabb.Min().X() : aabb.Max().X(), 
									(nV&2)?aabb.Min().Y() : aabb.Max().Y(), 
									(nV&4)?aabb.Min().Z() : aabb.Max().Z() );


			if ( __builtin_expect( normal.Dot( vertex ) < - plane -> GetDistance(), 0) )
			{
				return RFF_CLEAR;
			}
		}

		return (RFF_CLIPPING | RFF_VISIBLE);
	}
#endif

    /// calculate shadow clipping planes
    void CreateShadowFrustums(DMA_In* input, CullingFrustum (&shadowFrustums)[MAX_SHADOW_FRUSTUMS] )
    {
        ClipperDataIn const&    params = input -> m_controlData;

       	const float zdist = params.m_ZFar - params.m_ZNear;
		float zs[MAX_SHADOW_FRUSTUMS + 1];
		for(unsigned int i = 0; i <= MAX_SHADOW_FRUSTUMS; ++ i)
		{
			zs[i] = params.m_ZNear + zdist * params.m_shadowPercents[i] * 0.01f;
		}

		for(unsigned int i = 0; i < MAX_SHADOW_FRUSTUMS; ++ i)
		{
			CMatrix projMatrix;
			projMatrix.Perspective( params.m_FOV, params.m_aspectRatio, zs[i], zs[i+1] );
            CMatrix worldScreen;
            MatrixMultiply(&worldScreen, &params.m_worldToView, &projMatrix);
			shadowFrustums[i].Init( worldScreen );
		}
    }

	const CAABB&	GetAABB(DMA_In* input, int renderable)
	{
#ifndef CLIPPER_DMA_FROM_SPU
		return input -> m_renderables[renderable].m_AABB;
#else
		return g_boundingBoxes[renderable];
#endif
	}

    inline void ProcessRenderablesShadow(CullingFrustum const& frustum, DMA_In* input, DMA_Out* output, const CullingFrustum (&shadowFrustums)[MAX_SHADOW_FRUSTUMS], unsigned int numShadowPlanes, unsigned int& visibles, unsigned int& shadowCasters)
    {

		visibles = 0;
		for (unsigned int renderable = 0; renderable < input -> m_controlData.m_numRenderables; ++ renderable)
		{
			uint32_t renderableID = input -> m_renderables[renderable].m_renderableID;
			//const CAABB&	AABB = input -> m_renderables[renderable].m_AABB;
			const CAABB&	AABB = GetAABB(input, renderable);

            unsigned int flags = FastTestBox(frustum, AABB);
			if (RFF_CLEAR != flags)
			{

		        if( input -> m_renderables[renderable].m_flags & (1 << (rifReceiveShadow)) )
		        {
                    unsigned int shadowTests[4];
				    for( unsigned int i = 0; i < numShadowPlanes; i ++ )
				    {
                        unsigned int test = TestBox(shadowFrustums[i], AABB);
                        shadowTests[i] = test;
                        switch(test)
					    {
					    case CullingFrustum::PARTIALLY_INSIDE:
						    flags |= (RFF_CLIPS_SHADOWMAP0 << i);
						    // intential fallthrough
					    case CullingFrustum::COMPLETELY_INSIDE:
						    flags |= (RFF_RECIEVES_SHADOWMAP0 << i);
						    break;
					    default:
						    break;
					    }
			        }
		        }
			}


		    if( input -> m_renderables[renderable].m_flags & (1 << (rifCastShadow)) )
		    {
			    // the object doesn't intersect the frustum itself but
			    // what about its shadow?
			    // its definately not a reciever (its not visible) but the swept test
			    // will tell if its a caster
			    const float worldShadowLength = 1000.f;
                CDirection obWorldScaledShadowDir = input -> m_controlData.m_shadowDirection * worldShadowLength;

			    CSphereBound sphere( AABB );
                unsigned int shadowCastFlags = 0;
			    for( unsigned int i = 0; i < numShadowPlanes; i ++ )
			    {
                    unsigned int test = TestSweptSphere<RFF_CAST_SHADOWMAP0>(shadowFrustums[i], &sphere, &obWorldScaledShadowDir);
                    shadowCastFlags |= test << i;
			    }
                flags |= shadowCastFlags;

                if (shadowCastFlags)
                {
                    output -> m_shadowCasters[shadowCasters].m_renderableID = renderableID;
                    output -> m_shadowCasters[shadowCasters].m_flags       = flags;

                    ++ shadowCasters;
                }
		    }

			//if (input -> m_renderables[renderable].m_flags & (1 << rifRendering) && RFF_CLEAR != flags)
			if ( (input -> m_renderables[renderable].m_flags & (1 << rifRendering)) && (flags & RFF_VISIBLE))
			{
				output -> m_visibles[visibles].m_renderableID = renderableID;
				output -> m_visibles[visibles].m_flags       = flags;

				++ visibles;
			}

			if( flags & ( RFF_CAST_SHADOWMAP0 | RFF_RECIEVES_SHADOWMAP0 |          
												RFF_CAST_SHADOWMAP1 | RFF_RECIEVES_SHADOWMAP1 |
												RFF_CAST_SHADOWMAP2 | RFF_RECIEVES_SHADOWMAP2 |
												RFF_CAST_SHADOWMAP3 | RFF_RECIEVES_SHADOWMAP3) )
            {
                
                CAABB box = AABB;
               
				// remove camera/object small movement causes nasty shadow map jittering
				float fractPart, intPart;
				static const float fTruncer = 1;
				fractPart = modff( box.Min().X() * fTruncer, &intPart );
				float minX = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
				fractPart = modff( box.Min().Y() * fTruncer, &intPart );
				float minY = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
				fractPart = modff( box.Min().Z() * fTruncer, &intPart );
				float minZ = (intPart * (1.f / fTruncer)) - (1.f / fTruncer);
				fractPart = modff( box.Max().X() * fTruncer, &intPart );
				float maxX = (intPart * (1.f / fTruncer)) + (1.f / fTruncer);
				fractPart = modff( box.Max().Y() * fTruncer, &intPart );
				float maxY = (intPart * (1.f / fTruncer)) + (1.f / fTruncer);
				fractPart = modff( box.Max().Z() * fTruncer, &intPart );
				float maxZ = (intPart * (1.f / fTruncer)) + (1.f / fTruncer); 
                box.m_obMin = CPoint(minX, minY, minZ);
                box.m_obMax = CPoint(maxX, maxY, maxZ);

                for ( unsigned int shadowMap = 0; shadowMap < NUM_SHADOW_PLANES - 1; ++ shadowMap )
                {
                    if (flags & (RFF_CAST_SHADOWMAP0 << shadowMap))
                    {
                        output -> m_header.m_frustums[shadowMap].Union(box);
                    }

                    if (flags & (RFF_RECIEVES_SHADOWMAP0 << shadowMap))
                    {
						if( flags & (RFF_CLIPS_SHADOWMAP0 << shadowMap) )
						{
							CAABB clippedBox; // = box;

                            shadowFrustums[shadowMap].Intersect( box, &clippedBox );
							if( clippedBox.IsValid() )
							{
								output -> m_header.m_frustums[shadowMap + NUM_CASTER_AABB].Union( clippedBox );
							}
						} 
                        else
						{
							output -> m_header.m_frustums[shadowMap + NUM_CASTER_AABB].Union( box );
						}
                        
                    }

					//ntPrintf("Caster%i", shadowMap);
					//PrintAABB(" ", output -> m_header.m_frustums[shadowMap]);
					//ntPrintf("Frustum%i", shadowMap);
					//PrintAABB(" ", output -> m_header.m_frustums[shadowMap + NUM_CASTER_AABB]);

                }
              

            }

		}

    }

    inline unsigned short ProcessRenderables(CullingFrustum const& frustum, DMA_In* input, DMA_Out* output)
    {
		unsigned short visible = 0;
		for (unsigned int renderable = 0; renderable < input -> m_controlData.m_numRenderables; ++ renderable)
		{
			uint32_t renderableID = input -> m_renderables[renderable].m_renderableID;
			//const CAABB&	AABB = input -> m_renderables[renderable].m_AABB;
			const CAABB&	AABB = GetAABB(input, renderable);

            unsigned int flags = FastTestBox(frustum, AABB);
			if (RFF_CLEAR != flags)
			{
				output -> m_visibles[visible].m_renderableID = renderableID;
                output -> m_visibles[visible].m_flags       = flags;

                ++ visible;
			}
		}

        return visible;
    }

#ifdef CLIPPER_DMA_FROM_SPU
	typedef ntDMAList<s_renderablesPerTask>  ClipperDmaList;

	void DownloadAABBs(DMA_In* input)
	{
		static CMatrix	matrixArray[s_renderablesPerTask];

		int numRenderables = input -> m_controlData.m_numRenderables;

		ntDMA_ID	aabbListID			= ntDMA::GetFreshID();
		ntDMA_ID	transformListID		= ntDMA::GetFreshID();

		ClipperDmaList::Params	aabbListParams(g_boundingBoxes,  aabbListID);
		ClipperDmaList::Params	transformListParams(matrixArray, transformListID);

		for (int renderable = 0; renderable < numRenderables; ++ renderable)
		{
			ClipperDmaList::AddListElement32(aabbListParams, sizeof(CAABB), input -> m_renderables[renderable].m_renderableID);
			ClipperDmaList::AddListElement32(transformListParams, sizeof(CMatrix), input -> m_renderables[renderable].m_transformAddr);
		}
		ClipperDmaList::DmaToSPU(aabbListParams);
		ClipperDmaList::DmaToSPU(transformListParams);

		ntDMA::StallForCompletion(aabbListID);
		ntDMA::StallForCompletion(transformListID);

		ntDMA::FreeID(aabbListID);
		ntDMA::FreeID(transformListID);

		for (int renderable = 0; renderable < numRenderables; ++ renderable)
		{
			g_boundingBoxes[renderable].Transform(matrixArray[renderable]);
		}
	}
#endif


	void Process(DMA_In* input, DMA_Out* output)
	{
		ntAssert(input -> m_controlData.m_numRenderables <= s_renderablesPerTask);

#ifdef CLIPPER_DMA_FROM_SPU
		DownloadAABBs(input);
#endif
	
		CullingFrustum	frustum(input -> m_controlData.m_worldToScreen);

        CullingFrustum  shadowFrustums[MAX_SHADOW_FRUSTUMS];

		for (unsigned int frustum = 0; frustum < NUM_AABBS; ++ frustum)
		{
			AABB_ConstructInfiniteNegative(&output -> m_header.m_frustums[frustum]);
		}

        unsigned int visibles = 0;
        unsigned int shadowCasters = 0;
	    if( input -> m_controlData.m_shadowQuality > 0 )
	    {
            CreateShadowFrustums(input, shadowFrustums);
            ProcessRenderablesShadow(frustum, input, output, shadowFrustums, input -> m_controlData.m_shadowQuality, visibles, shadowCasters);
        }
        else
        {
            visibles = ProcessRenderables(frustum, input, output);
        }


		// process the input renderables

		// fill the rest of the output buffer with 0's (if the input vector was shorter than s_renderablesPerTask)
		for (unsigned int leadout = visibles; leadout < s_renderablesPerTask; ++ leadout)
		{
			output -> m_visibles[leadout].m_renderableID = 0;
		}

        for (unsigned int sleadout = shadowCasters; sleadout < s_renderablesPerTask; ++ sleadout)
		{
			output -> m_shadowCasters[sleadout].m_renderableID = 0;
		}


		// set the header
		output -> m_header.m_sizeVisible        = visibles;
        output -> m_header.m_sizeShadowcaster   = shadowCasters;
		
	}

}
