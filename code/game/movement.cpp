/***************************************************************************************************
*
*       DESCRIPTION	
*
*       NOTES          
*
***************************************************************************************************/

//#include "physics/world.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"

// Necessary includes
#include "game/movement.h"
#include "game/inputcomponent.h"
#include "game/aicomponent.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"
#include "game/entityinfo.h"
#include "game/movementcontrollerinterface.h"
#include "anim/animator.h"
#include "game/messagehandler.h"
#include "camera/camutils.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "game/nsmanager.h"
#include "entityboss.h"
#endif
#include "entityinteractable.h"
#include "game/entitycatapult.h"

const float CMovement::COMBAT_BLEND = 0.15f;
const float CMovement::MOVEMENT_BLEND = 0.15f;

/***************************************************************************************************
*
*	FUNCTION		CMovementInput::CMovementInput
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CMovementInput::CMovementInput( void )
{
	ClearMovementInput();
}


/***************************************************************************************************
*
*	FUNCTION		CMovementInput::ClearMovementInput
*
*	DESCRIPTION		Clear the input - called at the end of a movement update
*
***************************************************************************************************/
void CMovementInput::ClearMovementInput( void )
{
	m_fMoveSpeed = 0.0f;			// Will be a 0-1 value
	m_obMoveDirection.Clear();		// The direction to move in
	m_fMoveSpeedAlt = 0.0f;			// Alternative move speed - uses the right hand analogue for human control
	m_obMoveDirectionAlt.Clear();	// Alternative move direction - uses the right hand analogue for human control
	m_obFacingDirection.Clear();	// The direction to look in
	m_obTargetPoint.Clear();		// The point to aim for (targeting etc)
	m_bTargetPointSet = false;		// Has a target been set this frame
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::CMovement
*
*	DESCRIPTION		This initialises the component
*
***************************************************************************************************/
CMovement::CMovement( CEntity*					pobParentEntity,
					  CAnimator*				pobAnimator,
					  Physics::System*			pobSystem )
:	
	m_obMovementInput(),
	m_pobParentEntity( pobParentEntity ),
	m_pobAnimator( pobAnimator ),
	m_pobSystem( pobSystem ),
	m_obCurrentMovementState(),
	m_obPredictedMovementState(),
	m_bControllerCompleted( false ),
	m_obControllers(),
	m_obChainedControllers(),
	m_obInteruptFeedback(),
	m_obCompletionFeedback(),
	m_eMovementMode( DMM_STANDARD ),
	m_bOrientationChanged( true ),
	m_bEnabled( true ),
	m_bBlocked( false ),
	m_bInputDisabled( false ),
	m_bControllerFinished( false ),
	m_iEnabledCount(0),
	m_bAIRequested( false ),
	m_bBeenUpdated( false ),
	m_uiNewControllerCount( 0x100 )
{
	ATTACH_LUA_INTERFACE(CMovement);
#ifndef _RELEASE
		m_NumControllersAddedThisFrame = 0;
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CMovement::~CMovement
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CMovement::~CMovement( void )
{
	// Partial anim destruction
	if ( !(!m_pobPartialAnim) )
	{
		m_pobAnimator->RemoveAnimation( m_pobPartialAnim );
	}

	ClearControllers();
}


// Debug functionality below for the movement system
#ifndef _RELEASE

/***************************************************************************************************
*
*	FUNCTION		CMovement::GetControllerDebugDetails
*
*	DESCRIPTION		Get debug data for a particular controller on the stack
*
***************************************************************************************************/
void CMovement::GetControllerDebugDetails(	int				iControllerNumber, 
											const char*&	pcInstanceName,
											const char*&	pcTypeName,
											float&			fWeight ) const
{
	// Find the controller number requested
	int iController = 0;
	ntstd::List< CControllerDetails*, Mem::MC_ENTITY >::const_iterator obEnd = m_obControllers.end();
	for( ntstd::List< CControllerDetails*, Mem::MC_ENTITY>::const_iterator obIt = m_obControllers.begin(); obIt != obEnd; ++obIt )
	{
		// If this is the controller we want give them the details
		if ( iController == iControllerNumber )
		{
			pcInstanceName = ( *obIt )->pobController->GetInstanceName();
			pcTypeName = ( *obIt )->pobController->GetTypeName();
			fWeight = ( * obIt )->fLocalWeight;
			return;
		}

		// Otherwise count on
		++iController;
	}

	// If we are here we don't have enough controllers
	fWeight = 0.0f;
	pcInstanceName = 0;
	pcTypeName = 0;
}

#endif


/***************************************************************************************************
*
*	FUNCTION		CMovement::ClearControllers
*
*	DESCRIPTION		Clear out any content on this component
*
***************************************************************************************************/
void CMovement::ClearControllers( void )
{
	// Has the movement been interrupted?
	if ( !m_bControllerCompleted )
		m_obInteruptFeedback.GiveFeedback();

	// Clear the feedback systems
	m_obInteruptFeedback.Clear();
	m_obCompletionFeedback.Clear();

	// Remove everything from our controller lists
	m_bControllerFinished = false;
	ClearControllerList( m_obControllers );
	ClearChainedControllerList( m_obChainedControllers );

#ifndef _RELEASE
	m_NumControllersAddedThisFrame = 0;
#endif
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetMovementDynamicsMode
*
*	DESCRIPTION		This sets the correct movement mode depending on the type of movement controller
*					on the top of our movement stack.  Return true if the mode has changed - return
*					value not currently used to my knowledge.
*
*					MUS - there is currently no difference between hard and soft movement - but
*					obviously there is now the space to add it.
*
***************************************************************************************************/
bool CMovement::SetMovementDynamicsMode( DYNAMICS_MOVEMENT_MODE eMovementMode )
{
	// Early out if there is no state change
	if ( m_eMovementMode == eMovementMode )
		return false;

	// Switch on the current state
	switch ( m_eMovementMode )
	{
	case DMM_STANDARD:
		// Do we need to start hard relative mode?
		if ( eMovementMode == DMM_HARD_RELATIVE )
		{
			// If we have an associated physics system
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerCollidable( false );
#endif
			}
		}

		// Or maybe soft relative...
		else if ( eMovementMode == DMM_SOFT_RELATIVE )
		{
			//Removed the below due to odd Kai Skill-evade behaviour, where calling SetCharacterControllerDynamicCollidable
			//below actually blats over all of the physics-filtering we've set up, and just replace it with "collide with geometry".
			//This was causing shaky popping behaviour if a skill evade took you into any static geometry.
			//Duncan assures me this was only used in the combat system, and the combat system takes care of it's own filter-setting, so
			//isn't needed. However, if any wierd bugs arise due to this being removed, tell GavinC.
/*
			// If we have an associated physics system
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerDynamicCollidable( false );		
#endif
			}
*/
		}
		break;

	case DMM_HARD_RELATIVE:
		// Do we need to move to soft relative
		if ( eMovementMode == DMM_SOFT_RELATIVE )
		{
// If we have an associated physics system
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerDynamicCollidable( false );		
#endif
			}
		}

		// Or move back to standard mode
		else if ( eMovementMode == DMM_STANDARD )
		{
			// If we have an associated physics system
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerCollidable( true );
#endif
			}
		}
		break;

	case DMM_SOFT_RELATIVE:
		// Do we need to start hard relative mode?
		if ( eMovementMode == DMM_HARD_RELATIVE )
		{
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerCollidable( false );
#endif
			}
		}

		// Or maybe standard mode...
		else if ( eMovementMode == DMM_STANDARD )
		{
			// If we have an associated physics system
			if ( m_pobSystem )
			{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
				// Get if we can get hold of a character representation
				Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if ( ctrl ) 
					ctrl->SetCharacterControllerCollidable( true );
#endif
			}
		}

		break;

	default:
		ntAssert( 0 );
		break;
	}

	// Save the mode change
	m_eMovementMode = eMovementMode;

	// Indicate the change
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::ClearControllerList
*
*	DESCRIPTION		Clear out a list of controllers
*
***************************************************************************************************/
void CMovement::ClearControllerList( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>& obControllerList )
{
	// Loop through and destroy all the actual controllers
	ntstd::List< CControllerDetails*, Mem::MC_ENTITY >::iterator obEnd = obControllerList.end();
	for( ntstd::List< CControllerDetails*, Mem::MC_ENTITY >::iterator obIt = obControllerList.begin(); obIt != obEnd; ++obIt )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obIt )->pobController );
		NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obIt ) );
	}

	// Set the dynamics mode back to normal
	// This resets physics settings so commenting it out for now, 
	// doesn't really make sense because the next controller that 
	// gets pushed on will set whatever DMM it needs - DGF
	//SetMovementDynamicsMode( DMM_STANDARD );

	// Remove everything from the list
	obControllerList.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::ClearChainedControllerList
