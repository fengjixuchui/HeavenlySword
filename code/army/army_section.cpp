//------------------------------------------------------
//!
//!	\file army/army_section.cpp
//!
//------------------------------------------------------

#include "ai/AINavigationSystem/aiworldvolumes.h"
#include "army/army_ppu_spu.h"
#include "army/armydef.h"
#include "army/armymanager.h"
#include "army/army_section.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/battalion.h"
#include "army/general.h"
#include "army/grunt.h"
#include "army/unit.h"
#include "army/armyrenderable.h"
#include "army/armyrenderpool.h"
#include "core/gatso.h"
#include "core/timer.h"

#include "camera/camutils.h"

#include "core/visualdebugger.h"
#include "core/exportstruct_anim.h"

// SPU-exec related includes
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/execspujobadder_ps3.h"
#include "jobapi/joblist.h"
#include "game/randmanager.h"
#include "game/entityplayer.h"
#include "game/entitymanager.h"
#include "game/entityspawnpoint.h"
#include "game/entityprojectile.h"
#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"


// includes required for audio hooks
#include "audio/audiosystem.h"


//#define _ARMY_SECTION_DEBUG_RENDER // Visual debugging for battalions

// this is purely to make sure any hangs AREN'T caused by our overuse of FrameReset
//#define DEBUG_SAFETY


void ArmySection::ForceMaterials( CMaterial const* pobMaterial )
{
	for( unsigned int i = 0; i < m_pBattlefield->m_pGrunts->m_iNumGrunts; ++i )
	{
		if ( m_pBattlefield->m_pRenderables[i] != NULL )
			m_pBattlefield->m_pRenderables[i]->GetRenderable()->ForceMaterial( pobMaterial );
	}
}

void ArmySection::BatchUpdateAnims( const float fTimeStep )
{
	CGatso::Start( "ArmySection::BatchUpdateAnims" );

	// Batch update the animations.
#	define BATCH_UPDATE_ARMY_ANIMS
#	ifdef BATCH_UPDATE_ARMY_ANIMS
	AnimatorBatchUpdate anim_batcher;
	anim_batcher.StartBatch();
#	endif

	GruntGameState* pFirstGrunt = (GruntGameState*)(m_pBattlefield->m_pGrunts+1);
	uint16_t*		pGruntIndices = (uint16_t*)(pFirstGrunt + m_pBattlefield->m_pGrunts->m_iNumGrunts);

	// do sprites stuff will SPU animator does its thing
	CDirection poleVec = CDirection(0, m_pArmyGenericParameters->m_fFlagPoleVerticalOffset, 0 );
	for( uint32_t i=0;i < m_pBattlefield->m_pGrunts->m_iNumGrunts;i++)
	{
		GruntRenderState* pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[i];
		ArmyRenderable* pRenderable = m_pBattlefield->m_pRenderables[pRenderGrunt->m_GruntID];
		GruntGameState* pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];
		if( pRenderable == 0 )
		{
			// sprites have a really simple 'anim' system

			// see if the AI logic wants to play an anim
			if( pGrunt->m_AnimToPlayBits & (1 << GAM_DEAD ) )
			{
				pGrunt->m_AnimPlayingBits = (1 << GAM_DEAD ); // we are now playing this so clear other bits
			} else if( pGrunt->m_AnimToPlayBits & (1 << GAM_RUN ) )
			{
				pGrunt->m_AnimPlayingBits = (1 << GAM_RUN ); // we are now playing this clear other bits
			} else
			{
				pGrunt->m_AnimPlayingBits = (1 << GAM_IDLE ); // we are now playing this clear other bits
			}
			pGrunt->m_AnimToPlayBits = 0;

			// now see which anim we are playing and update the value we pass to SetSprite
			uint8_t iAnimState = GAM_IDLE;
			if( pGrunt->m_AnimPlayingBits & (1 << GAM_DEAD ) )
			{
				iAnimState = GAM_DEAD;
			} else if( pGrunt->m_AnimPlayingBits & (1 << GAM_RUN ) )
			{
				iAnimState = GAM_RUN;
			}

			// HACK not sure why but Dead still use idle sprites... hmmm so override based on major state
			if( pGrunt->m_GruntMajorState == GMS_DEAD )
			{
				iAnimState = GAM_DEAD;
			}

			m_pBattlefield->m_pRenderPools[ pRenderGrunt->m_UnitType ]->SetSprite( pRenderGrunt->m_Position, pRenderGrunt->m_GruntID, iAnimState );

			// update sprite guys flags
			uint8_t iFlagIndex = m_pFlagArray[pRenderGrunt->m_GruntID];
			if( iFlagIndex != 0xFF )
			{
				ntError( pRenderGrunt->m_GruntID == m_ArmyFlags[ iFlagIndex ].m_GruntID );
				ArmyRenderable* pFlagPole = m_ArmyFlags[ iFlagIndex].m_pFlagPole;
				pFlagPole->GetRootTransform()->SetLocalTranslation( pRenderGrunt->m_Position + poleVec );
				pFlagPole->UpdatePartTwo();
			}
		} else
		{
#		ifdef BATCH_UPDATE_ARMY_ANIMS
			pRenderable->Update( fTimeStep, &anim_batcher );
#		else
			pRenderable->Update( fTimeStep, NULL );
#		endif
		}
	}

#	ifdef BATCH_UPDATE_ARMY_ANIMS
	anim_batcher.FinishBatch();
#	endif

	CGatso::Stop( "ArmySection::BatchUpdateAnims" );
}

