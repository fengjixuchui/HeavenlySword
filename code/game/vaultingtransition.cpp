//------------------------------------------------------------------------------------------
//!
//!	\file vaultingtransition.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/vaultingtransition.h"

#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityarcher.h"
#include "game/movement.h"
#include "game/entityinfo.h"
#include "game/inputcomponent.h"
#include "game/messagehandler.h"
#include "game/entitymanager.h"
#include "game/archermovementparams.h"

#include "anim/animator.h"
#include "anim/hierarchy.h"

#include "objectdatabase/dataobject.h"

#include "core/visualdebugger.h"
#include "core/exportstruct_anim.h"

//------------------------------------------------------------------------------------------
//!
//!	Declare the XML interface.
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	( VaultingTransitionDef )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_ApplyGravity, true, ApplyGravity )
	PUBLISH_VAR_AS				( m_FinalPosition, FinalPosition )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_AnimSpeed, 1.0f, AnimSpeed )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_AllowAnimToBeSquashed, false, AllowAnimToBeSquashed )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_StartScalePercent, 0.0f, StartScalePercent )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_EndScalePercent, 0.0f, EndScalePercent )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_VerticalScale, 1.0f, VerticalScale )
#	ifndef _RELEASE
		DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#	endif
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	VaultingTransitionDef::VaultingTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
VaultingTransitionDef::VaultingTransitionDef()
:	m_ApplyGravity			( false )
,	m_FinalPosition			( CONSTRUCT_CLEAR )
,	m_Facing				( 0.0f, 0.0f, 1.0f )
,	m_AnimSpeed				( 1.0f )
,	m_AllowAnimToBeSquashed	( false )
,	m_StartScalePercent		( 0.0f )
,	m_EndScalePercent		( 1.0f )
,	m_VerticalScale			( 1.0f )
{}

//------------------------------------------------------------------------------------------
//!
//!	VaultingTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* VaultingTransitionDef::CreateInstance( CMovement *movement ) const
{
	return NT_NEW VaultingTransition( movement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	VaultingTransition::VaultingTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
VaultingTransition::VaultingTransition( CMovement *movement, const VaultingTransitionDef &definition )
:	MovementController	( movement )
,	m_Definition		( definition )
,	m_AnimDistance		( CONSTRUCT_CLEAR )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_Definition );

	// Create our animation and add it to the animator
	m_SingleAnimation = m_pobAnimator->CreateAnimation( CHashedString(definition.m_pVaultingParams->m_VaultAnimName) );
	m_SingleAnimation->SetBlendWeight( 0.0f );
	m_SingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
    m_SingleAnimation->SetSpeed( definition.m_AnimSpeed );
}




