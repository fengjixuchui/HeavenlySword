//------------------------------------------------------------------------------------------
//!
//!	\file interactiontransitions.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "interactiontransitions.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "game/entityinfo.h"
#include "game/inputcomponent.h"
#include "game/messagehandler.h"

#include "game/attacks.h"
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/interactioncomponent.h"

#include "awareness.h" // Required for targeting stuff
#include "anim/hierarchy.h"
#include "camera/camutils.h"
#include "objectdatabase/dataobject.h"

#include "Physics/system.h"
#include "physics/world.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/animatedlg.h"

#include "core/visualdebugger.h" // Debug rendering of ladder raycast

/*
START_STD_INTERFACE	(MoveToTransitionDef)
	IBOOL		(MoveToTransitionDef, ApplyGravity)
	ISTRING		(MoveToTransitionDef, AnimationName)
	IFLOAT		(MoveToTransitionDef, Distance)
	IFLOAT		(MoveToTransitionDef, MaximumRotationSpeed)
	IFLOAT		(MoveToTransitionDef, AnimSpeed)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE	(FacingTransitionDef)
	IBOOL		(FacingTransitionDef, ApplyGravity)
	IFLOAT		(FacingTransitionDef, AngularSpeed)
	IFLOAT		(FacingTransitionDef, EarlyOut)
	ISTRING		(FacingTransitionDef, AnimationName)
	IFLOAT		(FacingTransitionDef, AnimSpeed)
	IFLOAT		(FacingTransitionDef, StartTurnControl)
	IFLOAT		(FacingTransitionDef, EndTurnControl)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE	(SnapToTransitionDef)
	ISTRING		(SnapToTransitionDef, AnimationName)
	IPOINT		(SnapToTransitionDef, TranslationOffset)
	IQUAT		(SnapToTransitionDef, RotationOffset)
	IBOOL		(SnapToTransitionDef, ApplyGravity)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE
*/

/* // Use button mash object for crank
START_STD_INTERFACE( CrankSharedParameters )
	PUBLISH_VAR_AS							( m_obAnimCharacter,				AnimCharacter )
	PUBLISH_VAR_AS							( m_obAnimCrank,					AnimCrank )
	PUBLISH_VAR_AS							( m_obCharacterTranslationOffset,	CharacterTranslationOffset )
	PUBLISH_VAR_AS							( m_obCharacterRotationOffset,		CharacterRotationOffset )
	PUBLISH_VAR_AS							( m_fAnimAcceleration,				AnimAcceleration )
	PUBLISH_VAR_AS							( m_fAnimDeacceleration,			AnimDeacceleration )
	PUBLISH_VAR_AS							( m_fDoorOpenHeight,				DoorOpenHeight )
	PUBLISH_VAR_AS							( m_fDoorOpenSpeed,					DoorOpenSpeed )
	PUBLISH_VAR_AS							( m_fDoorCloseSpeed,				DoorCloseSpeed )
	PUBLISH_VAR_AS							( m_bDoorOpenOnStart,				DoorOpenOnStart )
	PUBLISH_VAR_AS							( m_bDoorLockedOnStart,				DoorLockedOnStart )
//	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
//	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
//	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE
*/

START_STD_INTERFACE( LadderParameters )
	DEFINE_INTERFACE_INHERITANCE(Attr_Interactable)
	COPY_INTERFACE_FROM(Attr_Interactable)

	PUBLISH_VAR_AS							( m_obAnimMoveTo,					AnimMoveTo )
	PUBLISH_VAR_AS							( m_obAnimRunTo,					AnimRunTo )
	PUBLISH_VAR_AS							( m_obAnimMountTop,					AnimMountTop )
	PUBLISH_VAR_AS							( m_obAnimMountBottom,				AnimMountBottom )
	PUBLISH_VAR_AS							( m_obAnimClimbUp,					AnimClimbUp )
	PUBLISH_VAR_AS							( m_obAnimClimbDown,				AnimClimbDown )
	PUBLISH_VAR_AS							( m_obAnimDismountTop,				AnimDismountTop )
	PUBLISH_VAR_AS							( m_obAnimDismountBottom,			AnimDismountBottom )
	PUBLISH_VAR_AS							( m_obMountTopPosition,				MountTopPosition )
	PUBLISH_VAR_AS							( m_obMountTopRotation,				MountTopRotation )
	PUBLISH_VAR_AS							( m_obMountBottomPosition,			MountBottomPosition )
	PUBLISH_VAR_AS							( m_obMountBottomRotation,			MountBottomRotation )
END_STD_INTERFACE

/* // Use buttonmash object for CW lever
START_STD_INTERFACE( CounterWeightLeverSharedParameters )
	PUBLISH_VAR_AS							( m_obAnimCharacterPush,			AnimCharacterPush )
	PUBLISH_VAR_AS							( m_obAnimLeverLift,				AnimLeverLift )
	PUBLISH_VAR_AS							( m_obAnimLeverFall,				AnimLeverFall )
	PUBLISH_VAR_AS							( m_obCharacterTranslationOffset,	CharacterTranslationOffset )
	PUBLISH_VAR_AS							( m_obCharacterRotationOffset,		CharacterRotationOffset )
	PUBLISH_VAR_AS							( m_fAnimAcceleration,				AnimAcceleration )
	PUBLISH_VAR_AS							( m_fAnimDeacceleration,			AnimDeacceleration )
END_STD_INTERFACE
*/

//------------------------------------------------------------------------------------------
//!
//!	EmptyTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* EmptyTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) EmptyTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	MoveToTransitionDef::MoveToTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
MoveToTransitionDef::MoveToTransitionDef( void )
:	m_bApplyGravity( true ),
	m_obAnimationName(),
	m_fDistance(0.0f),
	m_fMaximumRotationSpeed(0.0f),
	m_fAnimSpeed(1.0f),
	m_obOffset ( CONSTRUCT_CLEAR ),
	m_pobTargetEntity(0)
{
}


//------------------------------------------------------------------------------------------
//!
//!	MoveToTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* MoveToTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) MoveToTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	MoveToTransition::MoveToTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
MoveToTransition::MoveToTransition( CMovement* pobMovement, const MoveToTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
    m_obSingleAnimation->SetSpeed(obDefinition.m_fAnimSpeed);

	// Find the total distance moved by the animation
	CPoint obEndTranslation=m_obSingleAnimation->GetRootEndTranslation();
	obEndTranslation.Y()=0.0f;
	m_fAnimationDistance = obEndTranslation.Length();

	m_obAnimTranslation.Clear();
}




//------------------------------------------------------------------------------------------
//!
//!	MoveToTransition::~MoveToTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
MoveToTransition::~MoveToTransition( void )
{
	// Remove the animation if it has been added to the animator
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	MoveToTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool MoveToTransition::Update(	float						fTimeStep, 
								const CMovementInput&		/* obMovementInput */,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// This is the first update
	if ( m_bFirstFrame )
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;
	}

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	if ( m_obSingleAnimation->GetTime() < m_obSingleAnimation->GetDuration() )
	{
		CPoint obRelativePosition=(m_obDefinition.m_pobTargetEntity->GetPosition() + m_obDefinition.m_obOffset)  - obCurrentMovementState.m_obRootMatrix.GetTranslation();
		obRelativePosition.Y()=0.0f;

		/*CPoint obStart(m_obDefinition.m_pobTargetEntity->GetPosition() + m_obDefinition.m_obOffset);
		CPoint obEnd(obStart);
		obEnd.Y()+=2.0f;
		g_VisualDebug->RenderLine(obStart,obEnd,0xff0000ff);*/

		float fDistanceToMove=obRelativePosition.Length() - m_obDefinition.m_fDistance; // Distance character has to move

		// Find out the distance the anim has travelled
		m_obAnimTranslation+=m_obSingleAnimation->GetRootTranslationDelta();

		float fTimeRemaining = (m_obSingleAnimation->GetDuration()-m_obSingleAnimation->GetTime());
		float fAnimDistanceRemaining=(fTimeRemaining/m_obSingleAnimation->GetDuration())*m_fAnimationDistance;

		//float fAnimDistanceRemaining=m_obSingleAnimation->GetRootEndTranslation().Length()-m_obAnimTranslation.Length();

		float fScalar;
		
		if (fAnimDistanceRemaining>EPSILON) // Make sure we have a sensible value for the scalar
		{
            fScalar=fDistanceToMove/fAnimDistanceRemaining;
		}
		else
		{
			fScalar=0.0f;
		}

		obPredictedMovementState.m_obRootDeltaScalar=CDirection(fScalar,1.0f,fScalar);

		/*
		ntPrintf("BlendWeight=%f  Anim Translation=%f/%f  DistanceToMove=%f  AnimDistanceRemaining=%f\n",
			m_obSingleAnimation->GetBlendWeight(),
			m_obAnimTranslation.Length(),
			m_fAnimationDistance,
			fDistanceToMove,
			fAnimDistanceRemaining);
		//*/

		// ----- Turn the character towards the target -----
		
		CDirection obTargetDirection(m_obDefinition.m_pobTargetEntity->GetPosition() + m_obDefinition.m_obOffset);
		obTargetDirection -= CDirection(obCurrentMovementState.m_obRootMatrix.GetTranslation());

		float fTargetRotation = MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obRootMatrix.GetZAxis(), obTargetDirection );
		/*obStart = obCurrentMovementState.m_obRootMatrix.GetTranslation();
		
		CQuat obRot( CDirection(0.0f, 1.0f, 0.0f), fTargetRotation );
		CMatrix obMatrix;
		obMatrix.SetFromQuat(obRot);
		obEnd = obStart + obCurrentMovementState.m_obRootMatrix.GetZAxis() * obMatrix;
		
		g_VisualDebug->RenderLine(obStart,obEnd,0xffff0000);

		obEnd = obStart + obCurrentMovementState.m_obRootMatrix.GetZAxis()*2.0f;

		g_VisualDebug->RenderLine(obStart,obEnd,0xff00ff00);*/

		obPredictedMovementState.m_fProceduralYaw = (fTargetRotation/fTimeRemaining) * fTimeStep; // Provide a procedural rotation
		//obPredictedMovementState.m_fProceduralYaw = fTargetRotation; // Provide a procedural rotation
	}

	// When we are finished indicate that to the movement component
	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
		return true;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransitionDef::FacingMoveToTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
FacingMoveToTransitionDef::FacingMoveToTransitionDef( void )
:	m_bApplyGravity( true ),
	m_obAnimationName(),
	m_fDistance(0.0f),
	m_fMaximumRotationSpeed(0.0f),
	m_fAnimSpeed(1.0f),
	m_obOffsetLS ( CONSTRUCT_CLEAR ),
	m_obFacingDirLS( CONSTRUCT_CLEAR ),
	m_pobTargetEntity(0)
{
}