void ArmySection::AllocateRenderable( GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt )
{
	ntError_p( pCurRenderGrunt->m_RenderType == GRT_SPRITE, ("We can only alloc a renderable if we are currently a sprite") );

	// we always allow a change from sprite to renderable cos well its a good thing and stop 
	// we are fairly aggresive converting back to sprites, this will help cover up that aggresion

	ArmyRenderable* pRenderable = m_pBattlefield->m_pRenderPools[ pCurRenderGrunt->m_UnitType ]->Allocate();

	// its possible for us not to be able to actually do the alloc in this case just 
	// leave us as a sprite
	if( pRenderable )
	{
		ntError(pGrunt->m_GruntMajorState != GMS_REAL_DUDE );

		pRenderable->EnableRendering();

		if( pGrunt->m_GruntMajorState == GMS_DEAD || pGrunt->m_GruntMajorState == GMS_DYING )
		{
			// make sure we moved state over to being really dead in case we were just dying
			pGrunt->m_GruntMajorState = GMS_DEAD;
			pGrunt->m_AnimToPlayBits = (1 << GAM_DEAD);

		} else
		{
			// reset all other states to normal
			pGrunt->m_GruntMajorState = GMS_NORMAL;
			pGrunt->m_AnimToPlayBits = (1 << GAM_TAUNT);
		}

		pGrunt->m_RenderType = GRT_RENDERABLE;
		pCurRenderGrunt->m_RenderType = GRT_RENDERABLE;
		m_pBattlefield->m_pRenderables[ pCurRenderGrunt->m_GruntID ] = pRenderable;
		pGrunt->m_RenderStateChangeCounter = MAX_RENDERSTATECHANGECOUNTER_COUNTER;

		uint8_t iFlagIndex = m_pFlagArray[pCurRenderGrunt->m_GruntID];
		if( iFlagIndex != 0xFF )
		{
			pRenderable->SetFlagBearer( true );			


			ntError( pCurRenderGrunt->m_GruntID == m_ArmyFlags[ iFlagIndex ].m_GruntID );
			ArmyRenderable* pFlagPole = m_ArmyFlags[ iFlagIndex].m_pFlagPole;

			static uint32_t r_weapon_hash = (uint32_t) CHashedString("r_weapon").GetHash();
			if( pFlagPole->GetRootTransform()->GetParent() )
			{
				pFlagPole->GetRootTransform()->RemoveFromParent();
			}
			pRenderable->GetHierarchy()->GetTransformFromHash( r_weapon_hash )->AddChild( pFlagPole->GetRootTransform()  );
			pFlagPole->GetRootTransform()->SetLocalTranslation( CPoint(CONSTRUCT_CLEAR) );
		} else
		{
			pRenderable->SetFlagBearer( false );
		}
	} else
	{
		// this will tell us that if we are about to try alloc a real dude this frame for this6
		// grunt, to abort as we couldn't get a renderable as well
		pCurRenderGrunt->m_bRealDudeAllocFail = 1;
	}
}

void ArmySection::AllocateRealDude( ArmyRenderable* pRenderable, GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt )
{
	// early out if we can't alloc a real dude cos of renderable alloc fail
	if( pCurRenderGrunt->m_bRealDudeAllocFail || pGrunt->m_RenderStateChangeCounter != 0 )
	{
		return;
	}

	// note assert against render grunt, as SPU unit render modifies it but can't update game grunt
	ntError_p( pCurRenderGrunt->m_RenderType == GRT_RENDERABLE, ("We can only alloc a real dude if we are currently a renderable\n") );
	ntError_p( pRenderable, ("We can only alloc a real dude if we have a valid renderable pointer\n") );

	const Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[pGrunt->m_BattalionID];
	const Unit* pUnit = &pBattalion->m_Units[ pGrunt->m_UnitID ];

	AI* pAI = m_pArmyUnitParameters[ pUnit->m_UnitType ]->m_SpawnPool->Spawn( pCurRenderGrunt->m_Position, pCurRenderGrunt->m_Orientation );
	if( pAI )
	{
		ntError( pRenderable->GetRealDudeEntity() == 0 );
		pRenderable->TurnIntoRealDude( pAI );
		pRenderable->GetRootTransform()->SetLocalTranslation( pCurRenderGrunt->m_Position );
		pRenderable->GetRootTransform()->SetLocalRotation( pCurRenderGrunt->m_Orientation );
		pGrunt->m_GruntMajorState = GMS_REAL_DUDE;
		pGrunt->m_RenderType = GRT_REAL_DUDE;
		pCurRenderGrunt->m_RenderType = GRT_REAL_DUDE;
		pGrunt->m_RenderStateChangeCounter = MAX_RENDERSTATECHANGECOUNTER_COUNTER;
	}

}
void ArmySection::DeallocateRealDude( ArmyRenderable* pRenderable, GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt )
{
	// note real dude fake 
	ntError_p( pGrunt->m_RenderType == GRT_REAL_DUDE, ("We can only dealloc a real dude if we are currently a real dude\n") );

	if( pGrunt->m_RenderStateChangeCounter != 0 )
		return;

	// important note the actual AI despawn is done outside here, this is really just internal army book keeping...

	// we will put some state back into the AI to army state
	// position state TODO orientation
	// assume on battlefield if not, position will reset to where we were before the AI took over
	BF_WorldSpaceToBF_Position( pRenderable->LastKnownAIPosition(), m_pBattlefield->m_pHeader, &pGrunt->m_Position );
	pCurRenderGrunt->m_Position = pRenderable->LastKnownAIPosition();
	float rx,ry,rz;
	CCamUtil::EulerFromQuat_ZXY( pRenderable->LastKnownAIOrientation(), rx, ry, rz );
	ry = ry / (2.f * PI);
	ry = -ry * 65535.f;
	pGrunt->m_Orientation.m_Rotation = (uint16_t) ry;

	// dead or alive state
	if( pRenderable->LastKnownAIStateWasDead() )
	{
		pGrunt->m_GruntMajorState = GMS_DEAD;
		pGrunt->m_AnimToPlayBits = (1 << GAM_DEAD);
		m_pBattlefield->m_pHeader->m_eaInfo->m_iCurDead++;
		m_pBattlefield->m_pHeader->m_eaInfo->m_iTotalDeadCount++;
	} else
	{
		pGrunt->m_GruntMajorState = GMS_NORMAL;
		pGrunt->m_AnimToPlayBits = (1 << GAM_TAUNT);
	}

	pRenderable->GetRootTransform()->SetLocalTranslation( pCurRenderGrunt->m_Position );
	pRenderable->GetRootTransform()->SetLocalRotation( pCurRenderGrunt->m_Orientation );
	pRenderable->EnableRendering();
	pGrunt->m_RenderType = GRT_RENDERABLE;
	pCurRenderGrunt->m_RenderType = GRT_RENDERABLE;
	pGrunt->m_RenderStateChangeCounter = MAX_RENDERSTATECHANGECOUNTER_COUNTER;
}

