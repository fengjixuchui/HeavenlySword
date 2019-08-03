/***************************************************************************************************
*
*       DESCRIPTION	
*
*       NOTES          
*
***************************************************************************************************/

#ifndef	_MOVEMENT_H
#define	_MOVEMENT_H

// Forward declarations
class CAnimator;
class CInputComponent;
class MovementControllerDef;
class MovementController;
class CEntity;

namespace Physics {
	class System;
}

// Necessary includes
#include "anim/animation.h"
#include "movementstate.h"
#include "game/movement_lua.h"
#include "game/message.h"

/***************************************************************************************************
*	
*	CLASS			CMovementInput
*
*	DESCRIPTION		Replaces the old 'Pad Info'.  This should actually be anything to do with the
*					the input pad - this has to be used by the AI and stuff - we should never really
*					refer to 'pad' within the movement system because that will lead us to write
*					code that is human player centric - we need this stuff to be reusable.
*
***************************************************************************************************/
class CMovementInput
{
public:

	// Construction
	CMovementInput( void );

	// Clear the input - called at the end of a movement update
	void ClearMovementInput( void );

	// Our members
	float		m_fMoveSpeed;			// Will be a 0-1 value
	CDirection	m_obMoveDirection;		// The direction to move in
	float		m_fMoveSpeedAlt;		// Alternative move speed - uses the right hand analogue for human control
	CDirection	m_obMoveDirectionAlt;	// Alternative move direction - uses the right hand analogue for human control
	CDirection	m_obFacingDirection;	// The direction to look in
	CPoint		m_obTargetPoint;		// The point to aim for (targeting etc)
	bool		m_bTargetPointSet;		// Has a target been set this frame

	// For AI controlled cannons

	float		m_fYaw;
	float		m_fPitch;
	CEntity*	m_pAIUser;

	// This should be all we need for controller input.  If complex controllers need
	// to monitor the history of this then they can do it themselves - lets not let
	// this class fill up with the obscurities of CMovementPadInfo
};


/***************************************************************************************************
*	
*	CLASS			CMovement
*
*	DESCRIPTION		This is a component that is used to translate pad input into how an entity moves
*					through the environment
*
***************************************************************************************************/
class CMovement : public Movement_Lua
{
public:

	static const float COMBAT_BLEND;
	static const float MOVEMENT_BLEND;

	// We can have different movement modes with regards to the physics representation - when
	// a new controller is added the required dynamics mode will need to be indicated
	enum DYNAMICS_MOVEMENT_MODE
	{
		DMM_STANDARD,
		DMM_HARD_RELATIVE,
		DMM_SOFT_RELATIVE
	};

	CMovement(	CEntity*					pobParentEntity,
				CAnimator*					pobAnimator,
				Physics::System*			pobSystem);

	// Destruction
	~CMovement();

	// This is the update to be called before the animator is updated...
	void Update( float fTimeStep );

	//DGF
	int GetNumberOfActiveControllers() { return m_obControllers.size(); };

	// So physics can get access to an entities last velocities etc.
	const CMovementStateRef& GetCurrentMovementState() { return m_obCurrentMovementState; }

	// Our movement input - to be exposed properly soon
	CMovementInput m_obMovementInput;

	// ...and this one is to be called afterwards
	void UpdatePostAnimator(float fTimeStep);

	// This completely disables the whole component - no updates or new controllers
	void SetEnabled( bool bEnabled );
	bool IsEnabled()	{ return m_bEnabled; }

	// This allows the component to be updated but disallows new controllers
	void BlockNewControllers( bool bBlock ) { m_bBlocked = bBlock; }
	bool IsBlocked( void ) const { return m_bBlocked; }

	// This kills any input to the movement controllers
	void SetInputDisabled( bool bInputDisabled ) { m_bInputDisabled = bInputDisabled; }
	bool IsInputDisabled( void ) const { return m_bInputDisabled; }

	// Return the a pointer to our our owning entity
	const CEntity* GetParentEntity( void ) const { return m_pobParentEntity; }

	// Drop all the movement controllers on this component
	void ClearControllers( void );

	// Was controller requested by an AI character?
	void SetAIRequestedController( bool bAIRequested ) { m_bAIRequested = bAIRequested; }
	bool IsAIRequestedController( void ) const	{ return m_bAIRequested; }

