/***************************************************************************************************
*
*	FILE			debugbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "game/debugbindings.h"
#include "game/luaglobal.h"
#include "game/luardebug.h"
#include "game/luahelp.h"
#include "game/shellconfig.h"

#include "core/osddisplay.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/debugcam.h"

#include "hud/hudmanager.h"
#include "hud/objectivemanager.h"
#include "hud/healthbar.h"
#include "hud/stylebar.h"
#include "gui/guimanager.h"
#include "game/entitymanager.h"
#include "game/inputcomponent.h"
#include "game/fsm.h"
#include "Physics/system.h"

// Temporary
#include "game/projectilemanager.h"

//! allow the config system to decide whether the incredible slow lua logs are actually output
bool g_bEnableLuaLogs = false;

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.Message(channelID, colourID, string message)
// DESCRIPTION: Send a Message to an On Screen Display Channel
//-------------------------------------------------------------------------------------------------
static void Debug_Message(int iChan, uint32_t iColour,  const char* pcMsg)
{
#ifndef _RELEASE
	if( g_bEnableLuaLogs == true )
	{

#ifdef USER_gavin
	// Where are those prints coming from?
	ntPrintf("%s\n", CLuaGlobal::Get().State().FileAndLine() );
#endif

	OSD::Add(OSD::CHANNEL(iChan), iColour, pcMsg);
}
#else
	UNUSED(iChan);
	UNUSED(iColour);
	UNUSED(pcMsg);
#endif
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.EnableChannel(channelID)
// DESCRIPTION: Enable an On Screen Display Channel
//-------------------------------------------------------------------------------------------------
static int Debug_EnableChannel(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);

	// Check arguments.
	if(!args[1].IsInteger())
	{
		lua_bind_warn_msg(("Bad Arguments to Bind Function 'Debug_EnableChannel'\n"));
		return 0;
	}

	OSD::EnableChannel(OSD::CHANNEL(args[1].GetInteger()));
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.DisableChannel(channelID)
// DESCRIPTION: Disable an On Screen Display Channel
//-------------------------------------------------------------------------------------------------
static int Debug_DisableChannel(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);

	// Check arguments.
	if(!args[1].IsInteger())
	{
		lua_bind_warn_msg(("Bad Arguments to Bind Function 'Debug_DisableChannel'\n"));
		return 0;
	}

	OSD::DisableChannel(OSD::CHANNEL(args[1].GetInteger()));
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.ToggleChannel(channelID)
// DESCRIPTION: Toggle an On Screen Display Channel
//-------------------------------------------------------------------------------------------------
static int Debug_ToggleChannel(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);

	// Check arguments.
	if(!args[1].IsInteger())
	{
		lua_bind_warn_msg(("Bad Arguments to Bind Function 'Debug_ToggleChannel'\n"));
		return 0;
	}

	OSD::DisableChannel(OSD::CHANNEL(args[1].GetInteger()));
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.Assert(condition)
// DESCRIPTION: Break out in the lua debugger on a given condition
//-------------------------------------------------------------------------------------------------
static int Debug_Assert(NinjaLua::LuaState* state)
{
#ifndef _GOLD_MASTER
	NinjaLua::LuaStack args(state);

	if(args[1].IsBoolean() && !args[1].GetBoolean())
		LuaDebugServer::Assert();
#else
	UNUSED(state);
#endif
	return 0;
}


// BINDFUNC:		LOG( string text )
// DESCRIPTION:		Debug function to log a string to the logfile (a linefeed will be
//					automatically appended). Has no effect in release builds.
static void LOGFunc( const char* pcText )
{
#ifndef _RELEASE
	if( g_bEnableLuaLogs == true )
	{
		ntPrintf("LUA: %s\n", pcText);
		LuaDebugServer::Trace(pcText);
		LuaDebugServer::Trace("\n");
	}
#else
	UNUSED( pcText );
#endif
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    print(...)
// DESCRIPTION: Print a message to the lua debugger output pane and the game log.
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.Print(...)
// DESCRIPTION: Print a message to the lua debugger output pane and the game log.
//-------------------------------------------------------------------------------------------------
static int Debug_Print(NinjaLua::LuaState* pState)
{
#ifndef _RELEASE
	if( g_bEnableLuaLogs == true )
	{
	lua_State* L = &(**pState);

#ifdef USER_gavin
	// Where are those prints coming from?
	ntPrintf("%s\n", pState->FileAndLine() );
	LuaDebugServer::Trace(pState->FileAndLine());
	LuaDebugServer::Trace("\n");
#endif

	int n = lua_gettop(L);  // number of arguments
	int i;
	lua_getglobal(L, "towstring");
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) 
	{
		const char *s = NULL;
	
		lua_pushvalue(L, -1);  // function to be called
		lua_pushvalue(L, i);   // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  // get result
		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

		if (i>1)
		{
			LuaDebugServer::Trace("\t");
			ntPrintf("\t");
		}

		if(s)
		{
			LuaDebugServer::Trace(s);
			ntPrintf(s);
		}

		lua_pop(L, 1);  // pop result
	}

	LuaDebugServer::Trace("\n");
	ntPrintf("\n");
	}
#else
	UNUSED(pState);
#endif
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.CameraSetFOV
// DESCRIPTION: Set the field of view of the debug camera
//-------------------------------------------------------------------------------------------------
static void Debug_CameraSetFOV(float fFOV)
{
	DebugChaseCamera* pCam = CamMan::GetPrimaryView()->GetDebugCamera();
	if(pCam)
		pCam->SetFOV(fFOV);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.CameraSetDist
// DESCRIPTION: Set the distance of the debug camera
//-------------------------------------------------------------------------------------------------
static void Debug_CameraSetDist(float fDist)
{
	DebugChaseCamera* pCam = CamMan::GetPrimaryView()->GetDebugCamera();
	if(pCam)
		pCam->SetZoom(fDist);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.CameraSetMode
// DESCRIPTION: 0 = game, 1 = follow, 2 = free
//-------------------------------------------------------------------------------------------------
static void Debug_CameraSetMode(int iMode)
{
	switch(iMode)
	{
	case 1:
		CamMan::GetPrimaryView()->SetDebugCameraMode(CamView::DM_REL);
		break;
	case 2:
		CamMan::GetPrimaryView()->SetDebugCameraMode(CamView::DM_FREE);
		break;
	default:
		CamMan::GetPrimaryView()->SetDebugCameraMode(CamView::DM_NORMAL);
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.CameraPlaceAt(position, lookingAt)
// DESCRIPTION: Position the game camera...
//-------------------------------------------------------------------------------------------------
static int Debug_CameraPlaceAt(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);
	ntAssert(args[1].IsTable());
	ntAssert(args[2].IsTable());

	NinjaLua::LuaObject pos( args[1] );
	NinjaLua::LuaObject targ( args[2] );
	CPoint ptPos  = CLuaHelper::PointFromTable(pos);
	CPoint ptTarg = CLuaHelper::PointFromTable(targ);

	CamMan::GetPrimaryView()->SetDebugCameraPlacement(ptPos, ptTarg);

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug.CameraFix(bool YESorNO)
// DESCRIPTION: Fixed the angle of the debug camera
//-------------------------------------------------------------------------------------------------
static int Debug_CameraFix(bool b)
{
	CamMan::GetPrimaryView()->GetDebugCamera()->Fix(b);

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_AddObjective(text)
// DESCRIPTION: Add a new objective to the HUD objective manager
//				Objective remains on screen for ObjectiveRenderDef->m_fTimeRenderNew seconds
//-------------------------------------------------------------------------------------------------
static int Objective_AddObjective(const char* pcText)
{
	
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetCombatHUDElements() || !CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager )
		return -1;
	
	// Add the objective
	return CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager->AddObjective(pcText);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_AddTimedObjective(text, time)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
static int Objective_AddTimedObjective(const char* pcText, float fTime)
{
	
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetCombatHUDElements() || !CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager )
		return -1;
	
	// Add the objective
	return CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager->AddTimedObjective(pcText, fTime);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_AddStatusObjective(text)
// DESCRIPTION: Add a new status objective to the HUD objective manager
//				Remains on screen untill removed by Objective_RemoveObjective
//-------------------------------------------------------------------------------------------------
static int Objective_AddStatusObjective(const char* pcText)
{
	
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetCombatHUDElements() || !CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager )
		return -1;
	
	// Add the objective
	return CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager->AddStatusObjective(pcText);
}
//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_RemoveObjective(id)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
static void Objective_RemoveObjective(int iObjectiveID)
{
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetCombatHUDElements() || !CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager )
		return;

	// Remove the objective
	CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager->RemoveObjective(iObjectiveID);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    E3_DemoComplete()
// DESCRIPTION: Let the Gui manager know that the demo has been completed
//-------------------------------------------------------------------------------------------------
static void E3_DemoComplete(void)
{
	// Check we are using the GUI
	if ((g_ShellOptions->m_eFrontendMode == FRONTEND_NONE) || !CGuiManager::Exists() )
		return;
	
	CGuiManager::Get().OnComplete();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    E3_CommanderHealthBar()
// DESCRIPTION: Let the Hud manager know that commander health bar should be displayed
//-------------------------------------------------------------------------------------------------
static void E3_CommanderHealthBar(void)
{
	lua_bind_warn_msg(("Bind Function 'E3_CommanderHealthBar' obsolete\n"));
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    E3_CommanderEntity()
// DESCRIPTION: Let the Hud manager know the commander entity
//-------------------------------------------------------------------------------------------------
static void E3_CommanderEntity(CEntity* pobCommander)
{
	UNUSED ( pobCommander );

	lua_bind_warn_msg(("Bind Function 'E3_CommanderEntity' obsolete\n"));

	//ntAssert(pobCommander);

	//Character* pobCommanderChar = pobCommander->ToCharacter();
	//ntAssert(pobCommanderChar);

	// Check we have a health bar
	//if ( !CHud::Exists() || !CHud::Get().GetHitCounter())
	//	return;
	
	//CHud::Get().GetHitCounter()->ActivateSpecialStyleLevel( );

	// Set the entity
	//CHud::Get().GetCombatHUDElements()->m_pobHealthBar->SetEntity(pobCommanderChar);
	//CHud::Get().GetCombatHUDRenderDefs()->m_pobHitCounterRenderDef->SetEntity(pobCommanderChar);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    DumpMemDifference()
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
#ifdef _HAVE_MEMORY_TRACKING
static void DumpMemDifference()
{
	static uintptr_t puiMemoryTracker = 0;

	if( puiMemoryTracker )
	{
		Mem::DumpMemoryCheckpointDifference( puiMemoryTracker );
		Mem::FreeMemoryCheckpoint( puiMemoryTracker );
	}

	puiMemoryTracker = Mem::TakeMemoryCheckpoint();
}
#endif // _RELEASE

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    E3_PresentationBuild()
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static bool E3_PresentationBuild()
{
	return g_ShellOptions->m_eFrontendMode == FRONTEND_E3;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug_AddProjectileToManager()
// DESCRIPTION: TEMPORARY PROJECTILE FUNCTION
//-------------------------------------------------------------------------------------------------
static void Debug_AddProjectileToManager(CEntity* pProjectile)
{
	ProjectileManager::Get().AddProjectileToList(pProjectile);
}



//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Debug_RemoveProjectileFromManager()
// DESCRIPTION: TEMPORARY PROJECTILE FUNCTION
//-------------------------------------------------------------------------------------------------
static void Debug_RemoveProjectileFromManager(CEntity* pProjectile)
{
	ProjectileManager::Get().RemoveProjectileFromList(pProjectile);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    TGS_HeroSetupStage1()
// DESCRIPTION: TGS Hack to get the Hero to hold the bazooka straight away, and no frame delay
//-------------------------------------------------------------------------------------------------
static void TGS_HeroSetupStage1(const char* pcBazookaName)
{
	//ntPrintf("MartinB - TGS_HeroSetupStage1\n");
	ntAssert(pcBazookaName);
	UNUSED(pcBazookaName);

	// Get bazooka
	CEntity* pobBazooka = CEntityManager::Get().FindEntity(pcBazookaName);
	ntAssert( pobBazooka );

	// Get Hero
	CEntity* pobHero = CEntityManager::Get().GetPlayer();
	ntAssert( pobHero );

	// Turn the Hero's input component off
	pobHero->GetInputComponent()->SetDisabled(true);

	// Get the Bazooka's FSM and send it the equip message
	FSM::StateMachine* pobBazookaFSM = pobBazooka->GetFSM();
	ntAssert( pobBazookaFSM );

	Message BazookaEquipMsg(msg_equip);
	BazookaEquipMsg.SetEnt( CHashedString("Other"), pobHero );
	pobBazookaFSM->ProcessMessage(BazookaEquipMsg);

	// Get the Players's FSM and send it the equip message
	FSM::StateMachine* pobPlayerFSM = pobHero->GetFSM();
	ntAssert( pobHero );

	Message HeroEquipMsg(msg_forceequiprangedweapon);
	HeroEquipMsg.SetEnt( CHashedString("RangedWeapon"), pobBazooka );
	pobPlayerFSM->ProcessMessage(HeroEquipMsg);

}


static void TGS_HeroSetupStage2(const char* pcBazookaName, const char* pcNinjaSeqName)
{
	//ntPrintf("MartinB - TGS_HeroSetupStage2\n");
	ntAssert(pcBazookaName);
	ntAssert(pcNinjaSeqName);

	// Get bazooka
	CEntity* pobBazooka = CEntityManager::Get().FindEntity(pcBazookaName);
	ntAssert( pobBazooka );

	// Get Hero
	CEntity* pobHero = CEntityManager::Get().GetPlayer();
	ntAssert( pobHero );

	// Get NS
	CEntity* pobNS = CEntityManager::Get().FindEntity(pcNinjaSeqName);
	ntAssert( pobNS );

	// Deactivate them
	pobHero->GetPhysicsSystem()->Deactivate();
	pobBazooka->GetPhysicsSystem()->Deactivate();

	// Hide them
	pobHero->Hide();
	pobBazooka->Hide();

	// Move the hero into the correct position
	pobHero->SetPosition(pobNS->GetPosition());
}


static void TGS_HeroSetupStage3(const char* pcBazookaName)
{
	//ntPrintf("MartinB - TGS_HeroSetupStage3\n");
	ntAssert(pcBazookaName);

	// Get bazooka
	CEntity* pobBazooka = CEntityManager::Get().FindEntity(pcBazookaName);
	ntAssert( pobBazooka );

	// Get Hero
	CEntity* pobHero = CEntityManager::Get().GetPlayer();
	ntAssert( pobHero );

	// Activate them
	pobHero->GetPhysicsSystem()->Activate();
	pobBazooka->GetPhysicsSystem()->Activate();

	// Show them
	pobHero->Show();
	pobBazooka->Show();

	// Turn the Hero's input component back on
	pobHero->GetInputComponent()->SetDisabled(false);
}


/***************************************************************************************************
*
*	FUCNTIION		DebugBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/

void DebugBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.Register( "LOG",						LOGFunc );
	obGlobals.RegisterRaw("print",					Debug_Print);
	obGlobals.Register( "E3_PresentationBuild",		E3_PresentationBuild );

	
	obGlobals.Register("Objective_AddObjective",	Objective_AddObjective);
	obGlobals.Register("Objective_AddTimedObjective",	Objective_AddTimedObjective);
	obGlobals.Register("Objective_AddStatusObjective",	Objective_AddStatusObjective);
	obGlobals.Register("Objective_RemoveObjective",	Objective_RemoveObjective);

	// New Way - Register Debug functions in a debug library/table
	obGlobals.Set("E3", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["E3"].Register("DemoComplete",		E3_DemoComplete);
	obGlobals["E3"].Register("CommanderHealthBar",	E3_CommanderHealthBar);
	obGlobals["E3"].Register("CommanderEntity",		E3_CommanderEntity);

	// New Way - Register Debug functions in a debug library/table
	obGlobals.Set("Debug", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["Debug"].Register("Message",			Debug_Message);
	obGlobals["Debug"].RegisterRaw("EnableChannel",	Debug_EnableChannel);
	obGlobals["Debug"].RegisterRaw("DisableChannel",Debug_DisableChannel);
	obGlobals["Debug"].RegisterRaw("ToggleChannel",	Debug_ToggleChannel);
	obGlobals["Debug"].RegisterRaw("Assert",		Debug_Assert);
	obGlobals["Debug"].RegisterRaw("Print",			Debug_Print);

	obGlobals["Debug"].Register("CameraSetFOV",		Debug_CameraSetFOV);
	obGlobals["Debug"].Register("CameraSetDist",	Debug_CameraSetDist);
	obGlobals["Debug"].Register("CameraSetMode",	Debug_CameraSetMode);
	obGlobals["Debug"].RegisterRaw("CameraPlaceAt",	Debug_CameraPlaceAt);
	obGlobals["Debug"].Register("CameraFix",		Debug_CameraFix);

	// MARTIN'S AMAZING TGS HACK FUNCTION TO REMOVE THE FRAME DELAY OF THE HERO HOLDING BAZOOKA
	obGlobals.Set("TGS", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["TGS"].Register("HeroSetupStage1",	TGS_HeroSetupStage1);
	obGlobals["TGS"].Register("HeroSetupStage2",	TGS_HeroSetupStage2);
	obGlobals["TGS"].Register("HeroSetupStage3",	TGS_HeroSetupStage3);

	// TEMPORARY BIND FUNCTION FOR PROJECTILES!
	obGlobals.Set("Projectiles", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["Projectiles"].Register("AddProjectileToManager", Debug_AddProjectileToManager);
	obGlobals["Projectiles"].Register("RemoveProjectileFromManager", Debug_RemoveProjectileFromManager);
#ifdef _HAVE_MEMORY_TRACKING
	obGlobals["Debug"].Register("DumpMemDifference",DumpMemDifference);
#endif // _RELEASE
}