void ArmySection::DeallocateRenderable( ArmyRenderable* pRenderable, GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt )
{
	// early out if we can't dealloc a real dude
	if( pCurRenderGrunt->m_bRealDudeAllocFail || pGrunt->m_RenderStateChangeCounter != 0 )
	{
		return;
	}

	ntError_p( pCurRenderGrunt->m_RenderType == GRT_RENDERABLE, ("We can only dealloc a rendeable if we are currently a renderable Grunt ID 0x%x\n", pCurRenderGrunt->m_GruntID ) );
	ntError_p( pGrunt->m_GruntMajorState != GMS_REAL_DUDE, ("Eeek, real dude deallocating renderable is bad!") );

	ntError( pRenderable->GetRealDudeEntity() == 0 );
	// add back to the usable pools (turning stuff off first)
	pRenderable->StopAnimation();
	pRenderable->DisableRendering();
	m_pBattlefield->m_pRenderPools[ pCurRenderGrunt->m_UnitType ]->Deallocate( pRenderable );
	m_pBattlefield->m_pRenderables[ pCurRenderGrunt->m_GruntID ] = 0;

	pGrunt->m_RenderType = GRT_SPRITE;
	pCurRenderGrunt->m_RenderType = GRT_SPRITE;
	pGrunt->m_RenderStateChangeCounter = MAX_RENDERSTATECHANGECOUNTER_COUNTER;

	uint8_t iFlagIndex = m_pFlagArray[pCurRenderGrunt->m_GruntID];
	if( iFlagIndex != 0xFF )
	{
		pRenderable->SetFlagBearer( false );
		ntError( pCurRenderGrunt->m_GruntID == m_ArmyFlags[ iFlagIndex ].m_GruntID );
		ArmyRenderable* pFlagPole = m_ArmyFlags[ iFlagIndex].m_pFlagPole;

		CDirection poleVec( 0, m_pArmyGenericParameters->m_fFlagPoleVerticalOffset, 0 );

		if( pFlagPole->GetRootTransform()->GetParent() )
		{
			pFlagPole->GetRootTransform()->RemoveFromParent();
			CHierarchy::GetWorld()->GetRootTransform()->AddChild( pFlagPole->GetRootTransform() );
		}
		pFlagPole->GetRootTransform()->SetLocalTranslation( pCurRenderGrunt->m_Position + poleVec );

	}
}


void ArmySection::AllocateRenderables( const float fTimeStep )
{
	CGatso::Start( "ArmySection::AllocateRenderables" );

	GruntGameState* pFirstGrunt = (GruntGameState*)(m_pBattlefield->m_pGrunts+1);
	uint16_t*		pGruntIndices = (uint16_t*)(pFirstGrunt + m_pBattlefield->m_pGrunts->m_iNumGrunts);

	// DO NOT CHANGE THIS UPDATE ORDER OF DEALLOC and ALLOCS!!!

	// okay lets start by deallocating whats been asked this frame
	ArmyGruntRenderAllocator* pRenderAllocator = m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator;

	// real dude dealloc
	for( uint16_t i=0; i < pRenderAllocator->m_iNumRealDudeToDeAlloc;i++)
	{
		GruntRenderState*	pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[ pRenderAllocator->m_RealDudeDeAllocIds[i] ]; 
		GruntGameState*		pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];
		ArmyRenderable*		pRenderable = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ];
		ntError( pRenderGrunt->m_GruntID == pGrunt->m_GruntID );
		ntError_p( pRenderable, ("Deallocing a real dude requires a renderable but its NULL Grunt ID = 0x%x\n", pRenderGrunt->m_GruntID) );

		AI* pAI = pRenderable->GetRealDudeEntity();
		if( pAI )
		{
			if( pAI->CanRemoveFromWorld() && pAI->RemoveFromWorld( false ) )
			{
				DeallocateRealDude( pRenderable, pGrunt, pRenderGrunt );
			} else
			{
				// skip any requested renderable dealloc
				pRenderGrunt->m_bRealDudeAllocFail = 1;
			}
		} else
		{
			// okay looks like the AI code has asked to remove this guy, so we need to deallocate our stuff
			DeallocateRealDude( pRenderable, pGrunt, pRenderGrunt );
		}
	}

	// dealloc renderable
	for( uint16_t i=0; i < pRenderAllocator->m_iNumRenderablesToDeAlloc;i++)
	{
		GruntRenderState*	pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[ pRenderAllocator->m_RenderableDeAllocIds[i] ]; 
		GruntGameState*		pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];
		ArmyRenderable*		pRenderable = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ];
		ntError( pRenderGrunt->m_GruntID == pGrunt->m_GruntID );
		ntError_p( pRenderable, ("Deallocing a renderable but its NULL Grunt ID = 0x%x\n", pRenderGrunt->m_GruntID) );
		DeallocateRenderable( pRenderable, pGrunt, pRenderGrunt );
	}

	// now alloc pass

	// now alloc renderables
	for( uint16_t i=0; i < pRenderAllocator->m_iNumRenderablesToAlloc;i++)
	{
		GruntRenderState*	pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[ pRenderAllocator->m_RenderableAllocIds[i] ]; 
		GruntGameState*		pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];
		ArmyRenderable*		pRenderable = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ];
		ntError( pRenderGrunt->m_GruntID == pGrunt->m_GruntID );
		UNUSED( pRenderable );
		ntError_p( pRenderable == 0 , ("Aallocing a renderable but its not NULL Grunt ID = 0x%x\n", pRenderGrunt->m_GruntID) );
		AllocateRenderable( pGrunt, pRenderGrunt );
	}

	// now alloc real dude
	for( uint16_t i=0; i < pRenderAllocator->m_iNumRealDudeToAlloc;i++)
	{
		GruntRenderState*	pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[ pRenderAllocator->m_RealDudeAllocIds[i] ]; 
		GruntGameState*		pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];
		ArmyRenderable*		pRenderable = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ];
		ntError( pRenderGrunt->m_GruntID == pGrunt->m_GruntID );
		AllocateRealDude( pRenderable, pGrunt, pRenderGrunt );
	}

	CGatso::Stop( "ArmySection::AllocateRenderables" );
}
void ArmySection::UpdatePreviousFrameWork( const float fTimeStep )
{
#if !defined(_NO_DBGMEM_OR_RELEASE)
	if( m_iDebugMode )
	{
		float fY=60.0f;
		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Body count=%i\n", m_pBattlefield->m_pHeader->m_eaInfo->m_iCurDead );		
		fY+=12.0f;

		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Real dude Deallocs%i\n", m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator->m_iNumRealDudeToDeAlloc );		
		fY+=12.0f;
		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Real dude Allocs%i\n", m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator->m_iNumRealDudeToAlloc );		
		fY+=12.0f;
		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Renderable Deallocs%i\n", m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator->m_iNumRenderablesToDeAlloc );		
		fY+=12.0f;
		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Renderable Allocs%i\n", m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator->m_iNumRenderablesToAlloc );		
		fY+=12.0f;

		uint32_t iNumLines = *m_pBattlefield->m_pHeader->m_iNumDebugLines;
		// draw the battalion extents
		for( uint32_t i=0;i < iNumLines;++i )
		{
			const SPU_DebugLine* line = &m_pBattlefield->m_pHeader->m_pDebugLineBuffer[i];
			g_VisualDebug->RenderLine( line->a, line->b, line->col );
		}
		AtomicSet(  m_pBattlefield->m_pHeader->m_iNumDebugLines, 0 );

		if( m_iDebugMode == 1 )
		{
			g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Army Line Draw Debug\n" );		
			fY+=12.0f;

			// draw event circles
			for( uint8_t i=0;i < m_pBattlefield->m_pHeader->m_iNumEvents;++i)
			{
				BattlefieldEvent* pEvent = &m_pBattlefield->m_pHeader->m_Events[i];
				CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
				ArcMatrix.SetTranslation( BF_PositionToWorldSpace(&pEvent->m_EventLocation, m_pBattlefield->m_pHeader) );
				g_VisualDebug->RenderArc( ArcMatrix, BF_ApproxDistanceToWorldSpace( pEvent->m_iRadius, m_pBattlefield->m_pHeader ), TWO_PI, DC_RED );
			}

			// draw event circles
			for( uint8_t i=0;i < m_pBattlefield->m_pHeader->m_iNumObstacles;++i)
			{
				BattlefieldObstacle* pObs = &m_pBattlefield->m_pHeader->m_Obstacles[i];
				CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
				ArcMatrix.SetTranslation( BF_PositionToWorldSpace(&pObs->m_Location, m_pBattlefield->m_pHeader) );
				g_VisualDebug->RenderArc( ArcMatrix, BF_ApproxDistanceToWorldSpace( pObs->m_Radius, m_pBattlefield->m_pHeader ), TWO_PI, DC_BLUE );
			}
		} else
		if( m_iDebugMode == 2 )
		{
			g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Army State Debug\n" );		
			fY+=12.0f;

			GruntGameState* pFirstGrunt = (GruntGameState*)(m_pBattlefield->m_pGrunts+1);
			for( uint16_t i=0;i < m_pBattlefield->m_pGrunts->m_iNumGrunts;++i)
			{
				GruntRenderState* pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[i];
				uint16_t*		pGruntIndices = (uint16_t*)(pFirstGrunt + m_pBattlefield->m_pGrunts->m_iNumGrunts);
				GruntGameState* pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];

				char buf[256] = "";

				switch( pGrunt->m_GruntMajorState )
				{
				case GMS_NORMAL:		strcpy( buf, "" ); break;
				case GMS_SCARED:		strcpy( buf, "S" ); break;
				case GMS_DEAD:			strcpy( buf, "D" ); break;
				case GMS_DYING:			strcpy( buf, "DY" ); break;
				case GMS_DIVING:		strcpy( buf, "DIV" ); break;
				case GMS_GETTING_UP:	strcpy( buf, "GU" ); break;
				case GMS_KOED:			strcpy( buf, "K" ); break;
				case GMS_BLOCK:			strcpy( buf, "B" ); break;
				case GMS_RECOIL:		strcpy( buf, "RC" ); break;
				case GMS_REAL_DUDE:		
				{
					strcpy( buf, "RD" );
					AI* pAI = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->GetRealDudeEntity();
					strcat( buf, (pAI && pAI->IsHidden() ? " H" : "") ); 					
				}
				break;
				default: strcpy( buf, "EEK!!" ); break;
				}

				switch( pGrunt->m_RenderType )
				{
				case GRT_SPRITE: break;
				case GRT_REAL_DUDE: strcat( buf, "A" ); break;
				case GRT_RENDERABLE:
				{
					if( m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ] != 0 )
					{
						strcat( buf, (!m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->IsRenderingDisabled() ? "V" : "R") ); 
						CDirection diff = pRenderGrunt->m_Position ^ m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->GetRootTransform()->GetLocalTranslation();
						if( diff.LengthSquared() > 1.f )
						{
							strcat( buf, "EEK" );
//							sprintf( buf, "%s - M<%f, %f> RG<%f, %f>", buf, m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->GetRootTransform()->GetLocalTranslation().X(),
//																			m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->GetRootTransform()->GetLocalTranslation().Z(),
//																			pRenderGrunt->m_Position.X(),
//																			pRenderGrunt->m_Position.Z() );
//							pRenderGrunt->m_Position = m_pBattlefield->m_pRenderables[ pRenderGrunt->m_GruntID ]->GetRootTransform()->GetLocalTranslation();
						}
					}
				}
				break;
				}
				g_VisualDebug->Printf3D( pRenderGrunt->m_Position, 0, 1, DC_RED, 0, buf );
			}
		}
	}