//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* FacingMoveToTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) FacingMoveToTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransition::FacingMoveToTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
FacingMoveToTransition::FacingMoveToTransition( CMovement* pobMovement, const FacingMoveToTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
    m_obSingleAnimation->SetSpeed(obDefinition.m_fAnimSpeed);

	// Find the total distance moved by the animation
	CPoint obEndTranslation=m_obSingleAnimation->GetRootEndTranslation();
	obEndTranslation.Y()=0.0f;
	m_fAnimationDistance = obEndTranslation.Length();

	m_obAnimTranslation.Clear();
}




//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransition::~FacingMoveToTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
FacingMoveToTransition::~FacingMoveToTransition( void )
{
	// Remove the animation if it has been added to the animator
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool FacingMoveToTransition::Update(	float						fTimeStep, 
								const CMovementInput&		/* obMovementInput */,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// This is the first update
	if ( m_bFirstFrame )
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;
	}

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	if ( m_obSingleAnimation->GetTime() < m_obSingleAnimation->GetDuration() )
	{
		float fTimeRemaining = (m_obSingleAnimation->GetDuration()-m_obSingleAnimation->GetTime());
		
		// FIX ME These ideal pos/rots can go in constructor

		// Targets rotation matrix
		CMatrix obMatrix = m_obDefinition.m_pobTargetEntity->GetMatrix();	
		obMatrix.SetTranslation(CPoint(0.0f,0.0f,0.0f));
		
		// Target position - target root + offset in target's space
		CPoint obIdealPosition ( m_obDefinition.m_pobTargetEntity->GetPosition() 
			+ m_obDefinition.m_obOffsetLS * obMatrix );

		//  Vector from character to target - only on XZ plane
		CDirection obIdealDelta( obIdealPosition - obCurrentMovementState.m_obRootMatrix.GetTranslation() );
		obIdealDelta.Y() = 0.0f; // In the plane!!

		// Unit Vector character should face in - object Z * use rotation
		CDirection obFacingVector = m_obDefinition.m_obFacingDirLS * obMatrix.GetAffineInverse().GetTranspose();

		//////////////////////////////////////////////////////////////////////
		// Debug - ideal position (blue)
		/*CPoint obStart(obIdealPosition);
		CPoint obEnd(obStart);
		obEnd.Y()+=2.0f;
		g_VisualDebug->RenderLine(obStart,obEnd,0xff0000ff);
	
		// Debug - facing vector (white)
		obEnd = obStart + obFacingVector;
		g_VisualDebug->RenderLine(obStart,obEnd,0xffffffff);

		// Debug - user position (magenta)
		obStart = obCurrentMovementState.m_obRootMatrix.GetTranslation();
		obEnd = obStart;
		obEnd.Y()+=1.0f;
		g_VisualDebug->RenderLine(obStart,obEnd,0xffff00ff);*/

		//////////////////////////////////////////////////////////////////////

		// No anim delta
		obPredictedMovementState.m_obRootDeltaScalar=CDirection(0.0f,1.0f,0.0f);

		// Movement to get there in the time needed
		obIdealDelta /= fTimeRemaining;

		// Movement required this fram
		obIdealDelta *= fTimeStep;

		obPredictedMovementState.m_obProceduralRootDelta = obIdealDelta;

		// ----- Turn the character towards the target -----
		
		CDirection obTargetDirection(obIdealPosition);
		obTargetDirection -= CDirection(obCurrentMovementState.m_obRootMatrix.GetTranslation());


		
		float fTargetRotationSpeed = MovementControllerUtilities::GetTurnSpeedToFace( obCurrentMovementState.m_obRootMatrix.GetZAxis(), obFacingVector , fTimeRemaining);
		
		//////////////////////////////////////////////////////////////////////////
		// Debug - Target rotation (yellow)
		/*		float fTargetRotation = MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obRootMatrix.GetZAxis(), obFacingVector );
		obStart = obCurrentMovementState.m_obRootMatrix.GetTranslation();		
		CQuat obRot( CDirection(0.0f, 0.0f, 1.0f), fTargetRotation );
		obMatrix.SetFromQuat(obRot);
		obEnd = obStart + obCurrentMovementState.m_obRootMatrix.GetZAxis() * obMatrix;
		g_VisualDebug->RenderLine(obStart,obEnd,0xffffff00);

		// Debug - character facing (green)
		obEnd = obStart + obCurrentMovementState.m_obRootMatrix.GetZAxis()*2.0f;
		g_VisualDebug->RenderLine(obStart,obEnd,0xff00ff00);*/
		//////////////////////////////////////////////////////////////////////////

		// Provide a procedural rotation
		obPredictedMovementState.m_fProceduralYaw = fTargetRotationSpeed * fTimeStep; 
	}

	// When we are finished indicate that to the movement component
	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
		return true;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}




//------------------------------------------------------------------------------------------
//!
//!	FacingTransitionDef::FacingTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
FacingTransitionDef::FacingTransitionDef( void )
:	m_bApplyGravity( true ),
	m_fAngularSpeed( 360.0f ),
	m_fEarlyOut( 0.0f ),
	m_obAnimationName(),
	m_fAnimSpeed( 1.0f ),
	m_fAnimOffset( 0.0f ),
	m_fStartTurnControl( 0.0f ),
	m_fEndTurnControl( 0.0f ),
	m_fAlignToThrowTarget( false ),
	m_fAnimLength(-1.0f)
{
}


//------------------------------------------------------------------------------------------
//!
//!	FacingTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* FacingTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) FacingTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	FacingTransition::FacingTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
FacingTransition::FacingTransition( CMovement* pobMovement, const FacingTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fPreviousAnimTime( 0.0f )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obSingleAnimation->SetSpeed(obDefinition.m_fAnimSpeed);
	

	// Assign the duration of the animation back to the def
	obDefinition.m_fAnimLength = m_obSingleAnimation->GetDuration() * (1.0f - m_obDefinition.m_fAnimOffset) * obDefinition.m_fAnimSpeed;
	m_obDefinition.m_fAnimLength = obDefinition.m_fAnimLength;

	// Definition initialisation
	m_obDefinition.m_fAngularSpeed*=DEG_TO_RAD_VALUE;

	if (m_obDefinition.m_fEndTurnControl==0.0f) // The default end turn control time is the end of the anim
	{
		m_obDefinition.m_fEndTurnControl=m_obSingleAnimation->GetDuration();
	}

	if (m_obDefinition.m_fAlignToThrowTarget)
	{
		CEntity* pobThisEntity=(CEntity*)m_pobMovement->GetParentEntity(); // Sorry, this is to get around the C2663 ntError!
		CEntity* pobTargetEntity=pobThisEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_THROW, pobThisEntity, true );

		if (pobTargetEntity)
		{
			CPoint obFrom=pobThisEntity->GetPosition();
			obFrom.Y()=0.0f;
			CPoint obTo=pobTargetEntity->GetPosition();
			obTo.Y()=0.0f;
			
			CMatrix obLocalMatrix;
			
			CCamUtil::CreateFromPoints(obLocalMatrix,obFrom,obTo);

			obLocalMatrix.SetTranslation(pobThisEntity->GetHierarchy()->GetRootTransform()->GetLocalTranslation());

			pobThisEntity->GetHierarchy()->GetRootTransform()->SetLocalMatrix(obLocalMatrix);
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	FacingTransition::~FacingTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
FacingTransition::~FacingTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	FacingTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool FacingTransition::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Stuff to do on the first update
	if ( m_bFirstFrame )
	{
		// Add the single animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;

		if( m_obDefinition.m_fAnimOffset > 0.0f )
		{
			m_obSingleAnimation->SetTime( m_obSingleAnimation->GetDuration() * m_obDefinition.m_fAnimOffset );
		}
	}

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we have reached the end of our movement - indicate to the movement component
	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep - m_obDefinition.m_fEarlyOut ) )
	{
		return true;
	}

	// Allow the character to be rotated by pad input
	else
	{
		const float fAnimTime=m_obSingleAnimation->GetTime();

		// Player rotation control
		if (fAnimTime>=m_obDefinition.m_fStartTurnControl && fAnimTime<m_obDefinition.m_fEndTurnControl)
		{
			static const float fPAD_MAGNITUDE_THRESHOLD = 0.25f;

			if (m_obDefinition.m_fAngularSpeed>EPSILON && obMovementInput.m_fMoveSpeed>fPAD_MAGNITUDE_THRESHOLD)
			{
				const float fTargetRotation = MovementControllerUtilities::RotationAboutY(	obCurrentMovementState.m_obRootMatrix.GetZAxis(), obMovementInput.m_obMoveDirection );

				float fTurn=m_obDefinition.m_fAngularSpeed * fTimeStep;

				if (fTargetRotation<0.0f)
				{
					fTurn=-fTurn;
				}

				if (fabsf(fTurn)>fabsf(fTargetRotation))
					fTurn=fTargetRotation;

				obPredictedMovementState.m_fProceduralYaw=fTurn;
				//obPredictedMovementState.m_fProceduralYaw=fTargetRotation;
			}
			else if( obMovementInput.m_obFacingDirection.LengthSquared() )
			{
				// Rather nasty, but stops flicking
				CDirection obFace = CDirection::Lerp( obCurrentMovementState.m_obRootMatrix.GetZAxis(), 
													  obMovementInput.m_obFacingDirection, 0.2f );

				float fTargetRotation = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obRootMatrix.GetZAxis(), obFace );

				obPredictedMovementState.m_fProceduralYaw=fTargetRotation;
			}

		}
		else if (m_fPreviousAnimTime<=m_obDefinition.m_fEndTurnControl && fAnimTime>m_obDefinition.m_fEndTurnControl)
		{
			CEntity* pobThisEntity=(CEntity*)m_pobMovement->GetParentEntity(); // Sorry, this is to get around the C2663 ntError!
			
			CEntity* pobTargetEntity=pobThisEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_THROW, pobThisEntity, true );

			if (pobTargetEntity)
			{
				ntAssert(pobTargetEntity->GetHierarchy());

				const CPoint& obTargetPos=pobTargetEntity->GetPosition();

				CDirection obTargetDirection(
					obTargetPos.X()-obCurrentMovementState.m_obPosition.X(),
					obTargetPos.Y()-obCurrentMovementState.m_obPosition.Y(),
					obTargetPos.Z()-obCurrentMovementState.m_obPosition.Z());
				
				const float fTargetRotation = MovementControllerUtilities::RotationAboutY(	obCurrentMovementState.m_obRootMatrix.GetZAxis(), obTargetDirection );

				obPredictedMovementState.m_fProceduralYaw=fTargetRotation;
			}
		}

		m_fPreviousAnimTime=fAnimTime;
	}

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}










//------------------------------------------------------------------------------------------
//!
//!	SnapToTransitionDef::SnapToTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
SnapToTransitionDef::SnapToTransitionDef( void )
:	
	m_pobTargetEntity(NULL),
	m_sTransformName("ROOT"),
	m_obAnimationName(),
	m_obTranslationOffset(CONSTRUCT_CLEAR),
	m_obRotationOffset(CONSTRUCT_IDENTITY),
	m_bApplyGravity(true)
{
}

