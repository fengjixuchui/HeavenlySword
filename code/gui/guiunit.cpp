/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which all other GUI items are based - including screens
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiunit.h"
#include "guiresource.h"
#include "guimanager.h"
#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "anim/animloader.h"
#include "core/exportstruct_anim.h"
#include "game/renderablecomponent.h"
#include "gfx/clump.h"
#include "core/timer.h"
#include "guiscreen.h"

#include "core/visualdebugger.h"

#ifdef _DEBUG_GUI_UNIT_STATE
float g_fUnitStateDebugX = 100.0f;
float g_fUnitStateDebugY = 0.0f;
#endif 

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::CGuiUnit
*
*	DESCRIPTION		Construction
*
*					There should be no initialisation to defaults here.  Any parameter which can be
*					set on a screen element that requires a default should have its value set from 
*					a DTD file.
*
***************************************************************************************************/

CGuiUnit::CGuiUnit( void )
:	m_eHorizontalJustification( JUSTIFY_CENTRE )
,	m_eVerticalJustification( JUSTIFY_MIDDLE )
,	m_eRenderSpace( RENDER_CAMERASPACE )
,	m_pobBaseTransform( 0 )
,	m_pobClumpHeader( 0 )
,	m_pobHierarchy( 0 )
,	m_pobRenderable( 0 )
,	m_pobPreviousAnimation( 0 )
,	m_pobAnimator( 0 )
,	m_ePadNumber( PAD_NUM )
,	m_fPadTime( 0.0f )
,	m_obPadTimer()
,	m_pobParentScreen(CGuiScreen::ms_pobCurrentScreen)
, 	m_bAllowRender(true)
{
	// Initalise the animation details
	for ( int iState = 0; iState < UNIT_STATE_SIZE; iState++ )
	{
		m_astrAnimations[iState].pobAnimationHeader = 0;
	}

	// Initialise the state flag
	m_eUnitState = STATE_BORN;

	m_fScreenFadeScale = 1.0f;
	m_fScreenFade = 1.0f;
	m_bFadeRunning = false;

	m_bAutoFade = true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::~CGuiUnit
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/

CGuiUnit::~CGuiUnit( void )
{
	// Kill the animator
	if ( m_pobAnimator )
		CAnimator::Destroy( m_pobAnimator );

	// Deal with the renderable
	if ( m_pobRenderable )
		NT_DELETE(m_pobRenderable); 

	// Deal with the hierarchy
	if ( m_pobHierarchy )
	{
		m_pobHierarchy->GetRootTransform()->RemoveFromParent();
		CHierarchy::Destroy( m_pobHierarchy );
	}

	// Deal with any base transform
	if ( m_pobBaseTransform )
	{
		m_pobBaseTransform->RemoveFromParent();
		NT_DELETE( m_pobBaseTransform );
	}

	// If we have a clump header - drop it
	if ( m_pobClumpHeader )
		CClumpLoader::Get().UnloadClump( m_pobClumpHeader );

	// Deal with all the animation details
	for ( int iState = 0; iState < UNIT_STATE_SIZE; iState++ )
	{
		if ( m_astrAnimations[iState].pobAnimationHeader )
			CAnimLoader::Get().UnloadAnim( m_astrAnimations[iState].pobAnimationHeader );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::Update
*
*	DESCRIPTION		Returns true whilst the unit is still active.
*
***************************************************************************************************/
bool CGuiUnit::Update( void )
{
	// Do any general updates
	m_obPadTimer.Update();

#ifdef _DEBUG_GUI_UNIT_STATE
	float fXOut = g_fUnitStateDebugX;
	float fYOut = g_fUnitStateDebugY;
	g_fUnitStateDebugX = 120.0f;
	g_fUnitStateDebugY += 20.0f;
	char tmp[256] = {0};
	const char* s[UNIT_STATE_SIZE] = { "STATE_BORN", "STATE_ENTER", "STATE_IDLE", "STATE_FOCUS", "STATE_FOCUSIN", "STATE_FOCUSOUT", "STATE_EXIT", "STATE_DEAD", "STATE_FADEIN", "STATE_FADEOUT" };
	int offset = sprintf(tmp, "%s %s ", GetName(), s[m_eUnitState]);
#endif
	// Now do the state based update - this should probably be scripted
	switch ( m_eUnitState )
	{
	case STATE_ENTER:		UpdateEnter();		break;
	case STATE_IDLE:		UpdateIdle();		break;
	case STATE_FOCUS:		UpdateFocus();		break;
	case STATE_FOCUSIN:		UpdateFocusIn();	break;
	case STATE_FOCUSOUT:	UpdateFocusOut();	break;
	case STATE_EXIT:		UpdateExit();		break;
	case STATE_DEAD:		return false;		break;
	case STATE_FADEIN:		UpdateFadeIn();		break;
	case STATE_FADEOUT:		UpdateFadeOut();	break;
	default:				ntAssert( 0 );		break;
	}

#ifdef _DEBUG_GUI_UNIT_STATE
	sprintf(&tmp[offset], "%s [%0.2f]", s[m_eUnitState], ScreenFade());
	g_VisualDebug->Printf2D(fXOut, fYOut, 0xffffffff, 0, tmp);
//	/*if (strcmp(GetName(), "SCREEN") == 0)*/ ntPrintf("%s\n", tmp);
//	if (strcmp(GetName(), "SCREEN") == 0) ntPrintf("\n");
#endif

	// Still going...
	return ( m_eUnitState != STATE_DEAD );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::Render
*
*	DESCRIPTION		Returns true when OK.
*
***************************************************************************************************/

bool CGuiUnit::Render( void )
{
	return m_bAllowRender;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiUnit::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "clumpname" ) == 0 )
		{
			return ProcessClumpNameValue( pcValue );
		}

		else if ( strcmp( pcTitle, "baseposition" ) == 0 )
		{
			return ProcessBasePositionValue( pcValue );
		}

		else if ( strcmp( pcTitle, "verticaljustification" ) == 0 )
		{
			return ProcessVerticalJustificationValue( pcValue );
		}

		else if ( strcmp( pcTitle, "horizontaljustification" ) == 0 )
		{
			return ProcessHorizontalJustificationValue( pcValue );
		}

		else if ( strcmp( pcTitle, "renderspace" ) == 0 )
		{
			return ProcessRenderSpaceValue( pcValue );
		}

		else if ( strcmp( pcTitle, "animentry" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_ENTER );
		}

		else if ( strcmp( pcTitle, "animexit" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_EXIT );
		}

		else if ( strcmp( pcTitle, "animidle" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_IDLE );
		}

		else if ( strcmp( pcTitle, "animfocus" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_FOCUS );
		}

		else if ( strcmp( pcTitle, "animfocusin" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_FOCUSIN );
		}

		else if ( strcmp( pcTitle, "animfocusout" ) == 0 )
		{
			return ProcessAnimationNameValue( pcValue, STATE_FOCUSOUT );
		}

		else if ( strcmp( pcTitle, "padtiming" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fPadTime );
		}

		else if ( strcmp( pcTitle, "id" ) == 0 )
		{
			m_obUnitID = pcValue;
			return true;
		}
		else if ( strcmp( pcTitle, "fadescale" ) == 0 )
		{
			return GuiUtil::SetFloat(pcValue, &m_fScreenFadeScale);
		}
		else if ( strcmp( pcTitle, "fades" ) == 0 )
		{
			ntPrintf("GUI: Found \'fades\' attributes. needs removal or replace with autoface\n");
			return true;
		}
		else if ( strcmp( pcTitle, "autofade" ) == 0 )
		{
			GuiUtil::SetBool(pcValue, &m_bAutoFade);
			return true;
		}
		else if ( strcmp( pcTitle, "initialfade" ) == 0 )
		{
			GuiUtil::SetFloat(pcValue, &m_fScreenFade);
			m_fScreenFade = max(0.0f, min(1.0f, m_fScreenFade));
			return true;
		}
		else if ( strcmp( pcTitle, "allowrender" ) == 0 )
		{
			GuiUtil::SetBool(pcValue, &m_bAllowRender);
			return true;
		}
		else if ( strcmp( pcTitle, "moveposition" ) == 0 )
		{
			return ProcessMovePositionValue( pcValue );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessChild
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnit::ProcessChild( CXMLElement* pobChild )
{
	// We don't use them for most elements.  This must be overridden if
	// they are need elsewhere.
	UNUSED(pobChild);
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  Hence at this
*					stage we know we have all the information that will be set by a script.  This
*					function is used to set up any information that requireds two or more attributes
*					to avoid any attribute ordering issues.
*
***************************************************************************************************/

bool CGuiUnit::ProcessEnd( void )
{
	// If we have an entity
	if ( m_pobHierarchy )
	{
		// Give the hierarchy it's parent transform - if there is one setup
		if ( m_pobBaseTransform )
			m_pobBaseTransform->AddChild( m_pobHierarchy->GetRootTransform() );

		// Put the entire contraption in our world
		m_pobRenderable = NT_NEW CRenderableComponent( m_pobHierarchy, true, true, true );
	}

	// Get this show on the road
	SetStateEnter();

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessAnimationNameValue
*
*	DESCRIPTION		Sets up animation details for a state.  Returns false if it fails.
*
***************************************************************************************************/

bool CGuiUnit::ProcessAnimationNameValue( const char* pcValue, UNIT_STATE eUnitState )
{
	// Check our imputs
	ntAssert( pcValue );
	ntAssert( eUnitState < UNIT_STATE_SIZE );

	// Check nothing has been set for this state already
	ntAssert( m_astrAnimations[eUnitState].pobAnimationHeader == 0 );

	char acAnimPath[MAX_PATH] = "data\\";
	strcat( acAnimPath, pcValue );

	// Try and load up the header to the relevant state slot
	m_astrAnimations[eUnitState].pobAnimationHeader = CAnimLoader::Get().LoadAnim_Neutral( acAnimPath );
	ntAssert( m_astrAnimations[eUnitState].pobAnimationHeader );

	// We can't do any more here because we don't know what other info has been set at this stage
	// The rest of the anim setup will be in ProcessEnd when we know all is there
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessClumpNameValue
*
*	DESCRIPTION		Returns false if it fails.
*
***************************************************************************************************/

bool CGuiUnit::ProcessClumpNameValue( const char* pcValue )
{
	char acClumpPath[MAX_PATH] = "data\\";
	strcat( acClumpPath, pcValue );

	// Try to build a clump header from the given filename
	m_pobClumpHeader = CClumpLoader::Get().LoadClump_Neutral( acClumpPath, true );
	ntAssert( m_pobClumpHeader );

	// Create our hierarchy from the clump
	m_pobHierarchy = CHierarchy::Create( m_pobClumpHeader );
	ntAssert( m_pobHierarchy );

	// Create an animator for our hierarchy
	m_pobAnimator = CAnimator::Create( NULL, m_pobHierarchy );
	ntAssert( m_pobAnimator );

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessBasePositionValue
*
*	DESCRIPTION		Deals with an x, y, z position from a string.  Set offset in the base transform
*					matrix.  Returns false if it fails.
*
***************************************************************************************************/

bool CGuiUnit::ProcessBasePositionValue( const char* pcValue )
{
	// We'll get three floats from the string
	float fX, fY, fZ;
	int iResult = sscanf( pcValue, "%f,%f,%f", &fX, &fY, &fZ ); 

	// Make sure we extracted three values
	ntAssert( iResult == 3 );
	UNUSED( iResult );

	// Create a point with the data
	CPoint obBasePoint( fX, fY, fZ );
	m_BasePosition = obBasePoint;

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	m_pobBaseTransform = NT_NEW Transform();
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );

	switch ( m_eRenderSpace )
	{

		case RENDER_WORLDSPACE:
		{
			// Set this transform as a child of the world root
			CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobBaseTransform );
			break;
		}

		case RENDER_CAMERASPACE:
		{
			// Set this transform as a child of GuiManager's camera transform
			CGuiManager::Get().GetCamTransform()->AddChild( m_pobBaseTransform );
			break;
		}

		case RENDER_SCREENSPACE:
		{
			m_BasePosition.X() *= CGuiManager::Get().BBWidth();
			m_BasePosition.Y() *= CGuiManager::Get().BBHeight();
			obBaseMatrix.SetTranslation( m_BasePosition );
			m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );
			CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobBaseTransform );
			break;
		}

		default:
		{
			ntAssert(0);
			break;
		}
	}
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessMovePositionValue
*
*	DESCRIPTION		Deals with an x, y, z position from a string. Moves the component to a new location.
*
***************************************************************************************************/

bool CGuiUnit::ProcessMovePositionValue( const char* pcValue )
{
	// We'll get three floats from the string
	float fX, fY, fZ;
	int iResult = sscanf( pcValue, "%f,%f,%f", &fX, &fY, &fZ ); 

	// Make sure we extracted three values
	ntAssert( iResult == 3 );
	UNUSED( iResult );

	// Create a point with the data
	CPoint obBasePoint( fX, fY, fZ );
	m_BasePosition = obBasePoint;

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	ntAssert_p(m_pobBaseTransform, ("baseposition must be set before you can use moveposition. check xml associated with current screen layout."));
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );

	if ( m_eRenderSpace == RENDER_SCREENSPACE )
	{
		m_BasePosition.X() *= CGuiManager::Get().BBWidth();
		m_BasePosition.Y() *= CGuiManager::Get().BBHeight();
		obBaseMatrix.SetTranslation( m_BasePosition );
		m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );
	}

	m_pobBaseTransform->ForceResynchronise();
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessVerticalJustificationValue
*
*	DESCRIPTION		Returns false if it fails
*
***************************************************************************************************/

bool CGuiUnit::ProcessVerticalJustificationValue( const char* pcValue )
{
	// Set the enum based on value string contents
	if ( strcmp( pcValue, "TOP" ) == 0 )
	{
		m_eVerticalJustification = JUSTIFY_TOP;
	}

	else if ( strcmp( pcValue, "MIDDLE" ) == 0 )
	{
		m_eVerticalJustification = JUSTIFY_MIDDLE;
	}

	else if ( strcmp( pcValue, "BOTTOM" ) == 0 )
	{
		m_eVerticalJustification = JUSTIFY_BOTTOM;
	}

	else
	{
		ntAssert( 0 );
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessHorizontalJustificationValue
*
*	DESCRIPTION		Returns false if it fails
*
***************************************************************************************************/

bool CGuiUnit::ProcessHorizontalJustificationValue( const char* pcValue )
{
	// Set the enum based on value string contents
	if ( strcmp( pcValue, "LEFT" ) == 0 )
	{
		m_eHorizontalJustification = JUSTIFY_LEFT;
	}

	else if ( strcmp( pcValue, "CENTRE" ) == 0 )
	{
		m_eHorizontalJustification = JUSTIFY_CENTRE;
	}

	else if ( strcmp( pcValue, "RIGHT" ) == 0 )
	{
		m_eHorizontalJustification = JUSTIFY_RIGHT;
	}

	else
	{
		ntAssert( 0 );
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessRenderSpaceValue
*
*	DESCRIPTION		Returns false if it fails
*
***************************************************************************************************/

bool CGuiUnit::ProcessRenderSpaceValue( const char* pcValue )
{
	// Set the enum based on value string contents
	if ( strcmp( pcValue, "WORLD" ) == 0 )
	{
		m_eRenderSpace = RENDER_WORLDSPACE;
	}

	else if ( strcmp( pcValue, "CAMERA" ) == 0 )
	{
		m_eRenderSpace = RENDER_CAMERASPACE;
	}

	else if ( strcmp( pcValue, "SCREEN" ) == 0 )
	{
		m_eRenderSpace = RENDER_SCREENSPACE;
	}

	else
	{
		ntAssert( 0 );
	}

	return true;
}	



/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateEnter( void )
{
	// Update the flag
	m_eUnitState = STATE_ENTER;
	
	// Kick off the entrance animation if there is one
	if ( m_astrAnimations[STATE_ENTER].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_ENTER].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Set the flags and kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateExit( void )
{	
	// Set the state flag
	m_eUnitState = STATE_EXIT;

	// If we are currently playing a looping anim stop it at the end of the loop
	if ( m_pobPreviousAnimation && m_pobPreviousAnimation->IsActive() )
	{
		m_pobPreviousAnimation->ClearFlagBits( ANIMF_LOOPING );
	}

	// If there is an exit anim set it going
	if ( m_astrAnimations[STATE_EXIT].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_EXIT].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateIdle
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateIdle( void )
{
	// Set the state flag
	m_eUnitState = STATE_IDLE;

	// Do we have a new animation to kick off
	if ( m_astrAnimations[STATE_IDLE].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_IDLE].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Set the flags and kick it off
		pobAnimation->SetFlagBits( ANIMF_LOOPING );
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateFocus
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateFocus( void )
{
	// Set the state flag
	m_eUnitState = STATE_FOCUS;

	// If there is a new animation then have it
	if ( m_astrAnimations[STATE_FOCUS].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_FOCUS].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Set the flags and kick it off
		pobAnimation->SetFlagBits( ANIMF_LOOPING );
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateFocusIn( void )
{
	// Set the state flag
	m_eUnitState = STATE_FOCUSIN;

	// If we are currently playing a looping anim stop it at the end of the loop
	if ( m_pobPreviousAnimation && m_pobPreviousAnimation->IsActive() )
	{
		m_pobPreviousAnimation->ClearFlagBits( ANIMF_LOOPING );
	}

	// If there is an exit anim set it going
	if ( m_astrAnimations[STATE_FOCUSIN].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_FOCUSIN].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateFocusOut
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateFocusOut( void )
{
	// Set the state flag
	m_eUnitState = STATE_FOCUSOUT;

	// If we are currently playing a looping anim stop it at the end of the loop
	if ( m_pobPreviousAnimation && m_pobPreviousAnimation->IsActive() )
	{
		m_pobPreviousAnimation->ClearFlagBits( ANIMF_LOOPING );
	}

	// If there is an exit anim set it going
	if ( m_astrAnimations[STATE_FOCUSOUT].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_FOCUSOUT].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateDead
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateDead( void )
{
	// Just set the state
	m_eUnitState = STATE_DEAD;
}

bool CGuiUnit::BeginEnter( bool bForce )
{
	// We will only repsond to this if we are in a focused state
	if ( ( !bForce ) && ( m_eUnitState != STATE_BORN ) ) return false;

	// Transition to the idle state
	SetStateEnter();

	// We have successfully changed state
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::BeginIdle
*
*	DESCRIPTION		Sets the unit state to idle - or to a transition to idle if one has been
*					defined.  It might be necessary to force a unit into a state even though
*					transition wise, like the grolsh, it is not ready yet.
*
***************************************************************************************************/

bool CGuiUnit::BeginIdle( bool bForce )
{	
	// We will only repsond to this if we are in a focused state
	if ( ( !bForce ) && ( m_eUnitState != STATE_FOCUS ) ) return false;

	// Transition to the idle state
	SetStateFocusOut();

	// We have successfully changed state
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::BeginFocus
*
*	DESCRIPTION		Sets the unit state to focus - or to a transition to focus if one has been
*					defined.  It might be necessary to force a unit into a state even though
*					transition wise, like the grolsh, it is not ready yet.
*
***************************************************************************************************/

bool CGuiUnit::BeginFocus( bool bForce )
{
	// We will only respond to this if we are in an idle state
    if ( ( !bForce ) && ( m_eUnitState != STATE_IDLE ) ) return false;

	// Transition to the focus state
	SetStateFocusIn();

	// We have successfully changed state
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/

bool CGuiUnit::BeginExit( bool bForce )
{	
	// Don't respond unless we are in a steady state
    if ( ( !bForce ) && ( ( m_eUnitState == STATE_ENTER ) || ( m_eUnitState == STATE_EXIT ) ) ) return false;

/*	// Go straight to the exit state
	SetStateExit();
*/
	// reroute to the fadeout state
	SetStateFadeOut();

	// We have successfully changed state
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateEnter
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateEnter( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	// Go to the idle state when any entrance anim is over and once we have finished fading
	if ( !IsAnimating() )
	{
//		SetStateIdle();
		SetStateFadeIn();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateIdle
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateIdle( void )
{
	// Update the animation if there is one
	UpdateAnimations();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateFocus
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateFocus( void )
{
	// Update the animation if there is one
	UpdateAnimations();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateFocusIn
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateFocusIn( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	// Go to the focus state when any transition anim is over
	if ( !IsAnimating() )
	{
		SetStateFocus();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateFocusOut
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateFocusOut( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	// Go to the idle state when any transition anim is over
	if ( !IsAnimating() )
	{
		SetStateIdle();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateExit
*
*	DESCRIPTION		This should most likely sit in a script interface in the future.
*
***************************************************************************************************/

void CGuiUnit::UpdateExit( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	// Die when the anim is over and once we have finished fading
	if ( !IsAnimating() )
	{
		SetStateDead();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::UpdateAnimations( void )
{
	if ( m_pobAnimator )
	{
		m_pobAnimator->CreateBlends( CTimer::Get().GetGameTimeChange() );
		m_pobAnimator->UpdateResults();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnit::IsAnimating( void )
{
	return ( m_pobAnimator && m_pobAnimator->IsPlayingAnimation() );
}

/**************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetData
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnit::SetData(const char* pcName, void* pvData)
{
	UNUSED(pcName);
	UNUSED(pvData);
	return false;
}

/**************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetData
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnit::GetData(const char* pcName, void* pvData)
{
	UNUSED(pcName);
	UNUSED(pvData);
	return false;
}


CGuiUnit* CGuiUnit::FindChildUnit(const char* pcUnitID, bool bRecursive)
{
	CHashedString obID(pcUnitID);
	return FindChildUnit(obID, bRecursive);
}

CGuiUnit* CGuiUnit::FindChildUnit(CHashedString& obUnitID, bool bRecursive)
{
	// search our children first
	for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
	{
		CGuiUnit* pobUnit = (CGuiUnit*)(*obIt);

		if (obUnitID == pobUnit->GetUnitID())
			return pobUnit;
	}

	//and failing that, search our children
	if (bRecursive)
	{
		for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
		{
			CGuiUnit* pobUnit = (CGuiUnit*)(*obIt);

			CGuiUnit* pobResult = pobUnit->FindChildUnit(obUnitID, bRecursive);
			if (pobResult)
				return pobResult;
		}
	}

	return NULL;
}

float CGuiUnit::ScreenFade()
{
	if (m_bAutoFade)
	{
		float fade = m_fScreenFadeScale * GetParentScreen()->ScreenFade();
		return min(1.0f, fade);
	}
	return m_fScreenFade;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateFadeIn
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::UpdateFadeIn( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	if ( !IsFading() )
	{
		SetStateIdle();
	}
}
/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::UpdateFadeOut
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::UpdateFadeOut( void )
{
	// Update the animation if there is one
	UpdateAnimations();

	if ( !IsFading() )
	{
		SetStateExit();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateFadeIn
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateFadeIn( void )
{	
	// Set the state flag
	bool bAuto = GetParentScreen() ? GetParentScreen()->m_bAutoFade : m_bAutoFade;
	if (!bAuto)
		SetStateIdle();
	else
		m_eUnitState = STATE_FADEIN;

/*	Going to ignore anims for now.

	// If we are currently playing a looping anim stop it at the end of the loop
	if ( m_pobPreviousAnimation && m_pobPreviousAnimation->IsActive() )
	{
		m_pobPreviousAnimation->ClearFlagBits( ANIMF_LOOPING );
	}

	// If there is an exit anim set it going
	if ( m_astrAnimations[STATE_FADEIN].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_FADEIN].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
*/
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::SetStateFadeOut
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnit::SetStateFadeOut( void )
{	
	// Set the state flag
	bool bAuto = GetParentScreen() ? GetParentScreen()->m_bAutoFade : m_bAutoFade;
	if (!bAuto)
		SetStateExit();
	else
		m_eUnitState = STATE_FADEOUT;

/*	Going to ignore anims for now.

	// If we are currently playing a looping anim stop it at the end of the loop
	if ( m_pobPreviousAnimation && m_pobPreviousAnimation->IsActive() )
	{
		m_pobPreviousAnimation->ClearFlagBits( ANIMF_LOOPING );
	}

	// If there is an exit anim set it going
	if ( m_astrAnimations[STATE_FADEOUT].pobAnimationHeader )
	{
		// Double check we have all we need
		ntAssert ( m_pobAnimator );
		ntAssert ( m_pobHierarchy );

		// Create a new animation from the header
		CAnimationPtr pobAnimation = CAnimation::Create( m_pobAnimator, 0, m_astrAnimations[STATE_FADEOUT].pobAnimationHeader, "NULL" );
		ntAssert( pobAnimation );

		// Kick it off
		m_pobAnimator->AddAnimation( pobAnimation );
		m_pobPreviousAnimation = pobAnimation;
	}
*/
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::IsFading
*
*	DESCRIPTION		We are fading if our parent screen is fading
*
***************************************************************************************************/
bool CGuiUnit::IsFading( void )
{
	if (m_bAutoFade)
		return GetParentScreen() && GetParentScreen()->IsFading();
	return m_bFadeRunning;
}

void CGuiUnit::SetFade(float fFade)
{
	if (m_bAutoFade)
		return;

	m_fScreenFade = fFade;
	m_bFadeRunning = true;
}

void CGuiUnit::SetFadeComplete()
{
	if (m_bAutoFade)
		return;

	m_bFadeRunning = false;
}

bool CGuiUnit::AutoFade()
{
	return m_bAutoFade;
}