#endif
	AllocateRenderables( fTimeStep );

	GruntGameState* pFirstGrunt = (GruntGameState*)(m_pBattlefield->m_pGrunts+1);
	uint16_t*		pGruntIndices = (uint16_t*)(pFirstGrunt + m_pBattlefield->m_pGrunts->m_iNumGrunts);

	// We need to update the hierarchy root BEFORE we batch-update
	// the animators - this is because the batch updater on SPUs will
	// update the hierarchy world-matrices for free (pretty much), so
	// we might as well take advantage of this!

	CGatso::Start( "ArmySection::FirstPastUpdate" );

	// sprite and anim update system
	for( int i=0;i < m_pBattlefield->m_pGrunts->m_iNumGrunts;i++)
	{
		const GruntRenderState* pCurRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[i];
		ArmyRenderable* pRenderable = m_pBattlefield->m_pRenderables[pCurRenderGrunt->m_GruntID];
		// we will put some state back into the game fro now on in
		GruntGameState* pGrunt = pFirstGrunt + pGruntIndices[ pCurRenderGrunt->m_GruntID ];
		if( pGrunt->m_RenderStateChangeCounter > 0 )
		{
			pGrunt->m_RenderStateChangeCounter--;
		}

		if( pRenderable == 0 )
		{
			ntError_p( pGrunt->m_GruntMajorState != GMS_REAL_DUDE, ("Eeek, real dude without a renderable thats bad mmmkay!") );
			continue;
		}


		// lets check for AI removing real dudes from us
		if( pGrunt->m_GruntMajorState == GMS_REAL_DUDE )
		{
			AI* pAI = pRenderable->GetRealDudeEntity();
			if( pAI == 0 )
			{
				// okay looks like the AI code has asked to remove this guy, so we need to deallocate our stuff
				DeallocateRealDude( pRenderable, pGrunt, const_cast<GruntRenderState*>(pCurRenderGrunt) );
			} else
			{
				BF_WorldSpaceToBF_Position( pAI->GetPosition(), m_pBattlefield->m_pHeader, &pGrunt->m_Position );
			}
		}/* else
		{
			pRenderable->EnableRendering();
		}*/

		if( pGrunt->m_AnimToPlayBits != 0 )
		{
			for( int i=0;i < MAX_GRUNT_ANIM;i++)
			{
				bool bWantToPlaying = (pGrunt->m_AnimToPlayBits & (1 << i) );
				if( bWantToPlaying )
				{
					// start playing an anim
					pRenderable->PlayAnimation( (GruntAnimState) i );
					break;
				}
			}
			pGrunt->m_AnimToPlayBits = 0;
		}

		pRenderable->GetRootTransform()->SetLocalSpace( pCurRenderGrunt->m_Orientation, pCurRenderGrunt->m_Position );

	}
	CGatso::Stop( "ArmySection::FirstPastUpdate" );


	for( int i=0;i < MAX_UNIT_TYPES;i++)
	{
		if( m_pBattlefield->m_pRenderPools[ i ] != 0 )
		{
			m_pBattlefield->m_pRenderPools[ i ]->ResetSprites();
		}
	}

	BatchUpdateAnims( fTimeStep );


	CGatso::Start( "ArmySection::2ndPastUpdate" );
	// post batch anim update system (sprites now done at the same time at batch anim update)
	for( int i=0;i < m_pBattlefield->m_pGrunts->m_iNumGrunts;i++)
	{
		GruntRenderState* pRenderGrunt = &m_pBattlefield->m_pRenderDestBuffer[i];
		ArmyRenderable* pRenderable = m_pBattlefield->m_pRenderables[pRenderGrunt->m_GruntID];
		GruntGameState* pGrunt = pFirstGrunt + pGruntIndices[ pRenderGrunt->m_GruntID ];

		if( pRenderable != 0 )
		{
			pRenderable->UpdatePartTwo();

			// we will put some state back into the game fro now on in
			// TODO replace with 1 << GetAnimPlaying() while we don't do any blending
			pGrunt->m_AnimPlayingBits = 0;
			for( int i=0;i < MAX_GRUNT_ANIM;i++)
			{
				pGrunt->m_AnimPlayingBits |= pRenderable->IsAnimPlaying( (GruntAnimState) i ) << i;
			}
		}
	}
	CGatso::Stop( "ArmySection::2ndPastUpdate" );

	// okay let scan any update event flags back out to the game
	for( uint32_t i=0;i < m_pBattlefield->m_pHeader->m_iNumEvents;++i )
	{
		if( m_EventUpdates[i]->m_iDec <= 0 )
		{
			// okay we have a update hit, send a message back
			if( m_pBattlefield->m_pHeader->m_Events[i].m_iType == BET_BAZOOKA_SHOT )
			{
				// k its a bazooka shot event, so tell the bazooka to explode
				Object_Projectile* pobRocket = (Object_Projectile*)CEntityManager::Get().FindEntity( CHashedString((CHashedString::HashKeyType) m_EventUpdates[i]->m_iData ) );
				if ( pobRocket )
				{
					pobRocket->TGSDestroyAllProjectiles();
				}
			}
		}
	}

	NT_MEMCPY( m_pBattlefield->m_pHeader->m_Events, m_pEvents, sizeof( BattlefieldEvent ) * m_iNumEvents );
	m_pBattlefield->m_pHeader->m_iNumEvents = (uint8_t) m_iNumEvents;
	for( uint32_t i=0;i < m_iNumEvents;++i )
	{
		// update potentially for next frame...
		m_EventUpdates[i]->m_iDec = m_EventUpdates[i]->m_iOrigDec;
		m_EventUpdates[i]->m_iData = m_EventUpdates[i]->m_iOrigData;
	}
	m_iNumEvents = 0; // reset for next frame

	// copy camera and player data
	m_pBattlefield->m_pHeader->m_CameraPos = m_CameraPos;
	m_pBattlefield->m_pHeader->m_CameraFacing = m_CameraFacing;
	m_pBattlefield->m_pHeader->m_CameraRadius = m_CameraRadius;
	m_pBattlefield->m_pHeader->m_PlayerPos = m_PlayerPos;
	m_pBattlefield->m_pHeader->m_PlayerRadius = m_PlayerRadius;
	
	BF_WorldSpaceToBF_Position( m_PlayerPos, m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_PlayerBFPos );
	m_pBattlefield->m_pHeader->m_PlayerBFRadius = (uint16_t) CalculuteRadiusInBFSpace( m_PlayerRadius );

	// update any relative commands
	ArmyBattlefield::ArmyBattalionDefList::const_iterator bdlIt = m_pArmyArena->m_ArmyBattalionDefs.begin();
	for( unsigned int i = 0;
		 bdlIt != m_pArmyArena->m_ArmyBattalionDefs.end(); ++bdlIt, ++i )
	{
		const ArmyBattalion* pBattalionDef = (*bdlIt);
		Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[i];
		if( pBattalion->m_BattalionFlags & BF_RELATIVE_COMMANDS )
		{
			ntError( pBattalionDef->m_pParentEntity );
			BF_WorldSpaceToBF_Position(  pBattalionDef->m_pParentEntity->GetPosition(), m_pBattlefield->m_pHeader, &pBattalion->m_RelativePnt );
		}
	}
}