	// Add a controller for immediate usage - clears chained controllers and callbacks
	//bool BringInNewControllerFL(const char* pcFile, int iLine, const MovementControllerDef& obControllerDef, DYNAMICS_MOVEMENT_MODE eMovementMode = DMM_STANDARD, float fFadeInTime = DEFAULT_CONTROLLER_BLEND);
	bool BringInNewController(const MovementControllerDef& obControllerDef, DYNAMICS_MOVEMENT_MODE eMovementMode, float fFadeInTime);

	// Add a controller to be used when the current one completes - push on whole chains
	void AddChainedController( const MovementControllerDef& obControllerDef, DYNAMICS_MOVEMENT_MODE eMovementMode, float fFadeInTime );

	// Want a feedback when the current controller ( +chain ) is complete?
	void SetCompletionMessage( const char* pcCompletionCallbackMessage, const CEntity* pobCompletionMessageEntity = 0 );
	void SetCompletionMessage( Message& msgCompletion, const CEntity* pobCompletionMessageEntity = 0 );
	void SetCompletionCallback( void ( *CompletionCallback )( CEntity* ), CEntity* pobCompletionCallbackEntity = 0 );

	// Want a feedback when the current controller ( +chain ) is interrupted?
	void SetInterruptMessage( const char* pcInteruptCallbackMessage, const CEntity* pobInterruptMessageEntity = 0 );
	void SetInterruptCallback( void ( *InterruptCallback )( CEntity* ), CEntity* pobInterruptCallbackEntity = 0 );

	// A helper to terminate the movement nicely
	void SetCompletionPhysicsControl( void );

	// Partial anims (cheering, jeering etc.)
	bool IsPlayingPartialAnim();
	void PlayPartialAnim( const CHashedString& pcPartialAnimName, float fFadeIn = MOVEMENT_BLEND, float fFadeOut = MOVEMENT_BLEND );

	// Access components relevant to the movment system ( used by controllers to manipulate bits of an entity )
	CAnimator* GetAnimatorP() { return m_pobAnimator; }

	// Access to the dynamics representation - used by internal controllers
	Physics::System* GetPhysicsSystem( void ) const { return m_pobSystem; }

	// Used by the NS Manager - should really use the new callback system
	bool HasMovementCompleted( void ) const { return m_bControllerCompleted; }

	// Return the number of controllers pushed on by this entity.... Can be used to keep
	// track of the controller
	uint32_t GetNewControllerCount(void) {return m_uiNewControllerCount; }

#ifndef _RELEASE

	// Debug only code for movement - one function to stop looping for each bit of data
	void GetControllerDebugDetails(	int				iControllerNumber, 
									const char*&	pcInstanceName,
									const char*&	pcTypeName,
									float&			fWeight ) const;
#endif

	// Clears the interrupt feedback for the active controller
	void ClearInterruptFeedback(void) { m_obInteruptFeedback.Clear(); }

	// Clears the completion feedback for the active controller
	void ClearCompletionFeedback(void) { m_obCompletionFeedback.Clear(); }

private:

	//------------------------------------------------------------------------------------------
	// High-level partial anim stuff (one-off animations for cheering, jeering...)
	CAnimationPtr	m_pobPartialAnim;
	float			m_fPartialAnimFadeIn;
	float			m_fPartialAnimFadeOut;
	
	void UpdatePartialAnim( float fTimeStep );

	// A callback function to deal with moving to full physics control after a set of movement
	static void SetToPhysicsControl( CEntity* pobEntity );

	// We need to make some dynamics alterations when we are using relative movement controllers.  We
	// change the dynamics mode here
	bool SetMovementDynamicsMode( DYNAMICS_MOVEMENT_MODE eMovementMode );

	// Add a controller for immediate usage
	void InternalBringInNewController( const MovementControllerDef& obControllerDef, DYNAMICS_MOVEMENT_MODE	eMovementMode, float fFadeInTime );

	// Check the input values with the physics representation
	void PhysicsTweakInput( void );

	// A sub class to manage the details of our current controllers
	class CControllerDetails
	{
	public:
		CControllerDetails( void ) 
			:	pobController( 0 ), 
				fFadeInTime( 0.0f ), 
				fTimeSoFar( 0.0f ), 
				fLocalWeight( 0.0f ), 
				fGlobalWeight( 0.0f ),
				eMovementMode( DMM_STANDARD ) {}

		MovementController*		pobController;
		float					fFadeInTime;
		float					fTimeSoFar;
		float					fLocalWeight;
		float					fGlobalWeight;
		DYNAMICS_MOVEMENT_MODE	eMovementMode;
	};

