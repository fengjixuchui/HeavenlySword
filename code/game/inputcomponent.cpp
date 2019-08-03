/***************************************************************************************************
*
*	DESCRIPTION		Handles input collection for driving entity movement modules.
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "game/inputcomponent.h"
#include "core/gatso.h"

#include "camera/camman_public.h"
#include "camera/camtransition.h"
#include "camera/camutils.h"
#include "camera/camerainterface.h"
#include "camera/camview.h"
#include "core/timer.h"
#include "game/entityinfo.h"
#include "game/aicomponent.h"
#include "game/vkey.h"
#include "game/messagehandler.h"

// For some debuggy stuff
#include "game/randmanager.h"
#include "core/visualdebugger.h"

#include "game/entitymanager.h"

// Initialise statics
int CInputComponent::m_aiNumInputComponents[PAD_NUM];

#include "luaexptypes.h"
#include "lua/ninjalua.h"

#ifndef _RELEASE
#include "game/attacks.h"
#include "game/comboinspector.h"
#include "input/inputsequence.h"
#endif

// NinjaLua bindings
LUA_EXPOSED_START(CInputComponent)
	LUA_EXPOSED_METHOD(IsDirectionHeld,	IsDirectionHeld, "", "", "")
	LUA_EXPOSED_METHOD(SetDisabled,     SetDisabled,     "Enable or disable the input component", "bool bDisable", "")
LUA_EXPOSED_END(CInputComponent)

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::Reset
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CProcessedInput::Reset( void )
{
	// Clear the primary sticke details
	m_obInputDir.Clear();
	m_fSpeed = 0.0f;

	// Clear the secondary stick details
	m_obInputDirAlt.Clear();
	m_fSpeedAlt = 0.0f;	

	// Clear the button information
	m_uiHeld = 0;
	m_uiPressed = 0;
	m_uiReleased = 0;

	// Clear the virtual button information
	m_uiVPressed = 0;
	m_uiVHeld = 0;
	m_uiVReleased = 0;

	// Clear the wiggling details
	m_bWiggled = false;
	m_iWiggleSide = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::CInputComponent
*
*	DESCRIPTION		constructor
*
***************************************************************************************************/
CInputComponent::CInputComponent(CEntity* pobParent, PAD_NUMBER ePad )
:	m_ePadNumber( ePad ),
	m_obInput(),
	m_pobParent( pobParent ),
	m_pobVKeyManager( 0 ),
	m_bExternallyDisabled( false ),
	m_bRumble( false ),
	m_afVButtonHeldTimes(),
	m_bRandomOutput( false ),