//------------------------------------------------------------------------------------------
//!
//!	SnapToTransitionDef::~SnapToTransitionDef
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SnapToTransitionDef::~SnapToTransitionDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	SnapToTransitionDef::CreateInstance
//!	
//!
//------------------------------------------------------------------------------------------
MovementController* SnapToTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SnapToTransition( pobMovement, *this );
}











//------------------------------------------------------------------------------------------
//!
//!	SnapToTransition::SnapToTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
SnapToTransition::SnapToTransition( CMovement* pobMovement, const SnapToTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	if (!m_obDefinition.m_obAnimationName.IsNull()) // It is legal for this transition to not have an anim
	{
		m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
		m_obSingleAnimation->SetBlendWeight( 0.0f );
		m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	SnapToTransition::~SnapToTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SnapToTransition::~SnapToTransition( void )
{
	// Remove the animation if it was ever added
	if ( ( !m_bFirstFrame ) && ( !m_obDefinition.m_obAnimationName.IsNull() ) )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}

//------------------------------------------------------------------------------------------
//!
//!	SnapToTransition::Update
//!	Destruction
//!
//------------------------------------------------------------------------------------------
bool SnapToTransition::Update(	float						fTimeStep, 
								const CMovementInput&		/*obMovementInput*/,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Add the animation on the first frame if we have one
	if ( m_bFirstFrame )
	{
		if ( !m_obDefinition.m_obAnimationName.IsNull() )
			m_pobAnimator->AddAnimation( m_obSingleAnimation );

		// Snap the player's translation/rotation relative to the target entity

		if (m_obDefinition.m_pobTargetEntity)
		{
			Transform* pTransform = m_obDefinition.m_pobTargetEntity->GetHierarchy()->GetTransform(m_obDefinition.m_sTransformName);

			const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

			// Calculate the translation delta

			CDirection obTranslationOffset=CDirection(m_obDefinition.m_obTranslationOffset) * obTargetWorldMatrix;

			CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
			obNewPosition+=obTranslationOffset;

			obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);

			// Calculate the rotation delta
			
			CQuat obTargetRotation(obTargetWorldMatrix);

			CQuat obNewRotation=obTargetRotation * m_obDefinition.m_obRotationOffset;

			CMatrix obNewMatrix(obNewRotation);

			float ax,ay,az;
			
			CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

			obPredictedMovementState.m_fProceduralYaw=ay;//MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obNewMatrix.GetZAxis());
			obPredictedMovementState.m_bApplyExplicitRotations = true;
		}
		else // Move relative to the world
		{
			CMatrix obTargetWorldMatrix(CONSTRUCT_IDENTITY);

			// Calculate the translation delta

			CDirection obTranslationOffset=CDirection(m_obDefinition.m_obTranslationOffset) * obTargetWorldMatrix;

			CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
			obNewPosition+=obTranslationOffset;

			obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);

			// Calculate the rotation delta

			CQuat obNewRotation=CQuat(obTargetWorldMatrix) * m_obDefinition.m_obRotationOffset;

			CMatrix obNewMatrix(obNewRotation);

			float ax,ay,az;
			
			CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

			obPredictedMovementState.m_fProceduralYaw=ay;//MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obNewMatrix.GetZAxis());
			obPredictedMovementState.m_bApplyExplicitRotations = true;
		}

		

		// Clear the first frame flag
		m_bFirstFrame = false;
	}

	obPredictedMovementState.m_obRootDeltaScalar.Clear();
	obPredictedMovementState.m_fRootRotationDeltaScalar=0.0f;
	
	// Drop out early if there is no defined animation
	if ( m_obDefinition.m_obAnimationName.IsNull() )
		return true;

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Gravity setting
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// When we are finished indicate that to the movement component
	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
		return true;

	return false;
}





//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransitionDef::CreateInstance
//!	Construction
//!
//------------------------------------------------------------------------------------------
MovementController* LinkedMovementTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) LinkedMovementTransition( pobMovement, *this );
}



//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransition::LinkedMovementTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
LinkedMovementTransition::LinkedMovementTransition( CMovement* pobMovement, const LinkedMovementTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
	//m_obSingleAnimation->SetSpeed(1.0f);
}

//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransition::~LinkedMovementTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
LinkedMovementTransition::~LinkedMovementTransition( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}

//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransition::Update
//!	Destruction
//!
//------------------------------------------------------------------------------------------
bool LinkedMovementTransition::Update(	float						/*fTimeStep*/, 
										const CMovementInput&		/*obMovementInput*/,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation on the first frame if we have one
	if ( m_bFirstFrame )
	{
		if ( !m_obDefinition.m_obAnimationName.IsNull() )
			m_pobAnimator->AddAnimation( m_obSingleAnimation );

		m_bFirstFrame = false;
	}

	// Apply gravity if required
	ApplyGravity(false);

	// Snap the player's translation/rotation relative to the target entity

	if (m_obDefinition.m_pobTargetEntity)
	{
		Transform* pTransform = m_obDefinition.m_pobTargetEntity->GetHierarchy()->GetTransform(CHashedString(m_obDefinition.m_obTargetTransform));

		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta
		CDirection obTranslationOffset=CDirection(m_obDefinition.m_obTranslationOffset) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);		
		
		// Calculate the rotation delta
		obPredictedMovementState.m_fProceduralYaw=MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obTargetWorldMatrix.GetZAxis());
	}
		
	obPredictedMovementState.m_obRootDeltaScalar.Clear();
	obPredictedMovementState.m_fRootRotationDeltaScalar=0.0f;
	
	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransitionDef::CreateInstance
//!	Construction
//!
//------------------------------------------------------------------------------------------
MovementController* FullyLinkedMovementTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) FullyLinkedMovementTransition( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	FullyLinkedMovementTransition::FullyLinkedMovementTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
FullyLinkedMovementTransition::FullyLinkedMovementTransition( CMovement* pobMovement, const FullyLinkedMovementTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
	m_obSingleAnimation->SetSpeed(1.0f);
}

//------------------------------------------------------------------------------------------
//!
//!	FullyLinkedMovementTransition::~FullyLinkedMovementTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
FullyLinkedMovementTransition::~FullyLinkedMovementTransition( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}

//------------------------------------------------------------------------------------------
//!
//!	FullyLinkedMovementTransition::Update
//!	Destruction
//!
//------------------------------------------------------------------------------------------
bool FullyLinkedMovementTransition::Update(	float						/*fTimeStep*/, 
										const CMovementInput&		/*obMovementInput*/,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation on the first frame if we have one
	if ( m_bFirstFrame )
	{
		if ( !m_obDefinition.m_obAnimationName.IsNull() )
			m_pobAnimator->AddAnimation( m_obSingleAnimation );

		m_bFirstFrame = false;
	}

	// Apply gravity if required
	ApplyGravity(false);

	// Snap the player's translation/rotation relative to the target entity

	if (m_obDefinition.m_pobTargetEntity)
	{
		Transform* pTransform = m_obDefinition.m_pobTargetEntity->GetHierarchy()->GetTransform(CHashedString(m_obDefinition.m_obTargetTransform));

		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta
		CDirection obTranslationOffset=CDirection(m_obDefinition.m_obTranslationOffset) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);		
		
		// Calculate the rotation delta
		
		CQuat obTargetRotation(obTargetWorldMatrix);

		CQuat obNewRotation=obTargetRotation * m_obDefinition.m_obRotationOffset;

		CMatrix obNewMatrix(obNewRotation);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

		obPredictedMovementState.m_fProceduralPitch=ax;
		obPredictedMovementState.m_fProceduralYaw=ay;
		obPredictedMovementState.m_fProceduralRoll=az;
		obPredictedMovementState.m_bApplyExplicitRotations = true;
	}
		
	obPredictedMovementState.m_obRootDeltaScalar = CDirection( 0.0f, 0.0f, 0.0f);
	obPredictedMovementState.m_fRootRotationDeltaScalar=0.0f;
	
	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );
	m_obSingleAnimation->SetSpeed(1.0f);

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransitionDef::CreateInstance
//!	Construction
//!
//------------------------------------------------------------------------------------------
MovementController* CorrectiveMovementTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CorrectiveMovementTransition( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransition::CorrectiveMovementTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
CorrectiveMovementTransition::CorrectiveMovementTransition( CMovement* pobMovement, const CorrectiveMovementTransitionDef& obDefinition )
:	MovementController( pobMovement )
,	m_obDefinition( obDefinition )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );
}

//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransition::~CorrectiveMovementTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CorrectiveMovementTransition::~CorrectiveMovementTransition( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransition::Update
//!	Destruction
//!
//------------------------------------------------------------------------------------------
bool CorrectiveMovementTransition::Update(	float						/*fTimeStep*/, 
										const CMovementInput&		/*obMovementInput*/,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Apply gravity if required
	ApplyGravity(false);

	// Snap the player's translation/rotation to where we want it to be

	// Calculate the position delta
	obPredictedMovementState.m_obProceduralRootDelta=CDirection(m_obDefinition.m_obTargetPostion-obCurrentMovementState.m_obPosition);		
	
	// Calculate the rotation delta	
	CMatrix obNewMatrix(m_obDefinition.m_obTargetRotation);

	float ax,ay,az;
	
	CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

	obPredictedMovementState.m_fProceduralPitch=ax;
	obPredictedMovementState.m_fProceduralYaw=ay;
	obPredictedMovementState.m_fProceduralRoll=az;
	obPredictedMovementState.m_bApplyExplicitRotations = true;
	
	obPredictedMovementState.m_obRootDeltaScalar = CDirection( 0.0f, 0.0f, 0.0f);
	obPredictedMovementState.m_fRootRotationDeltaScalar=0.0f;

	return true;
}





//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimControllerDef::CreateInstance
//!	Construction
//!
//------------------------------------------------------------------------------------------
MovementController* InteractiveAnimControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) InteractiveAnimController( pobMovement, *this );
}



