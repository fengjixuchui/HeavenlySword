//--------------------------------------------------
//!
//!	\file game/entitycheckpoint.cpp
//!	Definition of the checkpoint entity object
//!
//--------------------------------------------------


#include "game/entitycheckpoint.h"

#include "game/checkpointmanager.h"
#include "gui/guimanager.h"
#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "game/movement.h"
#include "messagehandler.h"
#include "game/entitymanager.h"
#include "game/attacks.h"
#include "game/hitcounter.h"
#include "core/visualdebugger.h"
#include "game/combatstyle.h"
#include "game/gameinfo.h"
#include "game/shelllevel.h"
#include "game/shellmain.h"

#include "hud/hudmanager.h"
#include "hud/messagedata.h"

#ifndef _RELEASE
#include "core/OSDDisplay.h"
#endif


void ForceLinkFunctionCheckpoint()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCheckpoint() !ATTN!\n");
}

START_CHUNKED_INTERFACE(CheckpointSummaryDef, Mem::MC_ENTITY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPosX, 0.65f, PositionX)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPosY, 0.25f, PositionY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeOnScreen, 5.0f, TimeOnScreen)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNewNameDelay, 2.0f, NewNameDelay)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDoSummary, false, DoSummary)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDoNewCheckpointName, false, DoNewCheckpointName)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Object_Checkpoint, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iID, 0, ID)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Position, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Orientation, CQuat(0.0f, 0.0f, 0.0f, -1.0f), Orientation)
	PUBLISH_VAR_AS(m_LoadFunction, LuaFunctionToRunOnLoad)
	PUBLISH_VAR_AS(m_SaveFunction, LuaFunctionToRunOnSave)
	PUBLISH_VAR_AS(m_StartFunction, LuaFunctionToRunOnStart)

	PUBLISH_PTR_AS(m_pobSummaryDef, SummaryScreenParameters)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


