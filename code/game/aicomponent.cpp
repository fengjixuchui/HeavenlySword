/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "game/aicomponent.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"

#include "ai/aistates.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "ai/ainavpath.h"
#include "ai/aibehaviourmanager.h"
#include "ai/aitasks.h"
#include "ai/aiformation.h"
#include "ai/aiformationslot.h"
#include "ai/aiformationattack.h"
#include "ai/ainavfollowpoint.h"
#include "ai/aiformationcomponent.h"
#include "ai/aicoverpoint.h"

#include "audio/gameaudiocomponents.h"

#include "game/luaattrtable.h"
#include "input/inputhardware.h"
#include "game/attacks.h"
//#include "camera/simpletransition.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "camera/camman.h"
#include "game/movement.h"
#include "game/movementcontrollerinterface.h"
#include "game/interactiontransitions.h"
#include "game/luaexptypes.h"
#include "game/nsmanager.h"
#include "game/shellmain.h"

// for line of sight test
#include "physics/world.h"

#include "camera/camman_public.h"

#include "core/visualdebugger.h"

#include "game/messagehandler.h"

#ifdef _NETWORK_ENABLED
#include "game/netcomponent.h"
#endif

#include "objectdatabase/dataobject.h"

#include "ai/ainavigationsystem/ainavigsystemman.h"	// !!! - Dario (temp. only to switch between old and new AI system)

