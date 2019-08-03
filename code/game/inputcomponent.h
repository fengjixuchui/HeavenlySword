/***************************************************************************************************
*
*	DESCRIPTION		Handles input collection for driving entity movement modules.
*
*	NOTES
*
***************************************************************************************************/

#ifndef _INPUT_COMP_H
#define _INPUT_COMP_H

// Necessary includes
#include "input/inputhardware.h"
#include "editable/enumlist.h"
#include "lua/ninjalua.h"

#ifndef _RELEASE
#include "input/inputsequence.h"
#include "core/nt_std.h"
#endif

// Forward declarations
class CVKeyManager;
class CEntity;
class CamTransition;
class CameraInterface;
class CAttackLink;


/***************************************************************************************************
*
*	CLASS			CProcessedInput
*
*	DESCRIPTION		These button u_ints mirror whats inside a real pad
*					InputDir and Speed are the processed analogue LStick Inputs.
*					This whole struct may be faked up by an AI for combat purposes
*
***************************************************************************************************/
struct CProcessedInput
{
	// Construction
	CProcessedInput( void )	{ Reset(); }

	// Clear the members
	void Reset( void );

	// The primary analogue stick - a normalised direction and a speed in the range 0.0f to 1.0f
	CDirection	m_obInputDir;	
	float		m_fSpeed;			

	// Similar details for the secondary analogue stick
	CDirection	m_obInputDirAlt;	
	float		m_fSpeedAlt;		

	// Always copied from the input hardware pad itself.
	u_int		m_uiHeld;
	u_int		m_uiPressed;
	u_int		m_uiReleased;

	u_int		m_uiVPressed;
	u_int		m_uiVHeld;
	u_int		m_uiVReleased;

	// Monitoring of pad wiggling
	bool		m_bWiggled;
	int			m_iWiggleSide;
};


/***************************************************************************************************
*
*	CLASS			CInputComponent
*
*	DESCRIPTION		Entity component that motitors pad input or acts as an interface between the AI
*					and the movement component (The movement takes input from this module).
*
*	NOTES			Should we interpolate our worldspace input direction over camera transitions,
*					this is probably the correct place to do it.
*
***************************************************************************************************/
class CInputComponent
{
public:

	// Construction destruction
	CInputComponent( CEntity* pobParent, PAD_NUMBER ePad );
	~CInputComponent( void );

	HAS_LUA_INTERFACE()

	// The main update
	void Update( float fTimeChange );

	// Get Processed input - the primary stick
	CDirection	GetInputDir( void )		const { return m_obInput.m_obInputDir; }
	float		GetInputSpeed( void )	const { return m_obInput.m_fSpeed; }

	// ...the secondary analogue stick
	CDirection	GetInputDirAlt( void )		const { return m_obInput.m_obInputDirAlt; }
	float		GetInputSpeedAlt( void )	const { return m_obInput.m_fSpeedAlt; }

	// A central place to ask if we consider a certain input speed a direction
	bool IsDirectionHeld( void ) const { return ( GetInputSpeed() > 0.3f ); }

	// Get direct pad button information - SHOULD BE USED FR DEBUG FUNCTIONALITY ONLY
	u_int GetHeld( void )		const { return m_obInput.m_uiHeld;}
	u_int GetPressed( void )	const { return m_obInput.m_uiPressed;}
	u_int GetReleased( void )	const { return m_obInput.m_uiReleased;}

	// Get virtual pad button information
	u_int GetVPressed( void )	const { return m_obInput.m_uiVPressed;}
	u_int GetVHeld( void )		const { return m_obInput.m_uiVHeld;}
	u_int GetVReleased( void )	const { return m_obInput.m_uiVReleased;}

	// How long has a virtual button been pressed for
	float GetVHeldTime( VIRTUAL_BUTTON_TYPE eButton ) const;

	// Set a rumble to occur on this pad for a single frame
	void SetRumble( bool bRumble ) { m_bRumble = bRumble; }

	// See if a button has been pressed in the last frame - this includes 'wiggling'
	bool GetGeneralButtonPressed( void ) const { return ( ( m_obInput.m_bWiggled ) || ( m_obInput.m_uiVPressed != 0 ) ); }

	// Motion sensor functions
	bool IsMotionSensor( void ) const;
	float GetSensorRawMag( PAD_SENSOR_ID eSensor ) const;
	float GetSensorFilteredMag( PAD_SENSOR_ID eSensor, PAD_SENSOR_FILTER eFilter ) const;

	// Re-initialise statics
	static void Reset( void );

	// Switch the component on and off
	void SetDisabled( bool bDisabled );
	bool GetDisabled( void ) const { return m_bExternallyDisabled; }

	// What pad are we?
	PAD_NUMBER GetPadNumber() {return m_ePadNumber;}

	// Set this pad to give completely random output
	void SetRandomOutputEnabled( bool bRandomOutput ) { m_bRandomOutput = bRandomOutput; };