STATEMACHINE(OBJECT_CHECKPOINT_FSM, Object_Checkpoint)
	OBJECT_CHECKPOINT_FSM( bool bActive )
	{
		if ( bActive )
		{
			SET_INITIAL_STATE(DEFAULT);
		}
		else
		{
			SET_INITIAL_STATE(USED);
		}
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				
			END_EVENT(true)

			ON_UPDATE
			{
				ME->DebugRenderCheckpoint();
			}
			END_EVENT(true)

			EVENT(Activate)
			{
				// Player has walked into the checkpoint trigger
				ntPrintf("Hit Checkpoint Trigger: %s\n", ME->GetName().c_str());

				// Call function to handle checkpoint hit
				ME->PlayerHitCheckpoint();

				// Can no longer use the checkpoint
				SET_STATE(USED);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // DEFAULT

	STATE(USED)
		BEGIN_EVENTS
			ON_ENTER
				
			END_EVENT(true)
		END_EVENTS
	END_STATE // USED

END_STATEMACHINE // OBJECT_CHECKPOINT_FSM


//--------------------------------------------------
//!
//!	Object_Checkpoint::Object_Checkpoint()
//!	Constructor
//!
//--------------------------------------------------
Object_Checkpoint::Object_Checkpoint()
{
	m_eType				= EntType_Object;
	m_pobCheckpointData	= 0;

	m_iLevelID			= -1;
	m_iID				= 0;

	m_pobSummaryDef		= 0;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::~Object_Checkpoint()
//!	Destructor
//!
//--------------------------------------------------
Object_Checkpoint::~Object_Checkpoint()
{

}


//--------------------------------------------------
//!
//!	Object_Checkpoint::OnPostConstruct()
//!	Post Construction stuff
//!
//--------------------------------------------------
void Object_Checkpoint::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	// Install required components
	InstallMessageHandler();

	// Register the checkpoint with the level
	ShellLevel* pLoadingLevel = ShellMain::Get().GetCurrLoadingLevel();
	ntAssert( pLoadingLevel );

	// Level ID
	m_iLevelID = pLoadingLevel->GetLevelNumber();

	// Register the checkpoint with the level
	pLoadingLevel->RegisterCheckpoint( this );

	bool bActiveCheckpoint = true;

	// The first checkpoint can never be hit, so initially set it to used in its FSM
	if ( m_iID == 0 )
	{
		bActiveCheckpoint = false;
	}

	// Attach Checkpoint FSM to entity
	OBJECT_CHECKPOINT_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_CHECKPOINT_FSM( bActiveCheckpoint );
	ATTACH_FSM(pFSM);
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::StartAtCheckpoint()
//!	Starts the entity to the checkpoint, and performs
//! loading of checkpoint data.
//!
//--------------------------------------------------
void Object_Checkpoint::StartAtCheckpoint( void )
{
	// Call LUA start function if there is one first, as this script may swap the player to the archer etc.
	CLuaGlobal::CallLuaFunc( m_StartFunction );

	// Now get the player we are using at this checkpoint
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();

	// Sanity Checks
	ntError( pPlayer );
	ntError( pPlayer->IsPlayer() );

	// Load Data
	LoadCheckpoint( pPlayer );

	// Do initial combo unlocking, which doesn't notify the HUD
	CheckpointManager::Get().UnlockCombos( StyleManager::Get().GetStats().m_obCachedStylePointTotals , false );
	
	// Finally, set player position and rotation to the checkpoint entity
	pPlayer->SetPosition( m_Position );
	pPlayer->SetRotation( m_Orientation );

	// Remember that this is the last checkpoint we hit in the level,
	// so if the player dies they restart at this point
	ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntError( pPlayingLevel );

	// Disable this checkpoint and the previous ones
	DisableAllPreviousCheckpoints();
	DisableCheckpoint();
	
	pPlayingLevel->SetLastCheckpointID( m_iID );
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::PlayerHitCheckpoint()
//!	Called when the player hits the checkpoint
//!
//--------------------------------------------------
void Object_Checkpoint::PlayerHitCheckpoint( void )
{
	// If this is an invalid level i.e. not loaded through the FE, then do not save.
	if ( m_iLevelID == -1 )
	{
		//ntPrintf("Checkpoint Level ID invalid - No saving.\n");
		//return;
	}

	// Now get the player we are using at this checkpoint
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();

	// Sanity Checks
	ntError( pPlayer );
	ntError( pPlayer->IsPlayer() );

	// Save out checkpoint data
	SaveCheckpoint( pPlayer );

	// Checkpoint save summary screen
	SummaryScreen( pPlayer );

	// Set the style points for unlocking for this checkpoint
	SetupStylePointsForCheckpoint();
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::SetupStylePointsForCheckpoint()
//!	Sets up the style points for playing through this
//! checkpoint.
//!
//--------------------------------------------------
void Object_Checkpoint::SetupStylePointsForCheckpoint( void )
{
	// Setup style points and deficit (if applicable)
	//--------------------------------------------------
	if ( StyleManager::Exists() )
	{
		// Reset the points for the section
		StyleManager::Get().GetStats().m_obStylePointsForSection.Reset();

		// The deficit is actually the inverse of the style points of the NEXT checkpoint, so let's see if that exists
		CheckpointData* pNextCPData = CheckpointManager::Get().GetDataForCheckpoint( m_iLevelID, m_iID + 1 );

		if ( pNextCPData )
		{
			StanceStylePoints obDeficit = pNextCPData->m_GenericData.GetStanceStylePoints();
			
			// Set the deficit
			StyleManager::Get().GetStats().m_obStylePointsDeficitForSection = obDeficit;
		}
		else
		{
			// Player has not reached the next checkpoint, so there is no deficit
			StyleManager::Get().GetStats().m_obStylePointsDeficitForSection.Reset();
		}
	}

	// Cache the total style points for quick calculations during gameplay
	//--------------------------------------------------
	CheckpointManager::Get().CacheStylePointTotals();
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::SaveCheckpoint()
//!	Saves the checkpoint and all required data
//!
//--------------------------------------------------
void Object_Checkpoint::SaveCheckpoint( CEntity* pPlayer )
{
	ntError( pPlayer );

	// Get the data for this checkpoint, if we don't have a pointer to it already
	if ( !m_pobCheckpointData )
	{
		m_pobCheckpointData = CheckpointManager::Get().GetDataForCheckpoint( m_iLevelID, m_iID );
		
		// There is no current data for this checkpoint so create it
		if ( !m_pobCheckpointData )
		{
			m_pobCheckpointData = CheckpointManager::Get().CreateDataForCheckpoint( m_iLevelID, m_iID );
			ntAssert( m_pobCheckpointData );
		}
	}

	// If this is the first checkpoint, we don't want to save
	if ( m_iID == 0 )
	{
		return;
	}

	bool bSaveSuccessful = true;

	// Remember that this is the last checkpoint we hit in the level,
	// so if the player dies they restart at this point
	ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert( pPlayingLevel );

	pPlayingLevel->SetLastCheckpointID( m_iID );

	// Save the generic data
	if ( !SaveGenericData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Generic data save failed", m_iID );
		bSaveSuccessful = false;
	}

	// Save the global data
	if ( !SaveGlobalData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Global data save failed", m_iID );
		bSaveSuccessful = false;
	}

	// Save the level specific data
	if ( !SaveLevelSpecificData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Level specific data save failed", m_iID );
		bSaveSuccessful = false;
	}

	// At this point we tell the checkpoint manager to save out the data
	CheckpointManager::Get().SaveCheckpointData();

	if ( bSaveSuccessful )
	{
		ntPrintf( "Checkpoint %d: Save successful", m_iID );
	}
	else
	{
		ntPrintf( "Checkpoint %d: Save failed", m_iID );
	}

#ifndef _RELEASE
	OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "SAVING CHECKPOINT DATA");
	DebugPlayerValues( pPlayer );
#endif
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::LoadCheckpoint()
//!	Loads the checkpoint and all required data
//!
//--------------------------------------------------
void Object_Checkpoint::LoadCheckpoint( CEntity* pPlayer )
{
	ntError( pPlayer );
	
	// Get the data for this checkpoint, if we don't have a pointer to it already
	if ( !m_pobCheckpointData )
	{
		m_pobCheckpointData = CheckpointManager::Get().GetDataForCheckpoint( m_iLevelID, m_iID );
		
		if ( !m_pobCheckpointData )
		{
			// Couldn't get the data for the checkpoint, likely due to none being loaded, so return
			return;
		}
	}

	// If this is the first checkpoint, we don't want to load from it
	if ( m_iID == 0 )
	{
		return;
	}

	bool bLoadSuccessful = true;

	// Load the generic data
	if ( !LoadGenericData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Generic data load failed", m_iID );
		bLoadSuccessful = false;
	}

	// Load the global data
	if ( !LoadGlobalData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Global data load failed", m_iID );
		bLoadSuccessful = false;
	}

	// Load the level specific data
	if ( !LoadLevelSpecificData( pPlayer ) )
	{
		ntPrintf( "Checkpoint %d: Level specific data load failed", m_iID );
		bLoadSuccessful = false;
	}

	if ( bLoadSuccessful )
	{
		ntPrintf( "Checkpoint %d: Load successful", m_iID );
	}
	else
	{
		ntPrintf( "Checkpoint %d: Load failed", m_iID );
	}

#ifndef _RELEASE
	OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "LOADING CHECKPOINT DATA");
	DebugPlayerValues( pPlayer );
#endif
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::SaveGlobalData()
//!	Saves out the generic game data
//!
//--------------------------------------------------
bool Object_Checkpoint::SaveGlobalData( CEntity* pPlayer )
{
	ntError( pPlayer );

		// Calculate the delta lifeclock for this checkpoint
	double dDeltaLifeClock = LifeClockDeltaThisCheckpoint( pPlayer );

		// Compare to currently saved one
	if ( dDeltaLifeClock > m_pobCheckpointData->m_GlobalData.GetLifeClockDelta() )
	{
		// Save delta time
		m_pobCheckpointData->m_GlobalData.SetLifeClockDelta( dDeltaLifeClock );
	}

	return true;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::SaveGenericData()
//!	Saves out the generic game data
//!
//--------------------------------------------------
bool Object_Checkpoint::SaveGenericData( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED( pPlayer );

	// Stance Style Points
	if ( StyleManager::Exists() )
	{
		StanceStylePoints obCombinedStylePointsForCheckpoint;

		// Get the points the player has just scored
		const StanceStylePoints& obNewStylePoints = StyleManager::Get().GetStats().m_obStylePointsForSection;

		// Get the currently saved points
		const StanceStylePoints& obSavedStylePoints = m_pobCheckpointData->m_GenericData.GetStanceStylePoints();

		// Store the larger value
		obCombinedStylePointsForCheckpoint.CompareAndStoreHighest( obNewStylePoints, obSavedStylePoints );

		// Save it
		m_pobCheckpointData->m_GenericData.SetStanceStylePoints( obCombinedStylePointsForCheckpoint );
	}
	
	return true;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::SaveLevelSpecificData()
//!	Saves out the level specific game data
//!
//--------------------------------------------------
bool Object_Checkpoint::SaveLevelSpecificData( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED( pPlayer );

	// Set the CP Data we want to save to
	CheckpointManager::Get().SetLUAAccessCPData( m_pobCheckpointData );

	// Call the LUA save function
	CLuaGlobal::CallLuaFunc( m_SaveFunction );

	// Tell the manager we are finished
	CheckpointManager::Get().FinishedLUAAccessOfCPData();

	return true;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::LoadGenericData()
//!	Loads in the generic game data
//!
//--------------------------------------------------
bool Object_Checkpoint::LoadGenericData( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED( pPlayer );

	SetupStylePointsForCheckpoint();

	return true;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::LoadGlobalData()
//!	Loads in the generic game data
//!
//--------------------------------------------------
bool Object_Checkpoint::LoadGlobalData( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED( pPlayer );

	// Lifeclock
	//--------------------------------------------------
	LifeClock* pLifeClock = 0;

	// Get current life clock
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetLifeClock() );
		pLifeClock = StyleManager::Get().GetLifeClock();
	}

	if ( pLifeClock )
	{
		double dLifeClockSeconds = CheckpointManager::Get().GetLifeClockToThisCheckpoint( m_pobCheckpointData );

		pLifeClock->SetTotalInSeconds( dLifeClockSeconds );
	}

	return true;
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::LoadLevelSpecificData()
//!	Loads in the level specific game data
//!
//--------------------------------------------------
bool Object_Checkpoint::LoadLevelSpecificData( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED( pPlayer );

	// Set the CP Data we want to load from
	CheckpointManager::Get().SetLUAAccessCPData( m_pobCheckpointData );

	// Call the LUA load function
	CLuaGlobal::CallLuaFunc( m_LoadFunction );

	// Tell the manager we are finished
	CheckpointManager::Get().FinishedLUAAccessOfCPData();
    	
	return true;
}

//--------------------------------------------------
//!
//!	Object_Checkpoint::DisableAllPreviousCheckpoints()
//! Disables all previous checkpoints.
//!
//--------------------------------------------------
void Object_Checkpoint::DisableAllPreviousCheckpoints( void )
{
	// Get the playing level
	ShellLevel* pLoadingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntError( pLoadingLevel );

	// Go through all the checkpoints upto this one
	for ( int i = 0; i < m_iID; i++ )
	{
		Object_Checkpoint* pPrevCheckpoint = pLoadingLevel->GetCheckpointByID( i );	
		
		if ( pPrevCheckpoint )
		{
			pPrevCheckpoint->DisableCheckpoint();
		}
	}
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::DisableCheckpoint()
//! Disables this checkpoint from being triggered again.
//!
//--------------------------------------------------
void Object_Checkpoint::DisableCheckpoint( void )
{
	ntPrintf( "Disabling Checkpoint: %d\n", m_iID );
	EXTERNALLY_SET_STATE( OBJECT_CHECKPOINT_FSM, USED );
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::DebugRenderCheckpoint()
//!	Debug rendering of the checkpoint position
//!
//--------------------------------------------------
void Object_Checkpoint::DebugRenderCheckpoint( void )
{
#ifndef _GOLD_MASTER
	CPoint TopOfLinePos = m_Position + CPoint(0.0f, 1.5f, 0.0f);
	g_VisualDebug->RenderLine(m_Position, TopOfLinePos, DC_GREEN);

	CPoint TextPos = TopOfLinePos + CPoint(0.0f, 0.2f, 0.0f);

	g_VisualDebug->Printf3D(TextPos, DC_GREEN, 0, GetName().c_str() );
#endif
}


//--------------------------------------------------
//!
//!	Object_Checkpoint::DebugPlayerValues()
//!	Debug output of player values
//!
//--------------------------------------------------
void Object_Checkpoint::DebugPlayerValues( CEntity* pPlayer )
{
	ntError( pPlayer );
	UNUSED(pPlayer);

	// Get the data for this checkpoint, if we don't have a pointer to it already
	if ( !m_pobCheckpointData )
	{
		m_pobCheckpointData = CheckpointManager::Get().GetDataForCheckpoint( m_iLevelID, m_iID );
		ntAssert( m_pobCheckpointData );
	}

	ntPrintf("------------------------------------\n");
	ntPrintf("- CHECKPOINT LOAD/SAVE PLAYER DATA -\n");
	ntPrintf("------------------------------------\n");
	
	// Lifeclock
	ntPrintf("- Lifeclock Delta: %.2f\n", m_pobCheckpointData->m_GlobalData.GetLifeClockDelta() );
	// Style points
	//ntPrintf("- Style Points: %d\n", m_pobCheckpointData->m_GenericData.GetStylePoints() );
	// Style progression
	//ntPrintf("- Style Progression: %d\n", m_pobCheckpointData->m_GenericData.GetStyleProgression() );

	ntPrintf("------------------------------------\n");
}

//--------------------------------------------------
//!
//!	Object_Checkpoint::SummaryScreen()
//!	Checkpoint summary HUD element setup
//!
//--------------------------------------------------
void Object_Checkpoint::SummaryScreen( CEntity* pPlayer )
{
	// Make sure we have everything we need
	if ( !CHud::Exists() || !CHud::Get().GetMessageDataManager() )
		return;

	// Get the data for this checkpoint, if we don't have a pointer to it already
	if ( !m_pobCheckpointData )
	{
		m_pobCheckpointData = CheckpointManager::Get().GetDataForCheckpoint( m_iLevelID, m_iID );
		ntAssert( m_pobCheckpointData );
	}
	
	MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
	ntAssert( pobMsgDataMan );
	
	ntstd::Vector<ntstd::String> obStringList;

// Checkpoint name
	obStringList.push_back( ntstd::String("CHECKPOINT_COMPLETED") );
	
	pobMsgDataMan->SetValue( CHashedString("CHECKPOINT_NAME"), CheckpointName( m_iLevelID, m_iID-1 ) );

// Current Lifeclock Delta
	double dLifeClockDelta = LifeClockDeltaThisCheckpoint( pPlayer );

	if ( dLifeClockDelta> 0.0f )
	{
		obStringList.push_back( ntstd::String("CHECKPOINT_LIFECLOCK_GIAN") );
	}
	else
	{
		obStringList.push_back( ntstd::String("CHECKPOINT_LIFECLOCK_LOSS") );
		dLifeClockDelta *= -1.0;
	}

	int iDays, iHours, iMinutes, iSeconds;

	// Convert to readable time
	iDays = (int)dLifeClockDelta / 86400;
	dLifeClockDelta -= iDays * 86400.0;
	
	iHours = (int)dLifeClockDelta / 3600;
	dLifeClockDelta -= iHours * 3600.0;

	iMinutes = (int)dLifeClockDelta / 60;
	dLifeClockDelta -= iMinutes * 60.0;

	iSeconds = (int)dLifeClockDelta;

	pobMsgDataMan->SetValue( CHashedString("HOURS"), iHours);
	pobMsgDataMan->SetValue( CHashedString("MINS"), iMinutes);
	pobMsgDataMan->SetValue( CHashedString("SECS"), iSeconds);

// Lifeclock best
	dLifeClockDelta =  m_pobCheckpointData->m_GlobalData.GetLifeClockDelta();
	if ( dLifeClockDelta> 0.0f )
	{
		obStringList.push_back( ntstd::String("CHECKPOINT_LIFECLOCK_BEST_GIAN") );
	}
	else
	{
		obStringList.push_back( ntstd::String("CHECKPOINT_LIFECLOCK_BEST_LOSS") );
		dLifeClockDelta *= -1.0;
	}

	// Convert to readable time
	iDays = (int)dLifeClockDelta / 86400;
	dLifeClockDelta -= iDays * 86400.0;
	
	iHours = (int)dLifeClockDelta / 3600;
	dLifeClockDelta -= iHours * 3600.0;

	iMinutes = (int)dLifeClockDelta / 60;
	dLifeClockDelta -= iMinutes * 60.0;

	iSeconds = (int)dLifeClockDelta;
	
	pobMsgDataMan->SetValue( CHashedString("LIFECLOCK_BEST_HOURS"), iHours);
	pobMsgDataMan->SetValue( CHashedString("LIFECLOCK_BEST_MINS"), iMinutes);
	pobMsgDataMan->SetValue( CHashedString("LIFECLOCK_BEST_SECS"), iSeconds);

	// Kills
	//obStringList.push_back( ntstd::String("CHECKPOINT_KILLS") );
	//pobMsgDataMan->SetValue( CHashedString("KILLS"), StyleManager::Get().GetStats().m_iKills );

	// Total and reset counters ready for the next section
	StyleManager::Get().GetStats().DoSectionTotals();
	
	// Create message box 
	if ( m_pobSummaryDef )
	{
		if ( m_pobSummaryDef->m_bDoSummary )
		{
			CHud::Get().CreateMessageBox( obStringList, m_pobSummaryDef->m_fPosX, m_pobSummaryDef->m_fPosY );
			CHud::Get().RemoveMessageBox( m_pobSummaryDef->m_fTimeOnScreen );
		}
	
		if ( m_pobSummaryDef->m_bDoNewCheckpointName )
		{
			// Created delayed name for next checkpoint
			CHud::Get().CreateDelayedMessage( CheckpointName( m_iLevelID, m_iID ), m_pobSummaryDef->m_fTimeOnScreen + m_pobSummaryDef->m_fNewNameDelay );
		}
	}
}

//--------------------------------------------------
//!
//!	Object_Checkpoint::LifeClockDeltaThisCheckpoint()
//!	Lifeclock delta this checkpoint
//!
//--------------------------------------------------
double Object_Checkpoint::LifeClockDeltaThisCheckpoint( CEntity* pPlayer )
{
	ntAssert ( pPlayer );

	double dDeltaLifeClock = 0.0;
	
	LifeClock* pLifeClock = 0;

	// Get current life clock
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetLifeClock() );
		pLifeClock = StyleManager::Get().GetLifeClock();
	}

	// Only Nariko has a lifeclock
	if ( pPlayer->IsPlayer() && pPlayer->ToPlayer()->IsHero() && pLifeClock )
	{
		double dCurrentLifeClock = pLifeClock->GetTotalInSeconds();

		// Get the lifeclock time upto the previous checkpoint
		double dLifeClockUptoPreviousCheckpoint = CheckpointManager::Get().GetLifeClockToPreviousCheckpoint( m_pobCheckpointData );

		// Calculate the delta lifeclock for this checkpoint
		dDeltaLifeClock = dCurrentLifeClock - dLifeClockUptoPreviousCheckpoint;
	}
	
	return dDeltaLifeClock;
}

ntstd::String Object_Checkpoint::CheckpointName( int iLevelID, int iCheckpointID )
{
	ntstd::String obReturn = "STRING_NOT_IN_LAMS";

	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	if (! pobGameInfo )
		return obReturn;

	const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter( iLevelID );
	if (! pobChapterInfo )
		return obReturn;

	const CheckpointInfoDef* pobCheckpointInfo = pobChapterInfo->GetCheckpoint( iCheckpointID );
	if (! pobCheckpointInfo )
		return obReturn;

	obReturn = pobCheckpointInfo->CheckpointTitleID();

	return obReturn;
}
