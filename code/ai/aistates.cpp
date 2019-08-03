/***************************************************************************************************
*
*	DESCRIPTION		Simple state machine for our AIs
*
*	NOTES
*
***************************************************************************************************/

#include "ai/aistates.h"
#include "ai/aibehaviourmanager.h"
#include "ai/aibehaviourcontroller.h"
#include "ai/aiformation.h"
#include "ai/aiformationcomponent.h"
#include "ai/aistatemachine.h"
#include "ai/ainavgraphmanager.h"
#include "ai/ainavnodes.h"
#include "ai/airepulsion.h"
#include "ai/ainavfollowpoint.h"

#include "core/timer.h"
#include "core/visualdebugger.h"

#include "input/inputhardware.h"

#include "game/entityinfo.h"
#include "game/aicomponent.h"
#include "game/randmanager.h"
#include "game/movementcontrollerinterface.h"
#include "game/messagehandler.h"

#include "camera/smoother.h"

//#define _COMBO_DEBUG

int	AIStates::m_iAllocedIDs = 0;
int	AIStates::m_iCurrentID = 0;

/***************************************************************************************************
*
*	FUNCTION		AIStates constructor
*
*	DESCRIPTION		
*
***************************************************************************************************/
AIStates::AIStates(CAIComponent* pobParent) :
	m_pobParent(pobParent),
	m_eCurrState(INVALID),
	m_eLastState(INVALID),
	m_bReadyToUse(false),
	m_bStateInitialised(false),
	m_bFirstStateFrame(false),
	m_eAction(ACTION_NONE),
	m_obActionDest( CONSTRUCT_CLEAR ),
	m_obLastActionDest( CONSTRUCT_CLEAR ),
	m_bActionDestSet(false),
	m_fMovementSpeed(1.0f),
	m_bActionFacingSet(false),
	m_fLookAroundTimer( 0.0f ),
	m_obLookAroundDir( CONSTRUCT_CLEAR )
{
	ntAssert(m_pobParent);

	m_bStateSpin = false;
	m_bDisabled = false;

	// use default smoother values
	SmootherDef smootherDef;
	m_pRepulsionSmoother = NT_NEW_CHUNK( Mem::MC_AI ) CPointSmoother(smootherDef); 

	m_bLastFramePos = false;
}

/***************************************************************************************************
*
*	FUNCTION		AIStates::FirstFrameInit
*
*	DESCRIPTION		Anoying but we have to call this after construction
*
***************************************************************************************************/
void	AIStates::FirstFrameInit(void)
{
	ntAssert(!m_bReadyToUse);

	// initialised to our default state
	switch(m_pobParent->GetDefinition()->m_eStartState)
	{
	case AI_IDLE:
		m_eCurrState = IDLE;
		break;

	case AI_CUTSCENE:
		m_eCurrState = DISABLED;
		m_bDisabled = true;
		break;

	default:
		ntAssert(0);
	}

	// m_obWorldInfo.GatherWorldInfo(0.0f);
	InitialiseCurrState();
/*
	const ntstd::string& obControllerName = m_pobParent->GetParent()->GetAttributeTable()->GetString("Controller");
	if( m_pobParent->GetLuaState().IsNil() || obControllerName == "AIController_Test")
	{
		// create a behaviour manager, controller and default behaviour (GroupMove)
		m_pobBehaviourController = NT_NEW_CHUNK( Mem::MC_AI ) AIMeleeBehaviourController;
		ntAssert( m_pobBehaviourController );
		ntAssert( m_pobParent );

		m_pobBehaviours = NT_NEW_CHUNK( Mem::MC_AI ) CAIBehaviourManager( *m_pobParent, m_pobBehaviourController );
		//m_pobBehaviours = NT_NEW CAIBehaviourManager(*m_pobParent);
		ntAssert(m_pobBehaviours);
		
		CAIStateMachine*	pobDefaultBehaviour;
		if (this->m_pobParent->GetDefinition()->m_bGrouped)
		{
			pobDefaultBehaviour = m_pobBehaviours->CreateBehaviour( CAIBehaviourManager::WALKTOPOINT, m_pobParent, m_pobParent->GetParent() );
		}
		else
		{
			pobDefaultBehaviour = m_pobBehaviours->CreateBehaviour( CAIBehaviourManager::PATROL, m_pobParent, m_pobParent->GetParent() );
			//pobDefaultBehaviour = m_pobBehaviours->CreateBehaviour( CAIBehaviourManager::WALKTOPOINT, m_pobParent, m_pobParent->GetParent() );
		}

		ntAssert(pobDefaultBehaviour);

		m_pobBehaviours->SetInitialState( pobDefaultBehaviour );
	}
	else
*/
	{
		m_pobBehaviourController = 0;
		m_pobBehaviours = 0;
	}

	m_bReadyToUse = true;
}

