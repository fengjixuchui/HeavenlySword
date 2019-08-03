/***************************************************************************************************
*
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/exec_spu.h"

#include "army/army_ppu_spu.h"
#include "army/army_math_spu.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/unit.h"
#include "army/grunt.h"
#include <stdlib.h> // for qsort

#define CAM_DISTANCE_FAR_CONSTANT 0xFE
#define PLAYER_DISTANCE_FAR_CONSTANT 0xFE

/// just a temp global point to make the qsort callback easier to deal with
float* g_pTempDistBuf;

//------------------------------------------------------
//!
//! we need to know the distance of each renderable grunt from the camera
//! so we can decide how it should be rendered. this does a 2d camera check with 
//! people behind being treated as far away (using some simple maths)
//! the value is then stuck in the temp buffer, because its actually 'adjusted'
//! later on 
//!
//------------------------------------------------------
inline void UpdateDistanceToCamera( GruntRenderState* pRenderGrunt, const BattlefieldHeader* pBFHeader )
{
	v128 diff = spu_sub( pBFHeader->m_CameraPos.QuadwordValue(), pRenderGrunt->m_Position.QuadwordValue() );
	// kill height 
	diff = (v128) spu_and( (vector unsigned int)diff, (vector unsigned int){ 0xffffffff, 0, 0xffffffff, 0xffffffff } );

	v128 radi = (vector float){ pBFHeader->m_CameraRadius, pBFHeader->m_CameraRadius, pBFHeader->m_CameraRadius, pBFHeader->m_CameraRadius };
	v128 distsqr = Intrinsics::SPU_DP3( diff, diff );
	v128 cam_t = recipf4fast( spu_mul( rsqrtf4fast( distsqr ), radi ) );

	// make back hemisphere grunts further away than front hemisphere ones
	v128 v1 = spu_madd( pBFHeader->m_CameraFacing.QuadwordValue(), (vector float){5.f, 0.f, 5.f, 5.f}, pRenderGrunt->m_Position.QuadwordValue() );
	v1 = spu_sub( v1, pBFHeader->m_CameraPos.QuadwordValue() );
	v128 v2 = pBFHeader->m_CameraFacing.QuadwordValue();
	v1 = spu_mul( Intrinsics::SPU_RSqrt( Intrinsics::SPU_DP3(v1,v1) ), v1 );

	v128 viewdot = Intrinsics::SPU_DP3( v1, v2 );
	viewdot = Intrinsics::SPU_Min( viewdot, (vector float){0.f, 0.f, 0.f, 0.f} );
	cam_t = spu_madd( viewdot , (vector float){-50000.f, -50000.f, -50000.f, -50000.f}, cam_t );
	

	g_pTempDistBuf[ pRenderGrunt->m_GruntID ] = spu_extract( cam_t, 0 );
}

//------------------------------------------------------
//!
//! we also want to compute the distance to the player for the full AI logic
//! this does that with an per grunt option to not change to a real dude regardless of distance
//!
//------------------------------------------------------
inline void UpdateDistanceToPlayer( GruntRenderState* pRenderGrunt, const BattlefieldHeader* pBFHeader )
{
	v128 diff = spu_sub( pBFHeader->m_PlayerPos.QuadwordValue(), pRenderGrunt->m_Position.QuadwordValue() );
	// kill height 
	diff = (v128) spu_and( (vector unsigned int)diff, (vector unsigned int){ 0xffffffff, 0, 0xffffffff, 0xffffffff } );

	v128 radi = (vector float){ pBFHeader->m_PlayerRadius, pBFHeader->m_PlayerRadius, pBFHeader->m_PlayerRadius, pBFHeader->m_PlayerRadius };
	v128 distsqr = Intrinsics::SPU_DP3( diff, diff );

	v128 cam_t = recipf4fast( spu_mul( rsqrtf4fast( distsqr ), radi ) );
	cam_t = spu_madd( spu_splats( (float)pRenderGrunt->m_bDontRealDude), spu_splats(50000.f), cam_t );

	cam_t = Intrinsics::SPU_Min( cam_t, (vector float){1.f, 1.f, 1.f, 1.f} );
	cam_t = Intrinsics::SPU_Max( cam_t, (vector float){0.f, 0.f, 0.f, 0.f} );

	pRenderGrunt->m_iDistToPlayer = (uint8_t)(spu_extract( cam_t, 0 ) * 255.f);

}
//------------------------------------------------------
//!
//! Just a helper function to make allocating real dude tidy
//!
//------------------------------------------------------
inline bool AllocRealDude( ArmyGruntRenderAllocator* pAllocator, const uint16_t index )
{
	if( pAllocator->m_iNumRealDudeToAlloc >= MAX_ARMY_REAL_DUDE_ALLOCS_PER_FRAME )
	{
		return false;
	}

	pAllocator->m_RealDudeAllocIds[ pAllocator->m_iNumRealDudeToAlloc++ ] = index;
	return true;
}

//------------------------------------------------------
//!
//! Just a helper function to make de-allocating real dude tidy
//!
//------------------------------------------------------
inline bool DeAllocRealDude( ArmyGruntRenderAllocator* pAllocator, const uint16_t index )
{
	if( pAllocator->m_iNumRealDudeToDeAlloc >= MAX_ARMY_REAL_DUDE_DEALLOCS_PER_FRAME )
	{
		return false;
	}


	pAllocator->m_RealDudeDeAllocIds[ pAllocator->m_iNumRealDudeToDeAlloc++ ] = index;
	return true;
}

//------------------------------------------------------
//!
//! Just a helper function to make allocating renderable tidy
//!
//------------------------------------------------------
inline bool AllocRenderable( ArmyGruntRenderAllocator* pAllocator, const uint16_t index )
{
	if( pAllocator->m_iNumRenderablesToAlloc >= MAX_ARMY_RENDERABLE_ALLOCS_PER_FRAME )
	{
		return false;
	}

	pAllocator->m_RenderableAllocIds[ pAllocator->m_iNumRenderablesToAlloc++ ] = index;
	return true;
}

//------------------------------------------------------
//!
//! Just a helper function to make de-allocating renderable tidy
//!
//------------------------------------------------------
inline bool DeAllocRenderable( ArmyGruntRenderAllocator* pAllocator, const uint16_t index )
{
	if( pAllocator->m_iNumRenderablesToDeAlloc >= MAX_ARMY_RENDERABLE_DEALLOCS_PER_FRAME )
	{
		return false;
	}

	pAllocator->m_RenderableDeAllocIds[ pAllocator->m_iNumRenderablesToDeAlloc++ ] = index;
	return true;
}


//-----------------------------------------------------
//!
//!	QsortGruntComparator
//! sort render grunts by lod parameters
//!
//-----------------------------------------------------
int QsortGruntLodComparator( const void* a, const void* b )
{
	const GruntRenderState* bindA = static_cast<const GruntRenderState*>(a);
	const GruntRenderState* bindB = static_cast<const GruntRenderState*>(b);

	// distance to player must important metric, so closet to player
	// first in the list (for conversion in real dudes)
	// at the same distance (include FAR from player) sort by distance
	// to camera
	// intention is list like (where any particular group my be 0 sized)
	// |REAL DUDE | RENDERABLE          | SPRITES                 |
	if( bindA->m_iDistToPlayer < bindB->m_iDistToPlayer )
	{
		return -1;
	} else if( bindA->m_iDistToPlayer > bindB->m_iDistToPlayer )
	{
		return 1;
	} else
	{
		if( bindA->m_iDistToPlayer < PLAYER_DISTANCE_FAR_CONSTANT )
		{
			return (bindA->m_GruntID - bindB->m_GruntID);
		} else
		{
			// the multiple is jst because -0 and +0 are not differnt in ints, this give us 4 decimal point accuracy..
			return (int)((g_pTempDistBuf[ bindA->m_GruntID ] - g_pTempDistBuf[ bindB->m_GruntID ]) * 1000.f);
		}
	}
}


//------------------------------------------------------
//!
//! entry point to this program
//! takes grunts and produces the render grunt array
//! also works out what render state they should be in
//! and sorts the whole thing into a nice priority queue
//! so that the ppu has a fairly small amount of work todo
//!
//------------------------------------------------------
void UnitRender( SPUArgumentList &params )
{
	GetArrayInput( BattalionArray*,				pBattalionArray,		SRTA_BATTALION_ARRAY );
	GetArrayInput( BattlefieldHeader*,			pBattlefieldHeader,		SRTA_BATTLEFIELD_HEADER );
	GetArrayInput( GruntArray*,					pGruntArray,			SRTA_GRUNTS );
	GetArrayOutput( ArmyGruntRenderAllocator*,	pGruntRenderAllocator,	SRTA_GRUNT_RENDER_ALLOCATOR );
	GetArrayOutput( GruntRenderState*,			pRenderGrunts,			SRTA_RENDER_GRUNTS );

	GruntGameState* pGrunts = (GruntGameState*) (pGruntArray+1);
	uint16_t* pGruntIndex = (uint16_t*)(pGrunts + pGruntArray->m_iNumGrunts);

	// to improve the mesh/sprite allocate we store the float distance and 
	// sort on that then rescale the 8 bit distance camera from the closest
	// grunt.
	g_pTempDistBuf = (float*)Allocate( pGruntArray->m_iNumGrunts * sizeof(float) );

	uint16_t iRealDudes = 0;
	uint16_t iRenderables = 0;

	uint32_t iCurRenderNum = 0;
	// note this can be seperated into seperate tasks and distrubuted easily
	for( uint16_t iBatNum = 0; iBatNum < pBattalionArray->m_iNumBattalions; ++iBatNum )
	{
		const Battalion* pBattalion = &pBattalionArray->m_BattalionArray[iBatNum];
		for( uint16_t i=0;i < pBattalion->m_iNumUnits;i++)
		{
			const Unit* pUnit = &pBattalion->m_Units[i];
			ntAssert( pUnit->m_BattalionID == iBatNum );
			uint16_t iGruntID = pUnit->m_FirstGruntID;

			for( uint16_t j=0;j < pUnit->m_NumGrunts;j++, iGruntID++)
			{
				// do the double index to get this grunt
				GruntGameState* pGrunt = &pGrunts[ pGruntIndex[ iGruntID ] ];

				ntAssert( pGrunt->m_BattalionID == iBatNum );
				ntAssert( pGrunt->m_UnitID == i );
				
				GruntRenderState* pRenderGrunt = &pRenderGrunts[ iCurRenderNum++ ]; 

				pRenderGrunt->m_Position = BF_PositionToWorldSpace( &pGrunt->m_Position, pBattlefieldHeader );
				pRenderGrunt->m_Position = CPoint( pRenderGrunt->m_Position.X(), ((float)pGrunt->m_CurHeight) * (1.f/ARMY_HEIGHTFIELD_FLOAT_MULT), pRenderGrunt->m_Position.Z() );
				pRenderGrunt->m_Orientation = CQuat( CDirection(0,1,0), -pGrunt->m_Orientation.m_Rotation * 2.f * ARMY_PIE * (1.f/65535.f) ); 		
				pRenderGrunt->m_UnitType = pUnit->m_UnitType;
				pRenderGrunt->m_GruntID = iGruntID;
				pRenderGrunt->m_RenderType = pGrunt->m_RenderType; // copy over our current type
				pRenderGrunt->m_DesiredRenderType = GRT_NOCHANGE;
				pRenderGrunt->m_GruntMajorState = pGrunt->m_GruntMajorState;
				pRenderGrunt->m_bRealDudeAllocFail = 0;
				pRenderGrunt->m_bDontRealDude = 0;
				if( !(	(pRenderGrunt->m_GruntMajorState == GMS_NORMAL) || (pRenderGrunt->m_GruntMajorState == GMS_REAL_DUDE)) )
				{
					pRenderGrunt->m_bDontRealDude = 1;
				}
				if( !(pBattalion->m_BattalionFlags & BF_PLAYER_TRACKING) )
				{
					pRenderGrunt->m_bDontRealDude = 1;
				}


				UpdateDistanceToCamera( pRenderGrunt, pBattlefieldHeader );
				UpdateDistanceToPlayer( pRenderGrunt, pBattlefieldHeader );
			}
		}
	}

	// get render grunts sorted by distant to player and camera
	qsort( pRenderGrunts, pGruntArray->m_iNumGrunts, sizeof(GruntRenderState), QsortGruntLodComparator ); 

	// clear out the allocator
	pGruntRenderAllocator->m_iNumRealDudeToAlloc = 0;
	pGruntRenderAllocator->m_iNumRealDudeToDeAlloc = 0;
	pGruntRenderAllocator->m_iNumRealDudeToAlloc = 0;
	pGruntRenderAllocator->m_iNumRenderablesToAlloc = 0;
	pGruntRenderAllocator->m_iNumRenderablesToDeAlloc = 0;

	// recalibate dist to cam (so its relative to the closest grunt to maximise quality of far shots)
	// and determine what type we should be and whether we want to change
	for( uint16_t i=0; i < pGruntArray->m_iNumGrunts; ++i )
	{
		GruntRenderState* pRenderGrunt = &pRenderGrunts[ i ]; 
 
		float dist = g_pTempDistBuf[ pRenderGrunt->m_GruntID ] - g_pTempDistBuf[pRenderGrunts[0].m_GruntID];
		v128 vdist = (vector float){dist,dist,dist,dist};
		v128 cam_t = Intrinsics::SPU_Min( vdist, (vector float){1.f, 1.f, 1.f, 1.f} );
		cam_t = Intrinsics::SPU_Max( cam_t, (vector float){0.f, 0.f, 0.f, 0.f} );
		pRenderGrunt->m_iDistToCam = (uint8_t)(spu_extract( cam_t, 0 ) * 255.f);


		// the SPU likes a good branch... ahem won't even bother hinting as that bust
		// by the buggy SPU at the mo... oh well, probably faster than PPU anyway..
		if( (iRealDudes < pBattlefieldHeader->m_iMaxAIDudes) && 
			(pRenderGrunt->m_iDistToPlayer < PLAYER_DISTANCE_FAR_CONSTANT) ) 
		{
			// the sort has removed this if but its implicit, i've removed for speed but for clarity
			// only these two state EVER get in here...
			//( (pRenderGrunt->m_GruntMajorState == GMS_NORMAL) || (pRenderGrunt->m_GruntMajorState == GMS_REAL_DUDE)

			if( pRenderGrunt->m_RenderType == GRT_REAL_DUDE )
			{
				iRealDudes++;
				continue;
			}
			// ok getting here means we have room and we should be a 
			// real dude and aren't already

			// to be a real dude we also have to be a renderable
			if( pRenderGrunt->m_RenderType == GRT_SPRITE )
			{
				if( AllocRenderable( pGruntRenderAllocator, i ) == false )
				{
					// okay we can't Allocate any more renderables, so just leave it where it was
					continue;
				} else
				{
					pRenderGrunt->m_DesiredRenderType = GRT_RENDERABLE;
				}
			}

			// okay good to try allocating
			if( AllocRealDude( pGruntRenderAllocator, i ) == true )
			{
				// whohoo we got one, and now just need the PPU transform
				// to do its magic
				pRenderGrunt->m_DesiredRenderType = GRT_REAL_DUDE;
				iRealDudes++;
				continue;
			}
			// fall thro to next (Renderable) if state
		}

		// note this is a delibrate thr
		if( (iRenderables < pBattlefieldHeader->m_iMaxPeeps) &&
			(pRenderGrunt->m_iDistToCam < CAM_DISTANCE_FAR_CONSTANT) )
		{
			if( pRenderGrunt->m_RenderType == GRT_RENDERABLE )
			{
				iRenderables++;
				continue;
			}
			// ok getting here means we have room and we should be a 
			// renderable and aren't already

			// if we were a renderable lets dealloc that if we can
			if( pRenderGrunt->m_RenderType == GRT_REAL_DUDE )
			{
				if( DeAllocRealDude( pGruntRenderAllocator, i ) == false )
				{
					// okat we can't deallocate any more real dudes, so just leave it where it was
					// and goto the next grunt
					iRealDudes++;
					continue;
				} else
				{
					// all real dudes also have a renderable so make as well just
					// use it without going thro a dealloc, alloc cycle
					pRenderGrunt->m_DesiredRenderType = GRT_RENDERABLE;
					iRenderables++;
					continue;
				}
			}

			// okay good to try allocating
			if( AllocRenderable( pGruntRenderAllocator, i ) == true )
			{
				// whohoo we got one, and now just need the PPU transform
				// to do its magic
				pRenderGrunt->m_DesiredRenderType = GRT_RENDERABLE;
				iRenderables++;
				continue;
			}
			// fall thro to next (sprite) state
		} else
		{
			// if we were a real dude lets dealloc that if we can
			if( pRenderGrunt->m_RenderType == GRT_REAL_DUDE )
			{
				if( DeAllocRealDude( pGruntRenderAllocator, i ) == false )
				{
					// okat we can't deallocate any more real dudes, so just leave it where it was
					// and goto the next grunt
					iRealDudes++;
					continue;
				} else
				{
					if( DeAllocRenderable( pGruntRenderAllocator, i ) == false )
					{
						// okat we can't deallocate any more renderables, so just leave it where it was
						// and goto the next grunt
						iRenderables++;
						continue;
					} else
					{
						pRenderGrunt->m_DesiredRenderType = GRT_SPRITE;
						continue;
					}
				}
			}

			// if we were a renderable lets dealloc that if we can
			if( pRenderGrunt->m_RenderType == GRT_RENDERABLE )
			{
				if( DeAllocRenderable( pGruntRenderAllocator, i ) == false )
				{
					// okat we can't deallocate any more renderables, so just leave it where it was
					// and goto the next grunt
					iRenderables++;
					continue;
				}
			}
			pRenderGrunt->m_DesiredRenderType = GRT_SPRITE;
		}
	}
}
