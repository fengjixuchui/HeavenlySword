//------------------------------------------------------
//!
//!	\file army/armymanager.cpp
//!
//------------------------------------------------------

#include "army/army_section.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/grunt.h"
#include "army/army_chap0_0.h"
#include "army/army_chap1_1.h"
#include "army/army_chap1_3.h"
#include "army/army_chap5_1.h"
#include "army/army_chap5_2.h"
#include "army/army_chap5_3.h"
#include "army/army_chap5_4.h"
#include "army/army_chap5_5.h"
#include "army/army_chap6_1.h"

// SPU-exec related includes
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/sputask_ps3.h"

#include "army/armymanager.h"
#include "army/armydef.h"

#include "core/visualdebugger.h"
#include "core/timer.h"
#include "objectdatabase/dataobject.h"
#include "audio/audiosystem.h"




//------------------------------------------------------
//!
//! Just initialising things to zero.
//!
//------------------------------------------------------
ArmyManager::ArmyManager() :
	m_pSection(0),
	m_pAudioDef(0)
{
	// check some assumptions
	static_assert( sizeof(GruntGameState) == 32, GruntGameState_not_32_bytes );
	static_assert( sizeof(GruntRenderState) == 48, GruntRenderState_not_48_bytes );
	static_assert( sizeof(ShapeOrderedLineData) == 8, ShapeOrderedLineData_not_8_bytes );
	static_assert( sizeof(Unit) <= 8, Unit_more_than_8_bytes );
	static_assert( sizeof(Battalion) <= 256, Battalion_more_than_256_bytes );
	static_assert( sizeof(BattalionArray) <= 16384, BattalionArray_more_than_16_kilo_bytes );
	static_assert( sizeof(BattlefieldHeader) <= 1024, BattlefieldHeader_more_than_1_kilo_byte );
	static_assert( sizeof(BattlefieldInfo) == 128, BattlefieldInfo_not_128_bytes );
	static_assert( sizeof(ArmyGruntRenderAllocator) == 2048, ArmyGruntRenderAllocator_not_2_kiko_bytes );

}

//------------------------------------------------------
//!
//! Any non null pointer ArmySection is deleted
//!
//------------------------------------------------------
ArmyManager::~ArmyManager()
{
	DeleteCurrentSection();
}

//------------------------------------------------------
//!
//! Any non null pointer ArmySection is deleted
//!
//------------------------------------------------------
void ArmyManager::DeleteCurrentSection()
{
	if( m_pSection )
	{
		NT_DELETE( m_pSection );
		m_pSection = 0;
	}

}


//------------------------------------------------------
//!
//! Force materials on all army renderables
//!
//------------------------------------------------------
void ArmyManager::ForceMaterials( CMaterial const* pobMaterial )
{
	if( m_pSection )
	{
		m_pSection->ForceMaterials( pobMaterial );
	}

}


//------------------------------------------------------
//!
//! Update all the army stuff if we have a section
//!
//------------------------------------------------------
void ArmyManager::Update( float fTimeStep )
{
	if( m_pSection )
	{
		m_pSection->Update(fTimeStep);

		//if (m_pAudioDef)
		//	m_pAudioDef->Update(m_pSection);
	}

}
//------------------------------------------------------
//!
//! Update all the army stuff if we have a section
//!
//------------------------------------------------------
void ArmyManager::UpdateKickSPUs( float fTimeStep )
{
	if( m_pSection )
	{
		m_pSection->UpdateKickSPUs(fTimeStep);
	}

}


//------------------------------------------------------
//!
//! currently a debug renderer
//!
//------------------------------------------------------
void ArmyManager::Render()
{
	if( m_pSection )
	{
		m_pSection->Render();

		if (m_pAudioDef)
			m_pAudioDef->DebugRender();
	}
}

//------------------------------------------------------
//!
//! Any non null pointer ArmySection is deleted
//!
//------------------------------------------------------
void ArmyManager::RegisterSection( ArmySection* pSection )
{
	m_pSection = pSection;
}

//------------------------------------------------------
//!
//! RegisterAudioDef
//!
//------------------------------------------------------
void ArmyManager::RegisterAudioDef (ArmyAudioDefinition* pAudioDef)
{
	m_pAudioDef=pAudioDef;
}

