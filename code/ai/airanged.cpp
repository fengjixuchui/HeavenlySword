//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file airanged.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/airanged.h"
#include "game/aicomponent.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/entityinfo.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"
#include "game/movementcontrollerinterface.h"
#include "core/visualdebugger.h"
#include "core/boundingvolumes.h"
#include "game/renderablecomponent.h"

#include "ai/aicoverpoint.h"

static float MIN_RANGE = 0.0f;
static float MAX_RANGE = 200.0f;


void AIFindCover::FindCover()
{
	CoverPoint* coverPoint = NULL;

	// if the AI has been given a named cover point, get that
	if (strcmp( m_pobAIComp->GetScriptCoverPointName(), "" ) != 0)
	{
		coverPoint = ObjectDatabase::Get().GetPointerFromName<CoverPoint*>( m_pobAIComp->GetScriptCoverPointName() );
		user_warn_p( coverPoint, ( "Cover point not found: %s", m_pobAIComp->GetScriptCoverPointName() ) );
	}
	else
	{
		// get nearest cover point in range
		coverPoint = AICoverManager::Get().GetNearestCoverPointWithinRange( m_pobEnt->GetPosition(), MIN_RANGE, MAX_RANGE );
		user_warn_p( coverPoint, ( "No cover point found for AI: %s", m_pobEnt->GetName().c_str() ) );
	}

	if (coverPoint)
	{
		// if AI already had a coverpoint, release it
		int oldCoverPointIdx = m_pobAIComp->GetCoverPoint();
		if (oldCoverPointIdx != -1)
		{
			CoverPoint* oldCoverPoint = AICoverManager::Get().GetCoverPoint( oldCoverPointIdx );
			ntAssert( oldCoverPoint );
			oldCoverPoint->SetAvailable( true );
		}

		m_pobAIComp->SetCoverPoint( coverPoint->GetNum() );
		m_bCoverFound = true;

		// set the point as being in use
		coverPoint->GetNum();
		coverPoint->SetAvailable( false );

	}
}

/*
void AIRanged::PickupWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}
*/

void AIOpenFire::FireWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}

static bool AtPos( const CPoint& entPos, const CPoint& targetPos, float threshold )
{
	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = entPos - targetPos;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < (threshold*threshold))
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	AIFindCover
//!
//------------------------------------------------------------------------------------------

bool
AIFindCover::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.1f);
	//const float	fFireTime			(0.3f);
	//const float	fReloadTime			(0.8f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_bCoverFound = false;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_LOCATECOVER );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_LOCATECOVER )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			FindCover();
			if (m_bCoverFound) // distance to player is greater than attack distance
			{
				int coverPointIdx = m_pobAIComp->GetCoverPoint();
				CoverPoint* coverPoint = AICoverManager::Get().GetCoverPoint( coverPointIdx );
				ntAssert( coverPoint != NULL );
				m_obCoverPosition = coverPoint->GetPos();

				SetState( STATE_GOTOCOVER );
			}
			else
			{
				AI_BEHAVIOUR_SEND_MSG( FINDCOVER_NONEFOUND );
			}

		OnUpdate
			//m_fTimer += fTimeChange;
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GOTOCOVER )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_INTERACTING_WALK );
			m_pobAIComp->SetActionDest( m_obCoverPosition );
			m_pobAIComp->SetActionMoveSpeed( 1.0f );

		OnUpdate
			m_fTimer += fTimeChange;

			if (AtPos( m_pobEnt->GetPosition(), m_obCoverPosition, 1.5f))
			{
				AI_BEHAVIOUR_SEND_MSG( FINDCOVER_INCOVER );
				m_pobAIComp->SetInCover( true );
			}
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

//------------------------------------------------------------------------------------------
//!
//!	AIOpenFire
//!
//------------------------------------------------------------------------------------------

bool
AIOpenFire::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.1f);
	const float	fFireTime			(0.3f);
	const float	fReloadTime			(0.8f);

