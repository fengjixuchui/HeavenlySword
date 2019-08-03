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
{
	// Initalise the animation details
	for ( int iState = 0; iState < UNIT_STATE_SIZE; iState++ )
	{
		m_astrAnimations[iState].pobAnimationHeader = 0;
	}

	// Inatialise the state flag
	m_eUnitState = STATE_BORN;
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
	default:				ntAssert( 0 );		break;
	}

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
	return true;
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

	// Go straight to the exit state
	SetStateExit();

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

	// Go to the idle state when any entrance anim is over
	if ( !IsAnimating() )
	{
		SetStateIdle();
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

	// Die when the anim is over
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
		m_pobAnimator->Update( CTimer::Get().GetGameTimeChange() );
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