//------------------------------------------------------
//!
// every frame update the bazooka shots position
//!
//------------------------------------------------------
void ArmyManager::SetBazookaShotPosition( const CPoint& pos, CHashedString obRocketName  )
{
	UNUSED( obRocketName );
	// To use...
	/*
	Object_Projectile* pobRocket = (Object_Projectile*)CEntityManager::Get().FindEntity( obRocketName );
	if ( pobRocket )
	{
		pobRocket->TGSDestroyAllProjectiles();
	}
	*/

	if( m_pSection )
	{
		m_pSection->AddEvent( BET_BAZOOKA_SHOT, pos, -1, 0, 0, 1, (uint32_t) obRocketName.GetHash() );
	}
}
//------------------------------------------------------
//!
// explode a bit of the bazooka
//!
//------------------------------------------------------
void ArmyManager::ExplodeBazookaShot( const CPoint& pos )
{
	if( m_pSection )
	{
		// todo proper radius
		m_pSection->AddEvent( BET_EXPLOSION, pos, -1, 0, 0 );

		if (m_pAudioDef)
			m_pAudioDef->ExplosionEvent(m_pSection,pos);
	}
}

//------------------------------------------------------
//!
// explode a bit of the bazooka
//!
//------------------------------------------------------
void ArmyManager::SetCamera( const CPoint& pos, const CDirection& dir )
{
	if( m_pSection )
	{
		// todo proper radius
		m_pSection->SetCamera( pos, dir );
	}
}

//------------------------------------------------------
//!
// Some notifications of combat events
//!
//------------------------------------------------------
void ArmyManager::SyncAttackStrikeVolumeAt( const CPoint& pos, float fRadius )
{
	if( m_pSection )
	{
		m_pSection->AddEvent( BET_SYNC_ATTACK_WAKE, pos, fRadius, 0, 0 );
	}
}
void ArmyManager::SpeedAttackStrikeVolumeAt( const CPoint& pos, float fRadius )
{
	if( m_pSection )
	{
		m_pSection->AddEvent( BET_SPEED_ATTACK_WAKE, pos, fRadius, 0, 0 );
	}
}
void ArmyManager::PowerAttackStrikeVolumeAt( const CPoint& pos, float fRadius )
{
	if( m_pSection )
	{
		m_pSection->AddEvent( BET_POWER_ATTACK_WAKE, pos, fRadius, 0, 0 );
	}
}
void ArmyManager::RangeAttackStrikeVolumeAt( const CPoint& pos, float fRadius )
{
	if( m_pSection )
	{
		m_pSection->AddEvent( BET_RANGE_ATTACK_WAKE, pos, fRadius, 0, 0 );
	}
}