BeginStateMachine

	//---------------------------------------------------------------
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
	
			m_bUsingCover = false;
			m_bHasFirePoint = false;
			int coverPointIdx = m_pobAIComp->GetCoverPoint();
			if (coverPointIdx != -1)
			{
				m_bUsingCover = true;
				CoverPoint* coverPoint = AICoverManager::Get().GetCoverPoint( coverPointIdx );
				m_obCoverPosition = coverPoint->GetPos();
				
				FirePoint* firePoint = coverPoint->GetFirePoint();
				if (firePoint)
				{
					m_bHasFirePoint = true;
					m_obFirePosition = firePoint->GetPos();
				}
			}

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				// if the AI isn't using a cover point, just fire
				if (m_pobAIComp->GetCoverPoint() == -1)
				{
					SetState( STATE_ATTACK );
				}
				else
				{
					SetState( STATE_MOVETOFIREPOINT );
				}
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_PAUSE )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fReloadTime )
			{
				SetState( STATE_MOVETOFIREPOINT );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_MOVETOFIREPOINT )
		OnEnter
			m_fTimer = 0.0f;

			ntAssert( m_bHasFirePoint );
			//m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetAction( ACTION_SHUFFLE );
			m_pobAIComp->SetActionDest( m_obFirePosition );
			m_pobAIComp->SetInCover( false );

		OnUpdate
			m_fTimer += fTimeChange;
			
			// XXX: hardwired to player as a target
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );

			if (AtPos( m_pobEnt->GetPosition(), m_obFirePosition, 0.5f ))
			{
				AI_BEHAVIOUR_SEND_MSG( OPENFIRE_ATFIREPOINT );
                SetState( STATE_ATTACK );
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_ATTACK )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			m_fTimer += fTimeChange;

			// XXX: hardwired to player as a target
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );

			if( m_fTimer > fFireTime )
			{
				// try to fire the weapon
				FireWeapon();
				SetState( STATE_RELOAD );
				AI_BEHAVIOUR_SEND_MSG( OPENFIRE_FIREDSHOT );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_RETURNTOCOVER )
		OnEnter
			m_fTimer = 0.0f;

			user_warn_p( m_bUsingCover, ( "AI requested STATE_RETURNTOCOVER with no cover point set" ) );
			
			//m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetAction( ACTION_SHUFFLE );
			m_pobAIComp->SetActionDest( m_obCoverPosition );
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );
			m_pobAIComp->SetActionMoveSpeed( 0.5f );

		OnUpdate
			m_fTimer += fTimeChange;

			if (AtPos( m_pobEnt->GetPosition(), m_obCoverPosition, 1.0f ))
			{
				m_pobAIComp->SetInCover( true );
                SetState( STATE_PAUSE );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_RELOAD )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fReloadTime )
			{
				if (m_bUsingCover)
				{
					SetState( STATE_RETURNTOCOVER );
				}
				else
				{
					SetState( STATE_ATTACK );
				}
			}

		OnExit
			m_fTimer = 0.0f;
EndStateMachine
}

//------------------------------------------------------------------------------------------
//!
//!	AIHoldFire
//!
//------------------------------------------------------------------------------------------

bool
AIHoldFire::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.1f);
//	const float	fFireTime			(0.3f);
//	const float	fReloadTime			(0.8f);

BeginStateMachine

	//---------------------------------------------------------------
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_PAUSE );
			}
			
	OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_PAUSE )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
			}

		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}





















void AIRanged::FindCover()
{
	CoverPoint* coverPoint = NULL;

	// if the AI has been given a named cover point, get that
	if (strcmp( m_pobAIComp->GetScriptCoverPointName(), "" ) != 0)
	{
		coverPoint = ObjectDatabase::Get().GetPointerFromName<CoverPoint*>( m_pobAIComp->GetScriptCoverPointName() );
		user_warn_p( coverPoint, ( "Cover point not found: %s", m_pobAIComp->GetScriptCoverPointName() ) );
	}
	else
	{
		// get nearest cover point in range
		coverPoint = AICoverManager::Get().GetNearestCoverPointWithinRange( m_pobEnt->GetPosition(), MIN_RANGE, MAX_RANGE );
		user_warn_p( coverPoint, ( "No cover point found for AI: %s", m_pobEnt->GetName().c_str() ) );
	}

	if (coverPoint)
	{
		// if AI already had a coverpoint, release it
		int oldCoverPointIdx = m_pobAIComp->GetCoverPoint();
		if (oldCoverPointIdx != -1)
		{
			CoverPoint* oldCoverPoint = AICoverManager::Get().GetCoverPoint( oldCoverPointIdx );
			ntAssert( oldCoverPoint );
			oldCoverPoint->SetAvailable( true );
		}

		m_pobAIComp->SetCoverPoint( coverPoint->GetNum() );
		m_bCoverFound = true;

		// set the point as being in use
		coverPoint->GetNum();
		coverPoint->SetAvailable( false );

	}
}
void AIRanged::FireWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}


