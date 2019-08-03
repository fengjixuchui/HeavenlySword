#include "objectdatabase/dataobject.h"
#include "ainavnodes.h"

START_STD_INTERFACE( CAINavNodeAABB )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obWorldMin, CPoint(-0.5f, -0.5f, -0.5f), WorldMin )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obWorldMax, CPoint( 0.5f,  0.5f,  0.5f), WorldMax )
	PUBLISH_VAR_AS( m_obParentNavSet,	ParentNavSet )
	PUBLISH_VAR_AS( m_obNavSetName,		NavSetName )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE( CAINavNodeRegion )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fWorldMinHeight, -0.5f, WorldMinHeight )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fWorldMaxHeight,  0.5f, WorldMaxHeight )
	PUBLISH_VAR_AS( m_obParentNavSet, ParentNavSet )
	PUBLISH_VAR_AS( m_obNavSetName, NavSetName )
	PUBLISH_PTR_CONTAINER_AS( m_obPoints, Points )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE( CAIRegionVertex )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Pos )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE


#include "aiattackselection.h"

//------------------------------------------------------------------------------------------
// AICombatDef Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(AICombatDef)
	IHELP		("Combat definition for AIs.")
	_ISTRING	(ScriptOverrides)
	_ISTRING	(Formation)
	_ISTRING	(GroupCombat)
END_STD_INTERFACE

/*
//------------------------------------------------------------------------------------------
// AICombatDefinition Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(CAICombatDefinition)
	IHELP		("Combat definition for AIs.")
	IREFERENCE	(CAICombatDefinition, Easy)
	IREFERENCE	(CAICombatDefinition, Hard)
	ISTRING		(CAICombatDefinition, ScriptOverrides)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// AICombatTable Interface             
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(CAICombatTable)
	IHELP		("Attack selection for AIs.")

	IFLOAT		(CAICombatTable, ProbStartAttack)
	IFLOAT		(CAICombatTable, ProbStartBlock)
	IFLOAT		(CAICombatTable, ProbContinueCombo)
	IFLOAT		(CAICombatTable, MaxAttackDelay)
	IFLOAT		(CAICombatTable, MinAttackDelay)

	IFLOAT		(CAICombatTable, ATTACK[speed_fast])
	IFLOAT		(CAICombatTable, ATTACK[speed_medium])
	IFLOAT		(CAICombatTable, ATTACK[speed_grab])
	IFLOAT		(CAICombatTable, ATTACK[power_fast])
	IFLOAT		(CAICombatTable, ATTACK[power_medium])
	IFLOAT		(CAICombatTable, ATTACK[power_grab])
	IFLOAT		(CAICombatTable, ATTACK[range_fast])
	IFLOAT		(CAICombatTable, ATTACK[range_medium])
	IFLOAT		(CAICombatTable, ATTACK[range_grab])
	IFLOAT		(CAICombatTable, ATTACK[evade])
	IFLOAT		(CAICombatTable, ATTACK[speed_block])
	IFLOAT		(CAICombatTable, ATTACK[power_block])
	IFLOAT		(CAICombatTable, ATTACK[range_block])

	IFLOAT		(CAICombatTable, BlockTimeMin)
	IFLOAT		(CAICombatTable, BlockTimeMax)

END_STD_INTERFACE

*/

#include "aicharactersettings.h"

START_STD_INTERFACE( CAIMeleeSettings )
	PUBLISH_VAR_AS		( m_fPatrolWalkTimeMin, PatrolWalkTimeMin );
	PUBLISH_VAR_AS		( m_fPatrolWalkTimeMax, PatrolWalkTimeMax );
	PUBLISH_VAR_AS		( m_fPatrolLookTimeMin, PatrolLookTimeMin );
	PUBLISH_VAR_AS		( m_fPatrolLookTimeMax, PatrolLookTimeMax );
	PUBLISH_VAR_AS		( m_fInvHesitationTime, InvHesitationTime );
	PUBLISH_VAR_AS		( m_fInvSoundTimeMin, InvSoundTimeMin );
	PUBLISH_VAR_AS		( m_fInvSoundTimeMax, InvSoundTimeMax );
	PUBLISH_VAR_AS		( m_fInvSoundTimeExtension, InvSoundTimeExtension );
	PUBLISH_VAR_AS		( m_fInvSightTimeMin, InvSightTimeMin );
	PUBLISH_VAR_AS		( m_fInvSightTimeMax, InvSightTimeMax );
	PUBLISH_VAR_AS		( m_fInvSightTimeExtension, InvSightTimeExtension );