//	m_pCurrentTransition( 0 ),
	m_bTransitionStart(false),
	m_bImplicitTransInProgress(false),
	m_fImplicitTransTime(0.0f),
	m_fImplicitTransMaxTime(2.0f),
	m_pDestCam(0),
	m_bControlTransitionCancelled(false),
	m_bHoldInputDirection(false),
	m_bStoreHoldDirection(false)
{
	ATTACH_LUA_INTERFACE(CInputComponent);

	// Increment the number of pads with this id
	m_aiNumInputComponents[m_ePadNumber]++;

	// Create a new VKeyManager
	m_pobVKeyManager = NT_NEW CVKeyManager( CInputHardware::Get().GetPadP( m_ePadNumber ) );

	// Clear the button held times
	ClearVHeldTimes();
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::~CInputComponent
*
*	DESCRIPTION		destructor
*
***************************************************************************************************/
CInputComponent::~CInputComponent( void )
{
	// Decrement the number of pads with this id
	m_aiNumInputComponents[m_ePadNumber]--;

	// Clear the VKeyManager
	NT_DELETE( m_pobVKeyManager );
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::Reset
*
*	DESCRIPTION		static reset of static members
*
***************************************************************************************************/
void CInputComponent::Reset( void )
{
	for ( int i = 0; i < PAD_NUM; i++ )
	{
		m_aiNumInputComponents[i] = 0;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::ClearVHeldTimes
*
*	DESCRIPTION		Clear the button held times
*
***************************************************************************************************/
void CInputComponent::ClearVHeldTimes( void )
{
	for ( int iVKey = 0; iVKey < AB_NUM; ++iVKey )
		m_afVButtonHeldTimes[iVKey] = 0.0f;
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::SetVHeldTimes
*
*	DESCRIPTION		Store how long each virtual button has been pressed
*
***************************************************************************************************/
void CInputComponent::SetVHeldTimes( float fTimeChange )
{
	// Loop through all the virtual keys and save the values
	for ( int iVKey = 0; iVKey < AB_NUM; ++iVKey )
	{
		// If it's held add the time, otherwise reset it
		if ( m_obInput.m_uiVHeld & ( 1 << iVKey ) )
			m_afVButtonHeldTimes[iVKey] += fTimeChange;
		else
			m_afVButtonHeldTimes[iVKey] = 0.0f;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::UpdateButtonMashPerSecondCount
*
*	DESCRIPTION		Store how many times a button has been pressed in the past second in lists
*
***************************************************************************************************/
void CInputComponent::UpdateButtonMashPerSecondCount( float fTimeChange )
{
	if (m_fButtonMashStartReferenceTime != -1.0f)
		m_fButtonMashStartReferenceTime += fTimeChange;

	// Loop through all the virtual keys and save the values
	for ( int iVKey = 0; iVKey < AB_NUM; ++iVKey )
	{
		if (m_obInput.m_uiVPressed & ( 1 << iVKey ))
		{
			// Start reference timing?
			if (m_fButtonMashStartReferenceTime == -1.0f)
				m_fButtonMashStartReferenceTime = 0.0f;

			m_aobButtonMashTimes[iVKey].push_back(m_fButtonMashStartReferenceTime);
		}

		if (m_aobButtonMashTimes[iVKey].size() > 0)
		{
			// If the first entry in the list is longer than a second ago, lose it
			if ((*m_aobButtonMashTimes[iVKey].begin()) + 1.0f < m_fButtonMashStartReferenceTime)
			{
				m_aobButtonMashTimes[iVKey].pop_front();
			}
		}
	}

	// See if we can reset our reference time
	bool bCanReset = true;
	for ( int iVKey = 0; iVKey < AB_NUM; ++iVKey )
	{
		bCanReset &= m_aobButtonMashTimes[iVKey].size() == 0;
	}
	if (bCanReset)
		m_fButtonMashStartReferenceTime = -1.0f;

	// Debug spew
	/*float x = 10.0f;
	float y = 10.0f;
	for ( int iVKey = 0; iVKey < AB_NUM; ++iVKey )
	{
		g_VisualDebug->Printf2D(x,y,DC_RED,0,"%i",m_aobButtonMashTimes[iVKey].size());
		y += 15.0f;
	}*/
}

void CInputComponent::SetDisabled( bool bDisabled ) 
{ 
	m_bExternallyDisabled = bDisabled; 
	m_bControlTransitionCancelled = true;
//	m_pCurrentTransition = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::Update( float fTimeChange )
*
*	DESCRIPTION		update from pad (if AI, we've had our velocity set already)
*
***************************************************************************************************/
void CInputComponent::Update( float fTimeChange )
{
	// Don't do any work if we have been externally disabled

	// Should we read the input hardware we are associated with on this frame
	bool bReadMe = !m_bExternallyDisabled && (CInputHardware::Get().GetPadContext() == PAD_CONTEXT_GAME) ? true : false;

	// If we are should be looking at the hardware this frame...
	if(bReadMe)
	{
		// Find the 'speed' of the two analogue sticks
		m_obInput.m_fSpeed = CInputHardware::Get().GetPad(m_ePadNumber).GetAnalogLFrac();
		m_obInput.m_fSpeedAlt = CInputHardware::Get().GetPad(m_ePadNumber).GetAnalogRFrac();

		// If the camera is essentially upside down we need to flip our input in z
		float fZMultiply = (CamMan_Public::GetCurrMatrix().GetYAxis().Y() >= 0.0f) ? 1.0f : -1.0f;

		// Build a vector in camera space based on the pad input - taking up as positive Z 
		CDirection obInputRelativeToCamera( CONSTRUCT_CLEAR );
		obInputRelativeToCamera.X() = -fsinf( CInputHardware::Get().GetPad( m_ePadNumber ).GetAnalogLAngle() );
		obInputRelativeToCamera.Z() = fcosf( CInputHardware::Get().GetPad( m_ePadNumber ).GetAnalogLAngle() ) * fZMultiply;
		obInputRelativeToCamera.Normalise();

		// Do the same for the alternative analogue stick
		CDirection obInputRelativeToCameraAlt( CONSTRUCT_CLEAR );
		obInputRelativeToCameraAlt.X() = -fsinf( CInputHardware::Get().GetPad( m_ePadNumber ).GetAnalogRAngle() );
		obInputRelativeToCameraAlt.Z() = fcosf( CInputHardware::Get().GetPad( m_ePadNumber ).GetAnalogRAngle() ) * fZMultiply;
		obInputRelativeToCameraAlt.Normalise();
		
		// Get the z-axis of the camera in world space
		CDirection obWorldCameraZAxis( CamMan_Public::GetCurrMatrix().GetZAxis() );
		obWorldCameraZAxis.Normalise();

		// In the world x/z plane
		CDirection obWorldCameraZAxisXZ = obWorldCameraZAxis;
		obWorldCameraZAxisXZ.Y() = 0.0f;

		// Create a rotation matrix so to 'flatten' the input direction, otherwise we get a left-right weighting 
		// when the camera is looking up or down at a steep angle
		CMatrix obRotation( CQuat( CamMan_Public::GetCurrMatrix().GetZAxis(), obWorldCameraZAxisXZ ) );

		// Multiply by the camera matrix and the rotation matrix to get the world position
		m_obInput.m_obInputDir = obInputRelativeToCamera * CamMan_Public::GetCurrMatrix() * obRotation;
		m_obInput.m_obInputDirAlt = obInputRelativeToCameraAlt * CamMan_Public::GetCurrMatrix() * obRotation;

		// cache current input direction - used during camera transitions
		m_obPrevMainDir = m_obInput.m_obInputDir;

		// DEBUG RENDERING, SHOWS ALL THE RELEVANT DIRECTIONS
		// Debug the position state of the current camera
#ifdef _DEBUG_FAST
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CamMan_Public::GetCurrMatrix().GetXAxis() * 2.0f ), 0xffff0000 );
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CamMan_Public::GetCurrMatrix().GetYAxis() * 2.0f ), 0xff00ff00 );
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CamMan_Public::GetCurrMatrix().GetZAxis() * 2.0f ), 0xff0000ff );

		// Draw the world origin
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CVecMath::GetXAxis() * 1.5f ), 0xbbff0000 );
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CVecMath::GetYAxis() * 1.5f ), 0xbb00ff00 );
		g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( CVecMath::GetZAxis() * 1.5f ), 0xbb0000ff );
		g_VisualDebug->RenderArc( CMatrix( CONSTRUCT_IDENTITY ), 1.5f, TWO_PI, 0xbbffffff );

		// Draw the input directions
		if ( m_obInput.m_fSpeed > 0.1f )
			g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( m_obInput.m_obInputDir * 3.0f ), 0xffffff00 );
		if ( m_obInput.m_fSpeedAlt > 0.1f )
			g_VisualDebug->RenderLine( CPoint( CONSTRUCT_CLEAR ), CPoint( m_obInput.m_obInputDirAlt * 3.0f ), 0xff00ffff );
