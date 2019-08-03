//------------------------------------------------------------------------------------------
//!
//!	\file relativetransitions.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "relativetransitions.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "syncdmovement.h"
#include "anim/transform.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "Physics/world.h"

#include "core/visualdebugger.h"

#include "camera/camutils.h"

//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransitionDef::SimpleRelativeTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleRelativeTransitionDef::SimpleRelativeTransitionDef( void )
:	m_pobRelativeTransform( 0 ),
	m_fMovementDuration( 0.0f ),
	m_bOwnsTransform( false ),
	m_bLooping( false ),
	m_bNeedsToResynch( false ),
	m_pobResynchToEntity( 0 ),
	m_bInteractWithSyncdTransform( false ),
	m_fMaxRotationPerSecond( 0.0f ),
	m_obMovementSpeed( CONSTRUCT_CLEAR ),
	m_bReverseInteractiveStickInput( false ),
	m_fCollisionCheckDistance( 3.0f ),
	m_fInteractiveCollisionCheckStartHeight( 0.3f ),
	m_iCollisionCheckHeightCount( 5 ),
	m_fCollisionCheckHeightInterval( 0.4f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* SimpleRelativeTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SimpleRelativeTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransition::SimpleRelativeTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleRelativeTransition::SimpleRelativeTransition(	CMovement*							pobMovement, 
													const SimpleRelativeTransitionDef&	obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Set up the flags
	int iFlags = ANIMF_RELATIVE_MOVEMENT | ANIMF_INHIBIT_AUTO_DESTRUCT;

	// If we want to loop - set the flag
	if ( m_obDefinition.m_bLooping )
		iFlags |= ANIMF_LOOPING;

	if (m_obDefinition.m_bNeedsToResynch)
	{
		if (m_obDefinition.m_pobResynchToEntity)
			m_obDefinition.m_pobRelativeTransform->SetLocalMatrix( m_obDefinition.m_pobResynchToEntity->GetHierarchy()->GetRootTransform()->GetLocalMatrix() );
		else
			m_obDefinition.m_pobRelativeTransform->SetLocalMatrix( CMatrix( CONSTRUCT_IDENTITY ) );
	}	

	// Create our animation and add it to the animator
	m_obDefinition.m_pobAnimation->SetBlendWeight( 0.0f );
	m_obDefinition.m_pobAnimation->SetFlags( iFlags );
	m_obDefinition.m_pobAnimation->m_pobRelativeTransform = m_obDefinition.m_pobRelativeTransform;

	// If the duration is not specifically set - play the animation at its correct speed
	if ( m_obDefinition.m_fMovementDuration == 0.0f )
		m_obDefinition.m_fMovementDuration = m_obDefinition.m_pobAnimation->GetDuration();

	m_pobAnimator->AddAnimation( m_obDefinition.m_pobAnimation );
	m_obDefinition.m_pobAnimation->SetTimeRemaining( m_obDefinition.m_fMovementDuration );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransition::~SimpleRelativeTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SimpleRelativeTransition::~SimpleRelativeTransition( void )
{
	// If we have done an update then we have some clearing up to do
	if ( !m_bFirstFrame && m_obDefinition.m_pobAnimation->IsActive())
	{
		// Remove the animation from the animator if it has been added
		m_pobAnimator->RemoveAnimation( m_obDefinition.m_pobAnimation );
	}

	// If we own our transform we'll need to clean it up
	if ( m_obDefinition.m_bOwnsTransform )
	{
		m_obDefinition.m_pobRelativeTransform->RemoveFromParent();
		NT_DELETE( m_obDefinition.m_pobRelativeTransform );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool SimpleRelativeTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Debug render
	#ifdef _DEBUG
	g_VisualDebug->RenderAxis(m_obDefinition.m_pobRelativeTransform->GetWorldMatrix(),2.5f);
	#endif

	// If we are on the first frame - add the animation to the animator
	if ( m_bFirstFrame )
	{
		m_bFirstFrame = false;
	}

	// Set the weight on our animation and update it
	m_obDefinition.m_pobAnimation->SetBlendWeight( m_fBlendWeight );

	// Gravity setting - make sure it's false, should be effectively false because we should be in DMM_SOFT_RELATIVE anyway
	ApplyGravity(false);

	// If we want to take interaction, take it
	if ( m_obDefinition.m_bInteractWithSyncdTransform )
	{
		ntError( !m_pobMovement->IsInputDisabled() );

		// Get where we are and where we want to be
		CDirection obDesiredDirection = obMovementInput.m_obMoveDirection;
		CDirection obCurrentDirection = m_obDefinition.m_pobRelativeTransform->GetWorldMatrix().GetZAxis();
		obDesiredDirection.Normalise();
		obCurrentDirection.Normalise();

		// Raycast in some directions to avoid walls
		CDirection obHitNormal( CONSTRUCT_CLEAR );
		float fHitFraction = -1.0f;
		float fLookAhead = m_obDefinition.m_fCollisionCheckDistance; //m_obDefinition.m_obMovementSpeed.Length() * (1 / (m_obDefinition.m_fMaxRotationPerSecond / 3.1415f) );
		
		CPoint obRayStart( obCurrentMovementState.m_obPosition );
		obRayStart.Y() += m_obDefinition.m_fInteractiveCollisionCheckStartHeight; // Move the ray start quite far off the ground to avoid slopes and incidental static geom
		CPoint obRayEnd( CONSTRUCT_CLEAR );
		if (m_obDefinition.m_bReverseInteractiveStickInput)
			obRayEnd = obRayStart - (obCurrentDirection * fLookAhead);
		else
			obRayEnd = obRayStart + (obCurrentDirection * fLookAhead);

		CDirection obRayVector( obRayEnd - obRayStart );

		Physics::RaycastCollisionFlag obCollision;
		obCollision.base = 0;
		obCollision.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obCollision.flags.i_collide_with = Physics::LARGE_INTERACTABLE_BIT | Physics::SMALL_INTERACTABLE_BIT | Physics::AI_WALL_BIT;
		bool bDirectionCorrected = false;

		// Do however many collision ray casts we're asked to do
		for (int i = 0; i < m_obDefinition.m_iCollisionCheckHeightCount; i++)
		{
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayStart, obRayEnd, DC_YELLOW);
			#endif
			// Hit anything? If so, note it and bail
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ) )
			{			
				#ifdef _DEBUG
				CPoint obRayHit( obRayStart + (obRayVector*fHitFraction) );
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
				#endif
				bDirectionCorrected = true;

				break;
			}	

			obRayStart.Y() += m_obDefinition.m_fCollisionCheckHeightInterval;
			obRayEnd.Y() += m_obDefinition.m_fCollisionCheckHeightInterval;
		}

		// Old avoidance code
		// Prepare 5 ray directions, straight ahead, glancing left and right, fully left and right
		/*CDirection obRayVector( obRayEnd - obRayStart );
		CMatrix obRayRot( CONSTRUCT_IDENTITY );
		obRayRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), m_obDefinition.m_fCollisionAngleCheck * DEG_TO_RAD_VALUE );
		CDirection obRayVector2 = obRayVector * obRayRot;
		obRayRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), m_obDefinition.m_fCollisionAngleCheck * DEG_TO_RAD_VALUE * -1.0f);
		CDirection obRayVector3 = obRayVector * obRayRot;
		obRayRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), m_obDefinition.m_fCollisionAngleCheck * DEG_TO_RAD_VALUE * 2.0f);
		CDirection obRayVector4 = obRayVector * obRayRot;
		obRayRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), m_obDefinition.m_fCollisionAngleCheck * DEG_TO_RAD_VALUE * -2.0f);
		CDirection obRayVector5 = obRayVector * obRayRot;

		// Adjust their lengths because we don't need to look so far ahead at the sides
		obRayVector2.Normalise();
		obRayVector3.Normalise();
		obRayVector2 *= fLookAhead * 0.8f;
		obRayVector3 *= fLookAhead * 0.8f;
		obRayVector4.Normalise();
		obRayVector5.Normalise();
		obRayVector4 *= fLookAhead * 0.6f;
		obRayVector5 *= fLookAhead * 0.6f;

		#ifdef _DEBUG
		g_VisualDebug->RenderLine(obRayStart, obRayEnd, DC_YELLOW);
		#endif
		Physics::RaycastCollisionFlag obCollision;
		obCollision.base = 0;
		obCollision.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
		obCollision.flags.i_collide_with = Physics::LARGE_INTERACTABLE_BIT;
		bool bDirectionCorrected = false;
		// Check straight ahead, apply correction using a rotation about the hit normal
		if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ) )
		{
			CPoint obRayHit( obRayStart + (obRayVector*fHitFraction) );
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
			#endif
			float fAngle = MovementControllerUtilities::RotationAboutY( obHitNormal, obRayVector * -1.0f );
			fAngle *= -1.0f;
			CMatrix obRot( CONSTRUCT_IDENTITY );
			obRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), fAngle );
			obDesiredDirection = obHitNormal * obRot;
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayHit, obRayHit + obDesiredDirection, DC_GREEN);
			#endif
			bDirectionCorrected = true;
		}		

		if (!bDirectionCorrected)
		{			
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayStart, obRayStart+obRayVector2, DC_YELLOW);
			#endif
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayStart+obRayVector2, fHitFraction, obHitNormal, obCollision ) )
			{				
				#ifdef _DEBUG
				CPoint obRayHit( obRayStart + (obRayVector2*fHitFraction) );
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
				#endif
				obDesiredDirection = obRayVector3;
				#ifdef _DEBUG
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obDesiredDirection, DC_GREEN);
				#endif
				bDirectionCorrected = true;
			}
		}

		if (!bDirectionCorrected)
		{			
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayStart, obRayStart+obRayVector3, DC_YELLOW);
			#endif
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayStart+obRayVector3, fHitFraction, obHitNormal, obCollision ) )
			{
				#ifdef _DEBUG
				CPoint obRayHit( obRayStart + (obRayVector3*fHitFraction) );
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
				#endif
				obDesiredDirection = obRayVector2;
				#ifdef _DEBUG
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obDesiredDirection, DC_GREEN);
				#endif
				bDirectionCorrected = true;
			}
		}

		if (!bDirectionCorrected)
		{			
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayStart, obRayStart+obRayVector4, DC_YELLOW);
			#endif
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayStart+obRayVector4, fHitFraction, obHitNormal, obCollision ) )
			{
				#ifdef _DEBUG
				CPoint obRayHit( obRayStart + (obRayVector4*fHitFraction) );
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
				#endif
				obDesiredDirection = obRayVector5;
				#ifdef _DEBUG
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obDesiredDirection, DC_GREEN);
				#endif
				bDirectionCorrected = true;
			}
		}

		if (!bDirectionCorrected)
		{			
			#ifdef _DEBUG
			g_VisualDebug->RenderLine(obRayStart, obRayStart+obRayVector5, DC_YELLOW);
			#endif
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayStart+obRayVector5, fHitFraction, obHitNormal, obCollision ) )
			{
				#ifdef _DEBUG
				CPoint obRayHit( obRayStart + (obRayVector5*fHitFraction) );
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obHitNormal, DC_RED);
				#endif
				obDesiredDirection = obRayVector4;
				#ifdef _DEBUG
				g_VisualDebug->RenderLine(obRayHit, obRayHit + obDesiredDirection, DC_GREEN);
				#endif
				bDirectionCorrected = true;
			}
		}*/

		// Calc an angle difference, see if it's worth applying
		float fAngleDiff = MovementControllerUtilities::RotationAboutY( obCurrentDirection, obDesiredDirection );
		if (m_obDefinition.m_bReverseInteractiveStickInput)
			fAngleDiff *= -1;
		// Clamp it if needs be
		(fabs(fAngleDiff) < 0.05f || obMovementInput.m_fMoveSpeed < 0.05f) ? fAngleDiff = 0.0f : fAngleDiff = fAngleDiff;
 		float fMaxRotationThisFrame = ( m_obDefinition.m_fMaxRotationPerSecond ) * fTimeStep;
		if ( !bDirectionCorrected && fabs(fAngleDiff) > fMaxRotationThisFrame )
		{
			fAngleDiff < 0.0f ? fAngleDiff = -fMaxRotationThisFrame : fAngleDiff = fMaxRotationThisFrame;
		}

		// Get the matrix of our relative transform
		CMatrix obMtx = m_obDefinition.m_pobRelativeTransform->GetWorldMatrix();		
		CPoint obRelativeTransformTranslation = obMtx.GetTranslation();
		obMtx.SetTranslation( CPoint( CONSTRUCT_CLEAR ) );
		if (!bDirectionCorrected)
		{
			CDirection obMovement = ( ( m_obDefinition.m_obMovementSpeed * fTimeStep ) * obMtx );
			obRelativeTransformTranslation += obMovement;
		}

		// Get a rotation matrix
		CMatrix obRot( CONSTRUCT_IDENTITY );
		obRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f), fAngleDiff );
		// Apply rotation
		obMtx = obMtx * obRot;
		obMtx.SetTranslation( obRelativeTransformTranslation );
		
		// Set it back to the relative transform
		m_obDefinition.m_pobRelativeTransform->SetLocalMatrixFromWorldMatrix( obMtx );
	}

	// When we are finished indicate that to the movement component
	if ( ( !m_obDefinition.m_bLooping ) && ( m_obDefinition.m_pobAnimation->GetTime() > ( m_obDefinition.m_pobAnimation->GetDuration() - fTimeStep ) ) )
		return true;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}