//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimController::InteractiveAnimController
//!	Construction
//!
//------------------------------------------------------------------------------------------
InteractiveAnimController::InteractiveAnimController( CMovement* pobMovement, const InteractiveAnimControllerDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_fAnimSpeed( 0.0f ),
	m_fAccelerationTime( 0.0f )

{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obSingleAnimation->SetSpeed( 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimController::~InteractiveAnimController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
InteractiveAnimController::~InteractiveAnimController (void)
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}

//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimController::Update
//!	Destruction
//!
//------------------------------------------------------------------------------------------
bool InteractiveAnimController::Update(	float						fTimeStep, 
										const CMovementInput&		/*obMovementInput*/,
										const CMovementStateRef&	/*obCurrentMovementState*/,
										CMovementState&				/*obPredictedMovementState*/ )
{
	// Add the animation on the first frame if we have one
	if ( m_bFirstFrame )
	{
		if ( !m_obDefinition.m_obAnimationName.IsNull() )
			m_pobAnimator->AddAnimation( m_obSingleAnimation );

		m_bFirstFrame = false;
	}
	
	if ( m_obSingleAnimation->GetTime() < ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
	{
		if (m_obDefinition.m_pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ) )
		{
			m_fAccelerationTime=0.5f;
		}

		if (m_fAccelerationTime>0.0f)
		{
			m_fAccelerationTime-=fTimeStep;

			m_fAnimSpeed+=m_obDefinition.m_fAnimSpeedAcceleration * fTimeStep;

			if (m_fAnimSpeed>m_obDefinition.m_fAnimMaxSpeed)
				m_fAnimSpeed=m_obDefinition.m_fAnimMaxSpeed;
		}
		else
		{
			m_fAnimSpeed-=m_obDefinition.m_fAnimSpeedDeacceleration * fTimeStep;

			if (m_fAnimSpeed<0.0f)
				m_fAnimSpeed=0.0f;
		}

		m_obSingleAnimation->SetSpeed(m_fAnimSpeed);

		m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );
	}
	else // Anim is completed
	{
		m_obSingleAnimation->SetSpeed(0.0f);

		m_obSingleAnimation->SetBlendWeight( 0.0f );

		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

MovementController* LadderControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) LadderController( pobMovement, *this );
}


LadderController::LadderController( CMovement* pobMovement, const LadderControllerDef& obDefinition ) :
	MovementController( pobMovement ),
	m_obDefinition ( obDefinition ),
	m_eLadderState(MOUNT)
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	float fDismountOffset = 1.0f;

	if ( m_pobMovement->GetParentEntity()->IsPlayer() )
	{
		if ( m_pobMovement->GetParentEntity()->ToPlayer()->IsArcher() )
		{
			// Evil badness.
			fDismountOffset = 2.66f;
		}
	}

	// Magic numbers store in shared params later?
	// Offsets in +ve Y axis
	const float fTopDismountOffset = -0.5f;			// The height at which the character dismounts (ie moves away from climb loop) the top of the ladder
	const float fTopMountOffset = -1.0f;			// The height at which the character mounts (ie moves to climb loop) the top of the ladder
	const float fBottomDismountOffset = fDismountOffset;		// The height at which the character dismounts (ie moves away from climb loop) the bottom of the ladder
	const float fBottomMountOffset = 1.0f;			// The height at which the character mounts (ie moves to climb loop) the bottom of the ladder
	const float fHeightOffsetTolerance = 0.25f;		// Distance offset for the detection of whether we are getting on/off the ladder


	m_pobLadder = obDefinition.m_pobLadderEntity;

	// Ladder position
	m_obLadderWorldMatrix=m_pobLadder->GetHierarchy()->GetRootTransform()->GetWorldMatrix();

	// Height between floors (Use points must be level with floor)
	float fLadderHeightBetweenUsePoints = obDefinition.m_pobLadderEntity->GetInteractionComponent()->GetHeightFromUsePoints();

	if ( fLadderHeightBetweenUsePoints <= 0.0f )
	{
		ntPrintf("Ladder %s has a height of 0.0. Check it has use points\n", obDefinition.m_pobLadderEntity->GetName().c_str());
	}


	// User position
	CPoint obPosition(pobMovement->GetParentEntity()->GetHierarchy()->GetRootTransform()->GetLocalTranslation());

	// Character is using the ladder from the bottom
	if (obPosition.Y() < (m_obLadderWorldMatrix.GetTranslation().Y() + fHeightOffsetTolerance + fBottomMountOffset))
	{
		// Dist + Top - Bottom (offsets in +ve Y)
		m_fMoveHeight = fLadderHeightBetweenUsePoints + fTopDismountOffset - fBottomMountOffset;

		// Dist + Top (offsets in +ve Y)
		m_fDismountHeight = fLadderHeightBetweenUsePoints  + fTopDismountOffset;

		m_obMountAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimMountBottom );
		m_obClimbAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimClimbUp );
		m_obDismountAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimDismountTop );

		m_obMountPosition=obDefinition.m_pobLadderParameters->m_obMountBottomPosition;
		m_obMountRotation=obDefinition.m_pobLadderParameters->m_obMountBottomRotation;

		m_bClimbingUp=true;

		// Prevent the character from colliding with static geometry
		Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		pobCC->SetCharacterControllerSetFilterExceptionFlags( Physics::IGNORE_FIXED_GEOM | Physics::IGNORE_CCs );

		// Disable gravity
		pobCC->SetApplyCharacterControllerGravity(false);
	}
	// Character is using the ladder from the top
	else if (obPosition.Y() > (m_obLadderWorldMatrix.GetTranslation().Y() + fLadderHeightBetweenUsePoints - fHeightOffsetTolerance + fTopMountOffset))
	{
		// Dist + Top - Bottom (offsets in +ve Y)
		m_fMoveHeight = fLadderHeightBetweenUsePoints + fTopMountOffset - fBottomDismountOffset;

		// Dist - Bottom (offsets in +ve Y)
		m_fDismountHeight = fBottomDismountOffset;

		m_obMountAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimMountTop );
		m_obClimbAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimClimbDown );
		m_obDismountAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_pobLadderParameters->m_obAnimDismountBottom );

		m_obMountPosition=obDefinition.m_pobLadderParameters->m_obMountTopPosition;
		m_obMountRotation=obDefinition.m_pobLadderParameters->m_obMountTopRotation;

		m_obMountPosition.Y()=fLadderHeightBetweenUsePoints;

		m_bClimbingUp=false;

		// Prevent the character from colliding with static geometry
		Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		pobCC->SetCharacterControllerSetFilterExceptionFlags( Physics::IGNORE_FIXED_GEOM | Physics::IGNORE_CCs );

		// Disable gravity
		pobCC->SetApplyCharacterControllerGravity(false);
	}
	else
	{
		m_eLadderState=FINISHED;
	}
}

LadderController::~LadderController( void )
{
	if (m_eLadderState!=FINISHED)
	{
		// Re-enable collision with static geometry
		Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		pobCC->SetCharacterControllerSetFilterExceptionFlags( 0 );
	}

	if (!m_bFirstFrame)
	{
		m_pobAnimator->RemoveAnimation( m_obMountAnimation );
		m_pobAnimator->RemoveAnimation( m_obClimbAnimation );
		m_pobAnimator->RemoveAnimation( m_obDismountAnimation );
	}
}

bool LadderController::Update( float /*fTimeStep*/, const CMovementInput& /*obMovementInput*/, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	//m_pobLadder->GetInteractionComponent()->DebugRenderUsePoints();

	switch(m_eLadderState)
	{
		case MOUNT:
		{
			//ntPrintf("LadderController: Mounting\n");

			if (m_bFirstFrame)
			{

				m_obMountAnimation->SetBlendWeight( 0.0f );
				m_obMountAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
				m_obMountAnimation->SetSpeed(1.0f);
				
				m_obClimbAnimation->SetBlendWeight( 0.0f );
				m_obClimbAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
				m_obClimbAnimation->SetSpeed(0.0f);

				m_obDismountAnimation->SetBlendWeight( 0.0f );
				m_obDismountAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
				m_obDismountAnimation->SetSpeed(0.0f);

				m_pobAnimator->AddAnimation( m_obMountAnimation );
				m_pobAnimator->AddAnimation( m_obClimbAnimation );
				m_pobAnimator->AddAnimation( m_obDismountAnimation );

				m_eLadderState=CLIMB;

				m_obMountAnimation->SetSpeed(0.0f);
				m_obClimbAnimation->SetSpeed(1.0f);

				m_bFirstFrame=false;
			}

			if (m_obMountAnimation->GetTime()>=m_obMountAnimation->GetDuration()) // We are ready to move to the climb stage
			{
				m_eLadderState=CLIMB;

				m_obMountAnimation->SetSpeed(0.0f);
				m_obClimbAnimation->SetSpeed(1.0f);
			}
			else
			{
				m_obMountAnimation->SetBlendWeight( m_fBlendWeight );
			}

			break;
		}

		case CLIMB: 
		{
			//ntPrintf("LadderController: Climbinh\n");

			if (m_bClimbingUp)
			{
				// We have finished the climb up
				if (obCurrentMovementState.m_obPosition.Y() >= (m_obLadderWorldMatrix.GetTranslation().Y()+m_fDismountHeight)) 
				{
					//ntPrintf ("Climb up miss %f\n", obCurrentMovementState.m_obPosition.Y() - (m_obLadderWorldMatrix.GetTranslation().Y()+m_fDismountHeight) );

					// Scalar to adjust dismount anim 
					m_fDismountScalar = ( m_obDismountAnimation->GetRootEndTranslation().Y() + ( m_obLadderWorldMatrix.GetTranslation().Y() + m_fDismountHeight - obCurrentMovementState.m_obPosition.Y() ) ) /  m_obDismountAnimation->GetRootEndTranslation().Y();
					//ntPrintf ("Climb up dismount scalar %f\n", m_fDismountScalar );

					m_eLadderState=DISMOUNT;

					m_obClimbAnimation->SetSpeed(0.0f);
					m_obDismountAnimation->SetSpeed(1.0f);
				}
			}
			else
			{
				// Do line of sight check if the character is going down the ladder

				Physics::TRACE_LINE_QUERY stQuery;

				Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
				// [Mus] - What settings for this cast ?
				obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
				obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
												Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
												Physics::RAGDOLL_BIT						|
												Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT				);

				
				CPoint obStart(obCurrentMovementState.m_obPosition);
				CDirection obOffset(obCurrentMovementState.m_obFacing);
				obOffset *= 0.5f;
				obStart+=obOffset;
				
				CPoint obEnd(obStart);
				obEnd.Y()-=m_fMoveHeight;

				//g_VisualDebug->RenderLine(obStart,obEnd,0xffffffff);

				if (Physics::CPhysicsWorld::Get().TraceLine( obStart, obEnd, m_pobMovement->GetParentEntity(), stQuery, obFlag ) )
				{
					if (stQuery.pobEntity->IsPlayer() || stQuery.pobEntity->IsEnemy())
					{
						// Knock off anyone who might be trying to climb the ladder

						//ntPrintf("entity combat state=%d\n",stQuery.pobEntity->GetAttackComponent()->AI_Access_GetState());

						if (stQuery.pobEntity->GetAttackComponent()->AI_Access_GetState() == CS_STANDARD)
						{
							CAttackData* pobAttackData=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_throw_body");

							CStrike* pobStrike = NT_NEW CStrike(m_pobMovement->GetParentEntity(),
																stQuery.pobEntity,
																pobAttackData,
																1.0f, 1.0f,
																false, false, false, false, false, false, 0,
																stQuery.obIntersect );

							SyncdCombat::Get().PostStrike( pobStrike );

							//ntPrintf("entity combat state=%d\n",stQuery.pobEntity->GetAttackComponent()->AI_Access_GetState());
						}
					}
					else
					{
						//ntPrintf ("Climb down miss %f\n", (m_obLadderWorldMatrix.GetTranslation().Y()+m_fDismountHeight) - obCurrentMovementState.m_obPosition.Y());
					
						// Scalar to adjust dismount anim 
						m_fDismountScalar = ( m_obDismountAnimation->GetRootEndTranslation().Y() + ( m_obLadderWorldMatrix.GetTranslation().Y() + m_fDismountHeight - obCurrentMovementState.m_obPosition.Y() ) ) /  m_obDismountAnimation->GetRootEndTranslation().Y();
						//ntPrintf ("Climb down dismount scalar %f\n", m_fDismountScalar );

						m_eLadderState=DISMOUNT;

						m_obClimbAnimation->SetSpeed(0.0f);
						m_obDismountAnimation->SetSpeed(1.0f);
					}
				}
			

				// We have finished the climb down
				if (obCurrentMovementState.m_obPosition.Y() <= (m_obLadderWorldMatrix.GetTranslation().Y()+m_fDismountHeight)) 
				{
					//ntPrintf ("Climb down miss %f\n", (m_obLadderWorldMatrix.GetTranslation().Y()+m_fDismountHeight) - obCurrentMovementState.m_obPosition.Y());
				
					// Scalar to adjust dismount anim 
					m_fDismountScalar = ( m_obDismountAnimation->GetRootEndTranslation().Y() + ( m_obLadderWorldMatrix.GetTranslation().Y() + m_fDismountHeight - obCurrentMovementState.m_obPosition.Y() ) ) /  m_obDismountAnimation->GetRootEndTranslation().Y();
					//ntPrintf ("Climb down dismount scalar %f\n", m_fDismountScalar );

					m_eLadderState=DISMOUNT;

					m_obClimbAnimation->SetSpeed(0.0f);
					m_obDismountAnimation->SetSpeed(1.0f);
				}
			}

			m_obClimbAnimation->SetBlendWeight( m_fBlendWeight );

			// HOT-FIX by peterFe. WalkRunController is switching gravity on in every update (no idea why).
			// That's why if WalkEunController movement is blended with this movement the gravity is enabled. 
			// Ladder climbing needs disabled gravity. 
			// So like a hot fix disable gravity in every frame. Later probably blend betweeb gravity values.

			ApplyGravity(false);

			// Prevent the character from colliding with static geometry
			Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
			pobCC->SetCharacterControllerSetFilterExceptionFlags( Physics::IGNORE_FIXED_GEOM | Physics::IGNORE_CCs );


			break;
		}

		case DISMOUNT:
		{
			//ntPrintf("LadderController: Dismounting\n");

			if (m_obDismountAnimation->GetTime()>=m_obDismountAnimation->GetDuration()) // We are ready to move to the climb stage
			{
				m_obDismountAnimation->SetSpeed(0.0f);

				m_eLadderState=FINISHED;

				// Re-enable collision with static geometry
				Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
				pobCC->SetCharacterControllerSetFilterExceptionFlags( 0 );
				
				// Re-enable gravity
				pobCC->SetApplyCharacterControllerGravity(true);

				return true;
			}
			else
			{
				// Scale the dismount anim
				obPredictedMovementState.m_obRootDeltaScalar.Y() = m_fDismountScalar;
				m_obDismountAnimation->SetBlendWeight( m_fBlendWeight );
			}

			break;
		}

		case FINISHED:
		{
			// Apply gravity if required
			ApplyGravity(true);

			return true;
			
			break;
		}
	}

	return false;
}