#endif

		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
		if (pobPlayer)
		{
			CPoint obPlayerPosition = pobPlayer->GetPosition();

#ifdef _DEBUG_FAST
			if ( m_obInput.m_fSpeed > 0.1f )
				g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
										   CPoint( obPlayerPosition + (m_obInput.m_obInputDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xffffff00 );
			if ( m_obInput.m_fSpeedAlt > 0.1f )
				g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
										   CPoint( obPlayerPosition + (m_obInput.m_obInputDirAlt * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff00ffff );
#endif

			if( m_bControlTransitionCancelled==false && (m_obInput.m_fSpeedAlt>0.1f) )
			{
				m_bControlTransitionCancelled = true;
			}

#ifdef _DEBUG_FAST
			char acText[64] = "";
			sprintf( acText, "%.3f", (m_obStartingPadDirection.Dot( obInputRelativeToCamera )) );
			g_VisualDebug->Printf2D( 100, 100, 0xffffffff, 0, acText );
#endif
			if( m_bTransitionStart==true && m_bControlTransitionCancelled==false && (m_obStartingPadDirection.Dot( obInputRelativeToCamera )<0.75f) )
			{
				m_bControlTransitionCancelled = true;
			}

			// at some point, map input direction if a camera transition is taking place
			CamView* pCamView = const_cast<CamView*>( CamMan_Public::GetPrimaryView() );
			const CamTransition* pCurrentTransition = pCamView->ActiveTransition();
			if( pCurrentTransition )
			{
				float fControlTransTime = pCurrentTransition->GetControlTransTotal();
				if( fControlTransTime<pCurrentTransition->GetControlTransTotalTime() && !m_bControlTransitionCancelled )
				{
					DoMotionBlend( pCurrentTransition->GetDestination(), fControlTransTime, pCurrentTransition->GetControlTransTotalTime(),
								   obInputRelativeToCamera, obInputRelativeToCameraAlt, obRotation );
				}
			}
			else if( m_bImplicitTransInProgress==true )
			{
				m_fImplicitTransTime += fTimeChange;
				
				if( m_fImplicitTransTime<m_fImplicitTransMaxTime && m_pDestCam!=0 && !m_bControlTransitionCancelled )
				{
					DoMotionBlend( m_pDestCam, m_fImplicitTransTime, m_fImplicitTransMaxTime, obInputRelativeToCamera, 
								   obInputRelativeToCameraAlt, obRotation );
				}
				else
				{
					m_bImplicitTransInProgress = false;
					m_pDestCam = 0;
				}
			}
			else if( m_bHoldInputDirection==true )
			{
				m_fImplicitTransTime += fTimeChange;

				if( m_fImplicitTransTime<m_fImplicitTransMaxTime && m_pDestCam!=0 && !m_bControlTransitionCancelled )
				{
					DoMotionBlend2( m_obSourceCamDir, m_pDestCam, m_fImplicitTransTime, 
									m_fImplicitTransMaxTime, obInputRelativeToCamera, obInputRelativeToCameraAlt, 
									obRotation );

				}
				else
				{
					m_bHoldInputDirection = false;
					m_pDestCam = 0;

				}
// 				if( m_bStoreHoldDirection==true )
// 				{
// 					m_bStoreHoldDirection = false;
// 					m_obStartingPadDirection = obInputRelativeToCamera;
// 				}
// 
// 				if( m_obStartingPadDirection.Dot( obInputRelativeToCamera )<0.75f )
// 				{
// 					m_bHoldInputDirection = false;
// 				}
// 				else
// 				{
// 					m_obInput.m_obInputDir = m_obStartingPadDirection;
// 				}
			}
			else
			{
				m_bTransitionStart = false;
				m_obCachedInputDir = m_obPrevMainDir;
				m_bControlTransitionCancelled = false;
			}
		}

		

		// The VKey mapping requires the pad analogue stick inputs to be mapped from camera space
		// into the entities local space and converted to an angle around Y.
		CMatrix obEntityLWInv = m_pobParent->GetMatrix().GetAffineInverse();
		CDirection obInputInEntityLocal;

		// Primary analogue stick transformation
		obInputInEntityLocal = m_obInput.m_obInputDir * obEntityLWInv ;
		float fAngle1 = GetGlobalYAngle( obInputInEntityLocal );

		// Secondary analogue stick transformation
		obInputInEntityLocal = m_obInput.m_obInputDirAlt * obEntityLWInv ;
		float fAngle2 = GetGlobalYAngle( obInputInEntityLocal );

		// Update the VKeyManager
		m_pobVKeyManager->Update( fTimeChange, fAngle1, fAngle2 );

		// Get pad button information
		m_obInput.m_uiHeld		= CInputHardware::Get().GetPadP(m_ePadNumber)->GetHeld();
		m_obInput.m_uiPressed	= CInputHardware::Get().GetPadP(m_ePadNumber)->GetPressed();
		m_obInput.m_uiReleased	= CInputHardware::Get().GetPadP(m_ePadNumber)->GetReleased();
		m_obInput.m_uiVPressed	= m_pobVKeyManager->GetVPressed();
		m_obInput.m_uiVHeld		= m_pobVKeyManager->GetVHeld();
		m_obInput.m_uiVReleased = m_pobVKeyManager->GetVReleased();

		// If you want to try and break stuff - this is where to do it
		if ( m_bRandomOutput )
			DebugNinaSimulator();

#ifndef _RELEASE
		// If we're playing an input sequence, hijack controller input until it is finished.
		if ( m_obInputSequence.Update( fTimeChange ) )
			m_obInputSequence.Evaluate( m_obInput );

		float fPosX = g_VisualDebug->GetDebugDisplayWidth() - 155.0f;
		DebugOutputButtons( m_obInput.m_uiVPressed,  "Pressed:  ", fPosX, 10.0f );
		DebugOutputButtons( m_obInput.m_uiVHeld,     "Held:     ", fPosX, 30.0f );
		DebugOutputButtons( m_obInput.m_uiVReleased, "Released: ", fPosX, 50.0f );
#endif

		// Look for wiggling action
		CheckWiggle();

		// Store how long each virtual button has been pressed
		SetVHeldTimes( fTimeChange );

		// Make sure any 'button events' are posted
		SendButtonMessages();

		UpdateButtonMashPerSecondCount(fTimeChange);
	}

	// Otherwise reset all our stored input data
	else
	{
		m_obInput.Reset();
		ClearVHeldTimes();
	}

	// Set and reset pad rumble for this frame
#ifdef PLATFORM_PC // TODO implement PS3 rumble
	if ( m_bRumble )
	{
		CInputHardware::Get().GetPadP( m_ePadNumber )->SetRumble( true );
		m_bRumble = false;
	}
#endif 
}

