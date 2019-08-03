/***************************************************************************************************
*
*	DESCRIPTION		A screen unit that manages the user selection of button or toggle derived units
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiunitselect.h"

// TO BE REMOVED WHEN WE HAVE A MESSAGE SYSTEM
#include "guimanager.h"

#include "guisound.h"

#include "gui/guiscreen.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::CGuiUnitSelect
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiUnitSelect::CGuiUnitSelect( void )
{
	// Initialise the selection width to something sensible - the actual default will be defined in the DTD
	m_iSelectionWidth = 1;

	// Initialise the looping characteristic - again the DTD will define a game default
	m_bLooping = false;

	// Initialise the array of pointers to the selectedable items
	m_papobSelectableUnits = 0;
	m_iSelectedUnit = 0;

}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::~CGuiUnitSelect
*
*	DESCRIPTION		Destruction
*
*					There is no need to kill the child units here since this structure is built up 
*					at the CXMLElement level - and will be destroyed by it too.
*
***************************************************************************************************/

CGuiUnitSelect::~CGuiUnitSelect( void )
{
	// Destroy our array of pointers
	// No need to worry about the content here either
	NT_DELETE_ARRAY( m_papobSelectableUnits );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::Render
*
*	DESCRIPTION		Pass render onto contained GUI units
*
***************************************************************************************************/

bool CGuiUnitSelect::Render( void )
{
	// Render all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Render();
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateEnter
*
*	DESCRIPTION		We have to do some extra work before we come out of the entrance state.
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateEnter( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	// If we are done, try to focus on the selected child element
	if (GetParentScreen()->AutoFade())
	{
		if ( !IsAnimating() )
		{
			SetStateFadeIn();
		}
	}
	else
	{
		if ( !IsAnimating() && m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus() )
		{
			SetStateIdle();
		}
	}
}

void CGuiUnitSelect::UpdateFadeIn()
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	if ( !IsFading() )
	{
		// Try to set our selected item to focused
		if ( m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus() )
		{
			SetStateIdle();
		}
	}
}

void CGuiUnitSelect::UpdateFadeOut()
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	if ( !IsFading() )
	{
		SetStateExit();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateIdle
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateIdle( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateIdle();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateFocus
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateFocus( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocus();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateFocusIn( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocusIn();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateFocusOut
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateFocusOut( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocusOut();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiUnitSelect::UpdateExit( void )
{
	// Monitor of the children are alive
	bool bLiveChildren = false;

	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		bLiveChildren |= ( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	// Are the children dead dear?
	if ( !bLiveChildren )
	{
		// If we are done too
		if ( !IsAnimating() )
		{
			SetStateDead();
		}
	}

}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::BeginExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::BeginExit( bool bForce )
{
	// Call the base update first for this element
	bool bSuccess = CGuiUnit::BeginExit( bForce );

	// Now call it for all our children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		bSuccess &= ( *obIt )->BeginExit( bForce );
	}

	// Returns true if all elements changed state 
	return bSuccess;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiUnitSelect::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( stricmp( pcTitle, "selectionwidth" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iSelectionWidth );
		}

		else if ( stricmp( pcTitle, "looping" ) == 0 )
		{
			return GuiUtil::SetBool( pcValue, &m_bLooping );
		}

		else if ( stricmp( pcTitle, "looping" ) == 0 )
		{
			return GuiUtil::SetBool( pcValue, &m_bLooping );
		}

		else if ( stricmp( pcTitle, "selection" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iSelectedUnit );
		}
		else if ( strcmp( pcTitle, "removeitem" ) == 0 )
		{
			RemoveItem( pcValue );
			return true;
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::ProcessChild
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::ProcessChild( CXMLElement* pobChild )
{
	// Drop out if the data is shonky
	ntAssert( pobChild );

	// The screen only knows that it holds a list of screen units
	m_obSelectableUnits.push_back( ( CGuiUnit* )pobChild );

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  Hence at this
*					stage we know we have all the information that will be set by a script.  This
*					function is used to set up any information that requireds two or more attributes
*					to avoid any attribute ordering issues.
*
***************************************************************************************************/

bool CGuiUnitSelect::ProcessEnd( void )
{
	// Call the base class first
	CGuiUnit::ProcessEnd();

	BuildPointerList();

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::BuildPointerList
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CGuiUnitSelect::BuildPointerList()
{
	// Make sure that sensible values have been set up
	ntAssert( m_iSelectionWidth > 0 );
	ntAssert( m_obSelectableUnits.size() > 0 );
	ntAssert( ( m_obSelectableUnits.size() % m_iSelectionWidth ) == 0 );

	if (m_papobSelectableUnits)
		NT_DELETE_ARRAY(m_papobSelectableUnits);

	// Gather the child pointers into a nice array we can move between
	m_papobSelectableUnits = NT_NEW CGuiUnit*[m_obSelectableUnits.size()];
	ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin();
	for( int iIndex = 0; obIt != m_obSelectableUnits.end(); ++obIt )
	{
		m_papobSelectableUnits[iIndex] = ( *obIt );
		iIndex++;
	}

	if (m_iSelectedUnit >= (int)m_obSelectableUnits.size())
		m_iSelectedUnit = ((int)m_obSelectableUnits.size()) - 1;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::MoveLeftAction
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CGuiUnitSelect::RemoveItem( const char* pcItem)
{
	CGuiUnit* pobItem = NULL;
	CHashedString obID(pcItem);
	
	ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin();
	for( ; obIt != m_obSelectableUnits.end(); ++obIt )
	{
		pobItem = *obIt;

		if (pobItem->GetUnitID() == obID)
		{
			m_obSelectableUnits.erase(obIt);
			//RemoveChild(pobItem);
			//NT_DELETE(pobItem);		// We dont actually remove it from the base XML object. it will handle this itself later
			pobItem = NULL;
			break;
		}		
	}

	BuildPointerList();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::MoveLeftAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::MoveLeftAction( int iPads )
{
	// Set up our return value
	bool bActedOn = false;

	// If the selection width is one we can pass this message on to the selected child
	if ( m_iSelectionWidth == 1 )
	{
		bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->MoveLeftAction( iPads );
	}

	// Otherwise find a new selected item
	else
	{
		// Only do after a minimum time period
		if ( m_obPadTimer.Passed() )
		{
			int iCurrentUnit = m_iSelectedUnit;

			// Check if we are on a boundry
			if ( ( m_iSelectedUnit % m_iSelectionWidth ) == 0 )
			{
				// If we are looping - jump to the other side
				if ( m_bLooping )
				{
					m_iSelectedUnit--;
					m_iSelectedUnit += m_iSelectionWidth;
				}
			}

			// Otherwise simply decrement the selected item
			else
			{
				m_iSelectedUnit--;
			}

			// If we got a new item
			if (m_iSelectedUnit != iCurrentUnit )
			{
				// If we can remove the focus from the current item
				if	( m_papobSelectableUnits[iCurrentUnit]->BeginIdle() ) 
				{
				
					// Focus on the new item - force it to be so
					m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus( true );

					// Set a minimum 'act again' time
					m_obPadTimer.Set( m_fPadTime );

					bActedOn = true;
				}
				else
				// Opps, we should undo our attempt to change the selected unit
				{
					m_iSelectedUnit = iCurrentUnit;
				}
			}
		}
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::MoveRightAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::MoveRightAction( int iPads )
{
	// Set up our return value
	bool bActedOn = false;

	// If the selection width is one we can pass this message on to the selected child
	if ( m_iSelectionWidth == 1 )
	{
		bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->MoveRightAction( iPads );
	}

	// Otherwise find a new selected item
	else
	{
		// Only do after a minimum time period
		if ( m_obPadTimer.Passed() )
		{
			int iCurrentUnit = m_iSelectedUnit;

			// Increment the selected item
			m_iSelectedUnit++;
				
			// Check we haven't crossed any boundries
			if  ( ( m_iSelectedUnit % m_iSelectionWidth ) == 0 )
			{
				// If we are looping - jump to the other side
				if ( m_bLooping )
				{
					m_iSelectedUnit -= m_iSelectionWidth;
				}

				// Otherwise we need to simply undo the increment
				else
				{
					m_iSelectedUnit--;
				}
			}

			// If we got a new item
			if (m_iSelectedUnit != iCurrentUnit )
			{
				// If we can remove the focus from the current item
				if	( m_papobSelectableUnits[iCurrentUnit]->BeginIdle() ) 
				{
					// Focus on the new item - force it to be so
					m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus( true );

					// Set a minimum 'act again' time
					m_obPadTimer.Set( m_fPadTime );

					bActedOn = true;
				}
				else
				// Opps, we should undo our attempt to change the selected unit
				{
					m_iSelectedUnit = iCurrentUnit;
				}
			}
		}
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::MoveDownAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::MoveDownAction( int iPads )
{
	// Set up our return value
	bool bActedOn = false;

	// If the selection height is one we can pass this message on to the selected child
	if ( ( m_obSelectableUnits.size() / m_iSelectionWidth ) == 1 )
	{
		bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->MoveDownAction( iPads );
	}

	// Otherwise find a new selected item
	else
	{
		// Only do after a minimum time period
		if ( m_obPadTimer.Passed() )
		{
			int iCurrentUnit = m_iSelectedUnit;

			// Increase the selected index by the width
			m_iSelectedUnit += m_iSelectionWidth;

			// Check to see if we have crossed any boundries
			if ( m_iSelectedUnit >= static_cast<int>( m_obSelectableUnits.size() ) )
			{
				// If we are looping - jump back to the top
				if ( m_bLooping )
				{
					m_iSelectedUnit -= m_obSelectableUnits.size();
				}

				// Otherwise simply undo the change
				else
				{
					m_iSelectedUnit -= m_iSelectionWidth;
				}
			}
			
			// If we got a new item
			if (m_iSelectedUnit != iCurrentUnit )
			{
				// If we can remove the focus from the current item
				if	( m_papobSelectableUnits[iCurrentUnit]->BeginIdle() ) 
				{
				
					// Focus on the new item - force it to be so
					m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus( true );

					// Set a minimum 'act again' time
					m_obPadTimer.Set( m_fPadTime );

					bActedOn = true;
				}
				else
				// Opps, we should undo our attempt to change the selected unit
				{
					m_iSelectedUnit = iCurrentUnit;
				}
			}
		}
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::MoveUpAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::MoveUpAction( int iPads )
{
	// Set up our return value
	bool bActedOn = false;

	// If the selection height is one we can pass this message on to the selected child
	if ( ( m_obSelectableUnits.size() / m_iSelectionWidth ) == 1 )
	{
		bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->MoveUpAction( iPads );
	}

	// Otherwise find a new selected item

	else
	{
		// Only do after a minimum time period
		if ( m_obPadTimer.Passed() )
		{
			int iCurrentUnit = m_iSelectedUnit;

			// Increase the selected index by the width
			m_iSelectedUnit -= m_iSelectionWidth;

			// Check to see if we have crossed any boundries
			if ( m_iSelectedUnit < 0 )
			{
				// If we are looping - jump back to the top
				if ( m_bLooping )
				{
					m_iSelectedUnit += m_obSelectableUnits.size();
				}

				// Otherwise simply undo the change
				else
				{
					m_iSelectedUnit += m_iSelectionWidth;
				}
			}
			
			// If we got a new item
			if (m_iSelectedUnit != iCurrentUnit )
			{
				// If we can remove the focus from the current item
				if	( m_papobSelectableUnits[iCurrentUnit]->BeginIdle() ) 
				{
				
					// Focus on the new item - force it to be so
					m_papobSelectableUnits[m_iSelectedUnit]->BeginFocus( true );

					// Set a minimum 'act again' time
					m_obPadTimer.Set( m_fPadTime );

					bActedOn = true;
				}
				else
				// Opps, we should undo our attempt to change the selected unit
				{
					m_iSelectedUnit = iCurrentUnit;
				}
			}
		}
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::StartAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::StartAction( int iPads )
{
	bool m_bActedOn = false;
	// Pass the message on to the selected unit
	m_bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->StartAction( iPads );

	// If a selected item has not made an action then we need to move on
	if (! m_bActedOn )
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		CGuiManager::Get().MoveOnScreen( m_iSelectedUnit + 1 );

		if (! CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

	}

	// Did we act on this message?
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::SelectAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::SelectAction( int iPads )
{
	bool m_bActedOn = false;
	// Pass the message on to the selected unit
	m_bActedOn = m_papobSelectableUnits[m_iSelectedUnit]->SelectAction( iPads );

	// If a selected item has not made an action then we need to move on
	if (! m_bActedOn )
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
		CGuiManager::Get().MoveOnScreen( m_iSelectedUnit + 1 );
		if (! CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
		
	}

	// Did we act on this message?
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::BackAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::BackAction( int iPads )
{
	// Pass the message on to the selected unit
	return m_papobSelectableUnits[m_iSelectedUnit]->BackAction( iPads );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitSelect::DeleteAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiUnitSelect::DeleteAction( int iPads )
{
	// Pass the message on to the selected unit
	return m_papobSelectableUnits[m_iSelectedUnit]->DeleteAction( iPads );
}