END_STD_INTERFACE

#include "aidebugwall.h"

START_STD_INTERFACE( AIDebugWall )
	PUBLISH_VAR_AS( m_obWorldMin, WorldMin )
	PUBLISH_VAR_AS( m_obWorldMax, WorldMax )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

#include "aipatrolnode.h"

START_STD_INTERFACE( PatrolNode )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Position )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPathNum, 1, PathNum )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
END_STD_INTERFACE

//#include "aicoverpoint.h" (Dario-)
//
//START_STD_INTERFACE( CoverPoint )
//	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Position )
//	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
//END_STD_INTERFACE
//
//START_STD_INTERFACE( FirePoint )
//	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Position )
//	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
//END_STD_INTERFACE
//
//START_STD_INTERFACE( RallyPoint )
//	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Position )
//	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
//END_STD_INTERFACE
//
//START_STD_INTERFACE( AvoidPoint )
//	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPos, CPoint(0.0f, 0.0f, 0.0f), Position )
//	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
//END_STD_INTERFACE

#include "aisafezone.h"

START_STD_INTERFACE( AISafeZone )
	PUBLISH_VAR_AS( m_obCentre, Centre )
	PUBLISH_VAR_AS( m_fRadius, Radius )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )	
END_STD_INTERFACE

#include "game/aicomponent.h"

START_STD_INTERFACE( MovementSet )
	PUBLISH_PTR_AS( m_pobWalkingController, WalkingController )
	PUBLISH_PTR_AS( m_pobStrafingController, StrafingController )
	PUBLISH_PTR_AS( m_pobCloseStrafingController, CloseStrafingController )
	PUBLISH_PTR_AS( m_pobShuffleController, ShuffleController )
	PUBLISH_PTR_AS( m_pobPatrolWalkController, PatrolWalkController )
	PUBLISH_PTR_AS( m_pobInvestigateWalkController, InvestigateWalkController )
	PUBLISH_PTR_AS( m_pobCrossbowStrafeController, CrossbowStrafeController )
	PUBLISH_PTR_AS( m_pobCrossbowWalkingController, CrossbowWalkingController )
	PUBLISH_PTR_AS( m_pobCrossbowCloseStrafingController, CrossbowCloseStrafingController )
END_STD_INTERFACE

START_STD_INTERFACE( AnimSet )
	PUBLISH_PTR_AS( m_pobInvestigateLookAnim, InvestigateLookAnim )
	PUBLISH_PTR_AS( m_pobInvestigateShrugAnim, InvestigateShrugAnim )
	PUBLISH_PTR_AS( m_pobPatrolLookAAnim, PatrolLookAAnim )
	PUBLISH_PTR_AS( m_pobPatrolLookBAnim, PatrolLookBAnim )
END_STD_INTERFACE

START_STD_INTERFACE( CAIComponentDef )
	IHELP ("AI component simple config definition")
    IPREFIX ("AIComp_")

	_IENUM(StartState, AI_START_STATE)
	_IENUM(UsingState, AI_ACTION_USING)
	PUBLISH_PTR_AS( m_pobCombatDef, CombatDef )
	PUBLISH_PTR_AS( m_pobMovementSet, MovementSet )
	PUBLISH_VAR_AS( m_bGrouped, Grouped )
	PUBLISH_VAR_AS( m_bDrawViewCones, DrawViewCones )
	PUBLISH_VAR_AS( m_obAIBehaviourSet, AIBehaviourSet )
END_STD_INTERFACE

/*
START_STD_INTERFACE( CAIComponentDef )
	PUBLISH_ENUM_AS( m_eStartState, StartState )
//	PUBLISH_PTR_AS( m_pobCombatDef, CombatDef )
//	PUBLISH_PTR_AS( m_pobMovementSet, MovementSet )
//	PUBLISH_VAR_AS( m_bGrouped, Grouped )
//	PUBLISH_VAR_AS( m_bDrawViewCones, DrawViewCones )
//	PUBLISH_VAR_AS( m_obAIBehaviourSet, AIBehaviourSet )
END_STD_INTERFACE

*/

void ForceLinkFunction4()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction4() !ATTN!\n");
}