//------------------------------------------------------
//!
//! Called once per frame, to tick the army section
//!
//------------------------------------------------------
void ArmySection::Update( float fTimeStep )
{
	if( m_pBattlefield == 0 || m_pBattlefield->m_pBattalions->m_iNumBattalions == 0 || m_pBattlefield->m_pGrunts->m_iNumGrunts == 0)
		return;

#if defined( DEBUG_SAFETY )
//	army_task.StallForJobToFinish();
	Exec::FrameReset();
	Exec::FrameEnd();
#endif

	CGatso::Start( "ArmySection::Update" );

	if( m_bFirstFrameDone == true )
	{
		UpdatePreviousFrameWork( fTimeStep );
	} else
	{
		m_bFirstFrameDone = true;
	}

	// get the player (if in the level) and add a tracking event
	Player* pPlayer = CEntityManager::Get().GetPlayer();
	if( pPlayer )
	{
		AddEvent( BET_PLAYER_TRACKER, pPlayer->GetPosition(), -1, 0, 0 );
	}

	// update time in 30hz (if ur wondering its cos lots of timing are done with 16 bit ints not floats)
	m_pBattlefield->m_pHeader->m_deltaTime = fTimeStep * 30;
	m_pBattlefield->m_pHeader->m_accumTime += fTimeStep * 30;

#if !defined( _RELEASE )
	UpdateFromWelder();
#endif

	CGatso::Stop( "ArmySection::Update" );

}