//------------------------------------------------------------------------------------------
//!
//!	VaultingTransition::~VaultingTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
VaultingTransition::~VaultingTransition()
{
	// Remove the animation if it has been added to the animator
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_SingleAnimation );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	VaultingTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool VaultingTransition::Update(	float						fTimeStep, 
									const CMovementInput &		obMovementInput,
									const CMovementStateRef &	obCurrentMovementState,
									CMovementState &			obPredictedMovementState )
{
	UNUSED( obCurrentMovementState );
	UNUSED( obPredictedMovementState );

	// Set the weight on our animation and update it
	m_SingleAnimation->SetBlendWeight( m_fBlendWeight );

	// This is the first update
	if ( m_bFirstFrame )
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_SingleAnimation );
		m_bFirstFrame = false;
	}

	// Apply gravity if required
	ApplyGravity( m_Definition.m_ApplyGravity );

	// Accumulate the distance the animation has travelled up to this point.
	m_AnimDistance += m_SingleAnimation->GetRootTranslationDelta();

	// 
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_Definition.m_FinalPosition, 0.25f, 0xffff0000 );
#endif

	if ( m_SingleAnimation->GetTime() < m_SingleAnimation->GetDuration() )
	{
		Player* pPlayer = const_cast<Player*> ( m_pobMovement->GetParentEntity()->ToPlayer() );
		Archer* pArcher = pPlayer->ToArcher();

		ntAssert( pArcher->GetInputComponent() );

		// If in a popout window and the Archer can still vault.
		if( pArcher->IsVaultingSafe() &&
			m_Definition.m_pVaultingParams->m_NextVaultPopout.IsSet( m_SingleAnimation->GetTime(), m_SingleAnimation->GetDuration() ) )
		{
			if(	((pArcher->GetInputComponent()->GetVPressed() & (1 << AB_ACTION)) || (pArcher->GetInputComponent()->GetVHeld() & (1 << AB_ACTION))) && 
				 pArcher->VaultObjectIfAppropriate() )
			{
				return true;
			}
		}
		
		// If the pad input magnitude is greater than a magic value and the anim has a movement popout...
		if( m_Definition.m_pVaultingParams->m_MovementPopout.IsSet( m_SingleAnimation->GetTime(), m_SingleAnimation->GetDuration() ) &&
			 ( obMovementInput.m_fMoveSpeed >= 0.8f || CAINavigationSystemMan::Get().CloseToVaultingVolume( pArcher->GetPosition(), Archer::m_sCrouchActivatingRange ) ) )
		{
			/// .. Then pop out of the vault.
			Message msg(msg_vault_completed);
			pPlayer->GetMessageHandler()->Receive( msg );

			return true;
		}
		
		if ( m_Definition.m_pVaultingParams->m_DistanceScaling.IsSet( m_SingleAnimation->GetTime(), m_SingleAnimation->GetDuration() ) )
		{
			float fScaleWindowStart = m_Definition.m_pVaultingParams->m_DistanceScaling.GetFirstValue( m_SingleAnimation->GetDuration() );
			float fScaleWindowSize	= m_Definition.m_pVaultingParams->m_DistanceScaling.GetFirstValueLength( m_SingleAnimation->GetDuration() );
			float fScaleWindowPos	= m_SingleAnimation->GetTime();

			// Make sure we've got good values. 
			ntError( fScaleWindowStart != -1.0f && fScaleWindowSize != -1.0f );

			CPoint ptLeftToMove			= m_Definition.m_FinalPosition - obCurrentMovementState.m_obPosition;
			CPoint ptAnimMoveRemaining	= m_SingleAnimation->GetRootEndTranslation() - m_AnimDistance;

			// The difference from what the anim will move to the amount the anim should move.
			float fLenDiff = ptLeftToMove.Length() - ptAnimMoveRemaining.Length();

			// Make 
			if( fLenDiff > 0.0f )
			{
				// Calc a scalar based on the time remaining in the scaling window. 
				float fWindowComplete	= (fScaleWindowPos - fScaleWindowStart) / fScaleWindowSize;

				// Scale the animation.
				float fScale = 1.0f + (fLenDiff * fWindowComplete);
				obPredictedMovementState.m_obRootDeltaScalar = CDirection( fScale, 1.0f, fScale );
			}
		}
		else
		{
			//obPredictedMovementState.m_obRootDeltaScalar = CDirection( 1.0f, m_Definition.m_VerticalScale, 1.0f );
		}

		// Turn to the required angle.
		float target_rotation = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obRootMatrix.GetZAxis(), m_Definition.m_Facing );
		obPredictedMovementState.m_fProceduralYaw = target_rotation; // Provide a procedural rotation
	}

	// When we are finished indicate that to the movement component
	if ( m_SingleAnimation->GetTime() > ( m_SingleAnimation->GetDuration() - fTimeStep ) )
	{
		Player* pPlayer = const_cast<Player*> ( m_pobMovement->GetParentEntity()->ToPlayer() );

		/// .. Then pop out of the vault.
		Message msg(msg_vault_completed);
		pPlayer->GetMessageHandler()->Receive( msg );

		return true;
	}

	return false;
}