/***************************************************************************************************
*
*	FUNCTION		AIStates destructor
*
*	DESCRIPTION		
*
***************************************************************************************************/
AIStates::~AIStates(void)
{
	ShutdownCurrState(INVALID);

	NT_DELETE_CHUNK( Mem::MC_AI, m_pobBehaviours );
	NT_DELETE_CHUNK( Mem::MC_AI, m_pobBehaviourController );

	NT_DELETE_CHUNK( Mem::MC_AI, m_pRepulsionSmoother );
}

/***************************************************************************************************
*
*	FUNCTION		AIStates::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	AIStates::Update(float fTimeChange )
{
	ntAssert(m_bStateInitialised);
	ntAssert(m_bReadyToUse);

	// pull out any persistant state variables required by the states
	UpdatePersistantStateVars(fTimeChange);

	// update our current state
	UpdateCurrState( fTimeChange );
}

/***************************************************************************************************
*
*	FUNCTION		AIStates::DebugRender
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	AIStates::DebugRender(void) const
{
}


void AIStates::SetAction( const CAIState_ACTION eAction )
{
	m_eAction = eAction; m_fMovementSpeed = 1.0f;
	SetActionStyle( AS_NORMAL );
}

void AIStates::SetActionStyle( const CAIState_ACTIONSTYLE eStyle )
{
	m_eActionStyle = eStyle;
}

void AIStates::SetActionUsing( AI_ACTION_USING eUsing )
{
	m_eActionUsing = eUsing;
}


void AIStates::SetActionDest( const CPoint& obDest )
{
	if( !m_bLastFramePos )
	{
		m_obLastFramePos = obDest;
		m_bLastFramePos = true;
	}

	m_obActionDest = obDest;
	m_bActionDestSet = true;
	m_pobParent->SetPathfindStatus( CAIComponent::PF_SEARCHING );
}

void AIStates::SetActionFacing( const CDirection& dirFacing )
{
	m_dirActionFacing = dirFacing;
	m_bActionFacingSet = true;
}

void AIStates::SetActionMoveSpeed( const float fSpeed )
{
	m_fMovementSpeed = fSpeed;
}


/***************************************************************************************************
*
*	FUNCTION		AIStates::UpdatePersistantStateVars
*
*	DESCRIPTION		pull out any persistant state variables required by the states
*
*	NOTES			possibly we may be able to optimise this in the future, but for now, we dont 
*					have a fixed list of possible destination states from our current state.
*
*					NB, would be a good idea to use the actual info on what is a valid destination
*					state to check what to update based on our current state.
*
***************************************************************************************************/
void	AIStates::UpdatePersistantStateVars(float fTimeChange)
{
	UNUSED( fTimeChange );

	// DEPRECATED
	return;
}

/***************************************************************************************************
*
*	FUNCTION		AIStates::InitialiseCurrState
*
*	DESCRIPTION		Allocate anything and initialise our new active state
*
*	INPUTS			m_eCurrState is the state we're moving to
*					Any hand off information from the old state should be pushed into the persistant
*					State variables for this state, or pulled from the old one?
*
***************************************************************************************************/
void	AIStates::InitialiseCurrState(void)
{
	ntAssert(!m_bStateInitialised);
	m_fElapsedStateTime = 0.0f;
	m_bStateInitialised = true;
	m_bFirstStateFrame = true;
}

/***************************************************************************************************
*
*	FUNCTION		AIStates::ShutdownCurrState
*
*	DESCRIPTION		De-Allocate anything and shutdown our old active state
*
***************************************************************************************************/
void	AIStates::ShutdownCurrState(eSTATE eNextState)
{
	ntAssert(m_bStateInitialised);

	m_eLastState =  GetCurrState();
	m_bStateInitialised = false;
	m_eCurrState = eNextState;
}