/* // Crank and Crank door controllers replaced by the buttonmash controllers
MovementController* CrankOperatorControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CrankOperatorController( pobMovement, *this );
}


CrankOperatorController::CrankOperatorController( CMovement* pobMovement, const CrankOperatorControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Register our definition for debugging purposes
	//InternalRegisterDefinition( obDefinition );

	m_pobParameters = ObjectDatabase::Get().GetPointerFromName<CrankSharedParameters*>(obDefinition.m_obCrankParameters);

	ntAssert_p(m_pobParameters,("Serialised data %s does not exist!", ntStr::GetString(obDefinition.m_obCrankParameters)));

	if (m_pobParameters)
	{
		m_pobParameters->m_pobCrankOperatorEntity=(CEntity*)m_pobMovement->GetParentEntity();

		if (m_pobParameters->m_fAnimSpeed<0.0f) // Reset the anim if the door is currently closing
			m_pobParameters->m_fAnimSpeed=0.0f;

		m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_pobParameters->m_obAnimCharacter );
		m_obSingleAnimation->SetBlendWeight( 0.0f );
		m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
		m_obSingleAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); // Sync player anim speed to the crank anim speed
	}
}

CrankOperatorController::~CrankOperatorController( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
	}
}

bool CrankOperatorController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	UNUSED (fTimeStep);
	UNUSED (obMovementInput);

	if (!m_pobParameters) // Oh dear, we don't have valid parameters
		return true;


	if ( m_bFirstFrame )
	{
		// Start the animations
		m_pobAnimator->AddAnimation( m_obSingleAnimation );

		m_obSingleAnimation->SetTime(m_pobParameters->m_fAnimTime); // Sync player anim to the crank anim
		m_bFirstFrame = false;

		// ----- Snap the character to the crank entity -----

		Transform* pTransform = m_pobParameters->m_pobCrankEntity->GetHierarchy()->GetRootTransform();
		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta

		CDirection obTranslationOffset=CDirection(m_pobParameters->m_obCharacterTranslationOffset) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);

		// Calculate the rotation delta
		
		CQuat obRotation=CCamUtil::QuatFromEuler_XYZ(
			m_pobParameters->m_obCharacterRotationOffset.X() * DEG_TO_RAD_VALUE,
			m_pobParameters->m_obCharacterRotationOffset.Y() * DEG_TO_RAD_VALUE,
			m_pobParameters->m_obCharacterRotationOffset.Z() * DEG_TO_RAD_VALUE);

		CQuat obTargetRotation(obTargetWorldMatrix);
		CQuat obNewRotation=obTargetRotation * obRotation;
		CMatrix obNewMatrix(obNewRotation);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

		obPredictedMovementState.m_fProceduralYaw=ay;//MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obNewMatrix.GetZAxis());
		obPredictedMovementState.m_bApplyExplicitRotations = true;
	}

	// The player's anim speed is determined by the crank, but the player determines whether the crank is speeding up or slowing down

	CEntity* pobControllingEntity=(CEntity*)m_pobMovement->GetParentEntity();
	
	if (pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ))
	{
		m_pobParameters->m_fOpenTime=1.0f;
	}

	// Synchronise the anim to the shared parameters

	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );
	m_obSingleAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); // Sync player anim speed to the crank anim speed
	//m_obSingleAnimation->SetTime(m_pobParameters->m_fAnimTime); // Sync player anim to the crank anim

	return false;
}

MovementController* CrankLeverControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CrankLeverController( pobMovement, *this );
}

CrankLeverController::CrankLeverController( CMovement* pobMovement, const CrankLeverControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( obDefinition );

	m_pobParameters = ObjectDatabase::Get().GetPointerFromName<CrankSharedParameters*>(obDefinition.m_obCrankParameters);

	ntAssert_p(m_pobParameters,("Serialised data %s does not exist!", ntStr::GetString(obDefinition.m_obCrankParameters)));

	if (m_pobParameters)
	{
		m_pobParameters->m_bDoorLocked=m_pobParameters->m_bDoorLockedOnStart;
		m_pobParameters->m_pobCrankEntity=(CEntity*)m_pobMovement->GetParentEntity();

		m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_pobParameters->m_obAnimCrank );
		m_obSingleAnimation->SetBlendWeight( 1.0f );
		m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
		m_obSingleAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); // Sync to shared parameters
		m_obSingleAnimation->SetTime(m_pobParameters->m_fAnimTime); // Sync to shared parameters
	}
}

CrankLeverController::~CrankLeverController( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
	}
}

bool CrankLeverController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	UNUSED (obMovementInput);
	UNUSED (obCurrentMovementState);
	UNUSED (obPredictedMovementState);

	if (!m_pobParameters) // Oh dear, we don't have valid parameters
		return true;

	if ( m_bFirstFrame )
	{
		// Start the animations
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;
	}

	if (m_pobParameters->m_bDoorLocked)
	{
		m_pobParameters->m_fOpenTime=0.0f;
		m_obSingleAnimation->SetSpeed(0.0f);
		m_obSingleAnimation->SetBlendWeight(m_fBlendWeight);		
		return false;
	}

	// Update the anim speed - this is controlled by the crank

	if (m_pobParameters->m_fOpenTime>=0.0f) // Door is opening
	{
		m_pobParameters->m_fOpenTime-=fTimeStep;

		if (m_pobParameters->m_fOpenTime<0.0f)
		{
			CMessageSender::SendEmptyMessage( "msg_crank_release", m_pobMovement->GetParentEntity()->GetMessageHandler() );
		}

		m_pobParameters->m_fAnimSpeed+=m_pobParameters->m_fAnimAcceleration * fTimeStep;

		if (m_pobParameters->m_fAnimSpeed>1.0f)
			m_pobParameters->m_fAnimSpeed=1.0f;

		m_pobParameters->m_obDoorPosition.Y() += m_pobParameters->m_fAnimSpeed * m_pobParameters->m_fDoorOpenSpeed * fTimeStep;

		if (m_pobParameters->m_obDoorPosition.Y()>=m_pobParameters->m_fDoorMaxY) // Door has reached its limit
		{
			m_pobParameters->m_obDoorPosition.Y()=m_pobParameters->m_fDoorMaxY;
			m_pobParameters->m_fAnimSpeed=0.0f;
		}
	}
	else // Door is closing
	{
		if (m_pobParameters->m_obDoorPosition.Y()>m_pobParameters->m_fDoorMinY)
		{
			m_pobParameters->m_fAnimSpeed-=m_pobParameters->m_fAnimDeacceleration * fTimeStep;

			if (m_pobParameters->m_fAnimSpeed<-1.0f)
				m_pobParameters->m_fAnimSpeed=-1.0f;

			m_pobParameters->m_obDoorPosition.Y() += m_pobParameters->m_fAnimSpeed * m_pobParameters->m_fDoorCloseSpeed * fTimeStep;

			if (m_pobParameters->m_obDoorPosition.Y()<=m_pobParameters->m_fDoorMinY)
			{
				m_pobParameters->m_obDoorPosition.Y()=m_pobParameters->m_fDoorMinY;
				m_pobParameters->m_fAnimSpeed=0.0f;
			}
		}
	}

	// Sync player anim speed to the crank anim speed

	m_obSingleAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); 
	m_obSingleAnimation->SetBlendWeight(m_fBlendWeight);
	m_pobParameters->m_fAnimTime=m_obSingleAnimation->GetTime();

	return false;
}

MovementController* CrankDoorControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CrankDoorController( pobMovement, *this );
}

CrankDoorController::CrankDoorController( CMovement* pobMovement, const CrankDoorControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( obDefinition );

	m_pobParameters = ObjectDatabase::Get().GetPointerFromName<CrankSharedParameters*>(obDefinition.m_obCrankParameters);

	ntAssert_p(m_pobParameters,("Serialised data %s does not exist!", ntStr::GetString(obDefinition.m_obCrankParameters)));

	if (m_pobParameters)
	{
		m_pobParameters->m_pobDoorEntity=(CEntity*)m_pobMovement->GetParentEntity();
	}
}

bool CrankDoorController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	UNUSED (fTimeStep);
	UNUSED (obMovementInput);

	if (!m_pobParameters) // Oh dear, we don't have valid parameters
		return true;

	if (m_bFirstFrame)
	{
		m_pobParameters->m_obDoorPosition=obCurrentMovementState.m_obPosition; // Here we assume that the door is already in a closed position
		m_pobParameters->m_fDoorMinY=m_pobParameters->m_obDoorPosition.Y();
		m_pobParameters->m_fDoorMaxY=m_pobParameters->m_fDoorMinY+m_pobParameters->m_fDoorOpenHeight;

		if (m_pobParameters->m_bDoorOpenOnStart)
			m_pobParameters->m_obDoorPosition.Y()=m_pobParameters->m_fDoorMaxY;

		m_bFirstFrame=false;
	}


	

	CDirection obRootDelta(m_pobParameters->m_obDoorPosition - obCurrentMovementState.m_obPosition);
	obPredictedMovementState.m_obProceduralRootDelta=obRootDelta;

	return false;
}*/