	int GetVButtonMashesInTheLastSecond( VIRTUAL_BUTTON_TYPE eButton ) const { return m_aobButtonMashTimes[eButton].size(); };
	// set the current camera transition so we can blend camera relative movement over transitions.
//	void SetCameraTransition( const CamTransition* pCamTransition ) { m_pCurrentTransition = pCamTransition; }

	// handles implicit transition - for when a transition happens between 2 cameras where no transition object has been setup.
	// Blends camera relative motion over a default timeframe.
	void HandleImplicitTransition( const CameraInterface* pobCamInterface );

	// Cut down transition like effect - holds the input direction until it deviates by a set amount.
	// Used by boss cameras when there is an internal camera cut.
	void HoldInputDirection( const CDirection& obInitialZDir, const CameraInterface* pobCamInterface );

	typedef ntstd::List<const CAttackLink*> SequenceListType;

#ifndef _RELEASE
	void PlayMove( const CAttackLink* obMove );
	void PlaySequence( const CAttackLink* obSequence );
	void StopSequence();
	void DebugOutputButtons( u_int uiButtonState, const ntstd::String& obLabel, float fPosX, float fPosY ) const;
#endif

private:

	// Geometry helpers
	static float GetGlobalYAngle( const CDirection& obInputDirection );
	static float NormaliseAngle( float fAngle );

	// Helper for dealing with the time for how long virtual buttons have been held
	void ClearVHeldTimes( void );
	void SetVHeldTimes( float fTimeChange );

	// A helper function for checking if the pad is being wiggled
	void CheckWiggle( void );

	// Can we make the game crash - we can with this
	void DebugNinaSimulator( void );

	// A helper to send button events out
	void SendButtonMessages( void ) const;

	// Peforms camera relative motion blending during a transition
	void DoMotionBlend( const CameraInterface* pDestCamera, float fCurrentTransTime, float fMaxTransTime,
						CDirection& obCamRelativeMovement, CDirection& obCamRelativeMovementAlt, CMatrix& obFlattenRotation );

	// Does the same as the above function for cuts during boss camera usage, will eventually
	// be merged with the function above - kept separate to prevent breakage.
	void DoMotionBlend2( const CDirection& obInitialZDir, const CameraInterface* pDestCamera, float fCurrentTransTime, 
						 float fMaxTransTime, CDirection& obCamRelativeMovement, CDirection& obCamRelativeMovementAlt, 
						 CMatrix& obFlattenRotation );


	// this allows us to control seperate characters with the same pad...
	static int		m_aiNumInputComponents[PAD_NUM];

	// What pad number have we been assigned
	PAD_NUMBER m_ePadNumber;

	// A structure to hold the current state of the buttons
	CProcessedInput m_obInput;

	// Our parent entity
	CEntity* m_pobParent;

	// To look up our virtual button translations
	CVKeyManager* m_pobVKeyManager;

	// Have we been turned off?
	bool m_bExternallyDisabled;

	// Rumble values for a single frame
	bool m_bRumble; 

	// An array of times for which the virtual buttons have been held
	float m_afVButtonHeldTimes[AB_NUM];

	// Button mash counting
	ntstd::List<float> m_aobButtonMashTimes[AB_NUM];
	float m_fButtonMashStartReferenceTime;

	// Enable our button basher?
	bool m_bRandomOutput;

	// Button mash counting
	void UpdateButtonMashPerSecondCount( float fTimeChange );
	// camera transition - for blending camera relative movement over transitions
	//const CamTransition* m_pCurrentTransition;
	bool m_bTransitionStart;
	CDirection m_obSourceCamDir;
	CDirection m_obTargetCamDir;

	CDirection m_obPrevMainDir;
	CDirection m_obCachedInputDir;

#ifndef _RELEASE
	CInputSequence m_obInputSequence;
	bool IsAutoLinkTransition(const CAttackLink* pPreviousLink, const CAttackLink* pCurrentLink) const;
	ATTACK_MOVE_TYPE CalculateButtonType(const CAttackLink* pPreviousLink, const CAttackLink* pCurrentLink) const;
	float CalculateButtonTime(const CAttackLink* pPreviousLink, const ATTACK_MOVE_TYPE eButtonType) const;
	float CalculateButtonDuration(const CAttackLink* pPreviousLink, const SequenceListType& obNextLinks) const;
#endif

	bool m_bImplicitTransInProgress;
	float m_fImplicitTransTime;
	const float m_fImplicitTransMaxTime;

	const CameraInterface* m_pDestCam;
	bool m_bControlTransitionCancelled;
	CDirection m_obStartingPadDirection;

	bool m_bHoldInputDirection;
	bool m_bStoreHoldDirection;
};

LV_DECLARE_USERDATA(CInputComponent);

#endif // _INPUT_COMP_H#