void ArmySection::UpdateKickSPUs( float fTimeStep )
{
	// we can get a frame where we aren't yet ready cos of the change in update order
	// to get more SPU overlap... this guards against it benignly.
	if( m_pArmyAdder == 0 )
		return;

	// by now general specific (PPU) behaviour should have been done in the derived Update
	// so we assume that we just have to kick off the SPU task now

	SPUTask army_task( ElfManager::Get().GetProgram( ARMY_SPU_ELF ) );
	SPUTask army2_task( ElfManager::Get().GetProgram( ARMY2_SPU_ELF ) );

	Exec::PrepareSPUJobAdder( &army2_task, m_pArmyAdder );
	DMABuffer pArmyAdderDMA( m_pArmyAdder, DMABuffer::DMAAllocSize( sizeof( ExecSPUJobAdder ) ) );

	// need to send some parameter to the render_task... so an easy way is to stuff
	// some field in the spare data fields
	m_pArmyAdder->m_data0[ RJAP_DEST_GRUNTS_EA ] = (uint32_t) m_pBattlefield->m_pRenderDestBuffer;

	DMABuffer pBattalionDMA( m_pBattlefield->m_pBattalions, DMABuffer::DMAAllocSize( sizeof(BattalionArray) ) );
	DMABuffer pHeaderDMA( m_pBattlefield->m_pHeader, DMABuffer::DMAAllocSize( sizeof(BattlefieldHeader) ) );
	DMABuffer pGruntsDMA( m_pBattlefield->m_pGrunts, DMABuffer::DMAAllocSize( m_pBattlefield->m_pGrunts->m_iSize ) );
	DMABuffer pChunksHeaderDMA( m_pBattlefield->m_pChunksHeader, DMABuffer::DMAAllocSize( sizeof(BattlefieldChunksHeader) ) );

	// do chunkify before battalion means a potential frame delay for some things but probably safer this way
	Exec::InitDependency( m_pSortDependency, 1 );

	// NOTE pGrunts is actually an inputOutput buffer BUT we put it back manually to maximise conconurracy
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) GRUNT_CHUNKIFY_AND_SORT ), SRTA_RUN_TYPE );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pBattalionDMA ),		SRTA_BATTALION_ARRAY );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pHeaderDMA ),			SRTA_BATTLEFIELD_HEADER );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pGruntsDMA ),			SRTA_GRUNTS );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pChunksHeaderDMA ),	SRTA_BATTLEFIELD_CHUNKS_HEADER );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pArmyAdderDMA ),		SRTA_ARMY_JOB_ADDER );
	army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t)m_pRenderDependency ),		SRTA_ARMY_BARRIER_EA );

	// after GRUNT_CHUNKIFY_AND_SORT has finished (and dma's finished) we can move onto tasks its generated
	army_task.RequestDependencyDecrement( m_pSortDependency );
	Exec::RunTask( &army_task ); 

	Exec::AddBarrierJob( m_pSortDependency );


	// first thing to do is battallion logic, one task per battalion
	Exec::InitDependency( m_pBattalionDependency, m_pBattlefield->m_pBattalions->m_iNumBattalions );
	for( uint8_t i=0;i < m_pBattlefield->m_pBattalions->m_iNumBattalions;++i )
	{
		// note each battalion task will update its bit of the battalion array, hence the depenceny to the GRUNT_LOGIC
		// but not the GRUNT CHUNKIFY (which isn't dependent on the battalion orders)
		army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) BATTALION_UPDATE ), SRTA_RUN_TYPE );
		army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pBattalionDMA ),				SRTA_BATTALION_ARRAY );
		army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pHeaderDMA ),					SRTA_BATTLEFIELD_HEADER );
		army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pGruntsDMA ),					SRTA_GRUNTS );
		army_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) i ),				SRTA_BATTALION_ID );
		army_task.RequestDependencyDecrement( m_pBattalionDependency );
		Exec::RunTask( &army_task ); 
	}

	// make the task manager wait until GRUNT_CHUNKIFY_AND_SORT and BATTALION_UPDATEs has finished
	Exec::AddBarrierJob( m_pBattalionDependency );

}

void ArmySection::Render()
{
	/*
	// ----- Audio test -----

	float fY=50.0f;

	// Count how many soldiers are remaining
	int iTotalGrunts=m_pBattlefield->m_pGrunts->m_iNumGrunts;

	int iGruntsRemaining=iTotalGrunts;

	for(int j=0; j<iTotalGrunts; j++)
	{
		if( m_pBattlefield->m_pRenderDestBuffer[j].m_GruntMajorState == GMS_DEAD )
			iGruntsRemaining--;
	}

	float fPercentageRemaining=(float)iGruntsRemaining / (float)iTotalGrunts;

	CPoint obPlayerPosition(CEntityManager::Get().GetPlayer()->GetPosition());

	g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Player position=%.3f,%.3f,%.3f\n",obPlayerPosition.X(),obPlayerPosition.Y(),obPlayerPosition.Z());		
	fY+=12.0f;

	for(int i=0; i<m_pBattlefield->m_pBattalions->m_iNumBattalions; ++i)
	{
		Battalion* pBattalion=&m_pBattlefield->m_pBattalions->m_BattalionArray[i];

		unsigned int uiBattalionOrders=pBattalion->m_Orders;

	CPoint obTopLeft(BF_PositionToWorldSpace(&pBattalion->m_TopLeft,m_pBattlefield->m_pHeader));
	CPoint obBottomRight(BF_PositionToWorldSpace(&pBattalion->m_BotRight,m_pBattlefield->m_pHeader));

		CPoint obPosition(obTopLeft + obBottomRight);
		obPosition*=0.5f;

		g_VisualDebug->Printf2D(10.0f,fY,0xffffffff,0,"Battalion %d: Orders=%d Position=%.3f,%.3f,%.3f Remaining=%.3f\n",i,uiBattalionOrders,obPosition.X(),obPosition.Y(),obPosition.Z(),fPercentageRemaining);		
		fY+=12.0f;

	}
	*/



#ifdef _ARMY_SECTION_DEBUG_RENDER

	for( unsigned int i = 0;  i < m_pBattlefield->m_pBattalions->m_iNumBattalions; ++i )
	{
		Battalion* pBattalion = &(m_pBattlefield->m_pBattalions->m_BattalionArray[i]);

		CPoint obMin(BF_PositionToWorldSpace( &(pBattalion->m_TopLeft), m_pBattlefield->m_pHeader ));

		CPoint obMax(BF_PositionToWorldSpace( &(pBattalion->m_BotRight), m_pBattlefield->m_pHeader ));

		obMin.Y()=m_PlayerPos.Y();
		obMax.Y()=m_PlayerPos.Y()+4.0f;

		g_VisualDebug->RenderAABB(obMin,obMax,0xffffffff);

		//ntPrintf("Battalion %d: %.1f,%.1f,%.1f -> %.1f,%.1f,%.1f\n",i,obMin.X(),obMin.Y(),obMin.Z(),obMax.X(),obMax.Y(),obMax.Z());

		CPoint obCentre(obMin + obMax);
		obCentre*=0.5f;

		char acOrders [32];

		switch(pBattalion->m_Orders)
		{
			default:
			case BO_NO_CONTROL: strcpy(acOrders,"No control"); break;

			case BO_FORWARD_MARCH: strcpy(acOrders,"Forward march"); break;
			case BO_HOLD_POSITION: strcpy(acOrders,"Hold position"); break;
			case BO_RETREAT: strcpy(acOrders,"Retreat"); break;
			case BO_RUN_FOR_YOUR_LIVES: strcpy(acOrders,"Run for your lives"); break;
			case BO_ATTACK: strcpy(acOrders,"Attack"); break;
			case BO_CHARGE: strcpy(acOrders,"Charge"); break;
			case BO_ROTATE: strcpy(acOrders,"Rotate"); break;
			case BO_RESPAWN: strcpy(acOrders,"Respawn"); break;
			case BO_TAUNT: strcpy(acOrders,"Taunt"); break;
			case BO_RUN_TO_CIRCLE: strcpy(acOrders, "Run to circle"); break;
		}

		int iGruntCount=0;

		for(int unit=0; unit<pBattalion->m_iNumUnits; ++unit)
		{
			Unit* pUnit=&(pBattalion->m_Units[unit]);

			for(unsigned int grunt=pUnit->m_FirstGruntID; grunt<(pUnit->m_FirstGruntID + pUnit->m_NumGrunts); ++grunt)
			{
				GruntGameState* pGrunt = &((GruntGameState*)(m_pBattlefield->m_pGrunts+1))[grunt];

				if (pGrunt->m_GruntMajorState!=GMS_DEAD && pGrunt->m_GruntMajorState!=GMS_REAL_DUDE)
					iGruntCount++;
			}			
		}

		g_VisualDebug->Printf3D(obCentre,0.0f,-24.0f,0xffffffff,DTF_ALIGN_HCENTRE,"#%d: %s",i,acOrders);
		g_VisualDebug->Printf3D(obCentre,0.0f,-12.0f,0xffffffff,DTF_ALIGN_HCENTRE,"Grunts:%d/%d",iGruntCount,pBattalion->m_iNumGrunts);
	}

#endif // _ARMY_SECTION_DEBUG_RENDER


}