bool AIRanged::CheckFallback( float threshold ) const
{
	// if the distance to the player is less than threshold, return true
	if ((m_pobAIComp->GetActualPlayerPos() - m_pobEnt->GetPosition()).LengthSquared() < (threshold * threshold))
	{
		return true;
	}
	return false;
}

bool
AIRanged::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(2.0f);
	const float	fFireTime			(0.3f);
	const float	fReloadTime			(0.8f);

BeginStateMachine

	//---------------------------------------------------------------
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_bCoverFound = false;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_LOCATECOVER );
			}
			
	OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_LOCATECOVER )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			FindCover();
			if (m_bCoverFound) // distance to player is greater than attack distance
			{
				SetState( STATE_GOTOCOVER );
			}
			else
			{
				user_warn_msg( ("Couldn't find suitable cover location") );
				//SetState( STATE_ATTACK );
				//m_pobParent->SendMsg( GETWEAPON_NOWEAPONFOUND ); 
			}

		OnUpdate
			//m_fTimer += fTimeChange;
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_GOTOCOVER )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_INTERACTING_WALK );
			m_pobAIComp->SetActionDest( m_obCoverPosition );

		OnUpdate
			m_fTimer += fTimeChange;

			if (AtPos( m_pobEnt->GetPosition(), m_obCoverPosition, 4.0f))
			{
				//m_pobAIComp->SetActionMoveSpeed( 0.5f );
				m_pobAIComp->SetActionMoveSpeed( 1.0f );
			}
			//if (AtPos( m_obCoverPosition, 1.0f))
			if (AtPos( m_pobEnt->GetPosition(), m_obCoverPosition, 1.5f))
			{
                SetState( STATE_PAUSE );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_PAUSE )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
				SetState( STATE_MOVETOFIREPOINT );
			}
			//if (CheckFallback( MIN_RANGE - 1.0f ))
			{
			//	SetState( STATE_LOCATECOVER );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_MOVETOFIREPOINT )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetActionDest( m_obFirePosition );

		OnUpdate
			m_fTimer += fTimeChange;
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );

			if (AtPos( m_pobEnt->GetPosition(), m_obFirePosition, 0.4f ))
			{
                SetState( STATE_ATTACK );
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_ATTACK )
		OnEnter
			m_fTimer = 0.0f;

			//m_pobAIComp->SetAction( ACTION_INTERACTING_WALK );
			//m_pobAIComp->SetActionMoveSpeed( 0.5f );
			//m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			m_fTimer += fTimeChange;
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );

			if( m_fTimer > fFireTime )
			{
				// try to fire the weapon
				FireWeapon();
				SetState( STATE_RELOAD );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_RETURNTOCOVER )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetActionDest( m_obCoverPosition );
			m_pobAIComp->SetActionFacing( m_pobAIComp->GetActualPlayerPos() ^ m_pobEnt->GetPosition() );
			m_pobAIComp->SetActionMoveSpeed( 0.5f );

		OnUpdate
			m_fTimer += fTimeChange;

			if (AtPos( m_pobEnt->GetPosition(), m_obCoverPosition, 1.0f))
			{
                SetState( STATE_PAUSE );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_RELOAD )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			//m_pobAIComp->SetActionDest( m_obWeaponLocation );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fReloadTime )
			{
				if (CheckFallback( MIN_RANGE - 1.0f ))
				{
					SetState( STATE_LOCATECOVER );
				}
				else
				{
					SetState( STATE_RETURNTOCOVER );
				}
			}

		OnExit
			m_fTimer = 0.0f;