*
*	DESCRIPTION		Clear out a list of chained controllers
*
***************************************************************************************************/
void CMovement::ClearChainedControllerList( ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>& obControllerList )
{
	// Loop through and destroy all the actual controllers
	ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>::iterator obEnd = obControllerList.end();
	for(ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>::iterator obIt = obControllerList.begin(); obIt != obEnd; ++obIt)
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, (*obIt)->pobControllerDef);
		NT_DELETE_CHUNK(Mem::MC_ENTITY, (*obIt));
	}

	// Remove everything from the list
	obControllerList.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetEnabled
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CMovement::SetEnabled( bool bEnabled )
{ 
	if (bEnabled)
	{
		ntAssert(m_iEnabledCount > 0);

		if( m_iEnabledCount <= 0 )
		{
			m_iEnabledCount = 0;

			if( m_bEnabled )
			{
				return;
			}
		}
		else
		{
		--m_iEnabledCount;
		}

		
		// Set the flag
		if (m_iEnabledCount == 0)
			m_bEnabled = true; 
	}
	else
	{
		++m_iEnabledCount;
		// Set the flag
		m_bEnabled = false; 
	}

	// Clear off anim wieghts if we are disabled
	if ( m_bEnabled && m_pobAnimator )
		m_pobAnimator->ClearAnimWeights(); 
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetInputData
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CMovement::SetInputData( void )
{
	// Grab the input data for this update depending on entity type - if input disabled
	if ( m_bInputDisabled || m_pobParentEntity == NULL )
	{
		m_obMovementInput.m_fMoveSpeed = 0.0f;
		m_obMovementInput.m_obMoveDirection.Clear();
		m_obMovementInput.m_obFacingDirection.Clear();
		m_obMovementInput.m_obTargetPoint.Clear();
		m_obMovementInput.m_bTargetPointSet = false;
	}
	// ...player controlled
	else if ( const CInputComponent* pobInput = m_pobParentEntity->GetInputComponent() )
	{
		// Get the primary stick details
		m_obMovementInput.m_fMoveSpeed = pobInput->GetInputSpeed();
		m_obMovementInput.m_obMoveDirection = pobInput->GetInputDir();

		// Get the secondary stick details
		m_obMovementInput.m_fMoveSpeedAlt = pobInput->GetInputSpeedAlt();
		m_obMovementInput.m_obMoveDirectionAlt = pobInput->GetInputDirAlt();

		// If the action button is held then we give a vertical input
		if ( pobInput->GetVHeld() & ( 1 << AB_ACTION ) )
			m_obMovementInput.m_obMoveDirection.Y() = 1.0f;

		// ...to be dealt with
		m_obMovementInput.m_obFacingDirection = CDirection( CONSTRUCT_CLEAR );
	}
	// ...AI controlled
	else if(m_pobParentEntity->IsAI())
	{
		const CAIComponent* pobAI = ((AI*)m_pobParentEntity)->GetAIComponent();

		// Set the primary input speed - kill it if the last movement was stopped by dynamics
		//if ( m_pobDynamics )
		if( m_pobSystem )
		{
			#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
			Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if( ctrl ) 
			{
				// m_obMovementInput.m_fMoveSpeed = ( m_pobDynamics->m_HasMoved ) ? pobAI->GetMovementMagnitude() : 0.0f;
				m_obMovementInput.m_fMoveSpeed = pobAI->GetMovementMagnitude();
			}
			#endif
		} else {
			m_obMovementInput.m_fMoveSpeed = 0.0f;
		}
		m_obMovementInput.m_obMoveDirection = pobAI->GetMovementDirection();

		// Get the facing direction - also posted by the AI
		m_obMovementInput.m_obFacingDirection = pobAI->GetMovementFacing();

		// We set the AI secondary input from the primary - this may have to change if the 
		// possible movement becomes more complex than it currently is - the AI would need to post more info
		m_obMovementInput.m_fMoveSpeedAlt = pobAI->GetMovementMagnitude();
		m_obMovementInput.m_obMoveDirectionAlt = pobAI->GetMovementDirection();

		// For AIs controlling Cannons or other similar objects
		m_obMovementInput.m_fYaw	= pobAI->GetUseObjectYaw();
		m_obMovementInput.m_fPitch	= pobAI->GetUseObjectPitch();
		m_obMovementInput.m_pAIUser = pobAI->GetParent();
	}
	else if ( m_pobParentEntity->IsBoss() )
	{
		Boss* pobBoss = (Boss*)m_pobParentEntity;
		m_obMovementInput = *(pobBoss->GetBossMovement());
	}
	// ...Interactable FSM controlled
	else if ( m_pobParentEntity->IsInteractable() )
	{
		Interactable* pobInteractable = m_pobParentEntity->ToInteractable();

		// Currently catapult is only object moved using MovementInput
		// If more are added then it may be best to push the CMovementInput down
		// from catapult to Interactable - T McK
		if ( Interactable::EntTypeInteractable_Object_Catapult == pobInteractable->GetInteractableType() )
		{
			m_obMovementInput = *( ((Object_Catapult*)pobInteractable)->GetMovementInput() );
		}
		else
		// ...seemingly out of control
		{
			m_obMovementInput.m_fMoveSpeed = 0.0f;
			m_obMovementInput.m_obMoveDirection.Clear();
			m_obMovementInput.m_obFacingDirection.Clear();
			m_obMovementInput.m_obTargetPoint.Clear();
			m_obMovementInput.m_bTargetPointSet = false;
		}
	}

	// ...seemingly out of control
	else
	{
		m_obMovementInput.m_fMoveSpeed = 0.0f;
		m_obMovementInput.m_obMoveDirection.Clear();
		m_obMovementInput.m_obFacingDirection.Clear();
		m_obMovementInput.m_obTargetPoint.Clear();
		m_obMovementInput.m_bTargetPointSet = false;
	}

	// Now take this information and check with the physics representation whether this sort of input is acceptable
	PhysicsTweakInput();
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::PhysicsTweakInput
*
*	DESCRIPTION		Ask the CC if it's a reasonable input
*
***************************************************************************************************/
void CMovement::PhysicsTweakInput( void )
{
	// I've removed all this after rewriting the character controller because it was doing some quite expensive stuff without very good reason
	// I'll add stuff in here if and when it's needed - DGF
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CMovement::Update(	float fTimeStep )
{
#	ifndef _RELEASE
		m_NumControllersAddedThisFrame = 0;
#	endif

	// If we have been turned off - do nothing
	if( !m_bEnabled ) 
		return;

	// If we have a zero size time step we early out to be safe - the movement system
	// wouldn't do anything useful with a zero time step anyway
	if ( fTimeStep == 0.0f ) 
		return;

	// Flag that the movement has been updated.. This flag is used later in UpdatePostAnimator
	// because if this method hasn't been called UpdatePostAnimator will have some nasty side effects
	m_bBeenUpdated = true;

	// Set up the input stuff
	SetInputData();

	// Set up the current movement state and intialise the predicted state for this frame
	SetCurrentAndPredictedMovementStates( fTimeStep );

	// Update our current controller weights
	UpdateControllerWeights( fTimeStep );

	// The default is zero blend weight for all anims ( if other things create anims on these chars
	// will need to change this ) this is so that transitions can be cleaned up nicely
	if(m_pobAnimator)
		m_pobAnimator->ClearAnimWeights();	

	// Has the top controller finished its movement - may signal if it is not a looping controller
	m_bControllerFinished = false;

	// Update partial anim
	UpdatePartialAnim( fTimeStep );

	// Loop through all the controllers and give them an update message
	ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::reverse_iterator obEnd = m_obControllers.rend();
	for( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::reverse_iterator obIt = m_obControllers.rbegin(); obIt != m_obControllers.rend(); ++obIt )
	{
		// We reset the procural stuff for each controller - only the top controller has full procedural control
		m_obPredictedMovementState.m_obProceduralRootDelta.Clear();
		m_obPredictedMovementState.m_fProceduralYaw = 0.0f;
		m_obPredictedMovementState.m_fProceduralPitch = 0.0f;
		m_obPredictedMovementState.m_fProceduralRoll = 0.0f;
		m_obPredictedMovementState.m_fRootRotationDeltaScalar = 1.0f;
		m_obPredictedMovementState.m_obRootDeltaScalar = CDirection( 1.0f, 1.0f, 1.0f );
		m_obPredictedMovementState.m_bApplyExplicitRotations = false;

		m_bControllerFinished = ( *obIt )->pobController->Update(	fTimeStep,
																	m_obMovementInput,
																	m_obCurrentMovementState,
																	m_obPredictedMovementState );

		// ***Direct animation scalar and procedural control is given to the top level controller***
	}

	// Clear our input
	m_obMovementInput.ClearMovementInput();

	// If the current controller is complete we may have a callback
	if ( m_bControllerFinished )
		GiveControllerFeedback();	
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetCurrentAndPredictedMovementStates
*
*	DESCRIPTION		Set up the current movement state and then duplicate it into the predicted
*
***************************************************************************************************/
void CMovement::SetCurrentAndPredictedMovementStates( float fTimeStep )
{
	UNUSED( fTimeStep );
	// Reference data
	{
		//if ( m_pobDynamics )
		if( m_pobSystem )
		{
			#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
			Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if( ctrl ) 
			{
				// Get the current root matrix from the dynamic system
				//m_pobDynamics->GetCharacterWorldMatrix( m_obCurrentMovementState.m_obRootMatrix );
				ctrl->GetRootWorldMatrix( m_obCurrentMovementState.m_obRootMatrix );

				// Set the 'on ground' from the dynamics
				//m_obCurrentMovementState.m_bOnGround = m_pobDynamics->IsBodyOnGround();
				m_obCurrentMovementState.m_bOnGround = ctrl->IsOnGround();

				// Set the facing direction - just the z on the root
				m_obCurrentMovementState.m_obFacing = m_obCurrentMovementState.m_obRootMatrix.GetZAxis();

				// Get the orientation - also just straight from the root - this flag should only be true on
				// the first update - from then on we keep this orientation value to ourselves for stability
				// if system is softparented the orientation is changed during update
				if ( m_bOrientationChanged || ctrl->IsSoftParented())
					m_obCurrentMovementState.m_obOrientation = CQuat( m_obCurrentMovementState.m_obRootMatrix );
				
				// Move redundant data?  Set the position from the root matrix too.
				m_obCurrentMovementState.m_obPosition = m_obCurrentMovementState.m_obRootMatrix.GetTranslation();	

			} else {
	
				m_obCurrentMovementState.m_obRootMatrix = m_pobParentEntity->GetMatrix();
				m_obCurrentMovementState.m_bOnGround	= true;

				// Set the facing direction - just the z on the root
				m_obCurrentMovementState.m_obFacing = m_obCurrentMovementState.m_obRootMatrix.GetZAxis();

				// Get the orientation - also just straight from the root - this flag should only be true on
				// the first update - from then on we keep this orientation value to ourselves for stability
				if ( m_bOrientationChanged )
					m_obCurrentMovementState.m_obOrientation = CQuat( m_obCurrentMovementState.m_obRootMatrix );
				
				// Move redundant data?  Set the position from the root matrix too.
				m_obCurrentMovementState.m_obPosition = m_obCurrentMovementState.m_obRootMatrix.GetTranslation();

			}
			#endif
		}
	}

	// Predicted Data
	{
		// Clear all the procedural data
		m_obPredictedMovementState.m_obProceduralRootDelta.Clear();	
		m_obPredictedMovementState.m_fProceduralYaw = 0.0f;
		m_obPredictedMovementState.m_fProceduralPitch = 0.0f;
		m_obPredictedMovementState.m_fProceduralRoll = 0.0f;
		m_obPredictedMovementState.m_bApplyExplicitRotations = false;
		
		// Clear the scalar data
		m_obPredictedMovementState.m_obRootDeltaScalar = CDirection( 1.0f, 1.0f, 1.0f );
		m_obPredictedMovementState.m_fRootRotationDeltaScalar = 1.0f;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::UpdatePostAnimator
*
*	DESCRIPTION		SHOULD BE CONST ON ALL BUT DYNAMICS AND FOOT TRACKING
*
*					Concatenates the root delta from the animator to the current delta on the 
*					dynamics.
*
*					If we don't have any controllers we just add the animator deltas directly to the
*					dynamics details.
*
*					If we do have active controllers we have to scale the animator movement by the
*					scalars that movement controllers can set up.  Controllers can also add
*					procedural movement which has to be considered too.
*
*					This is all written in a way to reduce any rounding issues.  If movement is not
*					requested at any point the previous value is used directly.
*
***************************************************************************************************/
void CMovement::UpdatePostAnimator( float fTimeStep )
{
	// If we have been turned off - do nothing
	if (( !m_bEnabled || !m_bBeenUpdated ) && m_pobSystem)
	{
		// but if we are soft parented, we need update because character could been moved 
		Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		if ( ctrl && ctrl->IsSoftParented()) 
		{
			CMatrix obWorldMatrix;
			ctrl->GetRootWorldMatrix( obWorldMatrix );
			ctrl->SetTurnTo(CQuat(obWorldMatrix));
			ctrl->SetMoveTo(obWorldMatrix.GetTranslation());
		}
		return;
	}


	// If we have a zero size time step we early out to be safe - the movement system
	// wouldn't do anything useful with a zero time step anyway
	if ( fTimeStep == 0.0f ) return;

	// Where are we going
	CPoint	obNewPosition( CONSTRUCT_CLEAR );
	CQuat	obNewOrientation( CONSTRUCT_IDENTITY );

	// If we have controllers we try and move the character
	if ( m_obControllers.size() > 0 )
	{
		// Get the character's world matrix from the dynamics
		CMatrix obWorldMatrix;

		// If we have a dynamics component get the matrix from there
		if ( m_pobSystem )
		{
			#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
			Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if ( ctrl ) 
				ctrl->GetRootWorldMatrix( obWorldMatrix );
			else
				obWorldMatrix = m_pobParentEntity->GetMatrix();
			#endif
		}
		else
			obWorldMatrix = m_pobParentEntity->GetMatrix();

		// Make sure we don't have any pitch or roll on any of the character animations
		// ntAssert( ( m_pobAnimator->GetRootRotationDelta().X() < EPSILON ) && ( m_pobAnimator->GetRootRotationDelta().X() > -EPSILON ) );
		// ntAssert( ( m_pobAnimator->GetRootRotationDelta().Z() < EPSILON ) && ( m_pobAnimator->GetRootRotationDelta().Z() > -EPSILON ) );

		// Get the rotation supplied by the animator
		CQuat obRootRotation( m_pobAnimator ? m_pobAnimator->GetRootRotationDelta() : CQuat(CONSTRUCT_CLEAR) );

		//ntPrintf("%s: obRootRotation %f %f %f %f\n", m_pobParentEntity->GetName().c_str(), obRootRotation.X(), obRootRotation.Y(), obRootRotation.Z());
		//ntPrintf("%s: m_obProceduralRootDelta %f %f %f\n", m_pobParentEntity->GetName().c_str(), m_obPredictedMovementState.m_obProceduralRootDelta.X(), m_obPredictedMovementState.m_obProceduralRootDelta.Y(), m_obPredictedMovementState.m_obProceduralRootDelta.Z());
		//ntPrintf("%s: m_obProceduralYPR %f %f %f\n", m_pobParentEntity->GetName().c_str(), m_obPredictedMovementState.m_fProceduralYaw, m_obPredictedMovementState.m_fProceduralPitch, m_obPredictedMovementState.m_fProceduralRoll);
		//ntPrintf("%s: m_obRootDeltaScalar %f %f %f\n", m_pobParentEntity->GetName().c_str(), m_obPredictedMovementState.m_obRootDeltaScalar.X(), m_obPredictedMovementState.m_obRootDeltaScalar.Y(), m_obPredictedMovementState.m_obRootDeltaScalar.Z());
		//ntPrintf("%s: m_fRootRotationDeltaScalar %f\n", m_pobParentEntity->GetName().c_str(), m_obPredictedMovementState.m_fRootRotationDeltaScalar);

		// Only use the rotation supplied by animations if it is significant
		CDirection obAxis;
		float fAngle;
		obRootRotation.GetAxisAndAngle( obAxis, fAngle );
		if ( fAngle > EPSILON )
		{
			// Scale the animated rotation if requested
			if ( m_obPredictedMovementState.m_fRootRotationDeltaScalar != 1.0f )
				obRootRotation = CQuat::Slerp( CVecMath::GetQuatIdentity(), obRootRotation, m_obPredictedMovementState.m_fRootRotationDeltaScalar );

			// Update the new orientation and set the flag that we have changed it
			obNewOrientation = m_obCurrentMovementState.m_obOrientation * obRootRotation;
			m_bOrientationChanged = true;
		}

		// Otherwise we maintain the existing rotation
		else
		{
			obNewOrientation = m_obCurrentMovementState.m_obOrientation;
			m_bOrientationChanged = false;
		}

		// Apply Yaw and Pitch where necessary
		if ( m_obPredictedMovementState.m_bApplyExplicitRotations == true )
		{
			obNewOrientation=CCamUtil::QuatFromEuler_XYZ(	m_obPredictedMovementState.m_fProceduralPitch,
															m_obPredictedMovementState.m_fProceduralYaw,
															m_obPredictedMovementState.m_fProceduralRoll );

			// The orientation needs to be updated
			m_bOrientationChanged = true;
		}

		// We are applying pitch/yaw/roll deltas
		else 
		{
			if ( ( m_obPredictedMovementState.m_fProceduralYaw != 0.0f )
				||
				( m_obPredictedMovementState.m_fProceduralPitch != 0.0f )
				||
				( m_obPredictedMovementState.m_fProceduralRoll != 0.0f ) )
			{
				if ( fabsf( m_obPredictedMovementState.m_fProceduralPitch ) > 0.0f )
					obNewOrientation = obNewOrientation * CQuat( CVecMath::GetXAxis(), m_obPredictedMovementState.m_fProceduralPitch );

				if ( fabsf( m_obPredictedMovementState.m_fProceduralYaw ) > 0.0f )
					obNewOrientation = obNewOrientation * CQuat( CVecMath::GetYAxis(), m_obPredictedMovementState.m_fProceduralYaw );

				if ( fabsf( m_obPredictedMovementState.m_fProceduralRoll ) > 0.0f )
					obNewOrientation = obNewOrientation * CQuat( CVecMath::GetZAxis(), m_obPredictedMovementState.m_fProceduralRoll );

				// The orientation needs to be updated
				m_bOrientationChanged = true;
			}
		}

		// Get the translation from the animator and scale it by the controller set scalar - sanity check values 
		CDirection obRootDelta = CDirection( m_pobAnimator ? CDirection(m_pobAnimator->GetRootTranslationDelta()) : CDirection(CONSTRUCT_CLEAR) );

		//ntPrintf("%s: obRootDelta %f %f %f\n", m_pobParentEntity->GetName().c_str(), obRootDelta.X(), obRootDelta.Y(), obRootDelta.Z());
		
		// If we have some movement from the animator...
		if ( obRootDelta.LengthSquared() > EPSILON )
		{
			obRootDelta.X() *= m_obPredictedMovementState.m_obRootDeltaScalar.X();
			obRootDelta.Y() *= m_obPredictedMovementState.m_obRootDeltaScalar.Y();
			obRootDelta.Z() *= m_obPredictedMovementState.m_obRootDeltaScalar.Z();

			// Find the new translation by mutliplying by the world matrix from the dynamics
			CDirection obGoalMove;
			obGoalMove = obRootDelta * obWorldMatrix;

			// Add on the procedural translation that may be added by a controller - add the total to 
			// our current position ( should it be the dynamics position we add to? - GH )
			obGoalMove += m_obPredictedMovementState.m_obProceduralRootDelta;
			obNewPosition = m_obCurrentMovementState.m_obPosition + obGoalMove;
		}

		// Otherwise just use the existing position with any procedural movement
		else
		{
			obNewPosition = m_obCurrentMovementState.m_obPosition + m_obPredictedMovementState.m_obProceduralRootDelta;
		}

		// We may have some jolly clever relative movement stuff going on on the animator
		if(m_pobAnimator && m_pobAnimator->HasRelativeMovement() && m_obControllers.size() > 0 &&
			(m_obControllers.front()->eMovementMode == DMM_SOFT_RELATIVE || m_obControllers.front()->eMovementMode == DMM_HARD_RELATIVE))
		{
			// Check that the weight of our relative animations is sensible
			ntAssert( m_pobAnimator->GetRelativeMovementWeight() >= 0.0f );
			ntAssert( m_pobAnimator->GetRelativeMovementWeight() <= 1.0f );

			// Lerp the positions based on the combined weight of the relative anims
			obNewPosition = CPoint::Lerp(	obNewPosition, 
			 								m_pobAnimator->GetRelativeMovementPosition(), 
			 								( m_pobAnimator->GetRelativeMovementWeight() / 1.0f ) );

			// Slerp the rotations based on the combined weight of the relative anims
			obNewOrientation = CQuat::Slerp(	obNewOrientation, 
			 									m_pobAnimator->GetRelativeMovementRotation(),
												( m_pobAnimator->GetRelativeMovementWeight() / 1.0f ) ); 

			// The orientation needs to be updated
			m_bOrientationChanged = true;

			if (!m_pobParentEntity->IsInNinjaSequence() )
			{
				// Make sure we don't have any pitch or roll on any of the character animations
				// ntAssert( ( m_pobAnimator->GetRelativeMovementRotation().X() < EPSILON ) && ( m_pobAnimator->GetRelativeMovementRotation().X() > -EPSILON ) );
				// ntAssert( ( m_pobAnimator->GetRelativeMovementRotation().Z() < EPSILON ) && ( m_pobAnimator->GetRelativeMovementRotation().Z() > -EPSILON ) );
			}
		}

		// Tell the dynamics what to do - is this really the only interface we need with dynamics?
		if ( m_pobSystem )
		{
			#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
			Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobSystem->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if( ctrl ) 
			{
				ctrl->SetTurnTo( obNewOrientation);
				ctrl->SetMoveTo( obNewPosition );

				WS_SYNC_TYPE syncType = WS_SYNC_NONE;
				if(m_pobAnimator && m_pobAnimator->HasRelativeMovement() && m_obControllers.size() > 0)
				{
					if (m_obControllers.front()->eMovementMode == DMM_SOFT_RELATIVE)
						syncType = WS_SYNC_SOFT;
					else if (m_obControllers.front()->eMovementMode == DMM_HARD_RELATIVE)
						syncType = WS_SYNC_HARD;
				}

				ctrl->SetWorldSpaceSynchronised( syncType );
			}
			else // Parent entity does not have a character controller, so set the position/rotation on the hierarchy
			{
				m_pobParentEntity->SetRotation( obNewOrientation );
				m_pobParentEntity->SetPosition( obNewPosition );
			}
			#endif
		}
		else
		{
			// GM added this 13/01/06: Entities created during ninja sequences won't have physics components
			// but have relative animations so need their position updating...
			if (m_pobParentEntity->IsInNinjaSequence() )
			{
				m_pobParentEntity->SetRotation( obNewOrientation );
				m_pobParentEntity->SetPosition( obNewPosition );
			}
		}

		// If the orientation has changed then we should normalise it otherwise it starts to get a little screwy after lots of updates
		if ( m_bOrientationChanged )
			obNewOrientation.Normalise();
			
		m_bOrientationChanged = false;
		m_obCurrentMovementState.m_obOrientation = obNewOrientation;

		// CDirection paulsRequestedVelocity( ( obNewPosition - m_obCurrentMovementState.m_obPosition ) / fTimeStep );
		// if (paulsRequestedVelocity.Length() > 20.0f && m_pobParentEntity->GetName() != "table" && !NSManager::Get().IsNinjaSequencePlaying())
		// {
		// 	user_warn_p( false,( "Movement Controller requesting an excessively large velocity\n" ) );
		// 	ntPrintf( "%s, numControllers: %d current: %.1f last: %.1f\n", m_pobParentEntity->GetName().c_str(), m_obControllers.size(), paulsRequestedVelocity.Length(), m_obCurrentMovementState.m_obLastRequestedVelocity.Length() );
        // }
	}

	// What has been the requested velocity this frame?
	CDirection obRequestedVelocity( ( obNewPosition - m_obCurrentMovementState.m_obPosition ) / fTimeStep );

	// What has been the acceleration over the last frame
	m_obCurrentMovementState.m_obLastRequestedAcceleration = ( obRequestedVelocity - m_obCurrentMovementState.m_obLastRequestedVelocity ) / fTimeStep;

	// Using the requested positions and the time step we can find some useful velocities
	m_obCurrentMovementState.m_obLastRequestedVelocity = obRequestedVelocity;

	// If the current controller is complete we may have chained items to add
	if ( m_bControllerFinished )
		UseChainedControllers();
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::BringInNewController
*
*	DESCRIPTION		Add a controller to be used immediately
*
*					If someone is externally pushing on movement control then we need to clean up 
*					any chained controllers and callbacks that are setup in the system.
*
***************************************************************************************************/
//bool CMovement::BringInNewControllerFL(	const char* pcFile, int iLine,
bool CMovement::BringInNewController(
									    const MovementControllerDef&	obControllerDef, 
										DYNAMICS_MOVEMENT_MODE			eMovementMode, 
										float							fFadeInTime)
{
	//ntPrintf("%s(%d): BringInNewController on %s - ticks %d\n", pcFile, iLine, m_pobParentEntity->GetName().c_str(), CTimer::Get().GetSystemTicks());

	// We can't do anything if disabled or blocked
	if ( !m_bEnabled || m_bBlocked ) 
	{
		return false;
	}

	// Has the movement been interrupted?
	if ( !m_bControllerCompleted )
		m_obInteruptFeedback.GiveFeedback();

	// Clear the feedback systems
	m_obInteruptFeedback.Clear();
	m_obCompletionFeedback.Clear();

	// Reset the completed flag
	m_bControllerCompleted = false;

	// Make sure that the chained controller stack is empty
	ClearChainedControllerList( m_obChainedControllers );

	// Assume the controller was not requested by an AI
	m_bAIRequested = false;

	// Do the rest internally
	InternalBringInNewController( obControllerDef, eMovementMode, fFadeInTime );

	// return that a new controller was added. 
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::InternalBringInNewController
*
*	DESCRIPTION		Add a controller to be used immediately
*
***************************************************************************************************/
void CMovement::InternalBringInNewController(	const MovementControllerDef&	obControllerDef, 
												DYNAMICS_MOVEMENT_MODE			eMovementMode,
												float							fFadeInTime )
{
	// Some movement controllers do not have animators
	if(m_pobAnimator)
	{
		// Need to clear the anim event list here
		m_pobAnimator->GetAnimEventHandler().ClearAnimations();
		// Clear our any zero weight animations
		m_pobAnimator->RemoveAllAnimationsBelowWeight( 0.001f );
	}

	// Create a new controller reference
	CControllerDetails* pobControllerDetails = NT_NEW_CHUNK(Mem::MC_ENTITY) CControllerDetails;
	pobControllerDetails->pobController = obControllerDef.CreateInstance( this );
	pobControllerDetails->fFadeInTime = fFadeInTime;
	pobControllerDetails->eMovementMode = eMovementMode;

	// Set the appropriate dynamics mode for this movement
	SetMovementDynamicsMode( eMovementMode );

	// Push it on to the front of our stack of stuff
	m_obControllers.push_front( pobControllerDetails );

	++m_uiNewControllerCount;

#	ifndef _RELEASE
		m_NumControllersAddedThisFrame++;
		if ( m_NumControllersAddedThisFrame > 2)
		{
			ntPrintf("AI: [%s] is pushing in the same frame %d controllers\n",	m_pobParentEntity ? ntStr::GetString(m_pobParentEntity->GetName()) : "NULL!!!",
																				m_NumControllersAddedThisFrame);
			ntError_p( m_NumControllersAddedThisFrame <= 5, ("How many controllers do we need to add in a single frame?!") );
		}
#	endif
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::AddChainedController
*
*	DESCRIPTION		Add a controller to be used after the current lead controller is completed
*
***************************************************************************************************/
void CMovement::AddChainedController(	const MovementControllerDef&	obControllerDef, 
										DYNAMICS_MOVEMENT_MODE			eMovementMode,
										float							fFadeInTime )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Clear the feedback systems
	m_obInteruptFeedback.Clear();
	m_obCompletionFeedback.Clear();

	// Create a new controller reference
	CChainedControllerDetails* pobControllerDetails = NT_NEW_CHUNK(Mem::MC_ENTITY) CChainedControllerDetails;
	pobControllerDetails->pobControllerDef = obControllerDef.Clone();
	pobControllerDetails->fFadeInTime = fFadeInTime;
	pobControllerDetails->eMovementMode = eMovementMode;

	// Push it on to the front of our stack of stuff
	m_obChainedControllers.push_front( pobControllerDetails );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetCompletionMessage
*
*	DESCRIPTION		Want a feedback when the current controller ( +chain ) is complete?
*					Send a message to this or another entity with this.
*
***************************************************************************************************/
void CMovement::SetCompletionMessage( const char* pcCompletionCallbackMessage, const CEntity* pobCompletionMessageEntity )
{
	Message msg(eUndefinedMsgID);
	msg.SetString(CHashedString(HASH_STRING_MSG), CHashedString(pcCompletionCallbackMessage));
	SetCompletionMessage(msg, pobCompletionMessageEntity);
}

void CMovement::SetCompletionMessage( Message& msgCompletion, const CEntity* pobCompletionMessageEntity )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Make sure we have an entity pointer
	if ( !pobCompletionMessageEntity )
		pobCompletionMessageEntity = m_pobParentEntity;

	// Add the message to our completion object
	m_obCompletionFeedback.AddMessageFeedback( msgCompletion, pobCompletionMessageEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetCompletionCallback
*

*	DESCRIPTION		Want a feedback when the current controller ( +chain ) is complete?
*					Get a callback with this - that takes an enity pointer.
*
***************************************************************************************************/
void CMovement::SetCompletionCallback( void ( *CompletionCallback )( CEntity* ), CEntity* pobCompletionCallbackEntity )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Make sure we have an entity pointer
	if ( !pobCompletionCallbackEntity )
		pobCompletionCallbackEntity = m_pobParentEntity;

	// Add the callback to our completion object
	m_obCompletionFeedback.AddCallbackFeedback( CompletionCallback, pobCompletionCallbackEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetInterruptMessage
*
*	DESCRIPTION		Want a feedback when the current controller ( +chain ) is interrupted?
*					Send a message to this or another entity with this.
*
***************************************************************************************************/
void CMovement::SetInterruptMessage( const char* pcInteruptCallbackMessage, const CEntity* pobInterruptMessageEntity )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Make sure we have an entity pointer
	if ( !pobInterruptMessageEntity )
		pobInterruptMessageEntity = m_pobParentEntity;

	// Add the message to our interrupt object
	Message msg(eUndefinedMsgID);
	msg.SetString(CHashedString(HASH_STRING_MSG), CHashedString(pcInteruptCallbackMessage));
	m_obInteruptFeedback.AddMessageFeedback( msg, pobInterruptMessageEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetInterruptCallback
*
*	DESCRIPTION		Want a feedback when the current controller ( +chain ) is interrupted?
*					Get a callback with this - that takes an enity pointer.
*
***************************************************************************************************/
void CMovement::SetInterruptCallback( void ( *InterruptCallback )( CEntity* ), CEntity* pobInterruptCallbackEntity )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Make sure we have an entity pointer
	if ( !pobInterruptCallbackEntity )
		pobInterruptCallbackEntity = m_pobParentEntity;

	// Add the callback to our interrupt object
	m_obInteruptFeedback.AddCallbackFeedback( InterruptCallback, pobInterruptCallbackEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetCompletionPhysicsControl
*
*	DESCRIPTION		A helper to terminate the movement nicely
*
***************************************************************************************************/
void CMovement::SetCompletionPhysicsControl( void )
{
	// We can't do anything if disabled
	if ( !m_bEnabled ) 
		return;

	// Set the current physics mode to soft relative 
	SetMovementDynamicsMode( DMM_SOFT_RELATIVE );

	// If we have chained controllers set those to soft relative too
	ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>::iterator obEndIt = m_obChainedControllers.end();
	for ( ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>::iterator obIt = m_obChainedControllers.begin(); obIt != obEndIt; ++obIt )
		( *obIt )->eMovementMode = DMM_SOFT_RELATIVE;

	// If this is a character we need to activate controlled ragdoll at this point
	m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
		Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
		Physics::System::RAGDOLL_DEAD_ON_CONTACT );

	// Set a completion callback on this entity with our own physics function
	m_obCompletionFeedback.AddCallbackFeedback( SetToPhysicsControl, m_pobParentEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::IsPlayingPartialAnim
*
*	DESCRIPTION		Checks if a movement controller is playing a partial anim already
*
***************************************************************************************************/
bool CMovement::IsPlayingPartialAnim()
{
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::PlayPartialAnim
*
*	DESCRIPTION		Plays a partial anim on the component so that it goes on regardless of which
*					movement controllers are active.
*
***************************************************************************************************/
void CMovement::PlayPartialAnim( const CHashedString& pcPartialAnimName, float fFadeIn, float fFadeOut )
{
	ntAssert( fFadeIn >= 0.0f );
	ntAssert( fFadeOut >= 0.0f );

	if ( ntStr::IsNull( pcPartialAnimName ) )
	{
		return;
	}

	// Check we aren't playing one already
	if ( !(!m_pobPartialAnim) )
	{
		return;
	}

	// Store fade times
	m_fPartialAnimFadeIn = fFadeIn;
	m_fPartialAnimFadeOut = fFadeOut;

	m_pobPartialAnim = m_pobAnimator->CreateAnimation( pcPartialAnimName );
	ntAssert( m_pobPartialAnim );
	m_pobPartialAnim->SetFlags( ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_pobPartialAnim->SetBlendWeight( 0.0f );

    m_pobAnimator->AddAnimation( m_pobPartialAnim );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::UpdatePartialAnim
*
*	DESCRIPTION		Updates the high-level partial anim on the component.
*
***************************************************************************************************/
void CMovement::UpdatePartialAnim( float fTimeStep )
{
	UNUSED( fTimeStep );

	if ( m_pobPartialAnim )
	{
		float fAnimTime = m_pobPartialAnim->GetTime();
		float fAnimDuration = m_pobPartialAnim->GetDuration();

		if ( fAnimTime >= fAnimDuration )
		{
			// Animation has finished.  Remove it
			m_pobPartialAnim->SetBlendWeight( 0.0f );

			m_pobAnimator->RemoveAnimation( m_pobPartialAnim );

			m_pobPartialAnim = CAnimationPtr( 0 );
		}
		else
		{
			// Currently playing
			if ( fAnimTime < m_fPartialAnimFadeIn )
			{
				// FADE IN
				if ( m_fPartialAnimFadeIn <= 0.0f )
				{
					m_pobPartialAnim->SetBlendWeight( 1.0f );
				}
				else
				{
					float fNewBlend = ( fAnimTime / m_fPartialAnimFadeIn );
					fNewBlend = CMaths::SmoothStep( fNewBlend );
					m_pobPartialAnim->SetBlendWeight( fNewBlend );
				}
			}
			else if ( fAnimTime > ( fAnimDuration - m_fPartialAnimFadeOut ) )
			{
				// FADE OUT
				if ( m_fPartialAnimFadeOut <= 0.0f )
				{
					m_pobPartialAnim->SetBlendWeight( 0.0f );
				}
				else
				{
					float fNewBlend = ( ( fAnimDuration - fAnimTime ) / m_fPartialAnimFadeOut );
					fNewBlend = CMaths::SmoothStep( fNewBlend );
					m_pobPartialAnim->SetBlendWeight( fNewBlend );
				}
			}
			else
			{
				// MIDDLE
				m_pobPartialAnim->SetBlendWeight( 1.0f );
			}
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::SetCompletionPhysicsControl
*
*	DESCRIPTION		A callback function to deal with moving to full physics control after a set 
*					of movement.
*
***************************************************************************************************/
void CMovement::SetToPhysicsControl( CEntity* pobEntity )
{
	// If this is a character we can turn the ragdoll on now
	pobEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
		Physics::System::RAGDOLL_DEAD,
		Physics::System::RAGDOLL_IGNORE_CONTACT );
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::GiveControllerFeedback
*
*	DESCRIPTION		If the current controller is completed do we have a callback to make.
*
***************************************************************************************************/
void CMovement::GiveControllerFeedback( void )
{
	// If we have an item to chain add it to our stack
	if ( m_obChainedControllers.empty() )
	{
		// Mark the movement as completed
		m_bControllerCompleted = true;

		// Give feedback for completion
		m_obCompletionFeedback.GiveFeedback();
		
		// Clear out the interupt feedback
		m_obInteruptFeedback.Clear();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::UseChainedControllers
*
*	DESCRIPTION		If the current controller is completed do we have some controllers to chain
*
***************************************************************************************************/
void CMovement::UseChainedControllers( void )
{
	// If we have an item to chain add it to our stack
	if ( !m_obChainedControllers.empty() )
	{
		// Take the controller at the 'bottom' of the chain and push it onto the used stack
		CChainedControllerDetails* pobController = m_obChainedControllers.back();
		m_obChainedControllers.pop_back();

		// Push it on to the front of our stack of stuff
		//ntPrintf("Chaining in a new controller\n");
		InternalBringInNewController( *pobController->pobControllerDef, pobController->eMovementMode, pobController->fFadeInTime );

		// Clean up
		NT_DELETE_CHUNK(Mem::MC_ENTITY, pobController->pobControllerDef );
		NT_DELETE_CHUNK(Mem::MC_ENTITY, pobController );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CMovement::UpdateControllers
*
*	DESCRIPTION		Not written to be optimal - just clear whilst i am developing it.
*
***************************************************************************************************/
void CMovement::UpdateControllerWeights( float fTimeStep )
{
	// This it may be good to set this per character in the future, or
	// maybe make it dynamic
	static const int iMaxControllers = 5;

	// Loop through all the controllers to set their weight details
	ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obEnd = m_obControllers.end();
	for( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obIt = m_obControllers.begin(); obIt != obEnd; ++obIt )
	{
		// Get a direct pointer - just for clarity
		CControllerDetails* pobController = ( *obIt );

		// Crank up the time that the controller has been added
		pobController->fTimeSoFar += fTimeStep;

		// Update the weight of the controller
		pobController->fLocalWeight = pobController->fTimeSoFar / pobController->fFadeInTime;
		
		// If the blend weight is greater than 1
		if ( pobController->fLocalWeight >= 1.0f )
		{
			// Cap the controller weight
			pobController->fLocalWeight = 1.0f;

			// Loop through and destroy all the actual controllers
			for( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obSubIt = ++obIt; obSubIt != obEnd; ++obSubIt )
			{
				NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obSubIt )->pobController );
				NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obSubIt ) );
			}

			// Remove all following controllers
			m_obControllers.erase( obIt, obEnd );

			// Drop out here
			break;
		}
	}

	// A simple cull of the extra controllers here - we may need a good algorythm to smooth out
	// the weights when we are culling large numbers of controllers
	if ( (int)m_obControllers.size() > iMaxControllers )
	{
		int iControllerNumber = 0;
		obEnd = m_obControllers.end();
		for ( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obIt = m_obControllers.begin(); obIt != obEnd; ++obIt )
		{
			// Count the controllers
			iControllerNumber++;

			// When we reach the max kill the rest
			if ( iControllerNumber == iMaxControllers )
			{
				// Loop through and destroy all the actual controllers
				for( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obSubIt = obIt; obSubIt != obEnd; ++obSubIt )
				{
					NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obSubIt )->pobController );
					NT_DELETE_CHUNK(Mem::MC_ENTITY, ( *obSubIt ) );
				}

				// Remove all following controllers
				m_obControllers.erase( obIt, obEnd );

				// Drop out here
				break;
			}
		}
	}

	// Now loop through the remaining controllers setting relative weights
	float fRemainingWeight = 1.0f;
	obEnd = m_obControllers.end();
	for( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>::iterator obIt = m_obControllers.begin(); obIt != obEnd; ++obIt )
	{
		// Get a direct pointer - just for clarity
		CControllerDetails* pobController = ( *obIt );

		// Set the real weight of the controller
		pobController->fGlobalWeight = ( fRemainingWeight * pobController->fLocalWeight );
		
		// Pass the value through to the controller
		pobController->pobController->SetTotalWeight( pobController->fGlobalWeight );

		// Reduce the relative weight to that remaining after this controller
		fRemainingWeight -= pobController->fGlobalWeight;
	}
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::ControllerFeedback
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CMovement::ControllerFeedback::ControllerFeedback( void )
:	m_obMessages(),
	m_obCallbacks()
{
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::~ControllerFeedback
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CMovement::ControllerFeedback::~ControllerFeedback( void )
{
	// Reset the details
	Reset();
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::AddCallbackFeedback
*
*	DESCRIPTION		Add feedback in the form of a callback function
*
***************************************************************************************************/
void CMovement::ControllerFeedback::AddCallbackFeedback( void ( *function )( CEntity* ), CEntity* pobEntity )
{		
	// Make sure that we have a valid entity pointer
	if ( pobEntity && pobEntity->GetMessageHandler() )
	{
		//  Create our structure
		FeedbackCallback obFeedback;
		obFeedback.Callback = function;
		obFeedback.pobEntity = pobEntity;

		// Stick it in our list
		m_obCallbacks.push_back( obFeedback );
	}
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::AddMessageFeedback
*
*	DESCRIPTION		Add feedback in the form of a message
*
***************************************************************************************************/
void CMovement::ControllerFeedback::AddMessageFeedback( Message& msg, const CEntity* pobEntity )
{
	// Make sure we have a valid message and entity pointer
	if(pobEntity && pobEntity->GetMessageHandler())
	{
		//  Create our structure
		FeedbackMessage obFeedback;
		obFeedback.obMessage = msg;
		obFeedback.pobEntity = pobEntity;

		// Stick it in our list
		m_obMessages.push_back( obFeedback );
	}
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::GiveFeedback
*
*	DESCRIPTION		Give the feedback details
*
***************************************************************************************************/
void CMovement::ControllerFeedback::GiveFeedback( void )
{
	// We'll need this
	using namespace ntstd;
		
	// Loop through our messages
	List<FeedbackMessage>::iterator obEndMessageIt = m_obMessages.end();
	for ( List<FeedbackMessage>::iterator obIt = m_obMessages.begin(); obIt != obEndMessageIt; ++obIt )
	{
		( *obIt ).pobEntity->GetMessageHandler()->QueueMessage(( *obIt ).obMessage);
	}

	// Loop through out callbacks
	List<FeedbackCallback>::iterator obEndCallbackIt = m_obCallbacks.end();
	for ( List<FeedbackCallback>::iterator obIt = m_obCallbacks.begin(); obIt != obEndCallbackIt; ++obIt )
	{
		// Some callbacks might empty us out, so need to do this sanity check because 
		// the termination condition of this loop isn't sufficient in this case
		if (m_obCallbacks.size() == 0)
		{
			break;
		}

		( *obIt ).Callback( ( *obIt ).pobEntity );
	}

	// Reset the details
	Reset();
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::Clear
*
*	DESCRIPTION		Clear the feedback details
*
***************************************************************************************************/
void CMovement::ControllerFeedback::Clear( void )
{
	// Put some debug information in here

	// Reset the details
	Reset();
}


/***************************************************************************************************
*
*	FUNCTION		ControllerFeedback::Reset
*
*	DESCRIPTION		Reset the feedback details
*
***************************************************************************************************/
void CMovement::ControllerFeedback::Reset( void )
{
	// Clear out our lists if we have any details
	m_obMessages.clear();
	m_obCallbacks.clear();
}