void ArmySection::UpdateFromWelder()
{
	for( uint8_t i=0;i < m_pBattlefield->m_pBattalions->m_iNumBattalions;++i )
	{
		Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[i];

		pBattalion->m_fMinInnerPlayerTrackRadius = FLT_MAX;
		pBattalion->m_fMaxOuterPlayerTrackRadius = -FLT_MAX;
		pBattalion->m_MinCirclePlayerRadius = FLT_MAX;

		for( uint8_t j=0;j < pBattalion->m_iNumUnits;++j )
		{
			Unit* pUnit = &pBattalion->m_Units[j];
			const ArmyUnitParameters* pParams = (ArmyUnitParameters*) m_pArmyUnitParameters[ pUnit->m_UnitType ];
			if( pParams )
			{
				pUnit->m_RunSpeed = (uint32_t) (CalculuteRadiusInBFSpace(pParams->m_fRunSpeed) / 30.f);
				pUnit->m_WalkSpeed = (uint32_t) (CalculuteRadiusInBFSpace(pParams->m_fWalkSpeed) / 30.f);
				pUnit->m_DiveSpeed = (uint32_t) (CalculuteRadiusInBFSpace(pParams->m_fDiveSpeed) / 30.f);
				pUnit->m_PersonalSpace = pParams->m_iPersonalSpace;
				pUnit->m_fInnerPlayerTrackRadius = pParams->m_InnerPlayerTrackRadius;
				pUnit->m_fOuterPlayerTrackRadius = pParams->m_OuterPlayerTrackRadius;
				pUnit->m_CirclePlayerRadius = pParams->m_CirclePlayerRadius;
			}

			// get the battalion min/max version
			pBattalion->m_fMinInnerPlayerTrackRadius = ntstd::Min( pBattalion->m_fMinInnerPlayerTrackRadius, pUnit->m_fInnerPlayerTrackRadius );
			pBattalion->m_fMaxOuterPlayerTrackRadius = ntstd::Max( pBattalion->m_fMaxOuterPlayerTrackRadius, pUnit->m_fOuterPlayerTrackRadius );

			pBattalion->m_MinCirclePlayerRadius = ntstd::Min( pBattalion->m_MinCirclePlayerRadius, pUnit->m_CirclePlayerRadius );
		}
	}
	ArmyRenderable::s_iVisibleFrameLimit  = (uint32_t) m_pArmyGenericParameters->m_iVisibleFrameLimit;
	ArmyRenderable::s_fMaxRandomDurationVariationAnim = m_pArmyGenericParameters->m_fMaxRandomDurationVariationAnim;

	m_pBattlefield->m_pHeader->m_iPlayerTrackWallInset = (uint16_t) m_pArmyGenericParameters->m_iPlayerTrackWallInset;
	m_pBattlefield->m_pHeader->m_iPlayerTrackWallMultipler = (uint16_t) m_pArmyGenericParameters->m_iPlayerTrackWallMultipler;
	m_pBattlefield->m_pHeader->m_iPlayerTrackWallRandomishFactor = (uint16_t) m_pArmyGenericParameters->m_iPlayerTrackWallRandomishFactor;

	m_pBattlefield->m_pHeader->m_iMaxPeeps = (uint16_t) m_pArmyGenericParameters->m_iMaxPeeps;
	m_pBattlefield->m_pHeader->m_iMaxAIDudes = (uint16_t) m_pArmyGenericParameters->m_iMaxAIDudes;
}