	// A sub class to manage the details of our chained controllers
	class CChainedControllerDetails
	{
	public:
		CChainedControllerDetails( void ) 
			:	pobControllerDef( 0 ),
				fFadeInTime( 0.0f ), 
				eMovementMode( DMM_STANDARD ) {}

		MovementControllerDef*	pobControllerDef;
		float					fFadeInTime;
		DYNAMICS_MOVEMENT_MODE	eMovementMode;
	};

	// A sub class to manage the feedback system
	class ControllerFeedback
	{
	public:

		// Construction Destruction
		ControllerFeedback( void );
		~ControllerFeedback( void );

		// Build up feedback details
		void AddCallbackFeedback( void ( *function )( CEntity* ), CEntity* pobEntity );
		void AddMessageFeedback( Message& msg, const CEntity* pobEntity );

		// Give the feedback details
		void GiveFeedback( void );

		// Clear the current feedback details
		void Clear( void );

	private:

		// Reset the feedback details
		void Reset( void );

		// The information required for a feedback message
		struct FeedbackMessage
		{
			FeedbackMessage() : obMessage(eUndefinedMsgID) {;}

			// ALEXEY_TODO
			Message		    obMessage;
			const CEntity*	pobEntity;
		};

		// The information required for a feedback callback
		struct FeedbackCallback
		{
			void (*Callback)( CEntity* );
			CEntity* pobEntity;
		};

		// The lists of items that make uk the feedback system
		ntstd::List<FeedbackMessage>	m_obMessages;
		ntstd::List<FeedbackCallback>	m_obCallbacks;
	};

	// A function called at the beginning of an update - finds the current movement state from dynamics
	void SetCurrentAndPredictedMovementStates( float fTimeStep );

	// A helper used to control the blending of movement controllers
	void UpdateControllerWeights( float fTimeStep );

	// Helpers to deal with the completion of the current controller - deals with chains and callbacks
	void UseChainedControllers( void );
	void GiveControllerFeedback( void );

	// Clear out a list of controller details
	void ClearControllerList( ntstd::List<CControllerDetails*, Mem::MC_ENTITY>& obControllerList );
	void ClearChainedControllerList( ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY>& obControllerList );

	// Set the general input data required for the components
	void SetInputData( void );

	// Pointer to the info block of our parent
	CEntity* m_pobParentEntity;
	
	// Pointer to the animator that all the controllers may act on
	CAnimator* m_pobAnimator;

	// A pointer to this entity's physical representation
	Physics::System* m_pobSystem;

	// The data that is passed into all controller updates - needs to change a bit
	CMovementStateRef		m_obCurrentMovementState;
	CMovementState			m_obPredictedMovementState;

	// Has the current movement completed?
	bool	m_bControllerCompleted;

	// The list of active controllers
	ntstd::List<CControllerDetails*, Mem::MC_ENTITY> m_obControllers; 

	// The list of chained controllers
	ntstd::List<CChainedControllerDetails*, Mem::MC_ENTITY> m_obChainedControllers;

	// Feedback mechanisms for other systems
	ControllerFeedback m_obInteruptFeedback;
	ControllerFeedback m_obCompletionFeedback;

	// What is our current movement mode with regards the dynamics representation
	DYNAMICS_MOVEMENT_MODE m_eMovementMode;

#	ifndef _RELEASE
		int32_t	m_NumControllersAddedThisFrame;
#	endif

	// Track whether or not the orientation of the character has changed - this is necessary
	// because of the inaccuracy of contantly converting between quats and matricies - this
	// needs to be looked at in the future - maybe stop using quats in this system.
	bool m_bOrientationChanged;

	// This completely disables the whole component - no updates or new controllers
	bool m_bEnabled;

	// This allows the component to be updated but disallows new controllers
	bool m_bBlocked;

	// This kills any input to the movement controllers
	bool m_bInputDisabled;

	// Has the current movement controller finished
	bool m_bControllerFinished;

	// Count of component disables - to correctly clean up layered controllers
	uint32_t m_iEnabledCount;

	// Was controller requested by an AI character?
	bool m_bAIRequested;

	// Has the update been called? (at least once)
	bool m_bBeenUpdated;

	// Counts the number of times a new controller has been added. 
	uint32_t m_uiNewControllerCount;
};

LV_DECLARE_USERDATA(CMovement);

//#define BringInNewController(...) BringInNewControllerFL(__FILE__, __LINE__, __VA_ARGS__)

#endif	// _MOVEMENT_H