EndStateMachine
}


//------------------------------------------------------------------------------------------
//!
//!	AIBallista
//!
//!	Behaviour class for soldiers using the ballista
//------------------------------------------------------------------------------------------

int		AIBallista::m_iCount = 0;

bool
AIBallista::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(1.0f);
	const float	fFireTime			(0.01f);
	const float	fAngleThreshold		(0.01f);

BeginStateMachine

	//---------------------------------------------------------------
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_bTargetFound = false;
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				FindTarget();
				if (!m_bTargetFound)
				{
					user_warn_msg( ("Couldn't find AI ballista target") );
				}
				SetState( STATE_ACQUIRETARGET );
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_ACQUIRETARGET )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;

			//ntPrintf( "BALLISTA: pre-update yawDiff: %.3f\n", m_fYawDiff );
			UpdateTargetPos();
			UpdateAngleDifferences();
			//ntPrintf( "BALLISTA: post-update yawDiff: %.3f\n", m_fYawDiff );

			//ntPrintf( "pitchDiff: %.3f\n", m_fPitchDiff );
			//ntPrintf( "TargetPos: ( %.3f, %.3f, %.3f )\n", m_obTargetPos.X(), m_obTargetPos.Y(), m_obTargetPos.Z() );
			//ntPrintf( "TargetFacing: ( %.3f, %.3f, %.3f )\n", obTargetFacing.X(), obTargetFacing.Y(), obTargetFacing.Z() );
			//ntPrintf( "Offset: ( %.3f, %.3f, %.3f )\n", m_pobAIComp->GetScriptBallistaTargetOffset().X(), m_pobAIComp->GetScriptBallistaTargetOffset().Y(), m_pobAIComp->GetScriptBallistaTargetOffset().Z() );

			// translate to movement inputs
			SetTrackingAction( m_fYawDiff, m_fPitchDiff );
			//ntPrintf( "BALLISTA: post-actionset yawDiff: %.3f\n", m_fYawDiff );
			
			if ( fabsf(m_fYawDiff) < fAngleThreshold && fabsf(m_fPitchDiff) < fAngleThreshold) // angular diff < threshold amount
			{
				//ntPrintf( "BALLISTA: behaviour %d STATE_ACQUIRETARGET found target\n", m_iNum );
				SetState( STATE_WATCHTARGET );
				AI_BEHAVIOUR_SEND_MSG( BALLISTA_TARGETACQUIRED );
			}
			if (m_pobAIComp->BallistaFireRequested())
			{
				//ntPrintf( "BALLISTA: behaviour %d STATE_ACQUIRETARGET received fire request\n", m_iNum );
				SetState( STATE_FIRE );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_WATCHTARGET )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			UpdateTargetPos();
			UpdateAngleDifferences();

			if ( fabsf(m_fYawDiff) > fAngleThreshold || fabsf(m_fPitchDiff) > fAngleThreshold) // angular diff > threshold amount
			{
				SetState( STATE_ACQUIRETARGET );
				AI_BEHAVIOUR_SEND_MSG( BALLISTA_TARGETLOST );
			}
			if (m_pobAIComp->BallistaFireRequested())
			{
				//ntPrintf( "BALLISTA: behaviour %d STATE_WATCHTARGET received fire request\n", m_iNum );
				SetState( STATE_FIRE );
			}
			
		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_FIRE )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;
			UpdateTargetPos();
			if( m_fTimer > fFireTime )
			{
				// try to fire the weapon
				FireWeapon();
				m_pobAIComp->BallistaFired();
				//ntPrintf( "BALLISTA: behaviour %d STATE_FIRE acknowledged fire request\n", m_iNum );
				AI_BEHAVIOUR_SEND_MSG( BALLISTA_SHOTFIRED );
				SetState( STATE_WATCHTARGET );
			}

		OnExit
			m_fTimer = 0.0f;

	//---------------------------------------------------------------
	AIState( STATE_PAUSE )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
				SetState( STATE_WATCHTARGET );
			}

		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

void AIBallista::FireWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}