#ifndef _RELEASE
void CInputComponent::DebugOutputButtons( u_int uiButtonState, const ntstd::String& obLabel, float fPosX, float fPosY ) const
{
	g_VisualDebug->Printf2D( fPosX, fPosY, 0xffff0000, 0, "%s%c%c%c%c%c%c",
		obLabel.c_str(),
		( uiButtonState & ( 1 << AB_RSTANCE ) ) ? 'R' : '-',
		( uiButtonState & ( 1 << AB_ATTACK_MEDIUM ) ) ? 'M' : '-',
		( uiButtonState & ( 1 << AB_ATTACK_FAST ) ) ? 'F' : '-',
		( uiButtonState & ( 1 << AB_GRAB ) ) ? 'G' : '-',
		( uiButtonState & ( 1 << AB_ACTION ) ) ? 'A' : '-',
		( uiButtonState & ( 1 << AB_PSTANCE ) ) ? 'P' : '-');
}
#endif


//------------------------------------------------------------------------------------------
//!
//!	CInputComponent::HandleImplicitTransition
//!	Handles camera relative motion blending for implicit camera transitions.
//!
//------------------------------------------------------------------------------------------
void CInputComponent::HandleImplicitTransition( const CameraInterface* pobCamInterface )
{
	m_bImplicitTransInProgress = true;
	m_fImplicitTransTime = 0.0f;
	m_pDestCam = pobCamInterface;
}

//------------------------------------------------------------------------------------------
//!
//!	CInputComponent::HoldInputDirection
//!	Triggers holding of the input direction, until it deviates by a set amount.
//!
//------------------------------------------------------------------------------------------
void CInputComponent::HoldInputDirection( const CDirection& obInitialZDir, const CameraInterface* pobCamInterface )
{
	m_bHoldInputDirection = true;
	m_bStoreHoldDirection = true;
	m_obSourceCamDir = obInitialZDir;
	m_pDestCam = pobCamInterface;
	m_fImplicitTransTime = 0.0f;
}