//------------------------------------------------------------------------------------------
//!
//!	ButtonMashController::ButtonMashController
//!	Construction
//!
//------------------------------------------------------------------------------------------
ButtonMashController::ButtonMashController(CMovement* pobMovement, const ButtonMashControllerDef &obDefinition) :
	MovementController( pobMovement )
{
	// Save shared params pointer
	m_pSharedParams = obDefinition.m_pSharedParams;
	ntAssert(m_pSharedParams);

	// Set defaults
	m_pSharedParams->m_fRequiredMPS			= m_pSharedParams->m_fRequiredMPS;

	m_pSharedParams->m_fCurrentInterpolant	= 0.0f;
	m_pSharedParams->m_fCurrentMPS			= 0.0f;
	m_pSharedParams->m_fMashPressTime		= 0.0f;

	m_pSharedParams->m_fAnimSpeed			= 0.0f;
	m_pSharedParams->m_fAnimTime			= 0.0f;

	// Register our definition for debugging purposes
	InternalRegisterDefinition( obDefinition );

	m_obButtonMashAnim = m_pobAnimator->CreateAnimation( m_pSharedParams->m_obObjectButtonMashAnimName );
	m_obButtonMashAnim->SetBlendWeight( 1.0f );
	m_obButtonMashAnim->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
	m_obButtonMashAnim->SetSpeed( m_pSharedParams->m_fAnimSpeed );	// Sync to shared parameters
	m_obButtonMashAnim->SetTime( m_pSharedParams->m_fAnimTime );	// Sync to shared parameters

	m_pSharedParams->m_fAnimDurationObject	= m_obButtonMashAnim->GetDuration( );

	m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::INACTIVE;
}


//------------------------------------------------------------------------------------------
//!
//!	CounterWeightLeverControllerDef::CreateInstance
//!	Creates an instance of the controller
//!
//------------------------------------------------------------------------------------------
MovementController* ButtonMashControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ButtonMashController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashController::~ButtonMashController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ButtonMashController::~ButtonMashController( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obButtonMashAnim );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashController::Update
//!	Update
//!
//------------------------------------------------------------------------------------------
bool ButtonMashController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef&	obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	if ( m_bFirstFrame )
	{
		// Start the animations
		m_pobAnimator->AddAnimation( m_obButtonMashAnim );
		m_bFirstFrame = false;

		m_pSharedParams->m_iButtonMashCount = 0;
		// Sanity checks

		user_warn_p( m_pSharedParams->m_fAnimDurationUser == m_pSharedParams->m_fAnimDurationObject,
			( "ButtonMash: Operator and user button mash animations do not have equal duration\n") );
	}

	switch (m_pSharedParams->m_eButtonMashState)
	{
		case ButtonMashSharedParams::INACTIVE:
		{
			m_pSharedParams->m_iButtonMashCount = 0;
			break;
		}

		case ButtonMashSharedParams::BUTTONMASH:
		{

			// Increase button press timer
			// Done here as player may opt to leave the object and it still needs to fall
			m_pSharedParams->m_fMashPressTime += fTimeStep; 

			// Once we've exceeded the min rate update the rate so that the object correctly falls with no player
			if (m_pSharedParams->m_fMashPressTime > m_pSharedParams->m_fRequiredMPS)
				m_pSharedParams->m_fCurrentMPS = m_pSharedParams->m_fMashPressTime;
	
			// Update the anim speed
			// Rate faster than required rate
			if ( m_pSharedParams->m_fCurrentMPS <= m_pSharedParams->m_fRequiredMPS ) // Rising
			{
				// How much faster button rate is
				float fRateDelta = m_pSharedParams->m_fRequiredMPS - m_pSharedParams->m_fCurrentMPS;
				fRateDelta = (m_pSharedParams->m_fMPSPveRateVariation > 0.0f) ? fRateDelta/m_pSharedParams->m_fMPSPveRateVariation : 0.0f;
				
				// Set speed relative to excess button rate
				m_pSharedParams->m_fAnimSpeed = fRateDelta;

				// Check Bounds
				if (m_pSharedParams->m_fAnimSpeed > 1.0f)
					m_pSharedParams->m_fAnimSpeed = 1.0f;

				// Check here for end of button mashing anim
				if ( m_obButtonMashAnim->GetTime() >= m_obButtonMashAnim->GetDuration() )
				{
					if ( ++m_pSharedParams->m_iButtonMashCount >= m_pSharedParams->m_iButtonMashRepeats )
					{
						// All done.
					m_pSharedParams->m_fAnimSpeed = 0.0f;
					m_pSharedParams->m_fAnimTime = m_obButtonMashAnim->GetDuration();

					// Set animation to the end and stop it.
					m_obButtonMashAnim->SetSpeed(0.0f);
					m_obButtonMashAnim->SetTime(m_obButtonMashAnim->GetDuration());

					m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::SUCCESS;

					// We could send a message here to the parent entity to let it know we've finished
					// but that entity can poll this using its per-frame update function so let it do that.
				}
					else
					{
						//Have another go
						m_pSharedParams->m_fAnimTime = 0.0f;
						m_obButtonMashAnim->SetTime( 0.0f );
					}
				}
			}
			else // Lever falling back on player
			{
				if ( m_pSharedParams->m_fMPSNveRateVariation == 0.0f  && m_pSharedParams->m_bExitOnFail )
				{
					m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::FAILED;
					break;
				}

				// How much slower button rate is
				float fRateDelta = m_pSharedParams->m_fRequiredMPS - m_pSharedParams->m_fCurrentMPS;
				fRateDelta = (m_pSharedParams->m_fMPSNveRateVariation > 0.0f) ? fRateDelta/m_pSharedParams->m_fMPSNveRateVariation : 0.0f;
				
				// Set speed relative to excess button rate
				m_pSharedParams->m_fAnimSpeed = fRateDelta;

				// Check Bounds
				if (m_pSharedParams->m_fAnimSpeed < -1.0f)
					m_pSharedParams->m_fAnimSpeed = -1.0f;

				// Check here for start of anim
				if ( m_obButtonMashAnim->GetTime() <= 0.0f )
				{
					if ( --m_pSharedParams->m_iButtonMashCount < 0 )
					{
						// Right back to begining
					m_pSharedParams->m_fAnimSpeed = 0.0f;
						if ( m_pSharedParams->m_bExitOnFail )
							m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::FAILED;
						else
					m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::INACTIVE;
				}
					else
					{
						// Still more to go
						m_pSharedParams->m_fAnimTime = m_obButtonMashAnim->GetDuration();
						m_obButtonMashAnim->SetTime(m_obButtonMashAnim->GetDuration());
					}
				}
			}

			// Sync player anim speed to the lever anim speed
			m_obButtonMashAnim->SetSpeed(m_pSharedParams->m_fAnimSpeed); 
			m_obButtonMashAnim->SetBlendWeight(m_fBlendWeight);
			m_pSharedParams->m_fAnimTime = m_obButtonMashAnim->GetTime();

			// Figure out the currently required input rate
			m_pSharedParams->m_fRequiredMPS = m_pSharedParams->m_fSecondsPerMashStart - ( m_obButtonMashAnim->GetTime() / m_obButtonMashAnim->GetDuration() ) * (m_pSharedParams->m_fSecondsPerMashStart - m_pSharedParams->m_fSecondsPerMashEnd);
			
			break;
		}

		case ButtonMashSharedParams::SUCCESS:
		{
			return true;
			break;
		}

		case ButtonMashSharedParams::FAILED:
		{
			return true;
			break;
		}

	}  // switch (m_pobParameters->m_eLeverState)

	// Update current interpolant
	m_pSharedParams->m_fCurrentInterpolant = m_pSharedParams->m_fAnimTime / m_obButtonMashAnim->GetDuration();
	m_pSharedParams->m_fCurrentInterpolant += m_pSharedParams->m_iButtonMashCount;
	m_pSharedParams->m_fCurrentInterpolant /= m_pSharedParams->m_iButtonMashRepeats;

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashOperatorControllerDef::CreateInstance
//!	Creates an instance of the controller
//!
//------------------------------------------------------------------------------------------
MovementController* ButtonMashOperatorControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ButtonMashOperatorController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashOperatorController::ButtonMashOperatorController
//!	Construction
//!
//------------------------------------------------------------------------------------------
ButtonMashOperatorController::ButtonMashOperatorController( CMovement* pobMovement, const ButtonMashOperatorControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Save shared params pointer
	m_pSharedParams = obDefinition.m_pSharedParams;
	ntAssert(m_pSharedParams);

	m_bFirstFrame		= true;

	//InternalRegisterDefinition( obDefinition );

	// Copy in data from definition

	// Reset animation if it is reversing
	if ( m_pSharedParams->m_fAnimSpeed < 0.0f )
		m_pSharedParams->m_fAnimSpeed = 0.0f;

	// Create animation
	m_obButtonMashAnim = m_pobAnimator->CreateAnimation( m_pSharedParams->m_obOperatorButtonMashAnimName );
	m_obButtonMashAnim->SetBlendWeight( 0.0f );
	m_obButtonMashAnim->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
	m_obButtonMashAnim->SetSpeed( m_pSharedParams->m_fAnimSpeed ); // Sync operator to object

	m_pSharedParams->m_fAnimDurationUser	= m_obButtonMashAnim->GetDuration( );

}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashOperatorController::~ButtonMashOperatorController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ButtonMashOperatorController::~ButtonMashOperatorController()
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obButtonMashAnim );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonMashOperatorController::Update
//!	Update
//!
//------------------------------------------------------------------------------------------
bool ButtonMashOperatorController::Update(float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState,CMovementState& obPredictedMovementState )
{
	if (!m_pSharedParams) // Oh dear, we don't know have any shared params
		return true;


	if ( m_bFirstFrame )
	{
		m_bFirstFrame = false;

		// Start the animations
		m_pobAnimator->AddAnimation( m_obButtonMashAnim );

		m_obButtonMashAnim->SetTime( m_pSharedParams->m_fAnimTime ); // Sync player anim to object anim
		
		m_pSharedParams->m_fMashPressTime = 0.0f; // Reset button press timer
		m_pSharedParams->m_fCurrentMPS = 0.0f; // First frame so make sure some movement can be done
		m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::BUTTONMASH; // Activate the button mashing

		// ----- Snap the character to the object entity -----

		/*Transform* pTransform = m_pSharedParams->m_pObjectEntity->GetHierarchy()->GetRootTransform();
		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta

		CDirection obTranslationOffset = CDirection( m_pSharedParams->m_obCharacterTranslationOffset ) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);

		// Calculate the rotation delta
		CPoint obCharacterRotationOffset = m_pSharedParams->m_obCharacterRotationOffset;
		CQuat obRotation=CCamUtil::QuatFromEuler_XYZ(
			obCharacterRotationOffset.X() * DEG_TO_RAD_VALUE,
			obCharacterRotationOffset.Y() * DEG_TO_RAD_VALUE,
			obCharacterRotationOffset.Z() * DEG_TO_RAD_VALUE);
		
		CQuat obTargetRotation(obTargetWorldMatrix);
		CQuat obNewRotation = obTargetRotation * obRotation;
		CMatrix obNewMatrix(obNewRotation);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

		obPredictedMovementState.m_fProceduralYaw=ay;//MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obNewMatrix.GetZAxis());
		obPredictedMovementState.m_bApplyExplicitRotations = true;*/
	}

	// The player's anim speed is determined by the lever, but the player determines whether the lever is speeding up or slowing down
	CEntity* pobControllingEntity = (CEntity*)m_pobMovement->GetParentEntity();

	// Synchronise the anim to the shared parameters
	switch (m_pSharedParams->m_eButtonMashState)
	{
		case ButtonMashSharedParams::INACTIVE:
		{
			// We've gone all the way to the begining - wait for a new button press
			if (pobControllingEntity->IsAI() || pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ))
			{
				m_pSharedParams->m_fCurrentMPS = 0.0f;  // Make sure we do a decent first lift
				m_pSharedParams->m_fMashPressTime = 0.0f;	// Reset button press timer
				m_pSharedParams->m_eButtonMashState = ButtonMashSharedParams::BUTTONMASH;
			}
			
			break;
		}

		case ButtonMashSharedParams::BUTTONMASH:
		{

			// Lift state button press scheme
			if (pobControllingEntity->IsAI() || pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ))
			{
				m_pSharedParams->m_fCurrentMPS = ( m_pSharedParams->m_fMashPressTime ); // Store the accumulated time since button press
				m_pSharedParams->m_fMashPressTime = 0.0f;	// Reset button press timer
			}

			m_obButtonMashAnim->SetSpeed( m_pSharedParams->m_fAnimSpeed ); // Sync player anim speed to the object anim speed
			m_obButtonMashAnim->SetBlendWeight( m_fBlendWeight );
			m_obButtonMashAnim->SetTime( m_pSharedParams->m_fAnimTime ); // Sync player anim to the object anim

			break;
		}

		case ButtonMashSharedParams::SUCCESS:
		{
			/*
			// Recover section of anim is not synced to lever
			m_obButtonMashAnim->SetSpeed( m_fRecoverAnimSpeed ); 
			m_obButtonMashAnim->SetBlendWeight( m_fBlendWeight );
			
			// Has operator finished movement? - remove controller
			if ( m_obButtonMashAnim->GetTime() >= m_obButtonMashAnim->GetDuration() )
			{
				return true;
			}
			*/

			return true;

			// g_VisualDebug->Printf2D(10,10,DC_GREEN,0,"Operator LIFT m_fButtonPressTime %.2f, m_fButtonRate %.2f, m_fRequiredRate %.2f", m_pobParameters->m_fButtonPressTime, m_pobParameters->m_fButtonRate, m_pobParameters->m_fRequiredButtonRate);
			break;
		}

		case ButtonMashSharedParams::FAILED:
		{
			return true;

			break;
		}
	} // switch (m_pObjectController->GetState())

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransitionDef::CoordinatedTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoordinatedTransitionDef::CoordinatedTransitionDef( void )
:	m_bApplyGravity( true ),
	m_obAnimationName(),
	m_pobCoordinationParams(0)
{
}