// should get script defined target, but hardwire to player for mo
void AIBallista::FindTarget()
{
	m_bTargetFound = false;

	//ntPrintf( "BALLISTA: behaviour %d  target %s\n", m_iNum, m_pobAIComp->GetScriptBallistaTarget() );

	if (strcmp( m_pobAIComp->GetScriptBallistaTarget(), "" ) != 0)
	{
		m_pobTarget = ObjectDatabase::Get().GetPointerFromName<CEntity*>( m_pobAIComp->GetScriptBallistaTarget() );

		if (m_pobTarget != NULL)
		{
			m_bTargetFound = true;
		}
	}
}

void AIBallista::UpdateTargetPos()
{
	m_obTargetPos = m_pobTarget->GetPosition() + m_pobAIComp->GetScriptBallistaTargetOffset();
	//ntPrintf( "BALLISTA: behaviour %d  targetPos ( %.3f, %.3f, %.3f )\n", m_iNum, m_obTargetPos.X(), m_obTargetPos.Y(), m_obTargetPos.Z() );
}

void AIBallista::UpdateAngleDifferences()
{
	// Get angular difference between current facing and required facing
	CDirection obFacing;
	m_pobEnt->GetLookDirection( obFacing );
	obFacing /= obFacing.Length();
	CDirection obTargetFacing( m_obTargetPos - m_pobEnt->GetPosition() );
	obTargetFacing /= obTargetFacing.Length();

	m_fYawDiff = GetYawAngle( obFacing, obTargetFacing );
	m_fPitchDiff = 0.0f;
	if (fabs(m_fYawDiff) < PI * 0.2f)
	{
		m_fPitchDiff = GetPitchAngle( obFacing, obTargetFacing, m_pobEnt->GetMatrix() );
	}
}


float AIBallista::GetYawAngle( const CDirection& obVec1, const CDirection& obVec2 )
{
	// Remove any Y translation on the two input vectors
	CDirection obTempRotateFrom( obVec1.X(), 0.0f, obVec1.Z() );
	CDirection obTempRotateTo( obVec2.X(), 0.0f, obVec2.Z() );

	float fAngle = MovementControllerUtilities::RotationAboutY( obTempRotateFrom, obTempRotateTo );
	if (fabs( fAngle ) > PI)
	{
		if (fAngle < 0.0f)
		{
			fAngle += TWO_PI;
		}
		else
		{
			fAngle -= TWO_PI;
		}
	}

	return fAngle;
}


float AIBallista::GetPitchAngle( const CDirection& obVec1, const CDirection& obVec2, const CMatrix& worldMatrix )
{
	CDirection obTempRotateFrom = obVec1 * worldMatrix.GetAffineInverse();
	CDirection obTempRotateTo = obVec2 * worldMatrix.GetAffineInverse();

	float fAngle = MovementControllerUtilities::RotationAboutX( obTempRotateFrom, obTempRotateTo );
	if (fabs( fAngle ) > PI)
	{
		if (fAngle < 0.0f)
		{
			fAngle += TWO_PI;
		}
		else
		{
			fAngle -= TWO_PI;
		}
	}

	return fAngle;
}

float AIBallista::AngleDiffToInput( const float angle, const float minInput, const float maxInput )
{
	float input = minInput + (fabs( angle ) / PI) * (maxInput - minInput);
	if (angle < 0.0f)
	{
		input *= -1.0f;
	}

	// temporary, bodgy "fine adjustment" mode
	if (fabs( angle) < 0.01)
	{
		input *= 0.01f;
	}
	return input;
}

void AIBallista::SetTrackingAction( const float yawDiff, const float pitchDiff )
{
	const float MIN_YAW_INPUT = 0.57f;
	const float MIN_PITCH_INPUT = 0.7f;
	const float MAX_INPUT = 1.0f;


	CDirection inputDir( AngleDiffToInput( yawDiff, MIN_YAW_INPUT, MAX_INPUT ), 0.0f, AngleDiffToInput( pitchDiff, MIN_PITCH_INPUT, MAX_INPUT ) );

	m_pobAIComp->SetAction( ACTION_AIM_BALLISTA );
	m_pobAIComp->SetActionFacing( inputDir );
}