void ArmySection::AddEvent( BATTLEFIELD_EVENT_TYPE type, const CPoint& pnt, float radius, uint32_t iParam0, uint32_t iParam1, uint32_t iUpdateStartDec, uint32_t iUpdateData )
{
	// defend against odd out of order load issues.
	if( !m_pArmyGenericParameters )
		return;

	ntAssert_p( m_iNumEvents < MAX_BATTLEFIELD_EVENTS, ("Too many events this frame\n") );
	if( m_iNumEvents >= MAX_BATTLEFIELD_EVENTS )
	{
//		ntPrintf( "Too many battlefield events\n" );
		return;
	}
	// if the radius <= 0 we use a type specific number
	if( radius <= 0 )
	{
		switch( type )
		{
		case BET_BAZOOKA_SHOT:
		{
			// for bazooka shots param 1 is inner thread radius and as the caller has asked us to 
			// fill in the standard radii, we also override iParam1
			radius = CalculuteRadiusInBFSpace( m_pArmyGenericParameters->m_fBazookaThreatRadius ); 
			iParam1 = (uint32_t) CalculuteRadiusInBFSpace( m_pArmyGenericParameters->m_fBazookaInnerThreatRadius );

            //ntPrintf("Triggering audio for BET_BAZOOKA_SHOT\n");
            
            if (AudioSystem::Get().Sound_IsPlaying(m_uiSoundRocketFireID)) // Sound is already playing
            {
	            AudioSystem::Get().Sound_SetPosition(m_uiSoundRocketFireID,pnt); // Update the position on the sound
            }
            else // Sound is not currently playing
            {
	            if (AudioSystem::Get().Sound_Prepare(m_uiSoundRocketFireID,m_pArmyArena->m_obEventSoundRocketFire.GetString())) // Fire off a new sound
	            {
		            AudioSystem::Get().Sound_SetPosition(m_uiSoundRocketFireID,pnt);
		            AudioSystem::Get().Sound_Play(m_uiSoundRocketFireID);
	            }
            }

			break;
		}
			
		case BET_PLAYER_TRACKER:
		{
				radius = m_pArmyGenericParameters->m_fPlayerInRadius;
				break;
		}
		case BET_EXPLOSION:
			radius = CalculuteRadiusInBFSpace( m_pArmyGenericParameters->m_fBazookaExplodeRadius ); break;
		case BET_SYNC_ATTACK_WAKE:
		case BET_SPEED_ATTACK_WAKE:
		case BET_RANGE_ATTACK_WAKE:
		case BET_POWER_ATTACK_WAKE:
		{
			// Something useful.
			break;
		}
		}
	} else
	{
		radius = CalculuteRadiusInBFSpace( radius );
	}

	// if this event is off the battle field don't add it else stick it in the buffered list for upload next frame
	bool onbfield = BF_WorldSpaceToBF_Position( pnt, m_pBattlefield->m_pHeader, &m_pEvents[ m_iNumEvents ].m_EventLocation );

	// most offfield events are ignored but some times its still important
	// off the field, might have for camera tracker... if it is the camera plonk it in the near end of the battlefield, not
	// got but would need an intersection thing-me-bob that I ain't got time to right at the mo...
	if( !onbfield )
	{
		m_pEvents[ m_iNumEvents ].m_EventLocation.m_X = 65536/2;
		m_pEvents[ m_iNumEvents ].m_EventLocation.m_Y = 65536;
	}

	switch( type )
	{
	case BET_PLAYER_TRACKER:
		m_PlayerPos = pnt;
		m_PlayerRadius = radius;
		break;
	default:
		if( !onbfield )
			return;
		m_EventUpdates[ m_iNumEvents ]->m_iOrigDec = iUpdateStartDec;
		m_EventUpdates[ m_iNumEvents ]->m_iOrigData = iUpdateData;
		m_pBattlefield->m_pHeader->m_eaEventUpdates[ m_iNumEvents ] = m_EventUpdates[ m_iNumEvents ];
		m_pEvents[ m_iNumEvents ].m_iRadius = (uint32_t) radius;
		m_pEvents[ m_iNumEvents ].m_iType = type;
		m_pEvents[ m_iNumEvents ].m_iParam0 = iParam0;
		m_pEvents[ m_iNumEvents ].m_iParam1 = iParam1;
		m_iNumEvents++;
		break;
	}
}

float ArmySection::CalculuteRadiusInBFSpace( const float radius )
{
	CPoint pnt(	m_pBattlefield->m_pHeader->m_WorldTopLeft.X() + m_pBattlefield->m_pHeader->m_WorldHoriz / 2.f, 
					m_pBattlefield->m_pHeader->m_WorldTopLeft.Y() + m_pBattlefield->m_pHeader->m_WorldHeight / 2.f,
					m_pBattlefield->m_pHeader->m_WorldTopLeft.Z() + m_pBattlefield->m_pHeader->m_WorldVert / 2.f );

	BF_Position tmpRad;
	bool onbfield2 = BF_WorldSpaceToBF_Position( pnt + CDirection(0, 0,  radius) , m_pBattlefield->m_pHeader, &tmpRad );
	if( !onbfield2 )
	{
		// okay try the other direction
		bool onbfield3 = BF_WorldSpaceToBF_Position( pnt + CDirection(0, 0,  -radius) , m_pBattlefield->m_pHeader, &tmpRad );
		
		// err very small battlefield???
		if( !onbfield3 )
			return 0.f;
	}

	// the point is in the middle of the battledfield which is (65536/2). magic number suprememe
	float tmp = (65536/2) - tmpRad.m_Y;
	if( tmp > 0 )
	{
		return tmp;
	} else
	{
		tmp = tmpRad.m_Y - (65536/2);
		return tmp;
	}

}

void ArmySection::SetCamera( const CPoint& pos, const CDirection& dir )
{
	m_CameraPos = pos;
	m_CameraFacing = dir;
	if( m_pArmyGenericParameters )
	{
		m_CameraRadius = m_pArmyGenericParameters->m_fCamaraFarRadius; // upto 32K
	} else
	{
		m_CameraRadius = 0;
	}
}

unsigned int ArmySection::GetBodyCount()
{
	if( m_pBattlefield == 0)
	{
		return 0;
	}
	return m_pBattlefield->m_pHeader->m_eaInfo->m_iCurDead;
}

int ArmySection::GetUnitsAlive (const CPoint& obPosition, float fRadius)
{
	if( m_pBattlefield == 0)
	{
		return 0;
	}

	// Iterate through every grunt on the battlefield...
	const unsigned int iTotalGrunts=m_pBattlefield->m_pGrunts->m_iNumGrunts;

	const float fRadiusSqrd=fRadius*fRadius;

	float fPX=obPosition.X();
	float fPZ=obPosition.Z();

	int iNumGrunts=0;

	for(unsigned int i=0; i<iTotalGrunts; ++i)
	{
		GruntGameState* pGrunt = &((GruntGameState*)(m_pBattlefield->m_pGrunts+1))[i];

		if (pGrunt->m_GruntMajorState!=GMS_DEAD) // Make sure the grunt is alive
		{
			CPoint obGruntPos(BF_PositionToWorldSpace( &(pGrunt->m_Position), m_pBattlefield->m_pHeader ));

			float fDX=fabsf(obGruntPos.X() - fPX);
			float fDZ=fabsf(obGruntPos.Z() - fPZ);
			float fDistSqrd=fDX*fDX + fDZ* fDZ;

			if (fDistSqrd < fRadiusSqrd) // Find out if the grunt is inside the given area
			{
				iNumGrunts++;
			}
		}
	}

	return iNumGrunts;
}