#ifndef _RELEASE
/***************************************************************************************************
*
*	FUNCTION		CInputComponent::PlayMove
*
*	DESCRIPTION		Plays a single move rather than a whole combo, by mapping that move on a button
*					and then doing a simulated button press.
*
***************************************************************************************************/
void CInputComponent::PlayMove( const CAttackLink* obMove )
{
	// Clear input sequence
	m_obInputSequence.Clear();

	CAttackLink* pIdleStance = m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure->m_pobLeadCluster;
	pIdleStance->m_pobLinks[AM_ACTION] = obMove;
	m_obInputSequence.Push(AM_ACTION, 0.5f); // do the move after a short delay
	m_obInputSequence.Play();
	// TODO: We should unset the move again, as we're putting in junk in the action-link of the idle stance (and possibly overwriting data)
}

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::PlaySequence
*
*	DESCRIPTION		Plays back an InputSequence constructed for the supplied sequence pointer,
*					overriding joypad input until the sequence is complete or another sequence
*					is played.
*
***************************************************************************************************/
void CInputComponent::PlaySequence( const CAttackLink* pSequence )
{
	// Populate m_obInputSequence with the button presses that make up the sequence.
	m_obInputSequence.Clear();

	// Let's put in a little delay at the start, before we do the actual combo
	m_obInputSequence.Push(AM_NONE, 0.5f);
 
	// TODO: we shouldn't rely on the lead cluster being the idle stance, we should determine the idle stance dynamically
	const CAttackLink* pIdleStance = m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure->m_pobLeadCluster;
	SequenceListType obComboSequence(ComboInspector::SearchForComboPath(pIdleStance, pSequence));   
	SequenceListType::iterator sequenceIt = obComboSequence.begin();
	if (sequenceIt != obComboSequence.end()) // if collection is empty, we can't do this combo
	{
		++sequenceIt; // skip first entry, this is the "idle stance" and does not require a button press
		while (sequenceIt != obComboSequence.end())
		{
			const CAttackLink* pPreviousMove = *(--sequenceIt);
			const CAttackLink* pCurrentMove = *(++sequenceIt);
            
			if (IsAutoLinkTransition(pPreviousMove, pCurrentMove))
			{
				m_obInputSequence.Push(AM_NONE, pPreviousMove->GetAttackDataP()->GetAttackTime(1.2f)); // slightly longer than it actually takes
			}
			else
			{
				ATTACK_MOVE_TYPE eButtonType(CalculateButtonType(pPreviousMove, pCurrentMove));
				if (eButtonType == AM_NONE)
				{
					// TODO: Hack, why the speed threshold and not another? Well, by the time we get here we don't really know what stance/attack
					// originally required the button to be held done. Normally they're all the same anyway, so we'll just go with the standard one.
					m_obInputSequence.Push(AM_NONE, m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_fHeldAttackThresholdSpeed);
				}
				else
				{
					float fDuration = CalculateButtonDuration(pPreviousMove, SequenceListType(sequenceIt, obComboSequence.end()) );
					m_obInputSequence.Push(eButtonType, CalculateButtonTime(pPreviousMove, eButtonType), fDuration);
				}
			}

			++sequenceIt;
		}
	}

	m_obInputSequence.Play();
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::StopSequence
*
*	DESCRIPTION		Stops any InputSequence that is currently playing and restores joypad control.
*
***************************************************************************************************/
void CInputComponent::StopSequence()
{
	m_obInputSequence.Stop();
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::IsAutoLinkTransition
*
*	DESCRIPTION		True if the transition from the previous to the current link is an autolink
*                   transition, false if it is a manual transition (or no transition at all).
*
***************************************************************************************************/
bool CInputComponent::IsAutoLinkTransition(const CAttackLink* pPreviousLink, const CAttackLink* pCurrentLink) const
{
	const CAttackData* pPreviousAttackData = pPreviousLink->GetAttackDataP();
	if (!pPreviousAttackData || !pPreviousAttackData->m_bAutoLink)
	{
		// Autolink flag not set (or not attack data), impossible for this to autolink.
		return false;
	}
	else
	{
		if (pPreviousLink->m_pobLinks[AM_ACTION])
		{
			// If there is an action link but it is NOT the one we're transitioning to, then this is NOT an
			// autolink transition (at least, we don't want it to autolink).
			return pPreviousLink->m_pobLinks[AM_ACTION] == pCurrentLink;
		}
		else
		{
			for (int iIndex = 0; iIndex < AM_NONE; ++iIndex)
			{
				// The first link found will be the one autolinked to, this MUST be our wanted link or else
				// we do NOT want it to transition by autolink.
				if (pPreviousLink->m_pobLinks[iIndex])
				{
					return pPreviousLink->m_pobLinks[iIndex] == pCurrentLink;
				}
			}
			// Nothing is linked from the previous link at all, it's impossible for this to autolink (or do
			// anything else for that matter).
			return false;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::CalculateButtonType
*
*	DESCRIPTION		Calculate what button type the transition from one attack link to another
*                   attack link correspons to.
*
***************************************************************************************************/
ATTACK_MOVE_TYPE CInputComponent::CalculateButtonType(const CAttackLink* pPreviousLink, const CAttackLink* pCurrentLink) const
{
	for (int iIndex = 0; iIndex < AM_NONE; ++iIndex)
	{
		if (pPreviousLink->m_pobLinks[iIndex] && pPreviousLink->m_pobLinks[iIndex] == pCurrentLink)
		{
			return static_cast<ATTACK_MOVE_TYPE>(iIndex);
		}
	}
	return AM_NONE;
}

/***************************************************************************************************
*
*	FUNCTION		CInputComponent::CalculateButtonTime
*
*	DESCRIPTION		Calculate at what time a button needs to be pressed to transition into a new
*                   attack link.
*
***************************************************************************************************/
float CInputComponent::CalculateButtonTime(const CAttackLink* pPreviousLink, const ATTACK_MOVE_TYPE eButtonType) const
{
	if (pPreviousLink->GetAttackDataP())
	{
		float fTotalAttackTime = pPreviousLink->GetAttackDataP()->GetAttackTime(1.0f);
		float fFirstAvailable = eButtonType == AM_NONE ? 1.0f : pPreviousLink->GetAttackDataP()->m_obAttackPopOut.GetFirstValue(1.0f);
		if (fFirstAvailable < 0.0f) fFirstAvailable = 1.0f; // this happens when there is no pop out defined
		return fTotalAttackTime * fFirstAvailable;
	}
	else
	{
		return 0.0f;
	}
}

float CInputComponent::CalculateButtonDuration(const CAttackLink* pPreviousLink, const SequenceListType& obNextLinks) const
{
	float fResult = CInputSequence::DURATION_DEFAULT;
	const CAttackLink* pCurrentLink = pPreviousLink;
	ATTACK_MOVE_TYPE eType = CalculateButtonType(pCurrentLink, obNextLinks.front());
	pCurrentLink = obNextLinks.front();
	for (SequenceListType::const_iterator linkIt = (++obNextLinks.begin()); linkIt != obNextLinks.end(); ++linkIt)
	{
		const CAttackLink* pNextLink = *linkIt;
		if (pCurrentLink->m_pobButtonHeldAttack == pNextLink)
		{
			if (eType == AM_POWER_FAST || eType == AM_POWER_MEDIUM)
			{
				fResult += m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_fHeldAttackThresholdPower;
			}
			else if (eType == AM_RANGE_FAST || eType == AM_RANGE_MEDIUM)
			{
				fResult += m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_fHeldAttackThresholdRange;
			}
			else
			{
				fResult += m_pobParent->GetAttackComponent()->GetAttackDefinition()->m_fHeldAttackThresholdSpeed;
			}
			fResult += CInputSequence::DURATION_DEFAULT;
		}
		else
		{
			// stop looking, we've got our duration now
			break;
		}
		pCurrentLink = pNextLink;
	}
	return fResult;
}
#endif


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::DebugNinaSimulator
*
*	DESCRIPTION		Tries to replicate Nina's magical ability to break stuff
*	
*					We want this to be deterministic - then we can replay it
*
***************************************************************************************************/
void CInputComponent::DebugNinaSimulator( void )
{
	// Set all the button presses to random
	m_obInput.m_uiHeld		= grand();
	m_obInput.m_uiPressed	= grand();
	m_obInput.m_uiReleased	= grand();
	m_obInput.m_uiVPressed	= grand();
	m_obInput.m_uiVHeld		= grand();
	m_obInput.m_uiVReleased = grand();

	// Set the analogue stiick speeds
	m_obInput.m_fSpeed = grandf( 1.0f );
	m_obInput.m_fSpeedAlt = grandf( 1.0f );

	// Put in random stick directions
	m_obInput.m_obInputDir.X() = grandf( 2.0f ) - 1.0f;
	m_obInput.m_obInputDir.Y() = 0.0f;
	m_obInput.m_obInputDir.Z() = grandf( 2.0f ) - 1.0f;
	m_obInput.m_obInputDir.Normalise();

	m_obInput.m_obInputDirAlt.X() = grandf( 2.0f ) - 1.0f;
	m_obInput.m_obInputDirAlt.Y() = 0.0f;
	m_obInput.m_obInputDirAlt.Z() = grandf( 2.0f ) - 1.0f;
	m_obInput.m_obInputDirAlt.Normalise();
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::GetGlobalYAngle
*
*	DESCRIPTION		A static function to find an angle about the Y axis described by a world 
*					direction.
*
***************************************************************************************************/
float CInputComponent::GetGlobalYAngle( const CDirection& obInputDirection )
{
	// Find the angle described by the X and Y components of the input direction
	float fAngle = atan2f( obInputDirection.Z(), obInputDirection.X() ) - ( PI / 2.0f );

	// Make sure we have an angle between 0.0 and 2 * PI
	return NormaliseAngle( fAngle );
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::NormaliseAngle
*
*	DESCRIPTION		A static helper to make sure we have an angle between 0.0 and TWO_PI
*
***************************************************************************************************/
float CInputComponent::NormaliseAngle( float fAngle )
{
	// Find the remainder when divided by 2 * PI
	float fRem = fmod( fAngle, TWO_PI );
	
	// If the remainder is less than -PI
	if ( fRem < 0.0f )
	{
		fRem += TWO_PI;
		return fRem;
	}

	// Otherwise the remainder will do just fine
	return fRem;
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::CheckWiggle
*
*	DESCRIPTION		This bit of code is nabbed directly from KFC.  I do not want to spend more time 
*					thinking about what constitutes a wiggle ( waggle in KFC ) than i absolutely 
*					have to.
*
*					Obviously the commenting didn't come from KFC.
*
***************************************************************************************************/
void CInputComponent::CheckWiggle( void )
{
	// Generally we default to not wiggling
	m_obInput.m_bWiggled = false;

	// Get the current analogue stick angle
	float fPadAngle = CInputHardware::Get().GetPad( m_ePadNumber ).GetAnalogLAngle();
	
	// We only call it a wiggle if it is greater than the magnitude here
	if ( m_obInput.m_fSpeed > 0.5)
	{
		// What side of the analogue stick are do we think we are currently pointing at
		if ( m_obInput.m_iWiggleSide > 0 )
		{
			// We split the pad straight down the middle - the left is side minus one
			if ( ( fPadAngle < TWO_PI ) && ( fPadAngle > PI ) )
			{
				// ntPrintf( "********** WIGGLE **********  \n" );
				m_obInput.m_iWiggleSide = -1;
				m_obInput.m_bWiggled = true;
			}
		}
		else
		{
			if ( ( fPadAngle < PI ) && ( fPadAngle > 0.0f ) )
			{
				// ntPrintf( "********** WIGGLE **********  \n" );
				m_obInput.m_iWiggleSide = 1;
				m_obInput.m_bWiggled = true;
			}
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::GetVHeldTime
*
*	DESCRIPTION		Tells you how long a button has been held down for
*
***************************************************************************************************/
float CInputComponent::GetVHeldTime( VIRTUAL_BUTTON_TYPE eButton ) const
{
	return m_afVButtonHeldTimes[eButton];
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::IsMotionSensor
*
*	DESCRIPTION		Gets whether the pad has motion sensing abilities
*
***************************************************************************************************/
bool CInputComponent::IsMotionSensor( void ) const
{
	CInputPad* pobPad = CInputHardware::Get().GetPadP(m_ePadNumber);
	ntAssert( pobPad );

	return pobPad->IsMotionSensor();
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::GetSensorRawMag
*
*	DESCRIPTION		Gets the last raw magnitude of a particular motion sensor
*
***************************************************************************************************/
float CInputComponent::GetSensorRawMag( PAD_SENSOR_ID eSensor ) const
{
	CInputPad* pobPad = CInputHardware::Get().GetPadP(m_ePadNumber);
	ntAssert( pobPad );

	// Debug check - shouldnt really be calling this function if the pad is not motion sensing
	//ntAssert_p( pobPad->IsMotionSensor(), ("Pad does not have motion sensors") );

	return pobPad->GetSensorRawMag( eSensor );
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::GetSensorFilteredMag
*
*	DESCRIPTION		Gets a filtered magnitude of a particular motion sensor
*
***************************************************************************************************/
float CInputComponent::GetSensorFilteredMag( PAD_SENSOR_ID eSensor, PAD_SENSOR_FILTER eFilter ) const
{
	CInputPad* pobPad = CInputHardware::Get().GetPadP(m_ePadNumber);
	ntAssert( pobPad );

	// Debug check - shouldnt really be calling this function if the pad is not motion sensing
	//ntAssert_p( pobPad->IsMotionSensor(), ("Pad does not have motion sensors") );

	return pobPad->GetSensorFilteredMag( eSensor, eFilter );
}


/***************************************************************************************************
*
*	FUNCTION		CInputComponent::SendButtonMessages
*
*	DESCRIPTION		This sends input messages to the Lua environment - to be handled there if 
*					required.
*
***************************************************************************************************/
void CInputComponent::SendButtonMessages( void ) const
{
	if ( m_pobParent->GetMessageHandler() )
	{
		// Power stance button press
		if ( ( 1 << AB_PSTANCE ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_button_power>();

		// Power stance button release
		if ( ( 1 << AB_PSTANCE ) & GetVReleased() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_release_power>();

		// Range stance button press
		if ( ( 1 << AB_RSTANCE ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_button_range>();

		// Range stance button release
		if ( ( 1 << AB_RSTANCE ) & GetVReleased() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_release_range>();

		// Attack Special stuff
		if ( ( 1 << AB_SPECIAL ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_button_special>();

		// An attack button - general for now
		if ( ( ( 1 << AB_ATTACK_FAST ) | ( 1 << AB_ATTACK_MEDIUM ) ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		
		// An attack button released
		if ( ( ( 1 << AB_ATTACK_FAST ) | ( 1 << AB_ATTACK_MEDIUM ) ) & GetVReleased() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_release_attack>();

		// An action button press
		if ( ( 1 << AB_ACTION ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();

		// An action button press
		if ( ( 1 << AB_ACTION ) & GetVReleased() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_release_action>();

		// A grab button press
		if ( ( 1 << AB_GRAB ) & GetVPressed() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_buttongrab>();

		// A grab button released
		if ( ( 1 << AB_GRAB ) & GetVReleased() )
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_release_grab>();

		// A dodge button press
		if ( ( ( 1 << AB_DODGE_LEFT ) | ( 1 << AB_DODGE_RIGHT ) | ( 1 << AB_DODGE_FORWARD ) | ( 1 << AB_DODGE_BACK ) ) & GetVPressed() ) 
			m_pobParent->GetMessageHandler()->ReceiveMsg<msg_buttondodge>();
	}
}

void CInputComponent::DoMotionBlend( const CameraInterface* pDestCamera, float fCurrentTransTime, float fMaxTransTime,
									 CDirection& obCamRelativeMovement, CDirection& obCamRelativeMovementAlt, CMatrix& obFlattenRotation )
{
#ifdef _DEBUG_FAST
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	ntAssert(pobPlayer);
	CPoint obPlayerPosition = pobPlayer->GetPosition();
#endif
	//float fControlTransTime = m_pCurrentTransition->GetControlTransTotal();
	if( fCurrentTransTime<fMaxTransTime )
	{
		float fAngle = TWO_PI * fCurrentTransTime;
		if( fAngle>TWO_PI )
		{
			fAngle -= (float(int(fAngle/TWO_PI)) * TWO_PI );
		}


		// get matrices/directions for the two cameras
		if( m_bTransitionStart==false )
		{
			m_bTransitionStart = true;
			m_obSourceCamDir = CamMan_Public::GetCurrMatrix().GetZAxis();
			m_obTargetCamDir = pDestCamera->GetTransform().GetZAxis();
			m_obStartingPadDirection = obCamRelativeMovement;
		}

#ifdef _DEBUG_FAST
		CMatrix obCubeTrnfrm;
		obCubeTrnfrm.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f ), fAngle );
		//obCubeTrnfrm.SetTranslation( pobPlayer->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f ) );

		CMatrix cube( CONSTRUCT_IDENTITY );
		cube[0][0] = 1.0f;
		cube[1][1] = 0.1f;
		cube[2][2] = 1.0f;

		cube = cube * obCubeTrnfrm;
		cube.SetTranslation( obPlayerPosition + CPoint( 0.0f, 1.0f, 0.0f ) );
		g_VisualDebug->RenderCube( cube, 0xffff0000 );

		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
									CPoint( obPlayerPosition + (m_obSourceCamDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff0000ff );
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
									CPoint( obPlayerPosition + (m_obTargetCamDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff00ff00 );
#endif
		CPoint obCamPos = CamMan_Public::GetTransform()->GetWorldTranslation();
		CMatrix obCtrlTransMat( CONSTRUCT_IDENTITY );
		CCamUtil::CreateFromPoints( obCtrlTransMat, obCamPos, pDestCamera->GetLookAt() );


		m_obInput.m_obInputDir = obCamRelativeMovement * obCtrlTransMat * obFlattenRotation;
		m_obInput.m_obInputDirAlt = obCamRelativeMovementAlt * obCtrlTransMat * obFlattenRotation;

#ifdef _DEBUG_FAST
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), pDestCamera->GetLookAt(), 1.0f, 0xffff00ff );
		// input direction before transition
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
								CPoint( obPlayerPosition + (m_obCachedInputDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0x0000ffff );
#endif


		float fControlInterpValue = CCamUtil::Sigmoid( fCurrentTransTime, fMaxTransTime );
		if( fControlInterpValue > 1.f || fControlInterpValue < 0.f)
		{
			ntPrintf("Camera - Bad Sigmoid control interpolation value.\n");
			fControlInterpValue = clamp( fControlInterpValue, 0.f, 1.f );
		}

		fControlInterpValue *= fControlInterpValue;

		CQuat obCachedDirQuat( m_obCachedInputDir, m_obInput.m_obInputDir );
		CMatrix obInputDirInterpMat( obCachedDirQuat * fControlInterpValue );
		CDirection obResultDir = m_obCachedInputDir * obInputDirInterpMat;

#ifdef _DEBUG_FAST
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
								CPoint( obPlayerPosition + (obResultDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff0000ff );
#endif
		//m_obControlInterpLookatPoint = m_pSrc->GetLookAt() + ((m_pDst->GetLookAt() - m_pSrc->GetLookAt()) * fControlInterpValue);

		m_obInput.m_obInputDir = obResultDir;
	}
}

void CInputComponent::DoMotionBlend2( const CDirection& obInitialZDir, const CameraInterface* pDestCamera, float fCurrentTransTime, 
									  float fMaxTransTime, CDirection& obCamRelativeMovement, CDirection& obCamRelativeMovementAlt, 
									  CMatrix& obFlattenRotation )
{
#ifdef _DEBUG_FAST
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	ntAssert(pobPlayer);
	CPoint obPlayerPosition = pobPlayer->GetPosition();
#endif
	//float fControlTransTime = m_pCurrentTransition->GetControlTransTotal();
	if( fCurrentTransTime<fMaxTransTime )
	{
		float fAngle = TWO_PI * fCurrentTransTime;
		if( fAngle>TWO_PI )
		{
			fAngle -= (float(int(fAngle/TWO_PI)) * TWO_PI );
		}


		// get matrices/directions for the two cameras
		if( m_bTransitionStart==false )
		{
			m_bTransitionStart = true;
			m_obSourceCamDir = obInitialZDir;
			m_obTargetCamDir = pDestCamera->GetTransform().GetZAxis();
			m_obStartingPadDirection = obCamRelativeMovement;
		}

#ifdef _DEBUG_FAST
		CMatrix obCubeTrnfrm;
		obCubeTrnfrm.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f ), fAngle );
		//obCubeTrnfrm.SetTranslation( pobPlayer->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f ) );

		CMatrix cube( CONSTRUCT_IDENTITY );
		cube[0][0] = 1.0f;
		cube[1][1] = 0.1f;
		cube[2][2] = 1.0f;

		cube = cube * obCubeTrnfrm;
		cube.SetTranslation( obPlayerPosition + CPoint( 0.0f, 1.0f, 0.0f ) );
		g_VisualDebug->RenderCube( cube, 0xffff0000 );

		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
			CPoint( obPlayerPosition + (m_obSourceCamDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff0000ff );
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
			CPoint( obPlayerPosition + (m_obTargetCamDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff00ff00 );
#endif
		CPoint obCamPos = CamMan_Public::GetTransform()->GetWorldTranslation();
		CMatrix obCtrlTransMat( CONSTRUCT_IDENTITY );
		CCamUtil::CreateFromPoints( obCtrlTransMat, obCamPos, pDestCamera->GetLookAt() );


		m_obInput.m_obInputDir = obCamRelativeMovement * obCtrlTransMat * obFlattenRotation;
		m_obInput.m_obInputDirAlt = obCamRelativeMovementAlt * obCtrlTransMat * obFlattenRotation;

#ifdef _DEBUG_FAST
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), pDestCamera->GetLookAt(), 1.0f, 0xffff00ff );
		// input direction before transition
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
			CPoint( obPlayerPosition + (m_obCachedInputDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0x0000ffff );
#endif


		float fControlInterpValue = CCamUtil::Sigmoid( fCurrentTransTime, fMaxTransTime );
		if( fControlInterpValue > 1.f || fControlInterpValue < 0.f)
		{
			ntPrintf("Camera - Bad Sigmoid control interpolation value.\n");
			fControlInterpValue = clamp( fControlInterpValue, 0.f, 1.f );
		}

		fControlInterpValue *= fControlInterpValue;

		CQuat obCachedDirQuat( m_obCachedInputDir, m_obInput.m_obInputDir );
		CMatrix obInputDirInterpMat( obCachedDirQuat * fControlInterpValue );
		CDirection obResultDir = m_obCachedInputDir * obInputDirInterpMat;

#ifdef _DEBUG_FAST
		g_VisualDebug->RenderLine( CPoint( obPlayerPosition ) + CPoint( 0.0f, 1.0f, 0.0f ),
			CPoint( obPlayerPosition + (obResultDir * 3.0f) + CPoint( 0.0f, 1.0f, 0.0f ) ), 0xff0000ff );
#endif
		//m_obControlInterpLookatPoint = m_pSrc->GetLookAt() + ((m_pDst->GetLookAt() - m_pSrc->GetLookAt()) * fControlInterpValue);

		m_obInput.m_obInputDir = obResultDir;
	}
}