//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* CoordinatedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW CoordinatedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransition::CoordinatedTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoordinatedTransition::CoordinatedTransition( CMovement* pobMovement, const CoordinatedTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	int iFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT;

	/*if ( m_obDefinition.m_bLooping == true )
		iFlags |= ANIMF_LOOPING;*/

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlags( iFlags );
}


//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransition::~CoordinatedTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CoordinatedTransition::~CoordinatedTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool CoordinatedTransition::Update(	float					fTimeStep, 
								const CMovementInput&		/* obMovementInput */,
								const CMovementStateRef&	/*obCurrentMovementState */,
								CMovementState&				obPredictedMovementState )
{
	// If we are on the first frame - add the animation to the animator
	if ( m_bFirstFrame )
	{
		m_obPrevTrans = CPoint(CONSTRUCT_CLEAR);
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTime( m_obDefinition.m_pobCoordinationParams->m_fNormalisedTime * m_obSingleAnimation->GetDuration() );
		m_bFirstFrame = false;
	}
		
	// Use anim speed if we have one...
	m_obSingleAnimation->SetSpeed( m_obDefinition.m_pobCoordinationParams->m_fSpeed );

	// Or just sync with coordination params
	if ( m_obDefinition.m_pobCoordinationParams->m_fSpeed == 0.0f)
	{
		m_obSingleAnimation->SetTime( m_obDefinition.m_pobCoordinationParams->m_fNormalisedTime * m_obSingleAnimation->GetDuration() );

		// With zero speed we will get no root deltas, so do it manually.  
		// Better solution would be to sync by time - but that would mean rework of button mash which 
		// isn't called for at the moment. T McK
		CPoint obTrans = m_obSingleAnimation->GetRootTranslationAtTime( m_obDefinition.m_pobCoordinationParams->m_fNormalisedTime * m_obSingleAnimation->GetDuration(),
																		m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL );
		CDirection obDelta(obTrans - m_obPrevTrans);
		m_obPrevTrans = obTrans;

		obPredictedMovementState.m_obProceduralRootDelta=obDelta;	
	
	}
	else
	{
		m_obDefinition.m_pobCoordinationParams->m_fNormalisedTime = m_obSingleAnimation->GetTime() / m_obSingleAnimation->GetDuration();
	}
	
	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);


	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
	{
		//m_obDefinition.m_pobCoordinationParams->m_fNormalisedTime = 1.0f;
		return true;
	}

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}

