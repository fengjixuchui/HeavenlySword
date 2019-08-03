/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that will control screen timeout
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskintimer.h"
#include "guimanager.h"
#include "guiutil.h"
#include "guiflow.h"

#ifdef DEBUG_GUI
#	include "core/visualdebugger.h"
#endif

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinTimer(); }

// Register this class under it's XML tag
bool g_bTIMER = CGuiManager::Register( "TIMER", &ConstructWrapper );

CGuiSkinTimer::CGuiSkinTimer( void )
	: m_fTime(0.0f)
	, m_iTargetScreen(0)	
	, m_bAllowReset(false)
	, m_obAction(CGuiAction::MOVE_ON)
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTimer::ProcessAttribute
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinTimer::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitStatic::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "countdowntime" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fTime );
		}

		if ( strcmp( pcTitle, "targetscreen" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrScreenFlags[0], &m_iTargetScreen );
		//	ntAssert(false && "Unsupported: targetscreen");
		}

		if ( strcmp( pcTitle, "allowreset" ) == 0 )
		{
			return GuiUtil::SetBool( pcValue, &m_bAllowReset );
		}

		//pass down to action to see if it needs it
		return m_obAction.ProcessAttribute(pcTitle, pcValue);
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTimer::ProcessEnd
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinTimer::ProcessEnd( void )
{
	ntAssert ( m_fTime > ANIM_BLEND_TIME );

	// Kick off our timer
	m_obCountdownTime.Set( m_fTime );

	return CGuiUnitStatic::ProcessEnd();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTimer::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinTimer::Update( void )
{
	// Reduce our timer only in the idle state
	if ( m_eUnitState == STATE_IDLE )
	{
		// Take away the time change
		m_obCountdownTime.Update();

		// If we have finished we get out of here
		if ( m_obCountdownTime.Passed() )
		{
			//if (! CGuiManager::Get().MoveOnScreenType( m_iTargetScreen ) )
			//	CGuiManager::Get().MoveBackScreenType( m_iTargetScreen );

			//keep the old system aswell for now :( -> m_iTargetScreen
			if (m_iTargetScreen == 0)
			{
				m_obAction.TriggerAction(GetParentScreen());
			}
			else
			{
				if (! CGuiManager::Get().MoveOnScreenType( m_iTargetScreen ) )
					CGuiManager::Get().MoveBackScreenType( m_iTargetScreen );
			}

			//return false;
		}
	}

#ifdef DEBUG_GUI
	g_VisualDebug->Printf2D(10,30,DC_RED, 0, "Timer %.3f", m_obCountdownTime.Time() );
#endif

	// Call the base update
	return CGuiUnitStatic::Update();

	//return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTimer::AnyAction
*
*	DESCRIPTION		If we are alowed to reset the timer do it now
*
***************************************************************************************************/

bool	CGuiSkinTimer::AnyAction( int iPads )
{
	UNUSED(iPads);

	// A button was pressed, reset the timer if allowed
	if (m_bAllowReset)
		m_obCountdownTime.Set( m_fTime );

	// Return false as another element may want to use this input
	return false;	
}