#include "core/gatso.h"
//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MovementSet::MovementSet
//! Constructor
//!                                                                                         
//------------------------------------------------------------------------------------------
MovementSet::MovementSet()
{
	m_pobWalkingController         = 0; 
	m_pobCrossbowWalkingController	= 0; 
	m_pobStrafingController        = 0;
	m_pobCloseStrafingController   = 0;
	m_pobPatrolWalkController      = 0;
	m_pobInvestigateWalkController = 0;
	m_pobCrossbowStrafeController	= 0;
	m_pobCrossbowCloseStrafingController = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponentDef constructor
*
*	DESCRIPTION		Defualt values for AI's
*
***************************************************************************************************/
CAIComponentDef::CAIComponentDef( void )
{
	// general entity params
	//-----------------------
	m_eStartState = AI_IDLE;

	// Attack Selection
	m_pobCombatDef = 0;
	m_pobMovementSet = 0;
	m_bGrouped = false;
	m_bDrawViewCones = false;
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent constructor
*
*	DESCRIPTION		
*
***************************************************************************************************/
CAIComponent::CAIComponent( AI* pobParent, const CAIComponentDef* pobDef  ) 
:	m_bReqMoveTransition( false ),
	m_pobDefinition( pobDef ),
	m_pobNavPath(0),
	m_bStuck(false),
	m_bStuckLast(false),
	m_fThreshold(0.5f),
	m_bRun(false),
	m_bAnimLoop(false),
	m_bBallistaTargetChanged(false),
	m_obBallistaTargetOffset(CONSTRUCT_CLEAR),
	m_bNeedToFireBallista(false),
	m_pobEntity( pobParent ),
	m_fMovementMagnitude( 0.0f ),
	m_obMovementDirection( CONSTRUCT_CLEAR ),
	m_dirMovementFacing(CONSTRUCT_CLEAR),
	m_pobDirectTarget( 0 ),
	m_fPlayerVis( 0.0f ),
	m_obLastKnownPlayerPos( CONSTRUCT_CLEAR ),
	m_obActualPlayerPos( CONSTRUCT_CLEAR ),
	m_bCanAlwaysSee(false),
	m_bPlayerSeenEver(false),
	m_combatComponent(pobDef->m_pobCombatDef, m_pobEntity),
	m_recovering(false),
	m_fFormationMovementPaused(0.0f),
	m_iShutdownTicker(6),
	m_eCurrentMC(MC_NONE),
	m_bPlayingSingleAnim(false),
	m_bPlaySingleAnimUntilComplete(false),
	m_CoverPointIdx(-1),
	m_bInCover(true),
	m_MyWalkController(NULL),
	m_MyStrafeController(NULL),
	m_MyCloseStrafeController(NULL),
	m_eControllerModifier(AMCM_NONE)
{
	ATTACH_LUA_INTERFACE(CAIComponent);

	m_uiBehaviourTimeStamp = 0;

	// Create the ai formation component
	m_pFormationComponent = NT_NEW AIFormationComponent( pobParent );

	m_pobState = NT_NEW AIStates(this);
	m_pobFollowPoint = NT_NEW CAINavFollowPoint;

	// if our state is initialise to disabled, we reflect that here
	if(m_pobState->GetDisabled())
	{
		SetDisabled(true);
}

	// Action system declaring the weapon of choice. 
	SetActionUsing( m_pobDefinition->m_eUsingState );

	// !!! - Dario.
	m_Movement.SetParent(pobParent);
	m_Movement.SetPlayer(CEntityManager::Get().GetPlayer());
	m_Movement.SetCAIComp(this);

	m_AIVision.SetParent(pobParent);
	m_AIVision.SetCAIComp(this);

	//Store the parent for our targeting to access.
	m_AIRangedTargeting.SetParent(pobParent);

	//this->GetCombatComponent().SetVision(&m_AIVision);
	m_bPlayingFacingAction = false;

	// Update AIUpdate Indexes
	//static unsigned int	static_LocalNumberOfUpdateGroups = 0;
	//static unsigned int	static_LocalTotalNumberOfAIs = 0;
	//static unsigned int	static_LocalGroupNumberCount = 0;
	//static_LocalTotalNumberOfAIs++;
	//static_LocalGroupNumberCount++;
	//SetAIUpdateIndex(static_LocalNumberOfUpdateGroups);
	//SetTotalNumberofUpdateGroups(static_LocalNumberOfUpdateGroups);
	//if (static_LocalGroupNumberCount == 4)
	//{
	//	static_LocalNumberOfUpdateGroups++;
	//}
	//
	//SetCurrentUpdateGroups(0);

	//GavC - The controllers have been set-up from XML by this point, so by default assign the standard walk/run and strafe controllers.
	//After this point, the scripts can always change the current modifier to CROSSBOW or BAZOOKA to use those controllers instead.
	m_MyWalkController = m_pobDefinition->m_pobMovementSet->m_pobWalkingController;
	m_MyStrafeController = m_pobDefinition->m_pobMovementSet->m_pobStrafingController;
	m_MyCloseStrafeController = m_pobDefinition->m_pobMovementSet->m_pobCloseStrafingController;
}


/***************************************************************************************************
*
*	FUNCTION		CAIComponent destructor
*
*	DESCRIPTION		
*
***************************************************************************************************/
CAIComponent::~CAIComponent()
{
	m_pobDefinition = 0;

	KillPath();

	// decouple the message handler 
	if( !m_obLuaState.IsNil() )
	{
		ResetLuaFSM();
		GetParent()->GetMessageHandler()->RemoveHandler( m_obLuaState );
		m_obLuaState.SetNil();
	}

	NT_DELETE( m_pobState );
	NT_DELETE( m_pobFollowPoint );
	NT_DELETE( m_pFormationComponent );

	m_pobState = NULL;
	m_pobFollowPoint = NULL;
	m_pFormationComponent = NULL;

	CAINavGraphManager::Get().GetRepulsion()->Clear();
}


/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetDisabled
*
*	DESCRIPTION		set our component to be inactive
*
***************************************************************************************************/
void	CAIComponent::SetDisabled( bool bDisabled )
{
	if( m_pobState )
	{
		m_pobState->SetDisabled(bDisabled);
	}

	//if(bDisabled)
	//{
	//	m_bReqMoveTransition = false;
	//	m_recovering = false;
	//}
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::GetDisabled
*
*	DESCRIPTION		see if our component is currently disabled
*
***************************************************************************************************/
bool	CAIComponent::GetDisabled( void ) const
{
	if( m_pobState )
		return m_pobState->GetDisabled();
	return true;
}

//------------------------------------------------------------------------------------------
//!  public constant  FindAttackTarget
//!
//!  @return CEntity * Pointer to the new attack target, this could be NULL
//!
//!  @author GavB @date 20/07/2006
//------------------------------------------------------------------------------------------
const CEntity* CAIComponent::FindAttackTarget(void) const
{
	return m_AIVision.GetBestAttackTarget();
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::Shutdown
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAIComponent::Shutdown(void)
{
	// If in formation return the entity
	if( m_pFormationComponent )
	{
		m_pFormationComponent->Remove();
	}

	// release coverpoints for dead AIs
	if (m_CoverPointIdx != -1)
	{
		CoverPoint* coverPoint = AICoverManager::Get().GetCoverPoint( m_CoverPointIdx );
		ntAssert( coverPoint != NULL );
		coverPoint->SetAvailable( true );
	}

	// No longer need, the AI State
	NT_DELETE( m_pobState );
	m_pobState = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::Constructed( void )
*
*	DESCRIPTION		function called once our 'c++' object has been fully constructed
*
*	INPUTS			None
*
***************************************************************************************************/
void	CAIComponent::Constructed(void)
{
	ntAssert( m_pobState != NULL );
	m_pobState->FirstFrameInit();

	m_bFirstUpdate = true;

	if( m_obLuaState.IsTable() )
	{
		// Is there a construction funciton?
		NinjaLua::LuaFunction obOnConstructed;
		m_obLuaState.Get("OnConstructed", obOnConstructed );

		if( obOnConstructed.IsFunction() )
		{
			obOnConstructed(m_obLuaState);
		}
	}

	// Reset the stuck vars
	m_ptEntLastPos = GetParent()->GetPosition();
	m_fStuckTimer = 0.0f;

	// Force an update of the vision on the next update (if required)
	m_AIVision.ForceVisionScanNextUpdate();
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::Update( float fTimeChange )
*
*	DESCRIPTION		our update function
*
*	INPUTS			fTimeChange- time step, should really come from entities internal clock
*
***************************************************************************************************/
void	CAIComponent::Update( float fTimeChange )
{
	// If the entity is dead just return now. 
	if (GetParent()->ToCharacter()->IsDead())
	{
		// DARIO NOTE - I've disabled this for now as it stopped me respawning AIs...
		//if( m_iShutdownTicker && !--m_iShutdownTicker )
		//	Shutdown();

		return;
	}

	// Clear the information posted to the other components
	
	m_fMovementMagnitude = 0.0f;
	m_obMovementDirection.Clear();
	m_dirMovementFacing.Clear();
	m_pobDirectTarget = 0;

	// Debug do nothing if created from script
	if(m_pobState == NULL)
	{
		// John B's experimental code removed.
		ntAssert( 0 );
		return;
	}

#ifdef _NETWORK_ENABLED
	// Just return if the entity is networked but not local
	if (m_pobEntity->GetNetComponent() && !m_pobEntity->GetNetComponent()->IsLocalMaster())
	{
	   return;
	}
#endif

	if ((CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_P )))
	{
		SetDisabled( !GetDisabled() );
	}

	// update vision
	//if(!GetDisabled())
	//{
	//	VisionUpdate();
	//}

	// Update the path finding status
	PathfindStatusUpdate();

	// Update a test variable
	m_fFormationMovementPaused -= fTimeChange;
	
	if ( m_fFormationMovementPaused < 0.0f )
	{
		m_fFormationMovementPaused = 0.0f;
	}

	// Update the formation component
	if( m_pFormationComponent )
	{
		m_pFormationComponent->Update( fTimeChange );
	}


	// update our high level command logic. (AIStates)

//#if ACTIVATE_OLD_NAVGRAPH
//if (CAINavigationSystemMan::Get().IsNewAIActive())
	//{
		// !!! - (Dario). Here comes the new navigation and vision system update.
		if (!m_pobEntity->IsPaused() && !this->GetDisabled() )
		{
			CGatso::Start( "CEntityManager::AIMovement" );
			m_Movement.Update(fTimeChange);
			CGatso::Stop( "CEntityManager::AIMovement" );
			CGatso::Start( "CEntityManager::AIVision" );
			m_AIVision.Update(fTimeChange);
			CGatso::Stop( "CEntityManager::AIVision" );
			//Update our ranged targeting if we have a ranged weapon.
			if(m_pobEntity->IsCharacter() && m_pobEntity->ToCharacter()->GetRangedWeapon())
			{
				//TODO: More parameters here please. We want to avoid updating this if possible. Only update
				//when we can see the player in our targeting volume etc. Or perhaps have a flag for "bFullUpdate" that is only
				//true when in volume, but performs other checks (such as "have I just left the frustum? Should I trigger expansion?")
				//every frame...
				CGatso::Start( "CEntityManager::AIRangedTargeting" );
				m_AIRangedTargeting.Update(fTimeChange);
				CGatso::Stop( "CEntityManager::AIRangedTargeting" );
			}

			//if (m_Movement.IsUsingCannon())
			//{
			//	m_pobEntity->GetMovement()->m_obMovementInput.m_obFacingDirection = m_Movement.GetFacingAction();
			//	m_pobEntity->GetMovement()->m_obMovementInput.m_obMoveDirection = m_Movement.GetFacingAction();
			//	m_pobEntity->GetMovement()->m_obMovementInput.m_fMoveSpeed = 1.0f;
			//}

		}
	//}
	//else
	//{
	//	// For the OLD Navigation System to Work (Dario)
	//	m_pobState->Update( fTimeChange );
	//}
	

	// if we're playing a single anim, we need to keep track of its duration
	if(m_bPlayingSingleAnim)
	{
		m_fSingleAnimRemaining -= fTimeChange;
		if(m_fSingleAnimRemaining < 0.0f)
		{
			m_bPlaySingleAnimUntilComplete = false;
			m_bPlayingSingleAnim = false;
			//ntPrintf( "finished single anim on entity %s\n", m_pobEntity->GetName().c_str() );
			if( m_bPlaySingleAnimCompleteMsg )
			{
				m_pobEntity->GetMessageHandler()->Receive( CMessageHandler::Make(m_pobEntity, "msg_finished_single_anim") );
				m_bPlaySingleAnimCompleteMsg = false;
			}
		}
		else  if( m_dirMovementFacing.LengthSquared() > 0.0f )
		{
			m_pobEntity->GetMovement()->m_obMovementInput.m_obFacingDirection = m_dirMovementFacing;
		}
	}

	//if (!CAINavigationSystemMan::Get().IsNewAIActive())
	//{
	//// update the follow point for out current nav path
	//m_pobFollowPoint->Update( fTimeChange );
	//}

	// render the player follow point XXX: should this go in debug render?
	//if(!GetDisabled())
	//{
	//	m_pobFollowPoint->DebugRender();
	//}

	if(m_bFirstUpdate)
	{
		// if we don't have an attack component don't try and use it...
		if( GetParent()->GetAttackComponent() )
		{
			// Please can someone find a loving home for this abandoned line of code?
			GetParent()->GetAttackComponent()->AI_Setup_SetToDirectRequestMode();
			GetParent()->GetAttackComponent()->AI_Setup_RemoveAutoBlock();
		}
		m_bFirstUpdate = false;
	}

	//if (!CAINavigationSystemMan::Get().IsNewAIActive())
	//{
	//	m_bStuckLast = m_bStuck;
	//	CAIState_ACTION eAction = GetAction();

	//	// Test code to enable the AI's to detect being stuck against wall, obsticles etc.
	//	if( m_ptEntLastPos.Compare( GetParent()->GetPosition(), 0.01f ) 
	//		&& (	ACTION_WALK == eAction 
	//			||	ACTION_RUN == eAction
	//			||	ACTION_INVESTIGATE_WALK == eAction
	//			||	ACTION_PATROL_WALK == eAction
	//			||	ACTION_INTERACTING_WALK == eAction
	//			||	ACTION_STRAFE == eAction
	//			||	ACTION_SHUFFLE == eAction
	//			||	ACTION_PATROL_IDLE == eAction
	//			))
	//	{
	//		// If stuck for a given time, toggle the last stuck state, perhaps that'll wake up the entity
	//		float fInterval = fmodf( m_fStuckTimer, 1.0f );

	//		if( (fInterval + fTimeChange) > 1.0f )
	//			m_bStuckLast = false;

	//		m_fStuckTimer += fTimeChange;
	//		m_bStuck = m_fStuckTimer > 0.1f;
	//	}
	//	else
	//	{
	//		m_fStuckTimer = false;
	//		m_bStuck = false;
	//	}
	//}
	m_ptEntLastPos = GetParent()->GetPosition();
}


//!----------------------------------------------------------------------------------------------
//!
//!	CAIComponent::Reset
//!	Reset the AI Component - Used when respawning.
//!
//!----------------------------------------------------------------------------------------------
void CAIComponent::Reset()
{
	m_eCurrentMC = MC_NONE;
	m_recovering = false;
	m_fFormationMovementPaused = 0.0f;
	m_bPlayingSingleAnim = false;
	m_bPlaySingleAnimUntilComplete = false;
}



// Action interface, although just accessors they're in the cpp to avoid including AIStates in the header
/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetAction
*
***************************************************************************************************/
void	CAIComponent::SetAction( const CAIState_ACTION eAction )
{
	// Sanity Check
	ntError( m_pobState != NULL );

	if( !m_pobState )
		return;

	if( m_pobState->GetAction() != eAction )
	{
		// BODGE ON A BODGE!  ActivateSingleAnim records a time til the animation finishes...
		//                    If you change action to break out in the middle of one of these animations then the
		//                    time remaining was not being cleared...  This is now fixed here.
		m_fSingleAnimRemaining = -1.f;
	}

	m_pobState->SetAction( eAction );
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetActionStyle
*
***************************************************************************************************/
void CAIComponent::SetActionStyle( const CAIState_ACTIONSTYLE eStyle )
{
	// Sanity Check
	ntError( m_pobState != NULL );

	if( m_pobState )
	{
	m_pobState->SetActionStyle( eStyle );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetActionUsing
*
***************************************************************************************************/
void CAIComponent::SetActionUsing( const AI_ACTION_USING eUsing )
{
	// Sanity Check
	ntError( m_pobState != NULL );

	if( m_pobState )
	{
	m_pobState->SetActionUsing( eUsing );
}
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetActionDest
*
***************************************************************************************************/
void CAIComponent::SetActionDest( const CPoint& obDest )
{
	// Sanity Check
	ntError( m_pobState != NULL );

	if( m_pobState )
	{
		m_pobState->SetActionDest( obDest );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetActionFacing
*
***************************************************************************************************/
void CAIComponent::SetActionFacing(const CDirection& dirFacing) 
{
	if( m_pobState )
	m_pobState->SetActionFacing(dirFacing);
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::SetActionMoveSpeed
*
***************************************************************************************************/
void CAIComponent::SetActionMoveSpeed(float fSpeed)
{
	if( m_pobState )
		m_pobState->SetActionMoveSpeed(fSpeed);
}


/***************************************************************************************************
*
*	FUNCTION		CAIComponent::GetAction
*
***************************************************************************************************/
CAIState_ACTION CAIComponent::GetAction( void ) const
{
	if( m_pobState )
		return m_pobState->GetAction();
	return ACTION_NONE;
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::GetActionDest
*
***************************************************************************************************/
const CPoint& CAIComponent::GetActionDest( void ) const 
{
	if( m_pobState )
		return m_pobState->GetActionDest();

	static CPoint Error( 1.0f, 0.0f, 0.0f );
	return Error;
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::GetActionFacing
*
***************************************************************************************************/
const CDirection& CAIComponent::GetActionFacing() const
{
	if( m_pobEntity )
		return m_pobState->GetActionFacing();

	return m_pobEntity->GetMatrix().GetZAxis();
}


/***************************************************************************************************
*
*	FUNCTION		CAIComponent::MakePathToDest
*
***************************************************************************************************/
void CAIComponent::MakePathToDest(const CPoint* pobDest)
{
	const CPoint obDest = pobDest ? *pobDest : m_pobState->GetActionDest();

	m_pobNavPath = CAINavGraphManager::Get().MakePath( GetParent()->GetPosition(), obDest, &(m_pobEntity->GetKeywords()) );

	if (m_pobNavPath)
	{
		// turn off string pulling for group AI
		//m_pobNavPath->SetStringPulling( !m_pobDefinition->m_bGrouped );
		m_pobNavPath->Simplify();
		m_pobFollowPoint->SetCurrentPath( m_pobNavPath );
		m_pobFollowPoint->SetDest( obDest );
		m_ePathfindStatus = PF_SUCCESS;
	}
	else
	{
		m_ePathfindStatus = PF_FAILED;
		m_pobFollowPoint->SetCurrentPath( NULL );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIComponent::KillPath
*
***************************************************************************************************/
void CAIComponent::KillPath()
{
	NT_DELETE( m_pobNavPath );
	m_pobNavPath = NULL;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::GetBehaviourManager
//! Return a pointer to the behaviour manager.
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIBehaviourManager* CAIComponent::GetBehaviourManager()
{
	if( m_pobState )
	{
	return m_pobState->GetBehaviourManager();
}

	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::PathfindStatusUpdate
//! Checks to see if the AI is still in a valid location in the navgraph. If not, the AI is
//! flagged and disabled                                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------

void
CAIComponent::PathfindStatusUpdate()
{
	return;

	if( NSManager::Get().IsNinjaSequencePlaying() )
	{
		return;
	}
	if (m_pobEntity->IsInNinjaSequence())
	{
		return;
	}

	// don't worry about being out of the graph if the character is playing an anim or recovering from a combat strike
	if (!m_recovering && !m_bPlayingSingleAnim && !m_pobNavPath)
	{
		if (GetDisabled())
		{
#ifndef _GOLD_MASTER
			// print the status message, if there is one
			CPoint pt3D = m_pobEntity->GetPosition();
			pt3D.Y() += 1.0f;
			g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, m_strStatus.c_str() );
#endif
			return;
		}

		if ( !GetAIFormationComponent()->GetFormation() )
		{
			if (CAINavGraphManager::Get().InGraph( m_pobEntity->GetPosition() ))
			{
				m_strStatus.clear();
				return;
			}
			if ( m_pobEntity->HasBeenInUnsafePos() >= 30 * 2 )
			{
				m_strStatus = "OUT OF NAVGRAPH";
				SetDisabled( true );
			}
			m_pobEntity->IsInUnsafePosition();
		}

	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::DrawViewCone
//! Draws the AI's cone of vision
//!                                                                                         
//------------------------------------------------------------------------------------------

//static void DrawViewCone( const CPoint& obPosition, CDirection& obFacing, float fViewAngle, float fViewDist, unsigned int uColour )
//{
//	// debug draw the view cone
//	CPoint	obDiff1( obFacing );
//	obDiff1 /= obDiff1.Length();
//	obDiff1 *= fViewDist;
//
//	CPoint	obDiff2( obDiff1 );
//
//	// make one side of the cone
//	float fAngularModifier = fViewAngle;
//
//	float fCos = cos( fAngularModifier );
//	float fSin = sin( fAngularModifier );
//	float fNewX = fCos * obDiff1.X() + fSin * obDiff1.Z();
//	float fNewZ = fCos * obDiff1.Z() - fSin * obDiff1.X();
//
//	obDiff1.X() = fNewX;
//	obDiff1.Z() = fNewZ;
//
//	// and the other
//	fAngularModifier *= -1.0f;
//
//	fCos = cos( fAngularModifier );
//	fSin = sin( fAngularModifier );
//	fNewX = fCos * obDiff2.X() + fSin * obDiff2.Z();
//	fNewZ = fCos * obDiff2.Z() - fSin * obDiff2.X();
//
//	obDiff2.X() = fNewX;
//	obDiff2.Z() = fNewZ;
//
//	g_VisualDebug->RenderLine( obPosition, obPosition + obDiff1, uColour, DPF_NOCULLING );
//	g_VisualDebug->RenderLine( obPosition, obPosition + obDiff2, uColour, DPF_NOCULLING );
//}


void CAIComponent::HeadFacingUpdate()
{
	CHierarchy*	pobHierarchy = m_pobEntity->GetHierarchy();
	ntAssert( pobHierarchy );

	// Matrix to transform from world to model space
	CMatrix modelToWorld = pobHierarchy->GetRootTransform()->GetWorldMatrix();
	CMatrix worldToModel = modelToWorld.GetAffineInverse();

	static CHashedString headHash( "head" );
	int iIdx = pobHierarchy->GetTransformIndex( headHash );
	//ntAssert_p( iIdx != -1, ( "%s has no head!", m_pobEntity->GetName().c_str() ) );
	if (iIdx == -1)
	{
		// if there's no head, then fire a one time ntAssert and get the facing from the base joint
		one_time_assert_p( 0xB00B01, iIdx != -1, ( "%s has no head!", m_pobEntity->GetName().c_str() ) );
		m_pobEntity->GetLookDirection( m_obHeadFacing );
	}
	else
	{
		const CMatrix pobMatrix = pobHierarchy->GetTransform( iIdx )->GetWorldMatrix();
		m_obHeadFacing = pobMatrix.GetZAxis();
	}
}

//void
//CAIComponent::VisionUpdate()
//{
//	// get the player's position
//	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
//	//pobPlayer = NULL;
//	if(!pobPlayer)
//	{
//		m_fPlayerVis = 0.0f;
//		return;
//	}
//
//	if(pobPlayer)
//	{
//		m_obActualPlayerPos = pobPlayer->GetPosition();
//		//ntPrintf( "player x: %.2f   y: %.2f   z: %.2f\n",
//		//		m_obActualPlayerPos.X(), m_obActualPlayerPos.Y(), m_obActualPlayerPos.Z() );
//
//		if (m_bCanAlwaysSee)
//		{
//			m_fPlayerVis = 1.0f;
//			m_obLastKnownPlayerPos = m_obActualPlayerPos;
//			m_bPlayerSeenEver = true;
//			UNUSED(m_bAlerted);
//			return;
//		}
//	}
//
//	if (!m_pobEntity->IsEnemy())
//	{
//		return;
//	}
//
//	// create the vector from the AI to the player
//	CPoint	obPos( GetParent()->GetPosition() );
//	CPoint	obDiff( m_obActualPlayerPos - obPos );
//
//	if(obDiff.Length() < EPSILON)
//	{
//		m_fPlayerVis = 0.0f;
//		return;
//		//ntAssert( !"vision update couldn't find the player\n" );
//	}
//
//	// get the angle between this vector and the AI's facing vector
//	HeadFacingUpdate();
//	CDirection	obToPlayer( obDiff );
//
//    // if the angle is greater than our defined cone threshold then early out
//	float fAngle = MovementControllerUtilities::RotationAboutY( m_obHeadFacing, obToPlayer );
//
//	const float	fViewDist( 20.0f );
//	const float	fViewAngle( HALF_PI * 0.3f );
//	const float	fSuspicionDist( 30.0f );
//	const float	fSuspicionAngle( HALF_PI * 0.8f );
//
//	bool		bCanSee;
//	
//#define	VIS_NOTHING		0
//#define	VIS_SUSPICIOUS	1
//#define	VIS_SEEN		2
//
//	int	iVisState = VIS_NOTHING;
//
//	if(fabs( fAngle ) < fViewAngle && obDiff.LengthSquared() < (fViewDist * fViewDist))
//	{
//		iVisState = VIS_SEEN;
//	}
//	else if(fabs( fAngle ) < fSuspicionAngle && obDiff.LengthSquared() < (fSuspicionDist * fSuspicionDist))
//	{
//		iVisState = VIS_SUSPICIOUS;
//	}
//
//	if(m_bAlerted && iVisState == VIS_SUSPICIOUS)
//	{
//		iVisState = VIS_SEEN;
//	}
//
//	if(iVisState != VIS_NOTHING)
//	{
//		bCanSee = CAINavGraphManager::HasLineOfSight( obPos, m_obActualPlayerPos, pobPlayer, 1.5f );
//
//		if(bCanSee || m_bCanAlwaysSee)
//		{
//			// update visibility
//			if(iVisState == VIS_SUSPICIOUS)
//			{
//				m_fPlayerVis = 0.5f;
//			}
//			else
//			{
//				m_fPlayerVis = 1.0f;
//				m_bPlayerSeenEver = true;
//			}
//
//			// store last known pos
//			m_obLastKnownPlayerPos = m_obActualPlayerPos;
//		}
//		else
//		{
//			// reduce the visibility
//			m_fPlayerVis = 0.0f;
//		}
//	}
//	else
//	{
//			m_fPlayerVis = 0.0f;
//	}
//
//	//ntPrintf( "CanSee : %d\n", !bCollision );
//
//	// draw the view cones
//	if(	(CAINavGraphManager::Get().IsDebugRenderOn() || m_pobDefinition->m_bDrawViewCones)
//		&&	!GetParent()->ToCharacter()->IsDead() )
//	{
//		DrawViewCone( obPos, m_obHeadFacing, fViewAngle, fViewDist, 0xff20ff20 );
//		DrawViewCone( obPos, m_obHeadFacing, fSuspicionAngle, fSuspicionDist, 0xffff2020 );
//	}
//}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::ActivateController
//! Bring in the correct movement controller if it's not already being used.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CAIComponent::SetWalkRunMovementController(CHashedString hsMC )
{
	MovementControllerDef* pMC = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>(hsMC);
	 	
	if (!pMC)
	{
		ntPrintf("(ERROR!!!!) SetWalkRunMovementController() received an INVALID MovementControllerDef : [%s]\n",ntStr::GetString(hsMC));
		return false;
	}
	
	m_MyWalkController = pMC;
	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::ActivateController
//! Bring in the correct movement controller if it's not already being used.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CAIComponent::ActivateController(CAIComponent::MOVEMENT_CONTROLLER eMC, bool bAlwaysSet)
{
	ntAssert(eMC > MC_NONE);

	// If we're disabled don't add any more controllers!
	if (!m_pobState)// || m_pobState->GetDisabled())
		return false;
	
	if( m_recovering )
		return false;

	// If the entity isn't in an attack standard state, then exitout quick
	if( !m_pobEntity->GetAttackComponent()->AI_Access_IsInCSStandard() )
	{
		//ntAssert( false && "If you get this don't worry - just tell gav and then continue running the game" );
		return false;
	}

	if( !bAlwaysSet )
	{
		if( eMC == m_eCurrentMC && m_pobEntity->GetMovement()->IsAIRequestedController() )
			return true;
	}

	//ntPrintf("CAIComponent::ActivateController: (%s) MC:%d,%d, Recover:(%d)\n", GetParent()->GetName().c_str(), m_eCurrentMC, eMC, m_recovering );
	uint32_t uiCtrlID = m_pobEntity->GetMovement()->GetNewControllerCount();

	switch(eMC)
	{
		case MC_WALKING:
			if(m_MyWalkController != NULL)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_MyWalkController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_WALKING : MC_NONE;
			}
			else
			{
				ntAssert(false && "m_MyWalkController not set");
			}

			break;

		case MC_CROSSBOW_WALKING:
			if(m_pobDefinition->m_pobMovementSet && m_pobDefinition->m_pobMovementSet->m_pobCrossbowWalkingController)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_pobDefinition->m_pobMovementSet->m_pobCrossbowWalkingController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
				m_eCurrentMC = bCtrlAdded ? MC_CROSSBOW_WALKING : MC_NONE;
			}
			break;

		case MC_STRAFING:
			{
				bool bCtrlAdded = false;

				if ((m_Movement.GetActionStyle()==AS_AGGRESSIVE) && m_MyCloseStrafeController)
				{
					bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_MyCloseStrafeController, CMovement::DMM_STANDARD, m_eCurrentMC==MC_SINGLEANIM?0.2f:CMovement::MOVEMENT_BLEND);
				}
				else
				{
					ntAssert(m_MyStrafeController && "m_MyStrafeController not set");
					bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_MyStrafeController, CMovement::DMM_STANDARD, 0.4f);
				}

				m_eCurrentMC = bCtrlAdded ? MC_STRAFING : MC_NONE;
			}
			break;

		case MC_CLOSE_STRAFING:
			if(m_MyCloseStrafeController != NULL)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_MyCloseStrafeController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_CLOSE_STRAFING : MC_NONE;
			}
			else
			{
				ntAssert(false && "m_MyCloseStrafeController not set");
			}
			break;

		case MC_CROSSBOW_STRAFE:
			if(m_pobDefinition->m_pobMovementSet && m_pobDefinition->m_pobMovementSet->m_pobCrossbowStrafeController)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_pobDefinition->m_pobMovementSet->m_pobCrossbowStrafeController, CMovement::DMM_STANDARD, m_eCurrentMC==MC_SINGLEANIM?0.2f:CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_CROSSBOW_STRAFE : MC_NONE;
			}
			break;

		case MC_SHUFFLE:
			if(m_pobDefinition->m_pobMovementSet && m_pobDefinition->m_pobMovementSet->m_pobShuffleController)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_pobDefinition->m_pobMovementSet->m_pobShuffleController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_SHUFFLE : MC_NONE;
			}
			break;

		case MC_INVESTIGATE:
			if(m_pobDefinition->m_pobMovementSet && m_pobDefinition->m_pobMovementSet->m_pobInvestigateWalkController)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_pobDefinition->m_pobMovementSet->m_pobInvestigateWalkController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_INVESTIGATE : MC_NONE;
			}
			break;

		case MC_PATROL:
			if(m_pobDefinition->m_pobMovementSet && m_pobDefinition->m_pobMovementSet->m_pobPatrolWalkController)
			{
				bool bCtrlAdded = m_pobEntity->GetMovement()->BringInNewController(*m_pobDefinition->m_pobMovementSet->m_pobPatrolWalkController, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
				m_eCurrentMC = bCtrlAdded ? MC_PATROL : MC_NONE;
			}
			break;


		default:
			m_pobEntity->GetMovement()->SetAIRequestedController( false );
			return false;
	}


	m_pobEntity->GetMovement()->SetAIRequestedController( true );
	
	return uiCtrlID != m_pobEntity->GetMovement()->GetNewControllerCount();
}

//------------------------------------------------------------------------------------------
//!  public  RefreshController
//!
//!  @remarks Refresh the current controller
//!
//!  @author GavB @date 14/08/2006
//------------------------------------------------------------------------------------------
void CAIComponent::RefreshController()
{
	if (m_eCurrentMC != MC_NONE)
	{
		ActivateController( m_eCurrentMC, true );
		//ntPrintf("REFRESH CONTROLLER!!!!!");
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::SetDisabled
//! Static helper for disabling/enabling AI from callbacks
//!
//------------------------------------------------------------------------------------------
/*
void CAIComponent::SetDisabled( CEntity* pobEntity, bool bDisabled )
{
	if ( pobEntity && pobEntity->GetAIComponent() )
		pobEntity->GetAIComponent()->SetDisabled( bDisbabled );
}*/


//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::ActivateSingleAnim
//! Brings in a movement controller corresponding to a single anim. XXX: done by name, so fairly unpleasant
//!
//------------------------------------------------------------------------------------------

bool CAIComponent::ActivateSingleAnim( CHashedString pAnimName, float fAnimSpeed, float fAnimTimeOffsetPercentage, float fAnimBlend, bool bSendFinishedMsg )
{
	if(GetDisabled())
	{
		return false;
	}

	UNUSED( fAnimTimeOffsetPercentage );

	// Get a pointer to the entity we are acting on and check its validity
	ntAssert( m_pobEntity );
	ntAssert( m_pobEntity->GetMovement() );

	if(m_recovering)
	{
		CancelSingleAnim();
		return false;
	}

	if( GetAIFormationComponent()->GetFormation() && GetAIFormationComponent()->IsActive() )
	{
		//one_time_assert_p( 1, false, ("wake up, and smell the coffee") );

		const AIFormationSlot* pFormationSlot = ((const AIFormation*)GetAIFormationComponent()->GetFormation())->FindSlot( *m_pobEntity );
		if( pFormationSlot && !pFormationSlot->IsInPosition() && !GetAIFormationComponent()->GetFormationAttack() )
			return false;
	}


	// Define our movement
	FacingTransitionDef obDef;

	obDef.m_bApplyGravity		= true;
	obDef.m_fAngularSpeed		= 0.0f; // Degrees rotation per second
	obDef.m_fEarlyOut			= 0.0f; // Allows a transition to opt out slightly earlier (for blending)
    obDef.m_obAnimationName		= pAnimName;
	obDef.m_fAnimSpeed			= fAnimSpeed;
	obDef.m_fAnimOffset			= fAnimTimeOffsetPercentage;
	obDef.m_fStartTurnControl	= 0.0f; // Time offset in the animation when they player can control the character
	obDef.m_fEndTurnControl		= 0.0f; // The last time offset in the animation when the player can control the character
	obDef.m_fAlignToThrowTarget = 0.0f;

	// Push the controller onto our movement component
	m_pobEntity->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, fAnimBlend );

	// read the length of the animation
	m_fSingleAnimRemaining = obDef.GetDuration() * (1.0f - fAnimTimeOffsetPercentage) * fAnimSpeed;


	//ntPrintf( "anim name = %s   anim length = %.2f    ", pAnimName, m_pobEntity->FindAnimHeader( pAnimName, false )->m_fDuration );
	// The above is evil, and I'm going to add to it's evilness now, but it really needs fixing properly please! :) - JML
	m_fSingleAnimRemaining = m_fSingleAnimRemaining / fAnimSpeed;// - obDef.m_fTimeOffset;

	// set the flag saying we're playing a single anim
	m_bPlayingSingleAnim = true;
	m_bPlaySingleAnimCompleteMsg = bSendFinishedMsg;
	m_eCurrentMC = MC_SINGLEANIM;

	//ntPrintf( "ai(%s) playing anim: %s   length: %f\n", GetParent()->GetName().c_str(), pAnimName, m_fSingleAnimRemaining );
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::CancelSingleAnim
//! 
//!
//------------------------------------------------------------------------------------------

void CAIComponent::CancelSingleAnim()
{
	m_fSingleAnimRemaining = 0.0f;

	if( !m_bPlayingSingleAnim )
	{
		m_bPlaySingleAnimUntilComplete = false;
		ntAssert( m_pobEntity->GetMessageHandler() ); ntError( m_pobEntity->GetMessageHandler() );

		if( m_bPlaySingleAnimCompleteMsg )
		{
			m_pobEntity->GetMessageHandler()->Receive( CMessageHandler::Make(m_pobEntity, "msg_finished_single_anim") );
			m_bPlaySingleAnimCompleteMsg = false;
		}
			
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::CombatStateChanged
//! Called if the combat state is changed
//!
//------------------------------------------------------------------------------------------

void CAIComponent::CombatStateChanged( COMBAT_STATE eCombatState )
{
	// Set the recovering state
	m_recovering = eCombatState == CS_RECOVERING;

	// Send a message to the host entity about the state change
//	if( m_pobEntity->GetMessageHandler() )
//	{
//		ntstd::String obMsg = "MSG_";
//		obMsg += ObjectDatabase::Get().GetGlobalEnum( "COMBAT_STATE" ).GetName( eCombatState ).c_str();
//		m_pobEntity->GetMessageHandler()->Receive( CMessageHandler::Make(m_pobEntity, obMsg.c_str()) );
//	}

	// Inform the combat component that the state has changed. 
	m_combatComponent.CombatStateChanged(eCombatState);

	if( GetAIFormationComponent() && !GetAIFormationComponent()->GetFormationAttack() )
		GetAIFormationComponent()->CombatStateChanged( m_pobEntity, eCombatState );

	if( GetAIFormationComponent() && GetAIFormationComponent()->GetFormationAttack() )
		GetAIFormationComponent()->GetFormationAttack()->CombatStateChanged( m_pobEntity, eCombatState );
}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::CombatStateChanged
//! Called if the combat state is changed
//!
//------------------------------------------------------------------------------------------

void CAIComponent::HealthChanged( float fNewHealth, float fBaseHealth ) const
{
	// Send a message to the host entity about the state change
	if( m_pobEntity->GetMessageHandler() )
		m_pobEntity->GetMessageHandler()->Receive( CMessageHandler::Make(m_pobEntity, "OnHealthChange", fNewHealth / fBaseHealth, fBaseHealth ) );

	if( GetAIFormationComponent()->GetFormation() )
		GetAIFormationComponent()->GetFormation()->HealthChanged( m_pobEntity, fNewHealth, fBaseHealth );
}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::PlayVO
//! Uses chatterbox to play some VO for this entity. Hacked in for a milestone, so
//! definitely not the way to do things long-term
//!
//------------------------------------------------------------------------------------------

void CAIComponent::PlayVO( const char* bank, const char* cue )
{
	//NinjaLua::LuaFunction luaFunc = CLuaGlobal::Get().State().GetGlobals().Get<NinjaLua::LuaFunction>("PlayIncidentalVO");
	//luaFunc( bank, cue );

	//NinjaLua::LuaFunction luaFunc = CLuaGlobal::Get().State().GetGlobals().Get<NinjaLua::LuaFunction>("PlayTargetEntityVO");
	//luaFunc( m_pobEntity, bank, cue );
	//Audio_PlayEntitySound( m_pobEntity, CHANNEL_VOICE_HIGHPRI, SPATIALIZATION_HRTF, bank, cue );

	m_pobEntity->GetEntityAudioChannel()->Play( CHANNEL_VOICE_HIGHPRI, bank, cue );

}
//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::GetCurrentState
//!	Return string for current state
//!
//------------------------------------------------------------------------------------------

const char* CAIComponent::GetCurrentState () const
{
	if ( m_obLuaState.IsTable() ) 
	{
		NinjaLua::LuaObject obFunc;
		m_obLuaState.Get("GetStateName",obFunc);
		NinjaLua::LuaFunctionRet<const char*> obGetStateName(obFunc);
		return obGetStateName(m_obLuaState);
	} 

	if( GetDisabled() )
	{
		return "Disabled";
	}

	return "No State";
}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::ApplyNewAIDef
//! Test as to whether a new AI def could be applied during the life of an entity
//!
//------------------------------------------------------------------------------------------
void CAIComponent::ApplyNewAIDef(const char* pcNewAIDef)
{
	// Find the new AI def
	CAIComponentDef* pobDefinition = ObjectDatabase::Get().GetPointerFromName< CAIComponentDef* >( pcNewAIDef );
	if (!pobDefinition)
	{
		user_warn_p( 0, ("AI component def %s does not exist", pcNewAIDef) );
		return;
	}

	// Assign the ai define
	m_pobDefinition = pobDefinition;

	// Set the the combat def. 
	m_combatComponent.ApplyNewCombatDef( m_pobDefinition->m_pobCombatDef );

	// Action system declaring the weapon of choice. 
	SetActionUsing( m_pobDefinition->m_eUsingState );

}

//------------------------------------------------------------------------------------------
//!
//!	CAIComponent::SetRecovering
//! 
//!
//------------------------------------------------------------------------------------------
void CAIComponent::SetRecovering( bool recovering )
{
	m_recovering = recovering;

	if( recovering == false )
	{
		// Clear the current marked controller is the recovery has completed. 
		m_eCurrentMC = MC_NONE;

		// 
		m_bPlayingSingleAnim = false;

		// Reset the movement. 
		GetCombatComponent().ResetMovementState();
	}
}

// Dario

//------------------------------------------------------------------------------------------
//!	CAIComponent::SetIntention
//------------------------------------------------------------------------------------------
void CAIComponent::SetIntention	( unsigned int eIntentions, float fSpeed, unsigned int eMovFlag ) 
{ 
	m_Movement.SetIntention(eIntentions,fSpeed,eMovFlag);

	// Hack ... to prevent 
	if (eIntentions == NAVINT_GOTOLOCATORNODE)
	{
	//	CAINavigationSystemMan::Get().SteerToLocatorNode(m_pobEntity,eMovFlag);
	}
}

//------------------------------------------------------------------------------------------
//!	CAIComponent::SetEntity... and SetDestinationNode
//------------------------------------------------------------------------------------------
const CEntity* CAIComponent::GetEntityToAttack		( void ) const	{ return (m_Movement.GetEntityToAttack()); }
void CAIComponent::SetEntityToGoTo			( CEntity* pEnt )	{ if (pEnt) {m_Movement.SetEntityToGoTo(pEnt);} }
void CAIComponent::SetEntityToFollow		( CEntity* pEnt )	{ if (pEnt) {m_Movement.SetEntityToFollow(pEnt);} }
void CAIComponent::SetStartEndNodes			( const char* psSN, const char* psEN) { if (psSN && psEN)  {m_Movement.SetStartEndNodes(psSN, psEN);} }
void CAIComponent::SetDestinationNode		( const char* psN ) { if (psN)  {m_Movement.SetDestinationNode(psN);} }
void CAIComponent::SetPatrolling			( const char* psN ) { if (psN)  {CAINavigationSystemMan::Get().AddPatroller(m_pobEntity,psN);} }
void CAIComponent::SetDestinationRadius		( float f		)	{ m_Movement.SetDestinationRadius(f); }
void CAIComponent::SetAttackRange			( float f		)	{ m_Movement.SetAttackRange(f); }
void CAIComponent::SetMovementParam			( unsigned int uiParam, float fValue) { m_Movement.SetMovementParam(uiParam,fValue); }
void CAIComponent::SetVisionParam			( unsigned int uiParam, float fValue) { m_AIVision.SetVisionParam(uiParam,fValue); }
void CAIComponent::SetRangedParam			( unsigned int uiParam, float fValue) { m_Movement.SetRangedParam(uiParam,fValue); }
void CAIComponent::SetRangedTargetingParam	( unsigned int uiParam, float fValue) { m_AIRangedTargeting.SetRangedTargetingParam(uiParam, fValue); }
void CAIComponent::SetIdleFlags				( unsigned int uiFlags ) { m_Movement.SetIdleFlags(uiFlags); }
void CAIComponent::SetIdleClearsIntention	( bool b ) { m_Movement.SetIdleClearsIntention(b); }
void CAIComponent::SetExternalControlState	( bool b ) { m_Movement.SetExternalControlState(b); }
void CAIComponent::SetCoverAttitude			( unsigned int uiParam ) { m_Movement.SetCoverAttitude(uiParam); }
void CAIComponent::SetReuseCoverPoints		( bool b ) { m_Movement.SetReuseCoverPoints(b); }
void CAIComponent::SetMinWallDetRadius		( float f		)	{ m_Movement.SetMinWallDetRadius(f); }
void CAIComponent::SetMinMaxRadii			( float fMin, float fMax)	{ m_Movement.SetMinMaxRadii(fMin,fMax); }
void CAIComponent::SetTimeBetweenShoots		( float f )	{ m_Movement.SetTimeBetweenShoots(f); }
void CAIComponent::SetNumberOfConsecShots	( unsigned int u )	{ m_Movement.SetNumberOfConsecShots(u); }
void CAIComponent::SetCannonTarget			( CEntity* pE )							{ m_Movement.SetCannonTarget(pE); }
//void CAIComponent::SetCannonTargetLocatorPos ( const CPoint & obPos )				{ m_Movement.SetCannonTargetLocatorPos(obPos); }
void CAIComponent::ShootTheCannon			( void )								{ m_Movement.ShootTheCannon(); }

void CAIComponent::SetOffsetRadius			( float f )								{ m_Movement.SetOffsetRadius(f); }
void CAIComponent::SetShootingAccuracy		( float f )								{ m_Movement.SetShootingAccuracy(f); }
void CAIComponent::SetAlwaysMiss			( bool b )								{ m_Movement.SetAlwaysMiss(b); }

void CAIComponent::SetNavigGraphActiveLUA	( CHashedString hsName, bool b ) { CAINavigationSystemMan::Get().SetNavigGraphActiveLUA(hsName,b); }
void CAIComponent::SetWhackAMoleNode		( CHashedString hsNodeName ) { m_Movement.SetWhackAMoleNode(hsNodeName); }
void CAIComponent::SetAIBoolParam			( unsigned int eParam, bool b ) { m_Movement.SetAIBoolParam(eParam, b); }
void CAIComponent::SetGoToLastKnownPlayerPosInAttack ( bool b )	{ m_Movement.SetGoToLastKnownPlayerPosInAttack(b); }

float CAIComponent::GetRangedParameter		( unsigned int e ) { return m_Movement.GetRangedParameter(e); }

//! -------------------------------------------
//! SetHearingParam
//! -------------------------------------------
void CAIComponent::SetHearingParam ( unsigned int uiParam , float fValue )
{
	switch (uiParam)
	{
		case HP_IS_DEAF			:	if (fValue > 0.0f) 
										m_AIHearing.SetDeafen(true); 
									else 
										m_AIHearing.SetDeafen(false); 
									break;
		case HP_VOLUME_THRESHOLD:	if (fValue > 0.0f) 
										m_AIHearing.SetVolumeThreshold(fValue);
									else
										m_AIHearing.SetVolumeThreshold(0.0f);
									break;
	}								
}

//------------------------------------------------------------------------------------------
//!  public constant  GetFormFlagAttacking
//!
//!  @return bool 
//!
//!  @author GavB @date 25/07/2006
//------------------------------------------------------------------------------------------
bool CAIComponent::GetFormFlagAttacking( void ) const
{
	return GetAIFormationComponent() &&  GetAIFormationComponent()->GetFormationAttack();
}

//------------------------------------------------------------------------------------------
//!  public constant  IsInFormFormation
//!
//!  @return bool - Whether the entity is in formaiton or not
//!
//!  @author GavB @date 25/07/2006
//------------------------------------------------------------------------------------------
bool CAIComponent::IsInFormFormation( void ) const
{
	return GetAIFormationComponent() && GetAIFormationComponent()->GetFormation();
}

//------------------------------------------------------------------------------------------
//!  public  SetEntityToAttack
//!
//!  @param [in, out]   CEntity *    
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author Dario @date 30/06/2006
//------------------------------------------------------------------------------------------
void CAIComponent::SetEntityToAttack(const CEntity* pEntity)
{
	GetAIVision()->SetTarget(pEntity);
	GetCombatComponent().SetEntityToAttack(pEntity);
} 

//------------------------------------------------------------------------------------------
//!  public  SetEntityDescriptionsToAttack
//!
//!  @param [in, out]   const char *    
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author Dario @date 30/06/2006
//------------------------------------------------------------------------------------------
void CAIComponent::SetEntityDescriptionsToAttack(const char* pEntityDescriptions)
{
	GetCombatComponent().SetEntityDescriptionsToAttack(pEntityDescriptions);
}

//------------------------------------------------------------------------------------------
//!  public constant  ResetLuaFSM
//!
//!
//!  @author GavB @date 17/11/2006
//------------------------------------------------------------------------------------------
void CAIComponent::ResetLuaFSM(void) const
{
	// Is the lua state valid?
	if( !m_obLuaState.IsNil() )
	{
		CEntity* pOldTarg = CLuaGlobal::Get().GetTarg();
		CLuaGlobal::Get().SetTarg(this->GetParent());
		// Within the lua state, find a function called 'Clear Stack'
		NinjaLua::LuaFunction obPopHandler = NinjaLua::LuaFunction( m_obLuaState["ClearStack"] );

		// Only call the function if it's valid. 
		if( !obPopHandler.IsNil() )
		{
			// Call the function passing in the state and whether the game is running. 
			obPopHandler( m_obLuaState, ShellMain::Get().HaveLoadedLevel() );
		}

		CLuaGlobal::Get().SetTarg(pOldTarg);
	}
}