void ArmyManager::Activate( const ArmyBattlefield* pBattlefieldDef )
{
	// check for benign multiple activations
	if(m_pSection != 0)
	{
		Deactivate();
	}

	ntError_p( m_pSection == 0, ("Only one battlefield can be activate at any one time") );
	static const CHashedString s_Chapter0_0("Chapter0_0");
	static const CHashedString s_Chapter1_1("Chapter1_1");
	static const CHashedString s_Chapter1_3("Chapter1_3");
	static const CHashedString s_Chapter5_1("Chapter5_1");
	static const CHashedString s_Chapter5_2("Chapter5_2");
	static const CHashedString s_Chapter5_3("Chapter5_3");
	static const CHashedString s_Chapter5_4("Chapter5_4");
	static const CHashedString s_Chapter5_5("Chapter5_5");
	static const CHashedString s_Chapter6_1("Chapter6_1");

	// activate our army
	const CHashedString secName( pBattlefieldDef->m_SectionName.c_str() );
	ntPrintf( "Army Section Def Registered %s\n", pBattlefieldDef->m_SectionName.c_str() );

	if( secName.GetValue() == s_Chapter0_0.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap0_0(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter1_1.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap1_1(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter1_3.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap1_3(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter5_1.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap5_1(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter5_2.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap5_2(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter5_3.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap5_3(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter5_4.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap5_4(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter5_5.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap5_5(pBattlefieldDef ) );
	} else
	if( secName.GetValue() == s_Chapter6_1.GetValue() )
	{
		ArmyManager::Get().RegisterSection( NT_NEW ArmyChap6_1(pBattlefieldDef ) );
	} else

	{
		ntError_p( false, ("Unknown ArmySection %s\n", pBattlefieldDef->m_SectionName.c_str() ) );
	}
}

void ArmyManager::Deactivate()
{
	DeleteCurrentSection();
}

void ArmyManager::GlobalEventMessage( const CHashedString obGlobalEvent )
{
	if( m_pSection )
	{
		m_pSection->ProcessGlobalEvent( obGlobalEvent );
	}
}

unsigned int ArmyManager::GetBodyCount()
{
	if( m_pSection )
	{
		return m_pSection->GetBodyCount();
	} else
	{
		return 0;
	}
}

COMMAND_RESULT ArmyManager::ToggleDebugMode()
{
	if( m_pSection )
	{
		m_pSection->ToggleDebugMode();
	}

	return CR_SUCCESS;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------


START_CHUNKED_INTERFACE (ArmyAudioDefinition, Mem::MC_MISC)
	PUBLISH_VAR_AS(m_bDebugRender,			DebugRender)
	PUBLISH_VAR_AS(m_fMaxBattalionRange,	MaxBattalionRange)
	PUBLISH_VAR_AS(m_fBlownUpTestRadius,	BlownUpTestRadius)
	PUBLISH_VAR_AS(m_fBlownUpSoundRadius,	BlownUpSoundRadius)
	PUBLISH_VAR_AS(m_obSoundForwardMarch,	SoundForwardMarch)
	PUBLISH_VAR_AS(m_obSoundHoldPosition,	SoundHoldPosition)	
	PUBLISH_VAR_AS(m_obSoundRetreat,		SoundRetreat)	
	PUBLISH_VAR_AS(m_obSoundFlee,			SoundFlee)	
	PUBLISH_VAR_AS(m_obSoundAttack,			SoundAttack)
	PUBLISH_VAR_AS(m_obSoundCharge,			SoundCharge)
	PUBLISH_VAR_AS(m_obSoundTaunt,			SoundTaunt)	
	PUBLISH_VAR_AS(m_obSoundBlownUp,		SoundBlownUp)

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )	
END_STD_INTERFACE


ArmyAudioDefinition::ArmyAudioDefinition () :
	m_bDebugRender(false),
	m_fMaxBattalionRange(250.0f),
	m_fBlownUpTestRadius(50.f),
	m_fBlownUpSoundRadius(1000.0f)
{
	for(int i=0; i<iTOTAL_GROUPS; ++i)
	{
		m_BattalionInfo[i].m_uiSoundID=0;
		m_BattalionInfo[i].m_iTotalUnits=0;
		m_BattalionInfo[i].m_obPosition.Clear();
		m_BattalionInfo[i].m_bInvalidSound=false;
		m_BattalionInfo[i].m_fVolume=0.0f;
	}
}

ArmyAudioDefinition::~ArmyAudioDefinition ()
{
	if (ArmyManager::Exists())
		ArmyManager::Get().RegisterAudioDef(0);
}

void ArmyAudioDefinition::PostConstruct ()
{
	if (ArmyManager::Exists())
		ArmyManager::Get().RegisterAudioDef(this);

	m_bDebugRender=false;
}

bool ArmyAudioDefinition::EditorChangeValue(CallBackParameter /*obItem*/,CallBackParameter /*obValue*/)
{
	return true;
}

void ArmyAudioDefinition::Update (ArmySection* pArmySection)
{
	for(int i=0; i<iTOTAL_GROUPS; ++i) // Iterate through each group
	{
		const char* pcSound=GetSound((BATTALION_ORDERS)i);

		if (pcSound)
		{
			UpdateInfo(pArmySection,(BATTALION_ORDERS)i); // Update information about this battalion group

			const float fTimeChange=CTimer::Get().GetGameTimeChange(); 

			if (m_BattalionInfo[i].m_iTotalUnits>0) // There are active units in this group
			{
				// Ramp up the volume
				if (m_BattalionInfo[i].m_fVolume<1.0f)
				{
					m_BattalionInfo[i].m_fVolume+=fTimeChange;
				}
				else
				{
					m_BattalionInfo[i].m_fVolume=1.0f;
				}
			}
			else // There are no active units in this group
			{
				// Ramp down the volume
				if (m_BattalionInfo[i].m_fVolume>0.0f)
				{
					m_BattalionInfo[i].m_fVolume-=fTimeChange;
				}
				else
				{
					m_BattalionInfo[i].m_fVolume=0.0f;
				}
			}

			if (m_BattalionInfo[i].m_fVolume>0.0f) // Sound should be playing
			{
				if (AudioSystem::Get().Sound_IsPlaying(m_BattalionInfo[i].m_uiSoundID)) // Sound is already playing
				{
					AudioSystem::Get().Sound_SetPosition(m_BattalionInfo[i].m_uiSoundID,m_BattalionInfo[i].m_obPosition); // Update the position of this sound
					AudioSystem::Get().Sound_SetParameterValue(m_BattalionInfo[i].m_uiSoundID,"units",(float)m_BattalionInfo[i].m_iTotalUnits); // Update the units parameter for this sound
					AudioSystem::Get().Sound_SetVolume(m_BattalionInfo[i].m_uiSoundID,m_BattalionInfo[i].m_fVolume);
				}
				else // The sound isn't already playing at the moment, so lets kick it off
				{
					if (AudioSystem::Get().Sound_Prepare(m_BattalionInfo[i].m_uiSoundID,pcSound)) // Kick off the sound
					{
						AudioSystem::Get().Sound_SetPosition(m_BattalionInfo[i].m_uiSoundID,m_BattalionInfo[i].m_obPosition); // Update the position of this sound
						AudioSystem::Get().Sound_SetParameterValue(m_BattalionInfo[i].m_uiSoundID,"units",(float)m_BattalionInfo[i].m_iTotalUnits); // Update the units parameter for this sound
						AudioSystem::Get().Sound_SetVolume(m_BattalionInfo[i].m_uiSoundID,m_BattalionInfo[i].m_fVolume);
						AudioSystem::Get().Sound_Play(m_BattalionInfo[i].m_uiSoundID); // Begin playing
					}
				}
			}
			else // Sound should be stopped
			{
				AudioSystem::Get().Sound_Stop(m_BattalionInfo[i].m_uiSoundID);
				m_BattalionInfo[i].m_uiSoundID=0;
			}
		}
	}
}

void ArmyAudioDefinition::DebugRender ()
{
#ifndef _RELEASE

	if (!m_bDebugRender)
		return;

	float fX=10.0f;
	float fY=10.0f;
	
	for(int i=0; i<iTOTAL_GROUPS; ++i)
	{
		const char* pcSound=GetSound((BATTALION_ORDERS)i);

		const char* pcOrder;

		switch((BATTALION_ORDERS)i)
		{
			case BO_FORWARD_MARCH:			pcOrder="Forward March"; break;
			case BO_HOLD_POSITION:			pcOrder="Hold Position"; break;
			case BO_RETREAT:				pcOrder="Retreat      "; break;
			case BO_RUN_FOR_YOUR_LIVES:		pcOrder="Flee         "; break;
			case BO_ATTACK:					pcOrder="Attack       "; break;
			case BO_CHARGE:					pcOrder="Charge       "; break;
			case BO_TAUNT:					pcOrder="Taunt        "; break;
			case BO_RUN_TO_CIRCLE:			pcOrder="Run to Circle"; break;
			default:						pcOrder="???          "; break;
		}

		if (pcSound)
		{
			g_VisualDebug->Printf2D(fX,fY,0xffffffff,0,"Order %s: Sound:%s  Sound ID=%d  Vol=%.2f  Units=%d  Position=%.1f,%.1f,%.1f",
				pcOrder,pcSound,m_BattalionInfo[i].m_uiSoundID,m_BattalionInfo[i].m_fVolume,m_BattalionInfo[i].m_iTotalUnits,
				m_BattalionInfo[i].m_obPosition.X(),m_BattalionInfo[i].m_obPosition.Y(),m_BattalionInfo[i].m_obPosition.Z());

			fY+=12.0f;

			CPoint obCentre(m_BattalionInfo[i].m_obPosition);
			g_VisualDebug->RenderLine(CPoint(obCentre.X()+5.0f,obCentre.Y(),obCentre.Z()),CPoint(obCentre.X()-5.0f,obCentre.Y(),obCentre.Z()),0x00ffffff);
			g_VisualDebug->RenderLine(CPoint(obCentre.X(),obCentre.Y()+5.0f,obCentre.Z()),CPoint(obCentre.X(),obCentre.Y()-5.0f,obCentre.Z()),0x00ffffff);
			g_VisualDebug->RenderLine(CPoint(obCentre.X(),obCentre.Y(),obCentre.Z()+5.0f),CPoint(obCentre.X(),obCentre.Y(),obCentre.Z()-5.0f),0x00ffffff);
			g_VisualDebug->Printf3D(obCentre,0.0f,-12.0f,0x00ffffff,DTF_ALIGN_HCENTRE,"%s",pcOrder);
			g_VisualDebug->RenderAABB(m_BattalionInfo[i].m_obMinExtents,m_BattalionInfo[i].m_obMaxExtents,0x00ffffff,0);
		}
	}

#endif // RELEASE
}

void ArmyAudioDefinition::ExplosionEvent (ArmySection* pArmySection,const CPoint& obPosition)
{
	int iTotalUnitsAlive=pArmySection->GetUnitsAlive(obPosition,m_fBlownUpTestRadius);

	if (iTotalUnitsAlive>0) // Only play a sound if there are enemies inside the explosion radius
	{
	unsigned int id;

	if (AudioSystem::Get().Sound_Prepare(id,m_obSoundBlownUp.GetString()))
	{
		AudioSystem::Get().Sound_SetParameterValue(id,"units",(float)iTotalUnitsAlive);
		AudioSystem::Get().Sound_SetPosition(id,obPosition,m_fBlownUpSoundRadius);
		AudioSystem::Get().Sound_Play(id);
	}
	}
}


const char* ArmyAudioDefinition::GetSound (int iOrders)
{
	switch((BATTALION_ORDERS)iOrders)
	{
		case BO_FORWARD_MARCH:			return m_obSoundForwardMarch.GetString(); break;
		case BO_HOLD_POSITION:			return m_obSoundHoldPosition.GetString(); break;
		case BO_RETREAT:				return m_obSoundRetreat.GetString(); break;
		case BO_RUN_FOR_YOUR_LIVES:		return m_obSoundFlee.GetString(); break;
		case BO_ATTACK:					return m_obSoundAttack.GetString(); break;
		case BO_CHARGE:					return m_obSoundCharge.GetString(); break;
		case BO_TAUNT:					return m_obSoundTaunt.GetString(); break;
		default:						return 0; break;
	}
}

void ArmyAudioDefinition::UpdateInfo (ArmySection* pArmySection,int iOrders)
{
	CPoint obListenerPosition(AudioSystem::Get().GetListenerPosition()); // Get listener position

	Battlefield* pBattlefield=pArmySection->GetBattlefield();

	m_BattalionInfo[iOrders].m_iTotalUnits=0;
	m_BattalionInfo[iOrders].m_obPosition.Clear();
	m_BattalionInfo[iOrders].m_obMinExtents.Clear();
	m_BattalionInfo[iOrders].m_obMaxExtents.Clear();

	BATTALION_ORDERS eOrders=(BATTALION_ORDERS)iOrders;

	for( unsigned int i = 0;  i < pBattlefield->m_pBattalions->m_iNumBattalions; ++i ) // Iterate through the battalions
	{
		Battalion* pBattalion = &(pBattlefield->m_pBattalions->m_BattalionArray[i]);

		BATTALION_ORDERS eBattalionOrders = (BATTALION_ORDERS)pBattalion->m_Orders;

		if (eBattalionOrders==eOrders) // Look for battalions with matching order
		{
			CPoint obMin(BF_PositionToWorldSpace( &(pBattalion->m_TopLeft), pBattlefield->m_pHeader )); // Calculate the min bounds of the battalion area
			CPoint obMax(BF_PositionToWorldSpace( &(pBattalion->m_BotRight), pBattlefield->m_pHeader )); // Calculate the max bounds of the battalion area
			CPoint obBattalionCentre(obMin + obMax); // Calculate the centre position of the battalion
			obBattalionCentre*=0.5f;

			//CDirection obDiff(obListenerPosition-obBattalionCentre);

			//if (obDiff.LengthSquared() < (m_fMaxBattalionRange*m_fMaxBattalionRange)) // Check to see if the distance between the battalion and listener is inside our range
			{
				// Calculate the number of grunts still alive in the battalion
				int iGruntCount=0;

				for(int unit=0; unit<pBattalion->m_iNumUnits; ++unit)
				{
					Unit* pUnit=&(pBattalion->m_Units[unit]);

					for(unsigned int grunt=pUnit->m_FirstGruntID; grunt<(pUnit->m_FirstGruntID + pUnit->m_NumGrunts); ++grunt)
					{
						GruntGameState* pGrunt = &((GruntGameState*)(pBattlefield->m_pGrunts+1))[grunt];

						if (pGrunt->m_GruntMajorState!=GMS_DEAD && pGrunt->m_GruntMajorState!=GMS_REAL_DUDE)
							iGruntCount++;
					}			
				}

				if (iGruntCount>0)
				{
					// Calculate the centre position of the battalion
					//CPoint obPlayerPos(pArmySection->GetPlayerPosition());

					//obMin.Y()=obPlayerPos.Y();
					//obMax.Y()=obPlayerPos.Y()+4.0f;

					if (m_BattalionInfo[iOrders].m_iTotalUnits==0)
					{
						m_BattalionInfo[iOrders].m_obMinExtents.X()=obMin.X();
						m_BattalionInfo[iOrders].m_obMinExtents.Z()=obMin.Z();
						m_BattalionInfo[iOrders].m_obMaxExtents.X()=obMax.X();
						m_BattalionInfo[iOrders].m_obMaxExtents.Z()=obMax.Z();
					}
					else
					{
						if (obMin.X()<m_BattalionInfo[iOrders].m_obMinExtents.X())
							m_BattalionInfo[iOrders].m_obMinExtents.X()=obMin.X();

						if (obMin.Z()<m_BattalionInfo[iOrders].m_obMinExtents.Z())
							m_BattalionInfo[iOrders].m_obMinExtents.Z()=obMin.Z();

						if (obMax.X()>m_BattalionInfo[iOrders].m_obMaxExtents.X())
							m_BattalionInfo[iOrders].m_obMaxExtents.X()=obMax.X();

						if (obMax.Z()>m_BattalionInfo[iOrders].m_obMaxExtents.Z())
							m_BattalionInfo[iOrders].m_obMaxExtents.Z()=obMax.Z();
					}

					m_BattalionInfo[iOrders].m_iTotalUnits+=iGruntCount;
				}
			}
		}
	}

	CPoint obPlayerPos(pArmySection->GetPlayerPosition());
	m_BattalionInfo[iOrders].m_obMinExtents.Y()=obPlayerPos.Y();
	m_BattalionInfo[iOrders].m_obMaxExtents.Y()=obPlayerPos.Y()+4.0f;

	// Calculate the centre of this battalion group
	CPoint obCentre(m_BattalionInfo[iOrders].m_obMinExtents + m_BattalionInfo[iOrders].m_obMaxExtents);
	obCentre*=0.5f;

	// Calculate the half extents of the battalion group AABB
	CDirection obHalfExtents(obCentre - m_BattalionInfo[iOrders].m_obMinExtents);
	obHalfExtents.Abs();
	m_BattalionInfo[iOrders].m_obHalfExtents=obHalfExtents;


	//m_BattalionInfo[iOrders].m_obPosition=obCentre;

	// Calculate the radius of the sphere that more or less encompasses the group area
	float fBattalionRadius=0.0f;

	if (obHalfExtents.X()>fBattalionRadius);
		fBattalionRadius=obHalfExtents.X();

	if (obHalfExtents.Y()>fBattalionRadius);
		fBattalionRadius=obHalfExtents.Y();

	if (obHalfExtents.Z()>fBattalionRadius);
		fBattalionRadius=obHalfExtents.Z();

	// Next step is to calculate the nearest point of the group to the listener
	CDirection obDir(obListenerPosition - obCentre);

	if (obDir.LengthSquared() < fBattalionRadius) // The listen is inside the battalion area, therefore place the sound emitter in the same place as the listener
	{
		m_BattalionInfo[iOrders].m_obPosition=obListenerPosition;
	}
	else // Otherwise position the sound emitter on the edge of the battalion group area (thats closest to the listener)
	{
		obDir.Normalise();
		obDir*=fBattalionRadius;

		CPoint obEmitterPosition(obCentre + obDir);

		if (obEmitterPosition.Y()>m_BattalionInfo[iOrders].m_obMaxExtents.Y())
			obEmitterPosition.Y()=m_BattalionInfo[iOrders].m_obMaxExtents.Y();

		if (obEmitterPosition.Y()<m_BattalionInfo[iOrders].m_obMinExtents.Y())
			obEmitterPosition.Y()=m_BattalionInfo[iOrders].m_obMinExtents.Y();

		m_BattalionInfo[iOrders].m_obPosition=obEmitterPosition;
	}
}






