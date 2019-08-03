/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface select unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "guiskinselect.h"
#include "guimanager.h"

#include "guisound.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinTestSelect(); }

// Register this class under it's XML tag
bool g_bSELECT = CGuiManager::Register( "SELECT", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestSelect::CGuiSkinTestSelect
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinTestSelect::CGuiSkinTestSelect( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestSelect::~CGuiSkinTestSelect
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinTestSelect::~CGuiSkinTestSelect( void )
{
}

bool	CGuiSkinTestSelect::MoveDownAction( int iPads )
{
	if ( CGuiUnitSelect::MoveDownAction(iPads) )
	{
		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_DOWN);
		return true;
	}
	return false;
}

bool	CGuiSkinTestSelect::MoveUpAction( int iPads )
{
	if ( CGuiUnitSelect::MoveUpAction(iPads) )
	{
		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_UP);
		return true;
	}
	return false;
}

bool	CGuiSkinTestSelect::StartAction( int iPads )
{
	if ( CGuiUnitSelect::StartAction(iPads) )
	{
		return true;
	}
	return false;
}

bool	CGuiSkinTestSelect::SelectAction( int iPads )
{
	if ( CGuiUnitSelect::SelectAction(iPads) )
	{
		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
		return true;
	}
	return false;
}

bool	CGuiSkinTestSelect::BackAction( int iPads )
{
	if ( CGuiUnitSelect::BackAction(iPads) )
	{
		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_BACK);
		return true;
	}
	return false;
}

bool	CGuiSkinTestSelect::DeleteAction( int iPads )
{
	if ( CGuiUnitSelect::DeleteAction(iPads) )
	{
		return true;
	}
	return false;
}