/***************************************************************************************************
*
*	FUNCTION		AIStates::UpdateCurrState
*
*	DESCRIPTION		Update our current state
*
***************************************************************************************************/
void	AIStates::UpdateCurrState( float fTimeChange )
{
	ntAssert(m_bStateInitialised);

	eSTATE eNewState = GetCurrState();

	// Need to translate our disabled bool into a state type.
	if ( eNewState == DISABLED )
	{
		if( !GetDisabled() )
			eNewState = IDLE;
	}
	else if( GetDisabled() )
		eNewState = DISABLED;

	// right, update regular states
	if(eNewState != DISABLED)
	{
		// if there's no Navgraph, don't bother trying to do any of the behaviours
		if (!CAINavGraphManager::Get().HasNavData())
		{
			one_time_assert_p( 0xDEA01, false, ("AI will not do anything in a level without a valid NavGraph") );
		}
		else
		{
			// update the behaviours before attempting to resolve actions
			if( m_pobBehaviours )
			{
				m_pobBehaviours->Update( fTimeChange );
			}

			else
			{
				//if (m_eAction == ACTION_NONE)
				//{
				//	ntPrintf("");
				//}

				// handle single anim actions (with the most beautiful code ever written)
				
				// only trigger them if there's not an anim running already (d'oh!)
				if (m_pobParent->IsSimpleActionComplete())
				{
					if( ntstd::String("AI_ReactState") == m_pobParent->GetParent()->GetMessageHandler()->GetCurrentState() )
					{
					}			
					else if (m_eAction == ACTION_INVESTIGATE_LOOK)
					{
						m_pobParent->ActivateSingleAnim( "InvestigateCheck" );
					}
					else if (m_eAction == ACTION_INVESTIGATE_SHRUG)
					{
						m_pobParent->ActivateSingleAnim( "InvestigateFoundNothing" );
					}
					else if (m_eAction == ACTION_TAUNT)
					{
						m_pobParent->ActivateSingleAnim( "FormationJeer01" );
					}
					else if (m_eAction == ACTION_SCRIPTANIM)
					{
						m_pobParent->ActivateSingleAnim( m_pobParent->GetScriptAnimName() );
					}
					else if (m_eAction == ACTION_PATROL_LOOK)
					{
						if (grandf(1.0f) > 0.5f)
						{
							m_pobParent->ActivateSingleAnim( "PatrolLook1" );
						}
						else
						{
							m_pobParent->ActivateSingleAnim( "PatrolLook2" );
						}
					}
					else if (m_eAction == ACTION_PATROL_IDLE)
					{
						if (grandf(1.0f) > 0.5f)
						{
							m_pobParent->ActivateSingleAnim( "PatrolLook1" );
						}
						else
						{
							m_pobParent->ActivateSingleAnim( "PatrolLook2" );
						}
					}
					else if (m_eAction == ACTION_PATROL_SPOTTED)
					{
						m_pobParent->ActivateSingleAnim( "PatrolSpotted" );
					}
					else if (m_eAction == ACTION_PATROL_ALERT)
					{
						m_pobParent->ActivateSingleAnim( "PatrolAlert" );
					}
					else if (m_eAction == ACTION_PATROL_ALERTED)
					{
						m_pobParent->ActivateSingleAnim( "PatrolAlerted" );
					}
					else if(m_eAction == ACTION_INFORMATION_ANIM)
					{
						/*
						char pszAnim[256];
						
						if( GetActionStyle() == AS_AGGRESSIVE )
						{
							pszAnim = m_strAnimName.c_str();
						}
						else
						{
							int i = (grand() % 4) + 1;
							sprintf(pszAnim, "FormationAnim%d", i);
						}*/

						CHashedString sAnim = m_pobParent->GetFormationIdleAnim();
						float fAnimSpeedMulti = m_pobParent->GetAIFormationComponent()->GetFormation() ? m_pobParent->GetAIFormationComponent()->GetFormation()->GetIdleAnimSpeed() : 1.0f;

						// Set a default formation strafe if one isn't set. 
						if(ntStr::IsNull(sAnim) )
							sAnim = CHashedString("formationclose_inc_strafe");

						m_pobParent->ActivateSingleAnim(sAnim, ((grandf(.5f)+.75f)/1.5f) * fAnimSpeedMulti, grandf(0.6f), 0.4f);
						m_pobParent->PlaySingleAnimUntilComplete(true);
						return;
					}
				}

				// if we're playing a single anim, then no point progressing further here
				//if (!m_pobParent->IsSimpleActionComplete())
				//{
				//	break;
				//}

				// Ensure the correct controller is activated
				switch(m_eAction)
				{
					case ACTION_STRAFE:
						
						if( GetActionUsing() == AU_RCROSSBOW )
						{
							m_pobParent->ActivateController(CAIComponent::MC_STRAFING);
						}
						else
						{
							m_pobParent->ActivateController((GetActionStyle()==AS_AGGRESSIVE) ? CAIComponent::MC_CLOSE_STRAFING : CAIComponent::MC_STRAFING);
						}
						
						break;
					case ACTION_SHUFFLE:
						m_pobParent->ActivateController(CAIComponent::MC_SHUFFLE);
						break;
					case ACTION_WALK:
					case ACTION_RUN:					
						if( GetActionUsing() == AU_RCROSSBOW )
						{
							m_pobParent->ActivateController(CAIComponent::MC_CROSSBOW_WALKING);
						}
						else
						{
							m_pobParent->ActivateController(CAIComponent::MC_WALKING);
						}
						break;
					// XXX: we don't really want these to be separate actions, they should be replaced
					// with an "action" and "style"
					case ACTION_INVESTIGATE_WALK:
						m_pobParent->ActivateController(CAIComponent::MC_INVESTIGATE);
						break; 
					case ACTION_PATROL_WALK:
						m_pobParent->ActivateController(CAIComponent::MC_PATROL);
						break;

					default:
						break;
				}

#ifndef _GOLD_MASTER
				if( CInputHardware::Get().GetContext() == INPUT_CONTEXT_AI && m_bActionFacingSet )
				{
					g_VisualDebug->RenderLine(	m_pobParent->GetParent()->GetPosition() + CPoint(0.0f, 0.1f, 0.0f), 
												m_pobParent->GetParent()->GetPosition() + CPoint(0.0f, 0.1f, 0.0f) + m_dirActionFacing, 
												DC_PURPLE );
				}
#endif
				// Very simple if we're shuffling
				if( m_eAction == ACTION_SHUFFLE )
				{
					CDirection dHeading = m_obActionDest ^ m_pobParent->GetParent()->GetPosition();
					m_pobParent->SetMovementDirection(dHeading);
					m_pobParent->SetMovementMagnitude(clamp(dHeading.Length() * m_fMovementSpeed, 0.f, 1.f));
					m_pobParent->SetMovementFacing(m_dirActionFacing);
				}
				// equally simple if we're controlling the ballista
				else if( m_eAction == ACTION_AIM_BALLISTA )
				{
					m_pobParent->SetMovementDirection( m_dirActionFacing );
					m_pobParent->SetMovementMagnitude( clamp( m_dirActionFacing.Length(), 0.f, 1.f ) );
					m_pobParent->SetMovementFacing( m_dirActionFacing );
				}
				else
				{
					// Make sure the entity is facing its target. 
					if( m_eAction == ACTION_INFORMATION_ANIM )
					{
						m_pobParent->SetMovementFacing(m_dirActionFacing);
					}
					else
					{
						const CPoint	obEntityPos( m_pobParent->GetParent()->GetPosition() );
						float			fMoveMag( 0.0f );

						if (	m_eAction == ACTION_WALK
							||	m_eAction == ACTION_RUN
							||	m_eAction == ACTION_STRAFE
							||	m_eAction == ACTION_INVESTIGATE_WALK
							||	m_eAction == ACTION_INTERACTING_WALK
							||	m_eAction == ACTION_PATROL_WALK	)
						{
							// when pathfinding, facing is towards the follow point target is the followpoint
							//make sure we've got all the necessary bits and bobs required for pathfinding
							if (!m_pobParent->GetFollowPoint().PosSet())
							{
								m_pobParent->GetFollowPoint().SetPos( obEntityPos );
							}

							// if the action destination has been set this frame, then we should calculate a path
							if (m_bActionDestSet &&  (m_eAction != ACTION_STRAFE))
							{
								// KillPath is safe to call even when no path has been allocated
								m_pobParent->KillPath();
								m_pobParent->MakePathToDest();
								
								// I really don't understand all this navigation code, and I don't have time to either for the e3 demo
								// therefore, when entities walk into wall, I make it work. ! :)
								m_pobParent->GetFollowPoint().SetPos( m_obActionDest );

								CPoint	obTarget;
								if (m_pobParent->GetAIFormationComponent()->GetFormation() || m_eAction == ACTION_STRAFE)
								{
									obTarget = m_obActionDest;
									m_pobParent->GetFollowPoint().SetPos( m_obActionDest );
								}
								else
								{
									// we've set the position of the followpoint to be the current position of character, so now:
									//	- set the distance from char -> followpoint to zero
									//  - update the followpoint by a couple of timesteps to get the point ahead

									m_pobParent->GetFollowPoint().SetDest( m_obActionDest );
									m_pobParent->GetFollowPoint().SetDistSquared( 0.0f );
									m_pobParent->GetFollowPoint().Update( fTimeChange * 2.0f );

									obTarget = m_pobParent->GetFollowPoint().GetPos();
								}
								m_bActionDestSet = false;
							}
						}

						// get current destination
						CPoint	obTarget;
						if ( m_eAction == ACTION_STRAFE )
						{
							obTarget = m_obActionDest;
							
							if( m_pobParent->GetAIFormationComponent() && m_pobParent->GetAIFormationComponent()->GetFormation() )
							{
								// When in formation, try applying a smooth to the destination point. This 
								// might help entities that seem to dance. 
								obTarget = m_obLastActionDest + ((obTarget - m_obLastActionDest) * 0.5f);
							}
						}
						else
						{
							obTarget	= m_pobParent->GetFollowPoint().GetPos();
						}

						CPoint			obHeading	= obTarget - obEntityPos;
						obHeading.Y() = 0;
						float			fLength		= obHeading.Length();


						/*
						const float fSafeDist = 1.0f;
						if ( fLength <= fSafeDist && CAINavGraphManager::Get().GetRepulsion()->IsObstructed( obTarget, fSafeDist ))
						{
							m_fMovementSpeed = 0.0f;
						}
						*/

						if (m_eAction == ACTION_WALK ||	m_eAction == ACTION_PATROL_WALK || m_eAction == ACTION_INVESTIGATE_WALK)
						{
							fMoveMag = 0.5f;
						}
						else if (m_eAction == ACTION_NONE)
						{
							fMoveMag = 0.0f;
						}
						else
						{
							fMoveMag = 1.0f;
						}

						m_pobParent->GetFollowPoint().SetDistSquared( fLength * fLength );

						// adjust for local object avoidance (might not work well with formations, so don't use it for them just yet
						CPoint obRepulsion(CONSTRUCT_CLEAR);

						// This is a temp replacement for the replusion
						if ( !m_pobParent->GetAIFormationComponent() || !m_pobParent->GetAIFormationComponent()->GetFormation() )
						{
							// Avoid tables and barrels... 
							static const float TEST_DIST_OBSTRCUTION_ENT = 0.3f;
							static const float CHEATING_SCALAR = 3.0f;
							CSphereBound obResult;
							CPoint ptEnt			= obEntityPos;
							CPoint ptNormHeading	= obHeading / fLength;
							CPoint pt				= obEntityPos + (CHEATING_SCALAR * ptNormHeading);

							// Is the entity heading for an obstruction?
							if( CAINavGraphManager::Get().GetRepulsion()->IsObstructed( ptEnt, pt, TEST_DIST_OBSTRCUTION_ENT, &obResult ) 
								// Don't let small things bother the entity. (Test code)
								&& obResult.GetRadius() > 0.5f )
							{
								// Create a new point to navigate to. 
								CPoint ptAvoid = ptEnt + CDirection(obResult.GetPosition() - ptEnt).Cross( CDirection( 0.0f, CHEATING_SCALAR, 0.0f) );

								// Smooth out the effect of the obsticle by using the entities angle relative to the centre of the obsticle and
								// the destination of the ent;.
								CPoint ptP1 = obTarget - obResult.GetPosition();
								CPoint ptP2 = ptEnt - obResult.GetPosition();
								ptP1 /= ptP1.Length();
								ptP2 /= ptP2.Length();

								// Generate a magic number to apply the blend. 
								float fLerpMag = 1.0f - ((1.0f + ptP1.Dot( ptP2 ))/2.0f);

								// Blend between the requested and avoid vector
								pt = CPoint::Lerp( pt, ptAvoid, fLerpMag );

								obHeading		= pt - ptEnt;
								obHeading.Y()	= 0;
								fLength			= obHeading.Length();
							}

							/*
							CPoint obActualHeading = m_pobParent->GetParent()->GetPosition() - m_obLastFramePos;
							//CPoint obActualHeading = m_obLastFramePos - m_pobParent->GetParent()->GetPosition();
							if (obActualHeading.LengthSquared() > 0.001f)
							{
								obRepulsion = CAINavGraphManager::Get().GetRepulsion()->MakeForce( m_pobParent->GetParent(), 454564, obEntityPos, obActualHeading, fTimeChange  );
							}
							*/
						}
						else if( m_pobParent->GetAIFormationComponent() && m_pobParent->GetAIFormationComponent()->GetFormation() )
						{
							// Much better obsticle avoidance is required...
						}

#ifndef _GOLD_MASTER
						if( CInputHardware::Get().GetContext() == INPUT_CONTEXT_AI )
						{
							g_VisualDebug->RenderLine(	obTarget + CPoint(0.0f, 0.1f, 0.0f), 
														obEntityPos + CPoint(0.0f, 0.1f, 0.0f), 
														0xFFFFFFFF );

							g_VisualDebug->RenderLine(	obEntityPos + CPoint(0.0f, 0.1f, 0.0f), 
														obEntityPos + obHeading + CPoint(0.0f, 0.1f, 0.0f), 
														DC_RED );

						}
#endif
						m_pRepulsionSmoother->Update( obRepulsion, fTimeChange );				
						
						if (fLength > 0.0f)
						{
							obHeading /= fLength;
						}
						obHeading *= fMoveMag;

						CPoint obOriginalHeading = obHeading;
						if (m_eAction == ACTION_NONE)
						{
							obHeading = obRepulsion * 0.1f;
							fMoveMag = 0.0f;
							obOriginalHeading.Clear();
							m_fMovementSpeed = 0.0f;
						}
						else
						{
							obRepulsion = m_pRepulsionSmoother->GetTargetMean();
							obHeading += obRepulsion;
						}

						// if the repulsion vector takes us into a wall, then don't do it!
						//CPoint obTestPos = obEntityPos + obRepulsion;
						CPoint obTestPos = obEntityPos + obHeading;
						if (!CAINavGraphManager::Get().InGraph( obTestPos ))
						{
							obHeading = obOriginalHeading; 
						}

						if (m_eAction != ACTION_NONE)
						{
							fMoveMag = obHeading.Length();
						}

						// feed the required facing and speed to the movement controller				
						CDirection	obHeadingDir( obHeading );				// must be a better way than this

						// clamp the movemag so that if we say run, they run
						//const float fMinMag = (m_eAction == ACTION_RUN) ? 0.6f : 0.0f;
						const float fMinMag = 0.0f;
						//float fFinalMag = clamp(fMoveMag * m_fMovementSpeed, fMinMag, 1.0f);
						float fFinalMag = clamp(m_fMovementSpeed, fMinMag, 1.0f);

						//ntPrintf( "finalMag : %f\n", fFinalMag );

						obHeadingDir.Normalise();
						m_pobParent->SetMovementDirection( obHeadingDir );

						m_pobParent->SetMovementMagnitude( fFinalMag );
						m_pobParent->GetFollowPoint().SetAheadDist( fFinalMag*2.5f );

						if(m_bActionFacingSet)
						{
							m_pobParent->SetMovementFacing(m_dirActionFacing);
							m_bActionFacingSet = false;
						}
						else
						{
							m_pobParent->SetMovementDirection(obHeadingDir);
						}
						
						m_obLastFramePos = m_pobParent->GetParent()->GetPosition();
						m_obLastActionDest = obTarget;
						//ntPrintf( "updating AI : %X\n", this );
						//ntPrintf( "moveDir: (%.1f, %.1f, %.1f)   moveMag: %.2f\n", obHeadingDir.X(), obHeadingDir.Y(), obHeadingDir.Z(), fFinalMag ); 

						// post current position to avoidance manager
						//CAINavGraphManager::Get().GetRepulsion()->UpdatePos( obEntityPos, m_pobParent->GetAvoidanceID() );
						//CAINavGraphManager::Get().GetRepulsion()->UpdateDir( obHeadingDir, m_pobParent->GetAvoidanceID() );
					}
				}
			}
		}
	}

	m_fElapsedStateTime += fTimeChange;

	if(m_bFirstStateFrame)
		m_bFirstStateFrame = false;

	// swap to new state if requested
	if(GetCurrState() != eNewState)
	{
		ShutdownCurrState(eNewState);
		InitialiseCurrState();
	}
}