/*	// CounterWeightControllers replaced by ButtonMashControllers

//------------------------------------------------------------------------------------------
//!
//!	CounterWeightLeverOperatorController::CounterWeightLeverOperatorController()
//!	
//!
//------------------------------------------------------------------------------------------#

MovementController* CounterWeightLeverOperatorControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CounterWeightLeverOperatorController( pobMovement, *this );
}


CounterWeightLeverOperatorController::CounterWeightLeverOperatorController( CMovement* pobMovement, const CounterWeightLeverOperatorControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Register our definition for debugging purposes
	//InternalRegisterDefinition( obDefinition );

	m_pobParameters = ObjectDatabase::Get().GetPointerFromName<CounterWeightLeverSharedParameters*>(obDefinition.m_obLeverParameters);

	ntAssert_p(m_pobParameters,("Serialised data %s does not exist!", ntStr::GetString(obDefinition.m_obLeverParameters)));

	if (m_pobParameters)
	{
		m_pobParameters->m_pobLeverOperatorEntity=(CEntity*)m_pobMovement->GetParentEntity();

		if (m_pobParameters->m_fAnimSpeed<0.0f) // Reset the anim if the door is currently closing
			m_pobParameters->m_fAnimSpeed=0.0f;

		m_obPushAnimation = m_pobAnimator->CreateAnimation( m_pobParameters->m_obAnimCharacterPush );
		m_obPushAnimation->SetBlendWeight( 0.0f );
		m_obPushAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
		m_obPushAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); // Sync player anim speed to the crank anim speed

		//ntPrintf("m_obPushAnimation Duration %f\n", m_obPushAnimation->GetDuration() );

		m_fRecoverAnimSpeed = 1.0f;
	}
}

CounterWeightLeverOperatorController::~CounterWeightLeverOperatorController( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obPushAnimation );
	}
}

bool CounterWeightLeverOperatorController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	UNUSED (fTimeStep);
	UNUSED (obMovementInput);
	if (!m_pobParameters) // Oh dear, we don't have valid parameters
		return true;


	if ( m_bFirstFrame )
	{
		// Start the animations
		m_pobAnimator->AddAnimation( m_obPushAnimation );

		m_obPushAnimation->SetTime(m_pobParameters->m_fAnimTime); // Sync player anim to the lever anim
		m_bFirstFrame = false;

		m_pobParameters->m_fRequiredButtonRate = 1.0f;					// Default button rate
		m_pobParameters->m_fButtonRate = m_pobParameters->m_fRequiredButtonRate;
		
		m_pobParameters->m_fButtonPressTime = 0.0f;										// Reset button press timer


		m_pobParameters->m_fButtonRate = 0.0f;										// First frame so make sure some movement can be done
		m_pobParameters->m_eLeverState = CounterWeightLeverSharedParameters::LIFT;	// Activate the lever


		// ----- Snap the character to the lever entity -----

		Transform* pTransform = m_pobParameters->m_pobLeverEntity->GetHierarchy()->GetRootTransform();
		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta

		CDirection obTranslationOffset=CDirection(m_pobParameters->m_obCharacterTranslationOffset) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		obPredictedMovementState.m_obProceduralRootDelta=CDirection(obNewPosition-obCurrentMovementState.m_obPosition);

		// Calculate the rotation delta
		
		CQuat obRotation=CCamUtil::QuatFromEuler_XYZ(
			m_pobParameters->m_obCharacterRotationOffset.X() * DEG_TO_RAD_VALUE,
			m_pobParameters->m_obCharacterRotationOffset.Y() * DEG_TO_RAD_VALUE,
			m_pobParameters->m_obCharacterRotationOffset.Z() * DEG_TO_RAD_VALUE);

		CQuat obTargetRotation(obTargetWorldMatrix);
		CQuat obNewRotation=obTargetRotation * obRotation;
		CMatrix obNewMatrix(obNewRotation);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

		obPredictedMovementState.m_fProceduralYaw=ay;//MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing,obNewMatrix.GetZAxis());
		obPredictedMovementState.m_bApplyExplicitRotations = true;
	}

	// The player's anim speed is determined by the lever, but the player determines whether the lever is speeding up or slowing down
	CEntity* pobControllingEntity=(CEntity*)m_pobMovement->GetParentEntity();

	// Synchronise the anim to the shared parameters
	switch (m_pobParameters->m_eLeverState)
	{
		case CounterWeightLeverSharedParameters::INACTIVE:
		{
			// We've gone all the way to the begining - wait for a new button press
			if (pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ))
			{
				m_pobParameters->m_fButtonRate = 0.0f;  // Make sure we do a decent first lift
				m_pobParameters->m_fButtonPressTime = 0.0f;	// Reset button press timer
				m_pobParameters->m_eLeverState = CounterWeightLeverSharedParameters::LIFT;
			}
			
			break;
		}

		case CounterWeightLeverSharedParameters::LIFT:
		{

			// Lift state button press scheme
			if (pobControllingEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION ))
			{
				m_pobParameters->m_fButtonRate = m_pobParameters->m_fButtonPressTime;  // Store the accumulated time since button press
				m_pobParameters->m_fButtonPressTime = 0.0f;	// Reset button press timer
			}

			m_obPushAnimation->SetSpeed( m_pobParameters->m_fAnimSpeed ); // Sync player anim speed to the crank anim speed
			m_obPushAnimation->SetBlendWeight( m_fBlendWeight );
			m_obPushAnimation->SetTime( m_pobParameters->m_fAnimTime ); // Sync player anim to the crank anim

			// g_VisualDebug->Printf2D(10,10,DC_GREEN,0,"Operator LIFT m_fButtonPressTime %.2f, m_fButtonRate %.2f, m_fRequiredRate %.2f",m_pobParameters-> m_fButtonPressTime, m_pobParameters->m_fButtonRate, m_pobParameters->m_fRequiredButtonRate);

			break;
		}

		case CounterWeightLeverSharedParameters::FALL:
		{

			// Recover section of anim is not synced to lever
			m_obPushAnimation->SetSpeed(m_fRecoverAnimSpeed); 
			m_obPushAnimation->SetBlendWeight( m_fBlendWeight );
			
			// Has operator finished movement? - remove controller
			if ( m_obPushAnimation->GetTime() >= m_obPushAnimation->GetDuration() )
			{
				return true;
			}

			// g_VisualDebug->Printf2D(10,10,DC_GREEN,0,"Operator LIFT m_fButtonPressTime %.2f, m_fButtonRate %.2f, m_fRequiredRate %.2f", m_pobParameters->m_fButtonPressTime, m_pobParameters->m_fButtonRate, m_pobParameters->m_fRequiredButtonRate);
			break;
		}
	} // switch (m_pobParameters->m_eLeverState)

	return false;
}

MovementController* CounterWeightLeverControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CounterWeightLeverController( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	CounterWeightLeverController::CounterWeightLeverController()
//!	
//!
//------------------------------------------------------------------------------------------

CounterWeightLeverController::CounterWeightLeverController( CMovement* pobMovement, const CounterWeightLeverControllerDef& obDefinition ) :
	MovementController( pobMovement )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( obDefinition );

	m_pobParameters = ObjectDatabase::Get().GetPointerFromName<CounterWeightLeverSharedParameters*>(obDefinition.m_obLeverParameters);

	ntAssert_p(m_pobParameters,("Serialised data %s does not exist!", ntStr::GetString(obDefinition.m_obLeverParameters)));

	if (m_pobParameters)
	{
		m_pobParameters->m_pobLeverEntity=(CEntity*)m_pobMovement->GetParentEntity();

		m_obLiftAnimation = m_pobAnimator->CreateAnimation( m_pobParameters->m_obAnimLeverLift );
		m_obLiftAnimation->SetBlendWeight( 1.0f );
		m_obLiftAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
		m_obLiftAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); // Sync to shared parameters
		m_obLiftAnimation->SetTime(m_pobParameters->m_fAnimTime); // Sync to shared parameters

		m_obFallAnimation = m_pobAnimator->CreateAnimation( m_pobParameters->m_obAnimLeverFall );
		m_obFallAnimation->SetBlendWeight( 1.0f );
		m_obFallAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOCOMOTING );
		m_obFallAnimation->SetSpeed(1.0f); 
		m_obFallAnimation->SetTime(0.0f); 

		//ntPrintf("m_obLiftAnimation Duration %f\n", m_obLiftAnimation->GetDuration() );
		//ntPrintf("m_obFallAnimation Duration %f\n", m_obFallAnimation->GetDuration() );

	}
}

CounterWeightLeverController::~CounterWeightLeverController( void )
{
	// Remove the animation if it was ever added
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obLiftAnimation );
		m_pobAnimator->RemoveAnimation( m_obFallAnimation );
	}
}

bool CounterWeightLeverController::Update( float fTimeStep, const CMovementInput& obMovementInput, const CMovementStateRef& obCurrentMovementState, CMovementState& obPredictedMovementState )
{
	UNUSED (obMovementInput);

	if (!m_pobParameters) // Oh dear, we don't have valid parameters
		return true;

	if ( m_bFirstFrame )
	{
		// Start the animations
		m_pobAnimator->AddAnimation( m_obLiftAnimation );
		m_pobAnimator->AddAnimation( m_obFallAnimation );
		m_bFirstFrame = false;
	}

	switch (m_pobParameters->m_eLeverState)
	{
		case CounterWeightLeverSharedParameters::INACTIVE:
		{

			break;
		}

		case CounterWeightLeverSharedParameters::LIFT:
		{

			// Increase button press timer
			// Done here as player may opt to leave the lever and it still needs to fall
			m_pobParameters->m_fButtonPressTime += fTimeStep; 

			// Once we've exceded the min rate update the rate so that the lever correctly falls with no player
			if (m_pobParameters->m_fButtonPressTime > m_pobParameters->m_fRequiredButtonRate)
				m_pobParameters->m_fButtonRate = m_pobParameters->m_fButtonPressTime;

			const float fRateVariation = 0.4f;
	
			// Update the anim speed - this is controlled by the lever
			// Rate faster than required rate
			if ( m_pobParameters->m_fButtonRate <= m_pobParameters->m_fRequiredButtonRate ) // Lifting lever
			{
				// How much faster button rate is
				float fRateDelta = m_pobParameters->m_fRequiredButtonRate - m_pobParameters->m_fButtonRate;
				fRateDelta /= fRateVariation;
				
				// Set speed relative to excess button rate
				m_pobParameters->m_fAnimSpeed = fRateDelta;

				// Check Bounds
				if (m_pobParameters->m_fAnimSpeed>1.0f)
					m_pobParameters->m_fAnimSpeed=1.0f;

				// Check here for end of lift anim
				if ( m_obLiftAnimation->GetTime() >= m_obLiftAnimation->GetDuration() )
				{
					//m_pobParameters->m_fAnimSpeed = 1.0f;
					m_pobParameters->m_fAnimTime = 0.0f;
					m_pobParameters->m_eLeverState = CounterWeightLeverSharedParameters::FALL;
					m_obFallAnimation->SetTime( m_pobParameters->m_fAnimTime );

					// Let Lua know its time for levers action
					CMessageSender::SendEmptyMessage( "msg_lever_action", m_pobMovement->GetParentEntity()->GetMessageHandler() );
				}

				//g_VisualDebug->Printf2D(10,30,DC_PURPLE,0,"Button rate: Rate %.2f, RequestedRate %.2f, RateDelta %.2f, AnimSpeed %.2f", m_pobParameters->m_fButtonRate, m_pobParameters->m_fRequiredButtonRate, fRateDelta, m_pobParameters->m_fAnimSpeed );

			}
			else // Lever falling back on player
			{
				// How much slower button rate is
				float fRateDelta = m_pobParameters->m_fRequiredButtonRate - m_pobParameters->m_fButtonRate;
				fRateDelta /= fRateVariation;
				
				// Set speed relative to excess button rate
				m_pobParameters->m_fAnimSpeed = fRateDelta;

				// Check Bounds
				if (m_pobParameters->m_fAnimSpeed<-1.0f)
					m_pobParameters->m_fAnimSpeed=-1.0f;

				// Check here for start of lift anim
				if ( m_obLiftAnimation->GetTime() <= 0.0f )
				{
					m_pobParameters->m_fAnimSpeed = 0.0f;
					m_pobParameters->m_eLeverState = CounterWeightLeverSharedParameters::INACTIVE;
				}

				//g_VisualDebug->Printf2D(10,30,DC_PURPLE,0,"Button rate: Rate %.2f, RequestedRate %.2f, RateDelta %.2f, AnimSpeed %.2f", m_pobParameters->m_fButtonRate, m_pobParameters->m_fRequiredButtonRate, fRateDelta, m_pobParameters->m_fAnimSpeed );

			}

			
			// Sync player anim speed to the lever anim speed
			m_obLiftAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); 
			m_obLiftAnimation->SetBlendWeight(m_fBlendWeight);
			m_pobParameters->m_fAnimTime=m_obLiftAnimation->GetTime();

			// Figure out the currently required input rate
			m_pobParameters->m_fRequiredButtonRate = 0.8f - ( m_obLiftAnimation->GetTime() / m_obLiftAnimation->GetDuration() ) * 0.6f;
			
			break;
		}

		case CounterWeightLeverSharedParameters::FALL:
		{

			m_pobParameters->m_fAnimSpeed += m_pobParameters->m_fAnimAcceleration * fTimeStep;

			// Check Bounds
			if (m_pobParameters->m_fAnimSpeed>1.0f)
				m_pobParameters->m_fAnimSpeed=1.0f;

			// Anim not syncd once in fall/recover states
			m_obFallAnimation->SetSpeed(m_pobParameters->m_fAnimSpeed); 
			m_obFallAnimation->SetBlendWeight(m_fBlendWeight);
			//m_pobParameters->m_fAnimTime=m_obFallAnimation->GetTime();

			break;
		}

	}  // switch (m_pobParameters->m_eLeverState)

	return false;
}
*/

/* // Collpase now managed by objects FSM - AnimatedCollapseTransition unused
//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransitionDef::AnimatedCollapseTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimatedCollapseTransitionDef::AnimatedCollapseTransitionDef( void ) :
	m_obAnimationName()
{
}


//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* AnimatedCollapseTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) AnimatedCollapseTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransition::AnimatedCollapseTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimatedCollapseTransition::AnimatedCollapseTransition( CMovement* pobMovement, const AnimatedCollapseTransitionDef& obDefinition ) :
	MovementController( pobMovement ),
	m_obSingleAnimation(),
	m_bFinished( false )
{
	// Register our definition for debugging purposes
	//InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetSpeed( 1.0f );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransition::~AnimatedCollapseTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
AnimatedCollapseTransition::~AnimatedCollapseTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && !m_bFinished)
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool AnimatedCollapseTransition::Update(	float						fTimeStep, 
											const CMovementInput&		obMovementInput,
											const CMovementStateRef&	obCurrentMovementState,
											CMovementState&				obPredictedMovementState)
{
	UNUSED (obMovementInput);
	UNUSED (obCurrentMovementState);
	UNUSED (obPredictedMovementState);

	if (m_bFinished)
		return true;

	// If we are on the first frame - add the animation to the animator
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;
	}

	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep) )
	{
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );

		Physics::AnimatedLG* lg = (Physics::AnimatedLG*)m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ANIMATED_LG );
		if( lg && lg->IsActive() )
			lg->MakeDynamic();

		m_bFinished=true;

		return true;
	}

	return false;
}
*/

